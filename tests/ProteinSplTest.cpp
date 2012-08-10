/* 
 * File:   ProteinSplTest.cpp
 * Author: zhang30
 *
 * Created on Dec 8, 2011, 9:27:08 AM
 */

#include <stdlib.h>
#include <iostream>
#include "src/BackBone/Protein.h"

/*
 * Simple C++ Test Suite
 */
using namespace LBIND;

void testProtein() {
    Protein protein();
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testProtein (ProteinSplTest) message=error message sample" << std::endl;
    }
}

void testProtein2() {
    int idVal;
    std::string nameVal;
    Protein protein(idVal, nameVal);
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testProtein2 (ProteinSplTest) message=error message sample" << std::endl;
    }
}

void testProtein3() {
    const Protein orig;
    Protein protein(orig);
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testProtein3 (ProteinSplTest) message=error message sample" << std::endl;
    }
}

void testDownloadPDB(std::string pdbid) {
    Protein protein;
    protein.downloadPDB(pdbid);
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testDocking (ProteinSplTest) message=error message sample" << std::endl;
    }
}

void testDocking() {
    Protein protein;
    protein.docking();
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testDocking (ProteinSplTest) message=error message sample" << std::endl;
    }
}

void testFlexDocking() {
    Protein protein;
    protein.flexDocking();
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testFlexDocking (ProteinSplTest) message=error message sample" << std::endl;
    }
}

void testMmGBSA() {
    Protein protein;
    protein.mmGBSA();
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testMmGBSA (ProteinSplTest) message=error message sample" << std::endl;
    }
}

void testScoring() {
    Protein protein;
    protein.scoring();
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testScoring (ProteinSplTest) message=error message sample" << std::endl;
    }
}

void testSinglePtMM() {
    Protein protein;
    protein.singlePtMM();
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testSinglePtMM (ProteinSplTest) message=error message sample" << std::endl;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% ProteinSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testProtein (ProteinSplTest)" << std::endl;
    testProtein();
    std::cout << "%TEST_FINISHED% time=0 testProtein (ProteinSplTest)" << std::endl;

    std::cout << "%TEST_STARTED% testProtein2 (ProteinSplTest)" << std::endl;
    testProtein2();
    std::cout << "%TEST_FINISHED% time=0 testProtein2 (ProteinSplTest)" << std::endl;

    std::cout << "%TEST_STARTED% testProtein3 (ProteinSplTest)" << std::endl;
    testProtein3();
    std::cout << "%TEST_FINISHED% time=0 testProtein3 (ProteinSplTest)" << std::endl;

    std::cout << "%TEST_STARTED% testDownloadPDB (ProteinSplTest)" << std::endl;
    testDownloadPDB("1OMB");
    std::cout << "%TEST_FINISHED% time=0 testDocking (ProteinSplTest)" << std::endl;    
    
    std::cout << "%TEST_STARTED% testDocking (ProteinSplTest)" << std::endl;
    testDocking();
    std::cout << "%TEST_FINISHED% time=0 testDocking (ProteinSplTest)" << std::endl;

    std::cout << "%TEST_STARTED% testFlexDocking (ProteinSplTest)" << std::endl;
    testFlexDocking();
    std::cout << "%TEST_FINISHED% time=0 testFlexDocking (ProteinSplTest)" << std::endl;

    std::cout << "%TEST_STARTED% testMmGBSA (ProteinSplTest)" << std::endl;
    testMmGBSA();
    std::cout << "%TEST_FINISHED% time=0 testMmGBSA (ProteinSplTest)" << std::endl;

    std::cout << "%TEST_STARTED% testScoring (ProteinSplTest)" << std::endl;
    testScoring();
    std::cout << "%TEST_FINISHED% time=0 testScoring (ProteinSplTest)" << std::endl;

    std::cout << "%TEST_STARTED% testSinglePtMM (ProteinSplTest)" << std::endl;
    testSinglePtMM();
    std::cout << "%TEST_FINISHED% time=0 testSinglePtMM (ProteinSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

