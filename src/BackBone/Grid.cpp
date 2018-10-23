/* 
 * File:   Grid.cpp
 * Author: zhang
 * 
 * Created on February 21, 2014, 4:31 PM
 */

#include <algorithm>
#include <fstream>

#include "Grid.h"
#include "Structure/Complex.h"
#include "Structure/Constants.h"
#include "Structure/Atom.h"
#include "Structure/Molecule.h"
#include "Structure/Fragment.h"
#include "Structure/Coor3d.h"
#include "Parser/Pdb.h"
#include "Structure/Sstrm.hpp"
#include "Common/LBindException.h"

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

namespace LBIND{

Grid::Grid(Complex *pCom)  :
    pComplex(pCom), 
    numSphere(100),
    minVol(50),
    probe(1.4),
    spacing(1.4),
    cutoffCoef(1.1),
    outputPDB(true)
{
}

Grid::Grid(Complex *pCom, bool outPDB)  :
    pComplex(pCom), 
    numSphere(100),
    minVol(50),
    probe(1.4),
    spacing(1.4),
    cutoffCoef(1.1),
    outputPDB(outPDB)
{
}

Grid::Grid(const Grid& orig) {
}

Grid::~Grid() {
    for(unsigned i=0; i<spPoints.size(); ++i){
        Coor3d *point=spPoints[i];
        delete point;
    }
    spPoints.clear(); 
    
    for(unsigned i=0; i<grids.size(); ++i){
        Coor3d *pGrid=grids[i];
        delete pGrid;
    }
    grids.clear();    
    
}

void Grid::setSpacing(double spacing){
    this->spacing=spacing;
}

void Grid::setCutoffCoef(double cutoffCoeff){
    this->cutoffCoef=cutoffCoeff;
}

void Grid::run(double probeRadius, int numberSphere, double minVolume){
    probe=probeRadius;
    numSphere=numberSphere;
    minVol=minVolume;
    
    std::vector<Atom*> allAtoms=pComplex->getAtomList();
    for(unsigned i=0; i<allAtoms.size(); ++i){
        Atom* pAtom=allAtoms[i];
        std::string resName=pAtom->getParent()->getName();
        if(resName=="HOH" || resName=="WAT"){
            continue;
        }else{
            atomList.push_back(pAtom);
        }
    }
    generateSpPoints();
    
    getGridBox();
    
    std::string fileName;

    if (outputPDB) {
        fileName = "gridPDB1.pdb";
        writeGridPDB(fileName, grids);
    }

    getSiteGrids();

    if (outputPDB) {
        fileName = "gridPDB2.pdb";
        writeGridPDB(fileName, grids);
    }
    
    clustGrids();
    
    toOutput();
}

void Grid::generateSpPoints(){
    
    double radius=5.0;
    
    double inc=PI*(3-sqrt(5));
    double offset=2/static_cast<double>(numSphere);
    
    for(int i=0; i<numSphere; ++i){
        double y=i*offset-1+offset/2;
        double r=sqrt(1-y*y);
        double phi=i*inc;
        double x=cos(phi)*r;
        double z=sin(phi)*r;
        Coor3d *point=new Coor3d(radius*x, radius*y, radius*z);//scaled 
        spPoints.push_back(point);
    }
    
}

void Grid::getGridBox(){
    // Determine dimensions of protein
    double xMax=BIGNEGTIVE;
    double yMax=BIGNEGTIVE;
    double zMax=BIGNEGTIVE;
    
    double xMin=BIGPOSITIVE;
    double yMin=BIGPOSITIVE;
    double zMin=BIGPOSITIVE;
    
    
    
    for(unsigned i=0; i< atomList.size(); ++i){
        Coor3d* pCoor=atomList[i]->getCoords();
        if(pCoor->getX()>xMax) xMax=pCoor->getX();
        if(pCoor->getY()>yMax) yMax=pCoor->getY();
        if(pCoor->getZ()>zMax) zMax=pCoor->getZ();
        
        if(pCoor->getX()<xMin) xMin=pCoor->getX();
        if(pCoor->getY()<yMin) yMin=pCoor->getY();
        if(pCoor->getZ()<zMin) zMin=pCoor->getZ();        
    }
    
    ss << "Box dimension:" << std::endl;
    ss << "   MIN  x=" << xMin << "     y=" << yMin << "     z=" << zMin << std::endl;
    ss << "   MAX  x=" << xMax << "     y=" << yMax << "     z=" << zMax << std::endl;
    
    int xLowIndex=static_cast<int>(xMin/spacing)-1;
    int yLowIndex=static_cast<int>(yMin/spacing)-1;
    int zLowIndex=static_cast<int>(zMin/spacing)-1;

    int xHighIndex=static_cast<int>(xMax/spacing)+1;
    int yHighIndex=static_cast<int>(yMax/spacing)+1;
    int zHighIndex=static_cast<int>(zMax/spacing)+1;
        
    for(int i=xLowIndex; i<xHighIndex; ++i){
        for(int j=yLowIndex; j<yHighIndex; ++j){
            for(int k=zLowIndex; k<zHighIndex; ++k){
                double xGrid=static_cast<double>(i)*spacing;
                double yGrid=static_cast<double>(j)*spacing;
                double zGrid=static_cast<double>(k)*spacing;
                bool isOverlap=false;
                
                for(unsigned m=0; m<atomList.size(); ++m){                    
                    Atom* pAtom=atomList[m];
                    double dist2=pAtom->getCoords()->dist2(xGrid, yGrid, zGrid);
                    double r=pAtom->getElement()->getVDWRadius()+probe;
//                    
                    if(dist2<r*r){
                        isOverlap=true;
                        break;
                        
                    } // skip grid generation
//                    
   
                }

                if(!isOverlap){
                    Coor3d *pCoor = new Coor3d(xGrid, yGrid, zGrid);
                    grids.push_back(pCoor); 
                }
            }
        }
        
    }
    
    
    // 
    ss <<"Number of point in box: " << grids.size() <<std::endl;
    

    
}

void Grid::getSiteGrids() {
    
    std::vector<Coor3d*> tmpGrids;
    
    for(unsigned g=0; g<grids.size(); ++g){
        
        int numAccPoint = 0;
        Coor3d *pGrid=grids[g];

        for (unsigned p = 0; p < spPoints.size(); ++p) {
            
            Coor3d *pPoint=spPoints[p];

            double xPoint = pPoint->getX() + pGrid->getX();
            double yPoint = pPoint->getY() + pGrid->getY();
            double zPoint = pPoint->getZ() + pGrid->getZ();
            Coor3d coorPoint(xPoint, yPoint, zPoint);

            for (unsigned q = 0; q < atomList.size(); ++q) {
                Atom *pAtom = atomList[q];
                double dist2 = pAtom->getCoords()->dist2(coorPoint);
                double rr = probe + pAtom->getElement()->getVDWRadius();
                if (dist2 < rr * rr) {
                    ++numAccPoint;
                    break;
                }
            }

        }
        
        double ratio = static_cast<double> (numAccPoint) / spPoints.size();
        //std::cout <<"Ratio: " << ratio <<std::endl;

        if (ratio > 0.6) {
            Coor3d* tmpPoint=new Coor3d(pGrid->getX(),pGrid->getY(),pGrid->getZ());
            tmpGrids.push_back(tmpPoint);
        }
    }
    
    ss <<"Number of point in cavity sites: " << tmpGrids.size() <<std::endl;
    
    for(unsigned i=0; i<grids.size(); ++i){
        Coor3d *pGrid=grids[i];
        delete pGrid;
    }
    grids.clear();  
    
    grids=tmpGrids;
    ss <<"Number of point in cavity sites: " << grids.size() <<std::endl;
}

int Grid::getSiteIndex(){
    return this->siteIndex;
}

void Grid::writeCutRecPDB(std::string& fileName, Complex* pComplex, double cutRadius){
    std::ofstream outFile;
    try {
        outFile.open(fileName.c_str());
    }
    catch(...){
        std::cout << "PDB::read >> Cannot open file" << fileName << std::endl;
    }

    outFile << "REMARK CUT PROTEIN FROM CLUSTER " << this->siteIndex << std::endl;
    
    double cutRadius2=cutRadius*cutRadius;
    
    std::vector<Coor3d*> siteGridsCoors=this->clusters[this->siteIndex]; 
    
    boost::scoped_ptr<Pdb> pPdb(new Pdb());

    std::vector<Molecule*> molList=pComplex->getChildren();
    
    bool outputResidue=false;
    
    for(unsigned i=0;i<molList.size();i++){
        
        std::vector<Fragment*> resList=molList[i]->getChildren();

        for(unsigned j=0;j<resList.size();j++){
            Fragment* pResidue=resList[j];
            Coor3d coorCA;
//            if(pPdb->isAA(pResidue, coorCA)){
//                for(unsigned s=0; s<siteGridsCoors.size(); s++){
//                    Coor3d* pCoor=siteGridsCoors[s];
//                    if(pCoor->dist2(coorCA)<cutRadius2){
//                        outputResidue=true;
//                        break;
//                    }
//                }
//            }else{
            std::vector<Atom*> resAtomList=pResidue->getChildren();
            for(unsigned k=0;k<resAtomList.size();k++){
                Atom* pAtom=resAtomList[k];
                Coor3d* pCoorAt=pAtom->getCoords();
                for(unsigned s=0; s<siteGridsCoors.size(); s++){
                    Coor3d* pCoor=siteGridsCoors[s];
                    if(pCoor->dist2(pCoorAt)<cutRadius2){
                        outputResidue=true;
                        break;
                    }
                } 
                if(outputResidue){
                    break;
                }
            }
//            }
            
            if(outputResidue){
                std::vector<Atom*> resAtomList=pResidue->getChildren();
                
                for(unsigned k=0;k<resAtomList.size();k++){
                    Atom* pAtom=resAtomList[k];
                    outFile << "ATOM  "<< std::setw(5)<< pAtom->getFileID()
                            << " " << std::setw(4) << pAtom->getName() 
                            << " " << std::left << std::setw(3) << pResidue->getName()
                            << " " << std::setw(1) << molList[i]->getName()
                            << std::right << std::setw(4) << pResidue->getID()
                            << "    "                        
                            << std::fixed <<std::setprecision(3)
                            << std::setw(8) << pAtom->getX()
                            << std::setw(8) << pAtom->getY()
                            << std::setw(8) << pAtom->getZ() 
                            <<"                      "
                            << std::setw(2) << pAtom->getSymbol()
                            << std::endl;
                }                
            }
            outputResidue=false;
        }
        outFile << "TER" <<std::endl;
//        std::cout << "i= " << i << std::endl;
    }
    outFile << "END" <<std::endl;

    outFile.close();     
    
}

void Grid::writeGridPDB(std::string& fileName, std::vector<Coor3d*>& outGrids, const std::string& resName){
    std::vector<Atom*> gridAtoms;
    
    for(unsigned g=0; g<outGrids.size(); ++g){
        Atom *pAtom=new Atom();
        pAtom->setName("GPT");
        pAtom->setFileID(g+1);
        pAtom->setCoords(outGrids[g]->getX(), outGrids[g]->getY(), outGrids[g]->getZ()); 
        
        gridAtoms.push_back(pAtom);
    }
    
    Pdb pdbParser;
    
    pdbParser.write(fileName, gridAtoms,resName);
    
    for(unsigned i=0; i< gridAtoms.size(); ++i){
        Atom *pAtom=gridAtoms[i];
        delete pAtom;
    }
    gridAtoms.clear();
    
}

struct great_than_key
{
    inline bool operator() (const std::vector<Coor3d*>& struct1, const std::vector<Coor3d*>& struct2)
    {
        return (struct1.size() > struct2.size());
    }
};

void Grid::clustGrids(){
    //Using bottom-up hierarchical clustering approach.
      
    //Create grids.size() clusters for merge;
    
    for(unsigned i=0; i<grids.size(); ++i){
        Coor3d* pCoor=grids[i];
        std::vector<Coor3d*> clust;
        clust.push_back(pCoor);
        clusters.push_back(clust);
    }
    
    bool isMerged=true; 
    
    while (isMerged) {
        isMerged=false;
        //Keep the empty element here.
        for (unsigned i = 0; i < clusters.size()-1; ++i) {
            if(clusters[i].size()==0) break;

            for (unsigned j = i + 1; j < clusters.size(); ++j) {
                if(clusters[j].size()==0) break;
                
                bool doMerged=mergeGrids(clusters[i], clusters[j]);
                if(doMerged){
                    isMerged=true;
                }
                

            }
        }
        
        std::vector<std::vector<Coor3d*> > tmpClusters;
        
        for(unsigned i=0; i<clusters.size(); ++i){
            if(clusters[i].size()!=0) tmpClusters.push_back(clusters[i]);
        }   
        
        clusters=tmpClusters;
        ss << "Size of clusters: " << clusters.size() << std::endl;
        
    }
    
    //sort by volume
    std::sort(clusters.begin(), clusters.end(), great_than_key());

    
//    std::vector<std::vector<Coor3d*> > tmpClusters2;
//
//    for(unsigned i=0; i<clusters.size(); ++i){
//        // minVol default value 50 is about 4-5 water molecular volume.
//        if(clusters[i].size()>minVol) tmpClusters2.push_back(clusters[i]);
//    }   
//    clusters.clear();
//    clusters=tmpClusters2; 
    

    double spacing3=spacing*spacing*spacing;   
    
    for (unsigned i = 0; i < clusters.size(); ++i) {
        
        double volume=clusters[i].size()*spacing3;
        if(i!=0 && volume<minVol) break; 
        // always print out first one. if volume less than minVol don't print out.
        
        ss << "================================================="  << std::endl;
        std::string numStr=Sstrm<std::string, int>(i+1);
        if(i<9){
            numStr="0"+numStr;
        }
        ss << "Cluster " << numStr << "     size : " << clusters[i].size() << std::endl;
        ss << "Cluster " << numStr << "     volume : " << volume << std::endl;
        std::string filename="Grid-"+numStr+".pdb";
        std::string resName="C"+numStr;
        if(i>98){
            resName="CXX";
        }
        
        if(outputPDB){
            writeGridPDB(filename, clusters[i], resName);
        }
        
        siteCentroid(clusters[i]);
        
        siteSurface(clusters[i]);
        ss << "================================================="  << std::endl;
    }
}

bool Grid::mergeGrids(std::vector<Coor3d*>& clustI, std::vector<Coor3d*>& clustJ){
    double coutoff=spacing*spacing*cutoffCoef*cutoffCoef;
    for(unsigned i=0; i<clustI.size(); ++i){
        for(unsigned j=0; j<clustJ.size(); ++j){
            if(clustJ[j]->dist2(clustI[i]) <coutoff){ // Should <=1. 
                for(unsigned j=0; j<clustJ.size(); ++j){
                    clustI.push_back(clustJ[j]);
                }
                clustJ.clear();
                return true;
                
            }            
        }
    }
    
    return false;
    
}


void Grid::siteSurface(std::vector<Coor3d*>& clust){
    
    std::vector<Atom*> surfAtomList;
    for(unsigned i=0; i<atomList.size();++i){
        if(atomList[i]->getSASA() > 0.1){
            surfAtomList.push_back(atomList[i]);

        }
    }    
    
    double chargeArea=0;
    double polarArea=0;
    double hdyrophobArea=0;
    double specialPArea=0;
    double specialHArea=0;
    
    
    for(unsigned i=0; i<surfAtomList.size();++i){
        Atom *pAtom=surfAtomList[i];
        double r=pAtom->getElement()->getVDWRadius()+ probe +2.0; // 2.0 Angstroms for cuttoff
        bool isContacted=false;
        for(unsigned j=0; j<clust.size(); ++j){
            if(pAtom->getCoords()->dist2(clust[j])<r*r){
                isContacted=true;
                break;
            }
        }
        if(isContacted){
            Fragment* pRes=pAtom->getParent();
            //Charged
            if(pRes->getName()=="ARG" ||
               pRes->getName()=="HIS" ||
               pRes->getName()=="HIE" ||
               pRes->getName()=="HID" ||
               pRes->getName()=="HIP" ||
               pRes->getName()=="LYS" ||
               pRes->getName()=="ASP" ||
               pRes->getName()=="GLU"  ){
                chargeArea+=pAtom->getSASA();
            }
            //Polar non-charged
            if(pRes->getName()=="SER" ||
               pRes->getName()=="THR" ||
               pRes->getName()=="ASN" ||
               pRes->getName()=="GLN"  ){
                polarArea+=pAtom->getSASA();
            }
            // Hydrophobic
            if(pRes->getName()=="ALA" ||
               pRes->getName()=="ILE" ||
               pRes->getName()=="LEU" ||
               pRes->getName()=="MET" ||
               pRes->getName()=="PHE" ||
               pRes->getName()=="TRP" ||
               pRes->getName()=="TYR" ||
               pRes->getName()=="VAL"  ){
                hdyrophobArea+=pAtom->getSASA();
            } 
            //Special -Polar
            if(pRes->getName()=="CYS" ||
               pRes->getName()=="CYX" ||
               pRes->getName()=="SEC"  ){
                specialPArea+=pAtom->getSASA();
            }            
            //Special -Hydrophobic
            if(pRes->getName()=="GLY" ||
               pRes->getName()=="PRO"  ){
                specialHArea+=pAtom->getSASA();
            }             
        }
    }
    
    ss << "      Surface"  << std::endl;
    ss << "          Charged         Area:   " << chargeArea << std::endl;
    ss << "          Polar           Area:   " << polarArea << std::endl;
    ss << "          Hydrophobic     Area:   " << hdyrophobArea << std::endl;
    ss << "          Special (Polar) Area:   " << specialPArea << std::endl;
    ss << "          Special (Hydro) Area:   " << specialHArea << std::endl;
    ss << "          total           Area:   " << chargeArea+polarArea+ hdyrophobArea+ specialPArea+specialHArea<< std::endl;
    ss << "          Hydro/Polar    Ratio:   " << (hdyrophobArea+specialHArea)/(chargeArea+polarArea+ specialPArea)<< std::endl;
    
}


void Grid::siteCentroid(std::vector<Coor3d*>& clust) {
    Coor3d centroid(0,0,0);
    for(unsigned i=0; i<clust.size(); ++i){
        Coor3d *pCoor=clust[i];
        centroid=centroid+(*pCoor);
    }
    centroid=centroid/clust.size();
    
    ss << "      Centroid:"  << std::endl;
    
    ss << "                  X= " <<centroid.getX() << 
                                  " Y= " <<centroid.getY() << 
                                  " Z= " <<centroid.getZ() <<std::endl;
    
    // Determine dimensions of cluster
    double xMax=BIGNEGTIVE;
    double yMax=BIGNEGTIVE;
    double zMax=BIGNEGTIVE;
    
    double xMin=BIGPOSITIVE;
    double yMin=BIGPOSITIVE;
    double zMin=BIGPOSITIVE;
       
    for(unsigned i=0; i< clust.size(); ++i){
        Coor3d* pCoor=clust[i];
        if(pCoor->getX()>xMax) xMax=pCoor->getX();
        if(pCoor->getY()>yMax) yMax=pCoor->getY();
        if(pCoor->getZ()>zMax) zMax=pCoor->getZ();
        
        if(pCoor->getX()<xMin) xMin=pCoor->getX();
        if(pCoor->getY()<yMin) yMin=pCoor->getY();
        if(pCoor->getZ()<zMin) zMin=pCoor->getZ();        
    }
    
    double xDim=xMax-xMin;
    double yDim=yMax-yMin;
    double zDim=zMax-zMin;
    
    double xDock=xDim+4;
    double yDock=yDim+4;
    double zDock=zDim+4;
    
    //if(xDock<22) xDock=22;
    //if(yDock<22) yDock=22;
    //if(zDock<22) zDock=22;
    
    
    ss << "      Box dimension:" << std::endl;
    ss << "          MIN         x= " << xMin << "     y= " << yMin << "     z= " << zMin << std::endl;
    ss << "          MAX         x= " << xMax << "     y= " << yMax << "     z= " << zMax << std::endl; 
    ss << "          Dimension   X= " << xDim << "     Y= " << yDim << "     Z= " << zDim << std::endl;
    ss << "      Docking Box dimension Recommended (+/- 5 Angstroms):" << std::endl;
    ss << "          Dimension   X= " << xDock << "     Y= " << yDock << "     Z= " << zDock << std::endl;
    
    double peudoVol=xDim*yDim*zDim;
    double minDim=std::min(std::min(xDim, yDim), zDim);
    double maxDim=std::max(std::max(xDim, yDim), zDim);
    double ratioDim=0;
    if(minDim>0){
        ratioDim=maxDim/minDim;
    }
    double occupied=0;
    if(peudoVol>0){
        occupied=clust.size()/peudoVol;
    }
    double effRatioDim=0;
    if(occupied>0){
        effRatioDim=ratioDim/std::sqrt(occupied);
    }
    double effSize=0;
    if(effRatioDim>0){
        effSize=clust.size()/std::sqrt(effRatioDim);
    }
    
    ss << "      Box Shape:" << std::endl;
    ss << "          Ratio Dimension = " << ratioDim << "            Occupied (%)= " << 100*occupied << std::endl
              << "          Effect  Ratio Dimension =" << effRatioDim << "  Effect Girds= " << effSize << std::endl;
     

}

void Grid::getTopSiteGeo(Coor3d& dockDim, Coor3d& centroid){
    siteCentroid(this->clusters[0], dockDim, centroid);
}

void Grid::getTopSiteGeo(Coor3d& dockDim, Coor3d& centroid, double& volume){
    this->siteIndex=0;
    volume=this->clusters[0].size()*spacing*spacing*spacing;
    siteCentroid(this->clusters[0], dockDim, centroid);
}

bool Grid::getKeySiteGeo(Coor3d& aveKeyResCoor, Coor3d& dockDim, Coor3d& centroid, double& volume){
    
    double curDist2=BIGPOSITIVE;
    int index=-1;
    
    double spacing3=spacing*spacing*spacing;
    
    for(unsigned i=0; i<clusters.size(); ++i){
        if(clusters[i].size()*spacing3 > minVol){
            Coor3d clustCent;
            siteAverage(clusters[i], clustCent);
            if(clustCent.dist2(aveKeyResCoor)<curDist2){
                index=i;
                curDist2=clustCent.dist2(aveKeyResCoor);
            }
        }        
    }
    if(index==-1) index=0;
    
    this->siteIndex=index;
    volume=this->clusters[index].size()*spacing3;
    siteCentroid(this->clusters[index], dockDim, centroid); 
    return true;
}

void Grid::siteAverage(std::vector<Coor3d*>& clust, Coor3d& aveCoor) {
    aveCoor.set(0,0,0);
    for(unsigned i=0; i<clust.size(); ++i){
        Coor3d *pCoor=clust[i];
        aveCoor=aveCoor+(*pCoor);
    }
    aveCoor=aveCoor/clust.size();  
}


void Grid::siteCentroid(std::vector<Coor3d*>& clust, Coor3d& dockDim, Coor3d& centroid){
    //Make sure initialized with zero;
    centroid.set(0,0,0);
    for(unsigned i=0; i<clust.size(); ++i){
        Coor3d *pCoor=clust[i];
        centroid=centroid+(*pCoor);
    }
    centroid=centroid/clust.size();    

    // Determine dimensions of cluster
    double xMax=BIGNEGTIVE;
    double yMax=BIGNEGTIVE;
    double zMax=BIGNEGTIVE;
    
    double xMin=BIGPOSITIVE;
    double yMin=BIGPOSITIVE;
    double zMin=BIGPOSITIVE;
       
    for(unsigned i=0; i< clust.size(); ++i){
        Coor3d* pCoor=clust[i];
        if(pCoor->getX()>xMax) xMax=pCoor->getX();
        if(pCoor->getY()>yMax) yMax=pCoor->getY();
        if(pCoor->getZ()>zMax) zMax=pCoor->getZ();
        
        if(pCoor->getX()<xMin) xMin=pCoor->getX();
        if(pCoor->getY()<yMin) yMin=pCoor->getY();
        if(pCoor->getZ()<zMin) zMin=pCoor->getZ();        
    }
    
    double xDim=xMax-xMin;
    double yDim=yMax-yMin;
    double zDim=zMax-zMin;
    
    double xDock=xDim+4;
    double yDock=yDim+4;
    double zDock=zDim+4;
    
    //if(xDock<22) xDock=22;
    //if(yDock<22) yDock=22;
    //if(zDock<22) zDock=22;    
    
    dockDim.set(xDock, yDock, zDock);
}


void Grid::toOutput(){
    std::string gridFileName="site.txt";
    std::ofstream gridFile;
    try {
        gridFile.open(gridFileName.c_str());
    }
    catch(...){
        std::string mesg="Grid::toOutput()\n\t Cannot open site.txt file: "+gridFileName;
        throw LBindException(mesg);
    }   

    gridFile << ss.str() << std::endl;  
    
    gridFile.close();
}

} //namespace LBIND
