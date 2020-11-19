/* 
 * File:   SDL1ReceptorPO.h
 * Author: zhang
 *
 * Created on March 18, 2014, 12:10 PM
 */

#ifndef SDL1RECEPTORPO_H
#define	SDL1RECEPTORPO_H

#include <string>

struct POdata{
    std::string inputFile;
    std::string outputFile;
    std::string protonateFlg;
    std::string minimizeFlg;
    std::string siteFlg;
    std::string sitebylig;
    std::string forceRedoFlg;
    std::string cutProt;
    double radius;
    int surfSphNum;
    int gridSphNum;
    double spacing;
    double cutoffCoef;
    double boxExtend;
    double minVol;
    double cutRadius;
    int version;
    bool keep;
};

bool SDL1ReceptorPO(int argc, char** argv, POdata& podata);

#endif	/* SDL1RECEPTORPO_H */

