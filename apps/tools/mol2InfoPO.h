/* 
 * File:   mol2InfoPO.h
 * Author: zhang30
 *
 * Created on September 26, 2012, 4:30 PM
 */

#ifndef MOL2INFOPO_H
#define	MOL2INFOPO_H

#include <string>

struct POdata{
    std::string mol2File;
    std::string outputFile;
};

bool mol2InfoPO(int argc, char** argv, POdata& podata);


#endif	/* MOL2INFOPO_H */

