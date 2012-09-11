/* 
 * File:   Fragment.h
 * Author: zhang
 *
 * Created on May 21, 2010, 1:45 PM
 */

#ifndef _FRAGMENT_H
#define	_FRAGMENT_H

#include <string>
#include <vector>

#include "BaseStruct.h"

namespace LBIND{
    class Molecule;
    class Atom;

class Fragment : public BaseStruct {
public:
    Fragment();
    Fragment(Molecule* parent);
    Fragment(const Fragment& orig);
    virtual ~Fragment();

    virtual std::vector<Atom*> getChildren();
    virtual void setChildren(std::vector<Atom*>& atomList);
    
    virtual Atom* addAtom();
    
    Molecule* getParent();

protected:

    Molecule* pParent;
    std::vector<Atom*> itsChildren;

};

}//namespace LBIND

#endif	/* _FRAGMENT_H */

