/* 
 * File:   Control.h
 * Author: zhang30
 *
 * Created on January 19, 2012, 2:25 PM
 */

#ifndef CONTROL_H
#define	CONTROL_H

#include <string>

namespace LBIND {

class Control {
public:
    Control();
    Control(const Control& orig);
    virtual ~Control();
    
    double getAngleCrit();
    
    bool parseInput(int argc, char** argv);
    void printHeader();    
    
private:
    std::string input;
    std::string output;
    
    int printLev;
    
    bool help;
    bool help_advanced;
    
    double angleCrit;
};

#endif	/* CONTROL_H */

} //namespace LBIND 
