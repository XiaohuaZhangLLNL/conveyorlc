import argparse
import os.path
import datetime
import conduit
import conduit.relay as relay
import conduit.relay.io
import h5py

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='infile', default='scratch/receptor.hdf5',
                        help='receptor HDF5 input file')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='scratch/receptor_out.hdf5',
                        help='receptor HDF5 output file')
    parser.add_argument('-d', '--del', action='store_true', dest='delete', default=False,
                        help='receptor HDF5 output file')
    parser.add_argument('-n', '--name', action='store', dest='recname', default=None,
                        help='receptor name')
    parser.add_argument('-dn', '--deletename', action='store', dest='delname', default=None,
                        help='receptor name')
    parser.add_argument('-sn', '--savename', action='store', dest='savename', default=None,
                        help='receptor name')
    args = parser.parse_args()

    return args

def rmFailCalc(args):
    hdf5path = os.path.abspath(args.infile)

    with h5py.File(hdf5path, "a") as f:
        recGroup=f['/rec']
        for rec in recGroup:
            if recGroup[rec+'/status'][0] ==0:
                del recGroup[rec]

def getDataByName(args):

    hdf5path = os.path.abspath(args.infile)

    dirpath = os.path.dirname(hdf5path)
    ligpath = os.path.join(dirpath, 'rec/'+args.recname)

    if not os.path.exists(ligpath):
        os.makedirs(ligpath)

    os.chdir(ligpath)

    n = conduit.Node()
    relay.io.load(n, hdf5path)

    cmpdKey="rec/"+args.recname
    print("Receptor Name", args.recname)
    print("status", n[cmpdKey + '/status'])
    print("Mesg", n[cmpdKey + '/meta/Mesg'])
    print("GBSA", n[cmpdKey + '/meta/GBEN'])
    print("Receptor path", n[cmpdKey + '/meta/RecPath'] )
    print("Site     ")
    print("     Volume ",n[cmpdKey + '/meta/Site/Volume'], "     Cluster ", n[cmpdKey + '/meta/Site/Cluster'],)
    x = n[cmpdKey + '/meta/Site/Centriod/X']
    y = n[cmpdKey + '/meta/Site/Centriod/Y']
    z = n[cmpdKey + '/meta/Site/Centriod/Z']
    print("         Centriod  ", (x, y, z))
    x = n[cmpdKey + '/meta/Site/Dimension/X']
    y = n[cmpdKey + '/meta/Site/Dimension/Y']
    z = n[cmpdKey + '/meta/Site/Dimension/Z']
    print("         Dimension ", (x, y, z))
    print("=======================================================")

    itr=n["rec/"+args.recname+"/file"].children()

    for fileItr in itr:
        with open(fileItr.name(), 'w') as f:
            f.write(fileItr.node().value())

def rmCalcByName(args):
    hdf5path = os.path.abspath(args.infile)

    with h5py.File(hdf5path, "a") as f:
        recGroup=f['/rec']
        del recGroup[args.delname]

def saveDataByName(args):

    hdf5path = os.path.abspath(args.infile)
    hdf5pathOut=os.path.abspath(args.outfile)

    n = conduit.Node()
    relay.io.load(n, hdf5path)

    cmpdKey="rec/"+args.savename

    nOut=conduit.Node()

    nOut[cmpdKey]=n[cmpdKey]

    relay.io.save(nOut, hdf5pathOut)


def main():
    args=getArgs()
    print("Default inputs: ", args.infile, args.outfile)

    if args.delete:
        rmFailCalc(args)

    if args.recname:
        getDataByName(args)

    if args.delname:
        rmCalcByName(args)

    if args.savename:
        saveDataByName(args)

    #n_load = conduit.Node()
    #relay.io.load(n_load, hdf5path)


    #hdf5outpath = os.path.abspath(args.outfile)
    #relay.io.save_merged(n_load, hdf5outpath)

    #print("Created by PPL2toCDT2hdf5.py at "+datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))


if __name__ == '__main__':
    main()

