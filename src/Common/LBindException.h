/* 
 * File:   LBindException.h
 * Author: zhang30
 *
 * Created on December 12, 2011, 11:01 AM
 */

#ifndef LBINDEXCEPTION_H
#define	LBINDEXCEPTION_H

#include <string>

namespace LBIND {

class LBindException {
public:
    LBindException();
    LBindException(std::string mesg);
    LBindException(const LBindException& orig);
    virtual ~LBindException() throw();
    std::string what();
private:
    std::string message;
};

}//namespace LBIND 
#endif	/* LBINDEXCEPTION_H */

