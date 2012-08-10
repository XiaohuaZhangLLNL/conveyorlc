/* 
 * File:   XMLText.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:46 PM
 */

#ifndef XMLTEXT_H
#define	XMLTEXT_H

#include "XMLNode.h"

/** XML text. A text node can have 2 ways to output the next. "normal" output 
        and CDATA. It will default to the mode it was parsed from the XML file and
        you generally want to leave it alone, but you can change the output mode with 
        SetCDATA() and query it with CDATA().
 */

namespace LBIND{

class XMLText : public XMLNode {
    friend class XMLElement;
public:

    /** Constructor for text element. By default, it is treated as 
            normal, encoded text. If you want it be output as a CDATA text
            element, set the parameter _cdata to 'true'
     */
    XMLText(const char * initValue) : XMLNode(XMLNode::TINYXML_TEXT) {
        SetValue(initValue);
        cdata = false;
    }

    virtual ~XMLText() {
    }


    /// Constructor.

    XMLText(const std::string& initValue) : XMLNode(XMLNode::TINYXML_TEXT) {
        SetValue(initValue);
        cdata = false;
    }


    XMLText(const XMLText& copy) : XMLNode(XMLNode::TINYXML_TEXT) {
        copy.CopyTo(this);
    }

    XMLText& operator=(const XMLText& base) {
        base.CopyTo(this);
        return *this;
    }

    // Write this text object to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    /// Queries whether this represents text using a CDATA section.

    bool CDATA() const {
        return cdata;
    }
    /// Turns on or off a CDATA representation of text.

    void SetCDATA(bool _cdata) {
        cdata = _cdata;
    }

    virtual const char* Parse(const char* p, XMLParsingData* data, XMLEncoding encoding);

    virtual const XMLText* ToText() const {
        return this;
    } ///< Cast to a more defined type. Will return null not of the requested type.

    virtual XMLText* ToText() {
        return this;
    } ///< Cast to a more defined type. Will return null not of the requested type.

    /** Walk the XML tree visiting this node and all of its children. 
     */
    virtual bool Accept(XMLVisitor* content) const;

protected:
    ///  [internal use] Creates a new Element and returns it.
    virtual XMLNode* Clone() const;
    void CopyTo(XMLText* target) const;

    bool Blank() const; // returns true if all white space and new lines
    // [internal use]

    virtual void StreamIn(std::istream * in, std::string * tag);


private:
    bool cdata; // true if this should be input and output as a CDATA style text element
};

}//namespace LBIND

#endif	/* XMLTEXT_H */

