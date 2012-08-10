/* 
 * File:   SstrmSplTest1.cpp
 * Author: zhang30
 *
 * Created on Jan 25, 2012, 2:51:20 PM
 */

#include <stdlib.h>
#include <iostream>
#include <sstream>
/*
 * Simple C++ Test Suite
 */

void test1() {

    int i= 12;
    std::stringstream ss;
    ss << i;

    std::cout << ss.str() << std::endl;
    std::cout << "SstrmSplTest test 1" << std::endl;
}

void test2() {
    std::cout << "SstrmSplTest test 2" << std::endl;
    std::cout << "%TEST_FAILED% time=0 testname=test2 (SstrmSplTest) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% SstrmSplTest" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (SstrmSplTest)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (SstrmSplTest)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (SstrmSplTest)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (SstrmSplTest)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}
