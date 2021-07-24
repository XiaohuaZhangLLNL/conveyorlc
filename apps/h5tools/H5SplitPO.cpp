//
// Created by Zhang, Xiaohua on 7/16/21.
//

#include "H5SplitPO.h"
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 *
 */
bool H5SplitPO(int argc, char** argv, POdata& podata) {

    bool help;
    positional_options_description positional;

    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("input,i", value<std::string > (&podata.inputFile)->default_value("receptor.hdf5"), "Input HDF5 files")
                ("output,o", value<std::string > (&podata.outputDir)->default_value("out"), "Merged HDF5 output file")
                ("num,n", value<int>(&podata.num)->default_value(0), "number of sphere vectors for surface")
                ("type,t", value<std::string > (&podata.type)->default_value("rec"), "File type: 1. lig (default) 2. rec 3. dock 4. gbsa")
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
