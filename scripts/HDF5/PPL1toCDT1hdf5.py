import argparse
import os.path
import datetime
import conduit
import conduit.relay
import numpy as np

"""
Running the PPL2 conversion on LC

cd <your_running_directory_contains_scratch/lig>

/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL1toCDT1hdf5.py

Or if you provide inputs:

/usr/gapps/bbs/TOSS-3/spack/bin/cdtPython.sh PPL1toCDT1hdf5.py -d <path_to_scratch> -o <path_to_hdf5_file>

It assume that the scratch/lig has following data strcture

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


def main():
    args = getArgs()
    print("Default inputs: ", args.scrDir, args.outfile)

    n = conduit.Node()
    n['date'] = "Created by PPL1toCDT1hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S")

    print("Created by PPL1toCDT1hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))

    hdf5path = os.path.abspath(args.outfile)
    print(hdf5path)
    comDirPath = os.path.abspath(args.scrDir + "/com")
    print(comDirPath)
    os.chdir(comDirPath)
    dirs = os.listdir(".")
    for recid in dirs:
        recPath = os.path.join(comDirPath, recid+"/rec")
        # print(cmpdPath)
        os.chdir(recPath)
        print(os.getcwd())
        checkfile = os.path.join(recPath, "checkpoint.txt")
        if os.path.isfile(checkfile):
            parseCheckpoint(checkfile)
            checkData = parseCheckpoint(checkfile)
            cmpdKey = '/lig/' + cmpd
            print(checkData)
            if 'Mesg' in checkData:
                if checkData['Mesg'] == 'Finished!':
                    n[cmpdKey + '/status'] = np.int32(1)
                else:
                    n[cmpdKey + '/status'] = np.int32(0)
                n[cmpdKey + '/meta/Mesg'] = checkData['Mesg']
                n[cmpdKey + '/meta/Mesg'] = checkData['Mesg']

            if 'ligName' in checkData:
                n[cmpdKey + '/meta/name'] = checkData['ligName']

            if 'GBSA' in checkData:
                n[cmpdKey + '/meta/GBEN'] = float(checkData['GBSA'])

        n[cmpdKey + '/meta/LigPath'] = cmpdPath

        fileList = ['LIG.inpcrd', 'LIG.lib', 'LIG.prmtop', 'LIG_min.rst', 'LIG_minGB.out', 'ligand.frcmod',
                    'LIG_min.pdbqt']

        filesToHDF(n, cmpdKey, fileList)

    conduit.relay.io.save(n, hdf5path)


if __name__ == '__main__':
    main()