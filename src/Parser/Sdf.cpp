/* 
 * File:   Sdf.cpp
 * Author: zhang30
 * 
 * Created on August 10, 2012, 3:12 PM
 */

#include "Sdf.h"

#include <iostream>
#include <fstream>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Atom.h"


namespace LBIND{

Sdf::Sdf() {
}

Sdf::Sdf(const Sdf& orig) {
}

Sdf::~Sdf() {
}

void Sdf::parse(const std::string& fileName){

    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Sdf::parse >> Cannot open file" << fileName << std::endl;
    }

    std::string fileLine=""; 

    static const boost::regex terRegex("^TER");  
    
    while(inFile){
        std::getline(inFile, fileLine);
//
//        if(boost::regex_search(fileLine,what,terRegex)){
//            newMolecule=true;
//        }
    }
}


void Sdf::read(const std::string& fileName, Molecule* pMolecule) {
    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch (...) {
        throw LBindException("Sdf::read >> Cannot open file " + fileName);
    }

    std::string fileLine = "";

    double totalCharge = 0;

    std::vector<std::string> lines;
    while (inFile) {
        getline(inFile,fileLine);
        lines.push_back(fileLine);
    }

    if(lines.size()<4){
        throw LBindException("SDF file " + fileName +" has less than 4 lines");
    }

    std::string MoleculeName = lines[0];
    MoleculeName.erase(std::remove(MoleculeName.begin(), MoleculeName.end(), '\n'), MoleculeName.end());
    pMolecule->setName(MoleculeName);

    std::vector<std::string> tokens;
    tokenize(lines[3], tokens);
    int natoms=Sstrm<int, std::string>(tokens[0]);
    int nbonds=Sstrm<int, std::string>(tokens[1]);

    if(lines.size()<natoms+4){
        throw LBindException("SDF file " + fileName +" dosen't have " + tokens[0]+ " atom records");
    }

    for(int i=4; i<natoms+4; i++){
        std::vector<std::string> tokens;
        tokenize(lines[i], tokens);

        int fileID = i-3;
        std::string atomName= tokens[3];
//                int rID=Sstrm<int, std::string>(tokens[6]);

        double x=Sstrm<double, std::string>(tokens[0]);
        double y=Sstrm<double, std::string>(tokens[1]);
        double z=Sstrm<double, std::string>(tokens[2]);


        std::vector<std::string> tokens2;
        tokenize(tokens[5], tokens2, ".");
        std::string typeName=tokens2[0];

        Atom* pAtom=pMolecule->addAtom();

        pAtom->setID(fileID);
        pAtom->setFileID(fileID);
        pAtom->setName(atomName);
        pAtom->setCoords(x,y,z);
        pAtom->setSymbol(atomName);

    }

}

bool Sdf::calcBoundBox(const std::string& fileName, Coor3d& centroid, Coor3d& boxDim){
    Molecule* pMolecule=new Molecule();
    read(fileName, pMolecule);
    bool success=pMolecule->boundBox(centroid, boxDim);
    delete pMolecule;
    return success;
}


std::string Sdf::getInfo(const std::string& fileName, const std::string& keyword){

    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Sdf::parse >> Cannot open file" << fileName << std::endl;
    }

    std::string fileLine="";
    std::string info="";

    //static const boost::regex terRegex(keyword.c_str());  
    const boost::regex terRegex(keyword.c_str());  
    boost::smatch what;
    
    int count=3;
    
    while(inFile){
        std::getline(inFile, fileLine);

        if(boost::regex_search(fileLine,what,terRegex)){
            count=0;
        }
        if(count==1){
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens, "\n" );
            if(tokens.size() > 0){
                return tokens[0];
            }
        }
        count=count+1;
    }
    
    inFile.close();
    
    return info;
}

std::string Sdf::getTitle(const std::string& fileName){

    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Sdf::parse >> Cannot open file" << fileName << std::endl;
    }
    
    std::string fileLine="";
    std::string info="";
    std::getline(inFile, fileLine);
    std::vector<std::string> tokens;
    tokenize(fileLine, tokens, "\n" );
    if(tokens.size() > 0){
        return tokens[0];
    }    
    return info;
}


} //namespace LBIND
