/* 
 * File:   XMLPrinter.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:57 PM
 */

#ifndef XMLPRINTER_H
#define	XMLPRINTER_H


#include <string>
#include <iostream>
#include <sstream>

#include "XMLVisitor.hpp"

/** Print to memory functionality. The XMLPrinter is useful when you need to:

        -# Print to memory (especially in non-STL mode)
        -# Control formatting (line endings, etc.)

        When constructed, the XMLPrinter is in its default "pretty printing" mode.
        Before calling Accept() you can call methods to control the printing
        of the XML document. After XMLNode::Accept() is called, the printed document can
        be accessed via the CStr(), Str(), and Size() methods.

        XMLPrinter uses the Visitor API.
        @verbatim
        XMLPrinter printer;
        printer.SetIndent( "\t" );

        doc.Accept( &printer );
        fprintf( stdout, "%s", printer.CStr() );
        @endverbatim
 */

namespace LBIND{

class XMLPrinter : public XMLVisitor {
public:

    XMLPrinter() : depth(0), simpleTextPrint(false),
    buffer(), indent("    "), lineBreak("\n") {
    }

    virtual bool VisitEnter(const XMLDocument& doc);
    virtual bool VisitExit(const XMLDocument& doc);

    virtual bool VisitEnter(const XMLElement& element, const XMLAttribute* firstAttribute);
    virtual bool VisitExit(const XMLElement& element);

    virtual bool Visit(const XMLDeclaration& declaration);
    virtual bool Visit(const XMLText& text);
    virtual bool Visit(const XMLComment& comment);
    virtual bool Visit(const XMLUnknown& unknown);

    /** Set the indent characters for printing. By default 4 spaces
            but tab (\t) is also useful, or null/empty string for no indentation.
     */
    void SetIndent(const char* _indent) {
        indent = _indent ? _indent : "";
    }
    /// Query the indention string.

    const char* Indent() {
        return indent.c_str();
    }

    /** Set the line breaking string. By default set to newline (\n). 
            Some operating systems prefer other characters, or can be
            set to the null/empty string for no indenation.
     */
    void SetLineBreak(const char* _lineBreak) {
        lineBreak = _lineBreak ? _lineBreak : "";
    }
    /// Query the current line breaking string.

    const char* LineBreak() {
        return lineBreak.c_str();
    }

    /** Switch over to "stream printing" which is the most dense formatting without 
            linebreaks. Common when the XML is needed for network transmission.
     */
    void SetStreamPrinting() {
        indent = "";
        lineBreak = "";
    }
    /// Return the result.

    const char* CStr() {
        return buffer.c_str();
    }
    /// Return the length of the result string.

    size_t Size() {
        return buffer.size();
    }


    /// Return the result.

    const std::string& Str() {
        return buffer;
    }


private:

    void DoIndent() {
        for (int i = 0; i < depth; ++i)
            buffer += indent;
    }

    void DoLineBreak() {
        buffer += lineBreak;
    }

    int depth;
    bool simpleTextPrint;
    std::string buffer;
    std::string indent;
    std::string lineBreak;
};

}//namespace LBIND

#endif	/* XMLPRINTER_H */

