/* 
 * File:   SDL3Docking.cpp
 * Author: zhang
 *
 * Created on March 25, 2014, 2:51 PM
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <stack>
#include <vector> // ligand paths
#include <cmath> // for ceila
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <ctime>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename
#include <boost/thread/thread.hpp> // hardware_concurrency // FIXME rm ?
#include <boost/timer.hpp>

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
#include "VinaLC/coords.h" // add_to_output_container
#include "VinaLC/tokenize.h"
#include "Common/Command.hpp"

#include "dock.h"
#include "mpiparser.h"
#include "InitEnv.h"

using namespace conduit;
using namespace boost::filesystem;
using namespace LBIND;



void getKeysHDF5(std::string& fileName, std::vector<std::string>& keysFinish)
{

    Node n;

    hid_t dock_hid=relay::io::hdf5_open_file_for_read(fileName);

    std::vector<std::string> rec_names;
    relay::io::hdf5_group_list_child_names(dock_hid,"/dock/",rec_names);

    for(int i=0;i<rec_names.size();i++)
    {
        const std::string &curr_rec = rec_names[i];
        std::vector<std::string> lig_names;
        relay::io::hdf5_group_list_child_names(dock_hid,"/dock/"+curr_rec+"/",lig_names);

        for(int j=0; j<lig_names.size(); j++)
        {
            std::string key=curr_rec+"/"+lig_names[j];
            keysFinish.push_back(key);
        }
    }

    relay::io::hdf5_close_file(dock_hid);

}

void getKeysHDF5OLD(std::string& fileName, std::vector<std::string>& keysFinish)
{
    Node n;

    hid_t dock_hid=relay::io::hdf5_open_file_for_read(fileName);
    relay::io::hdf5_read(dock_hid, n);

    NodeIterator itrRec = n["dock"].children();

    while(itrRec.has_next())
    {
        Node &nRec=itrRec.next();

        NodeIterator itrLig = nRec.children();

        while(itrLig.has_next())
        {
            Node &nLig=itrLig.next();
            std::string key=nRec.name()+"/"+nLig.name();
            keysFinish.push_back(key);
        }

    }
    relay::io::hdf5_close_file(dock_hid);

}

void toConduit(JobOutData& jobOut, std::string& dockHDF5File){
    try {

        Node n;

        std::string keyPath="dock/"+jobOut.pdbID +"/"+jobOut.ligID;

        n[keyPath+ "/status"]=jobOut.error;

        std::string recIDMeta =keyPath+ "/meta/";
        n[recIDMeta+"ligName"]=jobOut.ligName;
        n[recIDMeta+"numPose"]=jobOut.numPose;
        n[recIDMeta+"Mesg"]=jobOut.mesg;

        for(int i=0; i< jobOut.scores.size(); ++i)
        {
            n[recIDMeta+"scores/"+std::to_string(i+1)]=jobOut.scores[i];
        }

        std::string recIDFile =keyPath + "/file/";

        n[recIDFile+"scores.log"]=jobOut.scorelog;
        n[recIDFile+"poses.pdbqt"]=jobOut.pdbqtfile;

        relay::io::hdf5_append(n, dockHDF5File);

    }catch(conduit::Error &error){
        jobOut.mesg= error.message();
    }


}

void toHDF5File(JobInputData& jobInput, JobOutData& jobOut, std::string& dockHDF5File)
{
    if(jobInput.useScoreCF){
        if(jobOut.scores.size()>0){
            if(jobOut.scores[0]>jobInput.scoreCF){
                //Mark the low score compound as "fail" so when restart program won't re-do it
                jobOut.error=false;
                //jobOut.numPose=0;
                jobOut.mesg="Beyond score cutoff";
                //jobOut.scorelog="";
                //jobOut.pdbqtfile="";
                //jobOut.scores.clear();
            }
        }else{
            jobOut.error=false;
        }
    }
    toConduit(jobOut, dockHDF5File);

}

std::string timestamp(){
    auto current = std::chrono::system_clock::now();
    std::time_t cur_time = std::chrono::system_clock::to_time_t(current);
    return std::ctime(&cur_time);
}

int main(int argc, char* argv[]) {

    std::string workDir;
    std::string inputDir;
    std::string dataPath;
    std::string localDir;

    std::cout << "SDL3Docking Begin: " << timestamp() << std::endl;

    if(!initConveyorlcEnv(workDir, localDir, inputDir, dataPath)){
        exit(-1);
    }

    bool useLocalDir=(localDir!=workDir);

    JobInputData jobInput;
    JobOutData jobOut;

    std::unordered_set<std::string> keysCalc;
    std::unordered_map<std::string, std::string> boxes;

    std::vector<std::string> keysFinish;

    std::string recFile;
    std::string ligFile;
    std::vector<std::string> recList;
    std::vector<std::string> fleList;
    std::vector<std::string> ligList;
    std::vector<std::string> comList;

    int success = mpiParser(argc, argv, ligFile, recFile, ligList, recList, comList, fleList, boxes, jobInput);
    if (success != 0) {
        std::cerr << "Error: Parser input error" << std::endl;
        exit(-1);
    }

    //Create a HDF5 output directory for docking
    std::string cmd = "mkdir -p " + workDir+"/scratch/dockHDF5";
    std::string errMesg="mkdir dockHDF5 fails";
    LBIND::command(cmd, errMesg);

    // Generate the keys based on combination or no combination
    // reserve large chunk of memory to speed up the process
    if (jobInput.comFile=="") {
        std::unordered_set<std::string>::size_type keySize = recList.size() * ligList.size();
        keysCalc.reserve(keySize);

        for (int i = 0; i < recList.size(); ++i) {
            for (int j = 0; j < ligList.size(); ++j) {
                std::string key = recList[i] + "/" + ligList[j];
                //std::cout << key <<std::endl;
                keysCalc.insert(key);
            }
        }
    }else {

        std::unordered_set<std::string>::size_type keySize = comList.size();
        keysCalc.reserve(keySize);

        for (int i = 0; i < comList.size(); ++i) {
            std::vector<std::string> tokens;
            std::string key=comList[i];
            tokenize(key, tokens, "/");
            if(tokens.size()==2) {
                if (std::find(recList.begin(), recList.end(),tokens[0])!=recList.end()) {
                    if (std::find(ligList.begin(), ligList.end(),tokens[1])!=ligList.end()) {
                        keysCalc.insert(key);
                    }
                }
            }
        }

    }

    //std::string dockhdf5File=workDir+"/scratch/dock.hdf5";

    std::string dockhdf5File=workDir+"/"+jobInput.outFile;
    // if use absolute path
    if(jobInput.outFile[0]=='/'){
        dockhdf5File=jobInput.outFile;
    }

    if(is_regular_file(dockhdf5File)) {
        getKeysHDF5(dockhdf5File, keysFinish);
    }

    for(int j=0; j< keysFinish.size(); ++j)
    {
        keysCalc.erase(keysFinish[j]);
    }

    std::cout << "SDL3Docking Found All Finished Calculation: " << timestamp() << std::endl;
    std::cout << "SDL3Docking Number of Calculations : " << keysCalc.size() << std::endl;

    if(jobInput.cpu<1) { // if user didn't define the number of CPUs
        unsigned num_cpus = boost::thread::hardware_concurrency();
        if (num_cpus > 0)
            jobInput.cpu = num_cpus;
        else
            jobInput.cpu = 1;
    }

    std::cout << "Number of CPUs used in Calculation: " << jobInput.cpu << std::endl;

    jobInput.flexible=false;

    if(jobInput.randomize){
        srand(unsigned(std::time(NULL)));
    }

    //std::string dockHDF5File=workDir+"/scratch/dock.hdf5:/";
    std::string dockHDF5FilePath=dockhdf5File+":/";

    bool hasBox= (boxes.size()>0);
    std::string delimiter = "/";
    //int count=0;
    std::unordered_set<std::string> :: iterator itr;
    for (itr = keysCalc.begin(); itr != keysCalc.end(); itr++) {

        jobInput.key = (*itr);

        std::cout << "Working on  Key: " << jobInput.key << std::endl;

        if(hasBox){
            std::string key = (*itr);
            if (boxes.find(key) == boxes.end()){
                std::cout << "CDT3Docking Warning :  key " << key  << " is not in box list"<< std::endl;
                continue;
            }else{
                jobInput.dockBx=boxes[key];std::cout << "DEBUG: dockBx=" << jobInput.dockBx  << std::endl;
                jobInput.useDockBx=true;
            }
        }
        dockjob(jobInput, jobOut, localDir);

        toHDF5File(jobInput, jobOut, dockHDF5FilePath);

        // Go back the localDir to get rid of following error
        // shell-init: error retrieving current directory: getcwd: cannot access
        chdir(localDir.c_str());

        // Remove the working directory
        std::string cmd = "rm -rf " + jobOut.dockDir;
        std::string errMesg="remove dockDir fails";
        LBIND::command(cmd, errMesg);

    }

    std::cout << "SDL3Docking End Calculation: " << timestamp() << std::endl;

    return (0);

}


