/* 
 * File:   MainSplTest.cpp
 * Author: zhang30
 *
 * Created on Jan 19, 2012, 1:58:24 PM
 */

#include <stdlib.h>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Common/LBindException.h"
#include "Common/Control.h"

/*
 * Simple C++ Test Suite
 */

using namespace LBIND;

int main(int argc, char** argv) {
    try {    
        boost::timer runingTime;

        boost::scoped_ptr<Control> pControl(new Control());
        pControl->printHeader();
        bool success=pControl->parseInput(argc, argv);
        if(!success) return(EXIT_FAILURE); 
        {
            boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
            std::cout <<"BEGIN TIME: " << now << std::endl;
        }
        
        
        
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        std::cout <<"END   TIME: " << now << std::endl;        
    }
    catch(LBindException& pE){
        std::cerr << "error: " << pE.what() << std::endl;
        return (EXIT_FAILURE);        
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return (EXIT_FAILURE);
    }
    catch(...) {
        std::cerr << "Exception of unknown type!" << std::endl;
        return (EXIT_FAILURE);
    }    

    return (EXIT_SUCCESS);
}

