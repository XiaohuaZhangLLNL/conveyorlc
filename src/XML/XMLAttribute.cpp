/* 
 * File:   XMLAttribute.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:11 PM
 */

#include "XMLAttribute.h"
#include "XMLUtils.hpp"
#include "XMLParsingData.h"
#include "XMLDocument.h"

namespace LBIND{

const XMLAttribute* XMLAttribute::Next() const {
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (next->value.empty() && next->name.empty())
        return 0;
    return next;
}

/*
XMLAttribute* XMLAttribute::Next()
{
        // We are using knowledge of the sentinel. The sentinel
        // have a value or name.
        if ( next->value.empty() && next->name.empty() )
                return 0;
        return next;
}
 */

const XMLAttribute* XMLAttribute::Previous() const {
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (prev->value.empty() && prev->name.empty())
        return 0;
    return prev;
}

/*
XMLAttribute* XMLAttribute::Previous()
{
        // We are using knowledge of the sentinel. The sentinel
        // have a value or name.
        if ( prev->value.empty() && prev->name.empty() )
                return 0;
        return prev;
}
 */

void XMLAttribute::Print(FILE* cfile, int /*depth*/, std::string* str) const {
    std::string n, v;

    EncodeString(name, &n);
    EncodeString(value, &v);

    if (value.find('\"') == std::string::npos) {
        if (cfile) {
            fprintf(cfile, "%s=\"%s\"", n.c_str(), v.c_str());
        }
        if (str) {
            (*str) += n;
            (*str) += "=\"";
            (*str) += v;
            (*str) += "\"";
        }
    } else {
        if (cfile) {
            fprintf(cfile, "%s='%s'", n.c_str(), v.c_str());
        }
        if (str) {
            (*str) += n;
            (*str) += "='";
            (*str) += v;
            (*str) += "'";
        }
    }
}

int XMLAttribute::QueryIntValue(int* ival) const {
    if (sscanf(value.c_str(), "%d", ival) == 1)
        return XML_SUCCESS;
    return XML_WRONG_TYPE;
}

int XMLAttribute::QueryDoubleValue(double* dval) const {
    if (sscanf(value.c_str(), "%lf", dval) == 1)
        return XML_SUCCESS;
    return XML_WRONG_TYPE;
}

void XMLAttribute::SetIntValue(int _value) {
    char buf [64];

    sprintf(buf, "%d", _value);

    SetValue(buf);
}

void XMLAttribute::SetDoubleValue(double _value) {
    char buf [256];

    sprintf(buf, "%g", _value);

    SetValue(buf);
}

int XMLAttribute::IntValue() const {
    return atoi(value.c_str());
}

double XMLAttribute::DoubleValue() const {
    return atof(value.c_str());
}

const char* XMLAttribute::Parse(const char* p, XMLParsingData* data, XMLEncoding encoding) {
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p) return 0;

    if (data) {
        data->Stamp(p, encoding);
        location = data->Cursor();
    }
    // Read the name, the '=' and the value.
    const char* pErr = p;
    p = ReadName(p, &name, encoding);
    if (!p || !*p) {
        if (document) document->SetError(XML_ERROR_READING_ATTRIBUTES, pErr, data, encoding);
        return 0;
    }
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p || *p != '=') {
        if (document) document->SetError(XML_ERROR_READING_ATTRIBUTES, p, data, encoding);
        return 0;
    }

    ++p; // skip '='
    p = SkipWhiteSpace(p, encoding);
    if (!p || !*p) {
        if (document) document->SetError(XML_ERROR_READING_ATTRIBUTES, p, data, encoding);
        return 0;
    }

    const char* end;
    const char SINGLE_QUOTE = '\'';
    const char DOUBLE_QUOTE = '\"';

    if (*p == SINGLE_QUOTE) {
        ++p;
        end = "\'"; // single quote in string
        p = ReadText(p, &value, false, end, false, encoding);
    } else if (*p == DOUBLE_QUOTE) {
        ++p;
        end = "\""; // double quote in string
        p = ReadText(p, &value, false, end, false, encoding);
    } else {
        // All attribute values should be in single or double quotes.
        // But this is such a common error that the parser will try
        // its best, even without them.
        value = "";
        while (p && *p // existence
                && !IsWhiteSpace(*p) // whitespace
                && *p != '/' && *p != '>') // tag end
        {
            if (*p == SINGLE_QUOTE || *p == DOUBLE_QUOTE) {
                // [ 1451649 ] Attribute values with trailing quotes not handled correctly
                // We did not have an opening quote but seem to have a 
                // closing one. Give up and throw an error.
                if (document) document->SetError(XML_ERROR_READING_ATTRIBUTES, p, data, encoding);
                return 0;
            }
            value += *p;
            ++p;
        }
    }
    return p;
}

}//namespace LBIND

