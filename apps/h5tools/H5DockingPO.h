//
// Created by Zhang, Xiaohua on 2019-04-01.
//

#ifndef CONVEYORLC_H5DOCKINGPO_H
#define CONVEYORLC_H5DOCKINGPO_H

#include <string>
#include <vector>

struct POdata{
    bool del;
    std::string dockInDir;
    std::string outputFile;
    std::string name;
    std::string delname;
    std::string storename;
    std::vector<std::string> checkdata;
};

bool H5ReceptorPO(int argc, char** argv, POdata& podata);


#endif //CONVEYORLC_H5DOCKINGPO_H
