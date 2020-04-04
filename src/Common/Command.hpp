/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Command.hpp
 * Author: zhang30
 *
 * Created on October 11, 2018, 12:42 PM
 */

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "Common/LBindException.h"

namespace LBIND {

inline void command(std::string& cmd, std::string& errMesg){

    struct timespec tim, tim2;
    tim.tv_sec = 1;
    tim.tv_nsec = 0;

    int status;

    while(WEXITSTATUS(status)==127) {
        status = system(cmd.c_str());
        nanosleep(&tim, &tim2);
    }
    if (status < 0) {
        std::string message = strerror(errno);
        throw LBindException(message);
    } else {
        if (WIFEXITED(status)) {
            std::cout << cmd << " return exit code " << WEXITSTATUS(status) << '\n';
        } else {
            throw LBindException(errMesg);
        }
    } 
    return;
}    
    
}

#endif /* COMMAND_HPP */

