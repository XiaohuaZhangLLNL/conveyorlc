/* 
 * File:   calcSASAPO.cpp
 * Author: zhang
 * 
 * Created on February 20, 2014, 4:59 PM
 */

#include "calcSASAPO.h"

#include <cstdlib>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 * 
 */
bool calcSASAPO(int argc, char** argv, POdata& podata) {
    
    bool help;
    positional_options_description positional;
    
    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("pdb", value<std::string > (&podata.pdbFile), "input PDB file name")
//                ("output", value<std::string > (&podata.outputFile), "output filename")
                ;   
        options_description info("Optional:");
        info.add_options()
                ("radius", value<double>(&podata.radius)->default_value(1.40), "probe sphere radius (Default=1.4 angstroms).")
                ("surfSphNum", value<int>(&podata.surfSphNum)->default_value(960), "number of sphere vectors for surface (Default=960).")
                ("gridSphNum", value<int>(&podata.gridSphNum)->default_value(100), "number of sphere vectors for grid (Default=100).")
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

        if (vm.count("pdb") <= 0) {
            std::cerr << "Missing input PDB file name.\n" << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }
        
//        if (vm.count("output") <= 0) {
//            std::cerr << "Missing output file name.\n" << "\nCorrect usage:\n" << desc << '\n';
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


