/* 
 * File:   AHA4mmgbsaPO.cpp
 * Author: zhang
 * 
 * Created on April 22, 2014, 1:58 PM
 */


#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

#include "AHA4mmgbsaPO.h"

using namespace boost::program_options;
/*
 * 
 */
bool AHA4mmgbsaPO(int argc, char** argv, POdata& podata) {
    
    bool help;
    positional_options_description positional;
    
    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("recFile", value<std::string > (&podata.recFile)->default_value("scratch/receptor.hdf5"), "receptor HDF5 file")
                ("ligFile", value<std::string > (&podata.ligFile)->default_value("scratch/ligand.hdf5"), "ligand HDF5 file")
                ("dockInFile", value<std::string > (&podata.dockInFile)->default_value(""), "Path to dock HDF5 files")
                ("ligDir", value<std::string > (&podata.ligDir)->default_value("lig"), "ligand input directory")
                ("version", value<int>(&podata.version)->default_value(16), "AMBER Version")
                ("intDiel", value<double>(&podata.intDiel)->default_value(4.0), "Solute dielectric constant")
                ("keep", bool_switch(&podata.keep)->default_value(false), "Keep all intermeidate files for fail calculation")
                ;
        options_description info("Optional:");
        info.add_options()
                ("help", bool_switch(&help), "display usage summary")
                ;
        options_description desc;
        desc.add(inputs).add(info);        

        variables_map vm;
        try {
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

        
    }catch (boost::filesystem::filesystem_error& e) {
        std::cerr << "\n\nFile system error: " << e.what() << '\n';
        return false;
    }catch (...) {
        std::cerr << "\n\nAn unknown error occurred. \n";
        return false;
    }
    
    
    
    return true;
}

