/* 
 * File:   XMLParsingData.cpp
 * Author: zhang30
 * 
 * Created on December 1, 2011, 11:49 AM
 */

#include "XMLParsingData.h"
#include "XMLConstant.hpp"
#include "XMLBase.h"

namespace LBIND{

void XMLParsingData::Stamp(const char* now, XMLEncoding encoding) {
    assert(now);

    // Do nothing if the tabsize is 0.
    if (tabsize < 1) {
        return;
    }

    // Get the current row, column.
    int row = cursor.row;
    int col = cursor.col;
    const char* p = stamp;
    assert(p);

    while (p < now) {
        // Treat p as unsigned, so we have a happy compiler.
        const unsigned char* pU = (const unsigned char*) p;

        // Code contributed by Fletcher Dunn: (modified by lee)
        switch (*pU) {
            case 0:
                // We *should* never get here, but in case we do, don't
                // advance past the terminating null character, ever
                return;

            case '\r':
                // bump down to the next line
                ++row;
                col = 0;
                // Eat the character
                ++p;

                // Check for \r\n sequence, and treat this as a single character
                if (*p == '\n') {
                    ++p;
                }
                break;

            case '\n':
                // bump down to the next line
                ++row;
                col = 0;

                // Eat the character
                ++p;

                // Check for \n\r sequence, and treat this as a single
                // character.  (Yes, this bizarre thing does occur still
                // on some arcane platforms...)
                if (*p == '\r') {
                    ++p;
                }
                break;

            case '\t':
                // Eat the character
                ++p;

                // Skip to next tab stop
                col = (col / tabsize + 1) * tabsize;
                break;

            case XML_UTF_LEAD_0:
                if (encoding == XML_ENCODING_UTF8) {
                    if (*(p + 1) && *(p + 2)) {
                        // In these cases, don't advance the column. These are
                        // 0-width spaces.
                        if (*(pU + 1) == XML_UTF_LEAD_1 && *(pU + 2) == XML_UTF_LEAD_2)
                            p += 3;
                        else if (*(pU + 1) == 0xbfU && *(pU + 2) == 0xbeU)
                            p += 3;
                        else if (*(pU + 1) == 0xbfU && *(pU + 2) == 0xbfU)
                            p += 3;
                        else {
                            p += 3;
                            ++col;
                        } // A normal character.
                    }
                } else {
                    ++p;
                    ++col;
                }
                break;

            default:
                if (encoding == XML_ENCODING_UTF8) {
                    // Eat the 1 to 4 byte utf8 character.
                    int step = XMLBase::utf8ByteTable[*((const unsigned char*) p)];
                    if (step == 0)
                        step = 1; // Error case from bad encoding, but handle gracefully.
                    p += step;

                    // Just advance one column, of course.
                    ++col;
                } else {
                    ++p;
                    ++col;
                }
                break;
        }
    }
    cursor.row = row;
    cursor.col = col;
    assert(cursor.row >= -1);
    assert(cursor.col >= -1);
    stamp = p;
    assert(stamp);
}

}//namespace LBIND
