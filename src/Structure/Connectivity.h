/* 
 * File:   Connectivity.h
 * Author: zhang
 *
 * Created on May 27, 2010, 9:17 AM
 */

#ifndef _CONNECTIVITY_H
#define	_CONNECTIVITY_H

namespace LBIND{
    class Atom;
    class Molecule;
    class Complex;

class Connectivity {
public:
    Connectivity();
    Connectivity(Complex* pCom);
    Connectivity(const Connectivity& orig);
    virtual ~Connectivity();

    void run();

    void assignBond();
    void assignBond(Molecule* pMol);
    bool isBonded(Atom* pAtom1, Atom* pAtom2);
    void determineRotate();
    void determineRotateBond(Molecule* pMol);
    void determineRotateAngle(Molecule* pMol);
    void determineRings();
    void determineRings(Molecule* pMol);
private:
    Complex* pComplex;

};

}//namespace LBIND

#endif	/* _CONNECTIVITY_H */

