/* 
 * File:   Pdb.cpp
 * Author: xiaohua
 * 
 * Created on May 26, 2009, 7:23 AM
 */

#include "Pdb.h"
#include "Complex.h"
#include "Molecule.h"
#include "Fragment.h"
#include "Atom.h"
#include "Sstrm.hpp"
#include "Element.h"
//#include "Conformer.h"
#include "Coordinates.h"

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

namespace LBIND{

Pdb::Pdb() {
}

Pdb::Pdb(const Pdb& orig) {
}

Pdb::~Pdb() {
}

void Pdb::read(const std::string& fileName, boost::shared_ptr<Complex> pComplex)
{
    this->read(fileName, pComplex);
}

void Pdb::read(const std::string& fileName, Complex* pComplex)
{
    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "PDB::read >> Cannot open file" << fileName << std::endl;
    }

    static const boost::regex terRegex("^TER");
    static const boost::regex infRegex("^(ATOM|HETATM)[ |\t]*(\\d+)[ |\t]+(\\w+)[ |\t]+(\\w+)");
    static const boost::regex chaRegex("^[ |\t]*([A-Za-z])");
    static const boost::regex corRegex("^[ |\t]*(\\d+)[ |\t]*([ |-]\\d+.\\d+)[ |\t]*([ |-]\\d+.\\d+)[ |\t]*([ |-]\\d+.\\d+)");
    static const boost::regex numRegex("^[ |\t]*([ |-]\\d+.\\d+)");
    static const boost::regex typRegex("^[ |\t]*(\\w+)");
    boost::smatch what;
    std::string fileLine="";
    
    bool newMolecule = true;
    bool newFragment = true;

    Molecule* pMolecule=0;
    Fragment* pRes=0;
    Atom* pAtom=0;
//    Element* pElement=0;

    int MoleculeID=0;
//    int resID=0;
    int atomID=0;

    std::string oldMoleculeName="";
    std::string oldResName="";
    int oldResID=0;

    while(inFile){
        std::getline(inFile, fileLine);

        if(boost::regex_search(fileLine,what,terRegex)){
            newMolecule=true;
        }

        if(boost::regex_search(fileLine,what,infRegex)){
            
            int fileID = SsMatch<int>(what,2);
            std::string atomName= SsMatch<std::string>(what,3);
            std::string resName= SsMatch<std::string>(what,4);

//            std::cout << fileID << " " << atomName << " " << resName << std::endl;

            fileLine=what.suffix();
//            std::cout << fileLine << std::endl;

            std::string MoleculeName="";
            if(boost::regex_search(fileLine,what,chaRegex)){
                MoleculeName= SsMatch<std::string>(what,1);
                fileLine=what.suffix();
            }
//            std::cout << MoleculeName << std::endl;

            int rID=0;
            double x=0;
            double y=0;
            double z=0;

//            std::cout << fileLine << std::endl;
            if(boost::regex_search(fileLine,what,corRegex)){
//                std::cout << what[1] << " " << what[2] << " "<< what[3] << " "<< what[4] << std::endl;
                rID=SsMatch<int>(what,1);
                x=SsMatch<double>(what,2);
                y=SsMatch<double>(what,3);
                z=SsMatch<double>(what,4);
               
//                std::cout <<"x,y,z=" <<x <<" " <<y << " " <<z <<std::endl;
                fileLine=what.suffix();
            }

//            std::cout << rID << " " << x << " " << y << " " << z << std::endl;

            while(boost::regex_search(fileLine,what,numRegex)){
                fileLine=what.suffix();
            }

            std::string typeName="";
            if(boost::regex_search(fileLine,what,typRegex)){               
                typeName=SsMatch<std::string>(what,1);
            }

//            std::cout << typeName << std::endl;

//            std::cout <<oldMoleculeName<< " " << MoleculeName<<std::endl;

            if((oldMoleculeName!=MoleculeName)) {
                newMolecule=true;
            }

            if(newMolecule){
//                std::cout << "New Molecule!" << std::endl;
                newFragment=true;
                pMolecule=pComplex->addMolecule();
                MoleculeID++;
                pMolecule->setID(MoleculeID);
                pMolecule->setName(MoleculeName);
                newMolecule = false;
            }

            if((oldResName != resName) ||(oldResID != rID)) newFragment=true;

            if(newFragment){
                pRes=pMolecule->addFragment();
                pRes->setID(rID);
                pRes->setName(resName);
                newFragment = false;
            }

            pAtom=pRes->addAtom();
            atomID++;
            pAtom->setID(atomID);
            pAtom->setFileID(fileID);
            pAtom->setName(atomName);
            pAtom->setCoords(x,y,z);
            pAtom->setSymbol(typeName);
//            pElement=pAtom->addElement();
//            pElement->setElementSymbol(typeName);
            
            oldMoleculeName = MoleculeName;
            oldResName = resName;
            oldResID = rID;

        }

    }

    inFile.close();

}

void Pdb::parse(const std::string& fileName, Complex* pComplex){
    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "PDB::read >> Cannot open file" << fileName << std::endl;
    }

    std::string fileLine="";
    
    bool newMolecule = true;
    bool newFragment = true;

    Molecule* pMolecule=0;
    Fragment* pRes=0;
    Atom* pAtom=0;
//    Element* pElement=0;

    int MoleculeID=0;
//    int resID=0;
    int atomID=0;

    std::string oldMoleculeName="";
    std::string oldResName="";
    int oldResID=0;
    
    const std::string terStr="TER";
    const std::string atomStr="ATOM";
    const std::string hetatmStr="HETATM";

    while(std::getline(inFile, fileLine)){


        if(fileLine.compare(0,3, terStr)==0){
            newMolecule=true;
        }

        if(fileLine.compare(0,4, atomStr)==0 || fileLine.compare(0,6, hetatmStr)==0){

            std::string fileIDstr=fileLine.substr(7,5);
            int fileID = atoi(fileIDstr.c_str());
            
        // 	tmp.id           = convert_substring<unsigned>   (str,  7, 11);
        //	tmp.name         = convert_substring<std::string>(str, 13, 16);
        //	tmp.residue_id   = convert_substring<int>        (str, 23, 26);
        //	tmp.residue_name = convert_substring<std::string>(str, 17, 19);
        //	tmp.coords[0]    = convert_substring<fl>         (str, 31, 38);
        //	tmp.coords[1]    = convert_substring<fl>         (str, 39, 46);
        //	tmp.coords[2]    = convert_substring<fl>         (str, 47, 54);
        //	tmp.b_factor     = convert_substring<fl>         (str, 61, 66);
        //	tmp.element      = convert_substring<std::string>(str, 77, 78);           
            
            
            std::string atomName= fileLine.substr(13,4);
            std::string resName= fileLine.substr(17,3);

//            std::cout << fileID << " " << atomName << " " << resName << std::endl;

            std::string MoleculeName=fileLine.substr(22,1);
//            std::cout << MoleculeName << std::endl;
            
            std::string rIDstr= fileLine.substr(23,4);
            int rID=atoi(rIDstr.c_str());
            std::string xstr= fileLine.substr(31,8);
            double x=atof(xstr.c_str());
            std::string ystr= fileLine.substr(39,8);
            double y=atof(ystr.c_str());
            std::string zstr= fileLine.substr(47,8);
            double z=atof(zstr.c_str());

//            std::cout << rID << " " << x << " " << y << " " << z << std::endl;

            std::string typeName=fileLine.substr(77,1);
            
//            std::cout << typeName << std::endl;

            if(newMolecule){
//                std::cout << "New Molecule!" << std::endl;
                newFragment=true;
                pMolecule=pComplex->addMolecule();
                MoleculeID++;
                pMolecule->setID(MoleculeID);
                pMolecule->setName(MoleculeName);
                newMolecule = false;
            }

            if((oldResName != resName) ||(oldResID != rID)) newFragment=true;

            if(newFragment){
                pRes=pMolecule->addFragment();
                pRes->setID(rID);
                pRes->setName(resName);
                newFragment = false;
            }

            pAtom=pRes->addAtom();
            atomID++;
            pAtom->setID(atomID);
            pAtom->setFileID(fileID);
            pAtom->setName(atomName);
            pAtom->setCoords(x,y,z);
            pAtom->setSymbol(typeName);
//            pElement=pAtom->addElement();
//            pElement->setElementSymbol(typeName);
            
            oldMoleculeName = MoleculeName;
            oldResName = resName;
            oldResID = rID;

        }

    }

    inFile.close();    
    
}


void Pdb::write(const std::string& fileName, Complex* pComplex)
{
    std::ofstream outFile;
    try {
        outFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "PDB::read >> Cannot open file" << fileName << std::endl;
    }

    outFile << "REMARK PDB FILE CONVERTED BY PMOL" << std::endl;

    std::vector<Molecule*> molList=pComplex->getChildren();
    for(unsigned i=0;i<molList.size();i++){

        std::vector<Atom*> atomList=molList[i]->getGrdChildren();

        for(unsigned k=0;k<atomList.size();k++){

            outFile << "ATOM  "<< std::setw(5)<< atomList[k]->getFileID();
            if((atomList[k]->getName()).size()>2){
                outFile  << " " << std::left << std::setw(4) <<atomList[k]->getName();
            }else{
                outFile  << " " << " " << std::left << std::setw(3) <<atomList[k]->getName();
            }

            outFile << " " << std::left << std::setw(3) <<"UNK"
                    << " " << std::setw(1) << "A"
                    << std::right << std::setw(4) << "1"
                    << "    "
                    << std::fixed <<std::setprecision(3)
                    << std::setw(8) << atomList[k]->getX()
                    << std::setw(8) << atomList[k]->getY()
                    << std::setw(8) << atomList[k]->getZ()
//                    << std::setw(22) << " "
//                    << std::right << std::setw(2) << atomList[k]->getElement()->getElementSymbol()
                    << std::endl;
//                std::cout << "List Size: " << atomList.size() << "x= "<< atomList[k]->getX() << " y= " <<atomList[k]->getY()
//                        << " z= "<<atomList[k]->getZ() << std::endl;


        }
        outFile << "TER" <<std::endl;
//        std::cout << "i= " << i << std::endl;
    }
    outFile << "END" <<std::endl;

    outFile.close();
}

void Pdb::write(const std::string& fileName, boost::shared_ptr<Complex> pComplex)
{
    std::ofstream outFile;
    try {
        outFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "PDB::read >> Cannot open file" << fileName << std::endl;
    }

    outFile << "REMARK PDB FILE CONVERTED BY PMOL" << std::endl;

    std::vector<Molecule*> MoleculeList=pComplex->getChildren();
    for(unsigned i=0;i<MoleculeList.size();i++){
        std::vector<Fragment*> resList=MoleculeList[i]->getChildren();

        for(unsigned j=0;j<resList.size();j++){
            std::vector<Atom*> atomList=resList[j]->getChildren();

            for(unsigned k=0;k<atomList.size();k++){

                outFile << "ATOM  "<< std::setw(5)<< atomList[k]->getFileID();
                if((atomList[k]->getName()).size()>2){
                    outFile  << " " << std::left << std::setw(4) <<atomList[k]->getName();
                }else{
                    outFile  << " " << " " << std::left << std::setw(3) <<atomList[k]->getName();
                }

                outFile << " " << std::left << std::setw(3) <<resList[j]->getName()
                        << " " << std::setw(1) <<MoleculeList[i]->getName()
                        << std::right << std::setw(4) <<resList[j]->getID()
                        << "    "
                        << std::fixed <<std::setprecision(3)
                        << std::setw(8) << atomList[k]->getX()
                        << std::setw(8) << atomList[k]->getY()
                        << std::setw(8) << atomList[k]->getZ()
                        << std::setw(22) << " "
                        << std::right << std::setw(2) << atomList[k]->getElement()->getElementSymbol()
                        << std::endl;
//                std::cout << "List Size: " << atomList.size() << "x= "<< atomList[k]->getX() << " y= " <<atomList[k]->getY()
//                        << " z= "<<atomList[k]->getZ() << std::endl;

            }
        }
        outFile << "TER" <<std::endl;
//        std::cout << "i= " << i << std::endl;
    }
    outFile << "END" <<std::endl;

    outFile.close();
}

//! for the signel molecule in the complex without Molecule/Fragment
void Pdb::write(const std::string& fileName, Molecule* pMol){
    std::ofstream outFile;
    try {
        outFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "PDB::read >> Cannot open file" << fileName << std::endl;
    }

    outFile << "REMARK PDB FILE CONVERTED BY PMOL" << std::endl;

    std::vector<Atom*> atomList=pMol->getGrdChildren();

    for(unsigned k=0;k<atomList.size();k++){

        outFile << "ATOM  "<< std::setw(5)<< atomList[k]->getFileID();
        if((atomList[k]->getName()).size()>2){
            outFile  << " " << std::left << std::setw(4) <<atomList[k]->getName();
        }else{
            outFile  << " " << " " << std::left << std::setw(3) <<atomList[k]->getName();
        }

        outFile << " " << std::left << std::setw(3) <<"UNK"
                << " " << std::setw(1) <<" "
                << std::right << std::setw(4) << 1
                << "    "
                << std::fixed <<std::setprecision(3)
                << std::setw(8) << atomList[k]->getX()
                << std::setw(8) << atomList[k]->getY()
                << std::setw(8) << atomList[k]->getZ()
                << std::setw(22) << " "
                << std::right << std::setw(2) << atomList[k]->getElement()->getElementSymbol()
                << std::endl;
    //                std::cout << "List Size: " << atomList.size() << "x= "<< atomList[k]->getX() << " y= " <<atomList[k]->getY()
    //                        << " z= "<<atomList[k]->getZ() << std::endl;
    }
    outFile << "END" <<std::endl;
    outFile.close();
}

//void Pdb::write(const std::string& fileName, boost::shared_ptr<Conformer> pConformer){
//    this->write(fileName, pConformer.get());
//}
//
//void Pdb::write(const std::string& fileName, Conformer* pConformer){
//    std::vector<Coordinates*> coordList;
//    pConformer->getCoordinates(coordList);
//    Complex* pComplex=pConformer->getComplex();
//
//    for(unsigned int i=0; i<coordList.size(); ++i){
//        std::string numString=Sstrm<std::string, int>(i+1);
//        std::string outFile=fileName+numString+".pdb";
////        std::cout <<"file name: " << outFile << std::endl;
//
//        pComplex->assignCoordinate(coordList[i]);
//        this->write(outFile,pComplex->getChildren()[0]);
//
//    }
//}

}// namespace LBIND
