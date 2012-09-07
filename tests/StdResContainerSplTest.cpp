/* 
 * File:   StdResContainerSplTest.cpp
 * Author: zhang30
 *
 * Created on September 5, 2012, 5:34 PM
 */

#include <cstdlib>
#include <iostream>
#include "src/Structure/StdResContainer.h"
#include "src/Parser/Pdb.h"

/*
 * 
 */

using namespace LBIND;

void testStdResContainer(){
    StdResContainer* pStdResContainer=new StdResContainer();
    if(pStdResContainer->find("MET")){
        std::cout << "MET is standard residue" << std::endl;
    }
    
    delete pStdResContainer;    
}

void testPDBstandardlize(char** argv){
    Pdb* pPdb=new Pdb();
    std::string input=argv[1];
    std::string output=argv[2];
    std::cout << "input=" << input << " output=" << output << std::endl;
    pPdb->standardlize(input, output);
}

void testRun(char** argv) {
    testStdResContainer();
    testPDBstandardlize(argv);
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% StdResContainerSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRun (StdResContainerSplTest)" << std::endl;
    testRun(argv);
    std::cout << "%TEST_FINISHED% time=0 testRun (StdResContainerSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}
