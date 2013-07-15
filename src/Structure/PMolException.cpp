/* 
 * File:   PMolException.cpp
 * Author: zhang
 * 
 * Created on August 25, 2010, 10:23 AM
 */

#include "PMolException.h"
#include <string>

namespace LBIND{

PMolException::PMolException() {
}

PMolException::PMolException(const std::string mesg) {
    this->message=mesg;
}

PMolException::PMolException(const char* mesg){
    this->message=mesg;
}

PMolException::PMolException(const PMolException& orig) {
}

PMolException::~PMolException() throw() {
}

std::string PMolException::what(){
    return message;
}

}//namespace LBIND





