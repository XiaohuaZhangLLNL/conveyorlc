/* 
 * File:   glideGBSAPO.cpp
 * Author: zhang30
 * 
 * Created on September 24, 2012, 11:33 AM
 */

#include "glideGBSAPO.h"

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 * 
 */
bool glideGBSAPO(int argc, char** argv, POdata& podata) {
    
    bool help;
    positional_options_description positional;
    
    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("sdf", value<std::string > (&podata.sdfFile), "input SDF file name")
                ("xmlout", value<std::string > (&podata.xmlOut)->default_value("JobTracking.xml"), "xml tracking file name")
                ("output", value<std::string > (&podata.outputFile), "output filename")
//                ("restart", value<bool>(&podata.restart)->default_value(false), "Flag to restart failed calculation")
                ("xmlrst", value<std::string > (&podata.xmlRst), "xml restart file name")
                ;   
        options_description info("Optional:");
        info.add_options()
                ("help", bool_switch(&help), "display usage summary")
                ;
        options_description desc;
        desc.add(inputs).add(info);        

        variables_map vm;
        try {
            //store(parse_command_line(argc, argv, desc, command_line_style::default_style ^ command_line_style::allow_guessing), vm);
            store(command_line_parser(argc, argv)
                    .options(desc)
                    .style(command_line_style::default_style ^ command_line_style::allow_guessing)
                    .positional(positional)
                    .run(),
                    vm);
            notify(vm);
        } catch (boost::program_options::error& e) {
            std::cerr << "Command line parse error: " << e.what() << '\n' << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }  
        
        if (help) {
            std::cout << desc << '\n';
            return false;
        }

        podata.restart=false;
        
        if (vm.count("sdf") <= 0 && vm.count("xmlrst") <=0) {
            std::cerr << "Missing input SDF file or restart xml file.\n" << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }
                
        if(vm.count("xmlrst") >0){ 
            podata.restart=true;
        }
        
        
    }catch (boost::filesystem::filesystem_error& e) {
        std::cerr << "\n\nFile system error: " << e.what() << '\n';
        return false;
    }catch (...) {
        std::cerr << "\n\nAn unknown error occurred. \n";
        return false;
    }
    
    
    
    return true;
}


