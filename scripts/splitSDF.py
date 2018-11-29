__author__ = 'zhang30'

import argparse
import os

"""
python findNoStdAA.py -i in.pdb -o out.pdb 

"""

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', action='store', dest='infile', default='in.sdf',
                        help='input SDF file (default=in.sdf)')
    parser.add_argument('-o', '--output', action='store', dest='outfile', default='out',
                        help='output directory (default=out.pdb).')


    args = parser.parse_args()

    return args

def main():

    args=getArgs()
    print ("Default inputs: ", args.infile, args.outfile)

    os.makedirs(args.outfile, exist_ok=True)

    count=0
    ncount=1
    with open(args.infile, "r") as f:
        for line in f:
            if count==0:
                fileName=line.strip()
                print(fileName)

                if fileName=="":
                    fileName="NoName"+str(ncount)
                    ncount=ncount+1

                fileName = args.outfile + "/" + fileName+".sdf"
                fh = open(fileName, "w")

            if line.strip()=='>  <FCharge>':
                line='> <i_user_TOTAL_CHARGE>\n'

            fh.write(line)
            count = count + 1
            if line.strip()=='$$$$':
                count=0



if __name__ == '__main__':
    main()


