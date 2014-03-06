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


#include "MM/SpMMPBSA.h"
#include "Common/Tokenize.hpp"
#include "Structure/Constants.h"
#include "Common/File.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"

#include "mmpbsaPO.h"

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



bool mmpbsa(std::string& dir, std::string& ligand, bool calcPB) {
    
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
    
    double minEn=9999999.0;
    int minID=0;
    
    outFile <<"MM-GBSA:" << std::endl;
    for(unsigned i=0; i < bindGB.size(); ++i){
        outFile <<"Pose " << i+1 << ": " <<bindGB[i] << " kcal/mol" << std::endl;
        if(bindGB[i]<minEn){
            minEn=bindGB[i];
            minID=i+1;
        }
    }
    
    outFile <<"Minimum Energy " << minID << ": " <<minEn << " kcal/mol" << std::endl;
    
    std::vector<double> bindPB;
    pSpMMPBSA->getbindPB(bindPB);

    minEn=9999999.0;
    minID=0;    
    
    outFile <<"\nMM-PBSA:" << std::endl;
    for(unsigned i=0; i < bindPB.size(); ++i){
        outFile <<"Pose " << i+1 << ": " <<bindPB[i] << " kcal/mol" << std::endl;
        if(bindPB[i]<minEn){
            minEn=bindPB[i];
            minID=i+1;
        }
    }

    outFile <<"Minimum Energy " << minID << ": " <<minEn << " kcal/mol" << std::endl;    
    
    outFile.close();
      
    return true;
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

struct JobOutData{  
    bool error;
    char dirBuffer[100];
    char ligBuffer[100];
    char message[100];
};


int main(int argc, char** argv) {
    
    int nproc, rank, rc;

    int jobFlag=1; // 1: doing job,  0: done job
    
    JobInputData jobInput;
    JobOutData jobOut;
    
    MPI_Status status1, status2;
        
    int rankTag=1;
    int jobTag=2;

    int inpTag=3;
    int outTag=4;

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
    
    POdata podata;
    int error=0;
    
    if (rank == 0) {        
        bool success=mmpbsaPO(argc, argv, podata);
        if(!success){
            error=1;           
        }        
    }

    MPI_Bcast(&error, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (error !=0) {
        MPI_Finalize();
        return 1;
    }    
    
    std::cout << "Number of tasks= " << nproc << " My rank= " << rank << std::endl;

    if (rank == 0) {
        
        //! Tracking error using XML file
	XMLDocument doc;  
 	XMLDeclaration* decl = new XMLDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	XMLElement * root = new XMLElement( "Errors" );  
	doc.LinkEndChild( root );  

	XMLComment * comment = new XMLComment();
	comment->SetValue(" Tracking calculation error using XML file " );  
	root->LinkEndChild( comment );  
        
        std::string xmlGoodFName="JobTrackingGood.xml";
        std::string xmlBadFName="JobTrackingBad.xml";
        std::string xmlFName="JobTracking.xml";
        if(podata.xmlFile.length()>0){
            xmlGoodFName=podata.xmlFile+"-Good.xml";
            xmlBadFName=podata.xmlFile+"-Bad.xml"; 
            xmlFName=podata.xmlFile+".xml";
        }
        FILE* xmlGoodFile=fopen(xmlGoodFName.c_str(), "w"); 
        FILE* xmlBadFile=fopen(xmlBadFName.c_str(), "w"); 
        //! END of XML header
        

        jobInput.pbFlag=1;
        if(!podata.pbFlag){
            jobInput.pbFlag=0;
        }

     
        std::vector<std::string> dirList;               
        saveStrList(podata.recFile, dirList);
        
        std::vector<std::string> ligList;               
        saveStrList(podata.ligFile, ligList); 
        
        int count=0;
           
        for(unsigned i=0; i<dirList.size(); ++i){
            for(unsigned j=0; j<ligList.size(); ++j){
                if(count >nproc-1){
                    MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
                    XMLElement * element = new XMLElement("error"); 
                    element->SetAttribute("receptor", jobOut.dirBuffer);
                    element->SetAttribute("ligand", jobOut.ligBuffer);
                    element->SetAttribute("mesg", jobOut.message);  
                    root->LinkEndChild(element);
                    if(!jobOut.error){
                        element->Print(xmlBadFile,2);
                        fputs("\n",xmlBadFile);
                        fflush(xmlBadFile);                      
                    }else{
                        element->Print(xmlGoodFile,2);
                        fputs("\n",xmlGoodFile);
                        fflush(xmlGoodFile);                    
                    }

                } 
            
                int freeProc;
                MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
                std::cout << "At Process: "<< freeProc << " Working on receptor: " << dirList[i]  << " Ligand: " << ligList[j] << std::endl;
                MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 

                strcpy(jobInput.dirBuffer, dirList[i].c_str());
                strcpy(jobInput.ligBuffer, ligList[j].c_str());

                MPI_Send(&jobInput, sizeof(JobInputData), MPI_CHAR, freeProc, inpTag, MPI_COMM_WORLD);
                
                ++count;
            }
        }
        
        int nJobs=count;
        int ndata=(nJobs<nproc-1)? nJobs: nproc-1;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(unsigned i=0; i < ndata; ++i){
            MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
            XMLElement * element = new XMLElement("error"); 
            element->SetAttribute("receptor", jobOut.dirBuffer);
            element->SetAttribute("ligand", jobOut.ligBuffer);
            element->SetAttribute("mesg", jobOut.message);  
            root->LinkEndChild(element);
            if(!jobOut.error){
                element->Print(xmlBadFile,2);
                fputs("\n",xmlBadFile);
                fflush(xmlBadFile);                      
            }else{
                element->Print(xmlGoodFile,2);
                fputs("\n",xmlGoodFile);
                fflush(xmlGoodFile);                    
            }
        } 
        
        doc.SaveFile( xmlFName.c_str() );          
        
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
                        
            strcpy(jobOut.message, "Finished!");
            try{
                bool jobStatus=mmpbsa(dir, lig, calcPB);            
                jobOut.error=jobStatus;
            } catch (LBindException& e){
                std::string message= e.what();  
                strcpy(jobOut.message, message.c_str());
            }            
            
            strcpy(jobOut.dirBuffer, dir.c_str());
            strcpy(jobOut.ligBuffer, lig.c_str());
            MPI_Send(&jobOut, sizeof(JobOutData), MPI_CHAR, 0, outTag, MPI_COMM_WORLD);            
            
        }
    }


    time=MPI_Wtime()-time;
    std::cout << "Rank= " << rank << " MPI Wall Time= " << time << std::endl;
    MPI_Finalize();
    
    
    return 0;
}
