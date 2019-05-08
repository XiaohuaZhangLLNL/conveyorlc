import argparse
import os.path
import datetime
import conduit
import conduit.relay as relay
import conduit.relay.io
import h5py
import glob

from mpi4py import MPI

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='indir', default='scratch/dockHDF5',
                        help='receptor HDF5 input file')
    parser.add_argument('-o', '--out', action='store', dest='outdir', default='scratch/dockHDF5out',
                        help='receptor HDF5 output file')
    parser.add_argument('-r', '--recname', action='store', dest='recname', default=None,
                        help='receptor name for extracting data and files (also need ligand id)')
    parser.add_argument('-l', '--ligid', action='store', dest='ligid', default=None,
                        help='ligand id for extracting data and files (also need receptor name)')
    parser.add_argument('-p', '--percent', action='store', dest='percent', default=None,
                        help='select top percent ligands into the HDF5 output file')
    parser.add_argument('-s', '--scoreonly', action='store', dest='scoreonly', default=None,
                        help='extract score-only data from HDF5 file to CSV file')
    args = parser.parse_args()

    return args

def getDataByName(args):

    dirpath=os.path.abspath("scratch")
    dockDir = os.path.abspath(args.indir)
    #dockOutDir = os.path.abspath(args.outdir)

    entryKey="dock/"+args.recname+"/"+args.ligid

    if os.path.isdir(dockDir):
        os.chdir(dockDir)

        for h5f in glob.glob('dock_proc*.hdf5'):
            n = conduit.Node()
            relay.io.load(n, h5f)
            if n.has_path(entryKey):
                ndata=n[entryKey]
                print("=======================================================")
                print("entry key: ", entryKey)
                print("data has been written to scratch/"+entryKey)
                print(ndata["status"])
                print(ndata["meta/Mesg"])
                print(ndata["meta/ligName"])
                numPose=ndata["meta/numPose"]
                print("number of poses ", numPose)
                for pose in range(numPose):
                    print("Pose "+str(pose+1), ndata["meta/scores/"+str(pose+1)])
                print("=======================================================")

                datapath = os.path.join(dirpath, 'dock/' + args.recname+"/"+args.ligid)

                if not os.path.exists(datapath):
                    os.makedirs(datapath)

                os.chdir(datapath)

                itr=ndata["/file"].children()

                for fileItr in itr:
                    with open(fileItr.name(), 'w') as f:
                        f.write(fileItr.node().value())

def getTopPercent(args):

    dockDir = os.path.abspath(args.indir)
    dockOutDir = os.path.abspath(args.outdir)

    if os.path.isdir(dockDir):
        os.chdir(dockDir)

        scores=[]

        for h5f in glob.glob('dock_proc*.hdf5'):
            n = conduit.Node()
            relay.io.load(n, h5f)

            itr = n["dock/"+args.recname].children()
            for lig in itr:
                ligid=lig.name()
                scorePath="dock/"+ args.recname+"/"+ligid+"/meta/scores/1"
                if n.has_path(scorePath):
                    scores.append(n[scorePath])

        scores_sorted=sorted(scores)
        numData=len(scores_sorted)
        topIndex=int(numData*float(args.percent))

        if not os.path.exists(dockOutDir):
            os.makedirs(dockOutDir)

        score_critic=scores_sorted[topIndex]

        for h5f in glob.glob('dock_proc*.hdf5'):
            h5fout=os.path.join(dockOutDir, h5f)
            n = conduit.Node()
            relay.io.load(n, h5f)

            itr = n["dock/" + args.recname].children()
            for lig in itr:
                ligid = lig.name()
                scorePath = "dock/" + args.recname + "/" + ligid + "/meta/scores/1"
                if n.has_path(scorePath):
                    if n[scorePath]<score_critic:
                        ndata=n["dock/" + args.recname + "/" + ligid]
                        relay.io.save(ndata, h5fout)

def extractScore_MPI(args):
    comm = MPI.COMM_WORLD
    rank = comm.rank
    size = comm.size

    hdf5pathDir = os.path.abspath(args.outfile)

    os.chdir(hdf5pathDir)
    h5files=glob.glob('dock_proc*.hdf5')
    h5KeyList=[]
    for i in xrange(rank, len(h5files), size):
        h5f=h5files[i]
        n = conduit.Node()
        conduit.relay.io.load(n, h5f)
        itrRec = n["dock"].children()
        for rec in itrRec:
            recid=rec.name()
            nrec=rec.node()
            itrLig=nrec.children()
            for lig in itrLig:
                ligid = lig.name()

    allh5KeyList = comm.gather(h5KeyList, root=0)
    finishKeyList=[]

def extractScoreOnly(args):
    outfh = open(args.scoreonly, "w")
    outfh.write("rec, lig, Scoreonly\n")

    dirpath = os.path.abspath("scratch")
    hdf5pathDir = os.path.abspath(args.indir)

    os.chdir(hdf5pathDir)
    h5files=glob.glob('dock_proc*.hdf5')
    h5KeyList=[]

    for h5f in h5files:
        n = conduit.Node()
        conduit.relay.io.load(n, h5f)
        itrRec = n["dock"].children()
        for rec in itrRec:
            recid=rec.name()
            nrec=rec.node()
            itrLig=nrec.children()
            for lig in itrLig:
                ligid = lig.name()
                entryKey = "dock/" + recid + "/" + ligid
                ndata = n[entryKey]
                if ndata.has_path("file/scores.log"):
                    scoreslog=ndata["file/scores.log"]
                    strs=scoreslog.split()
                    if strs[0]=="Affinity:":
                        outfh.write(recid+", "+ligid+", "+strs[1]+"\n")


def main():
    args=getArgs()
    print("Default inputs: ", args.indir, args.outdir)

    if args.recname and args.ligid:
        getDataByName(args)

    if args.percent and args.recname :
        getTopPercent(args)

    if args.scoreonly:
        extractScoreOnly(args)

if __name__ == '__main__':
    main()

