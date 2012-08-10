/* 
 * File:   AutoDockSplTest.cpp
 * Author: zhang30
 *
 * Created on Jan 11, 2012, 4:35:59 PM
 */

#include <stdlib.h>
#include <iostream>
#include "src/MM/AutoDock.h"
#include "src/BackBone/Protein.h"
#include "src/BackBone/Ligand.h"

/*
 * Simple C++ Test Suite
 */

using namespace LBIND;

void testRun() {
    
    std::string pdbid="3KF4";
    std::string ligname="STI";
    
    Protein* pProtein=new Protein();
    pProtein->setPDBID(pdbid);
//    pProtein->downloadPDB("1FKO");
    pProtein->getLigandPDB(ligname);
    
    Ligand* pLigand=new Ligand();
    pLigand->setName(ligname);
    
    pProtein->addLigand(pLigand);    
    
    AutoDock* pAutoDock=new AutoDock(pProtein);
    pAutoDock->run();
    
    delete pAutoDock;
    delete pLigand;
    delete pProtein;
    
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testRun (AutoDockSplTest) message=error message sample" << std::endl;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% AutoDockSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRun (AutoDockSplTest)" << std::endl;
    testRun();
    std::cout << "%TEST_FINISHED% time=0 testRun (AutoDockSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

