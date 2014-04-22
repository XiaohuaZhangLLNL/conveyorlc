/*

   Copyright (c) 2006-2010, The Scripps Research Institute

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Author: Dr. Oleg Trott <ot14@columbia.edu>, 
           The Olson Lab, 
           The Scripps Research Institute

 */
// which copy
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <stack>
#include <vector> // ligand paths
#include <cmath> // for ceila
#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename
#include <boost/thread/thread.hpp> // hardware_concurrency // FIXME rm ?
#include "parse_pdbqt.h"
#include "parallel_mc.h"
#include "file.h"
#include "cache.h"
#include "non_cache.h"
#include "naive_non_cache.h"
#include "parse_error.h"
#include "everything.h"
#include "weighted_terms.h"
#include "current_weights.h"
#include "quasi_newton.h"
//#include "gzstream.h"
//#include "tee.h"
#include "coords.h" // add_to_output_container
#include "tokenize.h"
#include "dock.h"


#include "mainProcedure.h"


void getScores(std::string& log, std::vector<double>& scores){

    std::vector<std::string> lines;
    tokenize(log, lines, "\n");  
    if(lines.size()<5) return;

    for (unsigned i = 4; i < lines.size(); ++i) {
        std::vector<std::string> values;
        tokenize(lines[i], values);
        if (values.size() > 2) {
            double score = Sstrm<double, std::string>(values[1]);
            scores.push_back(score);
        }
    }
    return;
}

int dockjob(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir){
    try {
//        std::string flex_name, config_name, out_name, log_name;
        int seed=jobInput.seed;
        int verbosity = 1;
        int num_modes = jobInput.num_modes;
        fl energy_range = jobInput.energy_range;

        // -0.035579, -0.005156, 0.840245, -0.035069, -0.587439, 0.05846
        fl weight_gauss1 = -0.035579;
        fl weight_gauss2 = -0.005156;
        fl weight_repulsion = 0.840245;
        fl weight_hydrophobic = -0.035069;
        fl weight_hydrogen = -0.587439;
        fl weight_rot = 0.05846;
        bool score_only = false, local_only = false, randomize_only = false; // FIXME

        std::string dockDir = workDir + "/scratch/com/" + jobInput.recBuffer + "/dock/" + jobInput.ligBuffer;
        std::string cmd = "mkdir -p " + dockDir;
        system(cmd.c_str());
        // cd to the rec directory to performance calculation
        chdir(dockDir.c_str());
        
        
        
        std::string ligand_name = jobInput.ligBuffer;
//        std::string rigid_name = jobInput.recBuffer;
        std::string rigid_name =workDir+"/scratch/com/"+jobInput.recBuffer+"/rec/"+jobInput.recBuffer+".pdbqt";
        std::string flex_name = jobInput.fleBuffer;
        int exhaustiveness=jobInput.exhaustiveness;
        int cpu=jobInput.cpu;
        
        std::stringstream ligSS;
        std::string pdbqtPath=workDir+"/scratch/lig/"+jobInput.ligBuffer+"/LIG_min.pdbqt";

        std::ifstream ligFile;
        ligFile.open(pdbqtPath.c_str());
        std::string fileLine;

        while (std::getline(ligFile, fileLine)) {
            ligSS << fileLine << std::endl;
        }
        
//        ligSS << jobInput.ligFile;
        
//        std::cout << "===========LIG file  jobInput ===============" << std::endl;
//        std::cout <<jobInput.ligFile;
//        
//        std::cout << "===========LIG file   ligSS===============" << std::endl;
//        std::cout <<ligSS.str();        
                

        sz max_modes_sz = static_cast<sz> (num_modes);

        boost::optional<std::string> rigid_name_opt;
        rigid_name_opt = rigid_name;

        boost::optional<std::string> flex_name_opt;
        if(jobInput.flexible){
                flex_name_opt = flex_name;
        }
//        out_name = default_output(ligand_name);
        std::stringstream out_name;
//        out_name << "REMARK RECEPTOR "<< rigid_name << std::endl;
//        out_name << "REMARK LIGAND " << ligand_name << std::endl;

        grid_dims gd; // n's = 0 via default c'tor

        flv weights;
        weights.push_back(weight_gauss1);
        weights.push_back(weight_gauss2);
        weights.push_back(weight_repulsion);
        weights.push_back(weight_hydrophobic);
        weights.push_back(weight_hydrogen);
        weights.push_back(5 * weight_rot / 0.1 - 1); // linearly maps onto a different range, internally. see everything.cpp
        
        VINA_FOR_IN(i, gd) {
            gd[i].n = jobInput.n[i];
            gd[i].begin = jobInput.begin[i];
            gd[i].end = jobInput.end[i];
        }
        
        std::stringstream log;

//        log_name = ligand_name +".log";
//        log.init(log_name);

        doing(verbosity, "Reading input", log);

//        model m = parse_bundle(rigid_name_opt, flex_name_opt, std::vector<std::string > (1, ligand_name));
        model m = parse_bundle(rigid_name_opt, flex_name_opt, ligSS);

        boost::optional<model> ref;
        done(verbosity, log);

        main_procedure(m, ref,
                out_name,
                score_only, local_only, randomize_only, false, // no_cache == false
                gd, exhaustiveness,
                weights,
                cpu, seed, verbosity, max_modes_sz, energy_range, log);
        
        std::ofstream logFile;
        std::ofstream outFile;
        
        logFile.open("scores.log");
        outFile.open("poses.pdbqt"); 
        
        std::string logStr=log.str();
        logFile << logStr;
        outFile << out_name.str();

        logFile.close();
        outFile.close();
        
        jobOut.pdbID=jobInput.recBuffer;
        jobOut.ligID=jobInput.ligBuffer;
        jobOut.logPath="scratch/com/" + jobInput.recBuffer + "/dock/" + jobInput.ligBuffer+"/scores.log";
        jobOut.posePath="scratch/com/" + jobInput.recBuffer + "/dock/" + jobInput.ligBuffer+"/poses.pdbqt";
        getScores(logStr, jobOut.scores);
        jobOut.mesg="Finished!";
        
        
    } catch (file_error& e) {
        std::cerr << "\n\nError: could not open \"" << e.name.string() << "\" for " << (e.in ? "reading" : "writing") << ".\n";
        jobOut.mesg="Could not open " + e.name.string() + " for " + (e.in ? "reading" : "writing");
        return 1;
    } catch (boost::filesystem::filesystem_error& e) {
        std::cerr << "\n\nFile system error: " << e.what() << '\n';
        jobOut.mesg="File system error: ";
        jobOut.mesg=jobOut.mesg+ e.what();
        return 1;
    } catch (usage_error& e) {
        std::cerr << "\n\nUsage error: " << e.what() << ".\n";
        jobOut.mesg="Usage error: ";
        jobOut.mesg=jobOut.mesg+ e.what();
        return 1;
    } catch (parse_error& e) {
        std::cerr << "\n\nParse error on line " << e.line << " in file \"" << e.file.string() << "\": " << e.reason << '\n';
        jobOut.mesg="Parse error on line " + Sstrm<std::string, unsigned>(e.line) + " in file " + e.file.string() + ": " + e.reason;
        return 1;
    } catch (std::bad_alloc&) {
        std::cerr << "\n\nError: insufficient memory!\n";
        jobOut.mesg="Insufficient memory!";
        return 1;
    } catch (std::exception& e) { // Errors that shouldn't happen:
        std::cerr << "\n\nAn error occurred: " << e.what() << ". " << std::endl;
        jobOut.mesg="An error occurred: " ;
        jobOut.mesg=jobOut.mesg+ e.what();
        return 1;
    } catch (internal_error& e) {
        std::cerr << "\n\nAn internal error occurred in " << e.file << "(" << e.line << "). " << std::endl;
        jobOut.mesg="An internal error occurred in " + e.file + " line: " + Sstrm<std::string, unsigned>(e.line);
        return 1;
    } catch (...) {
        std::cerr << "\n\nAn unknown error occurred. " << std::endl;
        jobOut.mesg="An unknown error occurred";
        return 1;
    }  
    
    return 0;
    
}

