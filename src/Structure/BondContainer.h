/* 
 * File:   BondContainer.h
 * Author: zhang
 *
 * Created on August 23, 2010, 1:20 PM
 */

#ifndef BONDCONTAINER_H
#define	BONDCONTAINER_H

#include <vector>

namespace LBIND{
    class Atom;
    class Bond;
    
class BondContainer {
public:
    BondContainer();
    BondContainer(const BondContainer& orig);
    virtual ~BondContainer();

    std::vector<Bond*> getBondList();
    void addBond(Atom* pAtom1, Atom* pAtom2);
    bool isBonded(Atom* pAtom1, Atom* pAtom2);
    Bond* getBond(Atom* pAtom1, Atom* pAtom2);
    int numBondNoH(Atom* pAtom);
    int numBond(Atom* pAtom);
    void getBondAtoms(Atom* pAtom, std::vector<Atom*>& bondAtoms);
    void printList();
    void printRotList();
    void loggingList();
    void loggingRotList();    
private:
    std::vector<Bond*> itsBondList;
};

}//namespace LBIND
#endif	/* BONDCONTAINER_H */

