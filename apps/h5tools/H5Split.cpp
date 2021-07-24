//
// Created by Zhang, Xiaohua on 7/16/21.
//

#include "H5Split.h"
#include "H5SplitPO.h"
#include "Common/Utils.h"
#include "Common/Command.hpp"

#include <iostream>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

using namespace LBIND;
using namespace conduit;

void split(POdata& podata){

    std::string cmd="mkdir -p "+podata.outputDir;
    std::string errMesg=cmd+" fails";
    command(cmd, errMesg);

    hid_t rec_hid=relay::io::hdf5_open_file_for_read(podata.inputFile);
    std::vector<std::string> item_names;
    std::string path = "/"+podata.type+"/";
    relay::io::hdf5_group_list_child_names(rec_hid, path,item_names);

    for(int i=0;i<item_names.size();i++) {
        const std::string &curr_item = item_names[i];

        Node nday;
        std::string ligCdtFile=podata.outputDir+"/"+curr_item+".hdf5:/";
        nday["date"]="Create By H5Merge "+timeStamp();
        relay::io::hdf5_append(nday, ligCdtFile);

        Node nData;
        relay::io::hdf5_read(rec_hid, path + curr_item, nData);

        Node n;
        std::string ligIDpath = path + curr_item;
        n[ligIDpath] = nData;
        relay::io::hdf5_append(n, ligCdtFile);
    }

}


void splitByNumber(POdata& podata){

    std::string cmd="mkdir -p "+podata.outputDir;
    std::string errMesg=cmd+" fails";
    command(cmd, errMesg);

    hid_t rec_hid=relay::io::hdf5_open_file_for_read(podata.inputFile);
    std::vector<std::string> item_names;
    std::string path = "/"+podata.type+"/";
    relay::io::hdf5_group_list_child_names(rec_hid, path,item_names);

    int count=0;
    int num = podata.num;
    std::string ligCdtFile=podata.outputDir+"/out_"+std::to_string(count/num)+".hdf5:/";
    for(int i=0;i<item_names.size();i++) {
        if(count%num==0){
            ligCdtFile=podata.outputDir+"/out_"+std::to_string(count/num)+".hdf5:/";
        }

        const std::string &curr_item = item_names[i];

        Node nday;

        nday["date"]="Create By H5Merge "+timeStamp();
        relay::io::hdf5_append(nday, ligCdtFile);

        Node nData;
        relay::io::hdf5_read(rec_hid, path + curr_item, nData);

        Node n;
        std::string ligIDpath = path + curr_item;
        n[ligIDpath] = nData;
        relay::io::hdf5_append(n, ligCdtFile);
    }

}

int main(int argc, char** argv) {
    POdata podata;

    bool success=H5SplitPO(argc, argv, podata);
    if(!success){
        return -1;
    }

    if(podata.num>0){
        splitByNumber(podata);
    }else{
        split(podata);
    }



    return 0;
}