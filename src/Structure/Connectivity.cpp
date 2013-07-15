/* 
 * File:   Connectivity.cpp
 * Author: zhang
 * 
 * Created on May 27, 2010, 9:17 AM
 */

#include "BondContainer.h"


#include "Connectivity.h"
#include "Molecule.h"
#include "Complex.h"
#include "Atom.h"
#include "Coor3d.h"
#include "Logger.h"
#include "Sstrm.hpp"
#include "Common/Control.h"
#include "Element.h"
#include "Bond.h"
#include "RingContainer.h"

#include <vector>
#include <sstream>

namespace LBIND{

Connectivity::Connectivity() {
}

Connectivity::Connectivity(Complex* pCom) : pComplex(pCom) {
}

Connectivity::Connectivity(const Connectivity& orig) {
}

Connectivity::~Connectivity() {
}

void Connectivity::run(){
    Logger::Instance()->writeToLogFile("Connectivity assignBond");
    assignBond();
    Logger::Instance()->writeToLogFile( "Connectivity determineRings" );
    determineRings();
    Logger::Instance()->writeToLogFile( "Connectivity determineRotate" );
    determineRotate();
}

void Connectivity::assignBond(){
//    std::cout << "Connectivity::assignBond" << std::endl;
    std::vector<Molecule*> molList=pComplex->getChildren();
    for(unsigned int i=0; i<molList.size(); ++i){
//        std::cout << "Mol[i]=" << i << std::endl;
        this->assignBond(molList[i]);
    }
}

void Connectivity::assignBond(Molecule* pMol){
    std::vector<Atom*> atomList=pMol->getGrdChildren();
    for(unsigned int i=0; i <atomList.size()-1; ++i){
        for(unsigned int j=i+1; j <atomList.size(); ++j){
            if( isBonded(atomList[i],atomList[j]) ){
//                std::cout << "Atom[i]=" << i << "-" <<"Atom[j]=" << j << std::endl;
                pMol->getPBondContainer()->addBond(atomList[i],atomList[j]);
            }
        }
    }

}

bool Connectivity::isBonded(Atom* pAtom1, Atom* pAtom2){
    return (pAtom1->isBoundto(pAtom2));
}

void Connectivity::determineRotate(){
    std::vector<Molecule*> molList=pComplex->getChildren();
    for(unsigned int i=0; i<molList.size(); ++i){
        this->determineRotateBond(molList[i]);
        this->determineRotateAngle(molList[i]);
    }
}

void Connectivity::determineRotateBond(Molecule* pMol){
    BondContainer* pBondContainer=pMol->getPBondContainer();
    std::vector<Bond*> bondList=pBondContainer->getBondList();
//    for(std::vector<Bond*>::iterator i=bondList.begin(); i!=bondList.end(); ++i){
//            (*i)->setRotatable(false);
//    }
    Logger::Instance()->writeToLogFile("Connectivity::determineRotateBond -------------");
    for(std::vector<Bond*>::iterator b=bondList.begin(); b!=bondList.end(); ++b){
        Bond* pBond=*b;
        if(pBond->getTopology()!=1){ 
            Atom* pAtom1=pBond->getAtom1();       
            Atom* pAtom2=pBond->getAtom2();
            if(pBondContainer->numBondNoH(pAtom1)> 1) {
                if(pBondContainer->numBondNoH(pAtom2)> 1){
                    int atom1AtomicNumber=pAtom1->getElement()->getAtomicNumber();
                    int atom2AtomicNumber=pAtom2->getElement()->getAtomicNumber();
                    double distance2=pAtom1->getCoords()->dist2(pAtom2->getCoords());
                    // C-C
                    if(atom1AtomicNumber==6 && atom2AtomicNumber==6){
                        if(distance2 > 2.0164 /* 1.42* 1.42*/){ // 1.527 1.325 1.189
                            pBond->setRotatable(true);
                        }

                    }
                    // C-N
                    if((atom1AtomicNumber==6 && atom2AtomicNumber==7) ||
                            (atom1AtomicNumber==7 && atom2AtomicNumber==6)){
                        if(distance2 > 1.8769 /* 1.37* 1.37*/){ //1.465 1.280
                            pBond->setRotatable(true);
                        }
                    }
                    // C-O
                    if((atom1AtomicNumber==6 && atom2AtomicNumber==8) ||
                            (atom1AtomicNumber==8 && atom2AtomicNumber==6)){
                        pBond->setRotatable(true);
                    }
                    // C-S
                    if((atom1AtomicNumber==6 && atom2AtomicNumber==16) ||
                            (atom1AtomicNumber==16 && atom2AtomicNumber==6)){
                        pBond->setRotatable(true);
                    }
                    // C-P
                    if((atom1AtomicNumber==6 && atom2AtomicNumber==15) ||
                            (atom1AtomicNumber==15 && atom2AtomicNumber==6)){
                        pBond->setRotatable(true);
                    }
    //                        determineRotAngle(pBond);
    //                        std::cout << "Rotable: "
    //                                  << pAtom1->getID() << " "
    //                                  << pAtom1->getName() << " <=> "
    //                                  << pAtom2->getID() << " "
    //                                  << pAtom2->getName() << std::endl;
                }
            }
        }
    }
}

void Connectivity::determineRotateAngle(Molecule* pMol){
    Control* pControl=pComplex->getControl();
    const double angleCrit=pControl->getAngleCrit();
    double a0=0;
    double a60=60;
    double a120=120;
    double a180=180;

    if(angleCrit != 0){
        a60=a60/angleCrit;
        a120=a120/angleCrit;
        a180=a180/angleCrit;
    } else {
        Logger::Instance()->writeToLogFile( "Angle Critique cannot be zero. Use Default angles");
    }
    std::stringstream ss;
    ss <<"Connectivity::determineRotateAngle " << "angleCrit=" << angleCrit << " a120=" << a120 << std::endl;
    
    BondContainer* pBondContainer=pMol->getPBondContainer();
    std::vector<Bond*> bondList=pBondContainer->getBondList();
    ss << "Connectivity::determineRotateAngle -------------" << std::endl;
    
    Logger::Instance()->writeToLogFile(ss.str());
    
    for(std::vector<Bond*>::iterator b=bondList.begin(); b!=bondList.end(); ++b){
        Bond* pBond=*b;
        if(pBond->getTopology()!=1){           
            Atom* pAtom1=pBond->getAtom1();
            Atom* pAtom2=pBond->getAtom2();
            int atom1AtomicNumber=pAtom1->getElement()->getAtomicNumber();
            int atom2AtomicNumber=pAtom2->getElement()->getAtomicNumber();
            int numBondAtom1=pBondContainer->numBond(pAtom1);
            int numBondAtom2=pBondContainer->numBond(pAtom2);
            if(numBondAtom1==4 && numBondAtom2==4){
                pBond->setRotAngle(a120);
            }else if((numBondAtom1==4 && numBondAtom2==3)||
                    (numBondAtom1==3 && numBondAtom2==4)){
                int atomAtomicNumber=0;
                if(numBondAtom1==3){
                    atomAtomicNumber= atom1AtomicNumber;
                }else{
                    atomAtomicNumber= atom2AtomicNumber;
                }
                if(atomAtomicNumber==6){
                    pBond->setRotAngle(a120);
                }else if(atomAtomicNumber==7){
                    pBond->setRotAngle(a120);
                }else{
                    pBond->setRotAngle(a120);
                }
            }else if((numBondAtom1==4 && numBondAtom2==2)||
                    (numBondAtom1==2 && numBondAtom2==4)){
                int atomAtomicNumber=0;
                if(numBondAtom1==2){
                    atomAtomicNumber= atom1AtomicNumber;
                }else{
                    atomAtomicNumber= atom2AtomicNumber;
                }
                if(atomAtomicNumber==6){
                    pBond->setRotAngle(a60);
                }else if(atomAtomicNumber==7 || atomAtomicNumber==8){
                    pBond->setRotAngle(a120);
                }else{
                    pBond->setRotAngle(a120);
                }
            }else if(numBondAtom1==3 && numBondAtom2==3){
                if(atom1AtomicNumber==6 && atom1AtomicNumber==6){
                    pBond->setRotAngle(a180);
                }else if((atom1AtomicNumber==6 && atom1AtomicNumber==7)||
                        (atom1AtomicNumber==7 && atom1AtomicNumber==6)){
                    pBond->setRotAngle(a120);
                }else{
                    pBond->setRotAngle(a120);
                }
            }else if((numBondAtom1==3 && numBondAtom2==2)||
                    (numBondAtom1==2 && numBondAtom2==3)){
                int atomAtomicNumber=0;
                if(numBondAtom1==2){
                    atomAtomicNumber= atom1AtomicNumber;
                }else{
                    atomAtomicNumber= atom2AtomicNumber;
                }
                if(atomAtomicNumber==6){
                    pBond->setRotAngle(a120);
                }else if(atomAtomicNumber==7 || atomAtomicNumber==8){
                    pBond->setRotAngle(a180);
                }else{
                    pBond->setRotAngle(a120);
                }
            }else if(numBondAtom1==2 && numBondAtom2==2){
                pBond->setRotAngle(a0); //Don't need to be rotate.
                pBond->setRotatable(false);
            }
        }
    }
}

void Connectivity::determineRings(){ 
    Logger::Instance()->writeToLogFile( "Connectivity::determineRings()" );
    std::vector<Molecule*> molList=pComplex->getChildren();
    for(unsigned int i=0; i<molList.size(); ++i){
        this->determineRings(molList[i]);
    }
}

void Connectivity::determineRings(Molecule* pMol){
    RingContainer* pRingContainer=pMol->getPRingContainer();
    std::vector<Atom*> atomList = pMol->getGrdChildren();
    Logger::Instance()->writeToLogFile( "Connectivity::determineRings(Molecule* pMol) " 
                       + Sstrm<std::string, double>(atomList.size()) );
    pRingContainer->determine();
    pRingContainer->kekulize();
}

}//namespace LBIND
