import argparse
import os.path
import datetime
import conduit
import conduit.relay as relay
import conduit.relay.io
import h5py
import numpy as np

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in', action='store', dest='infile', default='scratch/receptor.hdf5',
                        help='receptor HDF5 input file')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='scratch/receptor_out.hdf5',
                        help='receptor HDF5 output file')
    parser.add_argument('-d', '--del', action='store_true', dest='delete', default=False,
                        help='delete all paths for failed calculations')
    parser.add_argument('-n', '--name', action='store', dest='recname', default=None,
                        help='extract meta data and files by receptor name')
    parser.add_argument('-dn', '--deletename', action='store', dest='delname', default=None,
                        help='delete path by receptor name')
    parser.add_argument('-sn', '--savename', action='store', dest='savename', default=None,
                        help='save data to HDF5 output file by receptor name')
    parser.add_argument('-c', '--checkdata', nargs=2, action='store', dest='checkdata', default=None,
                        help='update meta data by protein name and checkpoint file name (e.g. sarinXtalnAChE  checkpoint.txt)')
    parser.add_argument('-m', '--meta', action='store', dest='meta', default=None,
                        help='extract meta data from HDF5 file to CSV file')
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
    x = n[cmpdKey + '/meta/Site/Centroid/X']
    y = n[cmpdKey + '/meta/Site/Centroid/Y']
    z = n[cmpdKey + '/meta/Site/Centroid/Z']
    print("         Centroid  ", (x, y, z))
    x = n[cmpdKey + '/meta/Site/Dimension/X']
    y = n[cmpdKey + '/meta/Site/Dimension/Y']
    z = n[cmpdKey + '/meta/Site/Dimension/Z']
    print("         Dimension ", (x, y, z))
    print("=======================================================")

    itr=n["rec/"+args.recname+"/file"].children()

    for fileItr in itr:
        with open(fileItr.name(), 'w') as f:
            f.write(fileItr.node().value())

def getMetaData(args):
    hdf5path = os.path.abspath(args.infile)

    n = conduit.Node()
    relay.io.load(n, hdf5path)
    recItr = n['rec'].children()

    outfh = open(args.meta, "w")
    outfh.write("Rec_Name, Status, GBSA, Volume, Cent_x, Cent_y, Cent_z, Dim_x, Dim_y, Dim_Z\n")

    for rec in recItr:
        nrec = rec.node()
        recName=rec.name()
        status=nrec['status']
        gbsa=nrec['meta/GBEN']
        vol = nrec['meta/Site/Volume']
        cent_x = nrec['meta/Site/Centroid/X']
        cent_y = nrec['meta/Site/Centroid/Y']
        cent_z = nrec['meta/Site/Centroid/Z']
        dim_x = nrec['meta/Site/Dimension/X']
        dim_y = nrec['meta/Site/Dimension/Y']
        dim_z = nrec['meta/Site/Dimension/Z']
        outfh.write("{}, {}, {}, {}, {}, {}, {}, {}, {}, {}\n".format(recName, status, gbsa, vol, cent_x, cent_y, cent_z, dim_x, dim_y,dim_z))

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

def parseCheckpoint(checkfile):
    checkData = {}
    with open(checkfile, "r") as f:
        for line in f:
            strs = line.strip().split(':')
            if len(strs) == 2:
                checkData[strs[0]] = strs[1]

    return checkData

def setCheckData(n, checkData, recKey):
    if 'Mesg' in checkData:
        if checkData['Mesg'] == 'Finished!':
            n[recKey + '/status'] = np.int32(1)
        else:
            n[recKey + '/status'] = np.int32(0)
        n[recKey + '/meta/Mesg'] = checkData['Mesg']

    if 'GBEN' in checkData:
        n[recKey + '/meta/GBEN'] = float(checkData['GBEN'])

    if 'Volume' in checkData:
        volume = float(checkData['Volume'])
        n[recKey + '/meta/Site/Volume'] = volume

    if 'Cluster' in checkData:
        clust = int(checkData['Cluster'])
        n[recKey + '/meta/Site/Cluster'] = clust

    if 'cx' in checkData:
        cx = float(checkData['cx'])
        n[recKey + '/meta/Site/Centroid/X'] = cx
    if 'cy' in checkData:
        cy = float(checkData['cy'])
        n[recKey + '/meta/Site/Centroid/Y'] = cy
    if 'cz' in checkData:
        cz = float(checkData['cz'])
        n[recKey + '/meta/Site/Centroid/Z'] = cz
    if 'dx' in checkData:
        n[recKey + '/meta/Site/Dimension/X'] = float(checkData['dx'])
    if 'dy' in checkData:
        n[recKey + '/meta/Site/Dimension/Y'] = float(checkData['dy'])
    if 'dz' in checkData:
        n[recKey + '/meta/Site/Dimension/Z'] = float(checkData['dz'])

    return (volume, clust, cx, cy, cz)


def updateCheckData(args):
    hdf5path = os.path.abspath(args.outfile)
    print(hdf5path)
    print(args.checkdata)

    if os.path.isfile(args.checkdata[1]):
        n = conduit.Node()
        checkData = parseCheckpoint(args.checkdata[1])
        recKey = '/rec/' + args.checkdata[0]
        print(checkData)
        (volume, clust, cx, cy, cz) = setCheckData(n, checkData, recKey)

        conduit.relay.io.save_merged(n, hdf5path)
    else:
        print("File - "+args.checkdata[1]+" doesn't exist")

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

    # just update checkpoint.txt file for one protein
    if args.checkdata:
        updateCheckData(args)

    if args.meta:
        getMetaData(args)

    #n_load = conduit.Node()
    #relay.io.load(n_load, hdf5path)


    #hdf5outpath = os.path.abspath(args.outfile)
    #relay.io.save_merged(n_load, hdf5outpath)

    #print("Created by PPL2toCDT2hdf5.py at "+datetime.datetime.now().strftime("%m-%d-%Y %H:%M:%S"))


if __name__ == '__main__':
    main()

