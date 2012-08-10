/* 
 * File:   Logger.h
 * Author: zhang
 *
 * Created on August 10, 2011, 3:50 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H

#include <fstream>
#include <string>

namespace LBIND {

class Logger {
    
public:
    static Logger* Instance();
    bool openLogFile(std::string logFile);
    void writeToLogFile(std::string logger);
    bool closeLogFile();

private:

    Logger() {
    }; // Private so that it can  not be called

    Logger(Logger const&) {
    }; // copy constructor is private

    Logger& operator=(Logger const&) {
        return *this; // The assignment operator is not full implement, so DO NOT use it. 
                      // return *this just to get rid of the annoying warning message at compiling.
    }; // assignment operator is private
    static Logger* m_pInstance;
    std::ofstream outFile;

};

} //namespace LBIND 

#endif	/* LOGGER_H */

