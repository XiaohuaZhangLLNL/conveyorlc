/* 
 * File:   SpMMGBSA.cpp
 * Author: zhang30
 * 
 * Created on August 31, 2012, 4:01 PM
 */
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "MM/SpMMGBSA.h"
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

SpMMGBSA::SpMMGBSA(const std::string& dir, const std::string& ligand) {
    WORKDIR=getenv("WORKDIR");
    amberDir=WORKDIR+"/"+dir+"/m_"+dir+"_amber/";
    ligDir=WORKDIR+"/../ligLib/"+ligand+"/";     
}


SpMMGBSA::SpMMGBSA(const SpMMGBSA& orig) {
}

SpMMGBSA::~SpMMGBSA() {
}

//bool SpMMGBSA::energy(const std::string& dir, const std::string& ligand){
//    std::string amberDir=WORKDIR+"/"+dir+"/m_"+dir+"_amber/";
//    
//    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
//    
//    std::string sanderOut=amberDir+"Rec_minGB2.out";
//    recGBen=0;
//    bool success;
//    success=pSanderOutput->getEAmber(sanderOut,recGBen);
//        
//    std::string ligDir=ligLibDir+"/"+ligand+"/";
//    
//    sanderOut=ligDir+"LIG_minGB.out";
//    ligGBen=0;
//    success=pSanderOutput->getEnergy(sanderOut,ligGBen);
//    
//    return true;
//}

//void SpMMGBSA::recRun(const std::string& dir){
//    
//    // RECEPTOR
//    std::string pdbqtFile="../../n_"+dir+"_vina/"+dir+".pdbqt";
//    std::string cmd="pdbqt_to_pdb.py -f " + pdbqtFile+" -o rec_qt.pdb";
//    system(cmd.c_str());
//    
//    cmd="reduce -Quiet -Trim rec_qt.pdb > rec_noh.pdb ";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());
//    
//    cmd="reduce -Quiet rec_noh.pdb > rec_rd.pdb";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());    
//
//    std::string tleapFName="rec_leap.in";
//    
//    {
//        std::ofstream tleapFile;
//        try {
//            tleapFile.open(tleapFName.c_str());
//        }
//        catch(...){
//            std::string mesg="mmgbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
//            throw LBindException(mesg);
//        }
//        
//        tleapFile << "source leaprc.protein.ff14SB\n"                
//                  << "source leaprc.gaff\n"
//                  << "REC = loadpdb rec_rd.pdb\n"
//                  << "saveamberparm REC REC.prmtop REC.inpcrd\n"
//                  << "quit\n";
//        
//        tleapFile.close();
//    }
//    
//    cmd="tleap -f rec_leap.in >& rec_leap.log";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());
//
//    std::string minFName="Rec_min.in";
//    {
//        std::ofstream minFile;
//        try {
//            minFile.open(minFName.c_str());
//        }
//        catch(...){
//            std::string mesg="mmgbsa::receptor()\n\t Cannot open min file: "+minFName;
//            throw LBindException(mesg);
//        }   
//
//        minFile << "title..\n" 
//                << "&cntrl\n" 
//                << "  imin   = 1,\n" 
//                << "  ntmin   = 3,\n" 
//                << "  maxcyc = 2000,\n" 
//                << "  ncyc   = 1000,\n" 
//                << "  ntpr   = 200,\n" 
//                << "  ntb    = 0,\n" 
//                << "  igb    = 5,\n" 
//                << "  cut    = 15,\n" 
//                << "  ntr=1,\n" 
//                << "  restraint_wt=5.0,\n" 
//                << "  restraintmask='!@H=',\n"        
//                << " /\n" << std::endl;
//        minFile.close();    
//    }
//    
//    cmd="sander -O -i Rec_min.in -o Rec_min.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());  
//    
//    std::string sanderOut="Rec_min.out";
//    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
//    recGBen=0;
//    bool success=pSanderOutput->getEAmber(sanderOut, recGBen);
//    std::cout << "Receptorn GB Minimization Energy: " << recGBen <<" kcal/mol."<< std::endl;
//       
//    cmd="ambpdb -p REC.prmtop -c Rec_min.rst > Rec_min_0.pdb";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());      
//    cmd="grep -v END Rec_min_0.pdb > Rec_min.pdb ";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());  
//    
//}


//void SpMMGBSA::ligRun(const std::string& ligand){
//        
//    //LIGAND
//    std::string cmd="ln -s "+ligLibDir+ligand+"/LIG.prmtop";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());
//    cmd="ln -s "+ligLibDir+ligand+"/LIG_min.rst";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str()); 
//    
//    std::string minFName="LIG_minGB.in";
//    {
//        std::ofstream minFile;
//        try {
//            minFile.open(minFName.c_str());
//        }
//        catch(...){
//            std::string mesg="mmgbsa::receptor()\n\t Cannot open min file: "+minFName;
//            throw LBindException(mesg);
//        }   
//
//        minFile << "title..\n" 
//                << "&cntrl\n" 
//                << "  imin   = 1,\n" 
//                << "  ntmin   = 3,\n" 
//                << "  maxcyc = 2000,\n" 
//                << "  ncyc   = 1000,\n" 
//                << "  ntpr   = 200,\n" 
//                << "  ntb    = 0,\n" 
//                << "  igb    = 5,\n" 
//                << "  gbsa   = 1,\n"
//                << "  cut    = 15,\n"       
//                << " /\n" << std::endl;
//        minFile.close();    
//    }
//    
//    cmd="sander -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG_min.rst -ref LIG_min.rst -x REC.mdcrd -r LIG_min0.rst";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());  
//    
//    
//    std::string sanderOut="LIG_minGB.out";
//    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
//    ligGBen=0;
//    bool success=pSanderOutput->getEnergy(sanderOut,ligGBen);
//    std::cout << "LIGAND GB Minimization Energy: " << ligGBen <<" kcal/mol."<< std::endl;
//       
//}
//


void SpMMGBSA::comRun(int poseID){
    
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

        tleapFile << "source leaprc.protein.ff14SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "loadoff atomic_ions.lib\n";
        tleapFile << "loadamberparams frcmod.ions234lm_1264_tip3p\n";
        tleapFile << "loadamberparams  " << ligDir <<  "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligDir << "/LIG.lib" <<std::endl;
        tleapFile << "LIG = loadpdb Lig_"<< poseID <<".pdb" << std::endl;
        tleapFile << "savepdb LIG Lig_lp_"<< poseID <<".pdb" << std::endl;            
        tleapFile << "quit " << std::endl;

        tleapFile.close();    
    }
    std::string cmd="tleap -f "+tleapFName+" >& Lig_leap.log";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
      
    cmd="cat Rec_min.pdb Lig_lp_"+Sstrm<std::string, int>(poseID)+
            ".pdb > Com_"+Sstrm<std::string, int>(poseID)+".pdb";
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

        tleapFile << "source leaprc.protein.ff14SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "loadoff atomic_ions.lib\n";
        tleapFile << "loadamberparams frcmod.ions234lm_1264_tip3p\n";
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
    
    std::string sanderOut="Com_min_GB_"+Sstrm<std::string, int>(poseID)+".out";
    cmd="sander  -O -i Com_min.in -o "+sanderOut+" -p Com.prmtop -c Com.inpcrd -ref Com.inpcrd  -x Com_" 
            +Sstrm<std::string, int>(poseID)+".mdcrd"+" -r Com_min"+Sstrm<std::string, int>(poseID)+".rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    double comEnergy=0;
    bool success=pSanderOutput->getEAmber(sanderOut,comEnergy);

    std::cout << "Complex GB Minimization Energy: " << comEnergy <<" kcal/mol."<< std::endl;   
    
    // receptor energy calculation
    cmd="ambpdb -p Com.prmtop -c Com_min"+Sstrm<std::string, int>(poseID)+".rst > Com_min.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    cmd="grep -v LIG Com_min.pdb > rec_tmp.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());      

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
    
    double bindEnergy=comEnergy-recEnergy-ligGBen;
    bindGBen.push_back(bindEnergy);    
}



void SpMMGBSA::run(const std::string& dir, const std::string& ligand){
    //! first get the GB energy of receptor and ligand.
//    bool calcEn=this->energy(dir, ligand);    
    
    std::string sanderOut=ligDir+"LIG_minGB.out";
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    bool success=pSanderOutput->getEnergy(sanderOut,ligGBen);
    
    chdir(WORKDIR.c_str());
    chdir(dir.c_str());
        
    std::string gbsaDir="p_"+dir+"_mmgbsa";
    std::string cmd="mkdir -p "+gbsaDir;
    system(cmd.c_str());
    chdir(gbsaDir.c_str());
    
    cmd="mkdir -p "+ligand;
    system(cmd.c_str());
    chdir(ligand.c_str());
   

    cmd="ln -s ../../m_"+dir+"_amber/Rec_min.pdb";
    system(cmd.c_str());
    
    //! Processing poses
    std::string poses="../../n_"+dir+"_vina/poses/"+ligand+".pdbqt";
    std::string posePDB=ligand+".pdb";
    
    cmd="babel -ipdbqt "+poses+" -opdb "+posePDB;
    system(cmd.c_str());
    
    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    std::string outFileBase="Lig_";
    int numPose=pPdb->splitByModel(posePDB,outFileBase);
    
    std::cout << "Number of Poses: " << numPose << std::endl;       

    // For receptor energy re-calculation
    std::string tleapFName="rec_leap.in";
    
    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="mmpbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }
        
        tleapFile << "source leaprc.protein.ff14SB\n"                
                  << "source leaprc.gaff\n"
                  << "loadoff atomic_ions.lib\n"
                  << "loadamberparams frcmod.ions234lm_1264_tip3p\n"
                  << "REC = loadpdb rec_tmp.pdb\n"
                  << "saveamberparm REC REC.prmtop REC.inpcrd\n"
                  << "quit\n";
        
        tleapFile.close();
    }
    
    // end receptor energy re-calculation
    
    for(int i=1; i<=numPose; ++i){
        if(this->checkRun(i)){
            this->comRun(i);
        }
    }
    

}

void SpMMGBSA::getbindGB(std::vector<double>& bindgb){
    bindgb=bindGBen;
}

bool SpMMGBSA::checkRun(int poseID){
    std::string sanderOutFile="Com_min_GB_"+Sstrm<std::string, int>(poseID)+".out";
    
//    if( !boost::filesystem::exists( sanderOutFile ) ){
//        return true;
//    }
  
    std::ifstream inFile(sanderOutFile.c_str());
    
    if(!inFile){
        return true;
    }
    
//    try {
//        inFile.open(sanderOutFile.c_str());
//    }
//    catch(...){
//        std::cout << "SanderOutput::getEnergy >> Cannot open file" << sanderOutFile << std::endl;
//    }
//
//    static const boost::regex finalRegex("FINAL RESULTS");
//    
//    boost::smatch what;
//    std::string fileLine="";
//    
//    while(inFile){
//        std::getline(inFile, fileLine);
//
//        if(boost::regex_search(fileLine,what,finalRegex)){
//            return false;
//        }
//        
//    }
    int i=system("grep 'FINAL RESULTS' file");
    if(i==0){
        return false;
    }
    
    return true;    
}


} //namespace LBIND
