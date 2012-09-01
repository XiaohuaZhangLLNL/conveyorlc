/* 
 * File:   premmpbsa.cpp
 * Author: zhang30
 *
 * Created on August 13, 2012, 4:08 PM
 */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cstring>

#include "src/Parser/Sdf.h"
#include "src/Parser/Pdb.h"
#include "src/MM/Amber.h"
#include "src/Structure/Sstrm.hpp"
#include "src/Common/Tokenize.hpp"
#include "src/Common/LBindException.h"

#include <boost/scoped_ptr.hpp>

#include <mpi.h>

using namespace LBIND;

/*
 * 
 */
void premmpbsa(std::string& dir) {
    
    chdir(dir.c_str());

    std::string sdfFile="ligand.sdf";
    std::string pdb1File="ligand.pdb";
    
    std::string cmd="babel -isdf " + sdfFile + " -opdb " +pdb1File;
    std::cout << cmd << std::endl;
    system(cmd.c_str());

    std::string pdbFile="ligrn.pdb";
    std::string tmpFile="ligstrp.pdb";
    
    boost::scoped_ptr<Pdb> pPdb(new Pdb());    
    
    pPdb->renameAtom(pdb1File, pdbFile);
    
    pPdb->strip(pdbFile, tmpFile);
    
    cmd="prepare_ligand4.py -l "+pdbFile+" -A hydrogens";
    std::cout << cmd << std::endl;
    system(cmd.c_str());
    
    std::string keyword="PUBCHEM_TOTAL_CHARGE";
    
    boost::scoped_ptr<Sdf> pSdf(new Sdf());
    std::string info=pSdf->getInfo(sdfFile, keyword);
    
    std::cout << "Charge:" << info << std::endl;
    int charge=Sstrm<int, std::string>(info);
    std::string chargeStr=Sstrm<std::string,int>(charge);
    
    std::stringstream ss;
    
    ss << charge;

    std::string output="ligand.mol2";
    std::string options=" -c bcc -nc "+ chargeStr;
    
    
    boost::scoped_ptr<Amber> pAmber(new Amber());
    pAmber->antechamber(tmpFile, output, options);
    
    pAmber->parmchk(output);
    
    std::string ligName="LIG";
    std::string tleapFile="leap.in";
    
    pAmber->tleapInput(output,ligName,tleapFile);
    pAmber->tleap(tleapFile); 
    
    std::string minFName="LIG_min.in";
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
        minFile << " /" << std::endl;

        minFile.close();    
    }          
    
    cmd="sander  -O -i LIG_min.in -o LIG_min.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());      
    
    
    chdir("../");
    
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
            premmpbsa(dir);
           
        }
    }


    time=MPI_Wtime()-time;
    std::cout << "Rank= " << rank << " MPI Wall Time= " << time << std::endl;
    MPI_Finalize();
    
    
    return 0;
}
