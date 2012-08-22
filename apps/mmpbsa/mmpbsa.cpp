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

#include <boost/scoped_ptr.hpp>

using namespace LBIND;

/*
 * 
 */

void recRun(std::string& dir){
    
    // RECEPTOR
    std::string pdbqtFile="../../n_"+dir+"_vina/"+dir+".pdbqt";
    std::string cmd="pdbqt_to_pdb.py -f " + pdbqtFile+" -o rec_qt.pdb";
    system(cmd.c_str());
    
    cmd="reduce -Trim rec_qt.pdb > rec_noh.pdb";
    system(cmd.c_str());
    
    cmd="reduce rec_noh.pdb > rec_rd.pdb";
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

        tleapFile << "source leaprc.ff99SB" << std::endl;
        tleapFile << "source leaprc.gaff" << std::endl;
        tleapFile << "REC = loadpdb rec_rd.pdb"<< std::endl;
        tleapFile << "saveamberparm REC REC.prmtop REC.inpcrd"<< std::endl;            
        tleapFile << "quit " << std::endl;

        tleapFile.close();
    }
    
    cmd="tleap -f rec_leap.in >& rec_leap.log";
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

        minFile << "title.." << std::endl;
        minFile << "&cntrl" << std::endl;
        minFile << "  imin   = 1," << std::endl;
        minFile << "  maxcyc = 2000," << std::endl;
        minFile << "  ncyc   = 1000," << std::endl;
        minFile << "  ntpr   = 200," << std::endl;
        minFile << "  ntb    = 0," << std::endl;
        minFile << "  igb    = 5," << std::endl;
        minFile << "  cut    = 15," << std::endl;
        minFile << "  ntr=1," << std::endl;
        minFile << "  restraint_wt=5.0," << std::endl;
        minFile << "  restraintmask='!@H='," << std::endl;       
        minFile << " /" << std::endl;

        minFile.close();    
    }
    
    cmd="sander -O -i Rec_min.in -o Rec_min.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());    
       
    cmd="ambpdb -p REC.prmtop < Rec_min.rst > Rec_min_0.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());      
    cmd="grep -v END Rec_min_0.pdb > Rec_min.pdb ";
}

void ligRun(std::string& ligand, std::string& ligLibDir){
        
    //LIGAND
    std::string cmd="ln -s "+ligLibDir+ligand+"/LIG.prmtop";
    system(cmd.c_str());
//    cmd="ln -s "+ligLibDir+ligand+"/LIG_min.rst";
//    system(cmd.c_str());  
}

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

        minFile << "title.." << std::endl;
        minFile << "&cntrl" << std::endl;
        minFile << "  imin   = 1," << std::endl;
        minFile << "  maxcyc = 2000," << std::endl;
        minFile << "  ncyc   = 1000," << std::endl;
        minFile << "  ntpr   = 200," << std::endl;
        minFile << "  ntb    = 0," << std::endl;
        minFile << "  igb    = 5," << std::endl;
        minFile << "  cut    = 15," << std::endl;
        minFile << "  ntr=1," << std::endl;
        minFile << "  restraint_wt=5.0," << std::endl;
        minFile << "  restraintmask='!:LIG'," << std::endl;          
        minFile << " /" << std::endl;

        minFile.close();    
    }          
    
    cmd="sander  -O -i Com_min.in -o Com_min.out  -p Com.prmtop -c Com.inpcrd -ref Com.inpcrd  -x Com_" 
            +Sstrm<std::string, int>(poseID)+".mdcrd"+" -r Com_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
        
    std::string pbsaFName="mmpbsa.in";
    {
        std::ofstream pbsaFile;
        try {
            pbsaFile.open(pbsaFName.c_str());
        }
        catch(...){
            std::string mesg="mmpbsa::receptor()\n\t Cannot open min file: "+pbsaFName;
            throw LBindException(mesg);
        }   

        pbsaFile << "Input file for running PB and GB" << std::endl;
        pbsaFile << "&general" << std::endl;
        pbsaFile << "   endframe=50, keep_files=2," << std::endl;
        pbsaFile << "/" << std::endl;
        pbsaFile << "&gb" << std::endl;
        pbsaFile << "  igb=2, saltcon=0.100," << std::endl;
        pbsaFile << "/" << std::endl;
        if(calcPB){
            pbsaFile << "&pb" << std::endl;
            pbsaFile << "  istrng=0.100," << std::endl;
            pbsaFile << "/" << std::endl;
        }
        pbsaFile.close();    
    } 
    cmd="MMPBSA.py -O -i mmpbsa.in -o mmpbsa_"+Sstrm<std::string, int>(poseID)
            +".dat -sp Com.prmtop -cp Com.prmtop -rp REC.prmtop -lp LIG.prmtop -y Com_"
            +Sstrm<std::string, int>(poseID)+".mdcrd";
    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());
    
}

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
    
    recRun(dir);
    
    std::string ligLibDir=WORKDIR+"/../ligLib/";
    
    ligRun(ligand,ligLibDir);
    
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

    std::string dir="1A02";
    std::string ligand="1";
    bool calcPB=true;
    mmpbsa(dir, ligand, calcPB);
    return 0;
}

