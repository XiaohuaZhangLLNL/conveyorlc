/* 
 * File:   XMLComment.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:43 PM
 */

#ifndef XMLCOMMENT_H
#define	XMLCOMMENT_H

#include "XMLNode.h"

/**	An XML comment.
 */

namespace LBIND{

class XMLComment : public XMLNode {
public:
    /// Constructs an empty comment.

    XMLComment() : XMLNode(XMLNode::TINYXML_COMMENT) {
    }
    /// Construct a comment from text.

    XMLComment(const char* _value) : XMLNode(XMLNode::TINYXML_COMMENT) {
        SetValue(_value);
    }
    XMLComment(const XMLComment&);
    XMLComment& operator=(const XMLComment& base);

    virtual ~XMLComment() {
    }

    /// Returns a copy of this Comment.
    virtual XMLNode* Clone() const;
    // Write this Comment to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    /*	Attribtue parsing starts: at the ! of the !--
                                             returns: next char past '>'
     */
    virtual const char* Parse(const char* p, XMLParsingData* data, XMLEncoding encoding);

    virtual const XMLComment* ToComment() const {
        return this;
    } ///< Cast to a more defined type. Will return null not of the requested type.

    virtual XMLComment* ToComment() {
        return this;
    } ///< Cast to a more defined type. Will return null not of the requested type.

    /** Walk the XML tree visiting this node and all of its children. 
     */
    virtual bool Accept(XMLVisitor* visitor) const;

protected:
    void CopyTo(XMLComment* target) const;

    // used to be public

    virtual void StreamIn(std::istream * in, std::string * tag);

    //	virtual void StreamOut( XML_OSTREAM * out ) const;

private:

};

}//namespace LBIND

#endif	/* XMLCOMMENT_H */

