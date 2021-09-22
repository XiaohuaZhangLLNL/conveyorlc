from Bio import PDB
import argparse

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

def main():
    args=getArgs()
    print("Input/Ouput: ", args.recfile, args.ligfile, args.radius, args.outfile)

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

    print(len(residueList))

    io = PDB.PDBIO()
    io.set_structure(rec)
    io.save(args.outfile+args.radius+'.pdb', select=ResSelect(residueList))

if __name__ == '__main__':
    main()