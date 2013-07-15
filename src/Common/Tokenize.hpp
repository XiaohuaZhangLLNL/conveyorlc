/* 
 * File:   Tokenize.hpp
 * Author: zhang30
 *
 * Created on January 19, 2012, 2:17 PM
 */

#ifndef TOKENIZE_HPP
#define	TOKENIZE_HPP

#include <algorithm>
#include <cstdlib>
#include <string>
#include <iterator>


namespace LBIND {

template < class ContainerT >
void tokenize(const std::string& str, ContainerT& tokens,
              const std::string& delimiters = " ", const bool trimEmpty = true) {
    typedef ContainerT Base; 
    typedef typename Base::value_type ValueType; 
    typedef typename ValueType::size_type SizeType;
    SizeType pos, lastPos = 0;
    while (true) {
        pos = str.find_first_of(delimiters, lastPos);
        if (pos == std::string::npos) {
            pos = str.length();

            if (pos != lastPos || !trimEmpty)
                tokens.push_back(ValueType(str.data() + lastPos, (SizeType)pos - lastPos));

            break;
        } else {
            if (pos != lastPos || !trimEmpty)
                tokens.push_back(ValueType(str.data() + lastPos, (SizeType)pos - lastPos));
        }

        lastPos = pos + 1;
    }
};


}//namespace LBIND 

#endif	/* TOKENIZE_HPP */

