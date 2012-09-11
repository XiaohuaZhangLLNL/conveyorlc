/* 
 * File:   Fragment.cpp
 * Author: zhang
 * 
 * Created on May 21, 2010, 1:45 PM
 */

#include "Fragment.h"
#include "Atom.h"

#include <vector>

namespace LBIND{

Fragment::Fragment() : BaseStruct() {
}

Fragment::Fragment(Molecule* parent): BaseStruct(),
        pParent(parent){
}

Fragment::Fragment(const Fragment& orig) {
}

Fragment::~Fragment() {
    for(unsigned i=0; i<itsChildren.size(); i++){
        delete itsChildren[i];
    }
    itsChildren.clear();
}

std::vector<Atom*> Fragment::getChildren(){
    return this->itsChildren;
}

void Fragment::setChildren(std::vector<Atom*>& atomList){
    this->itsChildren=atomList;
}

Molecule* Fragment::getParent(){
    return this->pParent;
}

Atom* Fragment::addAtom()
{
    Atom* pAtom = new Atom(this);
    this->itsChildren.push_back(pAtom);
    return pAtom;
}

} //namespace LBIND
