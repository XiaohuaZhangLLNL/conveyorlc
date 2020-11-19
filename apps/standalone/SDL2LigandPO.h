/* 
 * File:   SDL2LigandPO.h
 * Author: zhang
 *
 * Created on March 24, 2014, 11:17 AM
 */

#ifndef SDL2LIGANDPO_H
#define	SDL2LIGANDPO_H

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
    std::string ligNameFile;
    int firstLigID;
    bool restart;
    bool score_only;
    bool keep;
    bool useLigName;
    int version;
    double intDiel;
};

bool SDL2LigandPO(int argc, char** argv, POdata& podata);

#endif	/* SDL2LIGANDPO_H */

