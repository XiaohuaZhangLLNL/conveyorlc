import argparse
import os.path
import datetime
import conduit
import conduit.relay as relay
import conduit.relay.io
import h5py

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='infile', default='scratch/ligand.hdf5',
                        help='Ligand HDF5 input file')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='scratch/ligand_out.hdf5',
                        help='Ligand HDF5 output file')
    parser.add_argument('-d', '--del', action='store_true', dest='delete', default=False,
                        help='Ligand HDF5 output file')
    parser.add_argument('-l', '--ligid', action='store', dest='ligid', default=None,
                        help='ligand ID')
    parser.add_argument('-n', '--name', action='store', dest='ligname', default=None,
                        help='ligand name')
    parser.add_argument('-c', '--clist', action='store_true', dest='clist', default=False,
                        help='ligand name')
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


    fileList = ['LIG.inpcrd', 'LIG.lib', 'LIG.prmtop', 'LIG_min.rst', 'LIG_minGB.out', 'ligand.frcmod', 'LIG_min.pdbqt']
    for file in fileList:
        with open(file, 'w') as f:
            filedata=n[cmpdKey + "/file/" + file]
            if len(filedata)>0:
                f.write(filedata)
            else:
                print("Missing file "+file)

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

    n = conduit.Node()
    relay.io.load(n, hdf5path)

    itr = n['lig'].children()

    with open('idnamelist.txt', 'w') as f:
        for id in itr:
            name = n['lig/' + id.name() + "/meta/name"]
            f.write("{}  {}\n".format(name, id.name() ))


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

    #n_load = conduit.Node()
    #relay.io.load(n_load, hdf5path)


    #hdf5outpath = os.path.abspath(args.outfile)
    #relay.io.save_merged(n_load, hdf5outpath)

    #print("Created by PPL2toCDT2hdf5.py at "+datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))


if __name__ == '__main__':
    main()

