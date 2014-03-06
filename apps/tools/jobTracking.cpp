/* 
 * File:   JobTracking.cpp
 * Author: zhang30
 *
 * Created on September 17, 2012, 11:22 AM
 */

#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>

#include "XML/XMLHeader.hpp"
#include "Common/LBindException.h"
#include "Common/Tokenize.hpp"
#include "jobTrackingPO.h"

using namespace LBIND;

/*
 * 
 */
void saveSet(std::string& fileName, std::set<std::string>& strList){
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
            strList.insert(tokens[0]);
        }        
    }
    
}

int main(int argc, char** argv) {
    
    POdata podata;
    
    bool success=jobTrackingPO(argc, argv, podata);
    if(!success){
        return 1;
    }    
    
    std::set<std::string> list;
    saveSet(podata.listFile, list);
    
    XMLDocument doc(podata.xmlFile);
    bool loadOkay = doc.LoadFile();

    if (!loadOkay) {
        std::string mesg = doc.ErrorDesc();
        mesg = "Could not load elements.xml file.\nError: " + mesg;
        throw LBindException(mesg);
    } 
    
    XMLNode* fistNode;
    XMLNode* node;
    XMLNode* subNode;
    
    if(podata.isTmpFile){
        fistNode = doc.FirstChild("error");
        assert(fistNode);
    }else{    
        node = doc.FirstChild("Errors");
        assert(node);
        fistNode = node->FirstChild("error");
        assert(fistNode);
    }
    
    std::set<std::string> keyValues;

    for (subNode = fistNode; subNode != 0; subNode = subNode->NextSibling()) {
        XMLElement* itemElement = subNode->ToElement();
        bool print=false;
        std::string keyValue;
        for (XMLAttribute* pAttrib = itemElement->FirstAttribute(); pAttrib != 0; pAttrib = pAttrib->Next()) {
            std::string nameStr = pAttrib->NameTStr();
            
            if (nameStr == podata.keyword) {
                keyValue=pAttrib->ValueStr();
            }
            if (nameStr == "mesg") {
                if(pAttrib->ValueStr() == "Finished!"){
                    print=true;
                }
            }
        }
        if(print){
            keyValues.insert(keyValue);
        }

    }   
    
    std::vector<std::string> diffs;
    
    std::set_difference( list.begin(), list.end(), keyValues.begin(), keyValues.end(),
        std::back_inserter( diffs ) );

//    for(std::set<std::string>::iterator s=list.begin(); s!=list.end(); ++s){
//        std::cout << *s << std::endl;
//    }
//
//    for(std::set<std::string>::iterator s=keyValues.begin(); s!=keyValues.end(); ++s){
//        std::cout << *s << std::endl;
//    }    
    
    std::ofstream outFile;
    try {
        outFile.open(podata.outputFile.c_str());
    }
    catch(...){
        std::cout << "Cannot open file: " << podata.outputFile << std::endl;
    } 
    
    for(std::vector<std::string>::iterator s=diffs.begin(); s!=diffs.end(); ++s){
        outFile << *s << std::endl;
    }

    return 0;
}

