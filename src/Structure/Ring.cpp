/* 
 * File:   Ring.cpp
 * Author: zhang
 * 
 * Created on May 27, 2010, 9:09 AM
 */

#include <vector>
#include <sstream>

#include "Ring.h"
#include "Atom.h"
#include "Logger.h"

namespace LBIND{

Ring::Ring() : itsID(0),
        size(0),
        planar(false),
        aromatic(false),
        hetero(false),
        nHetero(0),
        nNitrogen(0),
        nOxygen(0),
        nSulfur(0)
{
}

Ring::Ring(const Ring& orig) {
}

Ring::~Ring() {
}

void Ring::setID(int id){
    this->itsID=id;
}

int Ring::getID(){
    return this->itsID;
}

void Ring::addAtom(Atom* pAtom){
    this->itsAtoms.push_back(pAtom);
}

std::vector<Atom*> Ring::getAtomList(){
    return this->itsAtoms;
}

void Ring::setSize(int s){
    this->size=s;
}
int Ring::getSize(){
    return this->size;
}

void Ring::setPlanar(bool pl){
    this->planar=pl;
}
bool Ring::getPlanar(){
    return this->planar;
}

void Ring::setAromatic(bool ar){
    this->aromatic=ar;
}
bool Ring::getAromatic(){
    return this->aromatic;
}

void Ring::setHetero(bool he){
    this->hetero=he;
}
bool Ring::getHetero(){
    return this->hetero;
}

void Ring::setNHetero(int nh){
    this->nHetero=nh;
}
int Ring::getNHetero(){
    return this->nHetero;
}

void Ring::setNNitrogen(int nn){
    this->nNitrogen=nn;
}
int Ring::getNNitrogen(){
    return this->nNitrogen;
}

void Ring::setNOxygen(int no){
    this->nOxygen=no;
}
int Ring::getNOxygen(){
    return this->nOxygen;
}

void Ring::setNSulfur(int ns){
    this->nSulfur=ns;
}
int Ring::getNSulfur(){
    return this->nSulfur;
}

void Ring::printRing(){
    std::cout << "Ring[";
    for(unsigned int i=0; i< itsAtoms.size(); ++i){
        std::cout << itsAtoms[i]->getFileID() << " ";
    }   
    std::cout << "]" << std::endl;
}

void Ring::loggingRing(){
    std::stringstream ss;
    ss << "Ring[";
    for(unsigned int i=0; i< itsAtoms.size(); ++i){
        ss << itsAtoms[i]->getFileID() << " ";
    }   
    ss << "]" << std::endl;
    Logger::Instance()->writeToLogFile(ss.str());
}

} //namespace LBIND
