from Bio import PDB
import numpy as np
import argparse
import time
import logging

# set up logger
"""
FORMAT = "%(asctime)s,%(msecs)d %(name)s %(filename)s:%(lineno)s %(funcName)s() - %(levelname)s - %(message)s"
logging.basicConfig(filename="cutProtein.log",
                    filemode='a',
                    format=FORMAT,
                    datefmt='%H:%M:%S',
                    level=logging.DEBUG)
"""

LOGLEVEL = 2
LOG_FMT = '%(asctime)s - %(name)s:%(funcName)s:%(lineno)s - %(levelname)s - %(message)s'
Log = logging.getLogger(__name__)
Log.setLevel(LOGLEVEL)

sh = logging.StreamHandler()
sh.setLevel(LOGLEVEL)
sh.setFormatter(logging.Formatter(LOG_FMT))
Log.addHandler(sh)

#Log = logging.getLogger(__name__)

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--rec', action='store', dest='recfile', default=None,
                        help='protein receptor pdb file')
    parser.add_argument('-l', '--lig', action='store', dest='ligfile', default=None,
                        help='ligand pdb file')
    parser.add_argument('-r', '--rad', action='store', dest='radius', default=None,
                        help='radius of cutoff')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='cut_',
                        help='output file as input')
    parser.add_argument('-s', '--slow', action='store_true', dest='slow',
                        help='Use fast by default')
    args = parser.parse_args()

    return args

class ResSelect(PDB.Select):
    def __init__(self, residueList):
        self.residueList=residueList
    def accept_residue(self, residue):
        if residue in self.residueList:
            return 1
        else:
            return 0

def cut_protein_slow(args):

    p = PDB.PDBParser()
    lig = p.get_structure('lig', args.ligfile)
    rec = p.get_structure('rec', args.recfile)

    d=float(args.radius)
    d2=d*d
    residueList=[]

    for residue in rec.get_residues():
        skipRes=False
        for atomI in residue:
            [xi, yi, zi]=atomI.get_coord()
            for atomJ in lig.get_atoms():
                [xj, yj, zj]=atomJ.get_coord()
                dx=xi-xj
                dy=yi-yj
                dz=zi-zj
                dist=dx*dx+dy*dy+dz*dz
                if dist<d2:
                    residueList.append(residue)
                    skipRes=True
                    break
            if skipRes:
                break

    Log.info("Number of residues: "+str(len(residueList)))

    io = PDB.PDBIO()
    io.set_structure(rec)
    io.save(args.outfile+args.radius+'.pdb', select=ResSelect(residueList))

def cut_protein_fast(args):
    """
    Parameters
    ----------
    rec : PDB structure
       the PDB structure for the receptor
    lig_atoms_coors : Nx3 ndarray
       atom coords for the ligand pose
    radius : float or list(float)
       the radius of the cutoff
    outfile : filename or file-like object
       location to write the selected residues PDB file
    radius : float
       cut radius of protein
    """


    p = PDB.PDBParser()
    lig = p.get_structure('lig', args.ligfile)
    rec = p.get_structure('rec', args.recfile)

    #pose_atoms= lig.get_atoms()
    lig_atoms_coors = np.array([a.get_coord() for a in lig.get_atoms()])

    d = float(args.radius)
    d2 = d * d
    residue_list = []
    residue_ids = []

    for residue_idx, residue in enumerate(rec.get_residues()):
        for atomI in residue:
            [xi, yi, zi] = atomI.get_coord()
            diff = [xi, yi, zi] - lig_atoms_coors
            dist = (diff**2).sum(axis=1)
            if np.any(dist < d2):
                residue_list.append(residue)
                residue_ids.append((residue.get_resname(), residue.get_id()[1]))
                break

    n_residues = len(residue_list)
    Log.info("Number of residues: "+str(n_residues))
    if not n_residues:
        Log.warning("Warning: not residues in cut protein")
        return

    io = PDB.PDBIO()
    io.set_structure(rec)
    io.save(args.outfile+args.radius+'.pdb', select=ResSelect(residue_list))

def main():
    args=getArgs()

    Log.info("Input/Ouput: "+ ", ".join([args.recfile, args.ligfile, args.radius, args.outfile]))
    start = time.time()
    if args.slow:
        cut_protein_slow(args)
    else:
        cut_protein_fast(args)
    end = time.time()
    Log.info("Cut protein takes: " + str(end - start))

if __name__ == '__main__':
    main()
