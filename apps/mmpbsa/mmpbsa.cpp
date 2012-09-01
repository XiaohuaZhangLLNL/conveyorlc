/* 
 * File:   mmpbsa.cpp
 * Author: zhang30
 *
 * Created on August 16, 2012, 10:52 AM
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>


#include "src/MM/SpMMPBSA.h"
#include "src/Common/Tokenize.hpp"

#include <boost/scoped_ptr.hpp>

#include <mpi.h>

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



int mmpbsa(std::string& dir, std::string& ligand, bool calcPB) {
    
    boost::scoped_ptr<SpMMPBSA> pSpMMPBSA(new SpMMPBSA(calcPB));
    pSpMMPBSA->run(dir, ligand);
  
    std::string outFileName=dir+"-"+ligand+".txt";
    
    std::ofstream outFile;
    try {
        outFile.open(outFileName.c_str());
    }
    catch(...){
        std::cout << "mmpbsa >> Cannot open file" << outFileName << std::endl;
    }
    
    std::vector<double> bindGB;
    pSpMMPBSA->getbindGB(bindGB); 
    
    outFile <<"MM-GBSA:" << std::endl;
    for(unsigned i=0; i < bindGB.size(); ++i){
        outFile <<"Pose " << i << ": " <<bindGB[i] << "kcal/mol" << std::endl;
    }
    
    std::vector<double> bindPB;
    pSpMMPBSA->getbindPB(bindPB);
       
    outFile <<"\nMM-PBSA:" << std::endl;
    for(unsigned i=0; i < bindPB.size(); ++i){
        outFile <<"Pose " << i << ": " <<bindPB[i] << "kcal/mol" << std::endl;
    }
    
    outFile.close();
      
    return 0;
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
    int pbFlag;
    char dirBuffer[100];
    char ligBuffer[100];
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
    
    std::cout << "Number of tasks= " << nproc << " My rank= " << rank << std::endl;

    if (rank == 0) {
        if(argc <3){
            std::cerr << "Usage: amberTools <dirListFileName> <ligListFileName>" << std::endl;
            return 1;        
        }  
        
        std::string recListFName=argv[1];
        std::string ligListFName=argv[2];

        jobInput.pbFlag=1;
        if(argc >3){
            jobInput.pbFlag=atoi(argv[3]);
        }

     
        std::vector<std::string> dirList;               
        saveStrList(recListFName, dirList);
        
        std::vector<std::string> ligList;               
        saveStrList(ligListFName, ligList);        
           
        for(unsigned i=0; i<dirList.size(); ++i){
            for(unsigned j=0; j<ligList.size(); ++j){
                std::cout << "Working on receptor: " << dirList[i]  << " Ligand: " << ligList[j] << std::endl;
                int freeProc;
                MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
                MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 

                strcpy(jobInput.dirBuffer, dirList[i].c_str());
                strcpy(jobInput.ligBuffer, ligList[j].c_str());

                MPI_Send(&jobInput, sizeof(JobInputData), MPI_CHAR, freeProc, inpTag, MPI_COMM_WORLD);
            }
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
            bool calcPB =true;            
            std::string dir=jobInput.dirBuffer;
            std::string lig=jobInput.ligBuffer;
            
            if(jobInput.pbFlag ==0) calcPB = false;
            
            mmpbsa(dir, lig, calcPB);
           
        }
    }


    time=MPI_Wtime()-time;
    std::cout << "Rank= " << rank << " MPI Wall Time= " << time << std::endl;
    MPI_Finalize();
    
    
    return 0;
}
