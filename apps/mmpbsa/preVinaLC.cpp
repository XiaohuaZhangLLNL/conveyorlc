/* 
 * File:   preVinaLC.cpp
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
#include "src/MM/VinaLC.h"
#include "src/Structure/Sstrm.hpp"
#include "src/Structure/Coor3d.h"
#include "src/Common/Tokenize.hpp"
#include "src/Common/LBindException.h"

#include <boost/scoped_ptr.hpp>

#include <mpi.h>

using namespace LBIND;

void preVinaLC(std::string& dir){
    
    std::string WORKDIR=getenv("WORKDIR");
    chdir(WORKDIR.c_str());
    chdir(dir.c_str());
    std::string siteDir="../c_"+dir+"_sites/";
    std::string vinaDir="n_"+dir+"_vina";
    std::string cmd="mkdir -p "+vinaDir;
    system(cmd.c_str());
    
    chdir(vinaDir.c_str());
    
    std::string csaPdbFile=siteDir+dir+"_00_ref_cent_1.pdb";
    
    cmd="prepare_receptor4.py -r "+csaPdbFile+" -o "+dir+".pdbqt";
    system(cmd.c_str());
    
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
    
    
    return;
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
            preVinaLC(dir);
           
        }
    }


    time=MPI_Wtime()-time;
    std::cout << "Rank= " << rank << " MPI Wall Time= " << time << std::endl;
    MPI_Finalize();
    
    
    return 0;
}
