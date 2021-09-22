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

    fileName=args.infile
    filebase=fileName[:-4]

    drug_df = pd.read_csv(fileName, skipinitialspace=True, error_bad_lines=False,
                          dtype={"comGB": float, "dockScore": float, "ligName": object})
    durg_sort_df = drug_df.sort_values(by='ligName', ascending=True)

    bindGB_min_s = durg_sort_df.groupby(['ligName'], sort=True)['bindGB'].min()
    bindGB_min_df = bindGB_min_s.to_frame()

    ml_2_df = bindGB_min_df.sort_values(by='bindGB', ascending=True)
    file1 = filebase + "_MMGBSA.csv"
    ml_2_df.head(32).to_csv(file1)

    dock_min_s = durg_sort_df.groupby(['ligName'], sort=True)['dockScore'].min()
    dock_min_df = dock_min_s.to_frame()

    ml_3_df = dock_min_df.sort_values(by='dockScore', ascending=True)
    file1 = filebase + "_VINA.csv"
    ml_3_df.head(30).to_csv(file1)

if __name__ == '__main__':
    main()