#!/usr/gapps/bbs/TOSS-3/anaconda3/bin/python

# This is a sample Python script.
import bz2
import json
import os
import argparse
import h5py

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', action='store', dest='infile', default=None,
                        help='HDF5 input file (default=None).')
    parser.add_argument('-d', '--dock_cutoff', action='store', dest='dock_cutoff', default=-7.5,
                        help='Cutoff of docking score for gbsa (default=-7.5).')
    parser.add_argument('-f', '--fusion_cutoff', action='store', dest='fusion_cutoff', default=7.5,
                        help='Cutoff of fusion score for gbsa (default=7.5).')
    parser.add_argument('-o', '--outdir', action='store', dest='outdir', default=None,
                        help='Output directory (default=None).')

    args = parser.parse_args()

    return args

def extract_data(args):
    dock_cutoff = float(args.dock_cutoff)
    fusion_cutoff = float(args.fusion_cutoff)
    writePDBQT = True
    if args.outdir == None:
        writePDBQT = False

    f = h5py.File(args.infile, 'r')

    # Map ligand_id to dataset idx.
    ligand_id_to_idx = dict((data_id, ii) for ii, data_id in enumerate(f['data_ids']))

    first_entry=True
    for ligand_id in ligand_id_to_idx:

        idx = ligand_id_to_idx[ligand_id]

        # Metadata.  receptor_id, ligand_id for header.
        try:
            bmetadata = f['metadata'][idx]
            metadata_json_str = bz2.decompress(bmetadata.tobytes()).decode()
            metadata = json.loads(metadata_json_str)

            receptor_id = metadata['receptor_id']
            ligand_id = metadata['ligand_id']

            # Pretty-print.
            #print(json.dumps(metadata, indent=4))
            dock_scoreList=metadata['docking_score']
            fusion_scoreList=metadata['fusion_score']
            fusion_max=max(fusion_scoreList)
            if dock_scoreList[0]< dock_cutoff or fusion_max> fusion_cutoff:
                #print(score_list)
                if first_entry:
                    #print("RECEPTER_ID: ", receptor_id)
                    recdir = args.outdir + "/" + receptor_id
                    os.makedirs(recdir, exist_ok=True)
                    hdf5filename = os.path.basename(args.infile)
                    #metafile = recdir+"/Meta-" + hdf5filename[:-4] + "txt"
                    #print(metafile)
                    #metafh=open(metafile, 'w')
                    first_entry=False
                metaline=receptor_id + "," + str(ligand_id) + "," + str(len(dock_scoreList))
                #metafh.write(metaline)
                print(metaline)
                if writePDBQT:
                    # pdbqt, if there.
                    bpdbqt = f['pdbqt'][idx]
                    pdbqt_str = bz2.decompress(bpdbqt.tobytes()).decode()
                    outdir=args.outdir+"/"+receptor_id+"/"+str(ligand_id)
                    os.makedirs(outdir, exist_ok=True)
                    outfile= outdir + "/poses.pdbqt"
                    with open(outfile, 'w') as outfh:
                        outfh.write(pdbqt_str[:-1]) #remove the last binary char in pdbqt_str
        except:
            pass


def main():
    args = getArgs()
    extract_data(args)

if __name__ == '__main__':
    main()


