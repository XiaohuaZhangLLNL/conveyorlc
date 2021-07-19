//
// Created by Zhang, Xiaohua on 7/16/21.
//

#ifndef CONVEYORLC_H5SPLITPO_H
#define CONVEYORLC_H5SPLITPO_H

#include <string>
#include <vector>

struct POdata{
    std::string inputFile;
    std::string outputDir;
    std::string type;
};

bool H5SplitPO(int argc, char** argv, POdata& podata);


#endif //CONVEYORLC_H5SPLITPO_H
