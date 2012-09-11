/* 
 * File:   Pdb.h
 * Author: xiaohua
 *
 * Created on May 26, 2009, 7:23 AM
 */

#ifndef _PDB_H
#define	_PDB_H

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "Structure/Coor3d.h"
#include "Structure/Fragment.h"

namespace LBIND{
    class Complex;
//    class Conformer;
    class Molecule;

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
    
    void renameAtom(const std::string& inFileName, const std::string& outFileName);
    void strip(const std::string& inFileName, const std::string& outFileName);
    void cutByRadius(const std::string& inFileName, const std::string& outFileName, Coor3d& center, double radius);
    
    int splitByModel(const std::string& inFileName, const std::string& outFileBase);
    
    void standardlize(const std::string& inFileName, const std::string& outFileName);
    void standardlize2(const std::string& inFileName, const std::string& outFileName);
    void standardlizeD(const std::string& inFileName, const std::string& outFileName);
//    void write(const std::string& fileName, boost::shared_ptr<Conformer> pConformer);
//    void write(const std::string& fileName, Conformer* pConformer);
private:
    std::string newAtomName(const std::string& atomType, int seq);
    bool isAA(Fragment* pFrag);

};

} //namespace LBIND
#endif	/* _PDB_H */

