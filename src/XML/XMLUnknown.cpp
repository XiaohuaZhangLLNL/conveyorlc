/* 
 * File:   XMLUnknown.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:51 PM
 */

#include "XMLParsingData.h"
#include "XMLDocument.h"
#include "XMLUnknown.h"
#include "XMLVisitor.hpp"


namespace LBIND{

void XMLUnknown::Print(FILE* cfile, int depth) const {
    for (int i = 0; i < depth; i++)
        fprintf(cfile, "    ");
    fprintf(cfile, "<%s>", value.c_str());
}

void XMLUnknown::CopyTo(XMLUnknown* target) const {
    XMLNode::CopyTo(target);
}

bool XMLUnknown::Accept(XMLVisitor* visitor) const {
    return visitor->Visit(*this);
}

XMLNode* XMLUnknown::Clone() const {
    XMLUnknown* clone = new XMLUnknown();

    if (!clone)
        return 0;

    CopyTo(clone);
    return clone;
}



void XMLUnknown::StreamIn(std::istream * in, std::string * tag) {
    while (in->good()) {
        int c = in->get();
        if (c <= 0) {
            XMLDocument* document = GetDocument();
            if (document)
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0, XML_ENCODING_UNKNOWN);
            return;
        }
        (*tag) += (char) c;

        if (c == '>') {
            // All is well.
            return;
        }
    }
}


const char* XMLUnknown::Parse(const char* p, XMLParsingData* data, XMLEncoding encoding) {
    XMLDocument* document = GetDocument();
    p = SkipWhiteSpace(p, encoding);

    if (data) {
        data->Stamp(p, encoding);
        location = data->Cursor();
    }
    if (!p || !*p || *p != '<') {
        if (document) document->SetError(XML_ERROR_PARSING_UNKNOWN, p, data, encoding);
        return 0;
    }
    ++p;
    value = "";

    while (p && *p && *p != '>') {
        value += *p;
        ++p;
    }

    if (!p) {
        if (document)
            document->SetError(XML_ERROR_PARSING_UNKNOWN, 0, 0, encoding);
    }
    if (p && *p == '>')
        return p + 1;
    return p;
}

}//namespace LBIND
