/* 
 * File:   JobTrackSplTest.cpp
 * Author: zhang30
 *
 * Created on October 11, 2012, 9:52 AM
 */

#include <cstdlib>
#include <vector>

#include "XML/XMLHeader.hpp"
#include "Common/LBindException.h"

using namespace LBIND;

/*
 * 
 */

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



int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% JobTrackSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRun (JobTrackSplTest)" << std::endl;
    //testRun();
    std::vector<std::string> ligList;
    std::string xmlFile="JobTracking.xml";
    xmlEJobs(xmlFile,ligList);
    
    for(int i=0; i<ligList.size(); ++i){
        std::cout << ligList[i] << std::endl;
    }
    
    std::cout << "%TEST_FINISHED% time=0 testRun (JobTrackSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}
