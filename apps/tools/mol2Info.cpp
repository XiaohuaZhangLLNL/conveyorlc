/* 
 * File:   mol2Info.cpp
 * Author: zhang30
 *
 * Created on September 26, 2012, 4:30 PM
 */

#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include "Structure/Molecule.h"
#include "Structure/Coor3d.h"
#include "Parser/Mol2.h"
#include "Common/LBindException.h"
#include "Common/Tokenize.hpp"
#include "mol2InfoPO.h"

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

using namespace LBIND;

/*
 * 
 */
int main(int argc, char** argv) {
    POdata podata;
    
    bool success=mol2InfoPO(argc, argv, podata);
    if(!success){
        return 1;
    }    
    
    boost::scoped_ptr<Molecule> pMolecule(new Molecule());
    
    boost::scoped_ptr<Mol2> pMol2(new Mol2());
    
    pMol2->read(podata.mol2File,pMolecule.get());
    
    int charge=static_cast<int>(pMolecule->getCharge()+0.5);
    
    std::cout << "Total Charge: " << charge << std::endl;
    
    Coor3d coor;
    
    pMolecule->center(coor);
    
    std::cout << "centroid:   " << coor<< std::endl;
    
    std::ofstream outFile;
    try {
        outFile.open(podata.outputFile.c_str());
    }
    catch(...){
        std::cout << "Cannot open file: " << podata.outputFile << std::endl;
    } 
    
    outFile << "centroid:   " << coor<< std::endl;

    return 0;
}

