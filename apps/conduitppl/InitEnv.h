//
// Created by Zhang, Xiaohua on 2018-12-17.
//

#ifndef CONVEYORLC_INITENV_H
#define CONVEYORLC_INITENV_H
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <iostream>

inline bool initConveyorlcEnv(std::string& workDir, std::string& inputDir, std::string& dataPath){
    //! get  working directory
    char* WORKDIR=getenv("WORKDIR");

    if(WORKDIR==0) {
        // use current working directory for working directory
        char BUFFER[200];
        getcwd(BUFFER, sizeof (BUFFER));
        workDir = BUFFER;
    }else{
        workDir=WORKDIR;
    }

    //! get  input directory
    char* INPUTDIR=getenv("INPUTDIR");

    if(INPUTDIR==0) {
        // use current working directory for input directory
        char BUFFER[200];
        getcwd(BUFFER, sizeof (BUFFER));
        inputDir = BUFFER;
    }else{
        inputDir = INPUTDIR;
    }

    //! get LBindData
    char* LBINDDATA=getenv("LBindData");

    if(LBINDDATA==0){
        std::cerr << "LBdindData environment is not defined!" << std::endl;
        return false;
    }
    dataPath=LBINDDATA;

    return true;
}


#endif //CONVEYORLC_INITENV_H
