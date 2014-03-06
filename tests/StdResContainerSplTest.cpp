/* 
 * File:   StdResContainerSplTest.cpp
 * Author: zhang30
 *
 * Created on September 5, 2012, 5:34 PM
 */

#include <cstdlib>
#include <iostream>
#include "Structure/StdResContainer.h"
#include "Structure/Coor3d.h"
#include "Parser/Pdb.h"
#include "Parser/SanderOutput.h"
#include "MM/VinaLC.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Constants.h"
#include "Common/File.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"
/*
 * 
 */

using namespace LBIND;

void testStdResContainer(){
    StdResContainer* pStdResContainer=new StdResContainer();
    if(pStdResContainer->find("MET")){
        std::cout << "MET is standard residue" << std::endl;
    }
    
    delete pStdResContainer;    
}

void testPDBfixElement(char** argv){
    Pdb* pPdb=new Pdb();
    std::string input=argv[1];
    std::cout << "input=" << input << std::endl;
//    pPdb->standardlize(input, output);
    pPdb->fixElement(input, "test-fix.pdb");    
}

void testPDBstandardlize(char** argv){
    Pdb* pPdb=new Pdb();
    std::string input=argv[1];
    std::string output=argv[2];
    std::cout << "input=" << input << " output=" << output << std::endl;
//    pPdb->standardlize(input, output);
    pPdb->standardlize2(input, "test-out.pdb");
    std::string cmd="reduce -Trim test-out.pdb> test-noh.pdb";
    system(cmd.c_str());

    cmd="reduce -BUILD test-noh.pdb> test-rd.pdb";
    system(cmd.c_str());
    
    pPdb->standardlize("test-rd.pdb", output);
    
}

void testRun(char** argv) {
    Coor3d coor1(1,1,1);
    Coor3d coor2(2,2,2);
    std::cout << coor1.dist2(coor2) << std::endl;
    testStdResContainer();
    testPDBstandardlize(argv);
}

bool preReceptors(std::string& dir, bool getPDBflg){
    bool jobStatus=true;
    
    std::string WORKDIR=getenv("WORKDIR");
    chdir(WORKDIR.c_str());
        
    chdir(dir.c_str());
    std::string siteDir="../c_"+dir+"_sites/";
    std::string amberDir="m_"+dir+"_amber";
    std::string cmd="mkdir -p "+amberDir;
    system(cmd.c_str());
    
    chdir(amberDir.c_str());
    
    cmd="pwd";
    system(cmd.c_str());

    std::string pdbFile=dir+".pdb";
    std::string checkFName=pdbFile;
    if(!fileExist(checkFName)){
        cmd="curl http://www.rcsb.org/pdb/files/"+pdbFile+" -o "+pdbFile;
        system(cmd.c_str());       
    }

    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    }    

    boost::scoped_ptr<Pdb> pPdb(new Pdb()); 
    std::string pdbOutFile=dir+"-out.pdb";
    pPdb->standardlize2(pdbFile, pdbOutFile);   
    
    
    std::string csaPdbFile=siteDir+dir+"_00_ref_cent_1.pdb";

    checkFName=csaPdbFile;
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    }    
        
    boost::scoped_ptr<VinaLC> pVinaLC(new VinaLC());
    
    std::string sumFile=siteDir+dir+"_00_ref.sum";
    Coor3d center;
    pVinaLC->centroid(sumFile,center);
//    std::cout << "Center coordinates: X=" << center.getX() 
//              << " Y=" << center.getY()
//              << " Z=" << center.getZ() 
//              << std::endl;

      
    std::string cutPdbFile=dir+"_00_ref_cut.pdb";
    pPdb->cutByRadius(csaPdbFile, cutPdbFile, center, 20);
    
    
    Coor3d gridDims;
    pVinaLC->vinalcGridDims(cutPdbFile, center, gridDims);

//    std::cout << "Box Dimension: X=" << gridDims.getX() 
//              << " Y=" << gridDims.getY()
//              << " Z=" << gridDims.getZ() 
//              << std::endl;   
    
    std::string geoFileName="geo.txt";
    std::ofstream geoFile;
    try {
        geoFile.open(geoFileName.c_str());
        geoFile << center.getX() << "  " 
                << center.getY() << "  " 
                << center.getZ() << "  " 
                << gridDims.getX() << "  " 
                << gridDims.getY() << "  " 
                << gridDims.getZ() << "  " 
                << std::endl;
    }
    catch(...){
        std::string mesg="PreVinaLC::sphgen()\n\t Cannot open geoFile input file: "+geoFileName;
        throw LBindException(mesg);
    }     
       
    geoFile.close();
       
    //! begin energy minimization of receptor 
    cmd="reduce -Quiet -Trim  "+pdbOutFile+" >& rec_noh.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());
    
    checkFName="rec_noh.pdb";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    }     
    
    cmd="reduce -Quiet -BUILD rec_noh.pdb >& rec_rd.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());        

    checkFName="rec_rd.pdb";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    }  

    std::string stdPdbFile="rec_std.pdb";
    pPdb->standardlize(checkFName, stdPdbFile);

        
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
                  << "REC = loadpdb rec_std.pdb\n"
                  << "saveamberparm REC REC.prmtop REC.inpcrd\n"
                  << "quit\n";
        
        tleapFile.close();
    }
    
    cmd="tleap -f rec_leap.in >& rec_leap.log";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());

    std::string minFName="Rec_minGB.in";
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
    
    cmd="sander -O -i Rec_minGB.in -o Rec_minGB.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    std::string sanderOut="Rec_minGB.out";
    double recGBen=0;
    bool success=pSanderOutput->getEnergy(sanderOut, recGBen);
    
    if(!success){
        std::string message="Receptor 1st GB minimization fails.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;          
    }
    
    minFName="Rec_minGB2.in";
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
    
    cmd="sander -O -i Rec_minGB2.in -o Rec_minGB2.out  -p REC.prmtop -c Rec_min.rst -ref Rec_min.rst -x REC.mdcrd -r Rec_min2.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    
    sanderOut="Rec_minGB2.out";
    recGBen=0;
    success=pSanderOutput->getEnergy(sanderOut,recGBen);
    std::cout << "Receptorn GB Minimization Energy: " << recGBen <<" kcal/mol."<< std::endl;

    if(!success){
        std::string message="Receptor 2nd GB minimization fails.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;          
    }    
    
    cmd="ambpdb -p REC.prmtop < Rec_min2.rst > Rec_min_0.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  

    checkFName="Rec_min_0.pdb";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    }  
    
    cmd="grep -v END Rec_min_0.pdb > Rec_min.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    cmd="prepare_receptor4.py -r Rec_min.pdb -o "+dir+".pdbqt";
    system(cmd.c_str());
    
    checkFName=dir+".pdbqt";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    }      
        
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
                << " /\n"
                << " &pb\n"
                << "  npbverb=0, epsout=80.0, radiopt=1, space=0.5,\n"
                << "  accept=1e-4, fillratio=6, sprob=1.6\n"
                << " / \n" << std::endl;
                
        minFile.close();    
    } 
    cmd="sander -O -i Rec_minPB.in -o Rec_minPB.out -p REC.prmtop -c Rec_min2.rst -ref Rec_min2.rst -x REC.mdcrd -r Rec_min3.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
       
    sanderOut="Rec_minPB.out";
    double recPBen=0;
    success=pSanderOutput->getEnergy(sanderOut,recPBen);
    std::cout << "Receptor PB Minimization Energy: " << recPBen <<" kcal/mol."<< std::endl;
    if(!success){
        std::string message="Receptor PB minimization fails.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;          
    }    
    
    return true;
}


int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% StdResContainerSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRun (StdResContainerSplTest)" << std::endl;
//    testRun(argv);
    std::string dir="1A3Q";
    bool getPDBflg=true;
    try{
//        bool sucess=preReceptors(dir, getPDBflg);
        testPDBfixElement(argv);
    }catch(LBindException &e){
        std::cout << "Error: " << e.what() << std::endl;
    }
    std::cout << "%TEST_FINISHED% time=0 testRun (StdResContainerSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}
