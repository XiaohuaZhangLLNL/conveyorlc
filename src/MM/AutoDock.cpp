/* 
 * File:   AutoDock.cpp
 * Author: zhang30
 * 
 * Created on January 9, 2012, 9:55 AM
 */

#include <vector>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>

#include "AutoDock.h"
#include "BackBone/Protein.h"
#include "BackBone/Ligand.h"
#include "Common/Tokenize.hpp"
#include "Common/LBindException.h"
#include "Amber.h"
#include "Structure/Pdb.h"
#include "Structure/Complex.h"
#include "Structure/Molecule.h"
#include "Structure/Atom.h"
#include "Structure/Coor3d.h"
#include "Structure/Fragment.h"

namespace LBIND {

AutoDock::AutoDock() {
    AUTODOCKPATH=getenv("AUTODOCKPATH");
}

AutoDock::AutoDock(Protein* pProt) : pProtein(pProt) {
    AUTODOCKPATH=getenv("AUTODOCKPATH");
}

AutoDock::AutoDock(const AutoDock& orig) {
}

AutoDock::~AutoDock() {
}

void AutoDock::run() {
    this->prepLigands();
}

void AutoDock::prepLigands(){

    std::string pdbid=pProtein->getPDBID();
    std::string pdbFName=pdbid+".pdb";
    // ! try to get active site geometry
    Amber* pAmber=new Amber(pProtein);
    std::string input=pdbid+".pdb";
    std::string output=pdbid+"-noH.pdb";
    std::string options="-Trim";
    pAmber->reduce(input, output, options);  
    //! get rid of connnectivity.
    input="STI.pdb";
    output="STI-noC.pdb";    
    std::string cmd="grep -v CONECT " + input +" > " +output;
    system(cmd.c_str());
    input="STI-noC.pdb";
    output="STI-noH.pdb";
    pAmber->reduce(input, output, options);
    input="STI-noH.pdb";
    output="STI-H.pdb";
    options="";
    pAmber->reduce(input, output, options);
    input="STI-H.pdb";
    output="STI.mol2";
    options=" -c gas -s 0";
    pAmber->antechamber(input, output, options);
    delete pAmber;
    cmd="rm -f rec.ms INSPH OUTSPH rec.sph selected_spheres.sph";
    system(cmd.c_str());
    this->dms(pdbid);
    this->sphgen();
    this->sphere_selector("STI");
    this->actSiteDimens();
    this->ligandCenter("STI");
    // ! end of get active site geometry
    this->prepareReceptor(pdbFName); 
    
    std::vector<Ligand*> ligList;
    pProtein->getLigands(ligList);    
    for(unsigned i=0; i<ligList.size(); ++i){
        std::string ligname=ligList[i]->getName();
        std::string ligFBase=pdbid+"-lig-"+ligname;
        std::string cmd="mkdir -p "+ligFBase;
        system(cmd.c_str());        
        chdir(ligFBase.c_str()); 
        std::string ligandFName=ligname+".pdb";
        this->prepareLigand(pdbid, ligandFName);
        this->prepVinaInput(pdbid, ligname);
        this->runVina();
        
        chdir("..");
    }
}

void AutoDock::prepareLigand(std::string pdbid, std::string ligandFName){
    std::string cmd_temp="cp ../"+pdbid+"-"+ligandFName+" "+ligandFName;
    system(cmd_temp.c_str()); 
    //! prepare_ligand4.py -l ligand_filename -A hydrogens    
    std::string cmd=AUTODOCKPATH +"/prepare_ligand4.py -l "+ligandFName+" -A hydrogens"; 
    system(cmd.c_str());    
}

void AutoDock::prepareReceptor(std::string pdbFName){
    //! Assue PDB file name suffix is .pdb
    std::string pdbFNbase=pdbFName.substr(0,pdbFName.size()-4);
    //! prepare_receptor4.py -r pdb_file -o outputfilename -A checkhydrogens    
    std::string cmd=AUTODOCKPATH +"/prepare_receptor4.py -r "+pdbFName+" -o " +pdbFNbase+".pdbqt -A checkhydrogens"; 
    system(cmd.c_str());     
}


// Determine the geometry of active site
// dms rec_noH.pdb -n -w 1.4 -v -o rec-n.ms
// sphgen
// sphere_selector rec.sph lig_charged.mol2 10.0
//void AutoDock::deprotonReceptor(std::string pdbFName){
//    //! Assue PDB file name suffix is .pdb
//    std::string pdbFNbase=pdbFName.substr(0,pdbFName.size()-4);    
//    std::string cmd=AUTODOCKPATH +"/reduce -Trim "+pdbFName+" > "+pdbFNbase+"-noH.pdb";
//}


//void AutoDock::antechamber(std::string pdbFName){
//    std::string pdbFNbase=pdbFName.substr(0,pdbFName.size()-4);
//    // ! antechamber -i sustiva_new.pdb -fi pdb -o sustiva.mol2 -fo mol2 -c bcc -s 2
//    std::string cmd=AUTODOCKPATH +"/antechamber -i " +pdbFName + " -fi pdb -o "
//            +pdbFNbase + ".mol2 -fo mol2 -c bcc -s 2"; 
//    std::cout << cmd << std::endl;
//    system(cmd.c_str());     
//}

void AutoDock::dms(std::string pdbid){
    std::string cmd=AUTODOCKPATH +"/dms "+pdbid+"-noH.pdb -n -w 1.4 -v -o rec.ms";
    system(cmd.c_str());
}

void AutoDock::sphgen(){
    
    std::string input="INSPH";
    std::ofstream sphgenFile;
    try {
        sphgenFile.open(input.c_str());
    }
    catch(...){
        std::string mesg="AutoDock::sphgen()\n\t Cannot open sphgen input file: "+input;
        throw LBindException(mesg);
    }   
       
    sphgenFile << "rec.ms" << std::endl;
    sphgenFile << "R" << std::endl;
    sphgenFile << "X" << std::endl;
    sphgenFile << "0.0" << std::endl;
    sphgenFile << "4.0" << std::endl;
    sphgenFile << "1.4" << std::endl;
    sphgenFile << "rec.sph" << std::endl;

    sphgenFile.close();    
    
    std::string cmd=AUTODOCKPATH +"/sphgen";
    system(cmd.c_str());    
}

void AutoDock::sphere_selector(std::string ligname){
    std::string cmd=AUTODOCKPATH +"/sphere_selector rec.sph "+ligname+".mol2 10.0";
    system(cmd.c_str());     
}

void AutoDock::actSiteDimens(){
    //! Analysis the output
    std::string selSphsFName="selected_spheres.sph";
    std::ifstream selSphsFile;
    try {
        selSphsFile.open(selSphsFName.c_str());
    }
    catch(...){
        std::string mesg="AutoDock::actSiteDimens()\n\t Cannot open sphere_selector output file: "+selSphsFName;
        throw LBindException(mesg);
    }    
    
    const std::string firststr="DOCK";
    const std::string secondstr="cluster";
    std::string fileLine="";
    std::vector<double> xCoords;
    std::vector<double> yCoords;
    std::vector<double> zCoords;
    
    while(std::getline(selSphsFile, fileLine)){
        if(fileLine.compare(0,4, firststr)!=0 && fileLine.compare(0,7, secondstr)!=0){
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens); 
            if(tokens.size()==8){
                xCoords.push_back(atof(tokens[1].c_str()));
                yCoords.push_back(atof(tokens[2].c_str()));
                zCoords.push_back(atof(tokens[3].c_str()));
            }
        }
    }    

    double xMax=*( std::max_element( xCoords.begin(), xCoords.end() ) );
    double yMax=*( std::max_element( yCoords.begin(), yCoords.end() ) );
    double zMax=*( std::max_element( zCoords.begin(), zCoords.end() ) );
    
    double xMin=*( std::min_element( xCoords.begin(), xCoords.end() ) );
    double yMin=*( std::min_element( yCoords.begin(), yCoords.end() ) );
    double zMin=*( std::min_element( zCoords.begin(), zCoords.end() ) );    
    
    std::cout << "xMax=" << xMax << "yMax=" << yMax << "zMax=" << zMax << std::endl;
    std::cout << "xMin=" << xMin << "yMin=" << yMin << "zMin=" << zMin << std::endl;
    actSiteSize.set((xMax-xMin),(yMax-yMin),(zMax-zMin));
    double init=0.0;
    int vecSize=xCoords.size();
    double xMean = std::accumulate(xCoords.begin(), xCoords.end(), init)/vecSize;
    double yMean = std::accumulate(yCoords.begin(), yCoords.end(), init)/vecSize;
    double zMean = std::accumulate(zCoords.begin(), zCoords.end(), init)/vecSize;

    std::cout << "xMean=" << xMean << "yMean=" << yMean << "zMean=" << zMean << std::endl;
    actSiteCenter.set(xMean,yMean,zMean);
    
    
}

void AutoDock::ligandCenter(std::string ligname){
    Complex* pComplex=new Complex();
    std::string pdbFName=ligname+".pdb";
    Pdb* pPdb=new Pdb();
    pPdb->parse(pdbFName, pComplex);

    Coor3d sumCoor(0,0,0);
    int count=0;
    std::vector<Molecule*> molList=pComplex->getChildren();
    for(unsigned int m=0; m<molList.size(); ++m){
        Molecule* pMolecule=molList[m];
        std::vector<Fragment*> fragList=pMolecule->getChildren();
        for(unsigned int f=0; f<fragList.size(); ++f){
            std::vector<Atom*> atomList=fragList[f]->getChildren();
            for(unsigned int j=0; j < atomList.size(); ++j){
                Atom* pAtom=atomList[j];
                Coor3d* pCoor=pAtom->getCoords();
                sumCoor=sumCoor+(*pCoor);
                ++count;
            }
        }
    }
    
    if(count>0) sumCoor=sumCoor/count;
    
    
    std::cout <<"Center of ligand: "<< sumCoor  << " count="<< count<< std::endl;
    delete pPdb;
    delete pComplex;
    
}

void AutoDock::prepVinaInput(std::string pdbid, std::string ligName){
    std::string input="vina.input";
    std::ofstream vinaFile;
    try {
        vinaFile.open(input.c_str());
    }
    catch(...){
        std::string mesg="AutoDock::prepVinaInput(std::string pdbid, std::string ligName)\n\t Cannot open Vina input file: "+input;
        throw LBindException(mesg);
    }   
       
    vinaFile << "receptor = ../"+pdbid+".pdbqt" << std::endl;
    vinaFile << "ligand = "+ligName+".pdbqt" << std::endl;
    vinaFile << "out = "+pdbid+"-"+ligName+"-all.pdbqt" << std::endl;
    vinaFile << "center_x=" << actSiteCenter.getX() << std::endl;
    vinaFile << "center_y=" << actSiteCenter.getY() << std::endl;
    vinaFile << "center_z=" << actSiteCenter.getZ() << std::endl;
    int xSize=static_cast<int>(actSiteSize.getX()+2);
    int ySize=static_cast<int>(actSiteSize.getY()+2);
    int zSize=static_cast<int>(actSiteSize.getZ()+2);
    vinaFile << "size_x = " << xSize << std::endl;
    vinaFile << "size_y = " << ySize << std::endl;
    vinaFile << "size_z = " << zSize << std::endl;
    vinaFile << "exhaustiveness = 8" << std::endl;
   
    vinaFile.close();
    
}

void AutoDock::runVina(){
    std::string input="vina.input";
    std::string cmd=AUTODOCKPATH +"/vina --config "+input+" --log vina.log"; 
    system(cmd.c_str());     
}
    
    
}//namespace LBIND 

