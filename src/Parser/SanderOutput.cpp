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


bool SanderOutput::getEAmber(std::string sanderOutFile, double& energy){
    std::ifstream inFile;
    try {
        inFile.open(sanderOutFile.c_str());
    }
    catch(...){
        std::cout << "SanderOutput::getEAmber >> Cannot open file" << sanderOutFile << std::endl;
    }

    std::vector<std::string> lineVector;

    static const boost::regex eamberRegex("EAMBER");
    static const boost::regex enRegex("ENERGY");
    
    boost::smatch what;
    std::string fileLine="";
    
    //bool finalFlag=false;
    while(inFile){
        std::getline(inFile, fileLine);
        lineVector.push_back(fileLine);
    }

    for(int i=lineVector.size()-1; i>0; i--){
        std::string fileLine=lineVector[i];
        if(boost::regex_search(fileLine,what,eamberRegex)){
            std::cout << fileLine << std::endl;
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);
            energy=Sstrm<double, std::string>(tokens[2]);
            return true;            
        }
    }
    
    return false;
}

bool SanderOutput::getEnergy(std::string sanderOutFile, double& energy){
    std::ifstream inFile;
    try {
        inFile.open(sanderOutFile.c_str());
    }
    catch(...){
        std::cout << "SanderOutput::getEnergy >> Cannot open file" << sanderOutFile << std::endl;
    }

    std::vector<std::string> lineVector;
	
    static const boost::regex finalRegex("NSTEP");
    static const boost::regex enRegex("ENERGY");
    
    boost::smatch what;
    std::string fileLine="";
    
    //bool finalFlag=false;

    while(inFile){
        std::getline(inFile, fileLine);
        lineVector.push_back(fileLine);
    }

    for(int i=lineVector.size()-1; i>0; i--){
        std::string fileLine=lineVector[i];
        if(boost::regex_search(fileLine,what,finalRegex)){
            //finalFlag=true;
            std::string enLine=lineVector[i+1];
            std::cout << enLine << std::endl;
            std::vector<std::string> tokens;
            tokenize(enLine, tokens);
            energy=Sstrm<double, std::string>(tokens[1]);
            return true;            
        }
        
        //if(finalFlag){
        //    if(boost::regex_search(fileLine,what,enRegex)){
        //        std::getline(inFile, fileLine);
        //        std::vector<std::string> tokens;
        //        tokenize(fileLine, tokens);
        //        energy=Sstrm<double, std::string>(tokens[1]);
        //        return true;                
        //    }            
        //}
    }
    
    return false;
}

}//namespace LBIND
