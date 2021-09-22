import numpy as np
import pandas as pd
import argparse

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='infile', default=None,
                        help='ddcmd output file as input')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='out.csv',
                        help='ddcmd output file as input')
    args = parser.parse_args()

    return args

def main():
    args=getArgs()
    print("Input/Ouput: ", args.infile, args.outfile)

    ml_df=pd.read_csv(args.infile, skipinitialspace=True)
    ml_df.head()

    ml_1_df=ml_df.sort_values(by='Late-Fusion', ascending=False)
    ml_1_df['fusion_rank'] = np.arange(1,len(ml_1_df)+1)
    ml_2_df=ml_1_df.sort_values(by='MMGBSA', ascending=True)
    ml_2_df['gbsa_rank'] = np.arange(1,len(ml_2_df)+1)
    ml_3_df=ml_2_df.sort_values(by='Vina', ascending=True)
    ml_3_df['vina_rank'] = np.arange(1,len(ml_3_df)+1)
    ml_3_df['invert'] = 1.0/ml_3_df.fusion_rank + 1.2/ml_3_df.gbsa_rank + 0.8/ml_3_df.vina_rank
    ml_4_df=ml_3_df.sort_values(by='invert', ascending=False)
    ml_4_df['invert_rank'] = np.arange(1,len(ml_4_df)+1)

    ml_4_df.to_csv(args.outfile)

if __name__ == '__main__':
    main()