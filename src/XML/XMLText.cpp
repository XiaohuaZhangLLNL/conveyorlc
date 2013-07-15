/* 
 * File:   XMLText.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:46 PM
 */

#include <cstring>

#include "XMLText.h"
#include "XMLParsingData.h"
#include "XMLDocument.h"
#include "XMLVisitor.hpp"

namespace LBIND{

void XMLText::Print(FILE* cfile, int depth) const {
    assert(cfile);
    if (cdata) {
        int i;
        fprintf(cfile, "\n");
        for (i = 0; i < depth; i++) {
            fprintf(cfile, "    ");
        }
        fprintf(cfile, "<![CDATA[%s]]>\n", value.c_str()); // unformatted output
    } else {
        std::string buffer;
        EncodeString(value, &buffer);
        fprintf(cfile, "%s", buffer.c_str());
    }
}

void XMLText::CopyTo(XMLText* target) const {
    XMLNode::CopyTo(target);
    target->cdata = cdata;
}

bool XMLText::Accept(XMLVisitor* visitor) const {
    return visitor->Visit(*this);
}

XMLNode* XMLText::Clone() const {
    XMLText* clone = 0;
    clone = new XMLText("");

    if (!clone)
        return 0;

    CopyTo(clone);
    return clone;
}


void XMLText::StreamIn(std::istream * in, std::string * tag) {
    while (in->good()) {
        int c = in->peek();
        if (!cdata && (c == '<')) {
            return;
        }
        if (c <= 0) {
            XMLDocument* document = GetDocument();
            if (document)
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0, XML_ENCODING_UNKNOWN);
            return;
        }

        (*tag) += (char) c;
        in->get(); // "commits" the peek made above

        if (cdata && c == '>' && tag->size() >= 3) {
            size_t len = tag->size();
            if ((*tag)[len - 2] == ']' && (*tag)[len - 3] == ']') {
                // terminator of cdata.
                return;
            }
        }
    }
}


const char* XMLText::Parse(const char* p, XMLParsingData* data, XMLEncoding encoding) {
    value = "";
    XMLDocument* document = GetDocument();

    if (data) {
        data->Stamp(p, encoding);
        location = data->Cursor();
    }

    const char* const startTag = "<![CDATA[";
    const char* const endTag = "]]>";

    if (cdata || StringEqual(p, startTag, false, encoding)) {
        cdata = true;

        if (!StringEqual(p, startTag, false, encoding)) {
            if (document)
                document->SetError(XML_ERROR_PARSING_CDATA, p, data, encoding);
            return 0;
        }
        p += strlen(startTag);

        // Keep all the white space, ignore the encoding, etc.
        while (p && *p
                && !StringEqual(p, endTag, false, encoding)
                ) {
            value += *p;
            ++p;
        }

        std::string dummy;
        p = ReadText(p, &dummy, false, endTag, false, encoding);
        return p;
    } else {
        bool ignoreWhite = true;

        const char* end = "<";
        p = ReadText(p, &value, ignoreWhite, end, false, encoding);
        if (p && *p)
            return p - 1; // don't truncate the '<'
        return 0;
    }
}

bool XMLText::Blank() const {
    for (unsigned i = 0; i < value.length(); i++)
        if (!IsWhiteSpace(value[i]))
            return false;
    return true;
}

}//namespace LBIND
