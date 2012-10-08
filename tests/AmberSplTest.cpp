/* 
 * File:   AmberSplTest.cpp
 * Author: zhang30
 *
 * Created on Jan 5, 2012, 2:46:50 PM
 */

#include <stdlib.h>
#include <iostream>
#include <cmath>
#include "src/MM/Amber.h"
#include "src/BackBone/Protein.h"
#include "src/BackBone/Ligand.h"
#include "src/Parser/SanderOutput.h"
#include "src/Structure/Constants.h"

#include <boost/scoped_ptr.hpp>
/*
 * Simple C++ Test Suite
 */

using namespace LBIND;

void testRun() {
    
    std::string pdbid="1FKO";
    std::string ligname="EFZ";
    
    Protein* pProtein=new Protein();
    pProtein->setPDBID(pdbid);
//    pProtein->downloadPDB("1FKO");
    pProtein->getLigandPDB(ligname);
    
    Ligand* pLigand=new Ligand();
    pLigand->setName(ligname);
    
    pProtein->addLigand(pLigand);
    
    Amber* pAmber=new Amber(pProtein);
    pAmber->run();
    
    delete pAmber;
    delete pLigand;
    delete pProtein;
    
    if (true /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testRun (AmberSplTest) message=error message sample" << std::endl;
    }
}

void testSanderOut(){
    boost::scoped_ptr<SanderOutput> pSanderOutput(new SanderOutput());
    std::string sanderOut="LIG_minGB.out";
    double ligGBen=0;
    bool success=pSanderOutput->getEnergy(sanderOut, ligGBen);
    
    if(!success){        
        std::string message="Ligand GB minimization fails.";
        std::cout << message << std::endl;
    }    
    
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% AmberSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRun (AmberSplTest)" << std::endl;
    //testRun();
    testSanderOut();
    std::cout << "%TEST_FINISHED% time=0 testRun (AmberSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

