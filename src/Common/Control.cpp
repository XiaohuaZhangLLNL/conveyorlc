/* 
 * File:   Control.cpp
 * Author: zhang30
 * 
 * Created on January 19, 2012, 2:25 PM
 */

#include "Control.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <exception>
#include <string>

#include <boost/config.hpp>
#include <boost/program_options/detail/config_file.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options.hpp>
#include <boost/timer.hpp>

namespace po = boost::program_options;

namespace LBIND {

Control::Control() :
    output("control.out"),
    help(false),
    help_advanced(false)
{
}

Control::Control(const Control& orig) {
}

Control::~Control() {
}

double Control::getAngleCrit(){
    return this->angleCrit;
}

bool Control::parseInput(int argc, char** argv) {
    
    po::options_description inputs("Inputs:");
    inputs.add_options()
            ("input-file,i", po::value< std::string > (&input), "Input file")
            ;

    po::options_description outputs("Outputs:");
    outputs.add_options()
            ("output-file,o", po::value< std::string > (&output), "Output file")
            ;

    po::options_description params("Options:");
    params.add_options()
             ("printLev", po::value< int >(&printLev), "Print level: 0=print less, 1=default normal,2,3=excess")
             ;

    po::options_description helps("Help:");
    helps.add_options()
            ("help", po::bool_switch(&help), "display usage summary")
            ("help_advanced", po::bool_switch(&help_advanced), "display usage summary with advanced options")
            ;
    po::options_description desc, desc_advance;
    desc.add(inputs).add(outputs).add(params).add(helps);
    desc_advance.add(inputs).add(outputs).add(helps);

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        
        std::fstream tf(input.c_str(), std::fstream::in);
        po::store(po::parse_config_file(tf, params), vm);
        po::notify(vm);        

        if (help) {
            std::cout << desc << '\n';
            return false;
        }
        if (help_advanced) {
            std::cout << desc_advance << '\n';
            return false;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "\n\nAn unknown error occurred. " << std::endl;
        return false;
    }
    
    return true;
}

void Control::printHeader() {

    std::string logger;
    logger += " -------------------------------------------------------------------------- \n";
#ifdef USE_MPI
    logger += "|               LBIND Modeling Suite (Parallel Version)                    |\n";
#else    
    logger += "|               LBIND Modeling Suite  (Serial Version)                     |\n";
#endif    
    logger += "|          Authors: Xiaohua Zhang, Sergio Wong, Felice Lightstone          |\n";
    logger += "|                Biosciences & Biotechnology Division (BBTD)               |\n";
    logger += "|             Lawrence Livermore National Laboratory, CA 94551             |\n";
    logger += " -------------------------------------------------------------------------- \n";

    //    Logger::Instance()->writeToLogFile(logger);
    std::cout << logger << std::endl;
}

} //namespace LBIND 
