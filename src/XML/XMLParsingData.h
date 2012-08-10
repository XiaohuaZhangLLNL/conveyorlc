/* 
 * File:   XMLParsingData.h
 * Author: zhang30
 *
 * Created on December 1, 2011, 11:49 AM
 */

#ifndef XMLPARSINGDATA_H
#define	XMLPARSINGDATA_H

#include <cassert>
#include "XMLUtils.hpp"

namespace LBIND{

class XMLParsingData {
    friend class XMLDocument;
public:
    void Stamp(const char* now, XMLEncoding encoding);

    const XMLCursor& Cursor() const {
        return cursor;
    }

private:
    // Only used by the document!

    XMLParsingData(const char* start, int _tabsize, int row, int col) {
        assert(start);
        stamp = start;
        tabsize = _tabsize;
        cursor.row = row;
        cursor.col = col;
    }

    XMLCursor cursor;
    const char* stamp;
    int tabsize;
};

} //namespace LBIND

#endif	/* XMLPARSINGDATA_H */

