/* 
 * File:   CDT3Docking.cpp
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
#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp> // filesystem::basename
#include <boost/thread/thread.hpp> // hardware_concurrency // FIXME rm ?
#include <boost/timer.hpp>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

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
#include "Common/Timer.hpp"

#include "dock.h"
#include "mpiparser.h"
#include "InitEnv.h"

using namespace conduit;
namespace mpi = boost::mpi;
using namespace boost::filesystem;
using namespace LBIND;

using namespace std::chrono_literals;

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

        n[recIDMeta+"Box/default"]=jobOut.useDockBx;
        if(jobOut.box.size()==6){
            n[recIDMeta+"Box/cx"]=jobOut.box[0];
            n[recIDMeta+"Box/cy"]=jobOut.box[1];
            n[recIDMeta+"Box/cz"]=jobOut.box[2];
            n[recIDMeta+"Box/dx"]=jobOut.box[3];
            n[recIDMeta+"Box/dy"]=jobOut.box[4];
            n[recIDMeta+"Box/dz"]=jobOut.box[5];
        }

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

//__attribute__((annotate("@critical_path()")))
void sendJob(std::unordered_set<std::string> :: iterator& itr, mpi::communicator& world, int& inpTag, int& rankTag, int& jobTag, int& jobFlag, JobInputData& jobInput){
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            world.send(freeProc, jobTag, jobFlag);
            jobInput.key = (*itr);

            std::cout << "At Process: " << freeProc << " working on  Key: " << jobInput.key << std::endl;

            world.send(freeProc, inpTag, jobInput);

}

// Time out docking calculation if it exceeds 1 hour
int dock_wrapper(JobInputData& jobInput, JobOutData& jobOut, std::string& localDir)
{
    std::mutex m;
    std::condition_variable cv;
    int retValue=1;

    std::thread t([&cv, &jobInput, &jobOut, &localDir]()
                  {
                      dockjob(jobInput, jobOut, localDir);
                      cv.notify_one();
                  });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, 1800s) == std::cv_status::timeout)
            throw std::runtime_error("Timeout");
    }

    return retValue;
}

int main(int argc, char* argv[]) {

    // ! MPI Parallel   
    int jobFlag=1; // 1: doing job,  0: done job

    int rankTag=1;
    int jobTag=2;

    int inpTag=3;
    int outTag=4;

    mpi::environment env(argc, argv);
    mpi::communicator world;    
    mpi::timer runingTime;

    std::string workDir;
    std::string inputDir;
    std::string dataPath;
    std::string localDir;

    if (world.rank() == 0) {

        std::cout << "CDT3Docking Begin: " << timestamp() << std::endl;
    }

    if(!initConveyorlcEnv(workDir, localDir, inputDir, dataPath)){
        world.abort(1);
    }

    bool useLocalDir=(localDir!=workDir);
    if(useLocalDir){
        std::string cmd = "rm -rf " + localDir+"/scratch";
        std::string errMesg = "Clean up local disk fails before calculation";
        //LBIND::command(cmd, errMesg);
        world.barrier();
    }

    std::cout << "Number of tasks= " << world.size() << " My rank= " << world.rank() << std::endl;

    JobInputData jobInput;
    JobOutData jobOut;

    std::unordered_set<std::string> keysCalc;
    std::unordered_map<std::string, std::string> boxes;

    std::vector<std::string> keysFinish;

    if (world.rank() == 0) {
        std::cout << "Master Node: " << world.size() << " My rank= " << world.rank() << std::endl;
        std::string recFile;
        std::string ligFile;
        std::vector<std::string> recList;
        std::vector<std::string> fleList;
        std::vector<std::string> ligList;
        std::vector<std::string> comList;

        int success = mpiParser(argc, argv, ligFile, recFile, ligList, recList, comList, fleList, boxes, jobInput);
        if (success != 0) {
            std::cerr << "Error: Parser input error" << std::endl;
            world.abort(1);
        }

        if (world.size() < 2) {
            std::cerr << "Error: Total process less than 2" << std::endl;
            world.abort(1);
        }

        //Create a HDF5 output directory for docking
        std::string cmd = "mkdir -p " + workDir + "/" +jobInput.outDir;
        if(jobInput.outDir[0]=='/'){
            cmd = "mkdir -p " + jobInput.outDir;
        }
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

        std::vector<std::vector<std::string> > allKeysFinish;
        gather(world, keysFinish, allKeysFinish, 0);

        for(int i=0; i < allKeysFinish.size(); ++i)
        {
            std::vector<std::string> keysVec=allKeysFinish[i];
            for(int j=0; j< keysVec.size(); ++j)
            {
                keysCalc.erase(keysVec[j]);
            }
        }

    }else{

        std::string dockHDF5Dir=workDir+"/scratch/dockHDF5";
        path dockHDF5path(dockHDF5Dir);
        std::vector<std::string> hdf5Files;
        if(is_directory(dockHDF5path)) {
            for(auto& entry : boost::make_iterator_range(directory_iterator(dockHDF5path), {}))
                hdf5Files.push_back(entry.path().string());
        }
        int start = world.rank()-1;
        int stride = world.size()-1;

        for(int i=start; i<hdf5Files.size(); i=i+stride)
        {
            getKeysHDF5(hdf5Files[i], keysFinish);
        }

        gather(world, keysFinish, 0);
    }

    if (world.rank() == 0) {
        std::cout << "CDT3Docking Found All Finished Calculation: " << timestamp() << std::endl;
        std::cout << "CDT3Docking Number of Calculations : " << keysCalc.size() << std::endl;
    }

    if (world.rank() == 0) {
        unsigned num_cpus = boost::thread::hardware_concurrency();
        if (num_cpus > 0)
            jobInput.cpu = num_cpus;
        else
            jobInput.cpu = 1;    

        jobInput.flexible=false;
        
        if(jobInput.randomize){
            srand(unsigned(std::time(NULL)));
        }

        bool hasBox= (boxes.size()>0);
        std::string delimiter = "/";
        //int count=0;
        std::unordered_set<std::string> :: iterator itr;
        for (itr = keysCalc.begin(); itr != keysCalc.end(); itr++) {
            //std::cout << (*itr) << std::endl;

            /*
            ++count;
            if (count > world.size()-1) {
                world.recv(mpi::any_source, outTag, jobOut);
                toFile(jobInput, jobOut);
            }
            */
            jobInput.useDockBx=false;
            if(hasBox){
                std::string key = (*itr);
                if (boxes.find(key) == boxes.end()){
                    std::cout << "CDT3Docking Warning :  key " << key  << " is not in box list, use default box"<< std::endl;
                }
                else{
                    jobInput.dockBx=boxes[key];
                    std::cout << "DEBUG: dockBx=" << jobInput.dockBx  << std::endl;
                    jobInput.useDockBx=true;
                }
            }
            sendJob(itr, world, inpTag, rankTag, jobTag, jobFlag, jobInput);
        }

        /*
        int nJobs=count;
        int nWorkers=world.size()-1;
        int ndata=(nJobs<nWorkers)? nJobs: nWorkers;

        std::cout << "ndata=" << ndata << " nJobs=" << nJobs << std::endl;
    
        for(unsigned i=0; i < ndata; ++i){
            world.recv(mpi::any_source, outTag, jobOut);
            toFile(jobInput, jobOut);
        }
        */

        for(unsigned i=1; i < world.size(); ++i){
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;
            world.send(freeProc, jobTag, jobFlag);
        }

    } else {

        std::string dockHDF5File=workDir+"/scratch/dockHDF5/dock_proc"+std::to_string(world.rank())+".hdf5:/";
        //hid_t dock_hid=relay::io::hdf5_open_file_for_read_write(dockHDF5File);
        while (1) {

            world.send(0, rankTag, world.rank());

            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }
            // Receive parameters

            world.recv(0, inpTag, jobInput);

            bool timedout = false;
            try {
                int i=5;
                dock_wrapper(jobInput, jobOut, localDir);
            }
            catch(std::runtime_error& e) {
                std::cout << e.what() << std::endl;
                timedout = true;
            }

            if(timedout) {
                std::cout << "TIMEOUT calculation for key = " << jobInput.key << std::endl;
                jobOut.mesg="Calculation Timeout";
                jobOut.error=false;
            }

            //dockjob(jobInput, jobOut, localDir);

            toHDF5File(jobInput, jobOut, dockHDF5File);

            // Go back the localDir to get rid of following error
            // shell-init: error retrieving current directory: getcwd: cannot access
            chdir(localDir.c_str());

            // Remove the working directory
            std::string cmd = "rm -rf " + jobOut.dockDir;
            std::string errMesg="remove dockDir fails";
            //LBIND::command(cmd, errMesg);

        }

        //relay::io::hdf5_close_file(dock_hid);
    }


    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;

    if(useLocalDir){
        world.barrier();
        std::string cmd = "rm -rf " + localDir+"/scratch";
        std::string errMesg = "Clean up local disk fails before calculation";
        //LBIND::command(cmd, errMesg);

    }

    if (world.rank() == 0) {
        std::cout << "CDT3Docking End Calculation: " << timestamp() << std::endl;
    }

    return (0);

}


