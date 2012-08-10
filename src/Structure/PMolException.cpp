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

PMolException::PMolException(std::string message) {
    this->message=message;
}

PMolException::PMolException(const PMolException& orig) {
}

PMolException::~PMolException() throw() {
}

std::string PMolException::what(){
    return message;
}

}//namespace LBIND





