/* 
 * File:   CDT4mmgbsaPO.cpp
 * Author: zhang
 * 
 * Created on April 22, 2014, 1:58 PM
 */

#include "CDT4mmgbsaPO.h"

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 * 
 */
bool CDT4mmgbsaPO(int argc, char** argv, POdata& podata) {
    
    bool help;
    positional_options_description positional;
    
    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("comXML", value<std::string > (&podata.recFile), "CDT3Track XML file")
//                ("ligList", value<std::string > (&podata.ligFile), "ligand directory file name")
//                ("output", value<std::string > (&podata.outputFile), "output filename")
                ("xml", value<std::string > (&podata.xmlFile), "JobTracking XML filename")
                ("restart", value<bool>(&podata.restart)->default_value(false), "Flag to restart calculation")
                ("version", value<int>(&podata.version)->default_value(10), "AMBER Version")
//                ("PB", value<bool>(&podata.pbFlag)->default_value(true), "Turn on/off PB calculation (Default on).")
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
            return 0;
        }

        if (vm.count("comXML") <= 0) {
            std::cerr << "Missing receptor directory file name.\n" << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }          

//        if (vm.count("ligList") <= 0) {
//            std::cerr << "Missing ligand directory file name.\n" << "\nCorrect usage:\n" << desc << '\n';
//            return false;
//        } 
        
    }catch (boost::filesystem::filesystem_error& e) {
        std::cerr << "\n\nFile system error: " << e.what() << '\n';
        return false;
    }catch (...) {
        std::cerr << "\n\nAn unknown error occurred. \n";
        return false;
    }
    
    
    
    return true;
}

