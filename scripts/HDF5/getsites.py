import argparse

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='infile', default='site.txt',
                        help='ddcmd output file as input')
    args = parser.parse_args()

    return args

def lineparse(line):
    strs = line.split('=')
    x = float(strs[1].split()[0])
    y = float(strs[2].split()[0])
    z = float(strs[3].rstrip())
    return (x, y, z)

def main():
    args=getArgs()
    print("Default inputs: ", args.infile)

    ### Temporary lists to store the lines related to grid information of each cluster
    clusts=[]
    clustLine = []
    prtFlag=False
    with open(args.infile, 'r') as f:
        for line in f:
            if line[:7]=='Cluster':
                prtFlag=True
            elif line[:2]=='==':
                prtFlag=False
                if len(clustLine)>0:
                    clusts.append(clustLine)
                clustLine = []
            if prtFlag:
                clustLine.append(line)

    ### Parse the grid information in each cluster
    clusters=[]
    for clustLine in clusts:
        strs=clustLine[1].split()
        id=strs[1]
        volume=float(strs[4])
        centroid = lineparse(clustLine[3])
        dimension = lineparse(clustLine[9])
        clustDict={'id':id, 'volume':volume, 'centroid':centroid, 'dimension':dimension}
        clusters.append(clustDict)

    ### Print out the grid information by cluster
    for clustDict in clusters:
        print(clustDict)


if __name__ == '__main__':
    main()