/* 
 * File:   preLigands.cpp
 * Author: zhang30
 *
 * Created on August 13, 2012, 4:08 PM
 */

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include "Parser/Sdf.h"
#include "Parser/Pdb.h"
#include "MM/Amber.h"
#include "Parser/SanderOutput.h"
#include "Structure/Sstrm.hpp"
#include "Structure/Constants.h"
#include "Structure/Molecule.h"
#include "Parser/Mol2.h"
#include "Common/File.hpp"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"

#include "preLigandsPO.h"

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
    export LIGDIR=`pwd`

    srun -N4 -n48 -ppdebug /g/g92/zhang30/medchem/NetBeansProjects/MedCM/apps/mmpbsa/preLignds  <input-file.sdf>

    <input-file.sdf>: multi-structure SDF file contains all ligands information.

    Requires: define LIGDIR 
   \endverbatim
 */


bool preLigands(std::string& dir) {
    
    bool jobStatus=true;
    // ! Goto sub directory
    std::string LIGDIR=getenv("LIGDIR");
    std::string subDir=LIGDIR+"/"+dir;   
    chdir(subDir.c_str());

    std::string sdfFile="ligand.sdf";
    std::string pdb1File="ligand.pdb";
    
    std::string cmd="obabel -isdf " + sdfFile + " -opdb -O " +pdb1File;
    std::cout << cmd << std::endl;
    system(cmd.c_str());
    
    std::string pdbFile="ligrn.pdb";
    std::string tmpFile="ligstrp.pdb";
    
    boost::scoped_ptr<Pdb> pPdb(new Pdb());   
    
    //! Rename the atom name.
    pPdb->renameAtom(pdb1File, pdbFile);
    
    pPdb->strip(pdbFile, tmpFile);
    
    //! Get ligand charge from SDF file.
    std::string keyword="TOTAL_CHARGE";
    
    boost::scoped_ptr<Sdf> pSdf(new Sdf());
    std::string info=pSdf->getInfo(sdfFile, keyword);
    
    std::cout << "Charge:" << info << std::endl;
    int charge=Sstrm<int, std::string>(info);
    std::string chargeStr=Sstrm<std::string,int>(charge);
    
    //! Start antechamber calculation
    std::string output="ligand.mol2";
    std::string options=" -c bcc -nc "+ chargeStr;
        
    boost::scoped_ptr<Amber> pAmber(new Amber());
    pAmber->antechamber(pdbFile, output, options);
    
    pAmber->parmchk(output);
    
    //! leap to obtain forcefield for ligand
    std::string ligName="LIG";
    std::string tleapFile="leap.in";

    //WIP
    //pAmber->tleapInput(output,ligName,tleapFile);
    //pAmber->tleap(tleapFile);
    
    std::string checkFName="LIG.prmtop";
    if(!fileExist(checkFName)){
        std::string message="LIG.prmtop does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    }
   
    //! GB energy minimization
    std::string minFName="LIG_minGB.in";
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
                << " /\n" << std::endl;

        minFile.close();    
    }          
    
    cmd="sander  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str()); 
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    std::string sanderOut="LIG_minGB.out";
    double ligGBen=0;
    bool success=pSanderOutput->getEnergy(sanderOut,ligGBen);
    
    if(!success){
        std::string message="Ligand GB minimization fails.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;          
    }
    
    //! Use ambpdb generated PDB file for PDBQT.
    cmd="ambpdb -p LIG.prmtop -c LIG_min.rst > LIG_minTmp.pdb";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());    

    checkFName="LIG_minTmp.pdb";
    if(!fileExist(checkFName)){
        std::string message="LIG_min.pdb minimization PDB file does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    }
    
    pPdb->fixElement("LIG_minTmp.pdb", "LIG_min.pdb"); 
        
    //! Get DPBQT file for ligand from minimized structure.
    cmd="prepare_ligand4.py -l LIG_min.pdb";
    std::cout << cmd << std::endl;
    system(cmd.c_str());
    
    checkFName="LIG_min.pdbqt";
    if(!fileExist(checkFName)){
        std::string message="LIG_min.pdbqt PDBQT file does not exist.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;        
    } 
    
    //! PB energy minimization
    minFName="LIG_minPB.in";
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
                << "  npbverb=0, npopt=2, epsout=80.0, radiopt=1, space=0.5,\n"
                << "  accept=1e-4, fillratio=6, sprob=1.6\n"
                << " / \n" << std::endl;

        minFile.close();    
    }          
    
    cmd="sander  -O -i LIG_minPB.in -o LIG_minPB.out  -p LIG.prmtop -c LIG_min.rst -ref LIG_min.rst  -x LIG.mdcrd -r LIG_min2.rst";
    std::cout <<cmd <<std::endl;
    system(cmd.c_str());       

    sanderOut="LIG_minPB.out";
    double ligPBen=0;
    success=pSanderOutput->getEnergy(sanderOut,ligPBen);
    
    if(!success){
        std::string message="Ligand PB minimization fails.";
        throw LBindException(message); 
        jobStatus=false; 
        return jobStatus;          
    }    
    return jobStatus;
}


//void saveStrList(std::string& fileName, std::vector<std::string>& strList){
//    std::ifstream inFile;
//    try {
//        inFile.open(fileName.c_str());
//    }
//    catch(...){
//        std::cout << "Cannot open file: " << fileName << std::endl;
//    } 
//
//    std::string fileLine;
//    while(inFile){
//        std::getline(inFile, fileLine);
//        std::vector<std::string> tokens;
//        tokenize(fileLine, tokens); 
//        if(tokens.size() > 0){
//            strList.push_back(tokens[0]);
//        }        
//    }
//    
//}

void splitSDF(std::string& sdfFile, std::vector<std::string>& ligList){
    std::ifstream inFile;
    try {
        inFile.open(sdfFile.c_str());
    }
    catch(...){
        std::cout << "preLigands >> Cannot open file" << sdfFile << std::endl;
    } 

    const std::string delimter="$$$$";
    std::string fileLine="";
    std::string contents="";            

    int count=1;

    while(inFile){
        std::getline(inFile, fileLine);
        contents=contents+fileLine+"\n";
        if(fileLine.size()>=4 && fileLine.compare(0,4, delimter)==0){
            std::string dir=Sstrm<std::string, int>(count);

            std::string cmd="mkdir -p " +dir;
            std::cout << cmd << std::endl;
            system(cmd.c_str());

            std::string outputFName=dir+"/ligand.sdf";
            std::ofstream outFile;
            try {
                outFile.open(outputFName.c_str());
            }
            catch(...){
                std::cout << "preLigands >> Cannot open file" << outputFName << std::endl;
            }

            outFile <<contents;
            outFile.close(); 

            contents=""; //! clean up the contents for the next structure.

            ligList.push_back(dir);                    
            ++count;
        }
    }    
    
}

void xmlEJobs(std::string& xmlFile, std::vector<std::string>& ligList){
    XMLDocument doc(xmlFile);
    bool loadOkay = doc.LoadFile();

    if (!loadOkay) {
        std::string mesg = doc.ErrorDesc();
        mesg = "Could not load elements.xml file.\nError: " + mesg;
        throw LBindException(mesg);
    }

    XMLNode* node = doc.FirstChild("Errors");
    assert(node);
    XMLNode* subNode = node->FirstChild("error");
    assert(subNode);

    for (subNode = node->FirstChild("error"); subNode != 0; subNode = subNode->NextSibling("error")) {
        XMLElement* itemElement = subNode->ToElement();
        bool rstFlg=false;
        std::string dir;
        for (XMLAttribute* pAttrib = itemElement->FirstAttribute(); pAttrib != 0; pAttrib = pAttrib->Next()) {
            std::string nameStr = pAttrib->NameTStr();
            if (nameStr == "ligand") {
                dir=pAttrib->ValueStr();
            }
            if (nameStr == "mesg") {
                if(pAttrib->ValueStr()=="LIG.prmtop does not exist."){
                    rstFlg=true;
                }
            }
            
        }
        if(rstFlg){
            ligList.push_back(dir);
        }
    }
}

struct JobInputData{        
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
        bool success=preLigandsPO(argc, argv, podata);
        if(!success){
            error=1;           
        }        
    }

    MPI_Bcast(&error, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (error !=0) {
        MPI_Finalize();
        return 1;
    } 
    
    std::string LIGDIR=getenv("LIGDIR");
    chdir(LIGDIR.c_str());

    std::vector<std::string> ligList;
    // split SDF file
    if(rank==0){
        if(!podata.restart){
            splitSDF(podata.sdfFile, ligList);            
        }else{
            xmlEJobs(podata.xmlRst, ligList);
        }                
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
         
        FILE* xmlFile=fopen("JobTrackingTemp.xml", "w"); 
 //! END of XML header

        for(unsigned i=0; i <ligList.size(); ++i){
            if(i >=nproc-1){
                MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
//                    if(!jobOut.error){
                    XMLElement * element = new XMLElement("error"); 
                    element->SetAttribute("ligand", jobOut.dirBuffer);
                    element->SetAttribute("mesg", jobOut.message);
                    element->Print(xmlFile,2);
                    fputs("\n",xmlFile);
                    fflush(xmlFile);
                    root->LinkEndChild(element);    
//                    }
            } 

            std::string dir=ligList[i];

            int freeProc;
            MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
            std::cout << "At Process: " << freeProc << " working on: " << dir << std::endl;
            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 

            strcpy(jobInput.dirBuffer, dir.c_str());

            MPI_Send(&jobInput, sizeof(JobInputData), MPI_CHAR, freeProc, inpTag, MPI_COMM_WORLD);   
                             
        }

        int nJobs=ligList.size();
        int ndata=(nJobs<nproc-1)? nJobs: nproc-1;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(unsigned i=0; i < ndata; ++i){
            MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
//            if(!jobOut.error){
                XMLElement * element = new XMLElement("error"); 
                element->SetAttribute("ligand", jobOut.dirBuffer);
                element->SetAttribute("mesg", jobOut.message);
                element->Print(xmlFile,2);
                fputs("\n",xmlFile);
                fflush(xmlFile);
                root->LinkEndChild(element);                  
//            }
        } 
        
        doc.SaveFile(podata.xmlOut);
        
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
                bool jobStatus=preLigands(dir);            
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
