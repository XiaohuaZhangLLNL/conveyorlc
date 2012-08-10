/* 
 * File:   XMLHandle.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:56 PM
 */

#ifndef XMLHANDLE_H
#define	XMLHANDLE_H

#include <string>
#include "XMLNode.h"
#include "XMLElement.h"

/**
        A XMLHandle is a class that wraps a node pointer with null checks; this is
        an incredibly useful thing. Note that XMLHandle is not part of the TinyXml
        DOM structure. It is a separate utility class.

        Take an example:
        @verbatim
        <Document>
                <Element attributeA = "valueA">
                        <Child attributeB = "value1" />
                        <Child attributeB = "value2" />
                </Element>
        <Document>
        @endverbatim

        Assuming you want the value of "attributeB" in the 2nd "Child" element, it's very 
        easy to write a *lot* of code that looks like:

        @verbatim
        XMLElement* root = document.FirstChildElement( "Document" );
        if ( root )
        {
                XMLElement* element = root->FirstChildElement( "Element" );
                if ( element )
                {
                        XMLElement* child = element->FirstChildElement( "Child" );
                        if ( child )
                        {
                                XMLElement* child2 = child->NextSiblingElement( "Child" );
                                if ( child2 )
                                {
                                        // Finally do something useful.
        @endverbatim

        And that doesn't even cover "else" cases. XMLHandle addresses the verbosity
        of such code. A XMLHandle checks for null	pointers so it is perfectly safe 
        and correct to use:

        @verbatim
        XMLHandle docHandle( &document );
        XMLElement* child2 = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", 1 ).ToElement();
        if ( child2 )
        {
                // do something useful
        @endverbatim

        Which is MUCH more concise and useful.

        It is also safe to copy handles - internally they are nothing more than node pointers.
        @verbatim
        XMLHandle handleCopy = handle;
        @endverbatim

        What they should not be used for is iteration:

        @verbatim
        int i=0; 
        while ( true )
        {
                XMLElement* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", i ).ToElement();
                if ( !child )
                        break;
                // do something
                ++i;
        }
        @endverbatim

        It seems reasonable, but it is in fact two embedded while loops. The Child method is 
        a linear walk to find the element, so this code would iterate much more than it needs 
        to. Instead, prefer:

        @verbatim
        XMLElement* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).FirstChild( "Child" ).ToElement();

        for( child; child; child=child->NextSiblingElement() )
        {
                // do something
        }
        @endverbatim
 */

namespace LBIND{
    
class XMLHandle {
public:
    /// Create a handle from any node (at any depth of the tree.) This can be a null pointer.

    XMLHandle(XMLNode* _node) {
        this->node = _node;
    }
    /// Copy constructor

    XMLHandle(const XMLHandle& ref) {
        this->node = ref.node;
    }

    XMLHandle operator=(const XMLHandle& ref) {
        if (&ref != this) this->node = ref.node;
        return *this;
    }

    /// Return a handle to the first child node.
    XMLHandle FirstChild() const;
    /// Return a handle to the first child node with the given name.
    XMLHandle FirstChild(const char * value) const;
    /// Return a handle to the first child element.
    XMLHandle FirstChildElement() const;
    /// Return a handle to the first child element with the given name.
    XMLHandle FirstChildElement(const char * value) const;

    /** Return a handle to the "index" child with the given name. 
            The first child is 0, the second 1, etc.
     */
    XMLHandle Child(const char* value, int index) const;
    /** Return a handle to the "index" child. 
            The first child is 0, the second 1, etc.
     */
    XMLHandle Child(int index) const;
    /** Return a handle to the "index" child element with the given name. 
            The first child element is 0, the second 1, etc. Note that only XMLElements
            are indexed: other types are not counted.
     */
    XMLHandle ChildElement(const char* value, int index) const;
    /** Return a handle to the "index" child element. 
            The first child element is 0, the second 1, etc. Note that only XMLElements
            are indexed: other types are not counted.
     */
    XMLHandle ChildElement(int index) const;



    XMLHandle FirstChild(const std::string& _value) const {
        return FirstChild(_value.c_str());
    }

    XMLHandle FirstChildElement(const std::string& _value) const {
        return FirstChildElement(_value.c_str());
    }

    XMLHandle Child(const std::string& _value, int index) const {
        return Child(_value.c_str(), index);
    }

    XMLHandle ChildElement(const std::string& _value, int index) const {
        return ChildElement(_value.c_str(), index);
    }


    /** Return the handle as a XMLNode. This may return null.
     */
    XMLNode* ToNode() const {
        return node;
    }

    /** Return the handle as a XMLElement. This may return null.
     */
    XMLElement* ToElement() const {
        return ( (node && node->ToElement()) ? node->ToElement() : 0);
    }

    /**	Return the handle as a XMLText. This may return null.
     */
    XMLText* ToText() const {
        return ( (node && node->ToText()) ? node->ToText() : 0);
    }

    /** Return the handle as a XMLUnknown. This may return null.
     */
    XMLUnknown* ToUnknown() const {
        return ( (node && node->ToUnknown()) ? node->ToUnknown() : 0);
    }

    /** @deprecated use ToNode. 
            Return the handle as a XMLNode. This may return null.
     */
    XMLNode* Node() const {
        return ToNode();
    }

    /** @deprecated use ToElement. 
            Return the handle as a XMLElement. This may return null.
     */
    XMLElement* Element() const {
        return ToElement();
    }

    /**	@deprecated use ToText()
            Return the handle as a XMLText. This may return null.
     */
    XMLText* Text() const {
        return ToText();
    }

    /** @deprecated use ToUnknown()
            Return the handle as a XMLUnknown. This may return null.
     */
    XMLUnknown* Unknown() const {
        return ToUnknown();
    }

private:
    XMLNode* node;
};

}//namespace LBIND

#endif	/* XMLHANDLE_H */

