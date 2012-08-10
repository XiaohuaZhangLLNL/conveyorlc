/* 
 * File:   DrugBankSplTest.cpp
 * Author: zhang30
 *
 * Created on Feb 15, 2012, 5:38:43 PM
 */

#include <stdlib.h>
#include <iostream>
#include "src/DataBase/DrugBank.h"

/*
 * Simple C++ Test Suite
 */

using namespace LBIND;

void testRun() {
    DrugBank drugBank;
    std::string xmlFile="drugbank.xml";
    drugBank.run(xmlFile);
    if (false /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testRun (DrugBankSplTest) message=error message sample" << std::endl;
    }
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% DrugBankSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testRun (DrugBankSplTest)" << std::endl;
    testRun();
    std::cout << "%TEST_FINISHED% time=0 testRun (DrugBankSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

