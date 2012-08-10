/* 
 * File:   amberTools.cpp
 * Author: zhang30
 *
 * Created on August 10, 2012, 10:15 AM
 */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include "src/MM/Amber.h"

using namespace LBIND;

/*
 * 
 */
int main(int argc, char** argv) {
    
    Amber* pAmber=new Amber();
    
    int charge=0;
    
    std::stringstream ss;
    
    ss << charge;
    
    std::string input="tmp.pdb";
    std::string output="antechamber.out";
    std::string options=" -c bcc -nc "+ ss.str();
  
    pAmber->antechamber(input, output, options);
    
    delete pAmber;

    return 0;
}

