/* 
 * File:   Pdb.h
 * Author: xiaohua
 *
 * Created on May 26, 2009, 7:23 AM
 */

#ifndef _PDB_H
#define	_PDB_H

#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "Structure/Coor3d.h"
#include "Structure/Fragment.h"

namespace LBIND{
    class Complex;
//    class Conformer;
    class Molecule;
    class Atom;

class Pdb {
public:
    Pdb();
    Pdb(const Pdb& orig);
    virtual ~Pdb();

    //! method to by-pass the stringstream crash on Linux box;
    void parse(const std::string& fileName, Complex* pComplex);
    void parseOut(const std::string& fileName, Complex* pComplex);
    
    void read(const std::string& fileName, Complex* pComplex);
    void read(const std::string& fileName, boost::shared_ptr<Complex> pComplex);
    void write(const std::string& fileName, Complex* pComplex);
    void write(const std::string& fileName, boost::shared_ptr<Complex> pComplex);
    void write(const std::string& fileName, Molecule* pMol);    
    void write(const std::string& fileName, std::vector<Atom*>& atomList, const std::string& resName);
    
    void renameAtom(const std::string& inFileName, const std::string& outFileName);
    void strip(const std::string& inFileName, const std::string& outFileName);
    void cutByRadius(const std::string& inFileName, const std::string& outFileName, Coor3d& center, double radius);
    bool aveKeyResCoor(const std::string& inFileName, std::vector<std::string>& keyRes, Coor3d& aveCoor);
    bool calcAverageCoor(const std::string& fileName, Coor3d& aveCoor);
    
    int splitByModel(const std::string& inFileName, const std::string& outFileBase);
    bool readByModel(const std::string& inFileName, const std::string& outFile, int modelID, double& score);
    
    void selectAForm(const std::string& inFileName, const std::string& outFileName);
    
    void standardlize(const std::string& inFileName, const std::string& outFileName);
    void standardlize2(const std::string& inFileName, const std::string& outFileName);
    void standardlizeD(const std::string& inFileName, const std::string& outFileName);
    void standardlizeSS(const std::string& inFileName, const std::string& outFileName, std::vector<std::vector<int> >& ssList);
    
    void getDisulfide(const std::string& inFileName, std::vector<std::vector<int> >& ssList);
    
    void fixElement(const std::string& inFileName, const std::string& outFileName);
//    void write(const std::string& fileName, boost::shared_ptr<Conformer> pConformer);
//    void write(const std::string& fileName, Conformer* pConformer);
    
    bool isAA(Fragment* pFrag, Coor3d& coorN, Coor3d& coorCA, Coor3d& coorC, Coor3d& coorO);
    bool isAA(Fragment* pFrag);
    bool isAA(Fragment* pFrag, Coor3d& coorCA);
private:
    void guessElement(std::string& atomType, const std::string& resName, const std::string& atomName);
    std::string newAtomName(const std::string& atomType, int seq);
    
    void toALA(Fragment* pFrag); //! Convert non-standard Amino Acid to ALA
    // a multi-definition issue.

    void pdbchomp(std::string& s); 
//    std::string guessSymbol(const std::string& atomName);

};

} //namespace LBIND
#endif	/* _PDB_H */

