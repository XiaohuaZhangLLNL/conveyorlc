import argparse
import rdkit
from rdkit import Chem
from rdkit.Chem import rdMolDescriptors,Descriptors, rdchem
import os.path


def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--list', action='store', dest='listfile', default='list',
                        help='list file to store protein names (default=list)')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='rmsd.csv',
                        help='ddcMD object output file (default=PPL3TrackNew.xml).')

    args = parser.parse_args()

    return args

def calc_mol_prop(pdbfile):
    try:
        m=Chem.MolFromPDBFile(pdbfile)
        mw = Descriptors.ExactMolWt(m)
        numrbond=rdMolDescriptors.CalcNumRotatableBonds(m)
        numheavy=rdchem.Mol.GetNumHeavyAtoms(m)

    except:
        print("Error")
        return False, 0, 0, 0

    return True, mw, numrbond, numheavy


def main():
    args=getArgs()
    print("Default inputs: ", args.listfile, args.outfile)

    outFh=open(args.outfile, "w")

    outFh.write('pdb,wt,numRbond,numHeavy\n')

    with open(args.listfile, "r") as f:
        for line in f:
            pdb=line.strip()
            pdbfile='xray_ligand/'+pdb+'_lig.pdb'
            pdbqtfile='lig_pdbqt/'+pdb+'.pdbqt'
            if os.path.exists(pdbfile) and os.path.exists(pdbqtfile):
                print(pdb)
                success, mw, numrbond, numheavy=calc_mol_prop(pdbfile)
                if success:
                    outLine = '{0}, {1}, {2}, {3}\n'.format(pdb, mw, numrbond, numheavy)
                    outFh.write(outLine)

if __name__ == '__main__':
    main()