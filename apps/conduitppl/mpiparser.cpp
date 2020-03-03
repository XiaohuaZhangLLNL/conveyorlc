#include <iostream>
#include <fstream>

#include <sstream>
#include <exception>
#include <stack>
#include <vector> // ligand paths
#include <cmath> // for ceila
#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

#include "VinaLC/parse_pdbqt.h"
#include "VinaLC/parallel_mc.h"
#include "VinaLC/file.h"
#include "VinaLC/cache.h"
#include "VinaLC/non_cache.h"
#include "VinaLC/naive_non_cache.h"
#include "VinaLC/parse_error.h"
#include "VinaLC/everything.h"
#include "VinaLC/weighted_terms.h"
#include "VinaLC/current_weights.h"
#include "VinaLC/quasi_newton.h"
//#include "gzstream.h"
//#include "tee.h"
#include "VinaLC/coords.h" // add_to_output_container
#include "VinaLC/tokenize.h"

#include "InitEnv.h"

//#include <mpi.h>
#include "dock.h"
#include "mpiparser.h"
#include "mainProcedure.h"
#include "XML/XMLHeader.hpp"
#include "Common/LBindException.h"

using namespace conduit;
using namespace LBIND;

void saveRecOLD(std::string& fileName, std::vector<std::string>& recList){
    Node n;

    hid_t rec_hid=relay::io::hdf5_open_file_for_read(fileName);
    relay::io::hdf5_read(rec_hid, n);

    NodeIterator itrRec = n["rec"].children();

    while(itrRec.has_next())
    {
        Node &nRec=itrRec.next();
        int status = 0;
        try {
            status = nRec["status"].as_int();
        }catch(...){
            std::cout << "Error in reading status of receptor ID " << nRec.name() << std::endl;
            continue;
        }        //std::cout << status << " " << nRec.name() << std::endl;
        if (status == 1)
        {
            recList.push_back(nRec.name());
        }
    }
    relay::io::hdf5_close_file(rec_hid);

}

void saveRec(std::string& fileName, std::vector<std::string>& recList){

    hid_t rec_hid=relay::io::hdf5_open_file_for_read(fileName);

    std::vector<std::string> rec_names;
    try {
        relay::io::hdf5_group_list_child_names(rec_hid, "/rec/", rec_names);
    }catch (...){
        std::cout << "Warning some error in receptor.hdf5" << std::endl;
    }

    std::cout << "Previous complete receptor " <<  rec_names.size() << std::endl;

    Node n_test;
    for(int i=0; i<rec_names.size(); i++){
        try {
            relay::io::hdf5_read(rec_hid, "rec/" + rec_names[i] + "/status", n_test);

            if (n_test.dtype().is_int()) {
                int status = n_test.as_int();
                if (status == 1) {
                    recList.push_back(rec_names[i]);
                }
            }
        }catch (...){
            std::cout << "Receptor " << rec_names[i] << " has status corrupted" << std::endl;
        }
    }

    relay::io::hdf5_close_file(rec_hid);
}

void saveLigWithCheck(std::string& fileName, std::vector<std::string>& ligList){

    hid_t lig_hid=relay::io::hdf5_open_file_for_read(fileName);

    std::vector<std::string> lig_names;
    try {
        relay::io::hdf5_group_list_child_names(lig_hid, "/lig/", lig_names);
    }catch (...){
        std::cout << "Warning some error in ligand.hdf5" << std::endl;
    }

    std::cout << "Previous complete ligand " <<  lig_names.size() << std::endl;

    Node n_test;
    for(int i=0; i<lig_names.size(); i++){
        try {
            relay::io::hdf5_read(lig_hid, "lig/" + lig_names[i] + "/status", n_test);

            if (n_test.dtype().is_int()) {
                int status = n_test.as_int();
                if (status == 1) {
                    ligList.push_back(lig_names[i]);
                }
            }
        }catch (...){
            std::cout << "Ligand " << lig_names[i] << " has status corrupted" << std::endl;
        }
    }

    relay::io::hdf5_close_file(lig_hid);
}

void saveLig(std::string& fileName, std::vector<std::string>& ligList){

    hid_t lig_hid=relay::io::hdf5_open_file_for_read(fileName);

    //std::vector<std::string> lig_names;
    try {
        relay::io::hdf5_group_list_child_names(lig_hid, "/lig/", ligList);
    }catch (...){
        std::cout << "Warning some error in ligand.hdf5" << std::endl;
    }

    std::cout << "Previous complete ligand " <<  ligList.size() << std::endl;

    //Node n_test;
    //for(int i=0; i<lig_names.size(); i++){
    //   try {
    //        relay::io::hdf5_read(lig_hid, "lig/" + lig_names[i] + "/status", n_test);

    //        if (n_test.dtype().is_int()) {
    //            int status = n_test.as_int();
    //            if (status == 1) {
    //                ligList.push_back(lig_names[i]);
    //            }
    //        }
    //    }catch (...){
    //        std::cout << "Ligand " << lig_names[i] << " has status corrupted" << std::endl;
    //   }
    //}

    relay::io::hdf5_close_file(lig_hid);
}

void saveLigOLD(std::string& fileName, std::vector<std::string>& ligList){
    Node n;

    hid_t lig_hid=relay::io::hdf5_open_file_for_read(fileName);
    relay::io::hdf5_read(lig_hid, n);

    NodeIterator itrLig = n["lig"].children();

    while(itrLig.has_next())
    {
        Node &nLig=itrLig.next();
        int status = 0;
        try {
            status =nLig["status"].as_int();
        }catch(...){
            std::cout << "Error in reading status of ligand ID " << nLig.name() << std::endl;
            continue;
        }
        if (status == 1)
        {
            ligList.push_back(nLig.name());
        }

    }
    relay::io::hdf5_close_file(lig_hid);
}

void saveCom(std::string& fileName, std::vector<std::string>& comList){
    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Cannot open file: " << fileName << std::endl;
    }

    const std::string comment="#";
    std::string fileLine;
    while(inFile){
        std::getline(inFile, fileLine);
        if(fileLine.compare(0, 1, comment)==0) continue;
        std::vector<std::string> tokens;
        tokenize(fileLine, tokens, " \n");
        if(tokens.size() == 2){
            comList.push_back(tokens[0]+"/"+tokens[1]);
        }
    }
}

int mpiParser(int argc, char* argv[],
        std::string& ligFile,
        std::string& recFile,
        std::vector<std::string>& ligList,
        std::vector<std::string>& recList,
        std::vector<std::string>& comList,
        std::vector<std::string>& fleList,
        JobInputData& jobInput){
    using namespace boost::program_options;
    const std::string version_string = "AutoDock Vina 1.1.2 (May 11, 2011)";
    const std::string error_message = "\n\n\
Please contact the author, Dr. Oleg Trott <ot14@columbia.edu>, so\n\
that this problem can be resolved. The reproducibility of the\n\
error may be vital, so please remember to include the following in\n\
your problem report:\n\
* the EXACT error message,\n\
* your version of the program,\n\
* the type of computer system you are running it on,\n\
* all command line options,\n\
* configuration file (if used),\n\
* ligand file as PDBQT,\n\
* receptor file as PDBQT,\n\
* flexible side chains file as PDBQT (if used),\n\
* output file as PDBQT (if any),\n\
* input (if possible),\n\
* random seed the program used (this is printed when the program starts).\n\
\n\
Thank you!\n";

    const std::string cite_message = "\
#################################################################\n\
# If you used AutoDock Vina in your work, please cite:          #\n\
#                                                               #\n\
# O. Trott, A. J. Olson,                                        #\n\
# AutoDock Vina: improving the speed and accuracy of docking    #\n\
# with a new scoring function, efficient optimization and       #\n\
# multithreading, Journal of Computational Chemistry 31 (2010)  #\n\
# 455-461                                                       #\n\
#                                                               #\n\
# DOI 10.1002/jcc.21334                                         #\n\
#                                                               #\n\
# Please see http://vina.scripps.edu for more information.      #\n\
#################################################################\n";


    std::string workDir;
    std::string inputDir;
    std::string dataPath;
    std::string localDir;

    std::string comFile;

    if(!initConveyorlcEnv(workDir, localDir, inputDir, dataPath)){
        return 1;
    }

    try {

        bool help;
        //jobInput.useScoreCF=false;
        
        positional_options_description positional; // remains empty
        
        options_description inputs("Input");
        inputs.add_options()
                ("recFile", value<std::string > (&recFile)->default_value("scratch/receptor.hdf5"), "receptor HDF5 file")
                ("ligFile", value<std::string > (&ligFile)->default_value("scratch/ligand.hdf5"), "ligand HDF5 file")
                ("comFile", value<std::string > (&comFile)->default_value(""), "Specify how receptor and ligand combine")
                ("exhaustiveness", value<int>(&(jobInput.exhaustiveness))->default_value(8), "exhaustiveness (default value 8) of the global search (roughly proportional to time): 1+")
                ("granularity", value<double>(&(jobInput.granularity))->default_value(0.375), "the granularity of grids (default value 0.375)")
                ("num_modes", value<int>(&jobInput.num_modes)->default_value(10), "maximum number (default value 10) of binding modes to generate")
                ("seed", value<int>(&jobInput.seed), "explicit random seed")
                ("randomize", bool_switch(&jobInput.randomize)->default_value(false), "Use different random seeds for complex")
                ("energy_range", value<fl> (&jobInput.energy_range)->default_value(2.0), "maximum energy difference (default value 2.0) between the best binding mode and the worst one displayed (kcal/mol)")
                ("min_rmsd", value<fl> (&jobInput.min_rmsd)->default_value(1.0), "maximum energy difference (default value 2.0) between the best binding mode and the worst one displayed (kcal/mol)")
                ("useScoreCF", bool_switch(&jobInput.useScoreCF)->default_value(false), "Use score cutoff to save ligand with top score higher than certain critical value")
                ("scoreCF", value<double>(&jobInput.scoreCF)->default_value(-8.0), "Score cutoff to save ligand with top score higher than certain value (default -8.0)")
                ("score_only", bool_switch(&jobInput.score_only)->default_value(false), "score only and not perform pose search")
                ("local_only",     bool_switch(&jobInput.local_only)->default_value(false), "do local search only")
                ("randomize_only", bool_switch(&jobInput.randomize_only)->default_value(false), "randomize input, attempting to avoid clashes")
                ;
        options_description info("Information (optional)");
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
            return 1;
        } 
        
        if (help) {
            std::cout << desc << '\n';
            return 0;
        }    
        
        if (vm.count("recFile") <= 0) {
            std::cerr << "Missing receptor List file.\n" << "\nCorrect usage:\n" << desc << '\n';
            return 1;
        }else{
            jobInput.recFile=workDir+"/"+recFile;
            saveRec(jobInput.recFile, recList);
        }
        
        if (vm.count("ligFile") <= 0) {
            std::cerr << "Missing ligand List file.\n" << "\nCorrect usage:\n" << desc << '\n';
            return 1;
        }else{
            jobInput.ligFile=workDir+"/"+ligFile;
            saveLig(jobInput.ligFile, ligList);
        }

        if(comFile!=""){
            jobInput.comFile=workDir+"/"+comFile;
            saveCom(jobInput.comFile, comList);
        }else{
            jobInput.comFile="";
        }
        
        if (jobInput.cpu < 1)
            jobInput.cpu = 1;
        if (vm.count("seed") == 0)
            jobInput.seed = auto_seed();
        if (jobInput.exhaustiveness < 1)
            throw usage_error("exhaustiveness must be 1 or greater");
        if (jobInput.num_modes < 1)
            throw usage_error("num_modes must be 1 or greater");        
        
    } catch (file_error& e) {
        std::cerr << "\n\nError: could not open \"" << e.name.string() << "\" for " << (e.in ? "reading" : "writing") << ".\n";
        return 1;
    } catch (boost::filesystem::filesystem_error& e) {
        std::cerr << "\n\nFile system error: " << e.what() << '\n';
        return 1;
    } catch (usage_error& e) {
        std::cerr << "\n\nUsage error: " << e.what() << ".\n";
        return 1;
    } catch (parse_error& e) {
        std::cerr << "\n\nParse error on line " << e.line << " in file \"" << e.file.string() << "\": " << e.reason << '\n';
        return 1;
    } catch (std::bad_alloc&) {
        std::cerr << "\n\nError: insufficient memory!\n";
        return 1;
    }// Errors that shouldn't happen:
    catch (std::exception& e) {
        std::cerr << "\n\nAn error occurred: " << e.what() << ". " << error_message;
        return 1;
    } catch (internal_error& e) {
        std::cerr << "\n\nAn internal error occurred in " << e.file << "(" << e.line << "). " << error_message;
        return 1;
    } catch (...) {
        std::cerr << "\n\nAn unknown error occurred. " << error_message;
        return 1;
    }
    
    return 0;
}

