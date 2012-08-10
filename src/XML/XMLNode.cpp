/* 
 * File:   XMLNode.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:18 PM
 */

#include "XMLNode.h"
#include "XMLDocument.h"
#include "XMLComment.h"
#include "XMLText.h"
#include "XMLUnknown.h"
#include "XMLDeclaration.h"
#include "XMLElement.h"
#include "XMLVisitor.hpp"
#include "XMLParsingData.h"
#include "XMLPrinter.h"

namespace LBIND{

XMLNode::XMLNode(NodeType _type) : XMLBase() {
    parent = 0;
    type = _type;
    firstChild = 0;
    lastChild = 0;
    prev = 0;
    next = 0;
}

XMLNode::~XMLNode() {
    XMLNode* node = firstChild;
    XMLNode* temp = 0;

    while (node) {
        temp = node;
        node = node->next;
        delete temp;
    }
}

void XMLNode::CopyTo(XMLNode* target) const {
    target->SetValue(value.c_str());
    target->userData = userData;
    target->location = location;
}

void XMLNode::Clear() {
    XMLNode* node = firstChild;
    XMLNode* temp = 0;

    while (node) {
        temp = node;
        node = node->next;
        delete temp;
    }

    firstChild = 0;
    lastChild = 0;
}

XMLNode* XMLNode::LinkEndChild(XMLNode* node) {
    assert(node->parent == 0 || node->parent == this);
    assert(node->GetDocument() == 0 || node->GetDocument() == this->GetDocument());

    if (node->Type() == XMLNode::TINYXML_DOCUMENT) {
        delete node;
        if (GetDocument())
            GetDocument()->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, XML_ENCODING_UNKNOWN);
        return 0;
    }

    node->parent = this;

    node->prev = lastChild;
    node->next = 0;

    if (lastChild)
        lastChild->next = node;
    else
        firstChild = node; // it was an empty list.

    lastChild = node;
    return node;
}

XMLNode* XMLNode::InsertEndChild(const XMLNode& addThis) {
    if (addThis.Type() == XMLNode::TINYXML_DOCUMENT) {
        if (GetDocument())
            GetDocument()->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, XML_ENCODING_UNKNOWN);
        return 0;
    }
    XMLNode* node = addThis.Clone();
    if (!node)
        return 0;

    return LinkEndChild(node);
}

XMLNode* XMLNode::InsertBeforeChild(XMLNode* beforeThis, const XMLNode& addThis) {
    if (!beforeThis || beforeThis->parent != this) {
        return 0;
    }
    if (addThis.Type() == XMLNode::TINYXML_DOCUMENT) {
        if (GetDocument())
            GetDocument()->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, XML_ENCODING_UNKNOWN);
        return 0;
    }

    XMLNode* node = addThis.Clone();
    if (!node)
        return 0;
    node->parent = this;

    node->next = beforeThis;
    node->prev = beforeThis->prev;
    if (beforeThis->prev) {
        beforeThis->prev->next = node;
    } else {
        assert(firstChild == beforeThis);
        firstChild = node;
    }
    beforeThis->prev = node;
    return node;
}

XMLNode* XMLNode::InsertAfterChild(XMLNode* afterThis, const XMLNode& addThis) {
    if (!afterThis || afterThis->parent != this) {
        return 0;
    }
    if (addThis.Type() == XMLNode::TINYXML_DOCUMENT) {
        if (GetDocument())
            GetDocument()->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, XML_ENCODING_UNKNOWN);
        return 0;
    }

    XMLNode* node = addThis.Clone();
    if (!node)
        return 0;
    node->parent = this;

    node->prev = afterThis;
    node->next = afterThis->next;
    if (afterThis->next) {
        afterThis->next->prev = node;
    } else {
        assert(lastChild == afterThis);
        lastChild = node;
    }
    afterThis->next = node;
    return node;
}

XMLNode* XMLNode::ReplaceChild(XMLNode* replaceThis, const XMLNode& withThis) {
    if (!replaceThis)
        return 0;

    if (replaceThis->parent != this)
        return 0;

    if (withThis.ToDocument()) {
        // A document can never be a child.	Thanks to Noam.
        XMLDocument* document = GetDocument();
        if (document)
            document->SetError(XML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, XML_ENCODING_UNKNOWN);
        return 0;
    }

    XMLNode* node = withThis.Clone();
    if (!node)
        return 0;

    node->next = replaceThis->next;
    node->prev = replaceThis->prev;

    if (replaceThis->next)
        replaceThis->next->prev = node;
    else
        lastChild = node;

    if (replaceThis->prev)
        replaceThis->prev->next = node;
    else
        firstChild = node;

    delete replaceThis;
    node->parent = this;
    return node;
}

bool XMLNode::RemoveChild(XMLNode* removeThis) {
    if (!removeThis) {
        return false;
    }

    if (removeThis->parent != this) {
        assert(0);
        return false;
    }

    if (removeThis->next)
        removeThis->next->prev = removeThis->prev;
    else
        lastChild = removeThis->prev;

    if (removeThis->prev)
        removeThis->prev->next = removeThis->next;
    else
        firstChild = removeThis->next;

    delete removeThis;
    return true;
}

const XMLNode* XMLNode::FirstChild(const char * _value) const {
    const XMLNode* node;
    for (node = firstChild; node; node = node->next) {
        if (strcmp(node->Value(), _value) == 0)
            return node;
    }
    return 0;
}

const XMLNode* XMLNode::LastChild(const char * _value) const {
    const XMLNode* node;
    for (node = lastChild; node; node = node->prev) {
        if (strcmp(node->Value(), _value) == 0)
            return node;
    }
    return 0;
}

const XMLNode* XMLNode::IterateChildren(const XMLNode* previous) const {
    if (!previous) {
        return FirstChild();
    } else {
        assert(previous->parent == this);
        return previous->NextSibling();
    }
}

const XMLNode* XMLNode::IterateChildren(const char * val, const XMLNode* previous) const {
    if (!previous) {
        return FirstChild(val);
    } else {
        assert(previous->parent == this);
        return previous->NextSibling(val);
    }
}

const XMLNode* XMLNode::NextSibling(const char * _value) const {
    const XMLNode* node;
    for (node = next; node; node = node->next) {
        if (strcmp(node->Value(), _value) == 0)
            return node;
    }
    return 0;
}

const XMLNode* XMLNode::PreviousSibling(const char * _value) const {
    const XMLNode* node;
    for (node = prev; node; node = node->prev) {
        if (strcmp(node->Value(), _value) == 0)
            return node;
    }
    return 0;
}

const XMLElement* XMLNode::FirstChildElement() const {
    const XMLNode* node;

    for (node = FirstChild();
            node;
            node = node->NextSibling()) {
        if (node->ToElement())
            return node->ToElement();
    }
    return 0;
}

const XMLElement* XMLNode::FirstChildElement(const char * _value) const {
    const XMLNode* node;

    for (node = FirstChild(_value);
            node;
            node = node->NextSibling(_value)) {
        if (node->ToElement())
            return node->ToElement();
    }
    return 0;
}

const XMLElement* XMLNode::NextSiblingElement() const {
    const XMLNode* node;

    for (node = NextSibling();
            node;
            node = node->NextSibling()) {
        if (node->ToElement())
            return node->ToElement();
    }
    return 0;
}

const XMLElement* XMLNode::NextSiblingElement(const char * _value) const {
    const XMLNode* node;

    for (node = NextSibling(_value);
            node;
            node = node->NextSibling(_value)) {
        if (node->ToElement())
            return node->ToElement();
    }
    return 0;
}

const XMLDocument* XMLNode::GetDocument() const {
    const XMLNode* node;

    for (node = this; node; node = node->parent) {
        if (node->ToDocument())
            return node->ToDocument();
    }
    return 0;
}

	

std::istream& operator>>(std::istream & in, XMLNode & base) {
    std::string tag;
    tag.reserve(8 * 1000);
    base.StreamIn(&in, &tag);

    base.Parse(tag.c_str(), 0, XML_DEFAULT_ENCODING);
    return in;
}

	

std::ostream& operator<<(std::ostream & out, const XMLNode & base) {
    XMLPrinter printer;
    printer.SetStreamPrinting();
    base.Accept(&printer);
    out << printer.Str();

    return out;
}

std::string& operator<<(std::string& out, const XMLNode& base) {
    XMLPrinter printer;
    printer.SetStreamPrinting();
    base.Accept(&printer);
    out.append(printer.Str());

    return out;
}


XMLNode* XMLNode::Identify(const char* p, XMLEncoding encoding) {
    XMLNode* returnNode = 0;

    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p || *p != '<') {
        return 0;
    }

    p = SkipWhiteSpace(p, encoding);

    if (!p || !*p) {
        return 0;
    }

    // What is this thing? 
    // - Elements start with a letter or underscore, but xml is reserved.
    // - Comments: <!--
    // - Decleration: <?xml
    // - Everthing else is unknown to tinyxml.
    //

    const char* xmlHeader = {"<?xml"};
    const char* commentHeader = {"<!--"};
    const char* dtdHeader = {"<!"};
    const char* cdataHeader = {"<![CDATA["};

    if (StringEqual(p, xmlHeader, true, encoding)) {
#ifdef DEBUG_PARSER
        XML_LOG("XML parsing Declaration\n");
#endif
        returnNode = new XMLDeclaration();
    } else if (StringEqual(p, commentHeader, false, encoding)) {
#ifdef DEBUG_PARSER
        XML_LOG("XML parsing Comment\n");
#endif
        returnNode = new XMLComment();
    } else if (StringEqual(p, cdataHeader, false, encoding)) {
#ifdef DEBUG_PARSER
        XML_LOG("XML parsing CDATA\n");
#endif
        XMLText* text = new XMLText("");
        text->SetCDATA(true);
        returnNode = text;
    } else if (StringEqual(p, dtdHeader, false, encoding)) {
#ifdef DEBUG_PARSER
        XML_LOG("XML parsing Unknown(1)\n");
#endif
        returnNode = new XMLUnknown();
    } else if (IsAlpha(*(p + 1), encoding)
            || *(p + 1) == '_') {
#ifdef DEBUG_PARSER
        XML_LOG("XML parsing Element\n");
#endif
        returnNode = new XMLElement("");
    } else {
#ifdef DEBUG_PARSER
        XML_LOG("XML parsing Unknown(2)\n");
#endif
        returnNode = new XMLUnknown();
    }

    if (returnNode) {
        // Set the parent, so it can report errors
        returnNode->parent = this;
    }
    return returnNode;
}

}//namespace LBIND
