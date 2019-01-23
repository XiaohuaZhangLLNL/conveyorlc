__author__ = 'zhang30'

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

from xml.dom import minidom

import argparse

"""
python PPL2FilterLigands.py -x PPL2Track.xml -s pur2.sdf -o PPL2BadLigand.xml -m PPL2MisLigand.xml

Two XML files are generated:
PPL2BadLigand.xml - ligands being calculated and failed
PPL2MisLigand.xml - ligands not being calculated
"""

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-x', '--xml', action='store', dest='xmlfile', default='PPL2Track.xml',
                        help='PPL2Track.xml input file (default=PPL2Track.xml)')
    parser.add_argument('-s', '--sdf', action='store', dest='sdffile', default=None,
                        help='SDF input file (default=None)')
    parser.add_argument('-il', '--idlist', action='store', dest='idlist', default=None,
                        help='ligand ID list to keep (default=None)')
    parser.add_argument('-nl', '--namelist', action='store', dest='namelist', default=None,
                        help='ligand name list to keep  (default=None)')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='PPL2BadLigand.xml',
                        help='ddcMD object output file (default=PPL2BadLigand.xml).')
    parser.add_argument('-m', '--miss', action='store', dest='misfile', default='PPL2MisLigand.xml',
                        help='ddcMD object output file (default=PPL2MisLigand.xml).')

    args = parser.parse_args()

    return args

def filter4BadMis(args):
    print ("Default inputs: ", args.xmlfile, args.sdffile)

    count=0
    with open(args.sdffile) as sdffile:
        for line in sdffile:
            if line[:4]=='$$$$':
                count=count+1

    calcFlag=[0]*count

    # read in XML
    tree = ET.ElementTree(file=args.xmlfile)
    root = tree.getroot()

    for ligand in root.findall('Ligand'):
        ligID=int(ligand.find('LigID').text)
        if ligID<=count:
            calcFlag[ligID-1]=1
        else:
            print("Error: ", ligID, "is larger than number of ligand in SDF.")
        mesg=ligand.find('Mesg').text
        if mesg=='Finished!':
            root.remove(ligand)

    tree.write(args.outfile)

    rootMis = ET.Element("Ligands")

    for index, flg in enumerate(calcFlag):
        if flg==0:
            misLig = ET.SubElement(rootMis, 'Ligand')
            ligIDele= ET.SubElement(misLig, 'LigID')
            ligIDele.text=str(index+1)

    xmlstr = minidom.parseString(ET.tostring(rootMis)).toprettyxml(indent="   ")

    with open(args.misfile, "w") as f:
        f.write(xmlstr)

def filterByLigandID(args):
    print ("Default inputs: ", args.xmlfile, args.idlist)

    ids=[]

    with open(args.idlist) as f:
        for line in f:
            ids.append(int(line.strip()))

    tree = ET.ElementTree(file=args.xmlfile)
    root = tree.getroot()

    for ligand in root.findall('Ligand'):
        ligID=int(ligand.find('LigID').text)
        if ligID not in ids:
            root.remove(ligand)

    tree.write(args.outfile)

def filterByLigandName(args):
    print ("Default inputs: ", args.xmlfile, args.idlist)

    names=[]

    with open(args.idlist) as f:
        for line in f:
            names.append(line.strip())

    tree = ET.ElementTree(file=args.xmlfile)
    root = tree.getroot()

    for ligand in root.findall('Ligand'):
        ligName=ligand.find('LigName').text
        if ligName not in names:
            root.remove(ligand)

    tree.write(args.outfile)


def main():

    args=getArgs()

    if args.sdffile:
        filter4BadMis(args)

    if args.idlist:
        filterByLigandID(args)

    if args.namelist:
        filterByLigandID(args)

if __name__ == '__main__':
    main()