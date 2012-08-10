/* 
 * File:   ParmContainer.cpp
 * Author: zhang
 * 
 * Created on September 15, 2010, 3:32 PM
 */

#include "ParmContainer.h"
#include "ElementContainer.h"
#include "ElementParser.h"

#include <boost/scoped_ptr.hpp>

namespace LBIND{
ParmContainer::ParmContainer() {
}

ParmContainer::ParmContainer(const ParmContainer& orig) {
}

ParmContainer::~ParmContainer() {
//    delete pElementContainer;
}

ElementContainer* ParmContainer::addElementContainer(){

    std::string dataPath=getenv("LBindData");

    pElementContainer=new ElementContainer();
    boost::scoped_ptr<ElementParser> pElementParser(new ElementParser(pElementContainer));
    std::string fileName=dataPath+"/elements.xml";
    pElementParser->read(fileName);
    return pElementContainer;
}

ElementContainer* ParmContainer::getElementContainer(){
    return this->pElementContainer;
}

}//namespace LBIND
