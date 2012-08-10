/* 
 * File:   XMLHandle.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:56 PM
 */

#include "XMLHandle.h"

namespace LBIND{

XMLHandle XMLHandle::FirstChild() const {
    if (node) {
        XMLNode* child = node->FirstChild();
        if (child)
            return XMLHandle(child);
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::FirstChild(const char * value) const {
    if (node) {
        XMLNode* child = node->FirstChild(value);
        if (child)
            return XMLHandle(child);
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::FirstChildElement() const {
    if (node) {
        XMLElement* child = node->FirstChildElement();
        if (child)
            return XMLHandle(child);
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::FirstChildElement(const char * value) const {
    if (node) {
        XMLElement* child = node->FirstChildElement(value);
        if (child)
            return XMLHandle(child);
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::Child(int count) const {
    if (node) {
        int i;
        XMLNode* child = node->FirstChild();
        for (i = 0;
                child && i < count;
                child = child->NextSibling(), ++i) {
            // nothing
        }
        if (child)
            return XMLHandle(child);
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::Child(const char* value, int count) const {
    if (node) {
        int i;
        XMLNode* child = node->FirstChild(value);
        for (i = 0;
                child && i < count;
                child = child->NextSibling(value), ++i) {
            // nothing
        }
        if (child)
            return XMLHandle(child);
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::ChildElement(int count) const {
    if (node) {
        int i;
        XMLElement* child = node->FirstChildElement();
        for (i = 0;
                child && i < count;
                child = child->NextSiblingElement(), ++i) {
            // nothing
        }
        if (child)
            return XMLHandle(child);
    }
    return XMLHandle(0);
}

XMLHandle XMLHandle::ChildElement(const char* value, int count) const {
    if (node) {
        int i;
        XMLElement* child = node->FirstChildElement(value);
        for (i = 0;
                child && i < count;
                child = child->NextSiblingElement(value), ++i) {
            // nothing
        }
        if (child)
            return XMLHandle(child);
    }
    return XMLHandle(0);
}

}//namespace LBIND
