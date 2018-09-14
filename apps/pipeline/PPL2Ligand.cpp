/* 
 * File:   PPL2Ligand.cpp
 * Author: zhang
 *
 * Created on March 24, 2014, 11:16 AM
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

#include "PPL2LigandPO.h"

#include <boost/scoped_ptr.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace mpi = boost::mpi;

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
 *                  Each ligand must have "TOTAL_CHARGE" field value for ligand total charge.  

    Requires: define 
   \endverbatim
 */

class JobInputData{
    
public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & ambVersion;
        ar & dirBuffer; 
        ar & sdfBuffer;
        ar & cmpName;
    }
    
    int ambVersion;
    std::string dirBuffer;
    std::string sdfBuffer;
    std::string cmpName;
};

//struct JobInputData{        
//    char dirBuffer[100];
//};


class JobOutData{
    
public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {        
        ar & error;
        ar & gbEn;
        ar & ligID;
        ar & ligName;
        ar & pdbFilePath;
        ar & pdbqtPath;        
        ar & message; 
       
    }
    
    bool error;
    double gbEn;
    std::string ligID;
    std::string ligName;
    std::string pdbFilePath;
    std::string pdbqtPath;    
    std::string message;
   
};


//struct JobOutData{  
//    bool error;
//    char dirBuffer[100];
//    char message[100];
//};

void toXML(JobOutData& jobOut, XMLElement* root, FILE* xmlTmpFile){
        XMLElement * element = new XMLElement("Ligand");
        
        XMLElement * pdbidEle = new XMLElement("LigID");
        XMLText * pdbidTx= new XMLText(jobOut.ligID.c_str()); // has to use c-style string.
        pdbidEle->LinkEndChild(pdbidTx);
        element->LinkEndChild(pdbidEle);

        XMLElement * ligNameEle = new XMLElement("LigName");
        XMLText * ligNameTx= new XMLText(jobOut.ligName.c_str()); // has to use c-style string.
        ligNameEle->LinkEndChild(ligNameTx);
        element->LinkEndChild(ligNameEle);        
        
        XMLElement * pdbPathEle = new XMLElement("PDBPath");
        XMLText * pdbPathTx= new XMLText(jobOut.pdbFilePath.c_str());
        pdbPathEle->LinkEndChild(pdbPathTx);        
        element->LinkEndChild(pdbPathEle);

        XMLElement * recPathEle = new XMLElement("PDBQTPath");
        XMLText * recPathTx= new XMLText(jobOut.pdbqtPath.c_str());
        recPathEle->LinkEndChild(recPathTx);        
        element->LinkEndChild(recPathEle);
        
        XMLElement * gbEle = new XMLElement("GBEN");
        XMLText * gbTx= new XMLText(Sstrm<std::string, double>(jobOut.gbEn));
        gbEle->LinkEndChild(gbTx);        
        element->LinkEndChild(gbEle); 
        
        XMLElement * mesgEle = new XMLElement("Mesg");
        XMLText * mesgTx= new XMLText(jobOut.message);
        mesgEle->LinkEndChild(mesgTx);          
        element->LinkEndChild(mesgEle);           

        root->LinkEndChild(element);

        element->Print(xmlTmpFile,1);
        fputs("\n",xmlTmpFile);
        fflush(xmlTmpFile);          
        
}

bool isRun(std::string& checkfile, JobOutData& jobOut){
    
     std::ifstream inFile(checkfile.c_str());
    
    if(!inFile){
        return false;
    }  
     
    std::string fileLine=""; 
    std::string delimiter=":";
    while(inFile){
        std::getline(inFile, fileLine);
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens, delimiter); 

            if(tokens.size()!=2) continue;
            if(tokens[0]=="ligName"){
                jobOut.ligName=tokens[1];
            }
            if(tokens[0]=="GBSA"){
                jobOut.gbEn=Sstrm<double, std::string>(tokens[1]);
            }
            if(tokens[0]=="Mesg"){
                jobOut.message=tokens[1];
            }            
    }  
    return true;
}

void checkPoint(std::string& checkfile, JobOutData& jobOut){
    
    std::ofstream outFile(checkfile.c_str());
    
    outFile << "ligName:" << jobOut.ligName << "\n"
            << "GBSA:" << jobOut.gbEn << "\n"
            << "Mesg:" << jobOut.message << "\n";
    outFile.close();
}

bool preLigands(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir) {
    
    bool jobStatus=false;
    
    //chdir(workDir.c_str());
    // ! Goto sub directory
    std::string subDir=workDir+"/scratch/lig/"+jobOut.ligID;  
    jobOut.pdbFilePath="scratch/lig/"+jobOut.ligID+"/LIG_min.pdb";
    jobOut.pdbqtPath="scratch/lig/"+jobOut.ligID+"/LIG_min.pdbqt";
    
    std::string checkfile=workDir+"/scratch/lig/"+jobOut.ligID+"/checkpoint.txt";    
    if(isRun(checkfile, jobOut)) return true;
    
    jobOut.gbEn=0.0;        
    std::string sdfPath=subDir+"/ligand.sdf";
  
    std::string cmd="mkdir -p "+subDir;
    system(cmd.c_str());  
    
    std::ofstream outFile;
    try {
        outFile.open(sdfPath.c_str());
    }
    catch(...){
        std::cout << "preLigands >> Cannot open file" << sdfPath << std::endl;
    }

    outFile <<jobInput.sdfBuffer;
    outFile.close();     
    
    if(!fileExist(sdfPath)){
        std::string message=sdfPath+" does not exist.";
        throw LBindException(message);  
        return jobStatus;          
    }
    
    chdir(subDir.c_str());

    std::string sdfFile="ligand.sdf";
    std::string pdb1File="ligand.pdb";
    
    cmd="obabel -isdf " + sdfFile + " -opdb -O " +pdb1File +" >> log";
    std::cout << cmd << std::endl;
    std::string echo="echo ";
    echo=echo+cmd+" > log";
    system(echo.c_str());
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
    if(jobInput.cmpName=="NoName"){
        jobOut.ligName=pSdf->getTitle(sdfFile);
    }else{
        jobOut.ligName=pSdf->getInfo(sdfFile, jobInput.cmpName);
    }
    
    std::cout << "Charge:" << info << std::endl;
    int charge=Sstrm<int, std::string>(info);
    std::string chargeStr=Sstrm<std::string,int>(charge);
    
    //! Start antechamber calculation
    std::string output="ligand.mol2";
    std::string options=" -c bcc -nc "+ chargeStr;
        
    boost::scoped_ptr<Amber> pAmber(new Amber(jobInput.ambVersion));
    pAmber->antechamber(tmpFile, output, options);
    
    pAmber->parmchk(output);
    
    //! leap to obtain forcefield for ligand
    std::string ligName="LIG";
    std::string tleapFile="leap.in";
    
    pAmber->tleapInput(output,ligName,tleapFile);
    pAmber->tleap(tleapFile); 
    
    std::string checkFName="LIG.prmtop";
    if(!fileExist(checkFName)){
        std::string message="LIG.prmtop does not exist.";
        throw LBindException(message); 
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
    
    if(jobInput.ambVersion==13){
        cmd="sander13  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
    }else{
        cmd="sander  -O -i LIG_minGB.in -o LIG_minGB.out  -p LIG.prmtop -c LIG.inpcrd -ref LIG.inpcrd  -x LIG.mdcrd -r LIG_min.rst  >> log";
    }
    std::cout <<cmd <<std::endl;
    echo="echo ";
    echo=echo+cmd+" >> log";
    system(echo.c_str());
    system(cmd.c_str()); 
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    std::string sanderOut="LIG_minGB.out";
    double ligGBen=0;
    bool success=pSanderOutput->getEnergy(sanderOut,ligGBen);
    jobOut.gbEn=ligGBen;
    
    if(!success){
        std::string message="Ligand GB minimization fails.";
        throw LBindException(message); 
        return jobStatus;          
    }
    
    //! Use ambpdb generated PDB file for PDBQT.
    if(jobInput.ambVersion==16){
        cmd="ambpdb -p LIG.prmtop -c LIG_min.rst > LIG_minTmp.pdb "; 
    }else{
        cmd="ambpdb -p LIG.prmtop < LIG_min.rst > LIG_minTmp.pdb ";
    } 
       
    std::cout <<cmd <<std::endl;
    echo="echo ";
    echo=echo+"\'"+cmd+"\'  >> log";
    system(echo.c_str());    
    system(cmd.c_str());    

    checkFName="LIG_minTmp.pdb";
    if(!fileExist(checkFName)){
        std::string message="LIG_min.pdb minimization PDB file does not exist.";
        throw LBindException(message);  
        return jobStatus;        
    }
    
    pPdb->fixElement("LIG_minTmp.pdb", "LIG_min.pdb"); 
    
    
        
    //! Get DPBQT file for ligand from minimized structure.
    cmd="prepare_ligand4.py -l LIG_min.pdb  >> log";
    std::cout << cmd << std::endl;
    echo="echo ";
    echo=echo+cmd+" >> log";
    system(echo.c_str());    
    system(cmd.c_str());

    checkFName="LIG_min.pdbqt";
    if(!fileExist(checkFName)){
        std::string message="LIG_min.pdbqt PDBQT file does not exist.";
        throw LBindException(message); 
        return jobStatus;        
    } 
    
    //! fix the Br element type
    cmd="sed -i '/Br.* LIG/{s! B ! Br!}' LIG_min.pdbqt";
    std::cout << cmd << std::endl;
    echo="echo ";
    echo=echo+cmd+" >> log";
    system(echo.c_str());
    system(cmd.c_str());

    checkPoint(checkfile, jobOut);
    
    //cmd="rm -f *.in divcon.pdb fort.7 leap.log mopac.pdb ligand.pdb ligrn.pdb ligstrp.pdb LIG_minTmp.pdb";
    cmd="rm -f divcon.pdb fort.7 leap.log mopac.pdb ligand.pdb ligrn.pdb ligstrp.pdb LIG_minTmp.pdb";
    std::cout <<cmd <<std::endl;    
    system(cmd.c_str());
    
    jobStatus=true;
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

void splitSDF(POdata& podata, std::vector<std::string>& ligList, std::string& workDir){
    std::ifstream inFile;
    try {
        inFile.open(podata.sdfFile.c_str());
    }
    catch(...){
        std::cout << "preLigands >> Cannot open file" << podata.sdfFile << std::endl;
    } 

    const std::string delimter="$$$$";
    std::string fileLine="";
    std::string contents="";            

    int count=podata.firstLigID;

    while(inFile){
        std::getline(inFile, fileLine);
        contents=contents+fileLine+"\n";
        if(fileLine.size()>=4 && fileLine.compare(0,4, delimter)==0){
            std::string dir=Sstrm<std::string, int>(count);
                // ! Goto sub directory
            std::string subDir=workDir+"/scratch/lig/"+dir;   
            std::string cmd="mkdir -p "+subDir;
            system(cmd.c_str());  

            std::string outputFName=subDir+"/ligand.sdf";
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
        mesg = "Could not load PPL2Track.xml file.\nError: " + mesg;
        throw LBindException(mesg);
    }
    
    XMLNode* node = doc.FirstChild("Ligands");
    assert(node);
    XMLNode* ligNode = node->FirstChild("Ligand");
    assert(ligNode);

    for (ligNode = node->FirstChild("Ligand"); ligNode != 0; ligNode = ligNode->NextSibling("Ligand")) {

        XMLNode* mesgNode = ligNode->FirstChild("Mesg");
        assert(mesgNode);
        XMLText* mesgTx =mesgNode->FirstChild()->ToText();
        assert(mesgTx);
        std::string mesgStr = mesgTx->ValueStr();

        if(mesgStr=="Finished!"){        
            XMLNode* ligIDnode = ligNode->FirstChild("LigID");
            assert(ligIDnode);
            XMLText* ligIDtx =ligIDnode->FirstChild()->ToText(); 
            std::string dir=ligIDtx->ValueStr();
            ligList.push_back(dir);
        }
        
    } 

    std::cout << "Print Previous Successful Ligand List: " << std::endl;
    for(unsigned i=0; i< ligList.size(); ++i){
        std::cout << "Ligand " << ligList[i] << std::endl;
    }
    
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
    
    // ! MPI Parallel

    int jobFlag=1; // 1: doing job,  0: done job
    
    JobInputData jobInput;
    JobOutData jobOut;
    
        
    int rankTag=1;
    int jobTag=2;

    int inpTag=3;
    int outTag=4;

    mpi::environment env(argc, argv);
    mpi::communicator world; 
    
    mpi::timer runingTime;

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        return 1;
    }
       
    POdata podata;
    int error=0;
    
    if (world.rank() == 0) {        
        bool success=PPL2LigandPO(argc, argv, podata);
        if(!success){
            error=1;           
        }        
    }
               
    if (world.rank() == 0) {
                
//! Tracking error using XML file

	XMLDocument doc;  
 	XMLDeclaration* decl = new XMLDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	XMLElement * root = new XMLElement( "Ligands" );  
	doc.LinkEndChild( root );  

	XMLComment * comment = new XMLComment();
	comment->SetValue(" Tracking calculation error using XML file " );  
	root->LinkEndChild( comment );  
         
        FILE* xmlFile=fopen("PPL2TrackTemp.xml", "w"); 
        fprintf(xmlFile, "<?xml version=\"1.0\" ?>\n");
        fprintf(xmlFile, "<Ligands>\n");
        fprintf(xmlFile, "    <!-- Tracking calculation error using XML file -->\n");
        fflush(xmlFile);         
 //! END of XML header
        
        // Pass the ligand name option
        jobInput.cmpName=podata.cmpName;

// Open output file
        std::ofstream outFile;
        outFile.open(podata.outputFile.c_str());

        // Start to read in the SDF file
        std::ifstream inFile;
        try {
            inFile.open(podata.sdfFile.c_str());
        } catch (...) {
            std::cout << "preLigands >> Cannot open file" << podata.sdfFile << std::endl;
        }

        const std::string delimter = "$$$$";
        std::string fileLine = "";
        std::string contents = "";

        int count = 0;

        while (inFile) {
            std::getline(inFile, fileLine);
            contents = contents + fileLine + "\n";
            if (fileLine.size() >= 4 && fileLine.compare(0, 4, delimter) == 0) {
                            
                if(count >= world.size()-1){
    //                MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
                    world.recv(mpi::any_source, outTag, jobOut);
                    toXML(jobOut, root, xmlFile);
                } 

                std::string dir=Sstrm<std::string, int>(count+1);

                int freeProc;
    //            MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
                world.recv(mpi::any_source, rankTag, freeProc);
                std::cout << "At Process: " << freeProc << " working on: " << dir << std::endl;
    //            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 
                world.send(freeProc, jobTag, jobFlag);

    //            strcpy(jobInput.dirBuffer, dir.c_str());
                jobInput.dirBuffer=dir;
                jobInput.sdfBuffer=contents;
                jobInput.ambVersion=podata.version;

    //            MPI_Send(&jobInput, sizeof(JobInputData), MPI_CHAR, freeProc, inpTag, MPI_COMM_WORLD);
                world.send(freeProc, inpTag, jobInput);
                
                contents = ""; //! clean up the contents for the next structure.
                count++;
            }
                             
        }

        int nJobs=count;
        int nWorkers=world.size()-1;
        int ndata=(nJobs<nWorkers)? nJobs: nWorkers;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(int i=0; i < ndata; ++i){
//            MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
            world.recv(mpi::any_source, outTag, jobOut);
            toXML(jobOut, root, xmlFile);
        } 
        
        fprintf(xmlFile, "</Ligands>\n");
        doc.SaveFile(podata.xmlOut);
        
        for(int i=1; i < world.size(); ++i){
            int freeProc;
//            MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;;
//            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 
            world.send(freeProc, jobTag, jobFlag);
        }
        
    }else {
        while (1) {
//            MPI_Send(&rank, 1, MPI_INTEGER, 0, rankTag, MPI_COMM_WORLD);
            world.send(0, rankTag, world.rank());
//            MPI_Recv(&jobFlag, 20, MPI_CHAR, 0, jobTag, MPI_COMM_WORLD, &status2);
            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }
            // Receive parameters

            world.recv(0, inpTag, jobInput);
//            MPI_Recv(&jobInput, sizeof(JobInputData), MPI_CHAR, 0, inpTag, MPI_COMM_WORLD, &status1);
                        
            jobOut.ligID=jobInput.dirBuffer;
            jobOut.message="Finished!";
            try{
                bool jobStatus=preLigands(jobInput, jobOut, workDir);            
                jobOut.error=jobStatus;
            } catch (LBindException& e){
                jobOut.message= e.what();  
            }
            
//            MPI_Send(&jobOut, sizeof(JobOutData), MPI_CHAR, 0, outTag, MPI_COMM_WORLD);
            world.send(0, outTag, jobOut);
        }
    }

    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;
    
    return 0;
}


