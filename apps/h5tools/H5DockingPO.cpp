//
// Created by Zhang, Xiaohua on 2019-04-01.
//

#include "H5DockingPO.h"


#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 *
 */
bool H5ReceptorPO(int argc, char** argv, POdata& podata) {

    bool help;
    positional_options_description positional;

    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("input,i", value<std::string > (&podata.dockInDir)->default_value("scratch/dockHDF5"), "path to dock HDF5 file Directory")
                ("output,o", value<std::string > (&podata.outputFile)->default_value("dock.csv"), "output meta CSV file")
        ;
        options_description info("Optional:");
        info.add_options()
                ("help,h", bool_switch(&help), "display usage summary")
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

    }catch (boost::filesystem::filesystem_error& e) {
        std::cerr << "\n\nFile system error: " << e.what() << '\n';
        return false;
    }catch (...) {
        std::cerr << "\n\nAn unknown error occurred. \n";
        return false;
    }



    return true;
}


