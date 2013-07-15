/* 
 * File:   MySQLConnSplTest.cpp
 * Author: zhang30
 *
 * Created on Jan 26, 2012, 3:03:30 PM
 */

#include <stdlib.h>
#include <iostream>
#include "src/DataBase/MySQLConnector.h"

/*
 * Simple C++ Test Suite
 */
using namespace LBIND;

void testInitConnector(MySQLConnector& mySQLConnector) {

    bool result = mySQLConnector.initConnector();    
    if (!result /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testInitConnector (MySQLConnSplTest) message=error message sample" << std::endl;
    }
}

void testCreateTable(MySQLConnector& mySQLConnector){
    bool result = mySQLConnector.createTable("test", "id INT, label CHAR(1)");
    if (!result /*check result*/) {
        std::cout << "%TEST_FAILED% time=0 testname=testCreateTable (MySQLConnSplTest) message=error message sample" << std::endl;
    }    
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% MySQLConnSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;
    MySQLConnector mySQLConnector;
    std::cout << "%TEST_STARTED% testInitConnector (MySQLConnSplTest)" << std::endl;
    testInitConnector(mySQLConnector);
    std::cout << "%TEST_FINISHED% time=0 testInitConnector (MySQLConnSplTest)" << std::endl;
    
    std::cout << "%TEST_STARTED% testCreateTable (MySQLConnSplTest)" << std::endl;
    testCreateTable(mySQLConnector);
    std::cout << "%TEST_FINISHED% time=0 testCreateTable (MySQLConnSplTest)" << std::endl;
    
    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

