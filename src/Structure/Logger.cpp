/* 
 * File:   Logger.cpp
 * Author: zhang
 * 
 * Created on August 10, 2011, 3:50 PM
 */

#include <iostream>

#include "Logger.h"
#include "Common/LBindException.h"

namespace LBIND {

// Global static pointer used to ensure a single instance of the class.
Logger* Logger::m_pInstance = 0;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
 */

Logger* Logger::Instance() {
    if (!m_pInstance) // Only allow one instance of class to be generated.
        m_pInstance = new Logger;

    return m_pInstance;
}

bool Logger::openLogFile(std::string logFile){
    
    try {
        outFile.open(logFile.c_str(), std::ios::out | std::ios::trunc);
    }
    catch(...){
        std::string message="Logger::openLogFile>> Cannot open file " + logFile;
        std::cout << message << std::endl;
        throw LBindException(message);
    }  
    
    return true;
    
}

void Logger::writeToLogFile(std::string logger){
    outFile << logger << std::endl;
}


bool Logger::closeLogFile(){
    outFile.close();
    return true;
}

}//namespace LBIND 





