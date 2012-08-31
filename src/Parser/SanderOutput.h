/* 
 * File:   SanderOutput.h
 * Author: zhang30
 *
 * Created on August 30, 2012, 6:17 PM
 */

#ifndef SANDEROUTPUT_H
#define	SANDEROUTPUT_H

#include <string>

namespace LBIND{

class SanderOutput {
public:
    SanderOutput();
    SanderOutput(const SanderOutput& orig);
    virtual ~SanderOutput();
    
    double getEAmber(std::string sanderOutFile);
    double getEnergy(std::string sanderOutFile);

private:

};

} //namespace LBIND
#endif	/* SANDEROUTPUT_H */

