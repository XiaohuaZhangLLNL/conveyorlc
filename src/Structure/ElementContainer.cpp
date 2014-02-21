/* 
 * File:   ElementContainer.cpp
 * Author: zhang
 * 
 * Created on August 25, 2010, 10:43 AM
 */

#include "ElementContainer.h"
#include "Element.h"
#include "Common/LBindException.h"
#include "Common/Chomp.hpp"

namespace LBIND{

ElementContainer::ElementContainer() {
}

ElementContainer::ElementContainer(const ElementContainer& orig) {
}

ElementContainer::~ElementContainer() {
    for(ElementMapIterator it=itsElementMap.begin(); it!=itsElementMap.end(); ++it){
        delete it->second;
    }
    itsElementMap.clear();
}

Element* ElementContainer::addElement(){
    Element* pElement=new Element();
    return pElement;
//    this->itsElementMap[pElement->getElementSymbol()]=pElement;
}

void ElementContainer::setElementMap(Element* pElement){
    this->itsElementMap[pElement->getElementSymbol()]=pElement;
}

std::map<std::string, Element*> ElementContainer::getElementMap(){
    return this->itsElementMap;
}

Element* ElementContainer::symbolToElement(std::string symbol){
    chomp(symbol);
    for(ElementMapIterator it=itsElementMap.begin(); it!=itsElementMap.end(); ++it){
        if(it->first.compare(symbol)==0) return it->second;
    }
    std::string message="Symbol {" + symbol 
                      + "} is not defined in the element table.";
    throw LBindException(message);    
    return 0;
}
}//namespace LBIND






