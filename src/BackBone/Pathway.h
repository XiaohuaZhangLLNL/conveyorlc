/* 
 * File:   Pathway.h
 * Author: zhang30
 *
 * Created on December 6, 2011, 8:31 AM
 */

#ifndef PATHWAY_H
#define	PATHWAY_H

#include <string>
#include <vector>
#include "Base.h"

namespace LBIND {
    class ADR;
    class Protein;
class Pathway : public Base {
public:
    Pathway();
    Pathway(int idVal, std::string nameVal);
    Pathway(const Pathway& orig);
    virtual ~Pathway();
private:
    std::string ecNumber;
    std::vector<ADR*> adrs;
    std::vector<Protein*> pdbinfos;
};

}//namespace LBIND 
#endif	/* PATHWAY_H */

