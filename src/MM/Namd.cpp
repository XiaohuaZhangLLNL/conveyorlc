/* 
 * File:   Namd.cpp
 * Author: zhang30
 * 
 * Created on December 15, 2011, 5:39 PM
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "Namd.h"
#include "BackBone/Protein.h"
#include "Common/LBindException.h"
#include "Common/Tokenize.hpp"

#include <boost/regex.hpp>

namespace LBIND {

Namd::Namd() {
    VMDEXEPATH=getenv("VmdDir");    
    NAMDEXEPATH=getenv("NamdDir");
    dataPath=getenv("LBindData");        
}

Namd::Namd(Protein* pProt) : pProtein(pProt){
    VMDEXEPATH=getenv("VmdDir");    
    NAMDEXEPATH=getenv("NamdDir");
    dataPath=getenv("LBindData");
}

Namd::Namd(const Namd& orig) {
}

Namd::~Namd() {
}

void Namd::run(){
    std::string pdbid=pProtein->getPDBID();
    this->psfgen(pdbid);
    this->solvate(pdbid);
    this->minimization(pdbid);
//    this->md(pdbid);
}

void Namd::psfgen(std::string pdbid){
    std::string vmdFName=pdbid+".tcl";
    std::ofstream vmdFile;
    try {
        vmdFile.open(vmdFName.c_str());
    }
    catch(...){
        std::string mesg="Namd::psfgen(std::string pdbid)\n\t Cannot open VMD file: "+vmdFName;
        throw LBindException(mesg);
    }     
    
    vmdFile << "# Save only protein structure" << std::endl;    
    vmdFile << "mol new "<<pdbid<<".pdb" << std::endl;
    vmdFile << "set prot [atomselect top protein]" << std::endl;
    vmdFile << "$prot writepdb " << pdbid << "-p.pdb" << std::endl;
    vmdFile.close();
    
    std::string cmd=VMDEXEPATH +"/vmd -dispdev text -eofexit < "+vmdFName;
    system(cmd.c_str());

    try {
        vmdFile.open(vmdFName.c_str());
    }
    catch(...){
        std::string mesg="Namd::psfgen(std::string pdbid)\n\t Cannot re-open VMD file: "+vmdFName;
        throw LBindException(mesg);
    }   
    
    std::string charmmTop=dataPath+"/charmm/top_all27_prot_lipid.inp";
    
    vmdFile << "# psfgen to generate PSF and PDB" << std::endl;    
    vmdFile << "mol new "<<pdbid << "-p.pdb" << std::endl;
    vmdFile << "package require psfgen" << std::endl;
    vmdFile << "topology " << charmmTop << std::endl;
    vmdFile << "pdbalias residue HIS HSE" << std::endl;
    vmdFile << "pdbalias atom ILE CD1 CD" << std::endl;
    vmdFile << "segment U {pdb "<< pdbid<<"-p.pdb}" << std::endl;
    vmdFile << "coordpdb "<< pdbid<<"-p.pdb U" << std::endl;
    vmdFile << "guesscoord" << std::endl;
    vmdFile << "writepdb "<< pdbid<<"-nowat.pdb" << std::endl;
    vmdFile << "writepsf "<< pdbid<<"-nowat.psf" << std::endl;
    vmdFile.close();
    
    system(cmd.c_str());
            
}

void Namd::solvate(std::string pdbid){
    double boxCutOff=5;
    
    std::string vmdFName=pdbid+".tcl";
    std::ofstream vmdFile;
    try {
        vmdFile.open(vmdFName.c_str());
    }
    catch(...){
        std::string mesg="Namd::solvate(std::string pdbid)\n\t Cannot open VMD file: "+vmdFName;
        throw LBindException(mesg);
    }  
    
//    vmdFile << "# Define move function:" << std::endl;
//    vmdFile << "proc moveby {sel offset} {" << std::endl;
//    vmdFile << "  foreach coord [$sel get {x y z}] {" << std::endl;
//    vmdFile << "    lappend newcoords [vecadd $coord $offset]" << std::endl;
//    vmdFile << "  }" << std::endl;
//    vmdFile << "  $sel set {x y z} $newcoords" << std::endl;
//    vmdFile << "}    " << std::endl;
    vmdFile << "# Solve protein in water box" << std::endl;
    vmdFile << "package require solvate" << std::endl;
    vmdFile << "solvate "<< pdbid<<"-nowat.psf "<< pdbid<<"-nowat.pdb -t " << boxCutOff << " -o "<< pdbid<<"-wb" << std::endl;
    vmdFile << "# Obtain geometry of water box" << std::endl;
    vmdFile << "set everyone [atomselect top all]" << std::endl;
//    vmdFile << "set movevec [vecscale -1.0 [measure center $everyone]]" << std::endl;
//    vmdFile << "moveby $everyone $movevec" << std::endl;
    vmdFile << "puts \"WATER BOX GEOMETRY:\"" << std::endl;
    vmdFile << "measure minmax $everyone" << std::endl;
    vmdFile << "measure center $everyone" << std::endl;
//    vmdFile << "writepdb "<< pdbid<<"-wbcent.pdb" << std::endl;    
    vmdFile.close();
    
    std::string cmd=VMDEXEPATH +"/vmd -dispdev text -eofexit < "+vmdFName+" > "+pdbid+"-wb.log";
    system(cmd.c_str());    
    
    // Parser the water geometry demension
    std::string wblogFName=pdbid+"-wb.log";
    std::ifstream wblogFile;
    try {
        wblogFile.open(wblogFName.c_str());
    }
    catch(...){
        std::string mesg="Namd::solvate(std::string pdbid)\n\t Cannot open VMD log file: "+wblogFName;
        throw LBindException(mesg);
    }      
    
//    static const boost::regex headRegex("^WATER BOX GEOMETRY:");
//    static const boost::regex floatRegex("([-]\\d+[.\\d+])");
//    boost::smatch what;
    
    const std::string headstr="WATER BOX GEOMETRY:";
    std::string fileLine="";    
    
    while(wblogFile){
        std::getline(wblogFile, fileLine);
        if(fileLine.compare(0,19, headstr)==0){
            std::getline(wblogFile, fileLine);
            std::cout << fileLine << std::endl;
//            std::replace( fileLine.begin(), fileLine.end(), '{', ' ');
//            std::replace( fileLine.begin(), fileLine.end(), '}', ' ');
//            std::cout << fileLine << std::endl;
            
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens, " {}");     
            
            std::vector<double> dims;
            for(unsigned i=0; i<tokens.size(); ++i){
//                std::cout << i << "=" << tokens[i] << std::endl;
                dims.push_back(atof(tokens[i].c_str()));
            }             
            

            if(dims.size()!=6){
                std::string mesg="Namd::solvate(std::string pdbid)\n\t Water Box Geometry Parser Error, Should have 6 numbers. ";
                throw LBindException(mesg);
            }
            double xdim=dims[3]-dims[0];
            double ydim=dims[4]-dims[1];
            double zdim=dims[5]-dims[2];
            watBoxSize.set(xdim, ydim, zdim);
            std::cout <<"Water Box Size: X="<< xdim << " Y=" << ydim << " Z=" << zdim << std::endl;            

            std::getline(wblogFile, fileLine);
            std::cout << fileLine << std::endl;

            tokens.clear();
            tokenize(fileLine, tokens);     
            
            dims.clear();
            for(unsigned i=0; i<tokens.size(); ++i){
                dims.push_back(atof(tokens[i].c_str()));
            }              

            if(dims.size()!=3){
                std::string mesg="Namd::solvate(std::string pdbid)\n\t Water Box Geometry Parser Error, Should have 3 numbers. ";
                throw LBindException(mesg);
            }

            watBoxCenter.set(dims[0], dims[1], dims[2]);
            std::cout << "Water Box Center: X="<< dims[0] << " Y=" << dims[1] << " Z=" << dims[2] << std::endl;             
           
            break;
        }    
    }
   
}

//void Namd::tokenize(std::string inputStr, std::vector<double>& tokens){
//        std::istringstream iss(inputStr);
//        std::vector<std::string> strs;
//        std::copy(std::istream_iterator<std::string > (iss),
//                std::istream_iterator<std::string > (),
//                std::back_inserter<std::vector<std::string> >(strs));
//
//        for(unsigned i=0; i<strs.size(); ++i){
//            std::cout << i << "=" << strs[i] << std::endl;
//            tokens.push_back(atof(strs[i].c_str()));
//        }    
//       
//}

void Namd::setupPBCinput(std::string pdbid){
    std::string confFName=pdbid+".conf";
    std::ofstream confFile;
    try {
        confFile.open(confFName.c_str());
    }
    catch(...){
        std::string mesg="Namd::setupPBCinput(std::string pdbid)\n\t Cannot open configuration file: "+confFName;
        throw LBindException(mesg);
    }      

    std::string charmmPar=dataPath+"/charmm/par_all27_prot_lipid.inp";
    
    confFile << "# Minimization and Equilibration of " << std::endl;
    confFile << "# Ubiquitin in a Water Box" << std::endl;
    confFile << "structure          "<< pdbid<<"-wb.psf" << std::endl;
    confFile << "coordinates        "<< pdbid<<"-wb.pdb" << std::endl;
    confFile << "set temperature    310" << std::endl;
    confFile << "set outputname     "<< pdbid<<"-wbeq" << std::endl;
    confFile << "firsttimestep      0" << std::endl;
    confFile << "# Input" << std::endl;
    confFile << "paraTypeCharmm      on" << std::endl;
    confFile << "parameters          " << charmmPar << std::endl;
    confFile << "temperature         $temperature" << std::endl;
    confFile << "# Force-Field Parameters" << std::endl;
    confFile << "exclude             scaled1-4" << std::endl;
    confFile << "1-4scaling          1.0" << std::endl;
    confFile << "cutoff              12.0" << std::endl;
    confFile << "switching           on" << std::endl;
    confFile << "switchdist          10.0" << std::endl;
    confFile << "pairlistdist        14.0" << std::endl;
    confFile << "# Integrator Parameters" << std::endl;
    confFile << "timestep            2.0  ;# 2fs/step" << std::endl;
    confFile << "rigidBonds          all  ;# needed for 2fs steps" << std::endl;
    confFile << "nonbondedFreq       1" << std::endl;
    confFile << "fullElectFrequency  2" << std::endl;
    confFile << "stepspercycle       10" << std::endl;
    confFile << "# Constant Temperature Control" << std::endl;
    confFile << "langevin            on    ;# do langevin dynamics" << std::endl;
    confFile << "langevinDamping     1     ;# damping coefficient (gamma) of 1/ps" << std::endl;
    confFile << "langevinTemp        $temperature" << std::endl;
    confFile << "langevinHydrogen    off    ;# don't couple langevin bath to hydrogens" << std::endl;
    confFile << "# Periodic Boundary Conditions" << std::endl;
    confFile << "cellBasisVector1    " << watBoxSize.getX() << "    0.   0.0" << std::endl;
    confFile << "cellBasisVector2     0.0  "<< watBoxSize.getY() <<"   0.0" << std::endl;
    confFile << "cellBasisVector3     0.0    0   " << watBoxSize.getZ() << std::endl;
    confFile << "cellOrigin          " << watBoxCenter.getX() << " "  << watBoxCenter.getY() << " " << watBoxCenter.getZ() << std::endl;
    confFile << "wrapAll             on" << std::endl;
    confFile << "# PME (for full-system periodic electrostatics)" << std::endl;
    confFile << "PME                 yes" << std::endl;
    confFile << "PMEGridSpacing      1.0" << std::endl;
    confFile << "#Auto grid definition" << std::endl;
    const double gridSpace=0.92;
    int xGridSize=static_cast<int>(watBoxSize.getX()/gridSpace);
    int yGridSize=static_cast<int>(watBoxSize.getY()/gridSpace);
    int zGridSize=static_cast<int>(watBoxSize.getZ()/gridSpace);
    confFile << "PMEGridSizeX        " << xGridSize << std::endl;
    confFile << "PMEGridSizeY        " << yGridSize << std::endl;
    confFile << "PMEGridSizeZ        " << zGridSize << std::endl;
    confFile << "# Constant Pressure Control (variable volume)" << std::endl;
    confFile << "useGroupPressure      yes ;# needed for rigidBonds" << std::endl;
    confFile << "useFlexibleCell       no" << std::endl;
    confFile << "useConstantArea       no" << std::endl;
    confFile << "langevinPiston        on" << std::endl;
    confFile << "langevinPistonTarget  1.01325 ;#  in bar -> 1 atm" << std::endl;
    confFile << "langevinPistonPeriod  100.0" << std::endl;
    confFile << "langevinPistonDecay   50.0" << std::endl;
    confFile << "langevinPistonTemp    $temperature" << std::endl;
    confFile << "# Output" << std::endl;
    confFile << "outputName          $outputname" << std::endl;
    confFile << "restartfreq         500     ;# 500steps = every 1ps" << std::endl;
    confFile << "dcdfreq             250" << std::endl;
    confFile << "xstFreq             250" << std::endl;
    confFile << "outputEnergies      100" << std::endl;
    confFile << "outputPressure      100" << std::endl;
    confFile << "# Minimization" << std::endl;
    confFile << "minimize            100" << std::endl;

}

void Namd::minimization(std::string pdbid){
    this->setupPBCinput(pdbid);
    std::string cmd=NAMDEXEPATH +"/namd2 "+pdbid+".conf >" + pdbid + "-min.log";
    system(cmd.c_str());  
    //Analysis.
    std::string minlogFName=pdbid+"-min.log";
    std::ifstream minlogFile;
    try {
        minlogFile.open(minlogFName.c_str());
    }
    catch(...){
        std::string mesg="Namd::minimization(std::string pdbid)\n\t Cannot open minimization log file: "+minlogFName;
        throw LBindException(mesg);
    }    
    
    const std::string headstr="ENERGY:";
    std::string fileLine="";    
    
    std::vector<double> energies;
    while(minlogFile){
        std::getline(minlogFile, fileLine);
        if(fileLine.compare(0,7, headstr)==0){    
            std::vector<std::string> tokens;
            tokenize(fileLine, tokens); 
            if(tokens.size() >12){
                energies.push_back(atof(tokens[11].c_str()));
            }
        }
    }
    minlogFile.close();
    std::cout << "The minimized total energy is: " << energies[energies.size()-1] << std::endl;
    
}

void Namd::md(std::string pdbid){
    this->setupPBCinput(pdbid);
    
    std::string confFName=pdbid+".conf";
    std::ofstream confFile;
    try {
        confFile.open(confFName.c_str(), std::ios::app);
    }
    catch(...){
        std::string mesg="Namd::solvate(std::string pdbid)\n\t Cannot append configuration file: "+confFName;
        throw LBindException(mesg);
    }     
    confFile << "reinitvels          $temperature" << std::endl;    
    confFile << "run 2500 ;# 5ps"<< std::endl;
    
    std::string cmd=NAMDEXEPATH +"/namd2 "+confFName;
    system(cmd.c_str());     
    
}

}//namespace LBIND 
