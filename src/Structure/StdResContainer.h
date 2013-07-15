/* 
 * File:   StdResContainer.h
 * Author: zhang30
 *
 * Created on September 5, 2012, 4:34 PM
 */

#ifndef STDRESCONTAINER_H
#define	STDRESCONTAINER_H

#include <string>
#include <vector>

namespace LBIND{
    
class StdResContainer {
public:
    StdResContainer();
    StdResContainer(const StdResContainer& orig);
    virtual ~StdResContainer();
    
    bool find(const char* resName);
    bool find(const std::string& resName);
    
private:
    void parseXML(std::string& fileName);
    
    std::vector<std::string> stdResidues;

};
}//namespace LBIND
#endif	/* STDRESCONTAINER_H */

