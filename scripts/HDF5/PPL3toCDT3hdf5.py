import argparse
import os.path
import datetime
import math
import conduit
import conduit.relay
import numpy as np
import glob
from mpi4py import MPI


"""
Running the serial PPL3 conversion on LC

cd <your_running_directory_contains_scratch/lig>

/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL3toCDT3hdf5.py

Or if you provide inputs:

/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL3toCDT3hdf5.py -d <path_to_scratch> -o <path_to_directory_hdf5_files>


Running the parallel version using the script:
--------------
#!/bin/bash
#MSUB -A bbs
#MSUB -l nodes=2
#MSUB -l walltime=16:00:00
#MSUB -l partition=syrah
#MSUB -q pbatch
#MSUB -m be
#MSUB -N x02
#MSUB -e x02.err
#MSUB -o x02.out

ml python
module load gcc/4.9.3
module load mvapich2/2.2

export SPACK_ROOT=/usr/gapps/bbs/TOSS-3/spack
export PATH=$SPACK_ROOT/bin:$PATH
source $SPACK_ROOT/share/spack/setup-env.sh

spack load python
spack load py-mpi4py
spack load py-numpy
spack load hdf5
spack load conduit

srun -N2 -n32 python PPL3toCDT3hdf5.py
---------------


It assume that the scratch/lig has following data structure

scratch/com/<pdbname1>/dock/1
scratch/com/<pdbname1>/dock/2
...
scratch/com/<pdbname1>/dock/N
...
scratch/com/<pdbnameM>/dock/1
...
scratch/com/<pdbnameM>/dock/N

Each directory must have: 'scores.log' (Otherwise it will skip the directory)
and may including: 'poses.pdbqt'
"""


def getArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--scrDir', action='store', dest='scrDir', default='scratch',
                        help='path to scratch directory')
    parser.add_argument('-o', '--outDir', action='store', dest='outfile', default='scratch/dockHDF5',
                        help='path to docking HDF5 files')

    args = parser.parse_args()

    return args


def filesToHDF(n, cmpdKey, fileList):
    for file in fileList:
        if os.path.isfile(file):
            with open(file, 'r') as f:
                n[cmpdKey + "/file/" + file] = f.read()
        else:
            print("File - "+file+" no found")

def getScore(scorefile):
    scores={}
    with open(scorefile, "r") as f:
        lines = f.readlines()
        for i in range(5, len(lines)):
            line = lines[i]
            strs=line.split()
            if len(strs)==4:
                scores[strs[0]]=float(strs[1])

    return scores

def PPL3toCDT3(args):

    nHeader = conduit.Node()
    nHeader['date'] = "Created by PPL3toCDT3hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S")
    print("Created by PPL3toCDT3hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))

    if not os.path.exists(args.outfile):
        os.makedirs(args.outfile)

    hdf5pathDir = os.path.abspath(args.outfile)
    hdf5path=os.path.join(hdf5pathDir, "dock_proc1.hdf5")
    print(hdf5path)

    conduit.relay.io.save(nHeader, hdf5path)

    comDirPath = os.path.abspath(args.scrDir + "/com")
    print(comDirPath)
    os.chdir(comDirPath)
    dirs = os.listdir(".")
    for recid in dirs:
        recPath = os.path.join(comDirPath, recid+"/dock")
        if os.path.isdir(recPath):
            os.chdir(recPath)
            print(os.getcwd())

            n = conduit.Node()

            ligs=os.listdir(".")

            for ligid in ligs:
                ligPath = os.path.join(recPath, ligid)
                if os.path.isdir(ligPath):
                    os.chdir(ligPath)
                    scorefile=os.path.join(ligPath, 'scores.log')

                    if os.path.isfile(scorefile):
                        entryKey = "/dock/" + recid + "/" + ligid
                        scores=getScore(scorefile)
                        numPose=len(scores)
                        n[entryKey + "/meta/numPose"] = np.int32(numPose)
                        if numPose>0:
                            n[entryKey + "/status"] = np.int32(1)
                            n[entryKey + "/meta/Mesg"] ="Finished!"
                        else:
                            n[entryKey + "/status"] = np.int32(0)
                            n[entryKey + "/meta/Mesg"] = "No Scores!"

                        for key, val in scores.iteritems():
                            n[entryKey+"/meta/scores/"+key]=val

                        fileList = ['poses.pdbqt', 'scores.log']

                        filesToHDF(n, entryKey, fileList)
            try:
                conduit.relay.io.save_merged(n, hdf5path)
            except:
                print(recid+ " cannot be saved into HDF5")

def PPL3toCDT3_MPI(args):
    comm=MPI.COMM_WORLD
    rank=comm.rank
    size = comm.size
    #print(rank, size)
    if rank==0:
        if not os.path.exists(args.outfile):
            os.makedirs(args.outfile)

    hdf5pathDir = os.path.abspath(args.outfile)

    keyList=[]

    comDirPath = os.path.abspath(args.scrDir + "/com")
    if rank==0:
        print(comDirPath)
    os.chdir(comDirPath)
    dirs = os.listdir(".")

    #for recid in dirs:
    for i in xrange(rank, len(dirs), size):
        recid=dirs[i]
        #print(rank, i, recid)
        recPath = os.path.join(comDirPath, recid+"/dock")
        if os.path.isdir(recPath):
            os.chdir(recPath)
            #print(rank, os.getcwd())
            ligs=os.listdir(".")

            for ligid in ligs:
                entryKey = recid + "/" + ligid
                keyList.append(entryKey)

    allKeys=comm.gather(keyList, root=0)

    calcKeyList=[]
    if rank==0:
        calcKeyList=[item for sublist in allKeys for item in sublist]

    calcKeyList = comm.bcast(calcKeyList, root=0)

    if rank==0:
        print(rank, 'calcKeyList initial', calcKeyList)

    # figure out the keys already in HDF5 files
    os.chdir(hdf5pathDir)
    h5files=glob.glob('dock_proc*.hdf5')
    h5KeyList=[]
    for i in xrange(rank, len(h5files), size):
        h5f=h5files[i]
        n = conduit.Node()
        conduit.relay.io.load(n, h5f)
        itrRec = n["dock"].children()
        for rec in itrRec:
            recid=rec.name()
            nrec=rec.node()
            itrLig=nrec.children()
            for lig in itrLig:
                ligid = lig.name()
                entryKey = recid + "/" + ligid
                h5KeyList.append(entryKey)

    allh5KeyList = comm.gather(h5KeyList, root=0)
    finishKeyList=[]
    if rank==0:
        finishKeyList=[item for sublist in allh5KeyList for item in sublist]

    finishKeyList = comm.bcast(finishKeyList, root=0)

    if rank==0:
        print(rank, 'finishKeyList', finishKeyList)

    calcKeyList=[item for item in calcKeyList if item not in finishKeyList]
    calcKeySize = len(calcKeyList)

    if rank==0:
        print(rank, 'calcKeyList end ',  calcKeyList)

    if rank<calcKeySize:

        nHeader = conduit.Node()
        nHeader['date'] = "Created by PPL3toCDT3hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S")
        if rank==0:
            print("Created by PPL3toCDT3hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))


        hdf5path=os.path.join(hdf5pathDir, "dock_proc"+str(rank)+".hdf5")
        print(rank, hdf5path)

        conduit.relay.io.save(nHeader, hdf5path)

        for i in xrange(rank, calcKeySize, size):
            calcKey=calcKeyList[i]
            strs=calcKey.split("/")
            #print(rank, strs)
            if len(strs)==2:
                recid=strs[0]
                ligid=strs[1]
                ligPath = os.path.join(comDirPath, recid + "/dock/"+ligid)
                print(rank, ligPath)
                if os.path.isdir(ligPath):
                    os.chdir(ligPath)
                    scorefile=os.path.join(ligPath, 'scores.log')
                    if os.path.isfile(scorefile):
                        entryKey = "/dock/" + recid + "/" + ligid
                        n = conduit.Node()
                        scores=getScore(scorefile)
                        numPose=len(scores)
                        n[entryKey + "/meta/numPose"] = numPose
                        if numPose>0:
                            n[entryKey + "/status"] = np.int32(1)
                            n[entryKey + "/meta/Mesg"] ="Finished!"
                        else:
                            n[entryKey + "/status"] = np.int32(0)
                            n[entryKey + "/meta/Mesg"] = "No Scores!"

                        for key, val in scores.iteritems():
                            n[entryKey+"/meta/scores/"+key]=val

                        fileList = ['poses.pdbqt', 'scores.log']

                        filesToHDF(n, entryKey, fileList)
                        try:
                            conduit.relay.io.save_merged(n, hdf5path)
                        except:
                            print(entryKey+" cannot be saved into HDF5")


def main():
    comm=MPI.COMM_WORLD
    size=comm.size
    rank=comm.rank

    args = getArgs()

    if rank==0:
        print("Default inputs: ", args.scrDir, args.outfile)

    if size==1:
        PPL3toCDT3(args)
    else:
        PPL3toCDT3_MPI(args)

if __name__ == '__main__':
    main()