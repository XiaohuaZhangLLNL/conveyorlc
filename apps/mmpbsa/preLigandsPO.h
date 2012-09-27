/* 
 * File:   preLigandsPO.h
 * Author: zhang30
 *
 * Created on September 24, 2012, 11:33 AM
 */

#ifndef PRELIGANDSPO_H
#define	PRELIGANDSPO_H

#include <string>

struct POdata{
    std::string xmlFile;
    std::string listFile;
    std::string outputFile;
    std::string keyword;
    bool isTmpFile;
};

bool preLigandsPO(int argc, char** argv, POdata& podata);

#endif	/* PRELIGANDSPO_H */

