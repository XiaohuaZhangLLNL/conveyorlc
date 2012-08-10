/* 
 * File:   BaseStruct.cpp
 * Author: zhang
 * 
 * Created on August 23, 2010, 5:05 PM
 */

#include "BaseStruct.h"

namespace LBIND{
    
BaseStruct::BaseStruct(): itsID(0), itsName(""){
}

BaseStruct::BaseStruct(const BaseStruct& orig) {
}

BaseStruct::~BaseStruct() {
}

void BaseStruct::setID(const int& id)
{
    this->itsID=id;
}

int BaseStruct::getID()
{
    return this->itsID;
}

void BaseStruct::setName(const std::string name)
{
    this->itsName = name;
}

std::string BaseStruct::getName()
{
    return this->itsName;
}

}//namespace LBIND





