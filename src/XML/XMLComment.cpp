/* 
 * File:   XMLComment.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:43 PM
 */

#include "XMLComment.h"
#include "XMLVisitor.hpp"
#include "XMLParsingData.h"
#include "XMLDocument.h"

namespace LBIND{

XMLComment::XMLComment(const XMLComment& copy) : XMLNode(XMLNode::TINYXML_COMMENT) {
    copy.CopyTo(this);
}

XMLComment& XMLComment::operator=(const XMLComment& base) {
    Clear();
    base.CopyTo(this);
    return *this;
}

void XMLComment::Print(FILE* cfile, int depth) const {
    assert(cfile);
    for (int i = 0; i < depth; i++) {
        fprintf(cfile, "    ");
    }
    fprintf(cfile, "<!--%s-->", value.c_str());
}

void XMLComment::CopyTo(XMLComment* target) const {
    XMLNode::CopyTo(target);
}

bool XMLComment::Accept(XMLVisitor* visitor) const {
    return visitor->Visit(*this);
}

XMLNode* XMLComment::Clone() const {
    XMLComment* clone = new XMLComment();

    if (!clone)
        return 0;

    CopyTo(clone);
    return clone;
}


void XMLComment::StreamIn(std::istream * in, std::string * tag) {
    while (in->good()) {
        int c = in->get();
        if (c <= 0) {
            XMLDocument* document = GetDocument();
            if (document)
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0, XML_ENCODING_UNKNOWN);
            return;
        }

        (*tag) += (char) c;

        if (c == '>'
                && tag->at(tag->length() - 2) == '-'
                && tag->at(tag->length() - 3) == '-') {
            // All is well.
            return;
        }
    }
}


const char* XMLComment::Parse(const char* p, XMLParsingData* data, XMLEncoding encoding) {
    XMLDocument* document = GetDocument();
    value = "";

    p = SkipWhiteSpace(p, encoding);

    if (data) {
        data->Stamp(p, encoding);
        location = data->Cursor();
    }
    const char* startTag = "<!--";
    const char* endTag = "-->";

    if (!StringEqual(p, startTag, false, encoding)) {
        if (document)
            document->SetError(XML_ERROR_PARSING_COMMENT, p, data, encoding);
        return 0;
    }
    p += strlen(startTag);

    // [ 1475201 ] TinyXML parses entities in comments
    // Oops - ReadText doesn't work, because we don't want to parse the entities.
    // p = ReadText( p, &value, false, endTag, false, encoding );
    //
    // from the XML spec:
    /*
     [Definition: Comments may appear anywhere in a document outside other markup; in addition, 
                  they may appear within the document type declaration at places allowed by the grammar. 
                              They are not part of the document's character data; an XML processor MAY, but need not, 
                              make it possible for an application to retrieve the text of comments. For compatibility, 
                              the string "--" (double-hyphen) MUST NOT occur within comments.] Parameter entity 
                              references MUST NOT be recognized within comments.

                              An example of a comment:

                              <!-- declarations for <head> & <body> -->
     */

    value = "";
    // Keep all the white space.
    while (p && *p && !StringEqual(p, endTag, false, encoding)) {
        value.append(p, 1);
        ++p;
    }
    if (p && *p)
        p += strlen(endTag);

    return p;
}

} //namespace LBIND
