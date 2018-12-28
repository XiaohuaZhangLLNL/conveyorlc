/*

   Copyright (c) 2006-2010, The Scripps Research Institute

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by aCDTicable law or agreed to in writing, software
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
#include "Common/Command.hpp"
#include "Common/LBindException.h"
//#include "gzstream.h"
//#include "tee.h"
#include "VinaLC/coords.h" // add_to_output_container
#include "VinaLC/tokenize.h"
#include "dock.h"


#include "mainProcedure.h"


using namespace conduit;

bool getScores(JobOutData& jobOut){

    std::vector<std::string> lines;
    tokenize(jobOut.scorelog, lines, "\n");
    if(lines.size()<5)  return false;

    for (unsigned i = 4; i < lines.size(); ++i) {
        std::vector<std::string> values;
        tokenize(lines[i], values);
        if (values.size() > 2) {
            double score = Sstrm<double, std::string>(values[1]);
            jobOut.scores.push_back(score);
        }
    }
    
    if(jobOut.scores.size()==0) return false;
    
    return true;
}


void getRecData(JobInputData& jobInput, std::string& recKey, grid_dims& gd){
    Node n;

    hid_t rec_hid=relay::io::hdf5_open_file_for_read(jobInput.recFile);
    relay::io::hdf5_read(rec_hid, n);

    std::string pdbqtPath="rec/"+recKey+"/file/rec_min.pdbqt";
    //std::cout << pdbqtPath << std::endl;
    if(n.has_path(pdbqtPath)){
        std::string outLines=n[pdbqtPath].as_string();

        std::ofstream outFile("rec_min.pdbqt");
        outFile << outLines;
    }else{
        throw LBIND::LBindException("Cannot retrieve pdbqt file for "+recKey);
    }

    std::vector<std::string> axis={"X", "Y", "Z"};
    std::vector<std::string> kind={"Centroid", "Dimension"};

    std::vector<double> geo;

    for(std::string& k : kind)
    {
        for(std::string& a : axis)
        {
            std::string keyPath="rec/"+recKey+"/meta/Site/"+k+"/"+a;
            if(n.has_path(keyPath)){
                double val=n[keyPath].as_double();
                geo.push_back(val);
            }else{
                throw LBIND::LBindException("Cannot retrieve "+k+" "+a);
            }
        }
    }

    relay::io::hdf5_close_file(rec_hid);

    if(geo.size()!=6) {
        throw LBIND::LBindException("Size of geo is not equal to 6");
    }
    vec center(geo[0], geo[1], geo[2]);
    vec span(geo[3], geo[4], geo[5]);


    VINA_FOR_IN(i, gd) {
        gd[i].n = sz(std::ceil(span[i] / jobInput.granularity));
        fl real_span = jobInput.granularity * gd[i].n;
        gd[i].begin = center[i] - real_span / 2;
        gd[i].end = gd[i].begin + real_span;
    }


}

void getLigData(std::string& fileName, std::string& ligKey, std::stringstream& ligSS){
    Node n;

    hid_t lig_hid=relay::io::hdf5_open_file_for_read(fileName);
    relay::io::hdf5_read(lig_hid, n);

    std::string pdbqtPath="lig/"+ligKey+"/file/LIG_min.pdbqt";
    if(n.has_path(pdbqtPath)){
        std::string outLines=n[pdbqtPath].as_string();

        ligSS << outLines;
    }else{
        throw LBIND::LBindException("Cannot retrieve pdbqt file for "+ligKey);
    }

    relay::io::hdf5_close_file(lig_hid);
}


void dockjob(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir){
    try{
        jobOut.error= true;
//        std::string flex_name, config_name, out_name, log_name;
        if (jobInput.randomize) {
            jobInput.seed = rand();
        }
        int seed=jobInput.seed;
        int verbosity = 1;
        int num_modes = jobInput.num_modes;
        fl energy_range = jobInput.energy_range;

        std::vector<std::string> keys;
        tokenize(jobInput.key, keys, "/");

        if(keys.size()!=2)
        {
            throw LBIND::LBindException("Keys should have a pair for "+ jobInput.key);
        }

        jobOut.pdbID=keys[0];
        jobOut.ligID=keys[1];

        // -0.035579, -0.005156, 0.840245, -0.035069, -0.587439, 0.05846
        fl weight_gauss1 = -0.035579;
        fl weight_gauss2 = -0.005156;
        fl weight_repulsion = 0.840245;
        fl weight_hydrophobic = -0.035069;
        fl weight_hydrogen = -0.587439;
        fl weight_rot = 0.05846;
        bool score_only = false, local_only = false, randomize_only = false; // FIXME

        jobOut.dockDir = workDir + "/scratch/dock/" + jobOut.pdbID + "/" + jobOut.ligID;
        std::string cmd = "mkdir -p " + jobOut.dockDir;
        std::string errMesg="mkdir dockDir fails";
        LBIND::command(cmd, errMesg);
        // cd to the rec directory to performance calculation
        chdir(jobOut.dockDir.c_str());


        grid_dims gd; // n's = 0 via default c'tor
        getRecData(jobInput, jobOut.pdbID, gd);

        std::string rigid_name =jobOut.dockDir+"/rec_min.pdbqt";
        std::string flex_name = "";
        int exhaustiveness=jobInput.exhaustiveness;
        int cpu=jobInput.cpu;

        std::string ligand_name = jobOut.ligID;
        std::stringstream ligSS;
        getLigData(jobInput.ligFile, ligand_name, ligSS);


        sz max_modes_sz = static_cast<sz> (num_modes);

        boost::optional<std::string> rigid_name_opt;
        rigid_name_opt = rigid_name;

        boost::optional<std::string> flex_name_opt;
        if(jobInput.flexible){
                flex_name_opt = flex_name;
        }

        std::stringstream out_name;

        flv weights;
        weights.push_back(weight_gauss1);
        weights.push_back(weight_gauss2);
        weights.push_back(weight_repulsion);
        weights.push_back(weight_hydrophobic);
        weights.push_back(weight_hydrogen);
        weights.push_back(5 * weight_rot / 0.1 - 1); // linearly maps onto a different range, internally. see everything.cpp

        std::stringstream log;

        doing(verbosity, "Reading input", log);

//        model m = parse_bundle(rigid_name_opt, flex_name_opt, std::vector<std::string > (1, ligand_name));
        model m = parse_bundle(rigid_name_opt, flex_name_opt, ligSS);

        boost::optional<model> ref;
        done(verbosity, log);

        sz how_many=0;
        main_procedure(m, ref,
                out_name,
                score_only, local_only, randomize_only, false, // no_cache == false
                gd, exhaustiveness,
                weights,
                cpu, seed, verbosity, max_modes_sz, energy_range, log, how_many);

        jobOut.numPose=how_many;

        jobOut.scorelog=log.str();

        jobOut.pdbqtfile=out_name.str();

        
        jobOut.scores.clear();
        bool success=getScores(jobOut);
        if(success){
            jobOut.mesg="Finished!";
        }else{
            jobOut.mesg="No scores!";
        }
        if(how_many<1){
            jobOut.mesg="No scores!";
        }

    } catch (file_error& e) {
        //std::cerr << "\n\nError: could not open \"" << e.name.string() << "\" for " << (e.in ? "reading" : "writing") << ".\n";
        jobOut.mesg="Could not open " + e.name.string() + " for " + (e.in ? "reading" : "writing");
        jobOut.error= false;
    } catch (boost::filesystem::filesystem_error& e) {
        //std::cerr << "\n\nFile system error: " << e.what() << '\n';
        jobOut.mesg="File system error: ";
        jobOut.mesg=jobOut.mesg+ e.what();
        jobOut.error= false;
    } catch (usage_error& e) {
        //std::cerr << "\n\nUsage error: " << e.what() << ".\n";
        jobOut.mesg="Usage error: ";
        jobOut.mesg=jobOut.mesg+ e.what();
        jobOut.error= false;
    } catch (parse_error& e) {
        //std::cerr << "\n\nParse error on line " << e.line << " in file \"" << e.file.string() << "\": " << e.reason << '\n';
        jobOut.mesg="Parse error on line " + Sstrm<std::string, unsigned>(e.line) + " in file " + e.file.string() + ": " + e.reason;
        jobOut.error= false;
    } catch (std::bad_alloc&) {
        //std::cerr << "\n\nError: insufficient memory!\n";
        jobOut.mesg="Insufficient memory!";
        jobOut.error= false;
    } catch (std::exception& e) { // Errors that shouldn't happen:
        //std::cerr << "\n\nAn error occurred: " << e.what() << ". " << std::endl;
        jobOut.mesg="An error occurred: " + std::string(e.what());
        jobOut.error= false;
    } catch (internal_error& e) {
        //std::cerr << "\n\nAn internal error occurred in " << e.file << "(" << e.line << "). " << std::endl;
        jobOut.mesg = "An internal error occurred in " + e.file + " line: " + Sstrm<std::string, unsigned>(e.line);
        jobOut.error= false;
    } catch (LBIND::LBindException& e) {
        jobOut.mesg = e.what();
        jobOut.error= false;
    } catch(conduit::Error &error){
        jobOut.mesg= error.message();
        jobOut.error= false;
    } catch (...) {
        //std::cerr << "\n\nAn unknown error occurred. " << std::endl;
        jobOut.mesg="An unknown error occurred";
        jobOut.error= false;
    }

    // Remove the working directory
    std::string cmd = "rm -rf " + jobOut.dockDir;
    std::string errMesg="remove dockDir fails";
    LBIND::command(cmd, errMesg);


    
}

