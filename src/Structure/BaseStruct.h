/* 
 * File:   BaseStruct.h
 * Author: zhang
 *
 * Created on August 23, 2010, 5:05 PM
 */

#ifndef BASESTRUCT_H
#define	BASESTRUCT_H

#include <string>

namespace LBIND{

class BaseStruct {
public:
    BaseStruct();
    BaseStruct(const BaseStruct& orig);
    virtual ~BaseStruct();

    void setName(const std::string name);

    std::string getName();

    void setID(const int& id); 

    int  getID();
    
private:
    int itsID;
    std::string itsName;

};

}//namespace LBIND
#endif	/* BASESTRUCT_H */

