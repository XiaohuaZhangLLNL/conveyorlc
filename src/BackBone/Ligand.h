/* 
 * File:   Ligand.h
 * Author: zhang30
 *
 * Created on January 5, 2012, 3:18 PM
 */

#ifndef LIGAND_H
#define	LIGAND_H

#include "Base.h"

namespace LBIND {
class Ligand : public Base {
public:
    Ligand();
    Ligand(const Ligand& orig);
    virtual ~Ligand();
private:

};

}//namespace LBIND 

#endif	/* LIGAND_H */

