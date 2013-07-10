/* 
 * File:   glideGBSAPO.h
 * Author: zhang30
 *
 * Created on September 24, 2012, 11:33 AM
 */

#ifndef GLIDEGBSA_H
#define	GLIDEGBSA_H

#include <string>

struct POdata{
    std::string sdfFile;
    std::string outputFile;
    std::string xmlOut;
    std::string xmlRst;
    bool restart;
};

bool glideGBSAPO(int argc, char** argv, POdata& podata);

#endif	/* GLIDEGBSA_H */

