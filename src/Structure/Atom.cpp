/* 
 * File:   Atom.cpp
 * Author: xiaohua
 * 
 * Created on May 26, 2009, 7:20 AM
 */

#include <cmath>
#include <sstream>

#include "Atom.h"

#include "Element.h"
#include "Constants.h"
#include "Coor3d.h"
#include "Logger.h"
#include "Fragment.h"
#include "Molecule.h"

//#include <boost/shared_ptr.hpp>

namespace LBIND {

Atom::Atom(): BaseStruct(),
        pElement(0),
        itsFileID(0),
        itsType(0),
        itscharge(0),
        itsHybridization(0),
        pCoords(new Coor3d(0.0)),
        pParent(NULL),
        pSupParent(NULL)
{
}

Atom::Atom(Molecule* supParent): BaseStruct(),
        pElement(0),
        itsFileID(0),
        itsType(0),
        itscharge(0),
        itsHybridization(0),
        pCoords(new Coor3d(0.0)),
        pParent(NULL),
        pSupParent(supParent)
{
}

Atom::Atom(Fragment* parent): BaseStruct(),
        pElement(0),
        itsFileID(0),
        itsType(0),
        itscharge(0),
        itsHybridization(0),
        pCoords(new Coor3d(0.0)),
        pParent(parent),
        pSupParent(NULL)
{
}

Atom::Atom(const Atom& orig) {
}

Atom::~Atom() {
    delete this->pCoords;
}

void Atom::setSymbol(std::string symb){
    this->symbol=symb;
}

std::string Atom::getSymbol(){
    return this->symbol;
}

 void Atom::setElement(Element* pEle)
{
    this->pElement=pEle;
}

Element* Atom::getElement()
{
    return this->pElement;
}

void Atom::setCoords(const double& x, const double& y, const double& z)
{
    this->pCoords->set(x,y,z);
}

Coor3d* Atom::getCoords()
{
    return this->pCoords;
}

double Atom::getX()
{
    return this->pCoords->getX();
}

double Atom::getY()
{
    return this->pCoords->getY();
}

double Atom::getZ()
{
    return this->pCoords->getZ();
}

void Atom::setFileID(const int& fileID)
{
    this->itsFileID=fileID;
}

int Atom::getFileID()
{
    return this->itsFileID;
}

int Atom::getFragID(){
    if(pParent!=NULL){
        return pParent->getID();
    }else{
        return 0;
    }
}

int Atom::getMolID(){
    if(pSupParent!=NULL){
        return pSupParent->getID();
    }else if(pParent!=NULL){
        return pParent->getParent()->getID();
    }else{
        return 0;
    }
}

void Atom::setType(int type){
    this->itsType=type;
}

int Atom::getType(){
    return this->itsType;
}

void Atom::setFormalCharge(int fCharge){
    this->itsFormalCharge=fCharge;
}

int Atom::getFormalCharge(){
    return this->itsFormalCharge;
}

void Atom::setHybridization(int hybrid){
    this->itsHybridization=hybrid;
}

int Atom::getHybridization(){
    return this->itsHybridization;
}

void Atom::setParent(Fragment* parent){
    this->pParent=parent;
    this->pSupParent=NULL; // guarantee Atom has only one pointer to up level.
}

Fragment* Atom::getParent()
{
    return this->pParent;
}

void Atom::setSupParent(Molecule* supParent){
    this->pParent=NULL;
    this->pSupParent=supParent; // guarantee Atom has only one pointer to up level.
}

Molecule* Atom::getSupParent()
{
    return this->pSupParent;
    
}

bool Atom::isBoundto(Atom* pAtom){
    // Use distance square to achieve fast calculation
    if(this->getSymbol()=="X") return false;
    if(pAtom->getSymbol()=="X") return false;
    
    double distance2=pCoords->dist2(pAtom->getCoords());
    if(distance2 < 0.25){
#ifdef USE_LOGGER        
        std::stringstream ss;
        ss << "Atom Clashed ::" << " Atom 1 ID " << this->getID()
                                       << " Atom 1 Name " << this->getName()
                                       << " Atom 2 ID " << pAtom->getID()
                                       << " Atom 2 Name " << pAtom->getName()
                                       << " Distance " << sqrt(distance2) <<std::endl;
        Logger::Instance()->writeToLogFile(ss.str());
#endif
        return false;
    }
    double covalentRadius=pElement->getCovalentRadius()+
                          pAtom->getElement()->getCovalentRadius()+BONDTOLERANCE;

    if(distance2 < covalentRadius*covalentRadius){
        return true;
    }
    return false;

}
}//namespace
