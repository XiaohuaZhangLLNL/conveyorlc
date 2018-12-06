/* 
 * File:   Grid.h
 * Author: zhang
 *
 * Created on February 21, 2014, 4:31 PM
 */

#ifndef GRID_H
#define	GRID_H

#include <vector>
#include <string>
#include <sstream>

namespace LBIND{
    
class Complex;
class Atom;
class Coor3d;

class Grid {
public:
    Grid(Complex *pCom);
    Grid(Complex *pCom, bool outPDB);
    Grid(const Grid& orig);
    virtual ~Grid();
    
    
    void run(double probeRadius, int numberSphere, double minVolume);
    void getTopSiteGeo(Coor3d& dockDim, Coor3d& centroid);
    void getTopSiteGeo(Coor3d& dockDim, Coor3d& centroid, double& volume);
    int  getSiteIndex();     
    bool getKeySiteGeo(Coor3d& aveKeyResCoor, Coor3d& dockDim, Coor3d& centroid, double& volume);
    
    //void setNumSphere(int numSphere);
    void setSpacing(double spacing);
    void setCutoffCoef(double cutoffCoeff);
    void writeCutRecPDB(std::string& fileName, Complex* pComplex, double cutRadius);
        
private:
    void generateSpPoints();
    void getGridBox();
    void getSiteGrids();
   
    void writeGridPDB(std::string& fileName, std::vector<Coor3d*>& outGrids, const std::string& resName="UNK");
    void clustGrids();
    // Merge two groups of grids and store in clustI if nearest points <=1 angstom.
    bool mergeGrids(std::vector<Coor3d*>& clustI, std::vector<Coor3d*>& clustJ);
    
    void siteSurface(std::vector<Coor3d*>& clust);
    void siteCentroid(std::vector<Coor3d*>& clust);
    void siteCentroid(std::vector<Coor3d*>& clust, Coor3d& dockDim, Coor3d& centroid);
    void siteAverage(std::vector<Coor3d*>& clust, Coor3d& aveCoor);
    
    void toOutput();
    
private:
    Complex *pComplex;
    int numSphere;
    unsigned minVol;
    double probe;
    double spacing;
    double cutoffCoef;
    bool outputPDB;
    
    int siteIndex;
    
    std::vector<Coor3d*> spPoints;
    std::vector<Coor3d*> grids;
    std::vector<Atom*> atomList;
    
    std::vector<std::vector<Coor3d*> > clusters;
    
    std::stringstream ss;

};

}//namespace LBIND
#endif	/* GRID_H */

