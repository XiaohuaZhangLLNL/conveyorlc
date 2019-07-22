//
// Created by Zhang, Xiaohua on 2019-07-19.
//


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

#include "Parser/Pdb.h"
#include "Parser/Mol2.h"
#include "Parser/Sdf.h"


using namespace LBIND;


void testBoundBox(){
    Coor3d centroid;
    Coor3d boxDim;
    bool hasSubResCoor;

    boost::scoped_ptr<Mol2> pMol2(new Mol2());
    hasSubResCoor=pMol2->calcBoundBox("1a50_ligand.mol2", centroid, boxDim);
    std::cout << "Average coordinates of mol2: " << centroid << std::endl;
    std::cout << "Box of mol2:                 " << boxDim << std::endl;

    boost::scoped_ptr<Pdb> pPdb(new Pdb());
    hasSubResCoor=pPdb->calcBoundBox("1a50_ligand.pdb", centroid, boxDim);
    std::cout << "Average coordinates of pdb: " << centroid << std::endl;
    std::cout << "Box of pdb:                 " << boxDim << std::endl;


    boost::scoped_ptr<Sdf> pSdf(new Sdf());
    hasSubResCoor=pSdf->calcBoundBox("1a50_ligand.pdb", centroid, boxDim);
    std::cout << "Average coordinates of sdf: " << centroid << std::endl;
    std::cout << "Box of sdf:                 " << boxDim << std::endl;
}

int main(int argc, char** argv) {
    testBoundBox();
}