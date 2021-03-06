import argparse
import os.path
import datetime
import math
import conduit
import conduit.relay
import numpy as np
import glob


"""
Running the PPL2 conversion on LC

cd <your_running_directory_contains_scratch/lig>

/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL1toCDT1hdf5.py

Or if you provide inputs:

/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL1toCDT1hdf5.py -d <path_to_scratch> -o <path_to_hdf5_file>

It assume that the scratch/lig has following data structure

scratch/lig/1
scratch/lig/2
...
scratch/lig/N

Each directory must have: 'checkpoint.txt' (Otherwise it will skip the directory)
and may including: 'LIG.inpcrd', 'LIG.lib', 'LIG.prmtop', 'LIG_min.rst', 'LIG_minGB.out', 'ligand.frcmod', 'LIG_min.pdbqt'
"""


def getArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--scrDir', action='store', dest='scrDir', default='scratch',
                        help='path to scratch directory')
    parser.add_argument('-c', '--checkData', nargs=2, action='store', dest='checkdata', default=None,
                        help='protein name and checkpoint file name (e.g. sarinXtalnAChE  checkpoint.txt)')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='scratch/receptor.hdf5',
                        help='path to ligand.hdf5 file')

    args = parser.parse_args()

    return args


def parseCheckpoint(checkfile):
    checkData = {}
    with open(checkfile, "r") as f:
        for line in f:
            strs = line.strip().split(':')
            if len(strs) == 2:
                checkData[strs[0]] = strs[1]

    return checkData


def filesToHDF(n, cmpdKey, fileList):
    for file in fileList:
        if os.path.isfile(file):
            with open(file, 'r') as f:
                n[cmpdKey + "/file/" + file] = f.read()

def equalFloat(v1, v2):
    if math.fabs(v1-v2) < 1:
        return True
    return False

def findNonAA():
    nonAA=[]
    if os.path.isfile("rec_leap.in"):
        with open("rec_leap.in", "r") as f:
            for line in f:
                if line[0:7]=='loadoff':
                    strs=line.split()
                    if len(strs)==2:
                        base = os.path.basename(strs[1])
                        aa = os.path.splitext(base)[0]
                        nonAA.append(aa)
    return nonAA

def findCluster(volume, cx, cy, cz):
    if os.path.isfile("site.txt"):
        with open("site.txt", "r") as f:
            lines = f.readlines()
            for i in range(0, len(lines)):
                line=lines[i]
                if line[0:7]=='Cluster':
                    if line[15:21]=='volume':
                        strs=line.split()
                        vol=float(strs[4])
                        if equalFloat(vol, volume):
                            substrs=lines[i+2].split()
                            if len(substrs)==6:
                                xe = equalFloat(float(substrs[1]), cx)
                                ye = equalFloat(float(substrs[3]), cy)
                                ze = equalFloat(float(substrs[5]), cz)
                                if xe and ye and ze:
                                    return (True, strs[1])
    return (False, None)

def setCheckData(n, checkData, recKey):
    if 'Mesg' in checkData:
        if checkData['Mesg'] == 'Finished!':
            n[recKey + '/status'] = np.int32(1)
        else:
            n[recKey + '/status'] = np.int32(0)
        n[recKey + '/meta/Mesg'] = checkData['Mesg']

    if 'GBEN' in checkData:
        n[recKey + '/meta/GBEN'] = float(checkData['GBEN'])

    if 'Volume' in checkData:
        volume = float(checkData['Volume'])
        n[recKey + '/meta/Site/Volume'] = volume

    if 'Cluster' in checkData:
        clust = int(checkData['Cluster'])
        n[recKey + '/meta/Site/Cluster'] = clust

    if 'cx' in checkData:
        cx = float(checkData['cx'])
        n[recKey + '/meta/Site/Centroid/X'] = cx
    if 'cy' in checkData:
        cy = float(checkData['cy'])
        n[recKey + '/meta/Site/Centroid/Y'] = cy
    if 'cz' in checkData:
        cz = float(checkData['cz'])
        n[recKey + '/meta/Site/Centroid/Z'] = cz
    if 'dx' in checkData:
        n[recKey + '/meta/Site/Dimension/X'] = float(checkData['dx'])
    if 'dy' in checkData:
        n[recKey + '/meta/Site/Dimension/Y'] = float(checkData['dy'])
    if 'dz' in checkData:
        n[recKey + '/meta/Site/Dimension/Z'] = float(checkData['dz'])

    return (volume, clust, cx, cy, cz)


def updateCheckData(args):
    hdf5path = os.path.abspath(args.outfile)
    print(hdf5path)
    print(args.checkdata)

    if os.path.isfile(args.checkdata[1]):
        n = conduit.Node()
        checkData = parseCheckpoint(args.checkdata[1])
        recKey = '/rec/' + args.checkdata[0]
        print(checkData)
        (volume, clust, cx, cy, cz) = setCheckData(n, checkData, recKey)
        try:
            conduit.relay.io.save_merged(n, hdf5path)
        except:
            print(args.checkdata[0] + " cannot be saved in HDF5")
    else:
        print("File - "+args.checkdata[1]+" doesn't exist")

def main():
    args = getArgs()
    print("Default inputs: ", args.scrDir, args.outfile, args.checkdata)

    # just update checkpoint.txt file for one protein
    if args.checkdata:
        updateCheckData(args)
        return

    nHeader = conduit.Node()
    nHeader['date'] = "Created by PPL1toCDT1hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S")
    print("Created by PPL1toCDT1hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))

    hdf5path = os.path.abspath(args.outfile)
    print(hdf5path)

    conduit.relay.io.save_merged(nHeader, hdf5path)

    comDirPath = os.path.abspath(args.scrDir + "/com")
    print(comDirPath)
    os.chdir(comDirPath)
    dirs = os.listdir(".")

    for recid in dirs:
        recPath = os.path.join(comDirPath, recid+"/rec")
        if os.path.isdir(recPath):
            # print(cmpdPath)
            os.chdir(recPath)
            print(os.getcwd())

            clust = None
            cx=None
            cy=None
            cz=None
            volume=None

            checkfile = os.path.join(recPath, "checkpoint.txt")
            if os.path.isfile(checkfile):
                n = conduit.Node()
                checkData = parseCheckpoint(checkfile)
                recKey = '/rec/' + recid
                print(checkData)
                (volume, clust, cx, cy, cz)=setCheckData(n, checkData, recKey)

                noAA=findNonAA()
                for idx, val in enumerate(noAA):
                    n[recKey + '/meta/NonStdAA/'+str(idx+1)] = val


                n[recKey + '/meta/RecPath'] = recPath

                pdbqtfile=recid+".pdbqt"
                if os.path.isfile(pdbqtfile):
                    print(pdbqtfile)
                    with open(pdbqtfile, 'r') as f:
                        n[recKey + "/file/rec_min.pdbqt"] = f.read()

                #if clust:
                #    gridPDBfile='Grid-'+str(clust).zfill(2)+".pdb"
                #    if os.path.isfile(gridPDBfile):
                #        with open(gridPDBfile, 'r') as f:
                #            n[recKey + "/file/Grid-"+str(clust)+".pdb"] = f.read()
                #else:
                #    if volume and cx and cy and cz:
                #        (found, clustID)=findCluster(volume, cx, cy, cz)
                #        if found:
                #            gridPDBfile = 'Grid-' + clustID + ".pdb"
                #            clust=int(clustID)
                #            n[recKey + '/meta/Site/Cluster'] = clust
                #            if os.path.isfile(gridPDBfile):
                #                with open(gridPDBfile, 'r') as f:
                #                    n[recKey + "/file/Grid-"+str(clust)+".pdb"] = f.read()
                if not clust:
                    (found, clustID) = findCluster(volume, cx, cy, cz)
                    if found:
                        clust = int(clustID)
                        n[recKey + '/meta/Site/Cluster'] = clust

                if (not os.path.exists('rec_min.pdb')) and os.path.isfile('Rec_min.pdb'):
                    os.rename('Rec_min.pdb', 'rec_min.pdb')

                fileList = ['rec.prmtop', 'rec_min.pdb', 'rec_min_orig','rec_min.rst',
                            'recCut.prmtop', 'recCut_min_orig.pdb', 'recCut_min.rst',
                            'rec_minGB.out', 'site.txt', 'rec_geo.txt']

                for gridfile in glob.glob('Grid-*.pdb'):
                    fileList.append(gridfile)

                filesToHDF(n, recKey, fileList)
                try:
                    conduit.relay.io.save_merged(n, hdf5path)
                except:
                    print(recid+ " cannot be saved in HDF5")


if __name__ == '__main__':
    main()