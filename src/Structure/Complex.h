/* 
 * File:   Complex.h
 * Author: zhang
 *
 * Created on May 21, 2010, 1:45 PM
 */

#ifndef _COMPLEX_H
#define	_COMPLEX_H

#include <vector>

namespace LBIND{
    class Control;
    class Molecule;
    class Coordinates;
    class Coor3d;
    class ElementContainer;

class Complex {
public:
    Complex();
    Complex(Control* pCtrl);
    Complex(const Complex& orig);
    virtual ~Complex();

    Control* getControl();
    std::vector<Molecule*> getChildren();

    Molecule* addMolecule();
    void assignElement(ElementContainer* pElementContainer);
    void assignCoordinate(std::vector<Coor3d*>& atmCoorList);
    void assignCoordinate(Coordinates* coords);
    void getCoordinate(Coordinates* pCoordinates);
    
    int getTotNumAtom();
    
protected:
    Control* pControl;
    std::vector<Molecule*> itsChildren; 

};

} //namespace LBIND

#endif	/* _COMPLEX_H */

