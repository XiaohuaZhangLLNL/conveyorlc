/* 
 * File:   preReceptors.cpp
 * Author: zhang30
 *
 * Created on August 16, 2012, 1:28 PM
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "src/Parser/Pdb.h"
#include "src/Parser/SanderOutput.h"
#include "src/MM/VinaLC.h"
#include "src/Structure/Sstrm.hpp"
#include "src/Structure/Coor3d.h"
#include "src/Common/Tokenize.hpp"
#include "src/Common/LBindException.h"

#include <boost/scoped_ptr.hpp>

#include <mpi.h>

using namespace LBIND;

/*!
 * \breif preReceptors calculation receptor grid dimension from CSA sitemap output
 * \param argc
 * \param argv argv[1] takes the input file name
 * \return success 
 * \defgroup preReceptors_Commands preReceptors Commands
 *
 * 
 * Usage on HPC slurm
 * 
 * \verbatim
 
    export AMBERHOME=/usr/gapps/medchem/amber/amber12
    export PATH=$AMBERHOME/bin/:$PATH
    export WORKDIR=`pwd`/workspace/

    srun -N4 -n48 -ppdebug /g/g92/zhang30/medchem/NetBeansProjects/MedCM/apps/mmpbsa/preReceptors  <input-file>

    <input-file>: contain a list of receptor subdirectory names.

    Requires: define WORKDIR 
   \endverbatim
 */

void preReceptors(std::string& dir){
    
    std::string WORKDIR=getenv("WORKDIR");
    chdir(WORKDIR.c_str());
    chdir(dir.c_str());
    std::string siteDir="../c_"+dir+"_sites/";
    std::string vinaDir="n_"+dir+"_vina";
    std::string cmd="mkdir -p "+vinaDir;
    system(cmd.c_str());
    
    chdir(vinaDir.c_str());
    
    std::string csaPdbFile=siteDir+dir+"_00_ref_cent_1.pdb";
        
    boost::scoped_ptr<VinaLC> pVinaLC(new VinaLC());
    
    std::string sumFile=siteDir+dir+"_00_ref.sum";
    Coor3d center;
    pVinaLC->centroid(sumFile,center);
    std::cout << "Center coordinates: X=" << center.getX() 
              << " Y=" << center.getY()
              << " Z=" << center.getZ() 
              << std::endl;

    boost::scoped_ptr<Pdb> pPdb(new Pdb());    
    
    std::string cutPdbFile=dir+"_00_ref_cut.pdb";
    pPdb->cutByRadius(csaPdbFile, cutPdbFile, center, 20);
    
    
    Coor3d gridDims;
    pVinaLC->vinalcGridDims(cutPdbFile, center, gridDims);

    std::cout << "Box Dimension: X=" << gridDims.getX() 
              << " Y=" << gridDims.getY()
              << " Z=" << gridDims.getZ() 
              << std::endl;   
    
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
    cmd="reduce -Quiet -Trim "+csaPdbFile+" > rec_noh.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());
    
    cmd="reduce -Quiet -BUILD rec_noh.pdb > rec_rd.pdb";
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
    
    cmd="sander -O -i Rec_minGB2.in -o Rec_minGB2.out  -p Rec_min.rst -c Rec_min.rst -ref REC.inpcrd -x REC.mdcrd -r Rec_min2.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    
    std::string sanderOut="Rec_minGB2.out";
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    double recGBen=pSanderOutput->getEAmber(sanderOut);
    std::cout << "Receptorn GB Minimization Energy: " << recGBen <<" kcal/mol."<< std::endl;
       
    cmd="ambpdb -p REC.prmtop < Rec_min2.rst > Rec_min_0.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());      
    cmd="grep -v END Rec_min_0.pdb > Rec_min.pdb ";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());  
    
    cmd="prepare_receptor4.py -r Rec_min.pdb -o "+dir+".pdbqt";
    system(cmd.c_str());
        
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
    double recPBen=pSanderOutput->getEAmber(sanderOut);
    std::cout << "Receptor PB Minimization Energy: " << recPBen <<" kcal/mol."<< std::endl;
}

void saveStrList(std::string& fileName, std::vector<std::string>& strList){
    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Cannot open file: " << fileName << std::endl;
    } 

    std::string fileLine;
    while(inFile){
        std::getline(inFile, fileLine);
        std::vector<std::string> tokens;
        tokenize(fileLine, tokens); 
        if(tokens.size() > 0){
            strList.push_back(tokens[0]);
        }        
    }
    
}

struct JobInputData{        
    char dirBuffer[100];
};

int main(int argc, char** argv) {
    
    int nproc, rank, rc;

    int jobFlag=1; // 1: doing job,  0: done job
    
    JobInputData jobInput;
    
    MPI_Status status1, status2;
        
    int rankTag=1;
    int jobTag=2;

    int inpTag=3;


    rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS) {
        std::cerr << "Error starting MPI program. Terminating.\n";
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
//    MPI_Barrier(MPI_COMM_WORLD);
    double time=MPI_Wtime();

    if (nproc < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        return 1;
    }
    
    std::string inputFName=argv[1];
    
    if(inputFName.size()==0){
        std::cerr << "Usage: amberTools <dirListFileName>" << std::endl;
        return 1;        
    }

    std::cout << "Number of tasks= " << nproc << " My rank= " << rank << std::endl;

    if (rank == 0) {
       
        std::vector<std::string> dirList;        
        
        saveStrList(inputFName, dirList);
           
        for(unsigned i=0; i<dirList.size(); ++i){
            std::cout << "Working on " << dirList[i] << std::endl;
            int freeProc;
            MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 

            strcpy(jobInput.dirBuffer, dirList[i].c_str());

            MPI_Send(&jobInput, sizeof(JobInputData), MPI_CHAR, freeProc, inpTag, MPI_COMM_WORLD);

        }
        
        for(unsigned i=1; i < nproc; ++i){
            int freeProc;
            MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
            jobFlag=0;;
            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD);            
        }        
        
    }else {
        while (1) {
            MPI_Send(&rank, 1, MPI_INTEGER, 0, rankTag, MPI_COMM_WORLD);
            MPI_Recv(&jobFlag, 20, MPI_CHAR, 0, jobTag, MPI_COMM_WORLD, &status2);
            if (jobFlag==0) {
                break;
            }
            // Receive parameters

            MPI_Recv(&jobInput, sizeof(JobInputData), MPI_CHAR, 0, inpTag, MPI_COMM_WORLD, &status1);
                        
            std::string dir=jobInput.dirBuffer;
            preReceptors(dir);
           
        }
    }


    time=MPI_Wtime()-time;
    std::cout << "Rank= " << rank << " MPI Wall Time= " << time << std::endl;
    MPI_Finalize();
    
    
    return 0;
}
