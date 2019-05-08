import argparse
import os.path
import datetime
import conduit
import conduit.relay
import numpy as np
import tarfile


"""
Running the PPL2 conversion on LC

cd <your_running_directory_contains_scratch/lig>
 
/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL2toCDT2hdf5.py
 
Or if you provide inputs:
 
/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL2toCDT2hdf5.py -d <path_to_scratch> -o <path_to_hdf5_file>

It assume that the scratch/lig has following data strcture

scratch/lig/1
scratch/lig/2
...
scratch/lig/N

Each directory must have: 'checkpoint.txt' (Otherwise it will skip the directory)
and may including: 'LIG.inpcrd', 'LIG.lib', 'LIG.prmtop', 'LIG_min.rst', 'LIG_minGB.out', 'ligand.frcmod', 'LIG_min.pdbqt'

If the required files are in the 'output.tgz'

/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL2toCDT2hdf5.py -z 

"""


def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--scrDir', action='store', dest='scrDir', default='scratch',
                        help='path to scratch directory')
    parser.add_argument('-s', '--sdf', action='store', dest='sdffile', default=None,
                        help='path to scratch directory')
    parser.add_argument('-os', '--outsdf', action='store', dest='outsdf', default=None,
                        help='path to scratch directory')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='scratch/ligand.hdf5',
                        help='path to ligand.hdf5 file')
    parser.add_argument('-z', '--isZip', action='store_true', dest='isZip', default=False,
                        help='path to scratch directory')

    args = parser.parse_args()

    return args

def parseCheckpoint(checkfile):
    checkData={}
    with open(checkfile, "r") as f:
        for line in f:
            strs=line.strip().split(':')
            if len(strs)==2:
                checkData[strs[0]]=strs[1]

    return checkData

def filesToHDF(n, cmpdKey, fileList):

    for file in fileList:
        if os.path.isfile(file):
            with open(file, 'r') as f:
                n[cmpdKey+"/file/"+file] = f.read()

def extract_files(members):
    extractList = ['checkpoint.txt', 'ligand.frcmod', 'LIG_minGB.out']
    for tarinfo in members:
        if os.path.basename(tarinfo.name) in extractList:
            yield tarinfo

def saveSDFtoHDF(args):
    if os.path.isfile(args.sdffile):
        with open(args.sdffile, 'r') as f:
            nSDF= conduit.Node()
            nSDF['SDF']=f.read()
            hdf5path = os.path.abspath(args.outfile)
            conduit.relay.io.save_merged(nSDF, hdf5path)
def extracSDF(args):
    hdf5path = os.path.abspath(args.outfile)
    n = conduit.Node()
    conduit.relay.io.load(n, hdf5path)
    with open(args.outsdf, 'w') as f:
        filedata = n['SDF']
        if len(filedata) > 0:
            f.write(filedata)
        else:
            print("Missing SDF data")

def main():
    args=getArgs()
    print("Default inputs: ", args.scrDir, args.outfile, args.sdffile)

    # if read in SDF
    if args.sdffile:
        saveSDFtoHDF(args)
        return

    if args.outsdf:
        extracSDF(args)
        return

    hdf5path = os.path.abspath(args.outfile)
    finishedList = []
    if os.path.isfile(hdf5path):
        n = conduit.Node()
        conduit.relay.io.load(n, hdf5path)

        itr = n['lig'].children()

        for id in itr:
            finishedList.append(id.name())

        print("Finished ligand list")
        print(finishedList)

    nHeader = conduit.Node()
    nHeader['date'] ="Created by PPL2toCDT2hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S")

    conduit.relay.io.save_merged(nHeader, hdf5path)

    print("Created by PPL2toCDT2hdf5.py at "+datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))

    print(hdf5path)
    ligDirPath = os.path.abspath(args.scrDir + "/lig")
    print(ligDirPath)
    os.chdir(ligDirPath)
    dirs = os.listdir(".")
    for cmpd in dirs:
        if cmpd.isdigit() and cmpd not in finishedList:
            n = conduit.Node()
            cmpdPath = os.path.join(ligDirPath, cmpd)
            #print(cmpdPath)
            os.chdir(cmpdPath)
            print(os.getcwd())
            if args.isZip:
                zipfile=os.path.join(cmpdPath, "output.tgz")
                if os.path.isfile(zipfile):
                    tar = tarfile.open(zipfile)
                    tar.extractall(members=extract_files(tar))
                    tar.close()

            checkfile = os.path.join(cmpdPath, "checkpoint.txt")
            if os.path.isfile(checkfile):
                parseCheckpoint(checkfile)
                checkData=parseCheckpoint(checkfile)
                cmpdKey='/lig/' + cmpd
                print(checkData)
                if 'Mesg' in checkData:
                    if checkData['Mesg'] == 'Finished!':
                        n[cmpdKey + '/status'] = np.int32(1)
                    else:
                        n[cmpdKey + '/status'] = np.int32(0)
                    n[cmpdKey+'/meta/Mesg']=checkData['Mesg']
                    n[cmpdKey+'/meta/Mesg']=checkData['Mesg']

                if 'ligName' in checkData:
                    n[cmpdKey + '/meta/name'] = checkData['ligName']

                if 'GBSA' in checkData:
                    n[cmpdKey + '/meta/GBEN'] = float(checkData['GBSA'])

                n[cmpdKey + '/meta/LigPath']=cmpdPath

                fileList=['LIG.inpcrd', 'LIG.lib', 'LIG.prmtop', 'LIG_min.rst', 'LIG_minGB.out', 'ligand.frcmod', 'LIG_min.pdbqt']

                filesToHDF(n, cmpdKey, fileList)

                conduit.relay.io.save_merged(n, hdf5path)

                if args.isZip:
                    extractList = ['checkpoint.txt', 'ligand.frcmod', 'LIG_minGB.out']
                    for file in extractList:
                        if os.path.isfile(file):
                            os.remove(file)


if __name__ == '__main__':
    main()