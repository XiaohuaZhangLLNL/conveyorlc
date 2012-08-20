/* 
 * File:   VinaLC.h
 * Author: zhang30
 *
 * Created on August 17, 2012, 1:32 PM
 */

#ifndef VINALC_H
#define	VINALC_H

#include <string>
#include <Structure/Coor3d.h>

namespace LBIND {
    
struct SphDat{
    std::string line;
    double dist;
};    

class VinaLC {
public:
    VinaLC();
    VinaLC(const VinaLC& orig);
    virtual ~VinaLC();
    
    void centroid(std::string& sumFile, Coor3d& center);
    void dms(std::string& cutPdbFile, std::string& surfFile);
    void sphgen(std::string& surfFile, std::string& sphFile);
    void sphere_selector(std::string& sphFile, std::string& selSphFile, Coor3d& center);
    void getGridDims(std::string& selSphFile, Coor3d& gridDims);
    void vinalcGridDims(std::string& cutPdbFile, Coor3d& center, Coor3d& gridDims);
    
private:

};

}//namespace LBIND

#endif	/* VINALC_H */

