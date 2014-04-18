/* 
 * File:   Sdf.cpp
 * Author: zhang30
 * 
 * Created on August 10, 2012, 3:12 PM
 */

#include "Sdf.h"

#include <iostream>
#include <fstream>

#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

#include "Common/Tokenize.hpp"


namespace LBIND{

Sdf::Sdf() {
}

Sdf::Sdf(const Sdf& orig) {
}

Sdf::~Sdf() {
}

void Sdf::parse(const std::string& fileName){

    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Sdf::parse >> Cannot open file" << fileName << std::endl;
    }

    std::string fileLine=""; 

    static const boost::regex terRegex("^TER");  
    
    while(inFile){
        std::getline(inFile, fileLine);
//
//        if(boost::regex_search(fileLine,what,terRegex)){
//            newMolecule=true;
//        }
    }
}


std::string Sdf::getInfo(const std::string& fileName, const std::string& keyword){

    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Sdf::parse >> Cannot open file" << fileName << std::endl;
    }

    std::string fileLine="";
    std::string info="";

    static const boost::regex terRegex(keyword.c_str());  
    boost::smatch what;
    
    int count=3;
    
    while(inFile){
        std::getline(inFile, fileLine);

        if(boost::regex_search(fileLine,what,terRegex)){
            count=0;
        }
        if(count==1){            
            info=fileLine;
        }
        count=count+1;
    }
    
    inFile.close();
    
    return info;
}

std::string Sdf::getTitle(const std::string& fileName){

    std::ifstream inFile;
    try {
        inFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "Sdf::parse >> Cannot open file" << fileName << std::endl;
    }
    
    std::string fileLine="";
    std::string info="";
    std::getline(inFile, fileLine);
    std::vector<std::string> tokens;
    tokenize(fileLine, tokens); 
    if(tokens.size() > 0){
        info=tokens[0];
    }    
    return info;
}


} //namespace LBIND
