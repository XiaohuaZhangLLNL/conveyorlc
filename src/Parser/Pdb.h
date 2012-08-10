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
    
    void read(const std::string& fileName, Complex* pComplex);
    void read(const std::string& fileName, boost::shared_ptr<Complex> pComplex);
    void write(const std::string& fileName, Complex* pComplex);
    void write(const std::string& fileName, boost::shared_ptr<Complex> pComplex);
    void write(const std::string& fileName, Molecule* pMol);
//    void write(const std::string& fileName, boost::shared_ptr<Conformer> pConformer);
//    void write(const std::string& fileName, Conformer* pConformer);
private:

};

} //namespace LBIND
#endif	/* _PDB_H */

