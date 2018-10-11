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
#include <string>


#include "MM/MMGBSA.h"
#include "Common/Tokenize.hpp"
#include "Structure/Constants.h"
#include "Common/File.hpp"
#include "Common/Command.hpp"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"

#include "PPL4mmgbsaPO.h"

#include <boost/scoped_ptr.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <mpi.h>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

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

class JobInputData {
public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & dirBuffer;
        ar & ligBuffer;
        ar & poseIDs;
    }

    std::string dirBuffer;
    std::string ligBuffer;
    std::vector<std::string> poseIDs;
};


//struct JobInputData{ 
//    int pbFlag;
//    char dirBuffer[100];
//    char ligBuffer[100];
//};

class JobOutData {
public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & error;
        ar & recID;
        ar & ligID;
        ar & message;
    }

    bool error;
    std::string recID;
    std::string ligID;
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

    XMLElement * mesgEle = new XMLElement("Mesg");
    XMLText * mesgTx = new XMLText(jobOut.message);
    mesgEle->LinkEndChild(mesgTx);
    element->LinkEndChild(mesgEle);

    root->LinkEndChild(element);

    element->Print(xmlTmpFile, 1);
    fputs("\n", xmlTmpFile);
    fflush(xmlTmpFile);

}

bool isRun(std::string& checkfile) {

    std::ifstream inFile(checkfile.c_str());

    if (!inFile) {
        return false;
    }

    return true;
}

struct PoseData {
    std::string recID;
    std::string ligID;
    std::string poseID;
    double score;
    double gbind;
    std::string mesg;
};

void checkPoint(std::string& checkfile, std::vector<PoseData>& poseDataVec) {

    std::ofstream outFile(checkfile.c_str());
    for (int i = 0; i < poseDataVec.size(); ++i) {
        PoseData poseData = poseDataVec[i];
        outFile << "recID:" << poseData.recID << "|"
                << "ligID:" << poseData.ligID << "|"
                << "poseID:" << poseData.poseID << "|"
                << "score:" << poseData.score << "|"
                << "gbind:" << poseData.gbind << "|"
                << "mesg:" << poseData.mesg
                << "\n";
    }
    outFile.close();
}

bool getPoseCheckData(std::string& checkfile, PoseData& poseData) {

    std::ifstream inFile(checkfile.c_str());

    if (!inFile) {
        return false;
    }

    std::string fileLine = "";
    std::string delimiter = ":";
    while (inFile) {
        std::getline(inFile, fileLine);
        std::vector<std::string> tokens;
        tokenize(fileLine, tokens, delimiter);

        if (tokens.size() != 2) continue;
        if (tokens[0] == "vina") {
            poseData.score = Sstrm<double, std::string>(tokens[1]);
        }
        if (tokens[0] == "GBSA") {
            poseData.gbind = Sstrm<double, std::string>(tokens[1]);
        }
        if (tokens[0] == "Mesg") {
            poseData.mesg = tokens[1];
        }
    }
    return true;
}

void removeDir(JobOutData& jobOut){
    std::string cmd="rm -rf lig_" + jobOut.ligID;
    std::string errMesg="rm exited abnormaly";
    
    command(cmd, errMesg);
    
    return;
}

bool postprocess(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir, std::string& inputDir) {

    jobOut.recID = jobInput.dirBuffer;
    jobOut.ligID = jobInput.ligBuffer;

    std::string ligCheckfile = workDir + "/scratch/com/" + jobOut.recID + "/gbsa/lig_" + jobOut.ligID + "_checkpoint.txt";
    std::string ligTarfile   = workDir + "/scratch/com/" + jobOut.recID + "/gbsa/lig_" + jobOut.ligID + ".tar.gz";
    
    std::vector<PoseData> poseDataVec;
    
    try {
        if (isRun(ligCheckfile) && isRun(ligTarfile)){
            std::string ligDir=workDir + "/scratch/com/" + jobOut.recID + "/gbsa/lig_" + jobOut.ligID;
            if(fileExist(ligDir)){
                removeDir(jobOut);
            }
            return true;
        }
    
        for (int i = 0; i < jobInput.poseIDs.size(); ++i) {
            std::string poseID = jobInput.poseIDs[i];
            std::string poseCheckfile = workDir + "/scratch/com/" + jobOut.recID + "/gbsa/lig_" + jobOut.ligID + "/pose_" + poseID + "/checkpoint.txt";
            //std::cout << poseCheckfile << std::endl;
            if (isRun(poseCheckfile)) {
                PoseData poseData;
                poseData.recID = jobOut.recID;
                poseData.ligID = jobOut.ligID;
                poseData.poseID = poseID;
                getPoseCheckData(poseCheckfile, poseData);
                poseDataVec.push_back(poseData);
            } else {
                std::string message = "Pose " + poseID + " has not been completed";
                throw LBindException(message);                
            }
        }

        std::string ligDir = workDir + "/scratch/com/" + jobOut.recID + "/gbsa";
        chdir(ligDir.c_str());
        std::string cmd = "tar -zcf lig_" + jobOut.ligID + ".tar.gz lig_" + jobOut.ligID;
        std::string errMesg="tar exited abnormaly";
        command(cmd, errMesg);   
        
        std::string ligTarfile="lig_" + jobOut.ligID + ".tar.gz";
        if(!fileExist(ligTarfile)) throw LBindException(ligTarfile+" not exist");
        if(fileEmpty(ligTarfile)) throw LBindException(ligTarfile+" is empty");
       
        removeDir(jobOut);
        
    } catch (LBindException& e) {
        jobOut.message = e.what();
        return false;
    }
    checkPoint(ligCheckfile, poseDataVec);
    return true;
}

int main(int argc, char** argv) {

    //! get  working directory
    char* WORKDIR = getenv("WORKDIR");
    std::string workDir;
    if (WORKDIR == 0) {
        // use current working directory for working directory
        char BUFFER[200];
        getcwd(BUFFER, sizeof (BUFFER));
        workDir = BUFFER;
    } else {
        workDir = WORKDIR;
    }

    //! get  input directory
    char* INPUTDIR = getenv("INPUTDIR");
    std::string inputDir;
    if (INPUTDIR == 0) {
        // use current working directory for input directory
        char BUFFER[200];
        getcwd(BUFFER, sizeof (BUFFER));
        inputDir = BUFFER;
    } else {
        inputDir = INPUTDIR;
    }

    int jobFlag = 1; // 1: doing job,  0: done job

    JobInputData jobInput;
    JobOutData jobOut;

    int rankTag = 1;
    int jobTag = 2;

    int inpTag = 3;
    int outTag = 4;

    mpi::environment env(argc, argv);
    mpi::communicator world;
    mpi::timer runingTime;

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        return 1;
    }

    POdata podata;
    int error = 0;

    if (world.rank() == 0) {
        bool success = PPL4mmgbsaPO(argc, argv, podata);
        if (!success) {
            error = 1;
            return 1;
        }
    }

    std::cout << "Number of tasks= " << world.size() << " My rank= " << world.rank() << std::endl;

    if (world.rank() == 0) {

        //! Tracking error using XML file
        XMLDocument doc;
        XMLDeclaration* decl = new XMLDeclaration("1.0", "", "");
        doc.LinkEndChild(decl);

        XMLElement * root = new XMLElement("Complexes");
        doc.LinkEndChild(root);

        XMLComment * comment = new XMLComment();
        comment->SetValue(" Tracking calculation error using XML file ");
        root->LinkEndChild(comment);

        FILE* xmlTmpFile = fopen("PPL4PostProcessTemp.xml", "w");
        fprintf(xmlTmpFile, "<?xml version=\"1.0\" ?>\n");
        fprintf(xmlTmpFile, "<Complexes>\n");
        fprintf(xmlTmpFile, "    <!-- Tracking calculation error using XML file -->\n");

        fflush(xmlTmpFile);

        int count = 0;

        XMLDocument ppl3doc(podata.recFile);
        bool loadOkay = ppl3doc.LoadFile();

        if (!loadOkay) {
            std::string mesg = ppl3doc.ErrorDesc();
            mesg = "Could not load PPL3Track.xml file.\nError: " + mesg;
            throw LBindException(mesg);
        }

        XMLNode* node = ppl3doc.FirstChild("Complexes");
        assert(node);
        XMLNode* comNode = node->FirstChild("Complex");
        assert(comNode);

        for (comNode = node->FirstChild("Complex"); comNode != 0; comNode = comNode->NextSibling("Complex")) {

            XMLNode* mesgNode = comNode->FirstChild("Mesg");
            assert(mesgNode);
            XMLText* mesgTx = mesgNode->FirstChild()->ToText();
            assert(mesgTx);
            std::string mesgStr = mesgTx->ValueStr();

            if (mesgStr == "Finished!") {
                ++count;
                if (count > world.size() - 1) {
                    world.recv(mpi::any_source, outTag, jobOut);
                    toXML(jobOut, root, xmlTmpFile);
                }

                XMLNode* recIDnode = comNode->FirstChild("RecID");
                assert(recIDnode);
                XMLText* recIDtx = recIDnode->FirstChild()->ToText();


                XMLNode* ligIDnode = comNode->FirstChild("LigID");
                assert(ligIDnode);
                XMLText* ligIDtx = ligIDnode->FirstChild()->ToText();

                jobInput.dirBuffer = recIDtx->ValueStr();
                jobInput.ligBuffer = ligIDtx->ValueStr();

                XMLNode* scoresnode = comNode->FirstChild("Scores");
                assert(scoresnode);                
 
		jobInput.poseIDs.clear();
                for (XMLNode* poseNode = scoresnode->FirstChild(); poseNode != 0; poseNode = poseNode->NextSibling()) {
                    std::string poseID = poseNode->ToElement()->Attribute("id");
                    jobInput.poseIDs.push_back(poseID);
                    //std::cout << " " << poseID << "," ;
                }
                //std::cout << std::endl;
                
                int freeProc;
                world.recv(mpi::any_source, rankTag, freeProc);
                std::cout << "At Process: " << freeProc
                        << " Working on receptor: " << jobInput.dirBuffer
                        << " Ligand: " << jobInput.ligBuffer
                        << " Number of poses:" << jobInput.poseIDs.size()
                        << std::endl;
                //                MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD);
                world.send(freeProc, jobTag, jobFlag);

                world.send(freeProc, inpTag, jobInput);
            }
        }

        int nJobs = count;
        int nWorkers = world.size() - 1;
        int ndata = (nJobs < nWorkers) ? nJobs : nWorkers;
        //int ndata=(nJobs<world.size()-1)? nJobs: world.size()-1;
        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;

        for (unsigned i = 0; i < ndata; ++i) {
            //            MPI_Recv(&jobOut, sizeof(JobOutData), MPI_CHAR, MPI_ANY_SOURCE, outTag, MPI_COMM_WORLD, &status2);
            world.recv(mpi::any_source, outTag, jobOut);
            toXML(jobOut, root, xmlTmpFile);
        }

        fprintf(xmlTmpFile, "</Complexes>\n");
        doc.SaveFile("PPL4PostProcess.xml");

        for (unsigned i = 1; i < world.size(); ++i) {
            int freeProc;
            //            MPI_Recv(&freeProc, 1, MPI_INTEGER, MPI_ANY_SOURCE, rankTag, MPI_COMM_WORLD, &status1);
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag = 0;
            ;
            //            MPI_Send(&jobFlag, 1, MPI_INTEGER, freeProc, jobTag, MPI_COMM_WORLD); 
            world.send(freeProc, jobTag, jobFlag);
        }

    } else {
        while (1) {
            //            MPI_Send(&rank, 1, MPI_INTEGER, 0, rankTag, MPI_COMM_WORLD);
            world.send(0, rankTag, world.rank());
            //            MPI_Recv(&jobFlag, 20, MPI_CHAR, 0, jobTag, MPI_COMM_WORLD, &status2);
            world.recv(0, jobTag, jobFlag);
            if (jobFlag == 0) {
                break;
            }
            // Receive parameters

            //            MPI_Recv(&jobInput, sizeof(JobInputData), MPI_CHAR, 0, inpTag, MPI_COMM_WORLD, &status1);
            world.recv(0, inpTag, jobInput);

            jobOut.message = "Finished!";

            jobOut.error = postprocess(jobInput, jobOut, workDir, inputDir);

            //            MPI_Send(&jobOut, sizeof(JobOutData), MPI_CHAR, 0, outTag, MPI_COMM_WORLD);
            world.send(0, outTag, jobOut);

        }
    }

    std::cout << "Rank= " << world.rank() << " MPI Wall Time= " << runingTime.elapsed() << " Sec." << std::endl;

    return 0;
}

