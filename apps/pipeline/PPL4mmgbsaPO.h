/* 
 * File:   PPL4mmgbsaPO.h
 * Author: zhang
 *
 * Created on April 22, 2014, 1:58 PM
 */

#ifndef PPL4MMGBSAPO_H
#define	PPL4MMGBSAPO_H

#include <string>

struct POdata{
    std::string recFile;
//    std::string ligFile;
//    std::string outputFile;
    std::string xmlFile;
    bool restart;
    int version;
//    bool pbFlag;
};

bool PPL4mmgbsaPO(int argc, char** argv, POdata& podata);

#endif	/* PPL4MMGBSAPO_H */

