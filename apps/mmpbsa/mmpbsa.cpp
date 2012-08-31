/* 
 * File:   mmpbsa.cpp
 * Author: zhang30
 *
 * Created on August 16, 2012, 10:52 AM
 */

#include <cstdlib>
#include <iostream>
#include <sstream>

#include "src/Parser/Sdf.h"
#include "src/Parser/Pdb.h"
#include "src/MM/Amber.h"
#include "src/Structure/Sstrm.hpp"
#include "src/Common/Tokenize.hpp"
#include "src/Common/LBindException.h"
#include "src/Parser/SanderOutput.h"

#include <boost/scoped_ptr.hpp>

using namespace LBIND;

/*!
 * \breif mmpbsa MM-PB(GB)SA calculations on HPC using amber forcefield
 * \param argc
 * \param argv
 * \return success 
 * \defgroup mmpbsa_Commands mmpbsa Commands
 * 
 * Usage: mmpbsa <input-file>
 */


/**
 * \breif recRun prepare pdbqt, tleap and run GB/PB minimization for receptor
 * \param dir the directory/receptor name under the WORKDIR
 * \param calcPB flag to switch between PB and GB calculation
 * 
 
 */
void recRun(std::string& dir, bool calcPB){
    
    // RECEPTOR
    std::string pdbqtFile="../../n_"+dir+"_vina/"+dir+".pdbqt";
    std::string cmd="pdbqt_to_pdb.py -f " + pdbqtFile+" -o rec_qt.pdb";
    system(cmd.c_str());
    
    cmd="reduce -Trim rec_qt.pdb > rec_noh.pdb >& reduce.log";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());
    
    cmd="reduce rec_noh.pdb > rec_rd.pdb>& reduce.log";
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
        
        tleapFile << "source leaprc.ff99SB\n"                
                  << "source leaprc.gaff\n"
                  << "REC = loadpdb rec_rd.pdb\n"
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
    double energy=pSanderOutput->getEAmber(sanderOut);
    std::cout << "Receptorn GB Minimization Energy: " << energy <<" kcal/mol."<< std::endl;
       
    cmd="ambpdb -p REC.prmtop < Rec_min.rst > Rec_min_0.pdb";
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
    energy=pSanderOutput->getEAmber(sanderOut);
    std::cout << "Receptor PB Minimization Energy: " << energy <<" kcal/mol."<< std::endl;    
}

/**
 * \breif ligRun prepare parameter files and run GB/PB minimization for ligand
 * \param ligand the ligand name under the ligLibDir
 * \param ligLibDir directory (WORKDIR/../ligLib) to save ligand parameter files 
 * \param calcPB flag to switch between PB and GB calculation
 * 
 
 */

void ligRun(std::string& ligand, std::string& ligLibDir, bool calcPB){
        
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
                << "  cut    = 15,\n"       
                << " /\n" << std::endl;
        minFile.close();    
    }
    
    cmd="sander -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG_min.rst -ref LIG_min.rst -x REC.mdcrd -r LIG_min0.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    
    std::string sanderOut="LIG_minGB.out";
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    double energy=pSanderOutput->getEnergy(sanderOut);
    std::cout << "LIGAND GB Minimization Energy: " << energy <<" kcal/mol."<< std::endl;
       
    
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
    energy=pSanderOutput->getEnergy(sanderOut);
    std::cout << "Ligand GB Minimization Energy: " << energy <<" kcal/mol."<< std::endl;        
}

/**
 * \breif comRun prepare tleap and run GB/PB minimization for complex
 * \param ligand the ligand name under the ligLibDir
 * \param poseID the pose ID (Model number in Vina pdbqt output)
 * \param ligLibDir directory (WORKDIR/../ligLib) to save ligand parameter files
 * \param calcPB flag to switch between PB and GB calculation
 * 
 
 */

void ComRun(std::string& ligand, int poseID, std::string& ligLibDir, bool calcPB){
    
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

        tleapFile << "source leaprc.ff99SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "loadamberparams  " << ligLibDir << ligand << "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligLibDir << ligand << "/LIG.lib" <<std::endl;
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
            std::string mesg="mmpbsa::receptor()\n\t Cannot open tleap file: "+tleapFName;
            throw LBindException(mesg);
        }   

        tleapFile << "source leaprc.ff99SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "loadamberparams  " << ligLibDir << ligand << "/ligand.frcmod" <<std::endl;
        tleapFile << "loadoff " << ligLibDir << ligand << "/LIG.lib" <<std::endl;
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
                << "  restraintmask='!:LIG'\n"        
                << " /\n" << std::endl;
        
        minFile.close();    
    }          
    
    cmd="sander  -O -i Com_min.in -o Com_min.out  -p Com.prmtop -c Com.inpcrd -ref Com.inpcrd  -x Com_" 
            +Sstrm<std::string, int>(poseID)+".mdcrd"+" -r Com_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  

    std::string sanderOut="Com_min.out";
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    double energy=pSanderOutput->getEAmber(sanderOut);
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
                << "  npbverb=0, epsout=80.0, radiopt=1, space=0.5,\n"
                << "  accept=1e-4, fillratio=6, sprob=1.6\n"
                << " / \n" << std::endl;
                
        minFile.close();    
    } 
    cmd="sander -O -i Com_minPB.in -o Com_minPB.out -p Com.prmtop -c Com_min.rst -ref Com_min.rst -x Com.mdcrd -r Com_min1.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
       
    sanderOut="Com_minPB.out";
    energy=pSanderOutput->getEAmber(sanderOut);
    std::cout << "Complex PB Minimization Energy: " << energy <<" kcal/mol."<< std::endl;            
    
}

/**
 * \breif mmpbsa run re-scoring for one receptor together with one ligand
 * \param dir the directory/receptor name under the WORKDIR
 * \param ligand the ligand name under the ligLibDir
 * \param calcPB flag to switch between PB and GB calculation
 * 
 */

void mmpbsa(std::string& dir, std::string& ligand, bool calcPB){
    std::string WORKDIR=getenv("WORKDIR");
    chdir(WORKDIR.c_str());
    chdir(dir.c_str());
        
    std::string pbsaDir="p_"+dir+"_mmpbsa";
    std::string cmd="mkdir -p "+pbsaDir;
    system(cmd.c_str());
    chdir(pbsaDir.c_str());
    
    cmd="mkdir -p "+ligand;
    system(cmd.c_str());
    chdir(ligand.c_str());
    
    recRun(dir,calcPB);
    
    std::string ligLibDir=WORKDIR+"/../ligLib/";
    
    ligRun(ligand,ligLibDir,calcPB);
    
    //Processing poses
    std::string poses="../../n_"+dir+"_vina/poses/"+ligand+".pdbqt";
    std::string posePDB=ligand+".pdb";
    
    cmd="babel -ipdbqt "+poses+" -opdb "+posePDB;
    system(cmd.c_str());
    
    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    std::string outFileBase="Lig_";
    int numPose=pPdb->splitByModel(posePDB,outFileBase);
    
    std::cout << "Number of Poses: " << numPose << std::endl;       
    numPose=1;
         
    for(int i=1; i<=numPose; ++i){
        ComRun(ligand, i, ligLibDir, calcPB);
    }
    
}

int main(int argc, char** argv) {

    std::string dir="1EDM";
    std::string ligand="206";
    bool calcPB=true;
    mmpbsa(dir, ligand, calcPB);
    return 0;
}

