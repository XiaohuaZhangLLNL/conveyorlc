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
    bool score_only;
    bool newapp;
    bool useLigName;
    int version;
    double intDiel;
    std::string recFile;
    std::string ligFile;
    std::string minimizeFlg;
    std::string ligNameFile;
};

bool SAF4mmgbsaPO(int argc, char** argv, POdata& podata);

#endif	/* SAF4MMGBSAPO_H */

