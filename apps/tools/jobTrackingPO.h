/* 
 * File:   jobTrackingPO.h
 * Author: zhang30
 *
 * Created on September 17, 2012, 11:43 AM
 */

#ifndef JOBTRACKINGPO_H
#define	JOBTRACKINGPO_H

#include <string>

struct POdata{
    std::string xmlFile;
    std::string listFile;
    std::string outputFile;
    std::string keyword;
    bool isTmpFile;
};

bool jobTrackingPO(int argc, char** argv, POdata& podata);

#endif	/* JOBTRACKINGPO_H */

