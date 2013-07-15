/* 
 * File:   Atom.h
 * Author: xiaohua
 *
 * Created on May 26, 2009, 7:20 AM
 */

#ifndef _ATOM_H
#define	_ATOM_H

#include <iostream>
#include <string>
#include <vector>

#include "BaseStruct.h"
#include "Element.h"

namespace LBIND{

    class Element;
    class Molecule;
    class Fragment;
    class Coor3d;

class Atom : public BaseStruct {
public:
    Atom();

    Atom(Fragment* parent);

    Atom(Molecule* supParent);
    
    Atom(const Atom& orig);

    Atom(Atom* pAtom, Fragment *parent = 0);

    Atom(Atom* pAtom, Molecule *parent = 0);

    virtual ~Atom();

    void setSymbol(std::string symb);

    std::string getSymbol();

    void setElement(Element* pEle);

    Element* getElement();

    void setCoords(const double& x,const double& y,const double& z);

    Coor3d*  getCoords();

    double  getX();

    double getY();

    double getZ();

    void setFileID(const int& fileID); // input file indexing

    int  getFileID();
    
    int getFragID();
    
    int getMolID();
    
    void setType(int type);
    
    int getType();
    
    void setCharge(double charge);
    
    double getCharge();
    
    void setFormalCharge(int fcharge);
    
    int getFormalCharge();    
    
    void setHybridization(int hybrid);
    int getHybridization();

    void setParent(Fragment* parent);
    Fragment* getParent();
    void setSupParent(Molecule* supParent);
    Molecule* getSupParent();

    bool isBoundto(Atom* pAtom);
    
protected:

    //! symbol
    std::string symbol;

    //! element point
    Element* pElement;

    //! file id
    int itsFileID;

    //! Atom Type for Ring
    /*!
        atom Type Definitions
        - 0 = Undefined
        - 1 = Hydrogen
        - 2 = Terminal Heavy Atom
        - 3 = Open Chain Heavy Atom
        - 4 = Closed Chain Heavy Atom
        - 5 = Ring Heavy Atom
        - 6 = Aromatic Ring Heavy Atom
        - 7 = Chain Atom (not terminal, Ring or Hydrogen)
        - 8 = Chain or terminal Atom
    */
    int itsType;

    //! nuclear charge
    double itscharge;
    //! formal charge
    int itsFormalCharge;    
    /*!
       atom hybridization definitions
       - 0 = undefined
       - 1 = s
       - 2 = sp
       - 3 = sp2
       - 4 = sp3
       - 5 = sp3d
       - 6 = sp3d2
       - 7 = sp3d3
       - 8 = other
    */
    int itsHybridization;    
    //! coordinates
    Coor3d* pCoords;

    Fragment* pParent;
    Molecule* pSupParent;
}; // class Atom

} // namespace LBIND.
#endif	/* _ATOM_H */

