/* 
 * File:   GromacsSplTest.cpp
 * Author: zhang30
 *
 * Created on Dec 12, 2011, 5:03:53 PM
 */

#include <stdlib.h>
#include <iostream>
#include "src/BackBone/Protein.h"
#include "src/MM/Gromacs.h"

/*
 * Simple C++ Test Suite
 */

using namespace LBIND;

void testRun() {
    Protein* pProtein=new Protein();
    pProtein->setPDBID("1OMB");
//    pPortein->downloadPDB("1OMB");
    
    Gromacs* pGromacs=new Gromacs(pProtein);
    pGromacs->run();
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testRun (GromacsSplTest) message=error message sample" << std::endl;
    }
    delete pGromacs;
    delete pProtein;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% GromacsSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRun (GromacsSplTest)" << std::endl;
    testRun();
    std::cout << "%TEST_FINISHED% time=0 testRun (GromacsSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

