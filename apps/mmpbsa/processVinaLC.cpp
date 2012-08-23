/* 
 * File:   processVinaLC.cpp
 * Author: zhang30
 *
 * Created on August 22, 2012, 4:30 PM
 */

#include <cstdlib>
#include <string>
#include <vector>

#include "src/Parser/GZstream.h"
#include "src/Common/Tokenize.hpp"
#include "src/Common/File.hpp"

using namespace LBIND;

/*
 * 
 */
int main(int argc, char** argv) {

    std::string inputFName=argv[1];
    
    iGZstream inGZFile;
    try {
        inGZFile.open(inputFName.c_str());
    }
    catch(...){
        std::cout << "processVinaLC >> Cannot open pdbqt.gz file" << inputFName << std::endl;
    }    
    
    
    const std::string recStr="REMARK RECEPTOR";
    const std::string ligStr="REMARK LIGAND";
    
    std::string fileLine="";
    std::string targetDir="";
    std::string ligName="";
    
    int count=0;
    
    bool fileOpen=false;
    
    std::ofstream outFile;
    
    while(std::getline(inGZFile, fileLine)){
        
        if(count==2){
            count=0;
            std::string fileName=targetDir+ligName+".pdbqt";
            outFile.open(fileName.c_str());
            fileOpen=true;
        }

        if(fileLine.compare(0,15, recStr)==0 ){
            ++count;
            
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);
            
            if(tokens.size()==3){
                std::vector<std::string> subTokens;
                std::string delimiter="/";
                tokenize(tokens[2], subTokens,delimiter);
                
                if(subTokens.size()>1){
                    getPathName(tokens[2], targetDir);
//                    std::cout << targetDir << std::endl;
                    targetDir=targetDir+"/poses/";
                }else{
                    targetDir="poses/";
                }
                
                std::string cmd="mkdir -p "+targetDir;
                system(cmd.c_str());
            }else{
                std::cerr << "processVinaLC >> REMARK RECEPTOR label error!" << std::endl;                
            }
            if(fileOpen){
                outFile.close();
                fileOpen=false;
            }
        }
        
        if(fileLine.compare(0,13, ligStr)==0 ){
            ++count;
            
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);  
            ligName=tokens[3];
        }
        
        if(count==0){
            if(fileOpen){
                outFile << fileLine <<std::endl;
            }
        }
        
    }
        
    
    
    return 0;
}

