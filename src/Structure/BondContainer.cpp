/* 
 * File:   BondContainer.cpp
 * Author: zhang
 * 
 * Created on August 23, 2010, 1:20 PM
 */

#include <iostream>
#include <sstream>

#include "BondContainer.h"
#include "Bond.h"
#include "Atom.h"
#include "Logger.h"

namespace LBIND{
BondContainer::BondContainer() {
}

BondContainer::BondContainer(const BondContainer& orig) {
}

BondContainer::~BondContainer() {
    for(unsigned int i=0; i<itsBondList.size(); i++){
        delete itsBondList[i];
    }
    itsBondList.clear();
}

std::vector<Bond*> BondContainer::getBondList(){
    return this->itsBondList;
}

void BondContainer::addBond(Atom* pAtom1, Atom* pAtom2){
    Bond* pBond=new Bond(pAtom1,pAtom2);
    this->itsBondList.push_back(pBond);
}

bool BondContainer::isBonded(Atom* pAtom1, Atom* pAtom2){
//    std::cout << " BondContainer::isBonded itsBondList="  << itsBondList.size() << std::endl;
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        if((*it)->getAtom1()==pAtom1 && (*it)->getAtom2()==pAtom2){
            return true;
        }else if((*it)->getAtom1()==pAtom2 && (*it)->getAtom2()==pAtom1){
            return true;
        }
    }
    return false;
}

Bond* BondContainer::getBond(Atom* pAtom1, Atom* pAtom2){
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        if((*it)->getAtom1()==pAtom1 && (*it)->getAtom2()==pAtom2){
            return (*it);
        }else if((*it)->getAtom1()==pAtom2 && (*it)->getAtom2()==pAtom1){
            return (*it);
        }
    }  
    return 0;
}

int BondContainer::numBondNoH(Atom* pAtom){
    int numBond=0;
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        if((*it)->getAtom1()==pAtom){
            if((*it)->getAtom2()->getElement()->getAtomicNumber()!=1){
                numBond++;
            }
        }else if((*it)->getAtom2()==pAtom){
            if((*it)->getAtom1()->getElement()->getAtomicNumber()!=1){
                numBond++;
            }
        }
    }
    return numBond;
}

int BondContainer::numBond(Atom* pAtom){
    int numBond=0;
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        if((*it)->getAtom1()==pAtom){
                numBond++;
            
        }else if((*it)->getAtom2()==pAtom){
                numBond++;
        }
    }
    return numBond;
}

void BondContainer::getBondAtoms(Atom* pAtom, std::vector<Atom*>& bondAtoms){
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        if((*it)->getAtom1()==pAtom){
            bondAtoms.push_back((*it)->getAtom2());     
        }else if((*it)->getAtom2()==pAtom){
            bondAtoms.push_back((*it)->getAtom1());
        }
    }
}

void BondContainer::printList(){
    std::cout<<"BondContainer::printList" << std::endl;
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        std::cout << (*it)->getAtom1()->getID() << " "
                  << (*it)->getAtom1()->getName() << " <=> "
                  << (*it)->getAtom2()->getID() << " "
                  << (*it)->getAtom2()->getName() << std::endl;
    }
}

void BondContainer::printRotList(){
    std::cout<<"BondContainer::printRotList" << std::endl;
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        bool rot=(*it)->getRotatable();
        if(rot){
            std::cout << (*it)->getAtom1()->getID() << " "
                      << (*it)->getAtom1()->getName() << " <=> "
                      << (*it)->getAtom2()->getID() << " "
                      << (*it)->getAtom2()->getName() << std::endl;
        }
    }
}

void BondContainer::loggingList(){
    std::stringstream ss;
    ss<<"BondContainer::printList" << std::endl;
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        ss << (*it)->getAtom1()->getID() << " "
                  << (*it)->getAtom1()->getName() << " <=> "
                  << (*it)->getAtom2()->getID() << " "
                  << (*it)->getAtom2()->getName() << std::endl;
    }
    Logger::Instance()->writeToLogFile(ss.str());
}

void BondContainer::loggingRotList(){
    std::stringstream ss;
    ss<<"BondContainer::printRotList" << std::endl;
    for(std::vector<Bond*>::iterator it=itsBondList.begin();
            it!=itsBondList.end(); ++it){
        bool rot=(*it)->getRotatable();
        if(rot){
            ss << (*it)->getAtom1()->getID() << " "
                      << (*it)->getAtom1()->getName() << " <=> "
                      << (*it)->getAtom2()->getID() << " "
                      << (*it)->getAtom2()->getName() << std::endl;
        }
    }
    Logger::Instance()->writeToLogFile(ss.str());
}

}//namespace LBIND
