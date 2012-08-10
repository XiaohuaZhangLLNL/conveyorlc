/* 
 * File:   Coordinates.cpp
 * Author: zhang
 * 
 * Created on December 3, 2010, 2:21 PM
 */

#include "Coordinates.h"

namespace LBIND{
Coordinates::Coordinates(): itsDelFlg(false), itsEnergy(0.0), itsFreeE(0.0),
    itsEntropy(0.0), itsEnthalpy(0.0){
}

Coordinates::Coordinates(const Coordinates& orig) {
}

Coordinates::~Coordinates() {
    for(unsigned int i=0; i<itsCoordinates.size(); i++){
        delete itsCoordinates[i];
    }
    itsCoordinates.clear();
}

void Coordinates::setDelFlg(bool delFlg){
    itsDelFlg=delFlg;
}

bool Coordinates::getDelFlg(){
    return itsDelFlg;
}

void Coordinates::setEnergy(double energy){
    itsEnergy=energy;
}

double Coordinates::getEnergy(){
    return itsEnergy;
}

void Coordinates::setFreeE(double freeE){
    itsFreeE=freeE;
}

double Coordinates::getFreeE(){
    return itsFreeE;
}

void Coordinates::setEnthalpy(double enthalpy){
    itsEnthalpy=enthalpy;
}

double Coordinates::getEnthalpy(){
    return itsEnthalpy;
}

void Coordinates::setEntropy(double entropy){
    itsEntropy=entropy;
}

double Coordinates::getEntropy(){
    return itsEntropy;
}

void Coordinates::getCoordinates(std::vector<Coor3d*>& atmCoorList){
    atmCoorList=itsCoordinates;
//    for (unsigned int i = 0; i < itsCoordinates.size(); ++i)
//    {
//        atmCoorList.push_back(itsCoordinates[i]);
//    }
}
void Coordinates::addCoor3d(Coor3d* pCoor){
    Coor3d* pNewCoor= new Coor3d(pCoor->getX(),pCoor->getY(),pCoor->getZ());
    itsCoordinates.push_back(pNewCoor);
}


}//namespace LBIND
