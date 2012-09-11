/* 
 * File:   StdResContainerSplTest.cpp
 * Author: zhang30
 *
 * Created on September 5, 2012, 5:34 PM
 */

#include <cstdlib>
#include <iostream>
#include "src/Structure/StdResContainer.h"
#include "src/Structure/Coor3d.h"
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
//    pPdb->standardlize(input, output);
    pPdb->standardlize2(input, "test-out.pdb");
    std::string cmd="reduce -Trim test-out.pdb> test-noh.pdb";
    system(cmd.c_str());

    cmd="reduce -BUILD test-noh.pdb> test-rd.pdb";
    system(cmd.c_str());
    
    pPdb->standardlize("test-rd.pdb", output);
    
}

void testRun(char** argv) {
    Coor3d coor1(1,1,1);
    Coor3d coor2(2,2,2);
    std::cout << coor1.dist2(coor2) << std::endl;
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
