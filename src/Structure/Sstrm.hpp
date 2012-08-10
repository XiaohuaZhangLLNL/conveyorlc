/* 
 * File:   sstrm.h
 * Author: zhang
 *
 * Created on June 3, 2010, 12:02 PM
 */

#ifndef _SSTRM_HPP
#define	_SSTRM_HPP

#include <algorithm>
#include <sstream>
#include <boost/regex.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>


namespace LBIND {

template <typename T1, typename T2>
T1 Sstrm(T2 inValue){
    std::stringstream ss;
    ss << inValue;
    T1 outValue;
    ss >> outValue;
    
    return(outValue);
}

template <typename T>
T SsMatch(boost::smatch inValue, int i){
    std::stringstream ss;
    ss << inValue[i];
    T outValue;
    ss >> outValue;

    return(outValue);
}


}//namespace LBIND 

#endif	/* _SSTRM_HPP */

