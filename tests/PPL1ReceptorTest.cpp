/* 
 * File:   PPL1ReceptorTest.cpp
 * Author: zhang
 *
 * Created on March 24, 2014, 1:48 PM
 */

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <exception>

#include "Parser/Pdb.h"
#include "Parser/SanderOutput.h"
#include "MM/VinaLC.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Coor3d.h"
#include "Structure/Constants.h"
#include "Structure/Complex.h"
#include "Structure/Atom.h"
#include "Common/LBindException.h"
#include "Common/Tokenize.hpp"
#include "BackBone/Surface.h"
#include "BackBone/Grid.h"
#include "Structure/ParmContainer.h"
#include "Common/File.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"


#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

#include <mpi.h>

using namespace LBIND;

/*
 * 
 */

bool preReceptor(std::string& pdbFilePath, std::string& workDir, std::string& dataPath){
    
    bool jobStatus=false;
    
    if(!fileExist(pdbFilePath)){
        std::string mesg="PPL1Receptor::preReceptors()\n\t PDB file "+pdbFilePath+" doesn't exist\n";
        throw LBindException(mesg);  
        return jobStatus; 
    }
    
    std::string pdbBasename;
    getFileBasename(pdbFilePath, pdbBasename);
        
    std::string recDir=workDir+"/scratch/com/"+pdbBasename+"/rec";
    std::string cmd="mkdir -p "+recDir;
    system(cmd.c_str());  
    
    cmd="cp "+pdbFilePath+" "+recDir;
    system(cmd.c_str());  
    
    // cd to the rec directory to performance calculation
    chdir(recDir.c_str());
    
    std::string pdbFile;
    getPathFileName(pdbFilePath, pdbFile);
              
     //! begin energy minimization of receptor 
    cmd="reduce -Quiet -Trim  "+pdbFile+" >& rec_noh.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());
    
    std::string checkFName="rec_noh.pdb";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message);  
        return jobStatus;        
    }     
    
    cmd="reduce -Quiet -BUILD rec_noh.pdb -DB \""+dataPath+"/reduce_wwPDB_het_dict.txt\" >& rec_rd.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    

    checkFName="rec_rd.pdb";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        return jobStatus;        
    }  
    
    {
        std::string stdPdbFile="rec_std.pdb";
        boost::scoped_ptr<Pdb> pPdb(new Pdb() );
        pPdb->standardlize(checkFName, stdPdbFile);
    }
        
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
                << "  gbsa   = 1,\n"
                << "  cut    = 15,\n" 
                << "  ntr=1,\n" 
                << "  restraint_wt=5.0,\n" 
                << "  restraintmask='!@H=',\n"        
                << " /\n" << std::endl;
        minFile.close();    
    }
    
    cmd="sander -O -i Rec_minGB.in -o Rec_minGB.out  -p REC.prmtop -c REC.inpcrd -ref REC.inpcrd -x REC.mdcrd -r Rec_min.rst";
    std::cout <<cmd <<std::endl;
//    system(cmd.c_str());  
    
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    std::string sanderOut="Rec_minGB.out";
    double recGBen=0;
    bool success=pSanderOutput->getEnergy(sanderOut, recGBen);
    
    if(!success){
        std::string message="Receptor GB minimization fails.";
        throw LBindException(message); 
        return jobStatus;          
    }
    
       
    cmd="ambpdb -p REC.prmtop < Rec_min.rst > Rec_min_0.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  

    checkFName="Rec_min_0.pdb";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message); 
        return jobStatus;        
    }  
    
    cmd="grep -v END Rec_min_0.pdb > Rec_min.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    cmd="prepare_receptor4.py -r Rec_min.pdb -o "+pdbBasename+".pdbqt";
    system(cmd.c_str());
    
    checkFName=pdbBasename+".pdbqt";
    if(!fileExist(checkFName)){
        std::string message=checkFName+" does not exist.";
        throw LBindException(message);  
        return jobStatus;        
    } 
    
    // Get geometry
    std::string stdPDBfile="rec_std.pdb";    
    boost::scoped_ptr<Complex> pComplex(new Complex());
    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    pPdb->parse(stdPDBfile, pComplex.get());

    boost::scoped_ptr<ParmContainer> pParmContainer(new ParmContainer());
    ElementContainer* pElementContainer = pParmContainer->addElementContainer();
    pComplex->assignElement(pElementContainer);
        

    boost::scoped_ptr<Surface> pSurface(new Surface(pComplex.get()));
    std::cout << "Start Calculation " << std::endl;
    pSurface->run(1.4, 960);
    std::cout << " Total SASA is: " << pSurface->getTotalSASA() << std::endl << std::endl;

    boost::scoped_ptr<Grid> pGrid(new Grid(pComplex.get()));
    pGrid->run(1.4, 100, 50);
    Coor3d dockDim;
    Coor3d centroid;  
    pGrid->getTopSiteGeo(dockDim, centroid);
    
    std::ofstream outFile;
    outFile.open("rec_geo.txt");
    outFile << centroid.getX() << " " << centroid.getY() << " " << centroid.getZ() << " " 
             << dockDim.getX() << " " << dockDim.getY() << " "  << dockDim.getZ() << "\n";
    outFile.close();
    delete pElementContainer;
    
    //END    
    jobStatus=true;
    return jobStatus;
}

int main(int argc, char** argv) {
    
    //! get  working directory
    char* WORKDIR=getenv("WORKDIR");
    std::string workDir;
    if(WORKDIR==0) {
        // use current working directory for working directory
        char BUFFER[200];
        getcwd(BUFFER, sizeof (BUFFER));
        workDir = BUFFER;        
    }else{
        workDir=WORKDIR;
    }
    //! get LBindData
    char* LBINDDATA=getenv("LBindData");

    if(LBINDDATA==0){
        std::cerr << "LBdindData environment is not defined!" << std::endl;
        return 1;
    }  

    try {
        std::string dataPath = LBINDDATA;
        std::string dir = "pdb/3BKL.pdb";
        bool jobStatus = preReceptor(dir, workDir, dataPath);
    } catch (LBindException& e) {
        std::cerr << "LBInd Exception: " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Std Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "UnKnown Error"<< std::endl;
    }
    
    return 0;
}

