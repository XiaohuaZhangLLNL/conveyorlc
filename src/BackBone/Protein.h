/* 
 * File:   Protein.h
 * Author: zhang30
 *
 * Created on December 6, 2011, 8:32 AM
 */

#ifndef PROTEIN_H
#define	PROTEIN_H

#include <string>
#include <vector>
#include "Base.h"

namespace LBIND {
    class Pathway;
    class Ligand;
class Protein : public Base {
public:
    Protein();
    Protein(int idVal, std::string nameVal);
    Protein(const Protein& orig);
    virtual ~Protein();
    
    void downloadPDB(std::string pdbid);    
    void getLigandPDB(std::string ligName);
    
    void docking();
    void scoring();
    void mmGBSA();
    void singlePtMM();
    void flexDocking();
    
    void setPDBID(std::string pdbid);
    std::string getPDBID();
    
    void addLigand(Ligand* pLigand);
    void getLigands(std::vector<Ligand*>& ligList);
    
private:
    std::string pdbID;
    std::vector<Pathway*> pathways;
    
    std::vector<Ligand*> ligands;
    // std::vector<QSAR*>
    // std::vector<ADR*> adrs;
    // std::vector<Grid*> grids;
};

}//namespace LBIND 

#endif	/* PROTEIN_H */

