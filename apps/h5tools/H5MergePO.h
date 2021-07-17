//
// Created by Zhang, Xiaohua on 7/16/21.
//

#ifndef CONVEYORLC_H5MERGEPO_H
#define CONVEYORLC_H5MERGEPO_H

#include <string>
#include <vector>

struct POdata{
    std::string inputFile;
    std::string outputFile;
    std::string type;
};

bool H5MergePO(int argc, char** argv, POdata& podata);


#endif //CONVEYORLC_H5MERGEPO_H
