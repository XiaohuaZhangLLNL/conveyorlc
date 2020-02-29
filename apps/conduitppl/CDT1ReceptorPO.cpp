/* 
 * File:   CDT1ReceptorPO.cpp
 * Author: zhang
 * 
 * Created on March 18, 2014, 12:10 PM
 */

#include "CDT1ReceptorPO.h"

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 * 
 */
bool CDT1ReceptorPO(int argc, char** argv, POdata& podata) {
    
    bool help;
    positional_options_description positional;
    
    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("input", value<std::string > (&podata.inputFile), "PDB file list for input")
                ("output", value<std::string > (&podata.outputFile), "output filename")
                ("version", value<int>(&podata.version)->default_value(13), "AMBER Version")
                ("radius", value<double>(&podata.radius)->default_value(1.40), "probe sphere radius")
                ("surfSphNum", value<int>(&podata.surfSphNum)->default_value(960), "number of sphere vectors for surface")
                ("gridSphNum", value<int>(&podata.gridSphNum)->default_value(100), "number of sphere vectors for grid")        
                ("spacing", value<double>(&podata.spacing)->default_value(1.0), "Grid spacing")
                ("cutoffCoef", value<double>(&podata.cutoffCoef)->default_value(1.1), "Cutoff Coefficient")
                ("boxExtend", value<double>(&podata.boxExtend)->default_value(2.0), "Extend box from grid dimension")
                ("minVolume", value<double>(&podata.minVol)->default_value(50), "minimum volume for a site")
                ("protonate", value<std::string> (&podata.protonateFlg)->default_value("on"), "Protonate protein by default")
                ("minimize", value<std::string> (&podata.minimizeFlg)->default_value("on"), "Run minimization by default")
                ("site", value<std::string> (&podata.siteFlg)->default_value("on"), "Run site calculation by default")
                ("sitebylig", value<std::string> (&podata.sitebylig)->default_value("off"), "Estimate site by ligand")
                ("cutProt", value<std::string> (&podata.cutProt)->default_value("off"), "Turn off cut protein by default")
                ("cutRadius", value<double>(&podata.cutRadius)->default_value(5.0), "Radius to cut the protein")
                ("forceRedo", value<std::string> (&podata.forceRedoFlg)->default_value("off"), "Not to redo calculation by default")
                ("keep", bool_switch(&podata.keep)->default_value(false), "Keep all intermeidate files")
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

        if (vm.count("input") <= 0) {
            std::cerr << "Missing pdb file list input.\n" << "\nCorrect usage:\n" << desc << '\n';
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


