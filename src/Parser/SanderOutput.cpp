/* 
 * File:   SanderOutput.cpp
 * Author: zhang30
 * 
 * Created on August 30, 2012, 6:17 PM
 */

#include "SanderOutput.h"
#include "Common/Tokenize.hpp"
#include "Structure/Sstrm.hpp"

#include <boost/regex.hpp>

namespace LBIND{

SanderOutput::SanderOutput() {
}

SanderOutput::SanderOutput(const SanderOutput& orig) {
}

SanderOutput::~SanderOutput() {
}


double SanderOutput::getEAmber(std::string sanderOutFile){
    std::ifstream inFile;
    try {
        inFile.open(sanderOutFile.c_str());
    }
    catch(...){
        std::cout << "SanderOutput::getEAmber >> Cannot open file" << sanderOutFile << std::endl;
    }

    static const boost::regex finalRegex("FINAL RESULTS");
    static const boost::regex eamberRegex("EAMBER");
    
    boost::smatch what;
    std::string fileLine="";
    
    bool finalFlag=false;
    double energy=0;

    while(inFile){
        std::getline(inFile, fileLine);

        if(boost::regex_search(fileLine,what,finalRegex)){
            finalFlag=true;
        }
        
        if(finalFlag){
            if(boost::regex_search(fileLine,what,eamberRegex)){
                std::vector<std::string> tokens;
                tokenize(fileLine, tokens);
                energy=Sstrm<double, std::string>(tokens[2]);
                return energy;
            }            
        }
    }
    
    return energy;
}

double SanderOutput::getEnergy(std::string sanderOutFile){
    std::ifstream inFile;
    try {
        inFile.open(sanderOutFile.c_str());
    }
    catch(...){
        std::cout << "SanderOutput::getEnergy >> Cannot open file" << sanderOutFile << std::endl;
    }

    static const boost::regex finalRegex("FINAL RESULTS");
    static const boost::regex enRegex("ENERGY");
    
    boost::smatch what;
    std::string fileLine="";
    
    bool finalFlag=false;
    bool enFlag=false;
    double energy=0;

    while(inFile){
        std::getline(inFile, fileLine);

        if(boost::regex_search(fileLine,what,finalRegex)){
            finalFlag=true;
        }
        
        if(enFlag){
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);
            energy=Sstrm<double, std::string>(tokens[1]);            
            return energy;
        }
        
        if(finalFlag){
            if(boost::regex_search(fileLine,what,enRegex)){
                enFlag=true;
            }            
        }
    }
    
    return energy;
}

}//namespace LBIND
