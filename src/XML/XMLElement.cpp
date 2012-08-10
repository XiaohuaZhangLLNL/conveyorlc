/* 
 * File:   XMLElement.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:40 PM
 */

#include <cstring>

#include "XMLElement.h"
#include "XMLVisitor.hpp"
#include "XMLText.h"
#include "XMLDocument.h"
#include "XMLParsingData.h"

namespace LBIND{
XMLElement::XMLElement(const char * _value)
: XMLNode(XMLNode::TINYXML_ELEMENT) {
    firstChild = lastChild = 0;
    value = _value;
}



XMLElement::XMLElement(const std::string& _value)
: XMLNode(XMLNode::TINYXML_ELEMENT) {
    firstChild = lastChild = 0;
    value = _value;
}


XMLElement::XMLElement(const XMLElement& copy)
: XMLNode(XMLNode::TINYXML_ELEMENT) {
    firstChild = lastChild = 0;
    copy.CopyTo(this);
}

XMLElement& XMLElement::operator=(const XMLElement& base) {
    ClearThis();
    base.CopyTo(this);
    return *this;
}

XMLElement::~XMLElement() {
    ClearThis();
}

void XMLElement::ClearThis() {
    Clear();
    while (attributeSet.First()) {
        XMLAttribute* node = attributeSet.First();
        attributeSet.Remove(node);
        delete node;
    }
}

const char* XMLElement::Attribute(const char* name) const {
    const XMLAttribute* node = attributeSet.Find(name);
    if (node)
        return node->Value();
    return 0;
}



const std::string* XMLElement::Attribute(const std::string& name) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    if (attrib)
        return &attrib->ValueStr();
    return 0;
}


const char* XMLElement::Attribute(const char* name, int* i) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    const char* result = 0;

    if (attrib) {
        result = attrib->Value();
        if (i) {
            attrib->QueryIntValue(i);
        }
    }
    return result;
}



const std::string* XMLElement::Attribute(const std::string& name, int* i) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    const std::string* result = 0;

    if (attrib) {
        result = &attrib->ValueStr();
        if (i) {
            attrib->QueryIntValue(i);
        }
    }
    return result;
}


const char* XMLElement::Attribute(const char* name, double* d) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    const char* result = 0;

    if (attrib) {
        result = attrib->Value();
        if (d) {
            attrib->QueryDoubleValue(d);
        }
    }
    return result;
}



const std::string* XMLElement::Attribute(const std::string& name, double* d) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    const std::string* result = 0;

    if (attrib) {
        result = &attrib->ValueStr();
        if (d) {
            attrib->QueryDoubleValue(d);
        }
    }
    return result;
}


int XMLElement::QueryIntAttribute(const char* name, int* ival) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    if (!attrib)
        return XML_NO_ATTRIBUTE;
    return attrib->QueryIntValue(ival);
}

int XMLElement::QueryUnsignedAttribute(const char* name, unsigned* value) const {
    const XMLAttribute* node = attributeSet.Find(name);
    if (!node)
        return XML_NO_ATTRIBUTE;

    int ival = 0;
    int result = node->QueryIntValue(&ival);
    *value = (unsigned) ival;
    return result;
}

int XMLElement::QueryBoolAttribute(const char* name, bool* bval) const {
    const XMLAttribute* node = attributeSet.Find(name);
    if (!node)
        return XML_NO_ATTRIBUTE;

    int result = XML_WRONG_TYPE;
    if (StringEqual(node->Value(), "true", true, XML_ENCODING_UNKNOWN)
            || StringEqual(node->Value(), "yes", true, XML_ENCODING_UNKNOWN)
            || StringEqual(node->Value(), "1", true, XML_ENCODING_UNKNOWN)) {
        *bval = true;
        result = XML_SUCCESS;
    } else if (StringEqual(node->Value(), "false", true, XML_ENCODING_UNKNOWN)
            || StringEqual(node->Value(), "no", true, XML_ENCODING_UNKNOWN)
            || StringEqual(node->Value(), "0", true, XML_ENCODING_UNKNOWN)) {
        *bval = false;
        result = XML_SUCCESS;
    }
    return result;
}




int XMLElement::QueryIntAttribute(const std::string& name, int* ival) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    if (!attrib)
        return XML_NO_ATTRIBUTE;
    return attrib->QueryIntValue(ival);
}


int XMLElement::QueryDoubleAttribute(const char* name, double* dval) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    if (!attrib)
        return XML_NO_ATTRIBUTE;
    return attrib->QueryDoubleValue(dval);
}



int XMLElement::QueryDoubleAttribute(const std::string& name, double* dval) const {
    const XMLAttribute* attrib = attributeSet.Find(name);
    if (!attrib)
        return XML_NO_ATTRIBUTE;
    return attrib->QueryDoubleValue(dval);
}


void XMLElement::SetAttribute(const char * name, int val) {
    XMLAttribute* attrib = attributeSet.FindOrCreate(name);
    if (attrib) {
        attrib->SetIntValue(val);
    }
}



void XMLElement::SetAttribute(const std::string& name, int val) {
    XMLAttribute* attrib = attributeSet.FindOrCreate(name);
    if (attrib) {
        attrib->SetIntValue(val);
    }
}


void XMLElement::SetDoubleAttribute(const char * name, double val) {
    XMLAttribute* attrib = attributeSet.FindOrCreate(name);
    if (attrib) {
        attrib->SetDoubleValue(val);
    }
}



void XMLElement::SetDoubleAttribute(const std::string& name, double val) {
    XMLAttribute* attrib = attributeSet.FindOrCreate(name);
    if (attrib) {
        attrib->SetDoubleValue(val);
    }
}
 

void XMLElement::SetAttribute(const char * cname, const char * cvalue) {
    XMLAttribute* attrib = attributeSet.FindOrCreate(cname);
    if (attrib) {
        attrib->SetValue(cvalue);
    }
}



void XMLElement::SetAttribute(const std::string& _name, const std::string& _value) {
    XMLAttribute* attrib = attributeSet.FindOrCreate(_name);
    if (attrib) {
        attrib->SetValue(_value);
    }
}


void XMLElement::Print(FILE* cfile, int depth) const {
    int i;
    assert(cfile);
    for (i = 0; i < depth; i++) {
        fprintf(cfile, "    ");
    }

    fprintf(cfile, "<%s", value.c_str());

    const XMLAttribute* attrib;
    for (attrib = attributeSet.First(); attrib; attrib = attrib->Next()) {
        fprintf(cfile, " ");
        attrib->Print(cfile, depth);
    }

    // There are 3 different formatting approaches:
    // 1) An element without children is printed as a <foo /> node
    // 2) An element with only a text child is printed as <foo> text </foo>
    // 3) An element with children is printed on multiple lines.
    XMLNode* node;
    if (!firstChild) {
        fprintf(cfile, " />");
    } else if (firstChild == lastChild && firstChild->ToText()) {
        fprintf(cfile, ">");
        firstChild->Print(cfile, depth + 1);
        fprintf(cfile, "</%s>", value.c_str());
    } else {
        fprintf(cfile, ">");

        for (node = firstChild; node; node = node->NextSibling()) {
            if (!node->ToText()) {
                fprintf(cfile, "\n");
            }
            node->Print(cfile, depth + 1);
        }
        fprintf(cfile, "\n");
        for (i = 0; i < depth; ++i) {
            fprintf(cfile, "    ");
        }
        fprintf(cfile, "</%s>", value.c_str());
    }
}

void XMLElement::CopyTo(XMLElement* target) const {
    // superclass:
    XMLNode::CopyTo(target);

    // Element class: 
    // Clone the attributes, then clone the children.
    const XMLAttribute* attribute = 0;
    for (attribute = attributeSet.First();
            attribute;
            attribute = attribute->Next()) {
        target->SetAttribute(attribute->Name(), attribute->Value());
    }

    XMLNode* node = 0;
    for (node = firstChild; node; node = node->NextSibling()) {
        target->LinkEndChild(node->Clone());
    }
}

bool XMLElement::Accept(XMLVisitor* visitor) const {
    if (visitor->VisitEnter(*this, attributeSet.First())) {
        for (const XMLNode* node = FirstChild(); node; node = node->NextSibling()) {
            if (!node->Accept(visitor))
                break;
        }
    }
    return visitor->VisitExit(*this);
}

XMLNode* XMLElement::Clone() const {
    XMLElement* clone = new XMLElement(Value());
    if (!clone)
        return 0;

    CopyTo(clone);
    return clone;
}

const char* XMLElement::GetText() const {
    const XMLNode* child = this->FirstChild();
    if (child) {
        const XMLText* childText = child->ToText();
        if (childText) {
            return childText->Value();
        }
    }
    return 0;
}

void XMLElement::RemoveAttribute(const char * name) {

    std::string str(name);
    XMLAttribute* node = attributeSet.Find(str);

    if (node) {
        attributeSet.Remove(node);
        delete node;
    }
}



void XMLElement::StreamIn(std::istream * in, std::string * tag) {
    // We're called with some amount of pre-parsing. That is, some of "this"
    // element is in "tag". Go ahead and stream to the closing ">"
    while (in->good()) {
        int c = in->get();
        if (c <= 0) {
            XMLDocument* document = GetDocument();
            if (document)
                document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0, XML_ENCODING_UNKNOWN);
            return;
        }
        (*tag) += (char) c;

        if (c == '>')
            break;
    }

    if (tag->length() < 3) return;

    // Okay...if we are a "/>" tag, then we're done. We've read a complete tag.
    // If not, identify and stream.

    if (tag->at(tag->length() - 1) == '>'
            && tag->at(tag->length() - 2) == '/') {
        // All good!
        return;
    } else if (tag->at(tag->length() - 1) == '>') {
        // There is more. Could be:
        //		text
        //		cdata text (which looks like another node)
        //		closing tag
        //		another node.
        for (;;) {
            StreamWhiteSpace(in, tag);

            // Do we have text?
            if (in->good() && in->peek() != '<') {
                // Yep, text.
                XMLText text("");
                text.StreamIn(in, tag);

                // What follows text is a closing tag or another node.
                // Go around again and figure it out.
                continue;
            }

            // We now have either a closing tag...or another node.
            // We should be at a "<", regardless.
            if (!in->good()) return;
            assert(in->peek() == '<');
            int tagIndex = (int) tag->length();

            bool closingTag = false;
            bool firstCharFound = false;

            for (;;) {
                if (!in->good())
                    return;

                int c = in->peek();
                if (c <= 0) {
                    XMLDocument* document = GetDocument();
                    if (document)
                        document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0, XML_ENCODING_UNKNOWN);
                    return;
                }

                if (c == '>')
                    break;

                *tag += (char) c;
                in->get();

                // Early out if we find the CDATA id.
                if (c == '[' && tag->size() >= 9) {
                    size_t len = tag->size();
                    const char* start = tag->c_str() + len - 9;
                    if (strcmp(start, "<![CDATA[") == 0) {
                        assert(!closingTag);
                        break;
                    }
                }

                if (!firstCharFound && c != '<' && !IsWhiteSpace(c)) {
                    firstCharFound = true;
                    if (c == '/')
                        closingTag = true;
                }
            }
            // If it was a closing tag, then read in the closing '>' to clean up the input stream.
            // If it was not, the streaming will be done by the tag.
            if (closingTag) {
                if (!in->good())
                    return;

                int c = in->get();
                if (c <= 0) {
                    XMLDocument* document = GetDocument();
                    if (document)
                        document->SetError(XML_ERROR_EMBEDDED_NULL, 0, 0, XML_ENCODING_UNKNOWN);
                    return;
                }
                assert(c == '>');
                *tag += (char) c;

                // We are done, once we've found our closing tag.
                return;
            } else {
                // If not a closing tag, id it, and stream.
                const char* tagloc = tag->c_str() + tagIndex;
                XMLNode* node = Identify(tagloc, XML_DEFAULT_ENCODING);
                if (!node)
                    return;
                node->StreamIn(in, tag);
                delete node;
                node = 0;

                // No return: go around from the beginning: text, closing tag, or node.
            }
        }
    }
}


const char* XMLElement::Parse(const char* p, XMLParsingData* data, XMLEncoding encoding) {
    p = SkipWhiteSpace(p, encoding);
    XMLDocument* document = GetDocument();

    if (!p || !*p) {
        if (document) document->SetError(XML_ERROR_PARSING_ELEMENT, 0, 0, encoding);
        return 0;
    }

    if (data) {
        data->Stamp(p, encoding);
        location = data->Cursor();
    }

    if (*p != '<') {
        if (document) document->SetError(XML_ERROR_PARSING_ELEMENT, p, data, encoding);
        return 0;
    }

    p = SkipWhiteSpace(p + 1, encoding);

    // Read the name.
    const char* pErr = p;

    p = ReadName(p, &value, encoding);
    if (!p || !*p) {
        if (document) document->SetError(XML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, encoding);
        return 0;
    }

    std::string endTag("</");
    endTag += value;

    // Check for and read attributes. Also look for an empty
    // tag or an end tag.
    while (p && *p) {
        pErr = p;
        p = SkipWhiteSpace(p, encoding);
        if (!p || !*p) {
            if (document) document->SetError(XML_ERROR_READING_ATTRIBUTES, pErr, data, encoding);
            return 0;
        }
        if (*p == '/') {
            ++p;
            // Empty tag.
            if (*p != '>') {
                if (document) document->SetError(XML_ERROR_PARSING_EMPTY, p, data, encoding);
                return 0;
            }
            return (p + 1);
        } else if (*p == '>') {
            // Done with attributes (if there were any.)
            // Read the value -- which can include other
            // elements -- read the end tag, and return.
            ++p;
            p = ReadValue(p, data, encoding); // Note this is an Element method, and will set the error if one happens.
            if (!p || !*p) {
                // We were looking for the end tag, but found nothing.
                // Fix for [ 1663758 ] Failure to report error on bad XML
                if (document) document->SetError(XML_ERROR_READING_END_TAG, p, data, encoding);
                return 0;
            }

            // We should find the end tag now
            // note that:
            // </foo > and
            // </foo> 
            // are both valid end tags.
            if (StringEqual(p, endTag.c_str(), false, encoding)) {
                p += endTag.length();
                p = SkipWhiteSpace(p, encoding);
                if (p && *p && *p == '>') {
                    ++p;
                    return p;
                }
                if (document) document->SetError(XML_ERROR_READING_END_TAG, p, data, encoding);
                return 0;
            } else {
                if (document) document->SetError(XML_ERROR_READING_END_TAG, p, data, encoding);
                return 0;
            }
        } else {
            // Try to read an attribute:
            XMLAttribute* attrib = new XMLAttribute();
            if (!attrib) {
                return 0;
            }

            attrib->SetDocument(document);
            pErr = p;
            p = attrib->Parse(p, data, encoding);

            if (!p || !*p) {
                if (document) document->SetError(XML_ERROR_PARSING_ELEMENT, pErr, data, encoding);
                delete attrib;
                return 0;
            }

            // Handle the strange case of double attributes:
            XMLAttribute* node = attributeSet.Find(attrib->NameTStr());

            if (node) {
                if (document) document->SetError(XML_ERROR_PARSING_ELEMENT, pErr, data, encoding);
                delete attrib;
                return 0;
            }

            attributeSet.Add(attrib);
        }
    }
    return p;
}

const char* XMLElement::ReadValue(const char* p, XMLParsingData* data, XMLEncoding encoding) {
    XMLDocument* document = GetDocument();

    // Read in text and elements in any order.
    const char* pWithWhiteSpace = p;
    p = SkipWhiteSpace(p, encoding);

    while (p && *p) {
        if (*p != '<') {
            // Take what we have, make a text element.
            XMLText* textNode = new XMLText("");

            if (!textNode) {
                return 0;
            }

            if (XMLBase::IsWhiteSpaceCondensed()) {
                p = textNode->Parse(p, data, encoding);
            } else {
                // Special case: we want to keep the white space
                // so that leading spaces aren't removed.
                p = textNode->Parse(pWithWhiteSpace, data, encoding);
            }

            if (!textNode->Blank())
                LinkEndChild(textNode);
            else
                delete textNode;
        }
        else {
            // We hit a '<'
            // Have we hit a new element or an end tag? This could also be
            // a XMLText in the "CDATA" style.
            if (StringEqual(p, "</", false, encoding)) {
                return p;
            } else {
                XMLNode* node = Identify(p, encoding);
                if (node) {
                    p = node->Parse(p, data, encoding);
                    LinkEndChild(node);
                }
                else {
                    return 0;
                }
            }
        }
        pWithWhiteSpace = p;
        p = SkipWhiteSpace(p, encoding);
    }

    if (!p) {
        if (document) document->SetError(XML_ERROR_READING_ELEMENT_VALUE, 0, 0, encoding);
    }
    return p;
}

}//namespace LBIND
