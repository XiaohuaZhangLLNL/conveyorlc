/* 
 * File:   NamdSplTest.cpp
 * Author: zhang30
 *
 * Created on Dec 20, 2011, 5:08:23 PM
 */

#include <stdlib.h>
#include <iostream>
#include "src/MM/Namd.h"
#include "src/BackBone/Protein.h"

/*
 * Simple C++ Test Suite
 */

using namespace LBIND;

void testRun() {
    
    Protein* pProtein=new Protein();
    pProtein->setPDBID("1UBQ");
//    pPortein->downloadPDB("1OMB");    
    Namd* pNamd=new Namd(pProtein);
    pNamd->run();
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testRun (NamdSplTest) message=error message sample" << std::endl;
    }
    
    delete pNamd;
    delete pProtein;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% NamdSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRun (NamdSplTest)" << std::endl;
    testRun();
    std::cout << "%TEST_FINISHED% time=0 testRun (NamdSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

