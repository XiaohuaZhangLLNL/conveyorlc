/* 
 * File:   PPL1ReceptorPO.h
 * Author: zhang
 *
 * Created on March 18, 2014, 12:10 PM
 */

#ifndef PPL1RECEPTORPO_H
#define	PPL1RECEPTORPO_H

#include <string>

struct POdata{
    std::string inputFile;
    std::string outputFile;
    std::string protonateFlg;
    std::string minimizeFlg;
    std::string siteFlg;
    std::string forceRedoFlg;
    std::string cutProt;
    double radius;
    int surfSphNum;
    int gridSphNum;
    double spacing;
    double cutoffCoef;
    double minVol;
    double cutRadius;
    int version;
};

bool PPL1ReceptorPO(int argc, char** argv, POdata& podata);

#endif	/* PPL1RECEPTORPO_H */

