/* 
 * File:   PMolException.h
 * Author: zhang
 *
 * Created on August 25, 2010, 10:23 AM
 */

#ifndef PMOLEXCEPTION_H
#define	PMOLEXCEPTION_H

#include <string>
#include <boost/exception/all.hpp>

namespace LBIND{

class PMolException : public boost::exception {
public:
    PMolException();
    PMolException(std::string message);
    PMolException(const PMolException& orig);
    virtual ~PMolException() throw();
    std::string what();
public:
    std::string message;
};
} //namespace LBIND
#endif	/* PMOLEXCEPTION_H */

