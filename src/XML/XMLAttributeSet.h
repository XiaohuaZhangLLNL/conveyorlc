/* 
 * File:   XMLAttributeSet.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:28 PM
 */

#ifndef XMLATTRIBUTESET_H
#define	XMLATTRIBUTESET_H

#include "XMLAttribute.h"

/*	A class used to manage a group of attributes.
        It is only used internally, both by the ELEMENT and the DECLARATION.
	
        The set can be changed transparent to the Element and Declaration
        classes that use it, but NOT transparent to the Attribute
        which has to implement a next() and previous() method. Which makes
        it a bit problematic and prevents the use of STL.

        This version is implemented with circular lists because:
                - I like circular lists
                - it demonstrates some independence from the (typical) doubly linked list.
 */

namespace LBIND{

class XMLAttributeSet {
public:
    XMLAttributeSet();
    ~XMLAttributeSet();

    void Add(XMLAttribute* attribute);
    void Remove(XMLAttribute* attribute);

    const XMLAttribute* First() const {
        return ( sentinel.next == &sentinel) ? 0 : sentinel.next;
    }

    XMLAttribute* First() {
        return ( sentinel.next == &sentinel) ? 0 : sentinel.next;
    }

    const XMLAttribute* Last() const {
        return ( sentinel.prev == &sentinel) ? 0 : sentinel.prev;
    }

    XMLAttribute* Last() {
        return ( sentinel.prev == &sentinel) ? 0 : sentinel.prev;
    }

    XMLAttribute* Find(const char* _name) const;
    XMLAttribute* FindOrCreate(const char* _name);


    XMLAttribute* Find(const std::string& _name) const;
    XMLAttribute* FindOrCreate(const std::string& _name);


private:
    //*ME:	Because of hidden/disabled copy-construktor in XMLAttribute (sentinel-element),
    //*ME:	this class must be also use a hidden/disabled copy-constructor !!!
    XMLAttributeSet(const XMLAttributeSet&); // not allowed
    void operator=(const XMLAttributeSet&); // not allowed (as XMLAttribute)

    XMLAttribute sentinel;
};

}//namespace LBIND

#endif	/* XMLATTRIBUTESET_H */

