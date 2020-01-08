import pandas as pd
from rdkit.Chem import PandasTools
from rdkit import Chem
from rdkit.Chem import AllChem
import argparse

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='infile', default='dskey_aurk_a_class.csv',
                        help='input SMILES file')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='dskey_aurk_a_class.sdf',
                        help='output SDF file')
    args = parser.parse_args()

    return args

def main():
    args=getArgs()
    print(args.infile, args.outfile)
    smiles_df=pd.read_csv(args.infile)
    pp = smiles_df[['rdkit_smiles','compound_id']]

    PandasTools.AddMoleculeColumnToFrame(pp,'rdkit_smiles','Molecule')

    for index, row in pp.iterrows():
        row['Molecule']=Chem.AddHs(row['Molecule'])
        AllChem.EmbedMolecule(row['Molecule'])
        pp.at[index,'i_user_TOTAL_CHARGE'] = Chem.rdmolops.GetFormalCharge(row['Molecule'])


    PandasTools.WriteSDF(pp, args.outfile, molColName='Molecule', idName='compound_id', properties=list(pp.columns))

if __name__ == '__main__':
    main()
