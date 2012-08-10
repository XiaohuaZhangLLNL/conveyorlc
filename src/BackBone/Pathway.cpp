/* 
 * File:   Pathway.cpp
 * Author: zhang30
 * 
 * Created on December 6, 2011, 8:31 AM
 */

#include "Pathway.h"

namespace LBIND {
Pathway::Pathway(): Base() {
}

Pathway::Pathway(int idVal, std::string nameVal) : Base(idVal, nameVal)  {
}

Pathway::Pathway(const Pathway& orig) {
}

Pathway::~Pathway() {
}

} //namespace LBIND 



