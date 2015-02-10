/* 
 * File:   mmgbsa.cpp
 * Author: zhang30
 *
 * Created on April 16, 2014, 10:52 AM
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>


#include "MM/MMGBSA.h"
#include "Common/Tokenize.hpp"
#include "Structure/Constants.h"
#include "Common/File.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"

#include "PPL4mmgbsaPO.h"

#include <boost/scoped_ptr.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

namespace mpi = boost::mpi;

using namespace LBIND;

/*!
 * \breif mmgbsa MM-PB(GB)SA calculations on HPC using amber forcefield
 * \param argc
 * \param argv
 * \return success 
 * \defgroup mmgbsa_Commands mmgbsa Commands
 * 
 * Usage: mmgbsa <input-file>
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
        ar & dirBuffer;  
        ar & ligBuffer;
        ar & poseBuffer;
        ar & nonRes;
    }
 
    std::string dirBuffer;
    std::string ligBuffer;
    std::string poseBuffer;
    std::vector<std::string> nonRes;
};


//struct JobInputData{ 
//    int pbFlag;
//    char dirBuffer[100];
//    char ligBuffer[100];
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
        ar & score;
        ar & gbbind;
        ar & recID;
        ar & ligID;
        ar & poseID;
        ar & message;    
    }
    
    bool error;
    double score;
    double gbbind;
    std::string recID;
    std::string ligID;
    std::string poseID;
    std::string message;
       
};

//struct JobOutData{  
//    bool error;
//    char dirBuffer[100];
//    char ligBuffer[100];
//    char message[100];
//};

void toXML(JobOutData& jobOut, XMLElement* root, FILE* xmlTmpFile) {
    XMLElement * element = new XMLElement("Complex");

    XMLElement * pdbidEle = new XMLElement("RecID");
    XMLText * pdbidTx = new XMLText(jobOut.recID.c_str()); // has to use c-style string.
    pdbidEle->LinkEndChild(pdbidTx);
    element->LinkEndChild(pdbidEle);

    XMLElement * ligidEle = new XMLElement("LigID");
    XMLText * ligidTx = new XMLText(jobOut.ligID.c_str()); // has to use c-style string.
    ligidEle->LinkEndChild(ligidTx);
    element->LinkEndChild(ligidEle);

    XMLElement * pdbPathEle = new XMLElement("PoseID");
    XMLText * pdbPathTx = new XMLText(jobOut.poseID.c_str());
    pdbPathEle->LinkEndChild(pdbPathTx);
    element->LinkEndChild(pdbPathEle);

    XMLElement * scoreEle = new XMLElement("Vina");
    XMLText * scoreTx = new XMLText(Sstrm<std::string, double>(jobOut.score));
    scoreEle->LinkEndChild(scoreTx);
    element->LinkEndChild(scoreEle);

    XMLElement * gbEle = new XMLElement("GBSA");
    XMLText * gbTx = new XMLText(Sstrm<std::string, double>(jobOut.gbbind));
    gbEle->LinkEndChild(gbTx);
    element->LinkEndChild(gbEle);

    XMLElement * mesgEle = new XMLElement("Mesg");
    XMLText * mesgTx = new XMLText(jobOut.message);
    mesgEle->LinkEndChild(mesgTx);
    element->LinkEndChild(mesgEle);

    root->LinkEndChild(element);

    element->Print(xmlTmpFile, 1);
    fputs("\n", xmlTmpFile);
    fflush(xmlTmpFile);          
        
}


bool mmgbsa(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir) {
    
    boost::scoped_ptr<MMGBSA> pMMGBSA(new MMGBSA(jobInput.dirBuffer, jobInput.ligBuffer, jobInput.nonRes, workDir));
    pMMGBSA->run(jobInput.poseBuffer); 
  
    jobOut.gbbind=pMMGBSA->getbindGB(); 
    jobOut.score=pMMGBSA->getScore();
    jobOut.recID=jobInput.dirBuffer;
    jobOut.ligID=jobInput.ligBuffer;
    jobOut.poseID=jobInput.poseBuffer;
          
    return true;
}

struct XmlData{
    std::string recID;
    std::string ligID;  
    std::vector<std::string> poseIDs;
    std::vector<std::string> nonRes;
};

void saveStrList(std::string& xmlFile, std::vector<XmlData*>& xmlList){
    XMLDocument doc(xmlFile);
    bool loadOkay = doc.LoadFile();

    if (!loadOkay) {
        std::string mesg = doc.ErrorDesc();
        mesg = "Could not load PPL3Track.xml file.\nError: " + mesg;
        throw LBindException(mesg);
    }
    
    XMLNode* node = doc.FirstChild("Complexes");
    assert(node);
    XMLNode* comNode = node->FirstChild("Complex");
    assert(comNode);

    for (comNode = node->FirstChild("Complex"); comNode != 0; comNode = comNode->NextSibling("Complex")) {

        XMLNode* mesgNode = comNode->FirstChild("Mesg");
        assert(mesgNode);
        XMLText* mesgTx =mesgNode->FirstChild()->ToText();
        assert(mesgTx);
        std::string mesgStr = mesgTx->ValueStr();

        if(mesgStr=="Finished!"){  
            XmlData* pXmlData=new XmlData();            
            
            XMLNode* recIDnode = comNode->FirstChild("RecID");
            assert(recIDnode);
            XMLText* recIDtx =recIDnode->FirstChild()->ToText(); 
            pXmlData->recID=recIDtx->ValueStr();

            XMLNode* ligIDnode = comNode->FirstChild("LigID");
            assert(ligIDnode);
            XMLText* ligIDtx =ligIDnode->FirstChild()->ToText(); 
            pXmlData->ligID=ligIDtx->ValueStr();  
            
            XMLNode* scoresnode = comNode->FirstChild("Scores");
            assert(scoresnode); 
            
            for (XMLNode* poseNode = scoresnode->FirstChild(); poseNode != 0; poseNode = poseNode->NextSibling()) {
                std::string poseID=poseNode->ToElement()->Attribute("id");
//                std::cout << "Pose ID=" << poseID << std::endl;
                pXmlData->poseIDs.push_back(poseID);
            }
            
            XMLNode* nonstdAAsnode = comNode->FirstChild("NonStdAAList");
            assert(nonstdAAsnode); 
            
            for (XMLNode* poseNode = nonstdAAsnode->FirstChild(); poseNode != 0; poseNode = poseNode->NextSibling()) {
                std::string nonstdAA=poseNode->FirstChild()->ToText()->ValueStr();
//                std::cout << "Pose ID=" << poseID << std::endl;
                pXmlData->nonRes.push_back(nonstdAA);
            }             
            
            xmlList.push_back(pXmlData);
        }
        
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
    

    int jobFlag=1; // 1: doing job,  0: done job
    
    JobInputData jobInput;
    JobOutData jobOut;
        
    int rankTag=1;
    int jobTag=2;

    int inpTag=3;
    int outTag=4;

    mpi::timer runingTime;
    
    mpi::environment env(argc, argv);
    mpi::communicator world;  

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        return 1;
    }
    
    POdata podata;
    int error=0;
    
    if (world.rank() == 0) {        
        bool success=PPL4mmgbsaPO(argc, argv, podata);
        if(!success){
            error=1;
            return 1;
        }        
    }
   
    
    std::cout << "Number of tasks= " << world.size() << " My rank= " << world.rank() << std::endl;

    if (world.rank() == 0) {
        
        //! Tracking error using XML file
	XMLDocument doc;  
 	XMLDeclaration* decl = new XMLDeclaration( "1.0", "", "" );  
	doc.LinkEndChild( decl );  
 
	XMLElement * root = new XMLElement( "Complexes" );  
	doc.LinkEndChild( root );  

	XMLComment * comment = new XMLComment();
	comment->SetValue(" Tracking calculation error using XML file " );  
	root->LinkEndChild( comment );  
        
        FILE* xmlTmpFile=fopen("PPL4TrackTemp.xml", "w");
        fprintf(xmlTmpFile, "<?xml version=\"1.0\" ?>\n");
        fprintf(xmlTmpFile, "<Complexes>\n");
        fprintf(xmlTmpFile, "    <!-- Tracking calculation error using XML file -->\n");
//        root->Print(xmlTmpFile, 0);
//        fputs("\n",xmlTmpFile);
        fflush(xmlTmpFile);        
        //! END of XML header   
        
//        jobInput.pbFlag=1;
//        if(!podata.pbFlag){
//            jobInput.pbFlag=0;
//        }
     
        std::vector<XmlData*> xmlList;
        
        try{
            saveStrList(podata.recFile, xmlList);
        }catch(LBindException& e){
            std::cerr << "LBindException: " << e.what() << std::endl;
            return 1;
        }catch(...){
            std::cerr << "Input XML parser error." << std::endl;
            return 1;            
        }
//        
        //        std::vector<std::string> ligList;               
        //        saveStrList(podata.ligFile, ligList); 

        int count = 0;

        for (unsigned i = 0; i < xmlList.size(); ++i) {
            for (unsigned j = 0; j < xmlList[i]->poseIDs.size(); ++j) {
                ++count;
                if (count > world.size()-1) {
                     world.recv(mpi::any_source, outTag, jobOut);
                    toXML(jobOut, root, xmlTmpFile);
                }

                int freeProc;
                world.recv(mpi::any_source, rankTag, freeProc);
                std::cout << "At Process: " << freeProc 
                        << " Working on receptor: " << xmlList[i]->recID 
                        << " Ligand: " << xmlList[i]->ligID 
                        << " Pose: " << xmlList[i]->poseIDs[j] << std::endl;
                //                MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD);
                world.send(freeProc, jobTag, jobFlag);

                jobInput.dirBuffer = xmlList[i]->recID;
                jobInput.ligBuffer = xmlList[i]->ligID;
                jobInput.poseBuffer= xmlList[i]->poseIDs[j];
                jobInput.nonRes= xmlList[i]->nonRes;

                world.send(freeProc, inpTag, jobInput);

            }
        }
        
        int nJobs=count;
        int ndata=(nJobs<world.size()-1)? nJobs: world.size()-1;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(unsigned i=0; i < ndata; ++i){
//            MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
            world.recv(mpi::any_source, outTag, jobOut);
            toXML(jobOut, root, xmlTmpFile); 
        } 
        
        fprintf(xmlTmpFile, "</Complexes>\n");        
        doc.SaveFile( "PPL4Track.xml" );          
        
        for(unsigned i=1; i < world.size(); ++i){
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

//            MPI_Recv(&jobInput, sizeof(JobInputData), MPI_CHAR, 0, inpTag, MPI_COMM_WORLD, &status1);
            world.recv(0, inpTag, jobInput);
            
            jobOut.message="Finished!";
            try{
                jobOut.error=mmgbsa(jobInput, jobOut, workDir);            
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

