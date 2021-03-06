/* 
 * File:   Complex.cpp
 * Author: zhang
 * 
 * Created on May 21, 2010, 1:45 PM
 */

#include <sstream>

#include "Complex.h"
#include "Molecule.h"
#include "Fragment.h"
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

void Complex::addMolecule(Molecule* pMolecule)
{

    this->itsChildren.push_back(pMolecule);

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
    
    for(unsigned i=0; i< itsChildren.size(); ++i){
        std::vector<Fragment*> resList=itsChildren[i]->getChildren();
        for(unsigned j=0; j<resList.size(); ++j){
            std::vector<Atom*> atomList=resList[j]->getChildren();
            for(unsigned k=0; k<atomList.size(); ++k){
                Atom *pAtom=atomList[k];
//                std::cout << "Residue=" << resList[j]->getName() << " ResID=" << resList[j]->getID() 
//                        << " Atom=" << pAtom->getName() << " file id=" << pAtom->getFileID() << " Symbol=" << pAtom->getSymbol()
//                        << std::endl;
                Element *pElement=pElementContainer->symbolToElement(pAtom->getSymbol());
                pAtom->setElement(pElement);
            }
        }
        
    }  
    
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

std::vector<Atom*> Complex::getAtomList(){
    std::vector<Atom*> retAtomList;
    for(unsigned i=0; i< itsChildren.size(); ++i){
        std::vector<Fragment*> resList=itsChildren[i]->getChildren();
        for(unsigned j=0; j<resList.size(); ++j){
            std::vector<Atom*> atomList=resList[j]->getChildren();
            for(unsigned k=0; k<atomList.size(); ++k){
                Atom *pAtom=atomList[k];
                retAtomList.push_back(pAtom);
            }
        }
        
    }  
   // std::cout << "Atom number in Complex: " << retAtomList.size() << std::endl;
    return retAtomList;
}

}//namespace LBIND


