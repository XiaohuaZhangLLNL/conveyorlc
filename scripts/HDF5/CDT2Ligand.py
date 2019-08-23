import argparse
import os.path
import datetime
import conduit
import conduit.relay as relay
import conduit.relay.io
import h5py
import errno

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='infile', default='scratch/ligand.hdf5',
                        help='Ligand HDF5 input file')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='scratch/ligand_out.hdf5',
                        help='Ligand HDF5 output file')
    parser.add_argument('-d', '--del', action='store_true', dest='delete', default=False,
                        help='delete all paths for failed calculations')
    parser.add_argument('-l', '--ligid', action='store', dest='ligid', default=None,
                        help='extract data and files by ligand ID')
    parser.add_argument('-n', '--name', action='store', dest='ligname', default=None,
                        help='extract data and files by ligand name')
    parser.add_argument('-c', '--clist', nargs=2, action='store', dest='clist', default=None,
                        help='output ligand name and id map into a file and choose only successful one or not (e.g. idnameList.txt T[A])')
    parser.add_argument('-e', '--extract', action='store', dest='extract', default=None,
                        help='extract all data from HDF5 file to CSV file')
    args = parser.parse_args()

    return args

def rmFailCalc(args):
    hdf5path = os.path.abspath(args.infile)

    with h5py.File(hdf5path, "a") as f:
        ligGroup=f['/lig']
        for lig in ligGroup:
            if ligGroup[lig+'/status'][0] ==0:
                del ligGroup[lig]

def getDataByligID(args, id=None):
    if not id:
        id=args.ligid

    hdf5path = os.path.abspath(args.infile)

    dirpath = os.path.dirname(hdf5path)
    ligpath = os.path.join(dirpath, 'lig/'+id)

    if not os.path.exists(ligpath):
        os.makedirs(ligpath)

    os.chdir(ligpath)

    n = conduit.Node()
    relay.io.load(n, hdf5path)

    cmpdKey="lig/"+id
    print("Ligand ID", id)
    print("Ligand Name", n[cmpdKey + '/meta/name'])
    print("status", n[cmpdKey + '/status'])
    print("Mesg", n[cmpdKey + '/meta/Mesg'])
    print("GBSA", n[cmpdKey + '/meta/GBEN'])
    print("Ligand Old path", n[cmpdKey + '/meta/LigPath'] )
    print("Ligand data path", os.getcwd())


    #fileList = ['LIG.inpcrd', 'LIG.lib', 'LIG.prmtop', 'LIG_min.rst', 'LIG_minGB.out', 'ligand.frcmod', 'LIG_min.pdbqt']
    itr = n[cmpdKey +"/file"].children()

    for fileItr in itr:
        with open(fileItr.name(), 'w') as f:
            f.write(fileItr.node().value())

def getDataByligName(args):
    hdf5path = os.path.abspath(args.infile)

    n = conduit.Node()
    relay.io.load(n, hdf5path)

    itr = n['lig'].children()

    for id in itr:
        name=n['lig/'+id.name()+"/meta/name"]
        print(id.name(), id.index(), name)
        if name == args.ligname:
            getDataByligID(args, id.name())
            break
    else:
        print("Cannot find ligand name " + args.ligname)


def getIdNameList(args):

    hdf5path = os.path.abspath(args.infile)

    filename=args.clist[0]
    flag=(args.clist[1]=='T')

    n = conduit.Node()
    relay.io.load(n, hdf5path)

    itr = n['lig'].children()


    with open(filename, 'w') as f:
        if flag:
            for id in itr:
                status= n['lig/' + id.name() + "/status"]
                if status == 1:
                    name = n['lig/' + id.name() + "/meta/name"]
                    f.write("{}  {}\n".format(name, id.name() ))
        else:
            for id in itr:
                name = n['lig/' + id.name() + "/meta/name"]
                f.write("{}  {}\n".format(name, id.name() ))

def extractAll(args):

    hdf5path = os.path.abspath(args.infile)
    dirpath = os.path.dirname(hdf5path)

    n = conduit.Node()
    conduit.relay.io.load(n, hdf5path)
    itrLig = n["lig"].children()
    for lig in itrLig:
        id=lig.name()
        ligpath = os.path.join(dirpath, 'lig/' + id)
        if not os.path.exists(ligpath):
            os.makedirs(ligpath)
        os.chdir(ligpath)

        cmpdKey = "lig/" + id
        print("Ligand ID", id)
        print("Ligand Name", n[cmpdKey + '/meta/name'])
        print("status", n[cmpdKey + '/status'])
        print("Mesg", n[cmpdKey + '/meta/Mesg'])
        print("GBSA", n[cmpdKey + '/meta/GBEN'])
        print("Ligand Old path", n[cmpdKey + '/meta/LigPath'])
        print("Ligand data path", os.getcwd())

        # fileList = ['LIG.inpcrd', 'LIG.lib', 'LIG.prmtop', 'LIG_min.rst', 'LIG_minGB.out', 'ligand.frcmod', 'LIG_min.pdbqt']
        itr = n[cmpdKey + "/file"].children()

        for fileItr in itr:
            with open(fileItr.name(), 'w') as f:
                f.write(fileItr.node().value())


def main():
    args=getArgs()
    print("Default inputs: ", args.infile, args.outfile)

    if args.delete:
        rmFailCalc(args)

    if args.ligid:
        getDataByligID(args)

    if args.ligname:
        getDataByligName(args)

    if args.clist:
        getIdNameList(args)

    if args.extract:
        extractAll(args)
    #n_load = conduit.Node()
    #relay.io.load(n_load, hdf5path)


    #hdf5outpath = os.path.abspath(args.outfile)
    #relay.io.save_merged(n_load, hdf5outpath)

    #print("Created by PPL2toCDT2hdf5.py at "+datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))


if __name__ == '__main__':
    main()

