/* 
 * File:   Mol2.cpp
 * Author: zhang30
 * 
 * Created on September 26, 2012, 2:13 PM
 */

#include <string>
#include <fstream>
#include <iostream>

#include "Mol2.h"
#include "Structure/Atom.h"
#include "Structure/Molecule.h"
#include "Structure/Sstrm.hpp"
#include "Common/Tokenize.hpp"

namespace LBIND{

Mol2::Mol2() {
}

Mol2::Mol2(const Mol2& orig) {
}

Mol2::~Mol2() {
}

void Mol2::read(const std::string& fileName, Molecule* pMolecule){
    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Sdf::parse >> Cannot open file" << fileName << std::endl;
    }

    std::string fileLine="";
    std::string MoleculeName="";
    int natoms=0;
    int nbonds=0; 
    
    double totalCharge=0;
    
    while(inFile){
        getline(inFile,fileLine);

        if (fileLine.substr(0,17) == "@<TRIPOS>MOLECULE") {
            getline(inFile,fileLine);
            MoleculeName=fileLine;
            
            getline(inFile,fileLine);
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);
            natoms=Sstrm<int, std::string>(tokens[0]);  
            nbonds=Sstrm<int, std::string>(tokens[1]);
        }
        
        if (fileLine.substr(0,13) == "@<TRIPOS>ATOM") {
            
            std::string resName="LIG";
            for (int i = 1; i <= natoms; ++i) {
                getline(inFile,fileLine);
                std::vector<std::string> tokens;
                tokenize(fileLine, tokens);
                
                int fileID = Sstrm<int, std::string>(tokens[0]);
                std::string atomName= tokens[1];
//                int rID=Sstrm<int, std::string>(tokens[6]);

                double x=Sstrm<double, std::string>(tokens[2]);
                double y=Sstrm<double, std::string>(tokens[3]);
                double z=Sstrm<double, std::string>(tokens[4]);
                
                double charge=Sstrm<double, std::string>(tokens[8]);
                totalCharge=totalCharge+charge;

                std::vector<std::string> tokens2;
                tokenize(tokens[5], tokens2, ".");                
                std::string typeName=tokens2[0];

                Atom* pAtom=pMolecule->addAtom();

                pAtom->setID(i);
                pAtom->setFileID(fileID);
                pAtom->setName(atomName);
                pAtom->setCoords(x,y,z);
                pAtom->setSymbol(typeName);
                pAtom->setCharge(charge);
            }
        
        }
    }

    pMolecule->setCharge(totalCharge);
}

} //namespace LBIND
