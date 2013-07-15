/* 
 * File:   BaseParser.h
 * Author: zhang
 *
 * Created on August 23, 2010, 4:05 PM
 */

#ifndef BASEPARSER_H
#define	BASEPARSER_H
#include <string>

namespace LBIND{

class BaseParser {
public:
    BaseParser();
    BaseParser(const BaseParser& orig);
    virtual ~BaseParser();

    virtual void read();
    virtual void write();
private:
    std::string errorMessage;

};

}//namespace LBIND

#endif	/* BASEPARSER_H */

