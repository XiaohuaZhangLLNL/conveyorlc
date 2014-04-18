/* 
 * File:   Sdf.h
 * Author: zhang30
 *
 * Created on August 10, 2012, 3:12 PM
 */

#ifndef SDF_H
#define	SDF_H

#include <string>

namespace LBIND{

class Sdf {
public:
    Sdf();
    Sdf(const Sdf& orig);
    virtual ~Sdf();
    
    void parse(const std::string& fileName);
    std::string getInfo(const std::string& fileName, const std::string& keyword);
    std::string getTitle(const std::string& fileName);
    
private:

};

} //namespace LBIND
#endif	/* SDF_H */

