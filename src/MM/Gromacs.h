/* 
 * File:   Gromacs.h
 * Author: zhang30
 *
 * Created on December 9, 2011, 3:32 PM
 */

#ifndef GROMACS_H
#define	GROMACS_H

#include <string>

namespace LBIND {
    class Protein;
class Gromacs {
public:
    Gromacs();
    Gromacs(Protein* pProt);
    Gromacs(const Gromacs& orig);
    virtual ~Gromacs();
    
    void run();
    void init();
    
    void prepare(std::string pdbid);    
    void pdb2gmx(std::string pdbid);
    void editconf(std::string pdbid);
    void genbox(std::string pdbid);
    void genbox(std::string pdbid, std::string groFile);
    void minimization(std::string pdbid);
    void grompp(std::string pdbid, int nTimes);
    void genion(std::string pdbid);
    void prmd(std::string pdbid);
    void md(std::string pdbid);
    
    
private:
    void minimizationInput(std::string pdbid);
    void prmdInput(std::string pdbid);
    void mdInput(std::string pdbid);
    
private:
    Protein* pProtein;
    std::string GromacsPath;
    std::string GromacsEXEPath;
    std::string GromacsTopPath;     
};

}//namespace LBIND 
#endif	/* GROMACS_H */

