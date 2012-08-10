/* 
 * File:   Bond.cpp
 * Author: zhang
 * 
 * Created on May 27, 2010, 9:09 AM
 */

#include "Bond.h"
#include "Atom.h"

namespace LBIND{

Bond::Bond(Atom* pAtom1,Atom* pAtom2):
        atom1(pAtom1),
        atom2(pAtom2),
        value(0),
        type(0),
        topology(0),
        rotatable(false),
        rotAngle(0){
}

Bond::Bond(const Bond& orig) {
}

Bond::~Bond() {
}

void Bond::setAtom1(Atom* pAtom1){
    this->atom1=pAtom1;
}
void Bond::setAtom2(Atom* pAtom2){
    this->atom2=pAtom2;
}

Atom* Bond::getAtom1(){
    return this->atom1;
}

Atom* Bond::getAtom2(){
    return this->atom2;
}

void Bond::setType(int ty){
    this->type=ty;
}
int Bond::getType(){
    return this->type;
}

void Bond::setTopology(int top){
    this->topology=top;
}
int Bond::getTopology(){
    return this->topology;
}

void Bond::setRotatable(bool rot){
    this->rotatable=rot;
}

bool Bond::getRotatable(){
    return this->rotatable;
}

void Bond::setRotAngle(double angle){
    this->rotAngle=angle;
}

double Bond::getRotAngle(){
    return this->rotAngle;
}

} //namespace LBIND
