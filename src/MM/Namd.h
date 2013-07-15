/* 
 * File:   Namd.h
 * Author: zhang30
 *
 * Created on December 15, 2011, 5:39 PM
 */

#ifndef NAMD_H
#define	NAMD_H

#include <string>
#include <vector>
#include <Structure/Coor3d.h>

namespace LBIND {
    class Protein;
class Namd {
public:
    Namd();
    Namd(Protein* pProt);
    Namd(const Namd& orig);
    virtual ~Namd();
    
    void run();
    void psfgen(std::string pdbid);
    void solvate(std::string pdbid);
    void minimization(std::string pdbid);
    void md(std::string pdbid);

private:
    void setupPBCinput(std::string pdbid);
//    void tokenize(std::string inputStr, std::vector<double>& tokens);
    
private:
    Protein* pProtein;
    std::string VMDEXEPATH;
    std::string NAMDEXEPATH;
    std::string dataPath;
    Coor3d watBoxSize;
    Coor3d watBoxCenter;

};

}//namespace LBIND 

#endif	/* NAMD_H */

