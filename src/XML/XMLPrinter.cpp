/* 
 * File:   XMLPrinter.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:57 PM
 */

#include "XMLPrinter.h"
#include "XMLElement.h"
#include "XMLAttribute.h"
#include "XMLText.h"
#include "XMLDeclaration.h"
#include "XMLComment.h"
#include "XMLUnknown.h"

namespace LBIND{

bool XMLPrinter::VisitEnter(const XMLDocument&) {
    return true;
}

bool XMLPrinter::VisitExit(const XMLDocument&) {
    return true;
}

bool XMLPrinter::VisitEnter(const XMLElement& element, const XMLAttribute* firstAttribute) {
    DoIndent();
    buffer += "<";
    buffer += element.Value();

    for (const XMLAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next()) {
        buffer += " ";
        attrib->Print(0, 0, &buffer);
    }

    if (!element.FirstChild()) {
        buffer += " />";
        DoLineBreak();
    } else {
        buffer += ">";
        if (element.FirstChild()->ToText()
                && element.LastChild() == element.FirstChild()
                && element.FirstChild()->ToText()->CDATA() == false) {
            simpleTextPrint = true;
            // no DoLineBreak()!
        } else {
            DoLineBreak();
        }
    }
    ++depth;
    return true;
}

bool XMLPrinter::VisitExit(const XMLElement& element) {
    --depth;
    if (!element.FirstChild()) {
        // nothing.
    } else {
        if (simpleTextPrint) {
            simpleTextPrint = false;
        } else {
            DoIndent();
        }
        buffer += "</";
        buffer += element.Value();
        buffer += ">";
        DoLineBreak();
    }
    return true;
}

bool XMLPrinter::Visit(const XMLText& text) {
    if (text.CDATA()) {
        DoIndent();
        buffer += "<![CDATA[";
        buffer += text.Value();
        buffer += "]]>";
        DoLineBreak();
    } else if (simpleTextPrint) {
        std::string str;
        XMLBase::EncodeString(text.ValueTStr(), &str);
        buffer += str;
    } else {
        DoIndent();
        std::string str;
        XMLBase::EncodeString(text.ValueTStr(), &str);
        buffer += str;
        DoLineBreak();
    }
    return true;
}

bool XMLPrinter::Visit(const XMLDeclaration& declaration) {
    DoIndent();
    declaration.Print(0, 0, &buffer);
    DoLineBreak();
    return true;
}

bool XMLPrinter::Visit(const XMLComment& comment) {
    DoIndent();
    buffer += "<!--";
    buffer += comment.Value();
    buffer += "-->";
    DoLineBreak();
    return true;
}

bool XMLPrinter::Visit(const XMLUnknown& unknown) {
    DoIndent();
    buffer += "<";
    buffer += unknown.Value();
    buffer += ">";
    DoLineBreak();
    return true;
}

}//namespace LBIND

