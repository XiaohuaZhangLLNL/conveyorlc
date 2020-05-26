//
// Created by Zhang, Xiaohua on 5/24/20.
//
#include "obtest.h"
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <iostream>
#include <algorithm>

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <boost/scoped_ptr.hpp>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

#include "Parser/Sdf.h"
#include "Parser/Pdb.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Constants.h"
#include "Structure/Molecule.h"
#include "Parser/Mol2.h"
#include "Common/File.hpp"
#include "Common/Utils.h"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "Common/Command.hpp"


using namespace LBIND;
using namespace std;
using namespace OpenBabel;

// PR #1831
void testMolToCdxmlConversion()
{
    OBConversion conv;
    OBMol mol;
    conv.SetInFormat("sdf");
    conv.SetOutFormat("pdb");
    conv.SetOutputIndex(1);

    conv.ReadString(&mol, OBTestUtil::ReadFileContent("ligand.sdf"));
    std::string cdxmlFromMol = conv.WriteString(&mol, true);

    std::cout << cdxmlFromMol << std::endl;
    //std::string cdxmlTarget = OBTestUtil::ReadFileContent("alanine.pdb");

    //OB_COMPARE(cdxmlFromMol, cdxmlTarget);
}

void sdf2pdb(std::string& inputStr, std::string& outputStr)
{
    OBConversion conv;
    OBMol mol;
    conv.SetInFormat("sdf");
    conv.SetOutFormat("pdb");
    conv.SetOutputIndex(1);

    conv.ReadString(&mol, inputStr);
    outputStr = conv.WriteString(&mol, true);

}

void pdb2pdbqt(std::string& inputStr, std::string& outputStr)
{
    OBConversion conv;
    OBMol mol;
    conv.SetInFormat("pdb");
    conv.SetOutFormat("pdbqt");
    conv.SetOutputIndex(1);
    conv.SetOptions("n", conv.OUTOPTIONS);

    conv.ReadString(&mol, inputStr);
    outputStr = conv.WriteString(&mol, true);

}

int main(int argc, char** argv) {
    //testMolToCdxmlConversion();
    std::string sdfStr=OBTestUtil::ReadFileContent("ligand.sdf");
    std::string pdbStr="";
    sdf2pdb(sdfStr, pdbStr);
    //std::cout << pdbStr << std::endl;

    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    //! Rename the atom name.
    std::string pdbReNameStr="";
    pPdb->renameAtomStr(pdbStr, pdbReNameStr);
    //std::cout << pdbReNameStr << std::endl;

    std::string pdbStripStr="";
    pPdb->stripStr(pdbReNameStr, pdbStripStr);
    std::cout << pdbStripStr << std::endl;

    boost::scoped_ptr<Sdf> pSdf(new Sdf());
    std::cout << "Title: " <<pSdf->getTitleStr(sdfStr) << std::endl;

    std::cout << "cmpName: " <<pSdf->getInfoStr(sdfStr, "s_st_Chirality_1")<< std::endl;

    std::string pdbEleStr="";
    pPdb->fixElementStr(pdbStripStr, pdbEleStr);
    //std::cout << pdbStripStr << std::endl;

    std::string pdbqtStr="";
    pdb2pdbqt(pdbEleStr, pdbqtStr);
    std::cout << pdbqtStr << std::endl;

}