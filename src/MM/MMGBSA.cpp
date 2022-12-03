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
#include "Common/File.hpp"
#include "Common/Command.hpp"
#include "Parser/SanderOutput.h"

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

namespace LBIND{

MMGBSA::MMGBSA(const std::string& dir, const std::string& ligand, const std::string& workDir, const std::string& inputDir, int amberVersion) {
    WORKDIR=workDir;
    INPUTDIR=inputDir;
    recID=dir;
    ligID=ligand;
    version=amberVersion;
}

MMGBSA::MMGBSA(const std::string& dir, const std::string& ligand, std::vector<std::string>& nonStdRes, const std::string& workDir, const std::string& inputDir, int amberVersion) {
    WORKDIR=workDir;
    INPUTDIR=inputDir;
    recID=dir;
    ligID=ligand; 
    nonRes=nonStdRes;
    version=amberVersion;
}

MMGBSA::MMGBSA(const MMGBSA& orig) {
}

MMGBSA::~MMGBSA() {
}

bool MMGBSA::getligGB(std::string& checkfile, double& ligGB){
    std::ifstream inFile(checkfile.c_str());
    std::string fileLine="";
    std::string delimiter=":";
    while(inFile){
        std::getline(inFile, fileLine);
        if(fileLine.substr(0, 4)=="GBSA"){
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens, delimiter);
            ligGB=Sstrm<double, std::string>(tokens[1]);
            return true;
        }
    }
    return false;
}

void MMGBSA::run(std::string& poseID, bool restart){
    
    std::string libDir=INPUTDIR+"/lib/";
    std::string ligDir=WORKDIR+"/scratch/lig/"+ligID+"/";
    std::string recDir=WORKDIR+"/scratch/com/"+recID+"/rec/";
    std::string dockDir=WORKDIR+"/scratch/com/"+recID+"/dock/"+ligID+"/";
    std::string poseDir=WORKDIR+"/scratch/com/"+recID+"/gbsa/lig_"+ligID+"/pose_"+poseID+"/";
    
    std::string cmd="mkdir -p "+poseDir;
    std::string errMesg="MMGBSA::run mkdir poseDir fails";
    command(cmd, errMesg);
    chdir(poseDir.c_str());


    std::string ligCheckFile=ligDir+"checkpoint.txt";
    if(fileExist(ligCheckFile)){
        bool success=this->getligGB(ligCheckFile, ligGBen);
        if(!success) throw LBindException("Cannot get ligand GB energy from checkpoint.txt");
    }else{
        std::string sanderOut=ligDir+"LIG_minGB.out";
        boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
        bool success=pSanderOutput->getEnergy(sanderOut,ligGBen);
        if(!success) throw LBindException("Cannot get ligand GB energy");
    }
       
    cmd="ln -sf "+recDir+"Rec_min.pdb";
    errMesg="MMGBSA::run ln Rec_min.pdb fails";
    command(cmd, errMesg);
    
    //! Processing poses
    std::string posespdbqt=dockDir+"poses.pdbqt";
    std::string ligpdbqt=poseID+".pdbqt";
    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    int pID=Sstrm<int, std::string>(poseID);
//    std::cout << " recID=" << recID<< " ligID=" << ligID << " poseID=" << pID << std::endl;
//    std::cout << posespdbqt << std::endl;
//    std::cout << ligpdbqt << std::endl;
    pPdb->readByModel(posespdbqt, ligpdbqt, pID, score);
//    std::cout << "Score=" << score << std::endl;

    std::string posePDB="Lig_"+poseID+".pdb";    
    cmd="obabel -ipdbqt "+ligpdbqt+" -opdb -O "+posePDB;
    errMesg="MMGBSA::run obabel recDir fails";
    command(cmd, errMesg);
    
       
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
        if(version==16 || version==13){
            tleapFile << "source leaprc.ff14SB" << std::endl;
        }else{
            tleapFile << "source leaprc.ff99SB" << std::endl;
        }
        tleapFile << "source leaprc.gaff" << std::endl;

        tleapFile << "source leaprc.water.tip3p\n";
        
        tleapFile << "loadamberparams  " << ligDir <<  "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligDir << "/LIG.lib" <<std::endl;
        tleapFile << "LIG = loadpdb Lig_"<< poseID <<".pdb" << std::endl;
        tleapFile << "set default PBRadii mbondi2" << std::endl;
        tleapFile << "savepdb LIG Lig_lp_"<< poseID <<".pdb" << std::endl;            
        tleapFile << "quit " << std::endl;

        tleapFile.close();    
    }
    
    cmd="tleap -f "+tleapFName+" > Lig_leap.log";
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run tleap ligand fails";
    command(cmd, errMesg);
      
    cmd="cat Rec_min.pdb Lig_lp_"+poseID+".pdb > Com_"+poseID+".pdb";
    errMesg="MMGBSA::run combine rec and lig to com pdb fails";
    command(cmd, errMesg);

    std::vector<std::vector<int> > ssList;
    {
        std::string stdPdbFile="Rec_min.pdb";
        boost::scoped_ptr<Pdb> pPdb(new Pdb() );        
        pPdb->getDisulfide(stdPdbFile, ssList);
    }    
    
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

        if(version==16 || version==13){
            tleapFile << "source leaprc.ff14SB" << std::endl;
        }else{
            tleapFile << "source leaprc.ff99SB" << std::endl;
        }
        tleapFile << "source leaprc.gaff" << std::endl;

        tleapFile << "source leaprc.water.tip3p\n";
        
        for(unsigned int i=0; i<nonRes.size(); ++i){
            std::string nonResRaw=nonRes[i];
            std::vector<std::string> nonResStrs;
            const std::string delimiter=".";
            tokenize(nonResRaw, nonResStrs, delimiter);
            if(nonResStrs.size()==2 && nonResStrs[1]=="M"){
                tleapFile << nonResStrs[0] <<" = loadmol2 "<< libDir << nonResStrs[0] << ".mol2 \n";
            }else{
                tleapFile << "loadoff " << libDir << nonResStrs[0] << ".off \n";
            }

            tleapFile << "loadamberparams "<< libDir << nonResStrs[0] <<".frcmod \n";
        }        
        
        tleapFile << "loadamberparams  " << ligDir  << "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligDir  << "/LIG.lib" <<std::endl;
        tleapFile << "COM = loadpdb Com_"<< poseID <<".pdb" << std::endl;
        
        for(unsigned int i=0; i<ssList.size(); ++i){
            std::vector<int> pair=ssList[i];
            if(pair.size()==2){
                tleapFile << "bond COM."<< pair[0] <<".SG COM." << pair[1] <<".SG \n";
            }
        }  
        
        tleapFile << "set default PBRadii mbondi2" << std::endl;
        tleapFile << "saveamberparm COM Com.prmtop Com.inpcrd"<< std::endl;            
        tleapFile << "quit " << std::endl;

        tleapFile.close();    
    }
    cmd="tleap -f "+tleapFName+" > Com_leap.log";
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run tleap complex fails";
    command(cmd, errMesg);
    
    std::string checkFName="Com.prmtop";
    {
        if(!fileExist(checkFName)){
            std::string message="Com.prmtop file does not exist.";
            throw LBindException(message);        
        }

        if(fileEmpty(checkFName)){
            std::string message="Com.prmtop is empty.";
            throw LBindException(message);              
        }
    }
    
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
                << "  restraintmask='!@H= & !:LIG'\n"        
                << " /\n" << std::endl;
        
        minFile.close();    
    }          
    
    std::string sanderOut="Com_min_GB_"+poseID+".out";
    if(version==13){
        cmd="sander13  -O -i Com_min.in -o "+sanderOut+" -p Com.prmtop -c Com.inpcrd -ref Com.inpcrd  -x Com_" 
            +poseID+".mdcrd"+" -r Com_min"+poseID+".rst";
    }else{
        cmd="sander  -O -i Com_min.in -o "+sanderOut+" -p Com.prmtop -c Com.inpcrd -ref Com.inpcrd  -x Com_" 
            +poseID+".mdcrd"+" -r Com_min"+poseID+".rst";
    }
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run complex minimization fails";
    command(cmd, errMesg);
    
//    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    double comEnergy=0;
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    bool success=pSanderOutput->getEAmber(sanderOut,comEnergy);
    if(!success) throw LBindException("Cannot get complex GB energy");

    std::cout << "Complex GB Minimization Energy: " << comEnergy <<" kcal/mol."<< std::endl;   
    
    // receptor energy calculation
    if (version == 16) {
        cmd="ambpdb -p Com.prmtop -aatm -c Com_min"+poseID+".rst > Com_min.pdb";
    } else {
        cmd="ambpdb -p Com.prmtop -aatm < Com_min"+poseID+".rst > Com_min.pdb";
    }
    
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run ambpdb complex fails";
    command(cmd, errMesg); 
    
    cmd="grep -v LIG Com_min.pdb > rec_tmp.pdb ";
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run grep LIG Com_min.pdb fails";
    command(cmd, errMesg);
    
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
        
        if (version == 16 || version==13) {
            tleapFile << "source leaprc.ff14SB\n";
        } else {
            tleapFile << "source leaprc.ff99SB\n";
        }

        tleapFile << "source leaprc.gaff\n";

        tleapFile << "source leaprc.water.tip3p\n";
        
        for(unsigned int i=0; i<nonRes.size(); ++i){
            std::string nonResRaw=nonRes[i];
            std::vector<std::string> nonResStrs;
            const std::string delimiter=".";
            tokenize(nonResRaw, nonResStrs, delimiter);
            if(nonResStrs.size()==2 && nonResStrs[1]=="M"){
                tleapFile << nonResStrs[0] <<" = loadmol2 "<< libDir << nonResStrs[0] << ".mol2 \n";
            }else{
                tleapFile << "loadoff " << libDir << nonResStrs[0] << ".off \n";
            }

            tleapFile << "loadamberparams "<< libDir << nonResStrs[0] <<".frcmod \n";
        }                   
                
        tleapFile << "REC = loadpdb rec_tmp.pdb\n";
        
        for(unsigned int i=0; i<ssList.size(); ++i){
            std::vector<int> pair=ssList[i];
            if(pair.size()==2){
                tleapFile << "bond REC."<< pair[0] <<".SG REC." << pair[1] <<".SG \n";
            }
        } 
        tleapFile << "set default PBRadii mbondi2" << std::endl;
        tleapFile << "saveamberparm REC REC.prmtop REC.inpcrd\n"
                  << "quit\n";
        
        tleapFile.close();
    }

    cmd="tleap -f rec_leap.in > rec_leap.log";
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run tleap receptor fails";
    command(cmd, errMesg);

    checkFName="REC.prmtop";
    {
        if(!fileExist(checkFName)){
            std::string message="REC.prmtop file does not exist.";
            throw LBindException(message);        
        }

        if(fileEmpty(checkFName)){
            std::string message="REC.prmtop is empty.";
            throw LBindException(message);              
        }
    }    
    // end receptor energy re-calculation  
    
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
                << "  restraintmask='!@H='\n"        
                << " /\n" << std::endl;
        
        minFile.close();    
    }
    
    if(version==13){
        cmd="sander13 -O -i Rec_minGB.in -o Rec_minGB.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    }else{
        cmd="sander -O -i Rec_minGB.in -o Rec_minGB.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    }
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run receptor minimization fails";
    command(cmd, errMesg); 
    
    sanderOut="Rec_minGB.out";
    double recEnergy=0;
    success=pSanderOutput->getEnergy(sanderOut, recEnergy); 
    if(!success) throw LBindException("Cannot get receptor GB energy");
    
    bindGBen=comEnergy-recEnergy-ligGBen;

    cmd="rm -f Rec_min.pdb "+poseID+".pdbqt *.in Lig*.pdb leap.log fort.7 REC.* Rec_min.rst rec_tmp.pdb mmgbsa_results.tar.gz";
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run rm remove intermediate files fails";
    command(cmd, errMesg);   
    
    std::string fileList="Com.inpcrd  Com.prmtop  Com_"+poseID+".pdb  Com_leap.log  Com_min.pdb  Com_min"+poseID+".rst  "
            "Com_min_GB_"+poseID+".out  Lig_leap.log  Rec_minGB.out  rec_leap.log";
    cmd="tar -zcf mmgbsa_results.tar.gz "+fileList;
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run tar gzip files fails";
    command(cmd, errMesg); 
    
    cmd="rm -f "+fileList;
    //std::cout <<cmd <<std::endl;
    errMesg="MMGBSA::run remove all files fails";
    command(cmd, errMesg); 
    
}

double MMGBSA::getbindGB(){
    return bindGBen;
}

double MMGBSA::getScore(){
    return score;
}


} //namespace LBIND
