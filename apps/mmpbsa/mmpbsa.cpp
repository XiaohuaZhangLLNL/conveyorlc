/* 
 * File:   mmpbsa.cpp
 * Author: zhang30
 *
 * Created on August 16, 2012, 10:52 AM
 */

#include <cstdlib>
#include <iostream>
#include <vector>


#include "src/MM/SpMMPBSA.h"

#include <boost/scoped_ptr.hpp>

using namespace LBIND;

/*!
 * \breif mmpbsa MM-PB(GB)SA calculations on HPC using amber forcefield
 * \param argc
 * \param argv
 * \return success 
 * \defgroup mmpbsa_Commands mmpbsa Commands
 * 
 * Usage: mmpbsa <input-file>
 */



int main(int argc, char** argv) {

    std::string dir="1EDM";
    std::string ligand="206";
    bool calcPB=true;
    
    boost::scoped_ptr<SpMMPBSA> pSpMMPBSA(new SpMMPBSA(calcPB));
    pSpMMPBSA->run(dir, ligand);
    
    std::vector<double> bindGB;
    pSpMMPBSA->getbindGB(bindGB);
    
    for(unsigned i=0; i < bindGB.size(); ++i){
        std::cout <<"Pose " << i << ": " <<bindGB[i] << "kcal/mol" << std::endl;
    }
    
    std::vector<double> bindPB;
    pSpMMPBSA->getbindPB(bindPB);
    
    for(unsigned i=0; i < bindPB.size(); ++i){
        std::cout <<"Pose " << i << ": " <<bindPB[i] << "kcal/mol" << std::endl;
    }
    
    
    return 0;
}

