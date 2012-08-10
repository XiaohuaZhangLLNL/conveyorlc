/* 
 * File:   Amber.h
 * Author: zhang30
 *
 * Created on December 15, 2011, 5:43 PM
 */

#include <string>

#ifndef AMBER_H
#define	AMBER_H

namespace LBIND {
    class Protein;
class Amber {
public:
    Amber();
    Amber(Protein* pProt);
    Amber(const Amber& orig);
    virtual ~Amber();
    
    void run();
    
    void reduce(std::string& input, std::string& output, std::string& options);
    void antechamber(std::string& input, std::string& output, std::string& options);
    void parmchk(std::string mol2FName);
    void tleap(std::string input);
    void minimization(std::string pdbid);
    void md(std::string pdbid);
    
    void ambpdb(std::string input, std::string output);
    
    void prepLigands();
    
private:
    void ligLeapInput(std::string pdbid, std::string ligName, std::string tleapFName);
    void comLeapInput(std::string pdbid, std::string ligName, std::string tleapFName);
    
private:
    Protein* pProtein;
    std::string AMBERPATH;
};

}//namespace LBIND 
#endif	/* AMBER_H */

