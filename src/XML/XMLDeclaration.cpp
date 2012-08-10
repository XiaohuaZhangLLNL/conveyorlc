/* 
 * File:   XMLDeclaration.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:48 PM
 */

#include "XMLDeclaration.h"
#include "XMLVisitor.hpp"
#include "XMLDocument.h"
#include "XMLParsingData.h"
#include "XMLAttribute.h"

namespace LBIND{

XMLDeclaration::XMLDeclaration(const char * _version,
        const char * _encoding,
        const char * _standalone)
: XMLNode(XMLNode::TINYXML_DECLARATION) {
    version = _version;
    encoding = _encoding;
    standalone = _standalone;
}


XMLDeclaration::XMLDeclaration(const std::string& _version,
        const std::string& _encoding,
        const std::string& _standalone)
: XMLNode(XMLNode::TINYXML_DECLARATION) {
    version = _version;
    encoding = _encoding;
    standalone = _standalone;
}


XMLDeclaration::XMLDeclaration(const XMLDeclaration& copy)
: XMLNode(XMLNode::TINYXML_DECLARATION) {
    copy.CopyTo(this);
}

XMLDeclaration& XMLDeclaration::operator=(const XMLDeclaration& copy) {
    Clear();
    copy.CopyTo(this);
    return *this;
}

void XMLDeclaration::Print(FILE* cfile, int /*depth*/, std::string* str) const {
    if (cfile) fprintf(cfile, "<?xml ");
    if (str) (*str) += "<?xml ";

    if (!version.empty()) {
        if (cfile) fprintf(cfile, "version=\"%s\" ", version.c_str());
        if (str) {
            (*str) += "version=\"";
            (*str) += version;
            (*str) += "\" ";
        }
    }
    if (!encoding.empty()) {
        if (cfile) fprintf(cfile, "encoding=\"%s\" ", encoding.c_str());
        if (str) {
            (*str) += "encoding=\"";
            (*str) += encoding;
            (*str) += "\" ";
        }
    }
    if (!standalone.empty()) {
        if (cfile) fprintf(cfile, "standalone=\"%s\" ", standalone.c_str());
        if (str) {
            (*str) += "standalone=\"";
            (*str) += standalone;
            (*str) += "\" ";
        }
    }
    if (cfile) fprintf(cfile, "?>");
    if (str) (*str) += "?>";
}

void XMLDeclaration::CopyTo(XMLDeclaration* target) const {
    XMLNode::CopyTo(target);

    target->version = version;
    target->encoding = encoding;
    target->standalone = standalone;
}

bool XMLDeclaration::Accept(XMLVisitor* visitor) const {
    return visitor->Visit(*this);
}

XMLNode* XMLDeclaration::Clone() const {
    XMLDeclaration* clone = new XMLDeclaration();

    if (!clone)
        return 0;

    CopyTo(clone);
    return clone;
}




void XMLDeclaration::StreamIn(std::istream * in, std::string * tag) {
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


const char* XMLDeclaration::Parse(const char* p, XMLParsingData* data, XMLEncoding _encoding) {
    p = SkipWhiteSpace(p, _encoding);
    // Find the beginning, find the end, and look for
    // the stuff in-between.
    XMLDocument* document = GetDocument();
    if (!p || !*p || !StringEqual(p, "<?xml", true, _encoding)) {
        if (document) document->SetError(XML_ERROR_PARSING_DECLARATION, 0, 0, _encoding);
        return 0;
    }
    if (data) {
        data->Stamp(p, _encoding);
        location = data->Cursor();
    }
    p += 5;

    version = "";
    encoding = "";
    standalone = "";

    while (p && *p) {
        if (*p == '>') {
            ++p;
            return p;
        }

        p = SkipWhiteSpace(p, _encoding);
        if (StringEqual(p, "version", true, _encoding)) {
            XMLAttribute attrib;
            p = attrib.Parse(p, data, _encoding);
            version = attrib.Value();
        } else if (StringEqual(p, "encoding", true, _encoding)) {
            XMLAttribute attrib;
            p = attrib.Parse(p, data, _encoding);
            encoding = attrib.Value();
        } else if (StringEqual(p, "standalone", true, _encoding)) {
            XMLAttribute attrib;
            p = attrib.Parse(p, data, _encoding);
            standalone = attrib.Value();
        } else {
            // Read over whatever it is.
            while (p && *p && *p != '>' && !IsWhiteSpace(*p))
                ++p;
        }
    }
    return 0;
}


}//namespace LBIND
