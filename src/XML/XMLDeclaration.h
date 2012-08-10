/* 
 * File:   XMLDeclaration.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:48 PM
 */

#ifndef XMLDECLARATION_H
#define	XMLDECLARATION_H

#include <string>
#include "XMLNode.h"

/** In correct XML the declaration is the first entry in the file.
        @verbatim
                <?xml version="1.0" standalone="yes"?>
        @endverbatim

        TinyXml will happily read or write files without a declaration,
        however. There are 3 possible attributes to the declaration:
        version, encoding, and standalone.

        Note: In this version of the code, the attributes are
        handled as special cases, not generic attributes, simply
        because there can only be at most 3 and they are always the same.
 */

namespace LBIND{

class XMLDeclaration : public XMLNode {
public:
    /// Construct an empty declaration.

    XMLDeclaration() : XMLNode(XMLNode::TINYXML_DECLARATION) {
    }

    /// Constructor.
    XMLDeclaration(const std::string& _version,
            const std::string& _encoding,
            const std::string& _standalone);

    /// Construct.
    XMLDeclaration(const char* _version,
            const char* _encoding,
            const char* _standalone);

    XMLDeclaration(const XMLDeclaration& copy);
    XMLDeclaration& operator=(const XMLDeclaration& copy);

    virtual ~XMLDeclaration() {
    }

    /// Version. Will return an empty string if none was found.

    const char *Version() const {
        return version.c_str();
    }
    /// Encoding. Will return an empty string if none was found.

    const char *Encoding() const {
        return encoding.c_str();
    }
    /// Is this a standalone document?

    const char *Standalone() const {
        return standalone.c_str();
    }

    /// Creates a copy of this Declaration and returns it.
    virtual XMLNode* Clone() const;
    // Print this declaration to a FILE stream.
    virtual void Print(FILE* cfile, int depth, std::string* str) const;

    virtual void Print(FILE* cfile, int depth) const {
        Print(cfile, depth, 0);
    }

    virtual const char* Parse(const char* p, XMLParsingData* data, XMLEncoding encoding);

    virtual const XMLDeclaration* ToDeclaration() const {
        return this;
    } ///< Cast to a more defined type. Will return null not of the requested type.

    virtual XMLDeclaration* ToDeclaration() {
        return this;
    } ///< Cast to a more defined type. Will return null not of the requested type.

    /** Walk the XML tree visiting this node and all of its children. 
     */
    virtual bool Accept(XMLVisitor* visitor) const;

protected:
    void CopyTo(XMLDeclaration* target) const;
    // used to be public

    virtual void StreamIn(std::istream * in, std::string * tag);


private:

    std::string version;
    std::string encoding;
    std::string standalone;
};

}//namespace LBIND

#endif	/* XMLDECLARATION_H */

