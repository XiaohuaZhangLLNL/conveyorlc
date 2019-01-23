import argparse
import os.path
import datetime
import conduit
import conduit.relay as relay
import conduit.relay.io
#import h5py
import glob

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='indir', default='scratch/gbsaHDF5',
                        help='receptor HDF5 input file')
    parser.add_argument('-o', '--out', action='store', dest='outdir', default='scratch/gbsaHDF5out',
                        help='receptor HDF5 output file')
    parser.add_argument('-r', '--recname', action='store', dest='recname', default=None,
                        help='receptor name')
    parser.add_argument('-l', '--ligid', action='store', dest='ligid', default=None,
                        help='ligand id')
    parser.add_argument('-m', '--meta', action='store', dest='meta', default=None,
                        help='extract meta data from HDF5 file to CSV file')
    args = parser.parse_args()

    return args

def getDataByName(args):

    dirpath=os.path.abspath("scratch")
    dockDir = os.path.abspath(args.indir)
    #dockOutDir = os.path.abspath(args.outdir)

    entryKey="gbsa/"+args.recname+"/"+args.ligid
    print("entry key: ", entryKey)
    if os.path.isdir(dockDir):
        os.chdir(dockDir)

        for h5f in glob.glob('gbsa_proc*.hdf5'):
            os.chdir(dockDir)
            n = conduit.Node()
            relay.io.load(n, h5f)
            if n.has_path(entryKey):
                ndata=n[entryKey]
                print(h5f)
                poseItr=ndata.children()

                for pose in poseItr:
                    poseData=pose.node()
                    print("=======================================================")

                    print("data has been written to scratch/"+entryKey+"/"+pose.name())
                    print("status", poseData["status"])
                    print("Mesg", poseData["meta/Mesg"])
                    print("bindGB", poseData["meta/bindGB"])
                    print("comGB", poseData["meta/comGB"])
                    print("recGB", poseData["meta/recGB"])
                    print("ligGB", poseData["meta/ligGB"])
                    print("dockScore", poseData["meta/dockScore"])

                    datapath = os.path.join(dirpath, 'gbsa/' + args.recname+"/"+args.ligid+"/"+pose.name())

                    if not os.path.exists(datapath):
                        os.makedirs(datapath)

                    os.chdir(datapath)

                    itr=poseData["/file"].children()

                    for fileItr in itr:
                        with open(fileItr.name(), 'w') as f:

                            f.write(fileItr.node().value())

def numpyIsNumber(obj):
    attrs = ['__add__', '__sub__', '__mul__', '__pow__']
    return all(hasattr(obj, attr) for attr in attrs)

def getMetaCSV(args):

    #keys = {"status", "meta/Mesg", "meta/bindGB", "meta/comGB", "meta/recGB", "meta/ligGB", "meta/dockScore"}
    keys = {"status", "meta/bindGB", "meta/comGB", "meta/recGB", "meta/ligGB", "meta/dockScore"}
    dockDir = os.path.abspath(args.indir)

    outfh=open(args.meta, "w")

    metadata={}

    if os.path.isdir(dockDir):
        os.chdir(dockDir)

        for h5f in glob.glob('gbsa_proc*.hdf5'):
            n = conduit.Node()
            relay.io.load(n, h5f)
            recItr=n['gbsa'].children()
            for rec in recItr:
                nrec=rec.node()
                recKey=rec.name()

                if recKey not in metadata:
                    metadata[recKey]={}

                ligItr=nrec.children()

                for lig in ligItr:
                    nlig=lig.node()
                    ligKey=lig.name()
                    recdata=metadata[recKey]

                    if ligKey not in recdata:
                        recdata[ligKey]={}

                    poseItr=nlig.children()

                    for pose in poseItr:
                        npose=pose.node()
                        poseKey=pose.name()
                        ligdata=recdata[ligKey]

                        if poseKey not in ligdata:
                            ligdata[poseKey]={}

                        posedata=ligdata[poseKey]
                        print(recKey, ligKey, poseKey)

                        for key in keys:
                            val=npose[key]
                            if numpyIsNumber(val):
                                posedata[key]=val
                            else:
                                posedata[key] = None

    print(metadata)
    outfh.write("rec, lig, pose, status, bindGB, comGB, recGB, ligGB, dockScore\n")
    for recKey in metadata:
        recdata=metadata[recKey]
        for ligKey in recdata:
            ligdata=recdata[ligKey]
            for poseKey in ligdata:
                posedata=ligdata[poseKey]
                line=recKey+', '+ligKey+', '+poseKey
                for key in keys:
                    line=line+', '+str(posedata[key])
                outfh.write(line+"\n")



def main():
    args=getArgs()
    print("Default inputs: ", args.indir, args.outdir)

    if args.recname and args.ligid:
        getDataByName(args)

    if args.meta:
        getMetaCSV(args)

if __name__ == '__main__':
    main()

