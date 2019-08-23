import argparse
import os.path


def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='sdfFile', default=None,
                        help='ligand SDF file input')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='skip.list',
                        help='output ligand id list')
    parser.add_argument('-c', '--cutoff', action='store', dest='cutoff', default=300,
                        help='max number of ligand atoms')
    args = parser.parse_args()

    return args


def main():
    args=getArgs()
    print("Default inputs: ", args.sdfFile, args.outfile, args.cutoff)

    cutoff=int(args.cutoff)
    outFh=open(args.outfile, "w")

    count=1
    contents=[]
    with open(args.sdfFile, "r") as f:
        for line in f:
            contents.append(line)
            if line[:4]=="$$$$":
                if len(contents)>4:
                    info=contents[3]
                    num=int(info[:3])
                    if num > cutoff:
                        outFh.write(str(count)+"\n")
                count=count+1
                contents=[]



if __name__ == '__main__':
    main()