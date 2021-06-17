/* 
 * File:   SAF4mmgbsaPO.h
 * Author: zhang
 *
 * Created on April 22, 2014, 1:58 PM
 */

#ifndef SAF4MMGBSAPO_H
#define	SAF4MMGBSAPO_H

#include <string>
#include "SAF4mmgbsa.h"

struct POdata{

    bool keep;
    int version;

    bool error;
    double intDiel;
    double dockscore;
    double gbbind;

    double ligGB;
    double recGB;
    double comGB;

    std::string recID;
    std::string ligID;
    std::string poseID;
    std::string message;

    std::string ligDir;
    std::string recFile;
    std::string ligFile;
    std::string ligName;

    std::string workDir;
    std::string localDir;
    std::string inputDir;
    std::string dataPath;

    std::string poseDir;

    std::string key;

    std::vector<std::string> nonRes;

};

bool SAF4mmgbsaPO(int argc, char** argv, POdata& podata);

#endif	/* SAF4MMGBSAPO_H */

