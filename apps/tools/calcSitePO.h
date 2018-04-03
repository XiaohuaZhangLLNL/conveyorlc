/* 
 * File:   calcSitePO.h
 * Author: zhang
 *
 * Created on February 20, 2014, 4:59 PM
 */

#ifndef CALCSASAPO_H
#define	CALCSASAPO_H

#include <string>

struct POdata{
    std::string pdbFile;
    std::string outputFile;
    double radius;
    int surfSphNum;
    int gridSphNum;
    int minVol;
};

bool calcSitePO(int argc, char** argv, POdata& podata);

#endif	/* CALCSASAPO_H */

