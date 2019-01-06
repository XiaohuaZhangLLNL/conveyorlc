import argparse

import os.path


def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--list', action='store', dest='listfile', default='list',
                        help='list file to store protein names (default=list)')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='rmsd.csv',
                        help='ddcMD object output file (default=PPL3TrackNew.xml).')

    args = parser.parse_args()

    return args


def main():
    args=getArgs()
    print("Default inputs: ", args.listfile, args.outfile)

    outFh=open(args.outfile, "w")

    outFh.write('pdb,wt,numRbond,numHeavy\n')

    with open(args.listfile, "r") as f:
        for line in f:
            pdb=line.strip()


if __name__ == '__main__':
    main()