#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <exception>

#include "Structure/Complex.h"
#include "Structure/Atom.h"
#include "Structure/Coor3d.h"
#include "Common/LBindException.h"
#include "Common/Tokenize.hpp"
#include "BackBone/Surface.h"
#include "BackBone/Grid.h"
#include "calcSASAPO.h"
#include "Parser/Pdb.h"
#include "Structure/ParmContainer.h"

#include <boost/scoped_ptr.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/timer.hpp>

using namespace LBIND;

/*
 * 
 */
int main(int argc, char** argv) {
    
    try{
        boost::timer runingTime;

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

        pSurface->run(podata.radius, podata.surfSphNum); 

    //    std::vector<Atom*> atomList=pComplex->getAtomList();
    //    
    //    int count=0;
    //    double area=0;
    //    
    //    for(unsigned i=0; i<atomList.size();++i){
    //        Atom* pAtom=atomList[i];
    //        std::cout << pAtom->getFileID() << " "  << pAtom->getParent()->getName() << " " << pAtom->getParent()->getID() << " "<< pAtom->getName() << " " << pAtom->getSASA() <<std::endl;
    //
    //        if(pAtom->getSASA() > 0.1){
    //            area+=pAtom->getSASA();
    //            ++count;
    //        }
    //    }

        std::cout << " Total SASA is: " << pSurface->getTotalSASA() << std::endl << std::endl;



        boost::scoped_ptr<Grid> pGrid(new Grid(pComplex.get()));

        pGrid->run(podata.radius, podata.gridSphNum);

        delete pElementContainer;

        std::cout << "\nTotal Calculation Time: " << runingTime.elapsed() << " Seconds."<< std::endl;
    
    }catch(LBindException &e){
        std::cout << "\nError: LBindException " << e.what() << std::endl;
    }catch(std::exception &e){
        std::cout << "\nError: std::exception " << e.what() << std::endl;
    }catch(...){
        std::cout << "\nError: unknown exception " <<  std::endl;
    }
    
    return 0;
}


