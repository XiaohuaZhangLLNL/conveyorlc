__author__ = 'zhang30'

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

from xml.dom.minidom import parseString
from xml.dom import minidom

import argparse

"""
python rmProteinPPL3.py -x PPL3Track.xml -l list -o PPL3TrackNew.xml

Two XML files are generated:
PPL2BadLigand.xml - ligands being calculated and failed
PPL2MisLigand.xml - ligands not being calculated
"""

def getArgs():

    parser = argparse.ArgumentParser()
    parser.add_argument('-x', '--xml', action='store', dest='xmlfile', default='PPL3Track.xml',
                        help='PPL3Track.xml input file (default=PPL3Track.xml)')
    parser.add_argument('-l', '--list', action='store', dest='listfile', default='list',
                        help='list file to store protein names (default=list)')
    parser.add_argument('-o', '--out', action='store', dest='outfile', default='PPL3TrackNew.xml',
                        help='ddcMD object output file (default=PPL3TrackNew.xml).')

    args = parser.parse_args()

    return args

def main():

    args=getArgs()
    print("Default inputs: ", args.xmlfile, args.listfile, args.outfile)

    protList=[]

    with open(args.listfile, "r") as f:
        for line in f:
            protList.append(line.strip())

    print(protList)
    # read in XML
    tree = ET.ElementTree(file=args.xmlfile)
    root = tree.getroot()

    for complex in root.findall('Complex'):
        recID=complex.find('RecID').text
        if recID in protList:
            root.remove(complex)

    #tree.write(args.outfile)
    pretty_print = lambda data: '\n'.join([line for line in parseString(ET.tostring(data)).toprettyxml(indent=' ' * 4).split('\n') if line.strip()])

    with open(args.outfile, "w") as f:
        f.write(pretty_print(root)+'\n')

if __name__ == '__main__':
    main()