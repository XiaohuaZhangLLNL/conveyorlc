/* 
 * File:   amberToolsPO.h
 * Author: zhang30
 *
 * Created on September 27, 2012, 12:06 PM
 */

#ifndef AMBERTOOLSPO_H
#define	AMBERTOOLSPO_H

#include <string>

struct POdata{
    std::string inputFile;
    std::string outputFile;
    bool getPDBflg;
};

bool amberToolsPO(int argc, char** argv, POdata& podata);

#endif	/* AMBERTOOLSPO_H */

