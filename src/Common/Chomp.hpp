/* 
 * File:   Chomp.hpp
 * Author: zhang
 *
 * Created on February 20, 2014, 9:24 PM
 */

#ifndef CHOMP_HPP
#define	CHOMP_HPP

#include <string>

namespace LBIND {
    
inline void chomp(std::string& s) {
    size_t p = s.find_first_not_of(" \t");
    s.erase(0, p);

    p = s.find_last_not_of(" \t");
    if (std::string::npos != p)
        s.erase(p + 1);
}



} // namespace LBIND 

#endif	/* CHOMP_HPP */

