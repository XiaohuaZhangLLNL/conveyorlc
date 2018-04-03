/* 
 * File:   Element.cpp
 * Author: zhang
 * 
 * Created on August 23, 2010, 4:15 PM
 */
#include <iostream>
#include "Element.h"
#include "BaseStruct.h"

namespace LBIND{
Element::Element() : BaseStruct(),
        symbol(""),
        mass(0.0),
        valence(0),
        covalentRadius(0.0),
        vdWRadius(0.0),
        paulingEN(0.0)
{
}

Element::Element(const Element& orig) {
}

Element::~Element() {
}

void Element::setAtomicNumber(int num){
    this->setID(num);
}
int Element::getAtomicNumber(){
    return (this->getID());
}
void Element::setElementSymbol(std::string str){
    this->symbol=str;
}
std::string Element::getElementSymbol(){
    return this->symbol;
}
//void Element::setName(std::string str){
//    this->name=str;
//}
//std::string Element::getName(){
//    return this->name;
//}
void Element::setAtomicMass(double atomMass){
    this->mass=atomMass;
}
double Element::getAtomicMass(){
    return this->mass;
}
void Element::setValence(int valence){
    this->valence=valence;
}
int Element::getValence(){
    return this->valence;
}
void Element::setCovalent(int covalent){
    this->covalent=covalent;
}
int Element::getCovalent(){
    return this->covalent;
}
void Element::setCovalentRadius(double radius){
    this->covalentRadius=radius;
}
double Element::getCovalentRadius(){
    return this->covalentRadius;
}
void Element::setVDWRadius(double radius){
    this->vdWRadius=radius;
}
double Element::getVDWRadius(){
    return this->vdWRadius;
}
void Element::setENV(double env){
    this->paulingEN=env;
}
double Element::getENV(){
    return this->paulingEN;
}

void Element::print(){
    std::cout << "Element: Symbol    --> " << symbol << std::endl;
    std::cout << "         AtomNumber--> " << this->getID() << std::endl;
    std::cout << "         Name      --> " << this->getName() << std::endl;
    std::cout << "         Mass      --> " << mass << std::endl;
    std::cout << "         valence   --> " << valence << std::endl;
    std::cout << "         vdWRadius --> " << vdWRadius << std::endl;
    std::cout << "         paulingEN --> " << paulingEN << std::endl;
    std::cout << "         covalentRadius--> " << covalentRadius << std::endl;

}

}//namespace LBIND




