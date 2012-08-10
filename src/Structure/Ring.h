/* 
 * File:   Ring.h
 * Author: zhang
 *
 * Created on May 27, 2010, 9:09 AM
 */

#ifndef _RING_H
#define	_RING_H

#include <vector>

namespace LBIND{
    class Atom;

class Ring {
public:
    Ring();
    Ring(const Ring& orig);
    virtual ~Ring();
    void setID(int id);
    int getID();
    void addAtom(Atom* pAtom);
    std::vector<Atom*> getAtomList();
    
    void setSize(int s);
    int getSize();
    
    void setPlanar(bool pl);
    bool getPlanar();
    
    void setAromatic(bool ar);
    bool getAromatic();
    
    void setHetero(bool he);
    bool getHetero();
    
    void setNHetero(int nh);
    int getNHetero();
    
    void setNNitrogen(int nn);
    int getNNitrogen();
    
    void setNOxygen(int no);
    int getNOxygen();
    
    void setNSulfur(int ns);
    int getNSulfur();
    
    void printRing();
    void loggingRing();
    
private:
    int itsID;
    std::vector<Atom*> itsAtoms;
    //! Size of ring
    int size;

    /*!
       Is the ring planar?
    */
    bool planar;

    /*!
       Is the ring aromatic?
    */
    bool aromatic;

    /*!
       Is the ring a heterocycle?
    */
    bool hetero;

    //! Number of heteroatoms
    int nHetero;

    //! Number of Nitrogen atoms
    int nNitrogen;

    //! Number of Oxygen atoms
    int nOxygen;

    //! Number of Sulfur atoms
    int nSulfur;

};

}//namespace LBIND

#endif	/* _RING_H */

