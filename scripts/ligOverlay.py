import argparse
import pybel
from pybel import ob
import math
import os.path


def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--list', action='store', dest='listfile', default='list',
                        help='list file to store protein names (default=list)')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='rmsd.csv',
                        help='ddcMD object output file (default=PPL3TrackNew.xml).')

    args = parser.parse_args()

    return args

def getAtmCoorList(mol):
    obmol = mol.OBMol

    atmCoorDict = {}
    for res in ob.OBResidueIter(obmol):
        for atom in ob.OBResidueAtomIter(res):
            atName = res.GetAtomID(atom).strip()
            if atName[0] != 'H':
                atmCoorDict[atName]=(atom.GetX(), atom.GetY(), atom.GetZ())

    return atmCoorDict

def molRMSD(atmCoorDict1, atmCoorDict2):
    num1 = len(atmCoorDict1)
    num2 = len(atmCoorDict2)
    #print num1, num2
    if num1 != num2:
        print ("number of atom doesn't match")
        return 9999.0

    if num1==0:
        print ("atom list 1 is empty")
        return 9999.0

    if num2==0:
        print ("atom list 2 is empty")
        return 9999.0

    sum=0.0
    for key, value1 in atmCoorDict1.items():
        if key in atmCoorDict2:
            value2=atmCoorDict2[key]
            for i in range(3):
                d=value1[i]-value2[i]
                sum=sum+d*d
        else:
            print (key)
            print ("atom name doesn't match")
            return 9999.0

    rmsd=math.sqrt(sum/num1)
    return rmsd

def calc_rmsd_complex(pdbfile, pdbqtfile):
    xmol=pybel.readfile('pdb', pdbfile).next()
    xrayAtmCoorDict = getAtmCoorList(xmol)

    rmsdList=[]
    dmols = pybel.readfile('pdbqt', pdbqtfile)
    for mol in dmols:
        dockAtmCoorDict = getAtmCoorList(mol)
        rmsd=molRMSD(xrayAtmCoorDict, dockAtmCoorDict)
        rmsdList.append(rmsd)

    if len(rmsdList)>0:
        return rmsdList.index(min(rmsdList))+1, min(rmsdList), rmsdList
    else:
        return -1, 9999.0, rmsdList

def main():
    args=getArgs()
    print("Default inputs: ", args.listfile, args.outfile)

    outFh=open(args.outfile, "w")
    outFh2=open('rmsd-all.txt', "w")

    outFh.write('pdb,pose_id,rmsd\n')

    with open(args.listfile, "r") as f:
        for line in f:
            pdb=line.strip()
            pdbfile='xray_ligand/'+pdb+'_lig.pdb'
            pdbqtfile='lig_pdbqt/'+pdb+'.pdbqt'
            if os.path.exists(pdbfile) and os.path.exists(pdbqtfile):
                print (pdb)
                id, rmsd, rmsdList = calc_rmsd_complex(pdbfile, pdbqtfile)
                outLine='{0}, {1}, {2}\n'.format(pdb, id, rmsd)
                outFh.write(outLine)
                outLine = '{0}, {1}\n'.format(pdb, rmsdList)
                outFh2.write(outLine)

if __name__ == '__main__':
    main()