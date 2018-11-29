from rdkit import Chem
from rdkit.Chem import AllChem
from rdkit.Chem import Draw
#from rdkit.Chem import PropertyMol
import argparse

"""
python findNoStdAA.py -i in.sdf -o out.sdf 

"""

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', action='store', dest='infile', default='in.sdf',
                        help='input sdf file (default=in.pdb)')
    parser.add_argument('-o', '--output', action='store', dest='outfile', default='out.sdf',
                        help='output sdf file (default=out.pdb).')


    args = parser.parse_args()

    return args

def main():

    args=getArgs()
    print ("Default inputs: ", args.infile, args.outfile)

    #open sdf file for dumping all the new oximes into
    w=Chem.SDWriter(args.outfile)

    suppl = Chem.SDMolSupplier(args.infile)

    ms = [x for x in suppl if x is not None]
    print("This is the number of entries read in")
    print(len(ms))

    for m in ms:
        #molName=m.GetProp("zinc_id")
        #m.SetProp("_Name",molName)
        #print(molName)

        ##check for nonsense
        test3=Chem.SanitizeMol(m)
    #    print(test3)
    #
        ##start generating 3 coordinates and optimize the conformation
        m3=Chem.AddHs(m)
        AllChem.EmbedMolecule(m3,useRandomCoords=True,enforceChirality=True,)
        AllChem.UFFOptimizeMolecule(m3,10000)

        #find molecular formal charge
        moleculeCharge=Chem.GetFormalCharge(m3)
        m3.SetProp('i_user_TOTAL_CHARGE',repr(moleculeCharge))
        w.write(m3)

if __name__ == '__main__':
    main()