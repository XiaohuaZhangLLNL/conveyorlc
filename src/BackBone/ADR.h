/* 
 * File:   ADR.h
 * Author: zhang30
 *
 * Created on December 6, 2011, 8:31 AM
 */

#ifndef ADR_H
#define	ADR_H

#include <string>
#include <vector>
#include "Base.h"

namespace LBIND {
    class Pathway;
class ADR : public Base {
public:
    ADR();
    ADR(int idVal, std::string nameVal);
    ADR(const ADR& orig);
    virtual ~ADR();
private:
    std::vector<Pathway*> pathways;

};

}//namespace LBIND 

#endif	/* ADR_H */

