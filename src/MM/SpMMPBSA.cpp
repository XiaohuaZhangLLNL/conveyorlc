/* 
 * File:   SpMMPBSA.cpp
 * Author: zhang30
 * 
 * Created on August 31, 2012, 4:01 PM
 */
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "MM/SpMMPBSA.h"
#include "Parser/Sdf.h"
#include "Parser/Pdb.h"
#include "MM/Amber.h"
#include "Structure/Sstrm.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "Parser/SanderOutput.h"

#include <boost/scoped_ptr.hpp>


namespace LBIND{

SpMMPBSA::SpMMPBSA() : calcPB(true){
    WORKDIR=getenv("WORKDIR");
    ligLibDir=WORKDIR+"/../ligLib/";     
}

SpMMPBSA::SpMMPBSA(bool calcPBSA) : calcPB(calcPBSA) {
    WORKDIR=getenv("WORKDIR");
    ligLibDir=WORKDIR+"/../ligLib/";    
}

SpMMPBSA::SpMMPBSA(const SpMMPBSA& orig) {
}

SpMMPBSA::~SpMMPBSA() {
}

bool SpMMPBSA::energy(const std::string& dir, const std::string& ligand){
    std::string amberDir=WORKDIR+"/"+dir+"/m_"+dir+"_amber/";
    
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    
    std::string sanderOut=amberDir+"Rec_minGB2.out";
    recGBen=0;
    bool success;
    success=pSanderOutput->getEAmber(sanderOut,recGBen);
    
    sanderOut=amberDir+"Rec_minPB.out";   
    recPBen=0;
    success=pSanderOutput->getEnergy(sanderOut,recPBen);
    
    std::string ligDir=ligLibDir+"/"+ligand+"/";
    
    sanderOut=ligDir+"LIG_minGB.out";
    ligGBen=0;
    success=pSanderOutput->getEnergy(sanderOut,ligGBen);
    
    sanderOut=ligDir+"LIG_minPB.out";
    ligPBen=0;
    success=pSanderOutput->getEnergy(sanderOut,ligPBen);

    return true;
}

void SpMMPBSA::recRun(const std::string& dir){
    
    // RECEPTOR
    std::string pdbqtFile="../../n_"+dir+"_vina/"+dir+".pdbqt";
    std::string cmd="pdbqt_to_pdb.py -f " + pdbqtFile+" -o rec_qt.pdb";
    system(cmd.c_str());
    
    cmd="reduce -Quiet -Trim rec_qt.pdb > rec_noh.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());
    
    cmd="reduce -Quiet rec_noh.pdb > rec_rd.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());    

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
                  << "REC = loadpdb rec_rd.pdb\n"
                  << "set default PBRadii mbondi2\n" 
                  << "saveamberparm REC REC.prmtop REC.inpcrd\n"
                  << "quit\n";
        
        tleapFile.close();
    }
    
    cmd="tleap -f rec_leap.in >& rec_leap.log";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());

    std::string minFName="Rec_min.in";
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
                << "  maxcyc = 2000,\n" 
                << "  ncyc   = 1000,\n" 
                << "  ntpr   = 200,\n" 
                << "  ntb    = 0,\n" 
                << "  igb    = 5,\n" 
                << "  cut    = 15,\n" 
                << "  ntr=1,\n" 
                << "  restraint_wt=5.0,\n" 
                << "  restraintmask='!@H=',\n"        
                << " /\n" << std::endl;
        minFile.close();    
    }
    
    cmd="sander -O -i Rec_min.in -o Rec_min.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    std::string sanderOut="Rec_min.out";
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    recGBen=0;
    bool success=pSanderOutput->getEAmber(sanderOut, recGBen);
    std::cout << "Receptorn GB Minimization Energy: " << recGBen <<" kcal/mol."<< std::endl;
       
    cmd="ambpdb -p REC.prmtop -c Rec_min.rst > Rec_min_0.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());      
    cmd="grep -v END Rec_min_0.pdb > Rec_min.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    if(!calcPB) return; //! return if not going to calculation PB energy.
    
    //! The following section is to calculate PB energy.
    minFName="Rec_minPB.in";
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
                << " &cntrl\n" 
                << "  imin   = 1,\n" 
                << "  ntmin   = 3,\n" 
                << "  maxcyc = 2000,\n" 
                << "  ncyc   = 1000,\n" 
                << "  ntpr   = 200,\n" 
                << "  ntb    = 0,\n" 
                << "  igb    = 10,\n" 
                << "  cut    = 15,\n" 
                << "  ntr=1,\n" 
                << "  restraint_wt=5.0,\n" 
                << "  restraintmask='!@H='\n"        
                << " /\n"
                << " &pb\n"
                << "  npbverb=0, epsout=80.0, radiopt=1, space=0.5,\n"
                << "  accept=1e-4, fillratio=6, sprob=1.6\n"
                << " / \n" << std::endl;
                
        minFile.close();    
    } 
    cmd="sander -O -i Rec_minPB.in -o Rec_minPB.out -p REC.prmtop -c Rec_min.rst -ref Rec_min.rst -x REC.mdcrd -r Rec_min1.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    
    
    sanderOut="Rec_minPB.out";
    recPBen=0;
    success=pSanderOutput->getEAmber(sanderOut,recPBen);
    std::cout << "Receptor PB Minimization Energy: " << recPBen <<" kcal/mol."<< std::endl;    
}


void SpMMPBSA::ligRun(const std::string& ligand){
        
    //LIGAND
    std::string cmd="ln -s "+ligLibDir+ligand+"/LIG.prmtop";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());
    cmd="ln -s "+ligLibDir+ligand+"/LIG_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    
    std::string minFName="LIG_minGB.in";
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
                << "  maxcyc = 2000,\n" 
                << "  ncyc   = 1000,\n" 
                << "  ntpr   = 200,\n" 
                << "  ntb    = 0,\n" 
                << "  igb    = 5,\n" 
                << "  gbsa   = 1,\n"
                << "  cut    = 15,\n"       
                << " /\n" << std::endl;
        minFile.close();    
    }
    
    cmd="sander -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG_min.rst -ref LIG_min.rst -x REC.mdcrd -r LIG_min0.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    
    std::string sanderOut="LIG_minGB.out";
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    ligGBen=0;
    bool success=pSanderOutput->getEnergy(sanderOut,ligGBen);
    std::cout << "LIGAND GB Minimization Energy: " << ligGBen <<" kcal/mol."<< std::endl;
       
    
    if(!calcPB) return; //! return if not going to calculation PB energy.
    
    //! The following section is to calculate PB energy.
    minFName="LIG_minPB.in";
    {
        std::ofstream minFile;
        try {
            minFile.open(minFName.c_str());
        }
        catch(...){
            std::string mesg="mmpbsa::ligRun()\n\t Cannot open min file: "+minFName;
            throw LBindException(mesg);
        }   

        minFile << "title..\n" 
                << " &cntrl\n" 
                << "  imin   = 1,\n" 
                << "  ntmin   = 3,\n" 
                << "  maxcyc = 2000,\n" 
                << "  ncyc   = 1000,\n" 
                << "  ntpr   = 200,\n" 
                << "  ntb    = 0,\n" 
                << "  igb    = 10,\n" 
                << "  cut    = 15,\n"         
                << " /\n"
                << " &pb\n"
                << "  npbverb=0, epsout=80.0, radiopt=1, space=0.5,\n"
                << "  accept=1e-4, fillratio=6, sprob=1.6\n"
                << " / \n" << std::endl;
                
        minFile.close();    
    } 
    cmd="sander -O -i LIG_minPB.in -o LIG_minPB.out -p LIG.prmtop -c LIG_min0.rst -ref LIG_min0.rst -x REC.mdcrd -r LIG_min1.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    
    
    sanderOut="LIG_minPB.out";
    ligPBen=0;
    success=pSanderOutput->getEnergy(sanderOut,ligPBen);
    std::cout << "Ligand GB Minimization Energy: " << ligPBen <<" kcal/mol."<< std::endl;        
}



void SpMMPBSA::comRun(const std::string& ligand, int poseID){
    
    std::string tleapFName="Lig_leap.in";
    {
        std::ofstream tleapFile;
        try {
            tleapFile.open(tleapFName.c_str());
        }
        catch(...){
            std::string mesg="mmpbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }   

        tleapFile << "source leaprc.protein.ff14SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "loadoff atomic_ions.lib\n";
        tleapFile << "loadamberparams frcmod.ions234lm_1264_tip3p\n";
        tleapFile << "loadamberparams  " << ligLibDir << ligand << "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligLibDir << ligand << "/LIG.lib" <<std::endl;
        tleapFile << "LIG = loadpdb Lig_"<< poseID <<".pdb" << std::endl;
        tleapFile << "set default PBRadii mbondi2" << std::endl;
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
            std::string mesg="mmpbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }   

        tleapFile << "source leaprc.protein.ff14SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "loadoff atomic_ions.lib\n";
        tleapFile << "loadamberparams frcmod.ions234lm_1264_tip3p\n";
        tleapFile << "loadamberparams  " << ligLibDir << ligand << "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligLibDir << ligand << "/LIG.lib" <<std::endl;
        tleapFile << "COM = loadpdb Com_"<< poseID <<".pdb" << std::endl;
        tleapFile << "set default PBRadii mbondi2" << std::endl;
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
            std::string mesg="mmpbsa::receptor()\n\t Cannot open min file: "+minFName;
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
                << "  restraint_wt=100.0,\n" 
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
    double energy=0;
    bool success=pSanderOutput->getEAmber(sanderOut,energy);
    comGBen.push_back(energy);
    std::cout << "Complex GB Minimization Energy: " << energy <<" kcal/mol."<< std::endl;    
    
    if(!calcPB) return; //! return if not going to calculation PB energy.
    
    //! The following section is to calculate PB energy.
    minFName="Com_minPB.in";
    {
        std::ofstream minFile;
        try {
            minFile.open(minFName.c_str());
        }
        catch(...){
            std::string mesg="mmpbsa::runCom()\n\t Cannot open min file: "+minFName;
            throw LBindException(mesg);
        }   

        minFile << "title..\n" 
                << " &cntrl\n" 
                << "  imin   = 1,\n" 
                << "  ntmin   = 3,\n" 
                << "  maxcyc = 2000,\n" 
                << "  ncyc   = 1000,\n" 
                << "  ntpr   = 200,\n" 
                << "  ntb    = 0,\n" 
                << "  igb    = 10,\n" 
                << "  cut    = 15,\n" 
                << "  ntr=1,\n" 
                << "  restraint_wt=5.0,\n" 
                << "  restraintmask='!:LIG'\n"        
                << " /\n"
                << " &pb\n"
                << "  npbverb=0, npopt=2, epsout=80.0, radiopt=1, space=0.5,\n"
                << "  accept=1e-4, fillratio=6, sprob=1.6\n"
                << " / \n" << std::endl;
                
        minFile.close();    
    } 
    
    //bypass the PB calculation.
    cmd="cp "+sanderOut;
    sanderOut="Com_min_PB_"+Sstrm<std::string, int>(poseID)+".out";
//    cmd="sander -O -i Com_minPB.in -o "+sanderOut+" -p Com.prmtop -c Com_min.rst -ref Com_min.rst -x Com.mdcrd -r Com_min1.rst";
    cmd=cmd+" "+sanderOut;
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
       
    energy=0;
    success=pSanderOutput->getEAmber(sanderOut,energy);
    comPBen.push_back(energy);
    std::cout << "Complex PB Minimization Energy: " << energy <<" kcal/mol."<< std::endl;            
    
}

void SpMMPBSA::calBind(){
    for(unsigned i=0; i < comGBen.size(); ++i){
        double binding=comGBen[i]-recGBen-ligGBen;
        bindGBen.push_back(binding);
    }
    for(unsigned i=0; i < comPBen.size(); ++i){
        double binding=comPBen[i]-recPBen-ligPBen;
        bindPBen.push_back(binding);
    }    
}


void SpMMPBSA::run(const std::string& dir, const std::string& ligand){
    //! first get the GB PB energy of receptor and ligand.
    bool calcEn=this->energy(dir, ligand);    
    
    chdir(WORKDIR.c_str());
    chdir(dir.c_str());
        
    std::string pbsaDir="p_"+dir+"_mmpbsa";
    std::string cmd="mkdir -p "+pbsaDir;
    system(cmd.c_str());
    chdir(pbsaDir.c_str());
    
    cmd="mkdir -p "+ligand;
    system(cmd.c_str());
    chdir(ligand.c_str());
   
//    this->recRun(dir);
//    
//    this->ligRun(ligand);
    cmd="ln -s ../../m_"+dir+"_amber/Rec_min.pdb";
    system(cmd.c_str());
    
    //! Processing poses
    std::string poses="../../n_"+dir+"_vina/poses/"+ligand+".pdbqt";
    std::string posePDB=ligand+".pdb";
    
    cmd="obabel -ipdbqt "+poses+" -opdb -O "+posePDB;
    system(cmd.c_str());
    
    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    std::string outFileBase="Lig_";
    int numPose=pPdb->splitByModel(posePDB,outFileBase);
    
    std::cout << "Number of Poses: " << numPose << std::endl;       
         
    for(int i=1; i<=numPose; ++i){
        this->comRun(ligand, i);
    }
    
    //! calculate binding energies
    this->calBind();
}

void SpMMPBSA::getbindGB(std::vector<double>& bindgb){
    bindgb=bindGBen;
}

void SpMMPBSA::getbindPB(std::vector<double>& bindpb){
    bindpb=bindPBen;
}

} //namespace LBIND
