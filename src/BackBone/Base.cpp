/* 
 * File:   Base.cpp
 * Author: zhang30
 * 
 * Created on December 6, 2011, 8:38 AM
 */

#include "Base.h"

namespace LBIND {
Base::Base() : id(0), name("") {
}

Base::Base(int idVal, std::string nameVal) : id(idVal), name(nameVal) {
}

Base::Base(const Base& orig) {
}

Base::~Base() {
}

void Base::setID(int idVal){
    id=idVal;
}

int Base::getID(){
    return id;
}

void Base::setName(std::string nameVal){
    name=nameVal;
}

std::string Base::getName(){
    return name;
}

}//namespace LBIND 



