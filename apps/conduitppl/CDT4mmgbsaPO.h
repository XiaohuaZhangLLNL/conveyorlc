/* 
 * File:   CDT4mmgbsaPO.h
 * Author: zhang
 *
 * Created on April 22, 2014, 1:58 PM
 */

#ifndef CDT4MMGBSAPO_H
#define	CDT4MMGBSAPO_H

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

bool CDT4mmgbsaPO(int argc, char** argv, POdata& podata);

#endif	/* CDT4MMGBSAPO_H */

