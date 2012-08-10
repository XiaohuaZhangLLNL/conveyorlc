/* 
 * File:   Bond.h
 * Author: zhang
 *
 * Created on May 27, 2010, 9:09 AM
 */

#ifndef _BOND_H
#define	_BOND_H

namespace LBIND{
    class Atom;

class Bond {
public:
    Bond(Atom* pAtom1,Atom* pAtom2);
    Bond(const Bond& orig);
    virtual ~Bond();
    void setAtom1(Atom* pAtom1);
    void setAtom2(Atom* pAtom2);
    Atom* getAtom1();
    Atom* getAtom2();
    void setType(int ty);
    int getType();
    void setTopology(int top);
    int getTopology();
    void setRotatable(bool rot);
    bool getRotatable();
    void setRotAngle(double angle);
    double getRotAngle();
private:
    Atom* atom1;
    Atom* atom2;
    double value;
    /*!
        Bond Type Definitions
        - 0 = Undefined
        - 1 = Single
        - 2 = Double
        - 3 = Triple
        - 4 = Aromatic
        - 5 = Single or Double
        - 6 = Single or Aromatic
        - 7 = Double or Aromatic
        - 8 = Any type
    */    
    int type;
    /*!
        Bond Topology Definitions
        - 0 = Either
        - 1 = Ring
        - 2 = Chain
    */
    int topology;
    bool rotatable;
    double rotAngle;
};

}//namespace LBIND

#endif	/* _BOND_H */

