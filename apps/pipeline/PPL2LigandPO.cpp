/* 
 * File:   PPL2LigandPO.cpp
 * Author: zhang
 * 
 * Created on March 24, 2014, 11:17 AM
 */

#include "PPL2LigandPO.h"

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 * 
 */
bool PPL2LigandPO(int argc, char** argv, POdata& podata) {
    
    bool help;
    positional_options_description positional;
    
    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("sdf", value<std::string > (&podata.sdfFile), "input SDF file name")
                ("xmlout", value<std::string > (&podata.xmlOut)->default_value("PPL2Track.xml"), "xml tracking file name")
                ("output", value<std::string > (&podata.outputFile), "output filename")
                ("cmpName", value<std::string > (&podata.cmpName)->default_value("NoName"), "Use the SDF field property as ligand name (default no name)")
                ("version", value<int>(&podata.version)->default_value(10), "AMBER Version")
                ("restart", value<bool>(&podata.restart)->default_value(false), "To restart the calculation")
                ("firstLigID", value<int> (&podata.firstLigID)->default_value(1), "First ligID default from 1")
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
        
        if (vm.count("sdf") <= 0) {
            std::cerr << "Missing input SDF file.\n" << "\nCorrect usage:\n" << desc << '\n';
            return false;
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

