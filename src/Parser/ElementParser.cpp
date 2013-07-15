/* 
 * File:   ElementParser.cpp
 * Author: zhang
 * 
 * Created on August 23, 2010, 4:00 PM
 */
#include <iostream>

#include "ElementParser.h"

#include "Structure/Element.h"
#include "BaseParser.h"
#include "Common/LBindException.h"
#include "Structure/ElementContainer.h"

#include "XML/XMLHeader.hpp"

namespace LBIND {

ElementParser::ElementParser(ElementContainer* pEleCon) : BaseParser(),
        pElementContainer(pEleCon) {
}

ElementParser::ElementParser(const ElementParser& orig) {
}

ElementParser::~ElementParser() {
}

void ElementParser::read(std::string& fileName) {
    XMLDocument doc(fileName);
    bool loadOkay = doc.LoadFile();

    if (!loadOkay) {
        std::string mesg = doc.ErrorDesc();
        mesg = "Could not load elements.xml file.\nError: " + mesg;
        throw LBindException(mesg);
    }

    XMLNode* node = doc.FirstChild("elementlist");
    assert(node);
    XMLNode* subNode = node->FirstChild("element");
    assert(subNode);

    for (subNode = node->FirstChild(); subNode != 0; subNode = subNode->NextSibling()) {
        Element* pElement = pElementContainer->addElement();
        XMLElement* itemElement = subNode->ToElement();
        for (XMLAttribute* pAttrib = itemElement->FirstAttribute(); pAttrib != 0; pAttrib = pAttrib->Next()) {
            std::string nameStr = pAttrib->NameTStr();
            if (nameStr == "number") {
                pElement->setAtomicNumber(pAttrib->IntValue());
            }
            if (nameStr == "name") {
                pElement->setName(pAttrib->ValueStr());
            }
            if (nameStr == "symbol") {
                pElement->setElementSymbol(pAttrib->ValueStr());
            }
            if (nameStr == "mass") {
                pElement->setAtomicMass(pAttrib->DoubleValue());
            }
            if (nameStr == "valence") {
                pElement->setValence(pAttrib->IntValue());
            }
            if (nameStr == "covalentRadius") {
                pElement->setCovalentRadius(pAttrib->DoubleValue());
            }
            if (nameStr == "vdWRadius") {
                pElement->setVDWRadius(pAttrib->DoubleValue());
            }
            if (nameStr == "paulingEN") {
                pElement->setENV(pAttrib->DoubleValue());
            }
        }
        pElementContainer->setElementMap(pElement);
    }
}

} //namespace LBIND

