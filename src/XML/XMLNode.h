/* 
 * File:   XMLNode.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:18 PM
 */

#ifndef XMLNODE_H
#define	XMLNODE_H

#include "XMLBase.h"

namespace LBIND{

class XMLElement;
class XMLDocument;
class XMLComment;
class XMLUnknown;
class XMLText;
class XMLDeclaration;
class XMLVisitor;

/** The parent class for everything in the Document Object Model.
        (Except for attributes).
        Nodes have siblings, a parent, and children. A node can be
        in a document, or stand on its own. The type of a XMLNode
        can be queried, and it can be cast to its more defined type.
 */
class XMLNode : public XMLBase {
    friend class XMLDocument;
    friend class XMLElement;

public:
    	

    /** An input stream operator, for every class. Tolerant of newlines and
            formatting, but doesn't expect them.
     */
    friend std::istream& operator >>(std::istream& in, XMLNode& base);

    /** An output stream operator, for every class. Note that this outputs
            without any newlines or formatting, as opposed to Print(), which
            includes tabs and new lines.

            The operator<< and operator>> are not completely symmetric. Writing
            a node to a stream is very well defined. You'll get a nice stream
            of output, without any extra whitespace or newlines.
		    
            But reading is not as well defined. (As it always is.) If you create
            a XMLElement (for example) and read that from an input stream,
            the text needs to define an element or junk will result. This is
            true of all input streams, but it's worth keeping in mind.

            A XMLDocument will read nodes until it reads a root element, and
                all the children of that root element.
     */
    friend std::ostream& operator<<(std::ostream& out, const XMLNode& base);

    /// Appends the XML node or attribute to a std::string.
    friend std::string& operator<<(std::string& out, const XMLNode& base);


    /** The types of XML nodes supported by TinyXml. (All the
                    unsupported types are picked up by UNKNOWN.)
     */
    enum NodeType {
        TINYXML_DOCUMENT,
        TINYXML_ELEMENT,
        TINYXML_COMMENT,
        TINYXML_UNKNOWN,
        TINYXML_TEXT,
        TINYXML_DECLARATION,
        TINYXML_TYPECOUNT
    };

    virtual ~XMLNode();

    /** The meaning of 'value' changes for the specific type of
            XMLNode.
            @verbatim
            Document:	filename of the xml file
            Element:	name of the element
            Comment:	the comment text
            Unknown:	the tag contents
            Text:		the text string
            @endverbatim

            The subclasses will wrap this function.
     */
    const char *Value() const {
        return value.c_str();
    }


    /** Return Value() as a std::string. If you only use STL,
        this is more efficient than calling Value().
            Only available in STL mode.
     */
    const std::string& ValueStr() const {
        return value;
    }

    const std::string& ValueTStr() const {
        return value;
    }

    /** Changes the value of the node. Defined as:
            @verbatim
            Document:	filename of the xml file
            Element:	name of the element
            Comment:	the comment text
            Unknown:	the tag contents
            Text:		the text string
            @endverbatim
     */
    void SetValue(const char * _value) {
        value = _value;
    }

    /// STL std::string form.

    void SetValue(const std::string& _value) {
        value = _value;
    }

    /// Delete all the children of this node. Does not affect 'this'.
    void Clear();

    /// One step up the DOM.

    XMLNode* Parent() {
        return parent;
    }

    const XMLNode* Parent() const {
        return parent;
    }

    const XMLNode* FirstChild() const {
        return firstChild;
    } ///< The first child of this node. Will be null if there are no children.

    XMLNode* FirstChild() {
        return firstChild;
    }
    const XMLNode* FirstChild(const char * value) const; ///< The first child of this node with the matching 'value'. Will be null if none found.
    /// The first child of this node with the matching 'value'. Will be null if none found.

    XMLNode* FirstChild(const char * _value) {
        // Call through to the const version - safe since nothing is changed. Exiting syntax: cast this to a const (always safe)
        // call the method, cast the return back to non-const.
        return const_cast<XMLNode*> ((const_cast<const XMLNode*> (this))->FirstChild(_value));
    }

    const XMLNode* LastChild() const {
        return lastChild;
    } /// The last child of this node. Will be null if there are no children.

    XMLNode* LastChild() {
        return lastChild;
    }

    const XMLNode* LastChild(const char * value) const; /// The last child of this node matching 'value'. Will be null if there are no children.

    XMLNode* LastChild(const char * _value) {
        return const_cast<XMLNode*> ((const_cast<const XMLNode*> (this))->LastChild(_value));
    }


    const XMLNode* FirstChild(const std::string& _value) const {
        return FirstChild(_value.c_str());
    } ///< STL std::string form.

    XMLNode* FirstChild(const std::string& _value) {
        return FirstChild(_value.c_str());
    } ///< STL std::string form.

    const XMLNode* LastChild(const std::string& _value) const {
        return LastChild(_value.c_str());
    } ///< STL std::string form.

    XMLNode* LastChild(const std::string& _value) {
        return LastChild(_value.c_str());
    } ///< STL std::string form.

    /** An alternate way to walk the children of a node.
            One way to iterate over nodes is:
            @verbatim
                    for( child = parent->FirstChild(); child; child = child->NextSibling() )
            @endverbatim

            IterateChildren does the same thing with the syntax:
            @verbatim
                    child = 0;
                    while( child = parent->IterateChildren( child ) )
            @endverbatim

            IterateChildren takes the previous child as input and finds
            the next one. If the previous child is null, it returns the
            first. IterateChildren will return null when done.
     */
    const XMLNode* IterateChildren(const XMLNode* previous) const;

    XMLNode* IterateChildren(const XMLNode* previous) {
        return const_cast<XMLNode*> ((const_cast<const XMLNode*> (this))->IterateChildren(previous));
    }

    /// This flavor of IterateChildren searches for children with a particular 'value'
    const XMLNode* IterateChildren(const char * value, const XMLNode* previous) const;

    XMLNode* IterateChildren(const char * _value, const XMLNode* previous) {
        return const_cast<XMLNode*> ((const_cast<const XMLNode*> (this))->IterateChildren(_value, previous));
    }


    const XMLNode* IterateChildren(const std::string& _value, const XMLNode* previous) const {
        return IterateChildren(_value.c_str(), previous);
    } ///< STL std::string form.

    XMLNode* IterateChildren(const std::string& _value, const XMLNode* previous) {
        return IterateChildren(_value.c_str(), previous);
    } ///< STL std::string form.

    /** Add a new node related to this. Adds a child past the LastChild.
            Returns a pointer to the new object or NULL if an error occured.
     */
    XMLNode* InsertEndChild(const XMLNode& addThis);


    /** Add a new node related to this. Adds a child past the LastChild.

            NOTE: the node to be added is passed by pointer, and will be
            henceforth owned (and deleted) by tinyXml. This method is efficient
            and avoids an extra copy, but should be used with care as it
            uses a different memory model than the other insert functions.

            @sa InsertEndChild
     */
    XMLNode* LinkEndChild(XMLNode* addThis);

    /** Add a new node related to this. Adds a child before the specified child.
            Returns a pointer to the new object or NULL if an error occured.
     */
    XMLNode* InsertBeforeChild(XMLNode* beforeThis, const XMLNode& addThis);

    /** Add a new node related to this. Adds a child after the specified child.
            Returns a pointer to the new object or NULL if an error occured.
     */
    XMLNode* InsertAfterChild(XMLNode* afterThis, const XMLNode& addThis);

    /** Replace a child of this node.
            Returns a pointer to the new object or NULL if an error occured.
     */
    XMLNode* ReplaceChild(XMLNode* replaceThis, const XMLNode& withThis);

    /// Delete a child of this node.
    bool RemoveChild(XMLNode* removeThis);

    /// Navigate to a sibling node.

    const XMLNode* PreviousSibling() const {
        return prev;
    }

    XMLNode* PreviousSibling() {
        return prev;
    }

    /// Navigate to a sibling node.
    const XMLNode* PreviousSibling(const char *) const;

    XMLNode* PreviousSibling(const char *_prev) {
        return const_cast<XMLNode*> ((const_cast<const XMLNode*> (this))->PreviousSibling(_prev));
    }


    const XMLNode* PreviousSibling(const std::string& _value) const {
        return PreviousSibling(_value.c_str());
    } ///< STL std::string form.

    XMLNode* PreviousSibling(const std::string& _value) {
        return PreviousSibling(_value.c_str());
    } ///< STL std::string form.

    const XMLNode* NextSibling(const std::string& _value) const {
        return NextSibling(_value.c_str());
    } ///< STL std::string form.

    XMLNode* NextSibling(const std::string& _value) {
        return NextSibling(_value.c_str());
    } ///< STL std::string form.

    /// Navigate to a sibling node.

    const XMLNode* NextSibling() const {
        return next;
    }

    XMLNode* NextSibling() {
        return next;
    }

    /// Navigate to a sibling node with the given 'value'.
    const XMLNode* NextSibling(const char *) const;

    XMLNode* NextSibling(const char* _next) {
        return const_cast<XMLNode*> ((const_cast<const XMLNode*> (this))->NextSibling(_next));
    }

    /** Convenience function to get through elements.
            Calls NextSibling and ToElement. Will skip all non-Element
            nodes. Returns 0 if there is not another element.
     */
    const XMLElement* NextSiblingElement() const;

    XMLElement* NextSiblingElement() {
        return const_cast<XMLElement*> ((const_cast<const XMLNode*> (this))->NextSiblingElement());
    }

    /** Convenience function to get through elements.
            Calls NextSibling and ToElement. Will skip all non-Element
            nodes. Returns 0 if there is not another element.
     */
    const XMLElement* NextSiblingElement(const char *) const;

    XMLElement* NextSiblingElement(const char *_next) {
        return const_cast<XMLElement*> ((const_cast<const XMLNode*> (this))->NextSiblingElement(_next));
    }


    const XMLElement* NextSiblingElement(const std::string& _value) const {
        return NextSiblingElement(_value.c_str());
    } ///< STL std::string form.

    XMLElement* NextSiblingElement(const std::string& _value) {
        return NextSiblingElement(_value.c_str());
    } ///< STL std::string form.

    /// Convenience function to get through elements.
    const XMLElement* FirstChildElement() const;

    XMLElement* FirstChildElement() {
        return const_cast<XMLElement*> ((const_cast<const XMLNode*> (this))->FirstChildElement());
    }

    /// Convenience function to get through elements.
    const XMLElement* FirstChildElement(const char * _value) const;

    XMLElement* FirstChildElement(const char * _value) {
        return const_cast<XMLElement*> ((const_cast<const XMLNode*> (this))->FirstChildElement(_value));
    }

    const XMLElement* FirstChildElement(const std::string& _value) const {
        return FirstChildElement(_value.c_str());
    } ///< STL std::string form.

    XMLElement* FirstChildElement(const std::string& _value) {
        return FirstChildElement(_value.c_str());
    } ///< STL std::string form.

    /** Query the type (as an enumerated value, above) of this node.
            The possible types are: TINYXML_DOCUMENT, TINYXML_ELEMENT, TINYXML_COMMENT,
                                                            TINYXML_UNKNOWN, TINYXML_TEXT, and TINYXML_DECLARATION.
     */
    int Type() const {
        return type;
    }

    /** Return a pointer to the Document this node lives in.
            Returns null if not in a document.
     */
    const XMLDocument* GetDocument() const;

    XMLDocument* GetDocument() {
        return const_cast<XMLDocument*> ((const_cast<const XMLNode*> (this))->GetDocument());
    }

    /// Returns true if this node has no children.

    bool NoChildren() const {
        return !firstChild;
    }

    virtual const XMLDocument* ToDocument() const {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual const XMLElement* ToElement() const {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual const XMLComment* ToComment() const {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual const XMLUnknown* ToUnknown() const {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual const XMLText* ToText() const {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual const XMLDeclaration* ToDeclaration() const {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual XMLDocument* ToDocument() {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual XMLElement* ToElement() {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual XMLComment* ToComment() {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual XMLUnknown* ToUnknown() {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual XMLText* ToText() {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    virtual XMLDeclaration* ToDeclaration() {
        return 0;
    } ///< Cast to a more defined type. Will return null if not of the requested type.

    /** Create an exact duplicate of this node and return it. The memory must be deleted
            by the caller. 
     */
    virtual XMLNode* Clone() const = 0;

    /** Accept a hierchical visit the nodes in the TinyXML DOM. Every node in the 
            XML tree will be conditionally visited and the host will be called back
            via the XMLVisitor interface.

            This is essentially a SAX interface for TinyXML. (Note however it doesn't re-parse
            the XML for the callbacks, so the performance of TinyXML is unchanged by using this
            interface versus any other.)

            The interface has been based on ideas from:

            - http://www.saxproject.org/
            - http://c2.com/cgi/wiki?HierarchicalVisitorPattern 

            Which are both good references for "visiting".

            An example of using Accept():
            @verbatim
            XMLPrinter printer;
            tinyxmlDoc.Accept( &printer );
            const char* xmlcstr = printer.CStr();
            @endverbatim
     */
    virtual bool Accept(XMLVisitor* visitor) const = 0;

protected:
    XMLNode(NodeType _type);

    // Copy to the allocated object. Shared functionality between Clone, Copy constructor,
    // and the assignment operator.
    void CopyTo(XMLNode* target) const;

    // The real work of the input operator.
    virtual void StreamIn(std::istream* in, std::string* tag) = 0;

    // Figure out what is at *p, and parse it. Returns null if it is not an xml node.
    XMLNode* Identify(const char* start, XMLEncoding encoding);

    XMLNode* parent;
    NodeType type;

    XMLNode* firstChild;
    XMLNode* lastChild;

    std::string value;

    XMLNode* prev;
    XMLNode* next;

private:
    XMLNode(const XMLNode&); // not implemented.
    void operator=(const XMLNode& base); // not allowed.
};

}//namespace LBIND

#endif	/* XMLNODE_H */

