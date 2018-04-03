/* 
 * File:   PPL2VolumeFilterPO.h
 * Author: zhang
 *
 * Created on March 24, 2014, 11:17 AM
 */

#ifndef PPL2VOLUMEFILTERPO_H
#define	PPL2VOLUMEFILTERPO_H

#include <string>

struct POdata{
    std::string sdfFile;
    std::string outputFile;
    std::string xmlOut;
    std::string xmlRst;
    double volume;
    bool restart;
    bool protonate;
};

bool PPL2VolumeFilterPO(int argc, char** argv, POdata& podata);

#endif	/* PPL2VOLUMEFILTERPO_H */

