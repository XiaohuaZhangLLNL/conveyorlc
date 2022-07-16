import argparse
import os.path
import datetime
import h5py
import glob

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='indir', default='scratch/gbsaHDF5',
                        help='receptor HDF5 input file')
    parser.add_argument('-o', '--out', action='store', dest='outdir', default='scratch/gbsaHDF5out',
                        help='receptor HDF5 output file')
    parser.add_argument('-d', '--del', action='store_true', dest='delete', default=True,
                        help='delete all paths for failed calculations')
    args = parser.parse_args()

    return args

def numpyIsNumber(obj):
    attrs = ['__add__', '__sub__', '__mul__', '__pow__']
    return all(hasattr(obj, attr) for attr in attrs)

def rmFailCalc(args):

    dockDir = os.path.abspath(args.indir)

    if os.path.isdir(dockDir):
        os.chdir(dockDir)

        print("Remove pose path if the pose calculation is fail")
        for h5f in glob.glob('gbsa_proc*.hdf5'):
            with h5py.File(h5f, "a") as f:
                print("     Working on "+h5f)
                recGroup = f['/gbsa']
                for rec in recGroup:
                    ligGroup=recGroup[rec]
                    for lig in ligGroup:
                        poseGroup=ligGroup[lig]
                        for pose in poseGroup:
                            pnode=poseGroup[pose]
                            meta = pnode['meta']
                            comGB = meta['comGB']
                            if comGB[0] == 0.0:
                                print("delete ", rec, lig, pose)
                                del poseGroup[pose]

        print("Remove ligand path if it is empty")
        for h5f in glob.glob('gbsa_proc*.hdf5'):
            with h5py.File(h5f, "a") as f:
                print("     Working on " + h5f)
                recGroup = f['/gbsa']
                for rec in recGroup:
                    ligGroup=recGroup[rec]
                    for lig in ligGroup:
                        poseGroup=ligGroup[lig]
                        if len(poseGroup) ==0:
                            del ligGroup[lig]

        print("Remove recepotr path if it is empty")
        for h5f in glob.glob('gbsa_proc*.hdf5'):
            with h5py.File(h5f, "a") as f:
                print("     Working on " + h5f)
                recGroup = f['/gbsa']
                for rec in recGroup:
                    ligGroup=recGroup[rec]
                    if len(ligGroup) == 0:
                        del recGroup[rec]

        print("Remove the HDF5 file if it is empty")
        for h5f in glob.glob('gbsa_proc*.hdf5'):
            with h5py.File(h5f, "a") as f:
                print("     Working on " + h5f)
                recGroup = f['/gbsa']
                if len(recGroup)==0:
                    os.remove(h5f)
                    print("     remove HDF5 file - "+h5f)


def main():
    args=getArgs()
    print("Default inputs: ", args.indir, args.outdir)

    if args.delete:
        rmFailCalc(args)

if __name__ == '__main__':
    main()

