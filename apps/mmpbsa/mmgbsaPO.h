/* 
 * File:   mmgbsaPO.h
 * Author: zhang30
 *
 * Created on October 2, 2012, 10:55 AM
 */

#ifndef MMGBSAPO_H
#define	MMGBSAPO_H

#include <string>

struct POdata{
    std::string recFile;
    std::string ligFile;
    std::string outputFile;
    std::string xmlFile;
    bool pbFlag;
};

bool mmgbsaPO(int argc, char** argv, POdata& podata);

#endif	/* MMGBSAPO_H */

