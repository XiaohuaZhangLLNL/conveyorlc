/* 
 * File:   VinaLC.cpp
 * Author: zhang30
 * 
 * Created on August 17, 2012, 1:32 PM
 */

#include "VinaLC.h"

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>

#include "Structure/Sstrm.hpp"
#include "Structure/Coor3d.h"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"

namespace LBIND {

VinaLC::VinaLC() {
}

VinaLC::VinaLC(const VinaLC& orig) {
}

VinaLC::~VinaLC() {
}

void VinaLC::centroid(std::string& sumFile, Coor3d& center){
    std::ifstream inFile;
    try {
        inFile.open(sumFile.c_str());
    }
    catch(...){
        std::cout << "PreVinaLC::centroid >> Cannot open CSA sum file" << sumFile << std::endl;
    }    
    
    std::string fileLine="";
    
    const std::string centrStr="centroid:";

    while(std::getline(inFile, fileLine)){
        
        if(fileLine.compare(0,9, centrStr)==0){
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens); 
            double coords[3];
            
            if(tokens.size()!=4){
                std::cerr << "Warning: the centeroid format is wrong!" <<std::endl;
                return;
            }
            
            for(int i=0;i<3;i++){
                coords[i]=Sstrm<double, std::string>(tokens[i+1]);
            }
            
            center.set(coords[0], coords[1], coords[2]);
                      
            return;
        }        
    }    
    
    std::cerr << "Warning: the centeroid coordinates do NOT exist!" <<std::endl;
    return;
    
}

void VinaLC::dms(std::string& cutPdbFile, std::string& surfFile){
    
    const double radius=1.5; // minimum sphere radius
       
    std::string cmd="dms "+cutPdbFile+" -o "+surfFile+" -n -w "+Sstrm<std::string,double>(radius)+" > dms.log";
    system(cmd.c_str());
}

void VinaLC::sphgen(std::string& surfFile, std::string& sphFile){
    
    std::string input="INSPH";
    std::ofstream sphgenFile;
    try {
        sphgenFile.open(input.c_str());
    }
    catch(...){
        std::string mesg="PreVinaLC::sphgen()\n\t Cannot open sphgen input file: "+input;
        throw LBindException(mesg);
    }   
       
    sphgenFile << surfFile << std::endl;
    sphgenFile << "R" << std::endl;
    sphgenFile << "X" << std::endl;
    sphgenFile << "0.0" << std::endl;
    sphgenFile << "4.0" << std::endl;  //  max sphere radius
    sphgenFile << "1.5" << std::endl;  //  min sphere radius
    sphgenFile << sphFile << std::endl;

    sphgenFile.close();    
    
    std::string cmd="sphgen > sphgen";
    system(cmd.c_str());    
}

void VinaLC::sphere_selector(std::string& sphFile, std::string& selSphFile, Coor3d& center){

//    std::clock_t start;
//    double duration;
//    start = std::clock();
    
    std::ifstream sphgenFile;
    try {
        sphgenFile.open(sphFile.c_str());
    }
    catch(...){
        std::string mesg="PreVinaLC::sphgen()\n\t Cannot open sphgen input file: "+sphFile;
        throw LBindException(mesg);
    } 
    
    std::string fileLine="";    
    const std::string clusterStr="cluster";
    
    const double maxDist=15;
    
    double lowestDist=9999;

    std::vector<std::vector<SphDat> > collections;
    std::vector<SphDat> vecSphDat;
    
    int tmpClustID;
    int minClustID;
    int minCount;
    
    int count=0;
    
    bool clustFlag=false;
            
    while(std::getline(sphgenFile, fileLine)){
    
        if(fileLine.compare(0,7, clusterStr)==0){
            clustFlag=true;
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);
            tmpClustID=Sstrm<int, std::string>(tokens[1]);            
            if(tmpClustID==0) break; //cluster 0 is the sum of all spheres, a superset rather than a true cluster
            
            collections.push_back(vecSphDat);
            vecSphDat.clear();
            count++;            
            
        }else if(clustFlag){
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);
            double x=atof(tokens[1].c_str());
            double y=atof(tokens[2].c_str());
            double z=atof(tokens[3].c_str());
            
            double distx=x-center.getX();
            double disty=y-center.getY();
            double distz=z-center.getZ();
            
            double tmpDist=sqrt(distx*distx+disty*disty+distz*distz)-atof(tokens[4].c_str());
            
            if(tmpDist<lowestDist){
                lowestDist=tmpDist;
                minClustID=tmpClustID;
                minCount=count;
            }
            
            if(tmpDist<maxDist){
                SphDat tmpDat;
                tmpDat.line=fileLine;
                tmpDat.dist=tmpDist;
                vecSphDat.push_back(tmpDat);
            }
            
        }
        
    }
    
    sphgenFile.close();
    
//    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
//    std::cout<<"time: "<< duration <<'\n';
  
//    std::cout << "minClustID=" << minClustID << " minCount=" << minCount << std::endl;
    //analyze distance distribution 
    //do histogram analysis
    
    std::vector<SphDat> curLines=collections[minCount];

    std::vector<double> hist;
    
    int i=0;
    while(i<=int(maxDist)){
        hist.push_back(0);
        i++;
    }
    
    for(unsigned int i=0; i<curLines.size(); ++i){

        int index=int(std::fabs(curLines[i].dist));
//        std::cout << "index=" << index << "   dist=" << curLines[i].dist << std::endl;       
        hist[index]=hist[index]+1.0/(float(index+1)*float(index+1));
    }
    
    double derCutoff=0.1;

    
    int cutoff=hist.size();
    for(unsigned int i=1; i<hist.size()-1; ++i){
        double ders=hist[i]-hist[hist.size()-1];
//        std::cout << "derivative approximation: " <<  i << "ders=" << ders << std::endl;
        if (ders < derCutoff){
            cutoff=i;
            break;
        }
    }
    
    int n_selected=0;
    
    for(unsigned int i=0; i<curLines.size(); ++i){
        if(curLines[i].dist <= cutoff){
            ++n_selected;
        }
    }
    
    std::ofstream selSphgenFile;
    try {
        selSphgenFile.open(selSphFile.c_str());
    }
    catch(...){
        std::string mesg="PreVinaLC::sphgen()\n\t Cannot open sleSphgen input file: "+selSphFile;
        throw LBindException(mesg);
    }     
    
    selSphgenFile << "cluster   " << minClustID << 
            "  number of spheres in cluster  " << n_selected << std::endl;
    
    for(unsigned int i=0; i<curLines.size(); ++i){
        if(curLines[i].dist <= cutoff){
            selSphgenFile << curLines[i].line << std::endl;
        }
    }    
    
    selSphgenFile.close();
    
}

/*
void VinaLC::sphere_selector(std::string& sphFile, std::string& selSphFile, Coor3d& center){
    std::clock_t start;
    double duration;
    start = std::clock();
    std::ifstream sphgenFile;
    try {
        sphgenFile.open(sphFile.c_str());
    }
    catch(...){
        std::string mesg="PreVinaLC::sphgen()\n\t Cannot open sphgen input file: "+sphFile;
        throw LBindException(mesg);
    } 
    
    std::string fileLine="";    
    const std::string clusterStr="cluster";
    
    const double maxDist=15;
    
    double lowestDist=9999;
    double tmpDist=9999;
    
    int curClustID=0;
    int tmpClustID=0;
       
    std::vector<SphDat> curLines;
    std::vector<SphDat> tmpLines;
//    typedef std::vector<SphDat>::iterator SphDatIterator;
    
    bool clustFlag=false;
    bool saveFlag=false;
            
    while(std::getline(sphgenFile, fileLine)){
    
        if(fileLine.compare(0,7, clusterStr)==0){
            clustFlag=true;
            saveFlag=false;
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);
            tmpClustID=Sstrm<int, std::string>(tokens[1]);
            tmpLines.clear();
            
            if(tmpClustID==0) break; //cluster 0 is the sum of all spheres, a superset rather than a true cluster
            
        }else if(clustFlag){
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens);
            double x=atof(tokens[1].c_str());
            double y=atof(tokens[2].c_str());
            double z=atof(tokens[3].c_str());
            
            double distx=x-center.getX();
            double disty=y-center.getY();
            double distz=z-center.getZ();
            
            tmpDist=sqrt(distx*distx+disty*disty+distz*distz)-atof(tokens[4].c_str());
            
            if(tmpDist<lowestDist){
                saveFlag=true;
                curLines.clear();
//                std::copy<SphDatIterator, SphDatIterator>(tmpLines.begin(), tmpLines.end(), curLines.begin());
                for(unsigned int i=0; i <tmpLines.size(); ++i ){
                    SphDat tmpDat=tmpLines[i];
                    curLines.push_back(tmpDat);
                }
                lowestDist=tmpDist;
                curClustID=tmpClustID;
            }
            
            if(tmpDist<maxDist){
                SphDat tmpDat;
                tmpDat.line=fileLine;
                tmpDat.dist=tmpDist;
                
                if(saveFlag){                    
                    curLines.push_back(tmpDat);
                }
                tmpLines.push_back(tmpDat);
            }
            
        }
        
    }
    
    sphgenFile.close();

    duration = ( std::clock() - start ) ;
    std::cout<<"time: "<< duration <<'\n';
      
    //analyze distance distribution 
    //do histogram analysis

    std::vector<double> hist;
    
    int i=0;
    while(i<=int(maxDist)){
        hist.push_back(0);
        i++;
    }
    
    for(unsigned int i=0; i<curLines.size(); ++i){

        int index=int(std::fabs(curLines[i].dist));
//        std::cout << "index=" << index << "   dist=" << curLines[i].dist << std::endl;       
        hist[index]=hist[index]+1.0/(float(index+1)*float(index+1));
    }
    
    double derCutoff=0.1;

    
    int cutoff=hist.size();
    for(unsigned int i=1; i<hist.size()-1; ++i){
        double ders=hist[i]-hist[hist.size()-1];
//        std::cout << "derivative approximation: " <<  i << "ders=" << ders << std::endl;
        if (ders < derCutoff){
            cutoff=i;
            break;
        }
    }
    
    int n_selected=0;
    
    for(unsigned int i=0; i<curLines.size(); ++i){
        if(curLines[i].dist <= cutoff){
            ++n_selected;
        }
    }
    
    std::ofstream selSphgenFile;
    try {
        selSphgenFile.open(selSphFile.c_str());
    }
    catch(...){
        std::string mesg="PreVinaLC::sphgen()\n\t Cannot open sleSphgen input file: "+selSphFile;
        throw LBindException(mesg);
    }     
    
    selSphgenFile << "cluster   " << curClustID << 
            "  number of spheres in cluster  " << n_selected << std::endl;
    
    for(unsigned int i=0; i<curLines.size(); ++i){
        if(curLines[i].dist <= cutoff){
            selSphgenFile << curLines[i].line << std::endl;
        }
    }    
    
    selSphgenFile.close();
    
}
  
*/

void VinaLC::getGridDims(std::string& selSphFile, Coor3d& gridDims){
    std::ifstream selSphgenFile;
    try {
        selSphgenFile.open(selSphFile.c_str());
    }
    catch(...){
        std::string mesg="AutoDock::gridDims()\n\t Cannot open sphere_selector output file: "+selSphFile;
        throw LBindException(mesg);
    }    
    
    const std::string firststr="DOCK";
    const std::string secondstr="cluster";
    std::string fileLine="";
 
    double xMin= 100000000000.0;
    double xMax=-100000000000.0;
    double yMin= 100000000000.0;
    double yMax=-100000000000.0;
    double zMin= 100000000000.0;
    double zMax=-100000000000.0;
        
    while(std::getline(selSphgenFile, fileLine)){
        if(fileLine.compare(0,4, firststr)!=0 && fileLine.compare(0,7, secondstr)!=0){
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens); 
            if(tokens.size()==8){
                double radius=std::fabs(atof(tokens[4].c_str()));
                double x=atof(tokens[1].c_str());
                double xMinTmp=x-radius;
                double xMaxTmp=x+radius;

                double y=atof(tokens[2].c_str());
                double yMinTmp=y-radius;
                double yMaxTmp=y+radius;
                
                double z=atof(tokens[3].c_str());
                double zMinTmp=z-radius;
                double zMaxTmp=z+radius;
                
                if(xMinTmp<xMin) xMin=xMinTmp;
                if(yMinTmp<yMin) yMin=yMinTmp;
                if(zMinTmp<zMin) zMin=zMinTmp;
            
                if(xMaxTmp>xMax) xMax=xMaxTmp;
                if(yMaxTmp>yMax) yMax=yMaxTmp;
                if(zMaxTmp>zMax) zMax=zMaxTmp;
            }
        }
    }    
    
    selSphgenFile.close();   

//    std::cout << "xMax=" << xMax << "yMax=" << yMax << "zMax=" << zMax << std::endl;
//    std::cout << "xMin=" << xMin << "yMin=" << yMin << "zMin=" << zMin << std::endl;
    double xDimen=xMax-xMin+10;
    double yDimen=yMax-yMin+10;
    double zDimen=zMax-zMin+10;
    
    // Limit the dimension to be less than 100.0 angstroms.
    if(xDimen>100.0) xDimen=45.0;
    if(yDimen>100.0) yDimen=45.0;
    if(zDimen>100.0) zDimen=45.0;
    
    gridDims.set(xDimen,yDimen,zDimen);
    
    return;
}

void VinaLC::vinalcGridDims(std::string& cutPdbFile, Coor3d& center, Coor3d& gridDims){
    // make sure all the outputs of sphgen is deleted.
    std::string cmd="rm -f INSPH OUTSPH *.sph *.surf ";
    system(cmd.c_str());

    std::string baseName=cutPdbFile.substr(0,(cutPdbFile.size()-4));
    std::string surfFile=baseName+".surf";   
    dms(cutPdbFile,surfFile);
    
    std::string sphFile=baseName+".sph";
    sphgen(surfFile,sphFile);
    
    std::string selSphFile=baseName+"_select.sph";
    sphere_selector(sphFile, selSphFile, center);
    
    getGridDims(selSphFile, gridDims);
    
    return;
    
}



}//namespace LBIND

