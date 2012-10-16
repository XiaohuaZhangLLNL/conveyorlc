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
    
    bool getEAmber(std::string sanderOutFile, double& energy);
    bool getEnergy(std::string sanderOutFile, double& energy);

private:

};

} //namespace LBIND
#endif	/* SANDEROUTPUT_H */

