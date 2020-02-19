/* 
 * File:   CDT4mmgbsaPO.h
 * Author: zhang
 *
 * Created on April 22, 2014, 1:58 PM
 */

#ifndef CDT4MMGBSAPO_H
#define	CDT4MMGBSAPO_H

#include <string>
#include "CDT4mmgbsa.h"

struct POdata{

    bool keep;
    bool score_only;
    bool newapp;
    int version;
    double intDiel;
    std::string dockInDir;
    std::string recFile;
    std::string ligFile;
    std::string minimizeFlg;

};

bool CDT4mmgbsaPO(int argc, char** argv, POdata& podata);

#endif	/* CDT4MMGBSAPO_H */

