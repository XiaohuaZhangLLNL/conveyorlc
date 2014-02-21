#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include "src/Structure/Complex.h"
#include "src/Structure/Coor3d.h"
#include "src/Common/LBindException.h"
#include "src/Common/Tokenize.hpp"
#include "src/BackBone/Surface.h"
#include "calcSASAPO.h"
#include "Parser/Pdb.h"
#include "Structure/ParmContainer.h"

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

using namespace LBIND;

/*
 * 
 */
int main(int argc, char** argv) {
    POdata podata;
    
    bool success=calcSASAPO(argc, argv, podata);
    if(!success){
        return 1;
    }    
    
    boost::scoped_ptr<Complex> pComplex(new Complex());
    
    boost::scoped_ptr<Pdb> pPdb(new Pdb());  
    
    pPdb->parse(podata.pdbFile,pComplex.get());
    
    boost::scoped_ptr<ParmContainer> pParmContainer(new ParmContainer());
    
    ElementContainer* pElementContainer=pParmContainer->addElementContainer();
    
    
    pComplex->assignElement(pElementContainer); 
    
    
    boost::scoped_ptr<Surface> pSurface(new Surface(pComplex.get()));
    
    std::cout << "Start Calculation " << std::endl;
      
    pSurface->run(1.4, 960); 
    
    std::cout << "Total SASA is: " << pSurface->getTotalSASA();

    delete pElementContainer;
    
    return 0;
}


