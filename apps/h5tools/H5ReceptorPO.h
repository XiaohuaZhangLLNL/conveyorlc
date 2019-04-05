//
// Created by Zhang, Xiaohua on 2019-03-29.
//

#ifndef CONVEYORLC_H5RECEPTORPO_H
#define CONVEYORLC_H5RECEPTORPO_H


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

bool H5ReceptorPO(int argc, char** argv, POdata& podata);

#endif //CONVEYORLC_H5RECEPTORPO_H
