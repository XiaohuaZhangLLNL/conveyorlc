//
// Created by Zhang, Xiaohua on 7/14/21.
//

#ifndef CONVEYORLC_H5LIGANDPO_H
#define CONVEYORLC_H5LIGANDPO_H


#include <string>
#include <vector>

struct POdata{
    bool del;
    std::string inputFile;
    std::string outputFile;
    std::string name;
    std::string delname;
    std::string storename;
    std::vector<std::string> checkdata;
};

bool H5LigandPO(int argc, char** argv, POdata& podata);


#endif //CONVEYORLC_H5LIGANDPO_H
