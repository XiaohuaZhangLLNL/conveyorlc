/* 
 * File:   Pdb.cpp
 * Author: xiaohua
 * 
 * Created on May 26, 2009, 7:23 AM
 */

#include "Pdb.h"
#include "Structure/Complex.h"
#include "Structure/Molecule.h"
#include "Structure/Fragment.h"
#include "Structure/Atom.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Element.h"
#include "Structure/StdResContainer.h"
//#include "Conformer.h"
#include "Structure/Coordinates.h"
#include "Structure/PMolException.h"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
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

            std::string fileIDstr=fileLine.substr(6,5);
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
            
            
            std::string atomName= fileLine.substr(12,4);
            std::string resName= fileLine.substr(17,3);

//            std::cout << fileID << " " << atomName << " " << resName << std::endl;

            std::string MoleculeName=fileLine.substr(21,1);
//            std::cout << MoleculeName << std::endl;
            
            std::string rIDstr= fileLine.substr(22,4);
            int rID=atoi(rIDstr.c_str());
            std::string xstr= fileLine.substr(30,8);
            double x=atof(xstr.c_str());
            std::string ystr= fileLine.substr(38,8);
            double y=atof(ystr.c_str());
            std::string zstr= fileLine.substr(46,8);
            double z=atof(zstr.c_str());

//            std::cout << rID << " " << x << " " << y << " " << z << std::endl;
            std::string typeName="  ";
            if(fileLine.size()>77){
                typeName=fileLine.substr(76,2);
            }
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

void Pdb::parseOut(const std::string& fileName, Complex* pComplex){
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
        
        std::vector<Fragment*> resList=molList[i]->getChildren();

        for(unsigned j=0;j<resList.size();j++){
            
        
            std::vector<Atom*> atomList=resList[j]->getChildren();

            for(unsigned k=0;k<atomList.size();k++){

                outFile << "ATOM  "<< std::setw(5)<< atomList[k]->getFileID()
                        << " " << std::setw(4) << atomList[k]->getName() 
                        << " " << std::left << std::setw(3) << resList[j]->getName()
                        << " " << std::setw(1) << molList[i]->getName()
                        << std::right << std::setw(4) << resList[j]->getID()
                        << "    "                        
                        << std::fixed <<std::setprecision(3)
                        << std::setw(8) << atomList[k]->getX()
                        << std::setw(8) << atomList[k]->getY()
                        << std::setw(8) << atomList[k]->getZ() 
                        <<"                      "
                        << std::setw(2) << atomList[k]->getSymbol()
                        << std::endl;
            }
        }
        outFile << "TER" <<std::endl;
//        std::cout << "i= " << i << std::endl;
    }
    outFile << "END" <<std::endl;

    outFile.close();    
}

std::string Pdb::newAtomName(const std::string& atomType, int seq){
    int strSize=5;
    if(seq<10){
        strSize=atomType.size()+1;
    }else if(seq<100){
        strSize=atomType.size()+2;
    }else if(seq<1000){
        strSize=atomType.size()+3;
    }
    std::stringstream ss;
    if(strSize==2){
        ss <<" " << atomType << seq<< " ";
    }else if(strSize==3){
        ss <<" " << atomType << seq;
    }else if(strSize==4){
        ss << atomType << seq;
    }else {
        int xxSize=4-atomType.size();
        ss << atomType << std::string(xxSize, 'X');
        std::cerr << "Warning: Atom name exceed 4 characters!" << std::endl;
    }
    return ss.str();
}

void Pdb::renameAtom(const std::string& inFileName, const std::string& outFileName){
    std::ifstream inFile;
    try {
        inFile.open(inFileName.c_str());
    }
    catch(...){
        std::cout << "PDB::read >> Cannot open file" << inFileName << std::endl;
    }
    
    std::ofstream outFile;
    try {
        outFile.open(outFileName.c_str());
    }
    catch(...){
        std::cout << "PDB::read >> Cannot open file" << outFileName << std::endl;
    }    

    std::string fileLine="";    
    
    const std::string atomStr="ATOM";
    const std::string hetatmStr="HETATM"; 
    
    const std::string ResStr="LIG";
    const std::string RidStr="  1";
    
    int nC=0;
    int nO=0;
    int nN=0;
    int nH=0;
    int nS=0;
    int nP=0;
    int nCL=0;
    int nE=0;   
    
    while(std::getline(inFile, fileLine)){

        if(fileLine.compare(0,4, atomStr)==0 || fileLine.compare(0,6, hetatmStr)==0){
            std::string newAtomName="";
            std::string atomType=fileLine.substr(77,1);
            
            if(atomType.compare(0, 1, "C")==0){
                nC=nC+1;
                newAtomName=this->newAtomName(atomType, nC);
            } else if(atomType.compare(0, 1, "O")==0){
                nO=nO+1;
                newAtomName=this->newAtomName(atomType, nO);
            } else if(atomType.compare(0, 1, "N")==0){
                nN=nN+1;
                newAtomName=this->newAtomName(atomType, nN);
            } else if(atomType.compare(0, 1, "H")==0){
                nH=nH+1;
                newAtomName=this->newAtomName(atomType, nH);
            } else if(atomType.compare(0, 1, "S")==0){
                nS=nS+1;
                newAtomName=this->newAtomName(atomType, nS);
            } else if(atomType.compare(0, 1, "P")==0){
                nP=nP+1;
                newAtomName=this->newAtomName(atomType, nP);
            } else if(atomType.compare(0, 1, "l")==0){
                nCL=nCL+1;
                atomType="CL";
                newAtomName=this->newAtomName(atomType, nCL);
            } else {
                nE=nE+1;
                newAtomName=this->newAtomName(atomType, nE);               
            }
            fileLine.replace(12, 4, newAtomName);
            // Also rename the residue name to LIG and residue ID to 1.
            fileLine.replace(17, 3, ResStr);
            fileLine.replace(23, 3, RidStr);
        }

        outFile << fileLine << std::endl;
           
    }
    
    inFile.close();
    outFile.close();
    
}

void Pdb::strip(const std::string& inFileName, const std::string& outFileName){
    std::ifstream inFile;
    try {
        inFile.open(inFileName.c_str());
    }
    catch(...){
        std::cout << "PDB::strip >> Cannot open file" << inFileName << std::endl;
    }
    
    std::ofstream outFile;
    try {
        outFile.open(outFileName.c_str());
    }
    catch(...){
        std::cout << "PDB::strip >> Cannot open file" << outFileName << std::endl;
    }    

    std::string fileLine="";    
    
    const std::string atomStr="ATOM";
    const std::string hetatmStr="HETATM";    
    
    while(std::getline(inFile, fileLine)){

        if(fileLine.compare(0,4, atomStr)==0 || fileLine.compare(0,6, hetatmStr)==0){
            outFile << fileLine << std::endl;
        }   
    }
    
    inFile.close();
    outFile.close();
    
}

void Pdb::cutByRadius(const std::string& inFileName, const std::string& outFileName, Coor3d& center, double radius){
    std::ifstream inFile;
    try {
        inFile.open(inFileName.c_str());
    }
    catch(...){
        std::cout << "PDB::cutByRadius >> Cannot open file" << inFileName << std::endl;
    }
    
    std::ofstream outFile;
    try {
        outFile.open(outFileName.c_str());
    }
    catch(...){
        std::cout << "PDB::cutByRadius >> Cannot open file" << outFileName << std::endl;
    }    

    std::string fileLine="";    
    
    const std::string atomStr="ATOM";
    const std::string hetatmStr="HETATM";   
    
    double radius2=radius*radius;
    
    while(std::getline(inFile, fileLine)){

        if(fileLine.compare(0,4, atomStr)==0 || fileLine.compare(0,6, hetatmStr)==0){

            std::string xstr= fileLine.substr(30,8);
            double x=atof(xstr.c_str());
            std::string ystr= fileLine.substr(38,8);
            double y=atof(ystr.c_str());
            std::string zstr= fileLine.substr(46,8);
            double z=atof(zstr.c_str());
            
            double distx=x-center.getX();
            double disty=y-center.getY();
            double distz=z-center.getZ();
            
            double dist2=distx*distx+disty*disty+distz*distz;
            
            if(dist2<radius2)
                outFile << fileLine << std::endl;
        }   
    }
    
    inFile.close();
    outFile.close();
    
}


int Pdb::splitByModel(const std::string& inFileName, const std::string& outFileBase){
    std::ifstream inFile;
    try {
        inFile.open(inFileName.c_str());
    }
    catch(...){
        std::cout << "PDB::cutByRadius >> Cannot open file" << inFileName << std::endl;
    }
    
   

    std::string fileLine="";    
    
    const std::string atomStr="ATOM";
    const std::string hetatmStr="HETATM";   
    const std::string modelStr="MODEL";
    const std::string endmdlStr="ENDMDL";

    bool outFlag=false;
    std::ofstream outFile;
    
    int count=0;
    
    while(std::getline(inFile, fileLine)){
        
        if(fileLine.compare(0,5, modelStr)==0 && outFlag==false){
            ++count;
            std::string outFileName=outFileBase+Sstrm<std::string, int>(count)+".pdb";
            try {
                outFile.open(outFileName.c_str());
            }
            catch(...){
                std::cout << "Pdb::splitByModel >> Cannot open file" << outFileName << std::endl;
            }   
            outFlag=true;
        }
        
        if(fileLine.compare(0,6, endmdlStr)==0){
            outFlag=false;
            outFile.close();
        }

        if(fileLine.compare(0,4, atomStr)==0 || fileLine.compare(0,6, hetatmStr)==0){
            if(outFlag){
                outFile << fileLine << std::endl;
            }
        }   
        
        
    }
    
    inFile.close();
    
    return count;
    
    
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

    std::vector<Molecule*> moleculeList=pComplex->getChildren();
    for(unsigned i=0;i<moleculeList.size();i++){
        std::vector<Fragment*> resList=moleculeList[i]->getChildren();

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
                        << " " << std::setw(1) <<moleculeList[i]->getName()
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

void Pdb::standardlizeD(const std::string& inFileName, const std::string& outFileName){
    
    boost::scoped_ptr<StdResContainer> pStdResContainer(new StdResContainer());
    
    std::ifstream inFile;
    try {
        inFile.open(inFileName.c_str());
    }
    catch(...){
        std::string mesg="PDB::standardlize >> Cannot open file" + inFileName;
        throw PMolException(mesg); 
    }
    
    std::ofstream outFile;
    try {
        outFile.open(outFileName.c_str());
    }
    catch(...){
        std::string mesg="PDB::standardlize >> Cannot open file" + outFileName;
        throw PMolException(mesg); 
    }  
    
//    std::ofstream outFile2;
//    try {
//        outFile2.open("out.pdb");
//    }
//    catch(...){
//        std::string mesg="PDB::standardlize >> Cannot open file" + outFileName;
//        throw PMolException(mesg); 
//    }     
       
    const std::string atomStr="ATOM";
    const std::string hetatmStr="HETATM";
    std::string fileLine="";
    
    bool printOUT=false;

    std::string oldResName="";
    int oldResID=0;
 
    std::vector<std::string> lines;

    while(std::getline(inFile, fileLine)){

        if(fileLine.compare(0,4, atomStr)==0 || fileLine.compare(0,6, hetatmStr)==0){
//            outFile2 << fileLine << std::endl;
            std::string resName= fileLine.substr(17,3);
            std::string rIDstr= fileLine.substr(23,4);
            int rID=atoi(rIDstr.c_str());
            
            if((oldResName != resName) ||(oldResID != rID)){
                if(pStdResContainer->find(resName)){
                    printOUT=true;
                }else{
                    std::cout << resName << " is not standard residue"<<std::endl;
                    printOUT=false;
                }                
            }
            
            if(printOUT){
                lines.push_back(fileLine);
                
            }
            
            oldResName = resName;
            oldResID = rID;            
           
        }

    }
    
    inFile.close(); 
    
    const std::string nAtom="N";
    const std::string cAtom="C";
    
    double nx=0;
    double ny=0;
    double nz=0;
    double cx=0;
    double cy=0;
    double cz=0; 
    
    oldResName="";
    oldResID=0;   
    
    std::vector<std::string> resLines;
    std::vector<std::string> oldResLines;
    
    int count=0;
    
    for(unsigned i=0; i<lines.size(); ++i){
   
        std::string resName= lines[i].substr(17,3);
        std::string rIDstr= lines[i].substr(23,4);
        int rID=atoi(rIDstr.c_str());        
        
        if((oldResName != resName) ||(oldResID != rID)){
            for(unsigned j=0; j<resLines.size(); ++j){
                std::string atomName=resLines[j].substr(12,4);
                atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());

//                std::cout << "N Atom Name =|" << atomName <<"|"<< std::endl;
                if(atomName==nAtom){
                    ++count;
                    if(count>1){
                        std::string xstr= resLines[j].substr(30,8);
                        nx=atof(xstr.c_str());
                        std::string ystr= resLines[j].substr(38,8);
                        ny=atof(ystr.c_str());
                        std::string zstr= resLines[j].substr(46,8);
                        nz=atof(zstr.c_str());
                    }
                } 
            } 
            if(count>1){
                double dist2=(nx-cx)*(nx-cx)+(ny-cy)*(ny-cy)+(nz-cz)*(nz-cz);
                std::cout << "C--N Distance Square =" << dist2 << std::endl;
                if(dist2>9){ //!bond distance large than 3 angstrom               
                    outFile<<"TER" << std::endl;
                    count=0;
                }
            }
            
            for(unsigned j=0; j<resLines.size(); ++j){
                outFile<<resLines[j]<< std::endl;
            }
            
            oldResLines=resLines;
            resLines.clear();
            for(unsigned j=0; j<oldResLines.size(); ++j){
                std::string atomName=resLines[j].substr(13,4);
                atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());
                
                if(atomName==cAtom){
                    std::string xstr= oldResLines[j].substr(30,8);
                    cx=atof(xstr.c_str());
                    std::string ystr= oldResLines[j].substr(38,8);
                    cy=atof(ystr.c_str());
                    std::string zstr= oldResLines[j].substr(46,8);
                    cz=atof(zstr.c_str());
                } 
            }
            

        }
        
        resLines.push_back(lines[i]);
        
        oldResName = resName;
        oldResID = rID;        
            
    }
    
    for(unsigned j=0; j<resLines.size(); ++j){
        outFile<<resLines[j]<< std::endl;
    }
    outFile << "END"<< std::endl;
         
}

void Pdb::standardlize(const std::string& inFileName, const std::string& outFileName){
    
    boost::shared_ptr<Complex> pComplex(new Complex());
    
    this->parse(inFileName, pComplex.get());
        
    // For HIS rename
    const std::string HIS="HIS";
    const std::string HD1="HD1";
    const std::string HE2="HE2";
    bool findHD1=false;
    bool findHE2=false;
           
    std::vector<Molecule*> moleculeList=pComplex->getChildren();
    for(unsigned i=0;i<moleculeList.size();i++){
        std::vector<Fragment*> resList=moleculeList[i]->getChildren();

        for(unsigned j=0;j<resList.size();j++){

            
            if(resList[j]->getName()==HIS){
//                std::cout << "old histidine=" << SresList[j]->getName() <<std::endl;
                
                std::vector<Atom*> atomList=resList[j]->getChildren();

                for(unsigned k=0;k<atomList.size();k++){
                    std::string atomName=atomList[k]->getName();
                    atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());
                    if(atomName==HD1){
                        findHD1=true;
                    }
                    if(atomName==HE2){
                        findHE2=true;
                    }
                }

                if(findHD1 && !findHE2){
//                    std::cout << "setting HID"  <<std::endl;
                    resList[j]->setName("HID");
                }
                if(!findHD1 && findHE2){
//                    std::cout << "setting HIE"  <<std::endl;
                    resList[j]->setName("HIE");
                }
                if(findHD1 && findHE2){
//                    std::cout << "setting HIP"  <<std::endl;
                    resList[j]->setName("HIP");
                }
//                std::cout << "new histidine=" << resList[j]->getName() <<std::endl;
                findHD1=false;
                findHE2=false;                

            }

        }
    }

    // Remove non-standard residues and ter by create only a copy molecule.
    boost::scoped_ptr<StdResContainer> pStdResContainer(new StdResContainer());
    
    boost::scoped_ptr<Molecule> pTmpMol(new Molecule());
    for(unsigned i=0;i<moleculeList.size();i++){
        
        std::vector<Fragment*> resList=moleculeList[i]->getChildren();
//        std::cout << "resList first size=" << resList.size() << std::endl;
//        for(unsigned j=0;j<resList.size();j++){
        for(std::vector<Fragment*>::iterator f=resList.begin(); f!=resList.end(); ++f){
            Fragment* pFrag=*f;
//            std::cout << "Residue Name="<< pFrag->getName() << std::endl;
            if(pStdResContainer->find(pFrag->getName())) {   
                pTmpMol->addFragment(pFrag);
                resList.erase(f);
                --f;               // if erase item iterator back up.
            }
        }
        
//        std::cout << "resList Second size=" << resList.size() << std::endl;
        
        moleculeList[i]->setChildren(resList); // To avoid residue pointers double deletion.
        
    }  

    for(unsigned i=0;i<moleculeList.size();i++){
        
        std::vector<Fragment*> resList=moleculeList[i]->getChildren();
//        std::cout << "resList final size=" << resList.size() << std::endl;
        
    }     
    
    // For create new molecular
    
    boost::shared_ptr<Complex> pNewCom(new Complex());
  
    std::vector<Fragment*> resList=pTmpMol->getChildren();
    std::vector<Fragment*> zeroResList;
    pTmpMol->setChildren(zeroResList); //! To avoid residue pointers double deletion.
    bool newMol=true;
    
    Molecule* pMolecule=0;
    
    const std::string cAtom="C";
    const std::string nAtom="N";

    bool foundC=false;
    bool foundN=false;    

    for(unsigned j=0;j<resList.size()-1;j++){
        if(newMol){
            pMolecule=pNewCom->addMolecule();
            newMol=false;
        }
        
        pMolecule->addFragment(resList[j]);
        

        
        Coor3d coorC;
        Coor3d coorN;
        
        std::vector<Atom*> atomList=resList[j]->getChildren();
        for(unsigned k=0;k<atomList.size();k++){
            std::string atomName=atomList[k]->getName();
            atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());
            if(atomName==cAtom){
                Coor3d* pCoor=atomList[k]->getCoords();
                coorC=*pCoor;
                foundC=true;
//                std::cout  <<resList[j]->getName() << " "<<atomName<< ":" << coorC << std::endl;
                break;
            }

        }
        
        atomList=resList[j+1]->getChildren();
        for(unsigned k=0;k<atomList.size();k++){
            std::string atomName=atomList[k]->getName();
            atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());
            if(atomName==nAtom){
                Coor3d* pCoor=atomList[k]->getCoords();
                coorN=*pCoor;
//                std::cout  <<resList[j+1]->getName() << " "<<atomName<< ":" << coorN << std::endl;
                foundN=true;
                break;
            }  
        }
        
//        std::cout << "C--N distance square=" << coorC.dist2(coorN) <<std::endl;
        if(foundC && foundN){
            if(coorC.dist2(coorN)>9){
//                std::cout << "C--N distance square=" << coorC.dist2(coorN) <<std::endl;
                newMol=true;
            }
        }else{
            newMol=true; 
        }
        
        foundC=false;
        foundN=false;
        
    }
        
    // Last residue
    if(newMol){
        pMolecule=pNewCom->addMolecule();
    }
    pMolecule->addFragment(resList[resList.size()-1]);
    
    this->parseOut(outFileName, pNewCom.get());
    
    moleculeList=pNewCom->getChildren();
    for(unsigned i=0;i<moleculeList.size();i++){
        
        std::vector<Fragment*> resList=moleculeList[i]->getChildren();
//        std::cout << "New molecule resList size=" << resList.size() << std::endl;
        
    }      
    return;
//    delete pComplex;
}

void Pdb::standardlize2(const std::string& inFileName, const std::string& outFileName){
    
    boost::shared_ptr<Complex> pComplex(new Complex());
    
    this->parse(inFileName, pComplex.get());
        

    std::ofstream nonAAfile;
    nonAAfile.open("nonAAlist");
    // Remove non-standard residues and ter by create only a copy molecule.
    boost::scoped_ptr<StdResContainer> pStdResContainer(new StdResContainer());
    
    boost::scoped_ptr<Molecule> pTmpMol(new Molecule());
    std::vector<Molecule*> moleculeList=pComplex->getChildren();
    
    for(unsigned i=0;i<moleculeList.size();i++){
        
        std::vector<Fragment*> resList=moleculeList[i]->getChildren();
//        std::cout << "resList first size=" << resList.size() << std::endl;
//        for(unsigned j=0;j<resList.size();j++){
        for(std::vector<Fragment*>::iterator f=resList.begin(); f!=resList.end(); ++f){
            Fragment* pFrag=*f;
//            std::cout << "Residue Name="<< pFrag->getName() << std::endl;
            if(pFrag->getName() != "HOH") {   
                // if erase item iterator back up.
               if(pStdResContainer->find(pFrag->getName())) {
                   pTmpMol->addFragment(pFrag);
                   resList.erase(f);
                   --f;  

               }else if(isAA(pFrag)){
                   toALA(pFrag);
                   pTmpMol->addFragment(pFrag);
                   resList.erase(f);
                   --f;                     
                   nonAAfile << pFrag->getName() << " convert to ALA" <<std::endl;
               }else{
                   nonAAfile << pFrag->getName() <<std::endl;
               }
                
            }
        }
        
//        std::cout << "resList Second size=" << resList.size() << std::endl;
        
        moleculeList[i]->setChildren(resList); // To avoid residue pointers double deletion.
        
    } 
    

//    for(unsigned i=0;i<moleculeList.size();i++){
//        
//        std::vector<Fragment*> resList=moleculeList[i]->getChildren();
////        std::cout << "resList final size=" << resList.size() << std::endl;
//        
//    }     
    
    // For create new molecular
    
    boost::shared_ptr<Complex> pNewCom(new Complex());
  
    std::vector<Fragment*> resList=pTmpMol->getChildren();
    std::vector<Fragment*> zeroResList;
    pTmpMol->setChildren(zeroResList); //! To avoid residue pointers double deletion.
    bool newMol=true;
    
    Molecule* pMolecule=0;
    
    const std::string cAtom="C";
    const std::string nAtom="N";

    bool foundC=false;
    bool foundN=false;    

    for(unsigned j=0;j<resList.size()-1;j++){
        if(newMol){
            pMolecule=pNewCom->addMolecule();
            newMol=false;
        }
        
        pMolecule->addFragment(resList[j]);
        

        
        Coor3d coorC;
        Coor3d coorN;
        
        std::vector<Atom*> atomList=resList[j]->getChildren();
        for(unsigned k=0;k<atomList.size();k++){
            std::string atomName=atomList[k]->getName();
            atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());
            if(atomName==cAtom){
                Coor3d* pCoor=atomList[k]->getCoords();
                coorC=*pCoor;
                foundC=true;
//                std::cout  <<resList[j]->getName() << " "<<atomName<< ":" << coorC << std::endl;
                break;
            }

        }
        
        atomList=resList[j+1]->getChildren();
        for(unsigned k=0;k<atomList.size();k++){
            std::string atomName=atomList[k]->getName();
            atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());
            if(atomName==nAtom){
                Coor3d* pCoor=atomList[k]->getCoords();
                coorN=*pCoor;
//                std::cout  <<resList[j+1]->getName() << " "<<atomName<< ":" << coorN << std::endl;
                foundN=true;
                break;
            }  
        }
        
//        std::cout << "C--N distance square=" << coorC.dist2(coorN) <<std::endl;
        if(foundC && foundN){
            if(coorC.dist2(coorN)>9){
//                std::cout << "C--N distance square=" << coorC.dist2(coorN) <<std::endl;
                newMol=true;
            }
        }else{
            newMol=true; 
        }
        
        foundC=false;
        foundN=false;
        
    }
        
    // Last residue
    if(newMol){
        pMolecule=pNewCom->addMolecule();
    }
    pMolecule->addFragment(resList[resList.size()-1]);
    
    this->parseOut(outFileName, pNewCom.get());
    
//    moleculeList=pNewCom->getChildren();
//    for(unsigned i=0;i<moleculeList.size();i++){
//        
//        std::vector<Fragment*> resList=moleculeList[i]->getChildren();
////        std::cout << "New molecule resList size=" << resList.size() << std::endl;
//        
//    }  
    return;
//    delete pComplex;
}

bool Pdb::isAA(Fragment* pFrag){
    Coor3d coorN;
    Coor3d coorCA;
    Coor3d coorC;
    Coor3d coorO;  
    bool findN;
    bool findCA;
    bool findC;
    bool findO;
    
    std::vector<Atom*> atomList=pFrag->getChildren();
    for(std::vector<Atom*>::iterator a=atomList.begin();a!=atomList.end();++a){
        Atom* pAtom=*a;
        std::string atomName=pAtom->getName();
        atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());
        if(atomName=="N"){
            Coor3d* pCoor=pAtom->getCoords();
            coorN=*pCoor;
            findN=true;
        }
        if(atomName=="CA"){        
            Coor3d* pCoor=pAtom->getCoords();
            coorCA=*pCoor;
            findCA=true;
        }
        if(atomName=="C"){           
            Coor3d* pCoor=pAtom->getCoords();
            coorC=*pCoor;
            findC=true;
        }
        if(atomName=="O"){           
            Coor3d* pCoor=pAtom->getCoords();
            coorO=*pCoor;
            findO=true;
        }        
    } 

    if(findN && findCA && findC && findO){
        if(coorN.dist2(coorCA)<9 && coorCA.dist2(coorC)<9 && coorC.dist2(coorO)<9){
            return true;
        }
    }
    
    return false;
}

void Pdb::toALA(Fragment* pFrag){

    std::vector<Atom*> newAtomList;    
    std::vector<Atom*> atomList=pFrag->getChildren();
    for(std::vector<Atom*>::iterator a=atomList.begin();a!=atomList.end();++a){
        Atom* pAtom=*a;
        std::string atomName=pAtom->getName();
        atomName.erase(std::remove_if(atomName.begin(), atomName.end(), isspace), atomName.end());
        if(atomName=="N"){
            newAtomList.push_back(pAtom);
            atomList.erase(a);
            --a;
        }
        if(atomName=="CA"){
            newAtomList.push_back(pAtom);
            atomList.erase(a);
            --a;            
        }
        if(atomName=="C"){
            newAtomList.push_back(pAtom);
            atomList.erase(a);
            --a;            
        }
        if(atomName=="O"){
            newAtomList.push_back(pAtom);
            atomList.erase(a);
            --a;            
        }        
    }
    
    for(unsigned k=0;k<atomList.size();k++){
        delete atomList[k];
    }
    
    pFrag->setName("ALA");
    pFrag->setChildren(newAtomList);

}

}// namespace LBIND
