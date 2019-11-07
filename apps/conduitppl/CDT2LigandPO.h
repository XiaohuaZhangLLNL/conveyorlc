/* 
 * File:   CDT2LigandPO.h
 * Author: zhang
 *
 * Created on March 24, 2014, 11:17 AM
 */

#ifndef CDT2LIGANDPO_H
#define	CDT2LIGANDPO_H

#include <string>

struct POdata{
    std::string sdfFile;
    std::string outputFile;
    std::string xmlOut;
    std::string cmpName;
    std::string minimizeFlg;
    std::string saveSDF;
    std::string backup;
    std::string skipFile;
    int firstLigID;
    bool restart;
    bool score_only;
    int version;
};

bool CDT2LigandPO(int argc, char** argv, POdata& podata);

#endif	/* CDT2LIGANDPO_H */

