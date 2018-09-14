/* 
 * File:   PPL3Docking.cpp
 * Author: zhang
 *
 * Created on March 25, 2014, 2:51 PM
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <stack>
#include <vector> // ligand paths
#include <cmath> // for ceila

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename
#include <boost/thread/thread.hpp> // hardware_concurrency // FIXME rm ?
#include <boost/timer.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include "VinaLC/parse_pdbqt.h"
#include "VinaLC/parallel_mc.h"
#include "VinaLC/file.h"
#include "VinaLC/cache.h"
#include "VinaLC/non_cache.h"
#include "VinaLC/naive_non_cache.h"
#include "VinaLC/parse_error.h"
#include "VinaLC/everything.h"
#include "VinaLC/weighted_terms.h"
#include "VinaLC/current_weights.h"
#include "VinaLC/quasi_newton.h"
//#include "gzstream.h"
//#include "tee.h"
#include "VinaLC/coords.h" // add_to_output_container
#include "VinaLC/tokenize.h"

#include "dock.h"
#include "mpiparser.h"
#include "XML/XMLHeader.hpp"

namespace mpi = boost::mpi;
using namespace LBIND;


void toXML(JobOutData& jobOut, XMLElement* root, FILE* xmlTmpFile) {
    XMLElement * element = new XMLElement("Complex");

    XMLElement * pdbidEle = new XMLElement("RecID");
    XMLText * pdbidTx = new XMLText(jobOut.pdbID.c_str()); // has to use c-style string.
    pdbidEle->LinkEndChild(pdbidTx);
    element->LinkEndChild(pdbidEle);
    
    XMLElement * nonstdAAsEle = new XMLElement("NonStdAAList");
    for (unsigned i = 0; i < jobOut.nonRes.size(); ++i) {
        std::string iStr=Sstrm<std::string, unsigned>(i+1);
        XMLElement * scEle = new XMLElement("NonStdAA");
        scEle->SetAttribute("id", iStr.c_str() );
        XMLText * scTx = new XMLText(jobOut.nonRes[i].c_str());
        scEle->LinkEndChild(scTx);
        nonstdAAsEle->LinkEndChild(scEle);
    }
    element->LinkEndChild(nonstdAAsEle);

    XMLElement * ligidEle = new XMLElement("LigID");
    XMLText * ligidTx = new XMLText(jobOut.ligID.c_str()); // has to use c-style string.
    ligidEle->LinkEndChild(ligidTx);
    element->LinkEndChild(ligidEle);

    XMLElement * pdbPathEle = new XMLElement("LogPath");
    XMLText * pdbPathTx = new XMLText(jobOut.logPath.c_str());
    pdbPathEle->LinkEndChild(pdbPathTx);
    element->LinkEndChild(pdbPathEle);

    XMLElement * recPathEle = new XMLElement("PosePath");
    XMLText * recPathTx = new XMLText(jobOut.posePath.c_str());
    recPathEle->LinkEndChild(recPathTx);
    element->LinkEndChild(recPathEle);

    XMLElement * scoresEle = new XMLElement("Scores");
    for (unsigned i = 0; i < jobOut.scores.size(); ++i) {
        std::string iStr=Sstrm<std::string, unsigned>(i+1);
        XMLElement * scEle = new XMLElement("Pose");
        scEle->SetAttribute("id", iStr.c_str() );
        XMLText * scTx = new XMLText(Sstrm<std::string, double>(jobOut.scores[i]));
        scEle->LinkEndChild(scTx);
        scoresEle->LinkEndChild(scEle);
    }
    element->LinkEndChild(scoresEle);

    XMLElement * mesgEle = new XMLElement("Mesg");
    XMLText * mesgTx = new XMLText(jobOut.mesg);
    mesgEle->LinkEndChild(mesgTx);
    element->LinkEndChild(mesgEle);

    root->LinkEndChild(element);

    element->Print(xmlTmpFile, 1);
    fputs("\n", xmlTmpFile);
    fflush(xmlTmpFile);          
        
}


inline void geometry(JobInputData& jobInput, std::vector<double>& geo){
//    const fl granularity = 0.375;
    vec center(geo[0], geo[1], geo[2]);
    vec span(geo[3], geo[4], geo[5]);

    for(unsigned j=0;j<3; ++j){
        jobInput.n[j]=sz(std::ceil(span[j] / jobInput.granularity));
        fl real_span = jobInput.granularity * jobInput.n[j];
        jobInput.begin[j]=center[j] - real_span / 2;
        jobInput.end[j]=jobInput.begin[j] + real_span;
    }    
}

bool isRun(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir){
    
    std::string checkfile =workDir + "/scratch/com/" + jobInput.recBuffer + "/dock/" + jobInput.ligBuffer+"/scores.log";
    std::ifstream inFile(checkfile.c_str());
     
    if(!inFile){
        return false;
    } 
    
    //std::cout << checkfile << std::endl;

    std::string logStr="";
    std::string fileLine;
    
    while(inFile){
        std::getline(inFile, fileLine);
        logStr=logStr+fileLine+"\n";       
    }
     
    jobOut.scores.clear();
    bool success=getScores(logStr, jobOut.scores);
    if(success){
        jobOut.mesg="Finished!";
    }else{
        jobOut.mesg="No scores!";
    }
    jobOut.nonRes=jobInput.nonRes;

    jobOut.pdbID=jobInput.recBuffer;
    jobOut.ligID=jobInput.ligBuffer;

    jobOut.logPath=checkfile;
    jobOut.posePath="scratch/com/" + jobInput.recBuffer + "/dock/" + jobInput.ligBuffer+"/poses.pdbqt";    
    
    return true;
}


int main(int argc, char* argv[]) {
    
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

    std::cout << "Number of tasks= " << world.size() << " My rank= " << world.rank() << std::endl;

    if (world.rank() == 0) {
        std::cout << "Master Node: " << world.size() << " My rank= " << world.rank() << std::endl;
        std::string recFile;
        std::string fleFile;
        std::string ligFile;      
        std::vector<std::string> recList;
        std::vector<std::string> fleList;
        std::vector<std::string> ligList;
        std::vector<std::vector<double> > geoList;
        std::vector<std::vector<std::string> > nonAAList;
       
        std::cout << "Begin Parser" << std::endl;
        int success=mpiParser(argc, argv, recFile, fleFile, ligFile, ligList, recList, fleList, geoList, nonAAList, jobInput);
        if(success!=0) {
            std::cerr << "Error: Parser input error" << std::endl;
            return 1;            
        }
        std::cout << "End of Parser" << std::endl;

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        return 1;
    }
        unsigned num_cpus = boost::thread::hardware_concurrency();
        if (num_cpus > 0)
            jobInput.cpu = num_cpus;
        else
            jobInput.cpu = 1;    
        
        
        int count=0;
                
        jobInput.flexible=false;
        if(fleList.size()==recList.size()){
            jobInput.flexible=true;
        }
        
        if(jobInput.randomize){
            srand(unsigned(std::time(NULL)));
        }
        
        
//! Tracking error using XML file
//        std::cout << "Begin XML" << std::endl;
	XMLDocument doc;  
 	XMLDeclaration* decl = new XMLDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	XMLElement * root = new XMLElement( "Complexes" );  
	doc.LinkEndChild( root );  

	XMLComment * comment = new XMLComment();
	comment->SetValue(" Tracking calculation error using XML file " );  
	root->LinkEndChild( comment );  
         
        FILE* xmlFile=fopen("PPL3TrackTemp.xml", "w"); 
        fprintf(xmlFile, "<?xml version=\"1.0\" ?>\n");
        fprintf(xmlFile, "<Complexes>\n");
        fprintf(xmlFile, "    <!-- Tracking calculation error using XML file -->\n");
        fflush(xmlFile); 
//        std::cout << "END XML" << std::endl;       
 //! END of XML header        
        if(recList.size() != geoList.size()){
            std::cout << "recList.size()= " << recList.size() << " geoList.size()= " << geoList.size()  << std::endl;
            std::cerr << "Error: recList and geoList should have same size" << std::endl;
            return 1;              
        }
        for (unsigned i = 0; i < recList.size(); ++i) {
            std::vector<double> geo = geoList[i];
            geometry(jobInput, geo);
            for (unsigned j = 0; j < ligList.size(); ++j) {
                if (jobInput.randomize) {
                    jobInput.seed = rand();
                }
                
//                std::cout << "i=" << i << " j=" << j << std::endl;

                ++count;
                if (count > world.size()-1) {
                    world.recv(mpi::any_source, outTag, jobOut);
                    toXML(jobOut, root, xmlFile);
                  
// add output here
                }
                int freeProc;
                world.recv(mpi::any_source, rankTag, freeProc);
                world.send(freeProc, jobTag, jobFlag);
                // Start to send parameters                                        
                jobInput.recBuffer = recList[i];
                jobInput.ligBuffer = ligList[j];
                
                jobInput.nonRes = nonAAList[i];
                 
                if (jobInput.flexible) {
                    jobInput.fleBuffer = fleList[i];
                }

                std::cout << "At Process: " << freeProc << " working on  Receptor: " << recList[i] << "  Ligand: " <<  ligList[j]<< std::endl;

                world.send(freeProc, inpTag, jobInput);

            }

        }
     

        int nJobs=count;
        int nWorkers=world.size()-1;
        int ndata=(nJobs<nWorkers)? nJobs: nWorkers;
        //int ndata=(nJobs<world.size()-1)? nJobs: world.size()-1;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(unsigned i=0; i < ndata; ++i){
            world.recv(mpi::any_source, outTag, jobOut);
            toXML(jobOut, root, xmlFile);
// add output here
        }

        fprintf(xmlFile, "</Complexes>\n");
        doc.SaveFile("PPL3Track.xml");       
        
        for(unsigned i=1; i < world.size(); ++i){
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;
            world.send(freeProc, jobTag, jobFlag);
        }

    } else {
        while (1) {

            world.send(0, rankTag, world.rank());

            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }
            // Receive parameters

            world.recv(0, inpTag, jobInput);
            
//            std::cout << "world.rank()=" << world.rank() 
//                    << " Rec=" << jobInput.recBuffer << " Lig=" << jobInput.ligBuffer << std::endl;
            if(!isRun(jobInput, jobOut, workDir)) {
                dockjob(jobInput, jobOut, workDir); 
            }
            world.send(0, outTag, jobOut);
        }
    }

    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;

    return (0);

}


