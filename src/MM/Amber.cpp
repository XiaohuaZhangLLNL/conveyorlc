/* 
 * File:   Amber.cpp
 * Author: zhang30
 * 
 * Created on December 15, 2011, 5:43 PM
 */

#include <vector>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Amber.h"
#include "BackBone/Protein.h"
#include "BackBone/Ligand.h"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"

namespace LBIND {

Amber::Amber() {
    version=10;
    AMBERPATH=getenv("AMBERHOME");
}    
    
Amber::Amber(int amberVersion) {
    version=amberVersion;
    AMBERPATH=getenv("AMBERHOME");
}

Amber::Amber(Protein* pProt) : pProtein(pProt){
    version=10;
    AMBERPATH=getenv("AMBERHOME");
}

Amber::Amber(Protein* pProt, int amberVersion) : pProtein(pProt){
    version=amberVersion;
    AMBERPATH=getenv("AMBERHOME");
}

Amber::Amber(const Amber& orig) {
}

Amber::~Amber() {
}

void Amber::run() {
    this->prepLigands();
}

void Amber::reduce(std::string& input, std::string& output, std::string& options){
    //! Assue PDB file name suffix is .pdb
//    std::string pdbFNbase=pdbFName.substr(0,pdbFName.size()-4);
    //! reduce sustiva.pdb > sustiva_h.pdb
    std::string cmd=AMBERPATH +"/bin/reduce "+options+" "+input+" > " +output;
    system(cmd.c_str());    
}

void Amber::antechamber(std::string& input, std::string& output, std::string& options){
    //! Assue PDB file name suffix is .pdb
//    std::string pdbFNbase=pdbFName.substr(0,pdbFName.size()-4);
    // ! antechamber -i sustiva_new.pdb -fi pdb -o sustiva.mol2 -fo mol2 -c bcc -s 2
//    std::stringstream ss;
//    ss << totCharge;
//    char buffer[256];
//    snprintf(buffer, sizeof(buffer), "%g", totCharge);
//    std::string strTotalCharge=buffer;
    std::string cmd=AMBERPATH +"/bin/antechamber -i " +input + " -fi pdb -o "
            + output + " -fo mol2 -s 0 -pf yes "+options+" >& antechamber.out"; 
    std::cout << cmd << std::endl;
    system(cmd.c_str());       
}

void Amber::parmchk(std::string mol2FName){
    //! Assue PDB file name suffix is .mol2
    std::string mol2FBase=mol2FName.substr(0,mol2FName.size()-5);
    //! parmchk -i sustiva.mol2 -f mol2 -o sustiva.frcmod
    std::string cmd=AMBERPATH +"/bin/parmchk -i " + mol2FName + " -f mol2 -o "
            +mol2FBase+".frcmod";

    std::cout << cmd << std::endl;
    system(cmd.c_str());     
}

void Amber::ligLeapInput(std::string pdbid, std::string ligName, std::string tleapFName){
    std::ofstream tleapFile;
    try {
        tleapFile.open(tleapFName.c_str());
    }
    catch(...){
        std::string mesg="Amber::createLeapInput()\n\t Cannot open VMD file: "+tleapFName;
        throw LBindException(mesg);
    }   

    if(version==16){
        tleapFile << "source leaprc.protein.ff14SB" << std::endl;
    }else{
        tleapFile << "source leaprc.ff99SB" << std::endl;
    }    

    tleapFile << "source leaprc.gaff" << std::endl;

    tleapFile << "source leaprc.water.tip3p\n";

    tleapFile << ligName <<" = loadmol2 " << pdbid <<"-lig-"<< ligName << ".mol2 " << std::endl;
    tleapFile << "check " << ligName << std::endl;
    tleapFile << "loadamberparams " << pdbid <<"-lig-"<< ligName << ".frcmod" << std::endl;
    tleapFile << "saveoff " << ligName <<" " << ligName <<".lib " << std::endl;
    tleapFile << "saveamberparm " << ligName <<" " << ligName <<".prmtop " << ligName <<".inpcrd" << std::endl;
    tleapFile << "quit " << std::endl;
    
    tleapFile.close();
}

void Amber::comLeapInput(std::string pdbid, std::string ligName, std::string tleapFName){

    std::ofstream tleapFile;
    try {
        tleapFile.open(tleapFName.c_str());
    }
    catch(...){
        std::string mesg="Amber::createLeapInput()\n\t Cannot open VMD file: "+tleapFName;
        throw LBindException(mesg);
    }   
    
    if(version==16){
        tleapFile << "source leaprc.protein.ff14SB" << std::endl;
    }else{
        tleapFile << "source leaprc.ff99SB" << std::endl;
    }
    
    tleapFile << "source leaprc.gaff" << std::endl;
    tleapFile << "source leaprc.water.tip3p\n";

    tleapFile << "loadamberparams " << pdbid <<"-lig-"<< ligName << ".frcmod" << std::endl;
    tleapFile << "loadoff " << ligName <<".lib " << std::endl;
    tleapFile << "complex = loadpdb ../" << pdbid << ".pdb " << std::endl;
    tleapFile << "saveamberparm complex " << pdbid <<".prmtop " << pdbid <<".inpcrd" << std::endl;
    tleapFile << "quit " << std::endl;
    
    tleapFile.close();
}

void Amber::tleapInput(std::string& mol2FName, std::string& ligName, std::string& tleapFName){
    std::ofstream tleapFile;
    try {
        tleapFile.open(tleapFName.c_str());
    }
    catch(...){
        std::string mesg="Amber::createLeapInput()\n\t Cannot open VMD file: "+tleapFName;
        throw LBindException(mesg);
    }  
    
    std::string mol2FBase=mol2FName.substr(0,mol2FName.size()-5);
       
    if(version==16){
        tleapFile << "source leaprc.protein.ff14SB" << std::endl;
    }else{
        tleapFile << "source leaprc.ff99SB" << std::endl;
    }
    
    tleapFile << "source leaprc.gaff" << std::endl;

    tleapFile << "source leaprc.water.tip3p\n";

    tleapFile << "loadamberparams " << mol2FBase << ".frcmod" << std::endl;    
    tleapFile << ligName <<" = loadmol2 " << mol2FName << std::endl;
    tleapFile << "check " << ligName << std::endl;
    tleapFile << "saveoff " << ligName <<" " << ligName <<".lib " << std::endl;
    tleapFile << "set default PBRadii mbondi2" << std::endl;
    tleapFile << "saveamberparm " << ligName <<" " << ligName <<".prmtop " << ligName <<".inpcrd" << std::endl;
    tleapFile << "quit " << std::endl;
    
    tleapFile.close();
}

void Amber::tleap(std::string input){
    std::string cmd=AMBERPATH +"/bin/tleap -f " + input + " > leap.out";
    std::cout << cmd << std::endl;
    system(cmd.c_str());        
}


void Amber::minimization(std::string pdbid){
    std::string minFname=pdbid+"-min.in";
    std::ofstream minFile;
    try {
        minFile.open(minFname.c_str());
    }
    catch(...){
        std::string mesg="Amber::minimization(std::string pdbid)\n\t Cannot open min input file: "+minFname;
        throw LBindException(mesg);
    }   

    minFile << "Initial minimisation of sustiva-RT complex" << std::endl;
    minFile << " &cntrl" << std::endl;
    minFile << "  imin=1, maxcyc=200, ncyc=50," << std::endl;
    minFile << "  cut=16, ntb=0, igb=1," << std::endl;
    minFile << " &end " << std::endl;
    
    minFile.close();    

    //! sander -O -i min.in -o 1FKO_sus_min.out -p 1FKO_sus.prmtop -c 1FKO_sus.inpcrd  -r 1FKO_sus_min.crd  & 
    std::string cmd=AMBERPATH +"/bin/sander -O -i "+pdbid+"-min.in -o "+pdbid+"-min.out -p "
            +pdbid +".prmtop -c " + pdbid +".inpcrd -r "+pdbid+"-min.crd";
    std::cout << cmd << std::endl;
    system(cmd.c_str());       
    
    //!ambpdb -p 1FKO_sus.prmtop <1FKO_sus_min.crd > 1FKO_sus_min.pdb
    cmd=AMBERPATH +"/bin/ambpdb -p " + pdbid +".prmtop < "+pdbid+"-min.crd > "+pdbid+"-min.pdb";
    std::cout << cmd << std::endl;
    system(cmd.c_str()); 
       
    //! Analysis the output
    std::string minlogFName=pdbid+"-min.out";
    std::ifstream minlogFile;
    try {
        minlogFile.open(minlogFName.c_str());
    }
    catch(...){
        std::string mesg="Namd::minimization(std::string pdbid)\n\t Cannot open minimization log file: "+minlogFName;
        throw LBindException(mesg);
    }    
    
    const std::string finalstr="FINAL RESULTS";
    const std::string headstr="NSTEP";
    
    std::string fileLine="";    
    
    double finalEnergy;
    bool isFinalResult=false;
    while(minlogFile){
        std::getline(minlogFile, fileLine);
//        std::cout << fileLine << std::endl;
        if(fileLine.size()>32){
            if(fileLine.compare(20,13, finalstr)==0){
                isFinalResult=true;
            }
        }
        
        if(isFinalResult){
            if(fileLine.size()>7){
                if(fileLine.compare(3,5, headstr)==0){
                    std::getline(minlogFile, fileLine);
                    std::vector<std::string> tokens;
                    tokenize(fileLine, tokens); 
                    if(tokens.size() > 2){
                        finalEnergy=atof(tokens[1].c_str());
                    }
                    break;
                }
            }
        }
    }
    minlogFile.close();
    
    std::cout << "The minimized total energy is: " << finalEnergy << std::endl;
                
}

void Amber::prepLigands(){
    std::vector<Ligand*> ligList;
    pProtein->getLigands(ligList);
    std::string pdbid=pProtein->getPDBID();
    for(unsigned i=0; i<ligList.size(); ++i){
        std::string ligname=ligList[i]->getName();
        std::string ligFBase=pdbid+"-lig-"+ligname;
        std::string cmd="mkdir -p "+ligFBase;
        system(cmd.c_str());        
        chdir(ligFBase.c_str());        
        std::string input=ligname+".pdb";
        std::string output=ligname+"-rd.pdb";
        std::string options="";
        this->reduce(input, output, options);
        input=output;
        output=ligname+".mol2";
        options=" -c bcc -s 2";
        this->antechamber(input, output, options);
        std::string ligFName=ligFBase+".mol2";
        this->parmchk(ligFName);
        std::string tleapFName="ligLeap.in";        
        this->ligLeapInput(pdbid, ligname, tleapFName);
        this->tleap(tleapFName);
        tleapFName="comLeap.in";
        this->comLeapInput(pdbid, ligname, tleapFName);
        this->tleap(tleapFName);
        this->minimization(pdbid);
        chdir("..");
    }
}


}//namespace LBIND 
