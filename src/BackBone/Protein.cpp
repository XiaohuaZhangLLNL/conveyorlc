/* 
 * File:   Protein.cpp
 * Author: zhang30
 * 
 * Created on December 6, 2011, 8:32 AM
 */

#include <cstdlib>
#include <iostream>

#include "Protein.h"

namespace LBIND {
    
Protein::Protein() : Base() {
}

Protein::Protein(int idVal, std::string nameVal) : Base(idVal, nameVal) {
}

Protein::Protein(const Protein& orig) {
}

Protein::~Protein() {
}

void Protein::downloadPDB(std::string pdbid){
    std::string gzSuffix=".pdb.gz";
    std::string gzfile=pdbid+gzSuffix;
    std::string cmd="wget --no-check-certificate http://www.rcsb.org/pdb/files/"+gzfile;
    system(cmd.c_str());
    cmd="gunzip -f "+gzfile;
    system(cmd.c_str());
}

void Protein::getLigandPDB(std::string ligname){
    std::string cmd="grep " + ligname + " "+ pdbID +".pdb > "+pdbID +"-lig-"+ligname+".pdb";
    system(cmd.c_str());
}

void Protein::docking(){
    std::cout << "docking to be implement" << std::endl;
}

void Protein::scoring(){
    std::cout << "scoring to be implement" << std::endl;
}
void Protein::mmGBSA(){
    std::cout << "mmGBSA to be implement" << std::endl;
}
void Protein::singlePtMM(){
    std::cout << "singlePtMM to be implement" << std::endl;
}
void Protein::flexDocking(){
    std::cout << "flexDocking to be implement" << std::endl;
}

void Protein::setPDBID(std::string pdbid){
    this->pdbID=pdbid;
}

std::string Protein::getPDBID(){
    return this->pdbID;
}

void Protein::addLigand(Ligand* pLigand){
    this->ligands.push_back(pLigand);
}

void Protein::getLigands(std::vector<Ligand*>& ligList){
    ligList=this->ligands;
}

} //namespace LBIND 


