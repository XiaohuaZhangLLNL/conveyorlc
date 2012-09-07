/* 
 * File:   StdResContainer.cpp
 * Author: zhang30
 * 
 * Created on September 5, 2012, 4:34 PM
 */

#include <algorithm>

#include "StdResContainer.h"
#include "Common/LBindException.h"
#include "XML/XMLHeader.hpp"

namespace LBIND{

StdResContainer::StdResContainer() {
    std::string dataPath=getenv("LBindData");
    std::string fileName=dataPath+"/residues.xml";
    parseXML(fileName);
    
}

StdResContainer::StdResContainer(const StdResContainer& orig) {
}

StdResContainer::~StdResContainer() {
}

void StdResContainer::parseXML(std::string& fileName) {
    XMLDocument doc(fileName);
    bool loadOkay = doc.LoadFile();

    if (!loadOkay) {
        std::string mesg = doc.ErrorDesc();
        mesg = "Could not load elements.xml file.\nError: " + mesg;
        throw LBindException(mesg);
    }

    XMLNode* topNode = doc.FirstChild("residuelist");
    assert(topNode);
    XMLNode* resNode = topNode->FirstChild("residue");
    assert(resNode);

    for (resNode = topNode->FirstChild(); resNode != 0; resNode = resNode->NextSibling()) {
        XMLNode* nameNode = resNode->FirstChild("name");
        XMLNode* textNode=nameNode->FirstChild();
//        std::cout << "Residue Name="<<textNode->Value() << " " << textNode->ValueStr() << std::endl;
        stdResidues.push_back(textNode->ValueStr());
//        XMLText* text=node->FirstChild("name");
//        std::cout << "Residue text="<<text->Value() << " " << text->ValueStr() << std::endl;
    }
    
     std::cout << "Residue List Size=" << stdResidues.size() << std::endl;
}

bool StdResContainer::find(const char* resName){
    std::string resN=resName;
    return this->find(resN);
}

bool StdResContainer::find(const std::string& resName){
     std::vector<std::string>::iterator pos=std::find(stdResidues.begin(), stdResidues.end(), resName);
     if(pos==stdResidues.end()){
         return false;
     }
     return true;
}

}//namespace LBIND
