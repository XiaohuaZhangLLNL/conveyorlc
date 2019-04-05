//
// Created by Zhang, Xiaohua on 2019-03-29.
//

#include "H5Receptor.h"
#include "H5ReceptorPO.h"

#include <iostream>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>


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
    POdata podata;

    bool success=H5ReceptorPO(argc, argv, podata);
    if(!success){
        return -1;
    }

    if(podata.del){
        rmFailCalc(podata);
    }


    for(int i=0; i<podata.checkdata.size(); ++i)
    {
        std::cout << podata.checkdata[i] << std::endl;
    }
}