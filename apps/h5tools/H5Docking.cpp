//
// Created by Zhang, Xiaohua on 2019-04-01.
//

#include "H5Docking.h"
#include "H5DockingPO.h"

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

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

#include "../apps/conduitppl/InitEnv.h"

using namespace conduit;
namespace mpi = boost::mpi;
using namespace boost::filesystem;
using namespace LBIND;

using namespace conduit;

void rmFailCalc(POdata& podata){
    hid_t rec_hid=relay::io::hdf5_open_file_for_read(podata.inputFile);

    std::vector<std::string> rec_names;
    relay::io::hdf5_group_list_child_names(rec_hid,"/rec/",rec_names);

    for(int i=0;i<rec_names.size();i++) {
        const std::string &curr_rec = rec_names[i];
        std::vector<std::string> lig_names;
        Node nStatus;
        relay::io::hdf5_read(rec_hid, "/rec/" + curr_rec + "/status", nStatus);
        if(nStatus.to_int()==1)
        {
            std::cout << curr_rec << std::endl;
        }
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
    std::string localDir;

    POdata podata;
    bool success=H5ReceptorPO(argc, argv, podata);
    if (!success) {
        world.abort(1);
    }

    if(!initConveyorlcEnv(workDir, localDir, inputDir, dataPath)){
        world.abort(1);
    }

    if (world.size() < 2) {
        std::cerr << "Error: Total process less than 2" << std::endl;
        world.abort(1);
    }



}