/* 
 * File:   Molecule.cpp
 * Author: xiaohua
 * 
 * Created on May 26, 2009, 7:19 AM
 */

#include "Molecule.h"
#include "Atom.h"
#include "Fragment.h"
#include "BondContainer.h"
#include "RingContainer.h"

namespace LBIND{

Molecule::Molecule() : BaseStruct(),
        itscharge(0),
        pBondContainer(new BondContainer()),
        pRingContainer(new RingContainer(this))
{
}

Molecule::Molecule(Complex* parent) : BaseStruct(),
        itscharge(0),
        pParent(parent),
        pBondContainer(new BondContainer()),
        pRingContainer(new RingContainer(this))
{
}

Molecule::Molecule(const Molecule& orig)
{
}

Molecule::~Molecule()
{
    for(unsigned i=0; i<itsChildren.size(); i++){
        delete itsChildren[i];
    }
    itsChildren.clear();

    for(unsigned i=0; i<itsChildren.size(); i++){
        delete itsGrdChildren[i];
    }
    itsGrdChildren.clear();
}

Complex* Molecule::getParent()
{
    return this->pParent;

}

std::vector<Fragment*> Molecule::getChildren()
{
    return this->itsChildren;
}

std::vector<Atom*> Molecule::getGrdChildren()
{
    return this->itsGrdChildren;
}

Atom* Molecule::addAtom()
{
    Atom* pAtom=new Atom(this);
    this->itsGrdChildren.push_back(pAtom);
    return pAtom;

}

Fragment* Molecule::addFragment()
{
    Fragment* pFragment=new Fragment(this);
    this->itsChildren.push_back(pFragment);
    return pFragment;
}

BondContainer* Molecule::getPBondContainer(){
    return this->pBondContainer;
}

RingContainer* Molecule::getPRingContainer(){
    return this->pRingContainer;
}

int Molecule::getTotNumAtom(){
    return this->itsGrdChildren.size();
}

} //namespace LBIND





