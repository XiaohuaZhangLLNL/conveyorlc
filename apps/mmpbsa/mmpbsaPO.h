/* 
 * File:   mmpbsaPO.h
 * Author: zhang30
 *
 * Created on October 2, 2012, 10:55 AM
 */

#ifndef MMPBSAPO_H
#define	MMPBSAPO_H

#include <string>

struct POdata{
    std::string recFile;
    std::string ligFile;
    std::string outputFile;
    std::string xmlFile;
    bool pbFlag;
};

bool mmpbsaPO(int argc, char** argv, POdata& podata);

#endif	/* MMPBSAPO_H */

