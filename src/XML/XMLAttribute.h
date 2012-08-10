/* 
 * File:   XMLAttribute.h
 * Author: zhang
 *
 * Created on November 30, 2011, 9:11 PM
 */

#ifndef XMLATTRIBUTE_H
#define	XMLATTRIBUTE_H

#include "XMLBase.h"

#include <string>
#include <iostream>
#include <sstream>

namespace LBIND{
    class XMLDocument;

/** An attribute is a name-value pair. Elements have an arbitrary
        number of attributes, each with a unique name.

        @note The attributes are not XMLNodes, since they are not
                  part of the tinyXML document object model. There are other
                  suggested ways to look at this problem.
 */
class XMLAttribute : public XMLBase {
    friend class XMLAttributeSet;

public:
    /// Construct an empty attribute.

    XMLAttribute() : XMLBase() {
        document = 0;
        prev = next = 0;
    }


    /// std::string constructor.

    XMLAttribute(const std::string& _name, const std::string& _value) {
        name = _name;
        value = _value;
        document = 0;
        prev = next = 0;
    }


    /// Construct an attribute with a name and value.

    XMLAttribute(const char * _name, const char * _value) {
        name = _name;
        value = _value;
        document = 0;
        prev = next = 0;
    }

    const char* Name() const {
        return name.c_str();
    } ///< Return the name of this attribute.

    const char* Value() const {
        return value.c_str();
    } ///< Return the value of this attribute.


    const std::string& ValueStr() const {
        return value;
    } ///< Return the value of this attribute.

    int IntValue() const; ///< Return the value of this attribute, converted to an integer.
    double DoubleValue() const; ///< Return the value of this attribute, converted to a double.

    // Get the tinyxml string representation

    const std::string& NameTStr() const {
        return name;
    }

    /** QueryIntValue examines the value string. It is an alternative to the
            IntValue() method with richer error checking.
            If the value is an integer, it is stored in 'value' and 
            the call returns XML_SUCCESS. If it is not
            an integer, it returns XML_WRONG_TYPE.

            A specialized but useful call. Note that for success it returns 0,
            which is the opposite of almost all other TinyXml calls.
     */
    int QueryIntValue(int* _value) const;
    /// QueryDoubleValue examines the value string. See QueryIntValue().
    int QueryDoubleValue(double* _value) const;

    void SetName(const char* _name) {
        name = _name;
    } ///< Set the name of this attribute.

    void SetValue(const char* _value) {
        value = _value;
    } ///< Set the value.

    void SetIntValue(int _value); ///< Set the value from an integer.
    void SetDoubleValue(double _value); ///< Set the value from a double.


    /// STL std::string form.

    void SetName(const std::string& _name) {
        name = _name;
    }
    /// STL std::string form.	

    void SetValue(const std::string& _value) {
        value = _value;
    }


    /// Get the next sibling attribute in the DOM. Returns null at end.
    const XMLAttribute* Next() const;

    XMLAttribute* Next() {
        return const_cast<XMLAttribute*> ((const_cast<const XMLAttribute*> (this))->Next());
    }

    /// Get the previous sibling attribute in the DOM. Returns null at beginning.
    const XMLAttribute* Previous() const;

    XMLAttribute* Previous() {
        return const_cast<XMLAttribute*> ((const_cast<const XMLAttribute*> (this))->Previous());
    }

    bool operator==(const XMLAttribute& rhs) const {
        return rhs.name == name;
    }

    bool operator<(const XMLAttribute& rhs) const {
        return name < rhs.name;
    }

    bool operator>(const XMLAttribute& rhs) const {
        return name > rhs.name;
    }

    /*	Attribute parsing starts: first letter of the name
                                             returns: the next char after the value end quote
     */
    virtual const char* Parse(const char* p, XMLParsingData* data, XMLEncoding encoding);

    // Prints this Attribute to a FILE stream.

    virtual void Print(FILE* cfile, int depth) const {
        Print(cfile, depth, 0);
    }
    void Print(FILE* cfile, int depth, std::string* str) const;

    // [internal use]
    // Set the document pointer so the attribute can report errors.

    void SetDocument(XMLDocument* doc) {
        document = doc;
    }

private:
    XMLAttribute(const XMLAttribute&); // not implemented.
    void operator=(const XMLAttribute& base); // not allowed.

    XMLDocument* document; // A pointer back to a document, for error reporting.
    std::string name;
    std::string value;
    XMLAttribute* prev;
    XMLAttribute* next;
};

}//namespace LBIND

#endif	/* XMLATTRIBUTE_H */

