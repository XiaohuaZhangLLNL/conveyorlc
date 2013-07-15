/* 
 * File:   Base.h
 * Author: zhang30
 *
 * Created on December 6, 2011, 8:38 AM
 */

#ifndef BASE_H
#define	BASE_H

#include <string>

namespace LBIND {
class Base {
public:
    Base();
    Base(int idVal, std::string nameVal);
    Base(const Base& orig);
    virtual ~Base();
    void setID(int idVal);
    int getID();
    void setName(std::string nameVal);
    std::string getName();
private:
    int id;
    std::string name;

};
} //namespace LBIND 
#endif	/* BASE_H */

