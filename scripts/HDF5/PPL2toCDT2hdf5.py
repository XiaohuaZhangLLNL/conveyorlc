import argparse
import os.path
import datetime
import conduit
import conduit.relay
import numpy as np

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--scrDir', action='store', dest='scrDir', default='scratch1',
                        help='ddcMD object output file (default=PPL3TrackNew.xml).')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='scratch1/ligand.hdf5',
                        help='ddcMD object output file (default=PPL3TrackNew.xml).')

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



def main():
    args=getArgs()
    print("Default inputs: ", args.scrDir, args.outfile)

    n = conduit.Node()
    n['date'] ="Created by PPL2toCDT2hdf5.py at " + datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S")

    print("Created by PPL2toCDT2hdf5.py at "+datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))

    hdf5path=os.path.abspath(args.outfile)
    print(hdf5path)
    ligDirPath = os.path.abspath(args.scrDir + "/lig")
    print(ligDirPath)
    os.chdir(ligDirPath)
    dirs = os.listdir(".")
    for cmpd in dirs:
        cmpdPath = os.path.join(ligDirPath, cmpd)
        print(cmpdPath)
        os.chdir(cmpdPath)
        print(os.getcwd())
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

        #exit(0)

    #conduit.relay.io.save(n, args.outfile)
    conduit.relay.io.save(n, hdf5path)


if __name__ == '__main__':
    main()