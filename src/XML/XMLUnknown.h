/* 
 * File:   XMLUnknown.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:51 PM
 */

#ifndef XMLUNKNOWN_H
#define	XMLUNKNOWN_H

#include "XMLNode.h"

/** Any tag that tinyXml doesn't recognize is saved as an
        unknown. It is a tag of text, but should not be modified.
        It will be written back to the XML, unchanged, when the file
        is saved.

        DTD tags get thrown into XMLUnknowns.
 */

namespace LBIND{

class XMLUnknown : public XMLNode {
public:

    XMLUnknown() : XMLNode(XMLNode::TINYXML_UNKNOWN) {
    }

    virtual ~XMLUnknown() {
    }

    XMLUnknown(const XMLUnknown& copy) : XMLNode(XMLNode::TINYXML_UNKNOWN) {
        copy.CopyTo(this);
    }

    XMLUnknown& operator=(const XMLUnknown& copy) {
        copy.CopyTo(this);
        return *this;
    }

    /// Creates a copy of this Unknown and returns it.
    virtual XMLNode* Clone() const;
    // Print this Unknown to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const;

    virtual const char* Parse(const char* p, XMLParsingData* data, XMLEncoding encoding);

    virtual const XMLUnknown* ToUnknown() const {
        return this;
    } ///< Cast to a more defined type. Will return null not of the requested type.

    virtual XMLUnknown* ToUnknown() {
        return this;
    } ///< Cast to a more defined type. Will return null not of the requested type.

    /** Walk the XML tree visiting this node and all of its children. 
     */
    virtual bool Accept(XMLVisitor* content) const;

protected:
    void CopyTo(XMLUnknown* target) const;


    virtual void StreamIn(std::istream * in, std::string * tag);


private:

};

}//namespace LBIND

#endif	/* XMLUNKNOWN_H */

