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
#include "Coor3d.h"

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

    for(unsigned i=0; i<itsGrdChildren.size(); i++){
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

void Molecule::setChildren(std::vector<Fragment*>& fragList){
    this->itsChildren=fragList;
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

void Molecule::addFragment(Fragment* pFragment)
{
    this->itsChildren.push_back(pFragment);

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


void Molecule::setCharge(double charge){
    this->itscharge=charge;
}

double Molecule::getCharge(){
    return this->itscharge;
}

void Molecule::center(Coor3d& coor){
    double xSum=0;
    double ySum=0;
    double zSum=0;
    for(unsigned i=0; i<itsGrdChildren.size(); i++){
        Coor3d* pACoor=itsGrdChildren[i]->getCoords();
//        std::cout << "pCoor "<< pACoor->getX() << " " <<  pACoor->getY() << " " <<  pACoor->getZ() << std::endl;
        xSum=xSum+pACoor->getX();
        ySum=ySum+pACoor->getY();
        zSum=zSum+pACoor->getZ();
    } 
    xSum/=itsGrdChildren.size();
    ySum/=itsGrdChildren.size();
    zSum/=itsGrdChildren.size();
    coor.set(xSum,ySum,zSum);
}

bool Molecule::boundBox(Coor3d& center, Coor3d& dim){

    if(itsGrdChildren.size()>0) {
        double xSum=0;
        double ySum=0;
        double zSum=0;

        Coor3d *pFirstCoor = itsGrdChildren[0]->getCoords();
        double xmin=pFirstCoor->getX();
        double xmax=xmin;
        double ymin=pFirstCoor->getY();
        double ymax=ymin;
        double zmin=pFirstCoor->getZ();
        double zmax=zmin;

        for (unsigned i = 0; i < itsGrdChildren.size(); i++) {
            Coor3d *pACoor = itsGrdChildren[i]->getCoords();
//        std::cout << "pCoor "<< pACoor->getX() << " " <<  pACoor->getY() << " " <<  pACoor->getZ() << std::endl;
            double x=pACoor->getX();
            double y=pACoor->getY();
            double z=pACoor->getZ();
            xSum = xSum + z;
            ySum = ySum + y;
            zSum = zSum + z;

            if(xmin>x) xmin=x;
            if(xmax<x) xmax=x;
            if(ymin>y) ymin=y;
            if(ymax<y) ymax=y;
            if(zmin>z) zmin=z;
            if(zmax<z) zmax=z;
        }
        xSum /= itsGrdChildren.size();
        ySum /= itsGrdChildren.size();
        zSum /= itsGrdChildren.size();

        center.set(xSum,ySum,zSum);
        dim.set((xmax-xmin), (ymax-ymin), (zmax-zmin));
        return true;
    }

    return false;

}

} //namespace LBIND
