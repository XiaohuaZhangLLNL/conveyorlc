//
// Created by Zhang, Xiaohua on 7/14/21.
//

//
// Created by Zhang, Xiaohua on 2019-03-29.
//

#include "H5LigandPO.h"

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

using namespace boost::program_options;
/*
 *
 */
bool H5LigandPO(int argc, char** argv, POdata& podata) {

    bool help;
    positional_options_description positional;

    try {
        options_description inputs("Required:");
        inputs.add_options()
                ("input,i", value<std::string > (&podata.inputFile)->default_value("scratch/receptor.hdf5"), "receptor HDF5 input file")
                ("output,o", value<std::string > (&podata.outputFile)->default_value("scratch/receptor_out.hdf5"), "receptor HDF5 output file")
                ("del,d", bool_switch(&podata.del), "delete all paths for failed calculations")
                ("name,n", value<std::string > (&podata.name)->default_value(""), "extract meta data and files by receptor name")
                ("delname,x", value<std::string > (&podata.delname)->default_value(""), "delete path by receptor name")
                ("storename,s", value<std::string > (&podata.storename)->default_value(""), "save data to HDF5 output file by receptor name")
                ("checkdata,c", value<std::vector<std::string> >(&podata.checkdata)->multitoken(), "update meta data by protein name and checkpoint file name (e.g. sarinXtalnAChE  checkpoint.txt)");
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



