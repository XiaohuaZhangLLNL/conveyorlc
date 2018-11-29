__author__ = 'zhang30'

import argparse

"""
python findNoStdAA.py -i in.pdb -o out.pdb 

"""

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', action='store', dest='infile', default='in.pdb',
                        help='input pdb file (default=in.pdb)')
    parser.add_argument('-o', '--output', action='store', dest='outfile', default='out.pdb',
                        help='output pdb file (default=out.pdb).')


    args = parser.parse_args()

    return args

def main():

    args=getArgs()
    print ("Default inputs: ", args.infile, args.outfile)

    stdAA=["ALA", "ARG", "ASH", "ASN", "ASP", "CYM", "CYS", "CYX", "GLH", "GLN", "GLU", "GLY",
           "HIS", "HID", "HIE", "HIP", "HYP", "ILE", "LEU", "LYN", "LYS", "MET", "PHE", "PRO",
           "SER", "THR", "TRP", "TYR", "VAL",
           "ZN", "CU", "FE", "MG", "MN"]

    aaResDict={}

    with open(args.infile, "r") as f:
        for line in f:
            if line[:4]=="ATOM" or line[:6]=="HETATM":
                res=line[17:20].strip()
                aaResDict[res]=1

    resList=list(aaResDict.keys())

    nonStdAA=[]
    for res in resList:
        if res not in stdAA:
            nonStdAA.append(res)

    #print nonStdAA

    outFh=open(args.outfile, "w")

    with open(args.infile, "r") as f:
        for line in f:
            prtFlg=True
            res = line[17:20].strip()
            for resNon in nonStdAA:
                if res ==resNon:
                    prtFlg = False

            if prtFlg:
                outFh.write(line)

if __name__ == '__main__':
    main()
