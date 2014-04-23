/* 
 * File:   MMGBSA.cpp
 * Author: zhang
 * 
 * Created on April 22, 2014, 4:27 PM
 */


#include <cstdlib>
#include <iostream>
#include <sstream>

#include "MM/MMGBSA.h"
#include "Parser/Sdf.h"
#include "Parser/Pdb.h"
#include "MM/Amber.h"
#include "Structure/Sstrm.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "Parser/SanderOutput.h"

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

namespace LBIND{

MMGBSA::MMGBSA(const std::string& dir, const std::string& ligand, const std::string& workDir) {
    WORKDIR=workDir;
    recID=dir;
    ligID=ligand; 
}

MMGBSA::MMGBSA(const MMGBSA& orig) {
}

MMGBSA::~MMGBSA() {
}

void MMGBSA::run(std::string& poseID){
    
    std::string ligDir=WORKDIR+"/scratch/lig/"+ligID+"/";
    std::string recDir=WORKDIR+"/scratch/com/"+recID+"/rec/";
    std::string dockDir=WORKDIR+"/scratch/com/"+recID+"/dock/"+ligID+"/";
    std::string poseDir=WORKDIR+"/scratch/com/"+recID+"/gbsa/lig_"+ligID+"/pose_"+poseID+"/";
    
    std::string cmd="mkdir -p "+poseDir;
    system(cmd.c_str());
    chdir(poseDir.c_str());
    
    if(!this->checkRun(poseID)){
        return;
    }    
    
    std::string sanderOut=ligDir+"LIG_minGB.out";
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    bool success=pSanderOutput->getEnergy(sanderOut,ligGBen);
       
    cmd="ln -s "+recDir+"Rec_min.pdb";
    system(cmd.c_str());
    
    //! Processing poses
    std::string posespdbqt=dockDir+"poses.pdbqt";
    std::string ligpdbqt=poseID+".pdbqt";
    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    int pID=Sstrm<int, std::string>(poseID);
    std::cout << " recID=" << recID<< " ligID=" << ligID << " poseID=" << pID << std::endl;
    std::cout << posespdbqt << std::endl;
    std::cout << ligpdbqt << std::endl;
    pPdb->readByModel(posespdbqt, ligpdbqt, pID, score);
    std::cout << "Score=" << score << std::endl;

    std::string posePDB="Lig_"+poseID+".pdb";    
    cmd="babel -ipdbqt "+ligpdbqt+" -opdb "+posePDB;
    system(cmd.c_str());
    
       
    std::string tleapFName="Lig_leap.in";
    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="mmgbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }   

        tleapFile << "source leaprc.ff99SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "loadamberparams  " << ligDir <<  "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligDir << "/LIG.lib" <<std::endl;
        tleapFile << "LIG = loadpdb Lig_"<< poseID <<".pdb" << std::endl;
        tleapFile << "savepdb LIG Lig_lp_"<< poseID <<".pdb" << std::endl;            
        tleapFile << "quit " << std::endl;

        tleapFile.close();    
    }
    
    cmd="tleap -f "+tleapFName+" >& Lig_leap.log";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
      
    cmd="cat Rec_min.pdb Lig_lp_"+poseID+".pdb > Com_"+poseID+".pdb";
    system(cmd.c_str());

    
    tleapFName="Com_leap.in";
    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="mmgbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }   

        tleapFile << "source leaprc.ff99SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "loadamberparams  " << ligDir  << "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligDir  << "/LIG.lib" <<std::endl;
        tleapFile << "COM = loadpdb Com_"<< poseID <<".pdb" << std::endl;
        tleapFile << "saveamberparm COM Com.prmtop Com.inpcrd"<< std::endl;            
        tleapFile << "quit " << std::endl;

        tleapFile.close();    
    }
    cmd="tleap -f "+tleapFName+" >& Com_leap.log";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    
    std::string minFName="Com_min.in";
    {
        std::ofstream minFile;
        try {
            minFile.open(minFName.c_str());
        }
        catch(...){
            std::string mesg="mmgbsa::receptor()\n\t Cannot open min file: "+minFName;
            throw LBindException(mesg);
        }   
        minFile << "title..\n" 
                << "&cntrl\n" 
                << "  imin   = 1,\n" 
                << "  ntmin   = 3,\n" 
                << "  maxcyc = 2000,\n" 
                << "  ncyc   = 1000,\n" 
                << "  ntpr   = 200,\n" 
                << "  ntb    = 0,\n" 
                << "  igb    = 5,\n" 
                << "  gbsa   = 1,\n"
                << "  cut    = 15,\n" 
                << "  ntr=1,\n" 
                << "  restraint_wt=5.0,\n" 
                << "  restraintmask='!:LIG'\n"        
                << " /\n" << std::endl;
        
        minFile.close();    
    }          
    
    sanderOut="Com_min_GB_"+poseID+".out";
    cmd="sander  -O -i Com_min.in -o "+sanderOut+" -p Com.prmtop -c Com.inpcrd -ref Com.inpcrd  -x Com_" 
            +poseID+".mdcrd"+" -r Com_min"+poseID+".rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
//    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    double comEnergy=0;
    success=pSanderOutput->getEAmber(sanderOut,comEnergy);

    std::cout << "Complex GB Minimization Energy: " << comEnergy <<" kcal/mol."<< std::endl;   
    
    // receptor energy calculation
    cmd="ambpdb -p Com.prmtop < Com_min"+poseID+".rst > Com_min.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    cmd="grep -v LIG Com_min.pdb > rec_tmp.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    
    // For receptor energy re-calculation
    tleapFName="rec_leap.in";
    
    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="mmpbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }
        
        tleapFile << "source leaprc.ff99SB\n"                
                  << "source leaprc.gaff\n"
                  << "REC = loadpdb rec_tmp.pdb\n"
                  << "saveamberparm REC REC.prmtop REC.inpcrd\n"
                  << "quit\n";
        
        tleapFile.close();
    }
    
    // end receptor energy re-calculation    

    cmd="tleap -f rec_leap.in >& rec_leap.log";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());

    minFName="Rec_minGB.in";
    {
        std::ofstream minFile;
        try {
            minFile.open(minFName.c_str());
        }
        catch(...){
            std::string mesg="mmpbsa::receptor()\n\t Cannot open min file: "+minFName;
            throw LBindException(mesg);
        }   

        minFile << "title..\n" 
                << "&cntrl\n" 
                << "  imin   = 1,\n" 
                << "  ntmin   = 3,\n" 
                << "  maxcyc = 1,\n" 
                << "  ncyc   = 1,\n" 
                << "  ntpr   = 1,\n" 
                << "  ntb    = 0,\n" 
                << "  igb    = 5,\n" 
                << "  gbsa   = 1,\n"
                << "  cut    = 15,\n" 
                << "  ntr=1,\n" 
                << "  restraint_wt=5.0,\n" 
                << "  restraintmask='!:LIG'\n"        
                << " /\n" << std::endl;
        
        minFile.close();    
    }
    
    cmd="sander -O -i Rec_minGB.in -o Rec_minGB.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    sanderOut="Rec_minGB.out";
    double recEnergy=0;
    success=pSanderOutput->getEnergy(sanderOut, recEnergy); 
    
    bindGBen=comEnergy-recEnergy-ligGBen;
      
}

double MMGBSA::getbindGB(){
    return bindGBen;
}

double MMGBSA::getScore(){
    return score;
}

bool MMGBSA::checkRun(std::string& poseID){
    std::string sanderOutFile="Com_min_GB_"+poseID+".out";
  
    std::ifstream inFile(sanderOutFile.c_str());
    
    if(!inFile){
        return true;
    }
    
    int i=system("grep 'FINAL RESULTS' file");
    if(i==0){
        return false;
    }
    
    return true;    
}


} //namespace LBIND