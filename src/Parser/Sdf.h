/* 
 * File:   Sdf.h
 * Author: zhang30
 *
 * Created on August 10, 2012, 3:12 PM
 */

#ifndef SDF_H
#define	SDF_H

#include <string>
#include "Structure/Molecule.h"

namespace LBIND{

class Sdf {
public:
    Sdf();
    Sdf(const Sdf& orig);
    virtual ~Sdf();
    
    void parse(const std::string& fileName);
    void read(const std::string& fileName, Molecule* pMolecule);
    std::string getInfo(const std::string& fileName, const std::string& keyword);
    std::string getTitle(const std::string& fileName);
    bool calcBoundBox(const std::string& fileName, Coor3d& centroid, Coor3d& boxDim);
    
private:

};

} //namespace LBIND
#endif	/* SDF_H */

