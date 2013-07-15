/* 
 * File:   XMLAttributeSet.cpp
 * Author: zhang
 * 
 * Created on November 30, 2011, 9:28 PM
 */

#include <cassert>
#include <cstring>
#include "XMLAttributeSet.h"

namespace LBIND{

XMLAttributeSet::XMLAttributeSet() {
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
}

XMLAttributeSet::~XMLAttributeSet() {
    assert(sentinel.next == &sentinel);
    assert(sentinel.prev == &sentinel);
}

void XMLAttributeSet::Add(XMLAttribute* addMe) {

    assert(!Find(std::string(addMe->Name()))); // Shouldn't be multiply adding to the set.

    addMe->next = &sentinel;
    addMe->prev = sentinel.prev;

    sentinel.prev->next = addMe;
    sentinel.prev = addMe;
}

void XMLAttributeSet::Remove(XMLAttribute* removeMe) {
    XMLAttribute* node;

    for (node = sentinel.next; node != &sentinel; node = node->next) {
        if (node == removeMe) {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            node->next = 0;
            node->prev = 0;
            return;
        }
    }
    assert(0); // we tried to remove a non-linked attribute.
}



XMLAttribute* XMLAttributeSet::Find(const std::string& name) const {
    for (XMLAttribute* node = sentinel.next; node != &sentinel; node = node->next) {
        if (node->name == name)
            return node;
    }
    return 0;
}

XMLAttribute* XMLAttributeSet::FindOrCreate(const std::string& _name) {
    XMLAttribute* attrib = Find(_name);
    if (!attrib) {
        attrib = new XMLAttribute();
        Add(attrib);
        attrib->SetName(_name);
    }
    return attrib;
}


XMLAttribute* XMLAttributeSet::Find(const char* name) const {
    for (XMLAttribute* node = sentinel.next; node != &sentinel; node = node->next) {
        if (strcmp(node->name.c_str(), name) == 0)
            return node;
    }
    return 0;
}

XMLAttribute* XMLAttributeSet::FindOrCreate(const char* _name) {
    XMLAttribute* attrib = Find(_name);
    if (!attrib) {
        attrib = new XMLAttribute();
        Add(attrib);
        attrib->SetName(_name);
    }
    return attrib;
}

}//namespace LBIND

