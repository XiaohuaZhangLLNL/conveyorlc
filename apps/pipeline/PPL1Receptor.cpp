/* 
 * File:   PPL1Receptor.cpp
 * Author: zhang
 *
 * Created on March 18, 2014, 12:10 PM
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

#include "PPL1ReceptorPO.h"

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

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
    export WORKDIR=`pwd`
    export LBindData=/usr/gapps/medchem/medcm/data/

    srun -N4 -n48 -ppdebug /g/g92/zhang30/medchem/NetBeansProjects/MedCM/apps/mmpbsa/PPL1Receptor  <input-file> 

    <input-file>: contain a list of receptor PDB Files.

    Requires: define WORKDIR 
 * 
 * Note: the non-standard residues are changed to ALA due to lack of amber forcefield.
   \endverbatim
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
    system(cmd.c_str());  
    
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
    std::string dataPath=LBINDDATA;     
           
    // ! start MPI parallel
    
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
        bool success=PPL1ReceptorPO(argc, argv, podata);
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
        
        FILE* xmlGoodFile=fopen("JobTrackingGood.xml", "w"); 
        FILE* xmlBadFile=fopen("JobTrackingBad.xml", "w"); 
        //! END of XML header        
       
        std::vector<std::string> dirList;        
        
        saveStrList(podata.inputFile, dirList);
        int count=0;
           
        for(unsigned i=0; i<dirList.size(); ++i){
            ++count;
            
            if(count >nproc){
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
                        
        }

        int nJobs=count;
        int ndata=(nJobs<nproc)? nJobs: nproc;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(int i=0; i < ndata; ++i){
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
        
        for(int i=1; i < nproc; ++i){
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
                bool jobStatus=preReceptor(dir, workDir, dataPath);            
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

