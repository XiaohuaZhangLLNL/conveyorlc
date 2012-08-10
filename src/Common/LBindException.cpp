/* 
 * File:   LBindException.cpp
 * Author: zhang30
 * 
 * Created on December 12, 2011, 11:01 AM
 */

#include "LBindException.h"

namespace LBIND {

LBindException::LBindException() {
}

LBindException::LBindException(std::string mesg) {
    this->message=mesg;
}

LBindException::LBindException(const LBindException& orig) {
}

LBindException::~LBindException() throw() {
}

std::string LBindException::what(){
    return message;
}


}//namespace LBIND 
