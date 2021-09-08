//
// Created by Zhang, Xiaohua on 7/16/21.
//

#include "H5Merge.h"
#include "H5MergePO.h"
#include "Common/Chomp.hpp"
#include "Common/Utils.h"
#include "Common/File.hpp"

#include <iostream>

#include <conduit.hpp>
#include <conduit_relay.hpp>
#include <conduit_relay_io_hdf5.hpp>
#include <conduit_blueprint.hpp>

using namespace LBIND;
using namespace conduit;

void saveStrList(std::string& fileName, std::vector<std::string>& strList){
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
        if(fileLine.size()==0) continue;
        chomp(fileLine);
        strList.push_back(fileLine);
    }

}

void merge(std::string& h5file, std::string& ligCdtFile, std::string& type, int& count){
    hid_t rec_hid=relay::io::hdf5_open_file_for_read(h5file);
    std::vector<std::string> item_names;
    std::string path = "/"+type+"/";
    relay::io::hdf5_group_list_child_names(rec_hid, path,item_names);

    bool reCount=(type=="lig");
    std::cout << std::string(10, '.') << "   0%"<<std::flush;
    double totalitems=double(item_names.size());
    double oldprogress=0;
    for(int i=0;i<item_names.size();i++) {
        const std::string &curr_item = item_names[i];
        Node nData;
        relay::io::hdf5_read(rec_hid, path + curr_item, nData);
        Node n;
        if (reCount) {
            std::string ligIDpath = "lig/" + std::to_string(count);
            n[ligIDpath] = nData;
        } else{
            std::string ligIDpath = path + curr_item;
            n[ligIDpath] = nData;
        }
        relay::io::hdf5_append(n, ligCdtFile);
        count++;

        int progress = double(i)/ totalitems *10;
        if (progress!=oldprogress) {
            std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
            std::cout << std::string(progress, '*') << std::string(10-progress, '.') << " "<< progress*10 <<"%"<<std::flush;
        }
        oldprogress=progress;
    }
    std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
    std::cout << std::string(10, '*') << " "<< 100 <<"%"<<std::flush;
    std::cout << std::endl;
}



int main(int argc, char** argv) {
    POdata podata;

    bool success=H5MergePO(argc, argv, podata);
    if(!success){
        return -1;
    }

    if(fileExist(podata.outputFile)){
        std::cout << "Output file " << podata.outputFile << " exists, please remove it." << std::endl;
        return -1;
    }

    std::vector<std::string> strList;
    saveStrList(podata.inputFile, strList);

    Node n;
    std::string ligCdtFile=podata.outputFile+":/";
    n["date"]="Create By H5Merge "+timeStamp();
    relay::io::hdf5_append(n, ligCdtFile);

    int count =1;
    for(int i=0; i<strList.size(); i++){
        std::cout << "Merging file " << strList[i] << " ..." << std::endl;
        merge(strList[i], ligCdtFile, podata.type, count);
    }

}