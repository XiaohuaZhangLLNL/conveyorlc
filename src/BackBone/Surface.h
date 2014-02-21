/* 
 * File:   Surface.h
 * Author: zhang
 *
 * Created on February 19, 2014, 11:56 PM
 */

/*
 * Calculate the Accessible Surface Area of a set of atoms using 
 * the Shrake-Rupley algorithm. 
 * 
 * Shrake, A., and J. A. Rupley. "Environment and Exposure to Solvent
 * of Protein Atoms. Lysozyme and Insulin." JMB (1973) 79:351-371."
*/

#include <vector>

#ifndef SURFACE_H
#define	SURFACE_H

namespace LBIND{

class Coor3d;
class Complex;
class Atom;

class Surface {
public:
    Surface(Complex *pCom);
    Surface(const Surface& orig);
    virtual ~Surface();
    
    void run(double prob, int numSphere);
    double getTotalSASA();
    
private:
    void generateSpPoints();
    void getAtomList();
    void findNeighbors(Atom* curAtom, std::vector<Atom*>& neighborAtoms, double cutoff);
    void calculateSASA();
    
private:
    Complex *pComplex;
    int numSphere;
    double probe;
    double totalSASA;
    std::vector<Coor3d*> spPoints; // list of 3d coordinates of points on a sphere
    std::vector<Atom*> atomList;
};

} //namespace LBIND
#endif	/* SURFACE_H */

