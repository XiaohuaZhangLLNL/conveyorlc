/* 
 * File:   Surface.cpp
 * Author: zhang
 * 
 * Created on February 19, 2014, 11:56 PM
 */

#include <cmath>
#include <vector>
#include "Surface.h"
#include "Structure/Coor3d.h"
#include "Structure/Constants.h"
#include "Structure/Atom.h"
#include "Structure/Fragment.h"
#include "Structure/Molecule.h"
#include "Structure/Complex.h"

namespace LBIND{

Surface::Surface(Complex* pCom) : 
    pComplex(pCom), 
    numSphere(960),
    totalSASA(0),
    probe(1.4){

}

Surface::Surface(const Surface& orig) {
}

Surface::~Surface() {
    for(unsigned i=0; i<spPoints.size(); ++i){
        Coor3d *point=spPoints[i];
        delete point;
    }
    spPoints.clear();
    atomList.clear();
}

void Surface::run(double probeRadius, int numberSphere){
    probe=probeRadius;
    numSphere=numberSphere;
    generateSpPoints();
    atomList=pComplex->getAtomList();
    
    //std::cout << "Number of atom: " << atomList.size() << std::endl;
    
    calculateSASA();   
}

double Surface::getTotalSASA(){
    return totalSASA;
}

void Surface::generateSpPoints(){
    
    double inc=PI*(3-sqrt(5));
    double offset=2/static_cast<double>(numSphere);
    
    for(int i=0; i<numSphere; ++i){
        double y=i*offset-1+offset/2;
        double r=sqrt(1-y*y);
        double phi=i*inc;
        double x=cos(phi)*r;
        double z=sin(phi)*r;
        Coor3d *point=new Coor3d(x, y, z);
        spPoints.push_back(point);
    }
    
}



void Surface::findNeighbors(Atom *curAtom, std::vector<Atom*>& neighborAtoms, double cutoff){

    double radius=curAtom->getElement()->getVDWRadius()+cutoff;
    Coor3d *curCoor=curAtom->getCoords();
    
    neighborAtoms.clear();
    
    for(unsigned i=0; i<atomList.size(); ++i){
        Atom *pAtom=atomList[i];
        if(pAtom!=curAtom){
            Coor3d *pCoor=pAtom->getCoords();
            double dist2=pCoor->dist2(curCoor);
            double r=radius+pAtom->getElement()->getVDWRadius();
            if(dist2<r*r){
                neighborAtoms.push_back(pAtom);
            }
        }
    }
    
}

void Surface::calculateSASA(){
    
    const double coef=4.0*PI/numSphere;
    const double cutoff=2*probe;
    
    // Calculate SASA for each atom;
    for(int i=0; i<atomList.size(); ++i){
        Atom *curAtom=atomList[i];
        std::vector<Atom*> neighborAtoms;
        findNeighbors(curAtom, neighborAtoms,cutoff);
        
        double radius=curAtom->getElement()->getVDWRadius()+probe;
        //std::cout << "Atom " << curAtom->getName() << " radius "<< curAtom->getElement()->getVDWRadius() << std::endl;
        
        int numAccPoint=0;
        
        for(int j=0; j<spPoints.size(); ++j){
            double xPoint=radius*spPoints[j]->getX()+curAtom->getX();
            double yPoint=radius*spPoints[j]->getY()+curAtom->getY();
            double zPoint=radius*spPoints[j]->getZ()+curAtom->getZ();
            Coor3d coorPoint(xPoint, yPoint, zPoint);
            
            bool isAccPoint=true;
            
            for(int k=0; k<neighborAtoms.size(); ++k){
                Atom *pAtom=neighborAtoms[k];
                double dist2=pAtom->getCoords()->dist2(coorPoint);
                double r=probe+pAtom->getElement()->getVDWRadius();
                if(dist2<r*r){
                    isAccPoint=false;
                    break;
                }
            }
            
            if(isAccPoint){
                ++numAccPoint;
            }
        }
        
        double area=coef*radius*radius*numAccPoint;       
        curAtom->setSASA(area);
        totalSASA+=area;
    }
        
    
}

}//namespace LBIND