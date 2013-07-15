/* 
 * File:   jobTrackingPO.cpp
 * Author: zhang30
 *
 * Created on September 17, 2012, 11:37 AM
 */

#include <cstdlib>

#include "jobTrackingPO.h"

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 * 
 */
bool jobTrackingPO(int argc, char** argv, POdata& podata) {
    
    bool help;
    positional_options_description positional;
    
    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("xml", value<std::string > (&podata.xmlFile), "input XML file name")
                ("list", value<std::string > (&podata.listFile), "input XML file name")
                ("output", value<std::string > (&podata.outputFile), "output filename")
                ("keyword", value<std::string > (&podata.keyword), "output filename")
                ("tmp", bool_switch(&podata.isTmpFile), "temporary XML file type")
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

        if (vm.count("xml") <= 0) {
            std::cerr << "Missing input XML file name.\n" << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }
        
        if (vm.count("list") <= 0) {
            std::cerr << "Missing input list file name.\n" << "\nCorrect usage:\n" << desc << '\n';
            return false;
        }
        
        if (vm.count("output") <= 0) {
            std::cerr << "Missing output file name.\n" << "\nCorrect usage:\n" << desc << '\n';
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

