/* 
 * File:   XMLDocument.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:54 PM
 */

#include "XMLDocument.h"
#include "XMLVisitor.hpp"
#include "XMLParsingData.h"
#include "XMLConstant.hpp"
#include "XMLDeclaration.h"

namespace LBIND{

XMLDocument::XMLDocument() : XMLNode(XMLNode::TINYXML_DOCUMENT) {
    tabsize = 4;
    useMicrosoftBOM = false;
    ClearError();
}

XMLDocument::XMLDocument(const char * documentName) : XMLNode(XMLNode::TINYXML_DOCUMENT) {
    tabsize = 4;
    useMicrosoftBOM = false;
    value = documentName;
    ClearError();
}



XMLDocument::XMLDocument(const std::string& documentName) : XMLNode(XMLNode::TINYXML_DOCUMENT) {
    tabsize = 4;
    useMicrosoftBOM = false;
    value = documentName;
    ClearError();
}

XMLDocument::XMLDocument(const XMLDocument& copy) : XMLNode(XMLNode::TINYXML_DOCUMENT) {
    copy.CopyTo(this);
}

XMLDocument& XMLDocument::operator=(const XMLDocument& copy) {
    Clear();
    copy.CopyTo(this);
    return *this;
}

bool XMLDocument::LoadFile(XMLEncoding encoding) {
    return LoadFile(Value(), encoding);
}

bool XMLDocument::SaveFile() const {
    return SaveFile(Value());
}


bool XMLDocument::LoadFile(const char* _filename, XMLEncoding encoding) {
    std::string filename(_filename);
    value = filename;

    // reading in binary mode so that tinyxml can normalize the EOL
    FILE* file = fopen(value.c_str(), "rb");

    if (file) {
        bool result = LoadFile(file, encoding);
        fclose(file);
        return result;
    } else {
        SetError(XML_ERROR_OPENING_FILE, 0, 0, XML_ENCODING_UNKNOWN);
        return false;
    }
}

bool XMLDocument::LoadFile(FILE* file, XMLEncoding encoding) {
    if (!file) {
        SetError(XML_ERROR_OPENING_FILE, 0, 0, XML_ENCODING_UNKNOWN);
        return false;
    }

    // Delete the existing data:
    Clear();
    location.Clear();

    // Get the file size, so we can pre-allocate the string. HUGE speed impact.
    long length = 0;
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Strange case, but good to handle up front.
    if (length <= 0) {
        SetError(XML_ERROR_DOCUMENT_EMPTY, 0, 0, XML_ENCODING_UNKNOWN);
        return false;
    }

    // Subtle bug here. TinyXml did use fgets. But from the XML spec:
    // 2.11 End-of-Line Handling
    // <snip>
    // <quote>
    // ...the XML processor MUST behave as if it normalized all line breaks in external 
    // parsed entities (including the document entity) on input, before parsing, by translating 
    // both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
    // a single #xA character.
    // </quote>
    //
    // It is not clear fgets does that, and certainly isn't clear it works cross platform. 
    // Generally, you expect fgets to translate from the convention of the OS to the c/unix
    // convention, and not work generally.

    /*
    while( fgets( buf, sizeof(buf), file ) )
    {
            data += buf;
    }
     */

    char* buf = new char[ length + 1 ];
    buf[0] = 0;

    if (fread(buf, length, 1, file) != 1) {
        delete [] buf;
        SetError(XML_ERROR_OPENING_FILE, 0, 0, XML_ENCODING_UNKNOWN);
        return false;
    }

    // Process the buffer in place to normalize new lines. (See comment above.)
    // Copies from the 'p' to 'q' pointer, where p can advance faster if
    // a newline-carriage return is hit.
    //
    // Wikipedia:
    // Systems based on ASCII or a compatible character set use either LF  (Line feed, '\n', 0x0A, 10 in decimal) or 
    // CR (Carriage return, '\r', 0x0D, 13 in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
    //		* LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix, Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
    //		* CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes, CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
    //		* CR:    Commodore 8-bit machines, Apple II family, Mac OS up to version 9 and OS-9

    const char* p = buf; // the read head
    char* q = buf; // the write head
    const char CR = 0x0d;
    const char LF = 0x0a;

    buf[length] = 0;
    while (*p) {
        assert(p < (buf + length));
        assert(q <= (buf + length));
        assert(q <= p);

        if (*p == CR) {
            *q++ = LF;
            p++;
            if (*p == LF) { // check for CR+LF (and skip LF)
                p++;
            }
        } else {
            *q++ = *p++;
        }
    }
    assert(q <= (buf + length));
    *q = 0;

    Parse(buf, 0, encoding);

    delete [] buf;
    return !Error();
}

bool XMLDocument::SaveFile(const char * filename) const {
    // The old c stuff lives on...
    FILE* fp = fopen(filename, "w");
    if (fp) {
        bool result = SaveFile(fp);
        fclose(fp);
        return result;
    }
    return false;
}

bool XMLDocument::SaveFile(FILE* fp) const {
    if (useMicrosoftBOM) {
        const unsigned char XML_UTF_LEAD_0 = 0xefU;
        const unsigned char XML_UTF_LEAD_1 = 0xbbU;
        const unsigned char XML_UTF_LEAD_2 = 0xbfU;

        fputc(XML_UTF_LEAD_0, fp);
        fputc(XML_UTF_LEAD_1, fp);
        fputc(XML_UTF_LEAD_2, fp);
    }
    Print(fp, 0);
    return (ferror(fp) == 0);
}

void XMLDocument::CopyTo(XMLDocument* target) const {
    XMLNode::CopyTo(target);

    target->error = error;
    target->errorId = errorId;
    target->errorDesc = errorDesc;
    target->tabsize = tabsize;
    target->errorLocation = errorLocation;
    target->useMicrosoftBOM = useMicrosoftBOM;

    XMLNode* node = 0;
    for (node = firstChild; node; node = node->NextSibling()) {
        target->LinkEndChild(node->Clone());
    }
}

XMLNode* XMLDocument::Clone() const {
    XMLDocument* clone = new XMLDocument();
    if (!clone)
        return 0;

    CopyTo(clone);
    return clone;
}

void XMLDocument::Print(FILE* cfile, int depth) const {
    assert(cfile);
    for (const XMLNode* node = FirstChild(); node; node = node->NextSibling()) {
        node->Print(cfile, depth);
        fprintf(cfile, "\n");
    }
}

bool XMLDocument::Accept(XMLVisitor* visitor) const {
    if (visitor->VisitEnter(*this)) {
        for (const XMLNode* node = FirstChild(); node; node = node->NextSibling()) {
            if (!node->Accept(visitor))
                break;
        }
    }
    return visitor->VisitExit(*this);
}

void XMLDocument::StreamIn(std::istream * in, std::string * tag) {
    // The basic issue with a document is that we don't know what we're
    // streaming. Read something presumed to be a tag (and hope), then
    // identify it, and call the appropriate stream method on the tag.
    //
    // This "pre-streaming" will never read the closing ">" so the
    // sub-tag can orient itself.

    if (!StreamTo(in, '<', tag)) {
        SetError(XML_ERROR_PARSING_EMPTY, 0, 0, XML_ENCODING_UNKNOWN);
        return;
    }

    while (in->good()) {
        int tagIndex = (int) tag->length();
        while (in->good() && in->peek() != '>') {
            int c = in->get();
            if (c <= 0) {
                SetError(XML_ERROR_EMBEDDED_NULL, 0, 0, XML_ENCODING_UNKNOWN);
                break;
            }
            (*tag) += (char) c;
        }

        if (in->good()) {
            // We now have something we presume to be a node of 
            // some sort. Identify it, and call the node to
            // continue streaming.
            XMLNode* node = Identify(tag->c_str() + tagIndex, XML_DEFAULT_ENCODING);

            if (node) {
                node->StreamIn(in, tag);
                bool isElement = node->ToElement() != 0;
                delete node;
                node = 0;

                // If this is the root element, we're done. Parsing will be
                // done by the >> operator.
                if (isElement) {
                    return;
                }
            } else {
                SetError(XML_ERROR, 0, 0, XML_ENCODING_UNKNOWN);
                return;
            }
        }
    }
    // We should have returned sooner.
    SetError(XML_ERROR, 0, 0, XML_ENCODING_UNKNOWN);
}

         

const char* XMLDocument::Parse(const char* p, XMLParsingData* prevData, XMLEncoding encoding) {
    ClearError();

    // Parse away, at the document level. Since a document
    // contains nothing but other tags, most of what happens
    // here is skipping white space.
    if (!p || !*p) {
        SetError(XML_ERROR_DOCUMENT_EMPTY, 0, 0, XML_ENCODING_UNKNOWN);
        return 0;
    }

    // Note that, for a document, this needs to come
    // before the while space skip, so that parsing
    // starts from the pointer we are given.
    location.Clear();
    if (prevData) {
        location.row = prevData->cursor.row;
        location.col = prevData->cursor.col;
    } else {
        location.row = 0;
        location.col = 0;
    }
    XMLParsingData data(p, TabSize(), location.row, location.col);
    location = data.Cursor();

    if (encoding == XML_ENCODING_UNKNOWN) {
        // Check for the Microsoft UTF-8 lead bytes.
        const unsigned char* pU = (const unsigned char*) p;
        if (*(pU + 0) && *(pU + 0) == XML_UTF_LEAD_0
                && *(pU + 1) && *(pU + 1) == XML_UTF_LEAD_1
                && *(pU + 2) && *(pU + 2) == XML_UTF_LEAD_2) {
            encoding = XML_ENCODING_UTF8;
            useMicrosoftBOM = true;
        }
    }

    p = SkipWhiteSpace(p, encoding);
    if (!p) {
        SetError(XML_ERROR_DOCUMENT_EMPTY, 0, 0, XML_ENCODING_UNKNOWN);
        return 0;
    }

    while (p && *p) {
        XMLNode* node = Identify(p, encoding);
        if (node) {
            p = node->Parse(p, &data, encoding);
            LinkEndChild(node);
        } else {
            break;
        }

        // Did we get encoding info?
        if (encoding == XML_ENCODING_UNKNOWN
                && node->ToDeclaration()) {
            XMLDeclaration* dec = node->ToDeclaration();
            const char* enc = dec->Encoding();
            assert(enc);

            if (*enc == 0)
                encoding = XML_ENCODING_UTF8;
            else if (StringEqual(enc, "UTF-8", true, XML_ENCODING_UNKNOWN))
                encoding = XML_ENCODING_UTF8;
            else if (StringEqual(enc, "UTF8", true, XML_ENCODING_UNKNOWN))
                encoding = XML_ENCODING_UTF8; // incorrect, but be nice
            else
                encoding = XML_ENCODING_LEGACY;
        }

        p = SkipWhiteSpace(p, encoding);
    }

    // Was this empty?
    if (!firstChild) {
        SetError(XML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding);
        return 0;
    }

    // All is well.
    return p;
}

void XMLDocument::SetError(int err, const char* pError, XMLParsingData* data, XMLEncoding encoding) {
    // The first error in a chain is more accurate - don't set again!
    if (error)
        return;

    assert(err > 0 && err < XML_ERROR_STRING_COUNT);
    error = true;
    errorId = err;
    errorDesc = errorString[ errorId ];

    errorLocation.Clear();
    if (pError && data) {
        data->Stamp(pError, encoding);
        errorLocation = data->Cursor();
    }
}

}//namespace LBIND
