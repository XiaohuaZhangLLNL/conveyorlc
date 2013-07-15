/* 
 * File:   XMLUtils.hpp
 * Author: zhang
 *
 * Created on November 30, 2011, 10:06 PM
 */

#ifndef XMLUTILS_HPP
#define	XMLUTILS_HPP

namespace LBIND{
/*	Internal structure for tracking location of items 
        in the XML file.
 */
struct XMLCursor {

    XMLCursor() {
        Clear();
    }

    void Clear() {
        row = col = -1;
    }

    int row; // 0 based.
    int col; // 0 based.
};


// Only used by Attribute::Query functions

enum {
    XML_SUCCESS,
    XML_NO_ATTRIBUTE,
    XML_WRONG_TYPE
};


// Used by the parsing routines.

enum XMLEncoding {
    XML_ENCODING_UNKNOWN,
    XML_ENCODING_UTF8,
    XML_ENCODING_LEGACY
};


const XMLEncoding XML_DEFAULT_ENCODING = XML_ENCODING_UNKNOWN;

}//namespace LBIND

#endif	/* XMLUTILS_HPP */

