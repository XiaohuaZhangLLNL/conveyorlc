/* 
 * File:   mmgbsa.cpp
 * Author: zhang30
 *
 * Created on April 16, 2014, 10:52 AM
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <mpi.h>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

#include "MM/CDTgbsa.h"
#include "Common/Tokenize.hpp"
#include "Structure/Constants.h"
#include "Common/File.hpp"
#include "Common/LBindException.h"
#include "Common/Command.hpp"

#include "InitEnv.h"
#include "CDT4mmgbsa.h"
#include "CDT4mmgbsaPO.h"

using namespace conduit;
namespace mpi = boost::mpi;
using namespace boost::filesystem;
using namespace LBIND;

/*!
 * \breif mmgbsa MM-PB(GB)SA calculations on HPC using amber forcefield
 * \param argc
 * \param argv
 * \return success 
 * \defgroup mmgbsa_Commands mmgbsa Commands
 * 
 * Usage: mmgbsa <input-file>
 */

void getDockingKeysHDF5(std::string& fileName, std::vector<std::string>& keysFinish)
{
    path p(fileName);
    std::string fileBase=p.stem().string();
    // asume filename are dock_proc1.hdf5 dock_proc2.hdf5, ... dock_procN.hdf5
    int procID=std::stoi(fileBase.substr(9, fileBase.size()-9));

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

            if(nLig["status"].as_int()==1){

                int numPose=0;
                Node nNumPose=nLig["meta/numPose"];
                if (nNumPose.dtype().is_int32()){
                    numPose=nNumPose.as_int32();
                }else if (nNumPose.dtype().is_int64()) {
                    numPose= nNumPose.as_int64();
                }else{
                    numPose= nNumPose.as_int();
                }

                if(numPose>0){
                    std::string key=nRec.name()+"/"+nLig.name()+"+"+std::to_string(numPose)+"+"+std::to_string(procID);
                    keysFinish.push_back(key);
                }
            }

        }

    }
    relay::io::hdf5_close_file(dock_hid);

}

void getKeysHDF5(std::string& fileName, std::vector<std::string>& keysFinish)
{
    Node n;

    hid_t dock_hid=relay::io::hdf5_open_file_for_read(fileName);
    relay::io::hdf5_read(dock_hid, n);

    NodeIterator itrRec = n["gbsa"].children();

    while(itrRec.has_next())
    {
        Node &nRec=itrRec.next();

        NodeIterator itrLig = nRec.children();

        while(itrLig.has_next())
        {
            Node &nLig=itrLig.next();

            NodeIterator itrPose = nLig.children();

            while(itrPose.has_next()) {
                Node &nPose=itrPose.next();
                std::string key = nRec.name() + "/" + nLig.name() + "/" + nPose.name();
                keysFinish.push_back(key);
            }
        }

    }
    relay::io::hdf5_close_file(dock_hid);

}

void toConduit(CDTmeta &cdtMeta, std::string& gbsaHDF5File){
    try {

        Node n;

        std::string keyPath="gbsa/"+cdtMeta.key;

        n[keyPath+ "/status"]=cdtMeta.error;

        std::string recIDMeta =keyPath+ "/meta/";
        n[recIDMeta+"comGB"]=cdtMeta.comGB;
        n[recIDMeta+"ligGB"]=cdtMeta.ligGB;
        n[recIDMeta+"recGB"]=cdtMeta.recGB;
        n[recIDMeta+"bindGB"]=cdtMeta.gbbind;
        n[recIDMeta+"dockScore"]=cdtMeta.dockscore;
        n[recIDMeta+"Mesg"]=cdtMeta.message;

        std::string recIDFile =keyPath + "/file/";

        std::vector<std::string> filenames={"Com.prmtop", "Com.inpcrd", "Com_min.rst",
                                            "Com_min.pdb", "Com_min_GB.out", "Rec_minGB.out",
                                            "Com_leap.log", "Lig_leap.log", "rec_leap.log",
                                            "Rec_min.rst"};

        for(std::string& name : filenames)
        {
            std::ifstream infile(name);
            if(infile.good())
            {
                std::string buffer((std::istreambuf_iterator<char>(infile)),
                                   std::istreambuf_iterator<char>());
                infile.close();
                n[recIDFile+name] = buffer;
            }
            else
            {
                std::cout << "File - " << name << " is not there." << std::endl;
            }
        }

        relay::io::hdf5_append(n, gbsaHDF5File);

    }catch(conduit::Error &error){
        cdtMeta.message= error.message();
    }


}

void mmgbsa(CDTmeta& cdtMeta) {

    try{

        CDTgbsa::run(cdtMeta);
        cdtMeta.message= "Finished!";
        cdtMeta.error=true;

    } catch (LBindException& e){
        cdtMeta.message= e.what();
        cdtMeta.error=false;
    } 

}

int main(int argc, char** argv) {

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

    POdata podata;
    bool success = CDT4mmgbsaPO(argc, argv, podata);
    if (!success) {
        world.abort(1);
    }

    if(!initConveyorlcEnv(workDir, inputDir, dataPath)){
        world.abort(1);
    }

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        world.abort(1);
    }

    JobInputData jobInput;

    std::vector<std::string> dockingKeys;
    std::vector<std::vector<std::string> > allDockingKeys;


    if (world.rank() == 0) {

        //Create a HDF5 output directory for docking
        std::string cmd = "mkdir -p " + workDir + "/scratch/gbsaHDF5";
        std::string errMesg = "mkdir gbsaHDF5 fails";
        LBIND::command(cmd, errMesg);

        gather(world, dockingKeys, allDockingKeys, 0);
    }else {

        // Read in the keys of docking calculation
        std::string dockHDF5Dir = workDir + "/scratch/dockHDF5";
        path dockHDF5path(dockHDF5Dir);
        std::vector<std::string> hdf5Files;
        if (is_directory(dockHDF5path)) {
            for (auto &entry : boost::make_iterator_range(directory_iterator(dockHDF5path), {}))
                hdf5Files.push_back(entry.path().string());
        }

        int start = world.rank()-1;
        int stride = world.size()-1;

        for(int i=start; i<hdf5Files.size(); i=i+stride)
        {
            getDockingKeysHDF5(hdf5Files[i], dockingKeys);
        }

        gather(world, dockingKeys, 0);
    }

    // Clean up the dockingKeys
    dockingKeys.clear();

    std::unordered_map<std::string, int> keysCalc;
    std::vector<std::string> keysFinish;

    if (world.rank() == 0) {
        for(int i=0; i < allDockingKeys.size(); ++i)
        {
            std::vector<std::string> keysVec=allDockingKeys[i];
            for(int j=0; j< keysVec.size(); ++j)
            {
                std::vector<std::string> keystrs;
                tokenize(keysVec[j], keystrs, "+");
                if(keystrs.size()==3){
                    int numPose=std::stoi( keystrs[1] );
                    int procID=std::stoi( keystrs[2] );
                    for(int k=1; k<numPose+1; k++)
                    {
                        std::string key=keystrs[0]+"/p"+std::to_string(k);
                        keysCalc[key]=procID;
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

        std::string gbsaHDF5Dir=workDir+"/scratch/gbsaHDF5";
        path gbsaHDF5path(gbsaHDF5Dir);
        std::vector<std::string> hdf5Files;
        if(is_directory(gbsaHDF5path)) {
            for(auto& entry : boost::make_iterator_range(directory_iterator(gbsaHDF5path), {}))
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

        for (auto cKey : keysCalc) {

            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);

            world.send(freeProc, jobTag, jobFlag);

            jobInput.key = cKey.first;
            jobInput.procID=cKey.second;
            std::cout << "At Process: " << freeProc << " working on  Key: " << jobInput.key << std::endl;

            world.send(freeProc, inpTag, jobInput);

        }

        for(unsigned i=1; i < world.size(); ++i){
            int freeProc;
            world.recv(mpi::any_source, rankTag, freeProc);
            jobFlag=0;;
            world.send(freeProc, jobTag, jobFlag);
        }        
        
    }else {

        std::string gbsaHDF5File=workDir+"/scratch/gbsaHDF5/gbsa_proc"+std::to_string(world.rank())+".hdf5:/";
        while (1) {
            world.send(0, rankTag, world.rank());
            world.recv(0, jobTag, jobFlag);
            if (jobFlag==0) {
                break;
            }
            // Receive parameters
            world.recv(0, inpTag, jobInput);

            //initialize Meta data
            CDTmeta cdtMeta;
            cdtMeta.version=podata.version;
            cdtMeta.dockInDir=podata.dockInDir;
            cdtMeta.recFile=podata.recFile;
            cdtMeta.ligFile=podata.ligFile;

            cdtMeta.workDir=workDir;
            cdtMeta.dataPath=dataPath;
            cdtMeta.inputDir=inputDir;

            cdtMeta.key=jobInput.key;
            cdtMeta.procID=jobInput.procID;

            mmgbsa(cdtMeta);

            toConduit(cdtMeta, gbsaHDF5File);

            // Remove the working dire
            std::string cmd = "rm -rf " + cdtMeta.poseDir;
            std::string errMesg="remove poseDir fails";
            LBIND::command(cmd, errMesg);
            
        }
    }

    std::cout << "Rank= " << world.rank() <<" MPI Wall Time= " << runingTime.elapsed() << " Sec."<< std::endl;
    
    return 0;
}

