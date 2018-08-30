/* 
 * File:   amberTools.cpp
 * Author: zhang30
 *
 * Created on August 10, 2012, 10:15 AM
 */

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "Parser/Pdb.h"
#include "Parser/SanderOutput.h"
#include "MM/VinaLC.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Coor3d.h"
#include "Structure/Constants.h"
#include "Common/File.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"

#include "amberToolsPO.h"

#include <boost/scoped_ptr.hpp>

#include <mpi.h>

using namespace LBIND;

/*!
 * \breif amberTools Calculate receptor GB/PB minimization energy from download PDB files.
 * \param argc
 * \param argv argv[1] takes the input file name
 * \return success 
 * \defgroup amberTools_Commands amberTools Commands
 *
 * 
 * Usage on HPC slurm
 * 
 * \verbatim
 
    export AMBERHOME=/usr/gapps/medchem/amber/amber12
    export PATH=$AMBERHOME/bin/:$PATH
    export WORKDIR=`pwd`/workspace/

    srun -N4 -n48 -ppdebug /g/g92/zhang30/medchem/NetBeansProjects/MedCM/apps/mmpbsa/amberTools  <input-file> <GetPDB|No>

    <input-file>: contain a list of receptor PDB IDs.

    Requires: define WORKDIR 
 * 
 * Note: the non-standard residues are changed to ALA due to lack of amber forcefield.
   \endverbatim
 */

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

    std::string pdbFile=dir+".pdb";
    std::string checkFName=pdbFile;
    if(!fileExist(checkFName)){
        cmd="wget http://www.rcsb.org/pdb/files/"+pdbFile;
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
    
    
//    std::string csaPdbFile=siteDir+dir+"_00_ref_cent_1.pdb";
//
//    checkFName=csaPdbFile;
//    if(!fileExist(checkFName)){
//        std::string message=checkFName+" does not exist.";
//        throw LBindException(message); 
//        jobStatus=false; 
//        return jobStatus;        
//    }    
        
    boost::scoped_ptr<VinaLC> pVinaLC(new VinaLC());
    
    std::string sumFile=siteDir+dir+"_00_ref.sum";
    Coor3d center;
    pVinaLC->centroid(sumFile,center);
//    std::cout << "Center coordinates: X=" << center.getX() 
//              << " Y=" << center.getY()
//              << " Z=" << center.getZ() 
//              << std::endl;

      
    std::string cutPdbFile=dir+"_00_ref_cut.pdb";
    pPdb->cutByRadius(pdbOutFile, cutPdbFile, center, 20);
    
    
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

    cmd="reduce -Quiet -Trim  "+stdPdbFile+" >& rec.pdb ";
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
        
        tleapFile << "source leaprc.ff14SB\n"                
                  << "source leaprc.gaff\n"
                  << "REC = loadpdb rec.pdb\n"
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
    
//    minFName="Rec_minGB2.in";
//    {
//        std::ofstream minFile;
//        try {
//            minFile.open(minFName.c_str());
//        }
//        catch(...){
//            std::string mesg="mmpbsa::receptor()\n\t Cannot open min file: "+minFName;
//            throw LBindException(mesg);
//        }   
//
//        minFile << "title..\n" 
//                << "&cntrl\n" 
//                << "  imin   = 1,\n" 
//                << "  ntmin   = 3,\n" 
//                << "  maxcyc = 2000,\n" 
//                << "  ncyc   = 1000,\n" 
//                << "  ntpr   = 200,\n" 
//                << "  ntb    = 0,\n" 
//                << "  igb    = 5,\n" 
//                << "  gbsa   = 1,\n"
//                << "  cut    = 15,\n" 
//                << "  ntr=1,\n" 
//                << "  restraint_wt=1.0,\n" 
//                << "  restraintmask='!@H=',\n"        
//                << " /\n" << std::endl;
//        minFile.close();    
//    }
//    
//    cmd="sander -O -i Rec_minGB2.in -o Rec_minGB2.out  -p REC.prmtop -c Rec_min.rst -ref Rec_min.rst -x REC.mdcrd -r Rec_min2.rst";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str()); 
    
//    sanderOut="Rec_minGB2.out";
//    recGBen=0;
//    success=pSanderOutput->getEnergy(sanderOut, recGBen);
//    std::cout << "Receptorn GB Minimization Energy: " << recGBen <<" kcal/mol."<< std::endl;

//    if(!success){
//        std::string message="Receptor 2nd GB minimization fails.";
//        throw LBindException(message); 
//        jobStatus=false; 
//        return jobStatus;          
//    }    
    
    cmd="ambpdb -p REC.prmtop -c Rec_min.rst > Rec_min_0.pdb";
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
//    minFName="Rec_minPB.in";
//    {
//        std::ofstream minFile;
//        try {
//            minFile.open(minFName.c_str());
//        }
//        catch(...){
//            std::string mesg="mmpbsa::receptor()\n\t Cannot open min file: "+minFName;
//            throw LBindException(mesg);
//        }   
//
//        minFile << "title..\n" 
//                << " &cntrl\n" 
//                << "  imin   = 1,\n" 
//                << "  ntmin   = 3,\n" 
//                << "  maxcyc = 2000,\n" 
//                << "  ncyc   = 1000,\n" 
//                << "  ntpr   = 200,\n" 
//                << "  ntb    = 0,\n" 
//                << "  igb    = 10,\n" 
//                << "  cut    = 15,\n"        
//                << " /\n"
//                << " &pb\n"
//                << "  npbverb=0, npopt=2, epsout=80.0, radiopt=1, space=0.5,\n"
//                << "  accept=1e-4, fillratio=6, sprob=1.6\n"
//                << " / \n" << std::endl;
//                
//        minFile.close();    
//    } 
//    cmd="sander -O -i Rec_minPB.in -o Rec_minPB.out -p REC.prmtop -c Rec_min2.rst -ref Rec_min2.rst -x REC.mdcrd -r Rec_min3.rst";
//    std::cout <<cmd <<std::endl;
//    system(cmd.c_str()); 
//       
//    sanderOut="Rec_minPB.out";
//    double recPBen=0;
//    success=pSanderOutput->getEnergy(sanderOut,recPBen);
//    std::cout << "Receptor PB Minimization Energy: " << recPBen <<" kcal/mol."<< std::endl;
//    if(!success){
//        std::string message="Receptor PB minimization fails.";
//        throw LBindException(message); 
//        jobStatus=false; 
//        return jobStatus;          
//    }    
    
    return jobStatus;
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
    bool getPDBflg;
    char dirBuffer[100];
};

struct JobOutData{  
    bool error;
    char dirBuffer[100];
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
        bool success=amberToolsPO(argc, argv, podata);
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
        
        jobInput.getPDBflg=podata.getPDBflg;
        
        //! Tracking error using XML file
	XMLDocument doc;  
 	XMLDeclaration* decl = new XMLDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	XMLElement * root = new XMLElement( "Errors" );  
	doc.LinkEndChild( root );  

	XMLComment * comment = new XMLComment();
	comment->SetValue(" Tracking calculation error using XML file " );  
	root->LinkEndChild( comment );     
        
        FILE* xmlGoodFile=fopen("JobTrackingGood.xml", "w"); 
        FILE* xmlBadFile=fopen("JobTrackingBad.xml", "w"); 
        //! END of XML header        
       
        std::vector<std::string> dirList;        
        
        saveStrList(podata.inputFile, dirList);
        int count=0;
           
        for(unsigned i=0; i<dirList.size(); ++i){
            
            if(count >nproc-1){
                MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
                XMLElement * element = new XMLElement("error"); 
                element->SetAttribute("receptor", jobOut.dirBuffer);
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
            std::cout << "At Process: " << freeProc << " working on: " << dirList[i]  << std::endl;
            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 

            strcpy(jobInput.dirBuffer, dirList[i].c_str());

            MPI_Send(&jobInput, sizeof(JobInputData), MPI_CHAR, freeProc, inpTag, MPI_COMM_WORLD);
            
            ++count;
        }

        int nJobs=count-1;
        int ndata=(nJobs<nproc-1)? nJobs: nproc-1;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(unsigned i=0; i < ndata; ++i){
            MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
            XMLElement * element = new XMLElement("error"); 
            element->SetAttribute("receptor", jobOut.dirBuffer);
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
        
        doc.SaveFile( "JobTracking.xml" );        
        
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
            strcpy(jobOut.message, "Finished!");
            try{
                bool jobStatus=preReceptors(dir, jobInput.getPDBflg);            
                jobOut.error=jobStatus;
            } catch (LBindException& e){
                std::string message= e.what();  
                strcpy(jobOut.message, message.c_str());
            }            
            
            strcpy(jobOut.dirBuffer, dir.c_str());
            MPI_Send(&jobOut, sizeof(JobOutData), MPI_CHAR, 0, outTag, MPI_COMM_WORLD);            
           
        }
    }


    time=MPI_Wtime()-time;
    std::cout << "Rank= " << rank << " MPI Wall Time= " << time << std::endl;
    MPI_Finalize();
    
    
    return 0;
}
