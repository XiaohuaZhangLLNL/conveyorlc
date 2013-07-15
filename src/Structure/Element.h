/* 
 * File:   Element.h
 * Author: zhang
 *
 * Created on August 23, 2010, 4:15 PM
 */

#ifndef ELEMENT_H
#define	ELEMENT_H

#include "BaseStruct.h"

namespace LBIND{

class Element :public BaseStruct {
public:
    Element();
    Element(const Element& orig);
    virtual ~Element();

    void setAtomicNumber(int num);
    int getAtomicNumber();
    void setElementSymbol(std::string str);
    std::string getElementSymbol();
    void setAtomicMass(double atomMass);
    double getAtomicMass();
    void setValence(int valence);
    int getValence();
    void setCovalentRadius(double radius);
    double getCovalentRadius();
    void setVDWRadius(double radius);
    double getVDWRadius();
    void setENV(double env);
    double getENV();
    void print();
private:

    //! Symbol
    std::string symbol;

    //! Mass
    double mass;

    //! Standard valence
    int valence;

    //! Covalent radius
    double covalentRadius;

    //! van der Waals radius
    double vdWRadius;

    //! Pauling electronegativity value
    double paulingEN;


};

}//namespace LBIND
#endif	/* ELEMENT_H */

