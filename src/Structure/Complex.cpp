/* 
 * File:   Complex.cpp
 * Author: zhang
 * 
 * Created on May 21, 2010, 1:45 PM
 */

#include <sstream>

#include "Complex.h"
#include "Molecule.h"
#include "Coordinates.h"
#include "Atom.h"
#include "ElementContainer.h"

#ifdef USE_LOGGER
#include "Logger.h"
#endif

namespace LBIND{

Complex::Complex() {
}

Complex::Complex(Control* pCtrl): pControl(pCtrl) {
}

Complex::Complex(const Complex& orig) {
}

Complex::~Complex() {
    for(unsigned i=0; i<itsChildren.size(); i++){
        delete itsChildren[i];
    }
    itsChildren.clear();
}

Molecule* Complex::addMolecule()
{
    Molecule* pMolecule=new Molecule(this);
    this->itsChildren.push_back(pMolecule);

    return pMolecule;
}


std::vector<Molecule*> Complex::getChildren()
{
    return this->itsChildren;
}

Control* Complex::getControl()
{
    return this->pControl;
}

void Complex::assignElement(ElementContainer* pElementContainer){
#ifdef USE_LOGGER
    std::stringstream ss;    
    ss << "... assignElement itsChildren.size" << itsChildren.size() << std::endl;
#endif
    for(std::vector<Molecule*>::iterator m=itsChildren.begin();
            m!=itsChildren.end();++m){
        std::vector<Atom*> atomList=(*m)->getGrdChildren();
#ifdef USE_LOGGER
        ss << "... assignElement atomList.size" << atomList.size() << std::endl;
#endif
        for(std::vector<Atom*>::iterator a=atomList.begin();
                a!=atomList.end();++a){
            Element* pElement=pElementContainer->symbolToElement((*a)->getSymbol());
            (*a)->setElement(pElement);
        }
    }
#ifdef USE_LOGGER    
    ss << "ParmContainer ... assignElement end" << std::endl;
    Logger::Instance()->writeToLogFile(ss.str());
#endif
    
}

void Complex::assignCoordinate(std::vector<Coor3d*>& atmCoorList){
    int i=0;
    for(std::vector<Molecule*>::iterator m=itsChildren.begin();
            m!=itsChildren.end();++m){
        std::vector<Atom*> atomList=(*m)->getGrdChildren();
        for(std::vector<Atom*>::iterator a=atomList.begin();
                a!=atomList.end();++a){
            Coor3d* pCoor=atmCoorList[i];
            (*a)->setCoords(pCoor->getX(),pCoor->getY(),pCoor->getZ());
            ++i;
        }
    }
}

void Complex::assignCoordinate(Coordinates* coords){

    std::vector<Coor3d*> atmCoorList;
    coords->getCoordinates(atmCoorList);
    
    this->assignCoordinate(atmCoorList);
}

void Complex::getCoordinate(Coordinates* pCoordinates){
    std::vector<Molecule*> molList=this->getChildren();
    for(unsigned int i=0; i<molList.size(); ++i){
        Molecule* pMolecule=molList[i];
        std::vector<Atom*> atomList=pMolecule->getGrdChildren();
        for(unsigned int j=0; j < atomList.size(); ++j){
            Atom* pAtom=atomList[j];
            pCoordinates->addCoor3d(pAtom->getCoords());
        }
    }    
}

int Complex::getTotNumAtom(){
    int totNumAtom=0;
    for(std::vector<Molecule*>::iterator m=itsChildren.begin();
            m!=itsChildren.end();++m){
        totNumAtom=totNumAtom+(*m)->getTotNumAtom();

    }
    return totNumAtom;
}

}//namespace LBIND


