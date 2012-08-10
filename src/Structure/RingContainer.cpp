/* 
* File:   RingContainer.cpp
* Author: zhang
* 
* Created on August 23, 2010, 1:19 PM
*/

#include <algorithm>
#include <sstream>

#include "RingContainer.h"

#include "Atom.h"
#include "Molecule.h"
#include "Bond.h"
#include "Fragment.h"
#include "Element.h"
#include "BondContainer.h"
#include "Ring.h"
#include "Complex.h"
#include "Coor3d.h"
#include "Constants.h"
#include "Sstrm.hpp"
#include "Common/LBindException.h"
#include "Logger.h"

namespace LBIND {

RingContainer::RingContainer(Molecule* parent) : pParent(parent) {
    pParent->getParent();
    
}

RingContainer::RingContainer(const RingContainer& orig) {
}

RingContainer::~RingContainer() {
}

void RingContainer::determine() {
     this->atomList = pParent->getGrdChildren();\
     
#ifdef USE_LOGGER    
    std::stringstream ss;
    ss << " Number of Atoms (a) = " << atomList.size() << std::endl;
#endif
    
    if (atomList.size() < 3) return;
    pBondContainer = pParent->getPBondContainer();
    bondList = pBondContainer->getBondList();
    int frerejaque = bondList.size() - atomList.size() + 1;

#ifdef USE_LOGGER    
    ss <<   "RingContainer::determine \n" <<
            " Number of Atoms (a) = " << atomList.size() <<
            "\n Number of Bonds (b) = " << bondList.size() <<
            "\n Frerejaque Number (b - a + 1) = " << frerejaque << std::endl;
    Logger::Instance()->writeToLogFile(ss.str());
#endif
    // if rings are present
    if (frerejaque > 0) {
        this->deleteHydrogen();

        this->deleteOpenAcyclic();

        this->deleteOpenAcyclicBonds();

        this->findNeighbors();

        this->deleteClosedAcyclic();

        this->findNeighbors();

        this->separateBlocks();

        this->findSSSR();
    } else {
        this->deleteHydrogen();

        this->deleteOpenAcyclic();

        this->deleteOpenAcyclicBonds();
    }

    std::stringstream ss2;
    // Set the bond topology
//    Ring* r;
//    std::cout << "RingContainer Set the bond topology" << std::endl;
    Bond* pRingBond;
//    unsigned int s;
    for (unsigned int i = 0; i < ringList.size(); i++) {
//        std::cout <<"i= "<< i <<std::endl;
        pRing = ringList[i];
        std::vector<Atom*> ringAtoms=pRing->getAtomList();
//        s = r->atoms.size();
        Atom* pRingAtom1;
        Atom* pRingAtom2;
        for (unsigned int j = 0; j < ringAtoms.size(); j++) {
//            std::cout <<"j= "<< j <<std::endl;
            if (j < ringList.size() - 1) {
                pRingAtom1=ringAtoms[j];
                pRingAtom2=ringAtoms[j+1];
            } else {
                pRingAtom1=ringAtoms[j];
                pRingAtom2=ringAtoms[0];                
            }
            if(pRingAtom1->getFileID()!=0 && pRingAtom2->getFileID()!=0){
                pRingBond = pBondContainer->getBond(pRingAtom1, pRingAtom2);
            }else{
                pRingBond =0;
            }
            if(pRingBond !=0){
                    pRingBond->setTopology(1);
            }else{
                if(pRingAtom1->getFileID()!=0 && pRingAtom2->getFileID()!=0){
                    ss2 << ringAtoms[j]->getFileID() << "-" << ringAtoms[j+1]->getFileID() << 
                            " bond doesn't exsit." << std::endl;
                }
            }
        }
    }
//    std::cout << "END of RingContainer Set the bond topology" << std::endl;
    // The number of the smallest rings in a block can be
    // calculated using the following:
    //      Nr = 1 + n(N3)/2 + n(N4)
    // J. Chem. Inf. Comput. Sci. 2000, 40, 1015-1017
    std::vector<Bond*> blockBonds;
    std::vector<int> nConnections;

    int numberRings = 0;
    int N3 = 0;
    int N4 = 0;
    for (unsigned int i = 0; i < blocks.size(); i++) {
      this->getBonds(blocks[i],cyclicBonds,blockBonds,nConnections);
        for (unsigned int j = 0; j < nConnections.size(); j++) {
            if (nConnections[j] == 3) N3++;
            if (nConnections[j] == 4) N4++;
        }
        numberRings += 1 + N3 / 2 + N4;
        N3 = 0;
        N4 = 0;
    }

    ss2 << "RingContainer::determine\n" << " Number of Rings = " << numberRings << std::endl;
    //    std::string errMessage2 =   " Number of Rings = " + i2s(numberRings) + "\n";
    //    errorLogger.throwError("rings::determine", errMessage2, 4);
    Logger::Instance()->writeToLogFile(ss2.str());
}

void RingContainer::deleteHydrogen() {
    //Delete the hydrogen aotms.
    for (unsigned int i = 0; i < atomList.size(); i++) {
        pAtom = atomList[i];
        if (pAtom->getElement()->getAtomicNumber() != 1) {
            cyclicAtoms.push_back(pAtom);
        } else {
            pAtom->setType(1);
            hydrogens.push_back(pAtom);
        }
    }
    //Delete the hydrogen atom's bonds.
    pBondContainer = pParent->getPBondContainer();
    bondList = pBondContainer->getBondList();
    for (unsigned int b = 0; b < bondList.size(); b++) {
        pBond = bondList[b];
        if ((pBond->getAtom1()->getElement()->getAtomicNumber() == 1) ||
                (pBond->getAtom2()->getElement()->getAtomicNumber() == 1)) {
            continue;
        } else {
            pBond->setTopology(2);
            cyclicBonds.push_back(pBond);
        }
    }

}

void RingContainer::deleteOpenAcyclic() {
    std::stringstream ss;
    int numBonds = 0;
    bool done = false;
    bool terminal = false;
    std::vector<Atom*> bondedAtoms;
    //    Atom* pAtom;
    //    Atom* pAtom2;
    pBondContainer = pParent->getPBondContainer();

    while (!done) {
        done = true;
        for (unsigned int i = 0; i < cyclicAtoms.size(); i++) {
            terminal = false;
            pAtom = cyclicAtoms[i];
            bondedAtoms.clear();
            pBondContainer->getBondAtoms(pAtom, bondedAtoms);
            //        bondedAtoms = pAtom->getBondedAtoms();
            numBonds = bondedAtoms.size();
            ss << pAtom->getFileID() << " has " << numBonds << " bond/s" << std::endl;
            // Determine if atom is a true terminal atom or a open chain atom
            int testTerminal = 0;
            for (unsigned int j = 0; j < bondedAtoms.size(); j++) {
                pAtom2 = bondedAtoms[j];
                if (pAtom2->getElement()->getAtomicNumber() != 1) {
                    testTerminal++;
                }
            }
            if (testTerminal == 1) {
                terminal = true;
            }

            for (unsigned int j = 0; j < bondedAtoms.size(); j++) {
                pAtom2 = bondedAtoms[j];
                atomIterator = std::find(hydrogens.begin(), hydrogens.end(), pAtom2);
                if (atomIterator != hydrogens.end()) {
                    numBonds--;
                }
                atomIterator = std::find(acyclicAtoms.begin(), acyclicAtoms.end(), pAtom2);
                if (atomIterator != acyclicAtoms.end()) {
                    numBonds--;
                }
            }
            if (numBonds <= 1) {
                atomIterator = std::find(acyclicAtoms.begin(), acyclicAtoms.end(), pAtom);
                if (atomIterator == acyclicAtoms.end()) {

                    if (terminal) {
                        pAtom->setType(2);
                    } else {
                        pAtom->setType(3);
                    }
                    acyclicAtoms.push_back(pAtom);
                    done = false;
                }
            }
            numBonds = 0;
        }
    }
    
    Logger::Instance()->writeToLogFile(ss.str());

    for (unsigned int i = 0; i < acyclicAtoms.size(); i++) {
        pAtom = acyclicAtoms[i];
        atomIterator = std::find(cyclicAtoms.begin(), cyclicAtoms.end(), pAtom);
        if (atomIterator != cyclicAtoms.end()) {
            cyclicAtoms.erase(atomIterator);
        }
    }

}

void RingContainer::deleteOpenAcyclicBonds() {
    //std::vector<Bond*>::iterator bondResult;
    pBondContainer = pParent->getPBondContainer();
    
    std::stringstream ss;
    
    for (unsigned int i = 0; i < acyclicAtoms.size(); i++) {
        pAtom = acyclicAtoms[i];
        for (unsigned int j = 0; j < acyclicAtoms.size(); j++) {
            pAtom2 = acyclicAtoms[j];
            pBond = pBondContainer->getBond(pAtom, pAtom2);
            if (pBond != 0) {
                bondIterator = std::find(cyclicBonds.begin(), cyclicBonds.end(), pBond);
                if (bondIterator != cyclicBonds.end()) {
                    pBond->setTopology(2);
                    cyclicBonds.erase(bondIterator);
                }
            }
        }
        for (unsigned int j = 0; j < cyclicAtoms.size(); j++) {
            pAtom2 = cyclicAtoms[j];
            pBond = pBondContainer->getBond(pAtom, pAtom2);
            if (pBond != 0) {
                bondIterator = std::find(cyclicBonds.begin(), cyclicBonds.end(), pBond);
                if (bondIterator != cyclicBonds.end()) {
                    pBond->setTopology(2);
                    //#ifdef DEBUG
                    ss << " rings:removeOpenAcyclicBonds -> Erasing Terminal Bond = "
                              << pBond->getAtom1()->getFileID() << "-" << pBond->getAtom2()->getFileID() << std::endl;
                    //#endif
                    cyclicBonds.erase(bondIterator);
                }
            }
        }
    }
    Logger::Instance()->writeToLogFile(ss.str());
}

void RingContainer::findNeighbors() {
    neighbors.clear();
    pBondContainer = pParent->getPBondContainer();
    for (unsigned int i = 0; i < cyclicAtoms.size(); i++) {
        pAtom = cyclicAtoms[i];
        std::vector<int> n;
        for (unsigned int j = 0; j < cyclicAtoms.size(); j++) {
            pAtom2 = cyclicAtoms[j];
//            pBond = pParent->getBond(pAtom, pAtom2);
            if (pBondContainer->isBonded(pAtom, pAtom2)) {
                n.push_back(j);
            }
        }
        neighbors.push_back(n);
    }
}

void RingContainer::findNeighbors(std::vector<Atom*> a, std::vector<std::vector<int> >& b) {
    pBondContainer = pParent->getPBondContainer();
    for (unsigned int i = 0; i < a.size(); i++) {
        pAtom = a[i];
        std::vector<int> n;
        for (unsigned int j = 0; j < a.size(); j++) {
            pAtom2 = a[j];
            if (pBondContainer->isBonded(pAtom, pAtom2)) {
                n.push_back(j);
            }
        }
        b.push_back(n);
    }
}

// ============================================================
// Function : getBonds()
// ------------------------------------------------------------
// Find all bonds between atoms in a from the list b
// ============================================================

void RingContainer::getBonds(std::vector<Atom*> a, std::vector<Bond*> b,
        std::vector<Bond*> &c, std::vector<int> &d) {
    int i = 0;
    c.clear();
    d.clear();

    for (unsigned int q = 0; q < a.size(); q++) {
        Atom* pAtomQ = a[q];
        for (unsigned int p = 0; p < a.size(); p++) {
            Atom* pAtomP = a[p];
            for (unsigned int f = 0; f < b.size(); f++) {
                Bond* pBondF = b[f];
                if ((pBondF->getAtom1() == pAtomQ && pBondF->getAtom2() == pAtomP) ||
                        (pBondF->getAtom1() == pAtomP && pBondF->getAtom2() == pAtomQ)) {
                    bondIterator = std::find(c.begin(), c.end(), pBondF);
                    if (bondIterator == c.end()) {
                        c.push_back(pBondF);
                    }
                    i++;
                }
            }
        }
        d.push_back(i);
        i = 0;
    }
}

void RingContainer::deleteClosedAcyclic() {
    // color: 0 == white
    // color: 1 == grey
    // color: 2 == black
    std::vector<int> vertexesColor;
    for (unsigned int i = 0; i < cyclicAtoms.size(); i++) {
        vertexesColor.push_back(0);
    }

    std::vector<int> edgesColor;
    for (unsigned int i = 0; i < cyclicBonds.size(); i++) {
        edgesColor.push_back(0);
    }

    std::vector< std::vector<Atom*> > paths;
    std::vector<Atom*> path;
    bool loop = false;

    for (unsigned int i = 0; i < cyclicAtoms.size(); i++) {
        std::vector<int> curNeighbors = neighbors[i];
        for (unsigned int j = 0; j < curNeighbors.size(); j++) {
            vertexesColor[i] = 1;
            int curNeighbor = curNeighbors[j];
            int t = 0;
            dfs_visit(curNeighbor, i, cyclicAtoms, cyclicBonds, neighbors, vertexesColor, edgesColor, loop, t, path, paths);
            t = 0;

            for (unsigned int k = 0; k < cyclicAtoms.size(); k++) {
                vertexesColor[k] = 0;
            }
            for (unsigned int l = 0; l < cyclicBonds.size(); l++) {
                edgesColor[l] = 0;
            }
            if (loop) {
                path.clear();
                break;
            }
        }
        if (!loop) {
            cyclicAtoms[i]->setType(4);
            acyclicAtoms.push_back(cyclicAtoms[i]);
            path.clear();
            continue;
        }
        loop = false;
    }

    std::vector<Atom*>::iterator result;
    for (unsigned int i = 0; i < acyclicAtoms.size(); i++) {
        pAtom = acyclicAtoms[i];
        result = std::find(cyclicAtoms.begin(), cyclicAtoms.end(), pAtom);
        if (result != cyclicAtoms.end()) {
            cyclicAtoms.erase(result);
        }
    }
    this->deleteOpenAcyclicBonds();
}

void RingContainer::separateBlocks() {
    std::vector<int> vertexesColor;
    for (unsigned int i = 0; i < cyclicAtoms.size(); i++) {
        vertexesColor.push_back(0);
    }

    std::vector<int> edgesColor;
    for (unsigned int i = 0; i < cyclicBonds.size(); i++) {
        edgesColor.push_back(0);
    }

    std::vector< std::vector<Atom*> > paths;
    std::vector<Atom*> path;
    std::vector<Atom*> block;
    std::vector<Atom*> b;

    int curNeighbor;

    bool loopMatrix[cyclicAtoms.size()][cyclicAtoms.size()];
    for (unsigned int k = 0; k < cyclicAtoms.size(); k++) {
        for (unsigned int h = 0; h < cyclicAtoms.size(); h++) {
            loopMatrix[k][h] = false;
        }
    }
    
    std::stringstream ss;

    bool loop = false;
    for (unsigned int i = 0; i < cyclicAtoms.size(); i++) {
        std::vector<int> curNeighbors = neighbors[i];
        for (unsigned int j = 0; j < curNeighbors.size(); j++) {
            vertexesColor[i] = 1;
            curNeighbor = curNeighbors[j];
            int t = 0;
            dfs_visit(curNeighbor, i, cyclicAtoms, cyclicBonds, neighbors, vertexesColor, edgesColor, loop, t, path, paths);
            loopMatrix[i][curNeighbor] = loop;
            t = 0;

            for (unsigned int k = 0; k < cyclicAtoms.size(); k++) {
                vertexesColor[k] = 0;
            }
            for (unsigned int l = 0; l < cyclicBonds.size(); l++) {
                edgesColor[l] = 0;
            }
            path.clear();
            loop = false;
        }
    }

    block.push_back(cyclicAtoms[0]);
    blocks.push_back(block);
    int curBlock = 0;

    unsigned int sumInBlocks = 1;
    bool someAdded = false;
    int k = 0;

    while (sumInBlocks < cyclicAtoms.size()) {
        for (unsigned int i = 0; i < blocks[curBlock].size(); i++) {
            for (unsigned int j = 0; j < cyclicAtoms.size(); j++) {
                if (blocks[curBlock][i] == cyclicAtoms[j]) k = j;
            }
            for (unsigned int j = 0; j < cyclicAtoms.size(); j++) {
                if (loopMatrix[k][j] == true) {
                    atomIterator = std::find(blocks[curBlock].begin(), blocks[curBlock].end(), cyclicAtoms[j]);
                    if (atomIterator == blocks[curBlock].end()) {
                        blocks[curBlock].push_back(cyclicAtoms[j]);
                        someAdded = true;
                    }
                }
            }
            k = 0;
        }

        sumInBlocks = 0;
        for (unsigned int x = 0; x < blocks.size(); x++) {
            sumInBlocks += blocks[x].size();
        }

        if (sumInBlocks == cyclicAtoms.size()) break;

        if (someAdded) {
            someAdded = false;
        } else {
            Atom* a = 0;
            bool gotIt = false;
            for (unsigned int i = 1; i < cyclicAtoms.size(); i++) {
                ss << cyclicAtoms[i]->getFileID() << std::endl;
                for (unsigned int j = 0; j < blocks.size(); j++) {
                    for (unsigned int j2 = 0; j2 < blocks[j].size(); j2++) {
                        if (cyclicAtoms[i] == blocks[j][j2]) {
                            gotIt = true;
                            ss << " gotIT " << std::endl;
                        }
                    }
                }
                if (!gotIt) {
                    ss << " !gotIt " << cyclicAtoms[i]->getFileID() << std::endl;
                    a = cyclicAtoms[i];
                    break;
                }
                gotIt = false;
            }
            curBlock++;
            std::vector<Atom*> c;
            c.push_back(a);
            blocks.push_back(c);
        }
    }

    // REMOVE INTER BLOCK BONDS
    std::vector<Atom*> block1;
    std::vector<Atom*> block2;
    Atom* block1Atom;
    Atom* block2Atom;
    /*
    #ifdef DEBUG
       std::cout << blocks.size() << std::endl;
       for (unsigned int i = 0; i < blocks.size(); i++) {
         std::cout << " ring::block: " << std::endl;
         for (unsigned int j = 0; j < blocks[i].size(); j++) {
           std::cout << blocks[i][j]->getFileID() << std::endl;
         }
       }
    #endif
     */
    pBondContainer = pParent->getPBondContainer();
    if (blocks.size() > 1) {
        for (unsigned int i = 0; i < blocks.size(); i++) {
            block1 = blocks[i];
            for (unsigned int j = i + 1; j < blocks.size(); j++) {
                block2 = blocks[j];
                for (unsigned int d = 0; d < block1.size(); d++) { // k to d
                    block1Atom = block1[d];
                    for (unsigned int l = 0; l < block2.size(); l++) {
                        block2Atom = block2[l];
//                        pBond = pParent->getBond(block1Atom, block2Atom);
                        if (pBondContainer->isBonded(block1Atom, block2Atom)) {
                            bondIterator = std::find(cyclicBonds.begin(), cyclicBonds.end(), pBond);
                            if (bondIterator != cyclicBonds.end()) {
                                cyclicBonds.erase(bondIterator);
                                ss << block1Atom->getName() << " " << block2Atom->getName() << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }
    Logger::Instance()->writeToLogFile(ss.str());
}

int RingContainer::pickRootAtom(std::vector<Atom*> a) {
    int root = 0;
    int nBonds = 0;

    typedef std::vector<int>::iterator iVectorIt;
    iVectorIt start, end;

    std::vector<int> possibleRoots;

    pBondContainer = pParent->getPBondContainer();
    
    for (unsigned int i = 0; i < a.size(); i++) {
        pAtom = a[i];
        for (unsigned int j = 0; j < a.size(); j++) {
            pAtom2 = a[j];
//            pBond = pParent->getBond(pAtom, pAtom2);
            if (pBondContainer->isBonded(pAtom, pAtom2)) {
                nBonds++;
            }
        }
        if (nBonds == 2) {
            possibleRoots.push_back(i);
        }
        nBonds = 0;
    }
    start = possibleRoots.begin();
    end = possibleRoots.end();
    std::random_shuffle(start, end);
    root = possibleRoots[0];
    return root;
}

void RingContainer::findSSSR() {
    for (unsigned int i = 0; i < blocks.size(); i++) {
        this->decomposeBlock(blocks[i]);
    }
}

void RingContainer::addRing(std::vector<Atom*> r) {
    std::vector<Atom*>::iterator atomIterator;
    bool unique = true;
    unsigned int c = 0;
    std::stringstream ss;
    ss << "RingContainer::addRing(std::vector<Atom*> r)" << std::endl;
    // Check to see if ring is already been added
    for (unsigned int i = 0; i < ringList.size(); i++) {
        std::vector<Atom*> ringAtoms=ringList[i]->getAtomList();
        for (unsigned int j = 0; j < ringAtoms.size(); j++) {
            atomIterator = std::find(r.begin(), r.end(), ringAtoms[j]);
            if (atomIterator != r.end()) {
                c++;
            }
        }
        if (c == r.size()) {
            unique = false;
        }
        c = 0;
    }

    bool error = false;
    if (unique) {
        Ring* myRing = new Ring();
        Bond* pRingBond = 0;
        pBondContainer = pParent->getPBondContainer();
        for (unsigned int i = 0; i < r.size(); i++) {
            if (i != r.size() - 1) {
                pRingBond = pBondContainer->getBond(r[i], r[i + 1]);
                if (pRingBond != 0) {
                    pRingBond->setTopology(1);
                } else {
                    ss << " NO BOND BETWEEN " <<
                            r[i]->getName() <<  "-" <<  r[i+1]->getName() << std::endl;
                    //std::string errMessage = " NO BOND BETWEEN " + r[i]->getName() + "-" + r[i+1]->getName();
                    //errorLogger.throwError("molecule::addRing", errMessage, MTK_ERROR);
                    error = true;
                }
            } else {
                pRingBond = pBondContainer->getBond(r[i], r[0]);
                if (pRingBond !=0) {
                    pRingBond->setTopology(1);
                } else {
                    ss << " NO BOND BETWEEN " <<
                            r[i]->getName() << "-" << r[0]->getName() << std::endl;
                    //std::string errMessage = " NO BOND BETWEEN " + r[i]->getName() + "-" + r[0]->getName();
                    //errorLogger.throwError("molecule::addRing", errMessage, MTK_ERROR);
                    error = true;
                }
            }
            myRing->addAtom(r[i]);
        }
//        myRing->size = myRing->atoms.size();
//        myRing->planar = 0;
//        myRing->aromatic = 0;
//        myRing->hetero = 0;
//        myRing->nHetero = 0;
//        myRing->nNitrogen = 0;
//        myRing->nOxygen = 0;
//        myRing->nSulfur = 0;
        int nHetero = 0;
        int nNitrogen = 0;
        int nOxygen = 0;
        int nSulfur = 0;
        
        
        if (error) {
            delete myRing;
        } else {

            //#ifdef DEBUG
            std::string errMessage = "\n";

            std::vector<Atom*> ringAtoms=myRing->getAtomList();
            Bond* pRingBond = 0;
            for (unsigned int i = 0; i < ringAtoms.size(); i++) {
                if (i != ringAtoms.size() - 1) {
                    pRingBond = pBondContainer->getBond(ringAtoms[i], ringAtoms[i + 1]);
                    if (pRingBond != 0) {
//                        errMessage += "    " + i2s(pRingBond->atom1->getFileID()) + "-" + i2s(pRingBond->atom2->getFileID())
//                                + " type = " + i2s(pRingBond->type) + " topology = " + i2s(pRingBond->topology) + "\n";
                    }
                } else {
                    pRingBond = pBondContainer->getBond(ringAtoms[i], ringAtoms[0]);
                    if (pRingBond !=0) {
//                        errMessage += "    " + i2s(pRingBond->atom1->getFileID()) + "-" + i2s(pRingBond->atom2->getFileID())
//                                + " type = " + i2s(pRingBond->type) + " topology = " + i2s(pRingBond->topology) + "\n";
                    }
                }
            }
//            errorLogger.throwError("molecule::addRing", errMessage, INFO);
            //#endif

            for (unsigned int i = 0; i < ringAtoms.size(); i++) {
                std::string symbol = ringAtoms[i]->getSymbol();
                if (symbol == "O") {
                    nHetero++;
                    nOxygen++;
                    myRing->setHetero(true);
                    myRing->setNHetero(nHetero);
                    myRing->setNOxygen(nOxygen);
                }
                if (symbol == "S") {
                    nHetero++;
                    nSulfur++;
                    myRing->setHetero(true); 
                    myRing->setNHetero(nHetero);
                    myRing->setNSulfur(nSulfur);

                }
                if (symbol == "N") {
                    nHetero++;
                    nNitrogen++;
                    myRing->setHetero(true);
                    myRing->setNHetero(nHetero);
                    myRing->setNNitrogen(nNitrogen);
                }
            }
            this->ringList.push_back(myRing);
            myRing->setID(this->ringList.size());
        }
    }
    //else {
    //  puts("ITS NOT UNIQUE");
    //}
    ss << "END of RingContainer::addRing(std::vector<Atom*> r)" << std::endl;
    Logger::Instance()->writeToLogFile(ss.str());
}

void RingContainer::decomposeBlock(std::vector<Atom*> block) {
    std::stringstream ss;
    if (this->numberRingsInBlock(block) > 1) {
        for (unsigned int i = 0; i < block.size(); i++) {
            //int root = this->pickRootAtom(block);
            std::vector<Atom*> path;
            this->getIrreducibleClosedPath(block, i, path);

            if (path.size() > 2) {
                
//#ifdef DEBUG
                ss << " Try To Add Ring:";
                for (unsigned int cv = 0; cv < path.size(); cv++) {
                ss << " " << path[cv]->getFileID();
                }
                ss << " \n" << std::endl;
//#endif
                 
                this->addRing(path);
            }
        }
    } else {
        std::vector<Atom*> path;
        this->getIrreducibleClosedPath(block, 0, path);
        
//#ifdef DEBUG
        ss << " Try To Add Ring:";
        for (unsigned int cv = 0; cv < path.size(); cv++) {
        ss << " " << path[cv]->getFileID();
        }
        ss << " \n" << std::endl;
//#endif
         
        this->addRing(path);
    }
    
    Logger::Instance()->writeToLogFile(ss.str());
}

// ============================================================
// Function : getIrreducibleClosedPath()
// ------------------------------------------------------------
//
// ============================================================

void RingContainer::getIrreducibleClosedPath(std::vector<Atom*> block, int root, std::vector<Atom*>& path) {
    bool loop = false;
    std::vector<Bond*> blockBonds;
    std::vector< std::vector<int> > blockNeighbors;
    std::vector<int> vertexesColor;
    std::vector<int> edgesColor;
    std::vector<Atom*> curPath;
    std::vector<int> nConnections;

    blockBonds.clear();
    nConnections.clear();
    blockNeighbors.clear();
    vertexesColor.clear();
    edgesColor.clear();
    this->getBonds(block, cyclicBonds, blockBonds, nConnections);
    this->findNeighbors(block, blockNeighbors);
    
    std::stringstream ss;

    for (unsigned int tI = 0; tI < block.size(); tI++) {
        vertexesColor.push_back(0);
    }
    for (unsigned int tI = 0; tI < blockBonds.size(); tI++) {
        edgesColor.push_back(0);
    }
    int curAtom = blockNeighbors[root][0];

    bool done = false;
    bool firstTime = true;
    int nN3 = 0;
    std::vector<int> nN3s;
    while (!done) {

        if (firstTime) {
            bool ok = false;
            while (!ok) {
                curPath.push_back(block[root]);
                curPath.push_back(block[curAtom]);
                vertexesColor[root] = 1; // black
                Bond* pBond;
                for (unsigned int j = 0; j < blockBonds.size(); j++) {
                    pBond = blockBonds[j];
                    if ((pBond->getAtom1() == block[root] && pBond->getAtom2() == block[curAtom]) ||
                            (pBond->getAtom2() == block[root] && pBond->getAtom1() == block[curAtom])) {
                        edgesColor[j] = 1; // black
                    }
                }
                this->dfs_visitNEW(curAtom, root, 1, loop, block, blockBonds,
                        blockNeighbors, vertexesColor, edgesColor, curPath);

                this->checkPath(curPath, ok);

                if (!ok) {
                    for (unsigned int cv = 0; cv < curPath.size(); cv++) {
                      ss << " '" << curPath[cv]->getFileID() << "'";
                    }
                    ss << " " << std::endl;

                    loop = false;
                    curPath.clear();
                    for (unsigned int tI = 0; tI < vertexesColor.size(); tI++) {
                        vertexesColor[tI] = 0;
                    }
                    for (unsigned int tI = 0; tI < edgesColor.size(); tI++) {
                        edgesColor[tI] = 0;
                    }
                }
            }

            path = curPath;

            //std::cout << " First curPath:";
            //for (unsigned int cv = 0; cv < curPath.size(); cv++) {
            //  std::cout << " '" << curPath[cv]->getFileID() << "'";
            //}
            //std::cout << " " << std::endl;

            firstTime = false;
            int index;
            std::vector<Atom*> tempPath = curPath;
            curPath.clear();
            for (unsigned int q = 0; q < tempPath.size(); q++) {
                for (unsigned int e = 0; e < block.size(); e++) {
                    if (tempPath[q] == block[e]) {
                        index = e;
                    }
                }
                if (nConnections[index] > 2) {
                    if (q != 0) {
                        curAtom = index;
                        nN3 = index;
                        nN3s.push_back(index);
                        curPath.push_back(block[index]);
                        break;
                    } else {
                        curPath.push_back(block[index]);
                    }
                } else {
                    curPath.push_back(block[index]);
                }
            }
            if (curPath.size() == path.size()) done = true;
        } else {
            loop = false;

            if (block[curAtom] != path[path.size() - 1]) {

                ss << " curPath beforeX:";
                for (unsigned int cv = 0; cv < curPath.size(); cv++) {
                  ss << " " << curPath[cv]->getFileID();
                }
                ss << " " << std::endl;

                this->dfs_visitNEW(curAtom, root, 0, loop, block, blockBonds,
                        blockNeighbors, vertexesColor, edgesColor, curPath);

                if (curPath.size() == 0) {
                    curPath = path;
                }
                ss << " curPath afterX:";
                for (unsigned int cv = 0; cv < curPath.size(); cv++) {
                  ss << " " << curPath[cv]->getFileID();
                }
                ss << " " << std::endl;

                if (curPath.size() < path.size()) {
                    this->getNewPath(path, curPath, done);
                    curPath.clear();
                    curPath = path;
                } else {
                    curPath.clear();
                    curPath = path;
                    ss << " curPath reset:";
                    for (unsigned int cv = 0; cv < curPath.size(); cv++) {
                      ss << " " << curPath[cv]->getFileID();
                    }
                    ss << " " << std::endl;
                }
            } else {
                done = true;
            }

            if (!done) {
                std::vector<Atom*> tempPath = curPath;
                curPath.clear();
                int index;
                //int pastPreviousN3 = 0;
                bool newN3 = false;
                for (unsigned int q = 0; q < tempPath.size(); q++) {
                    for (unsigned int e = 0; e < block.size(); e++) {
                        if (tempPath[q] == block[e]) {
                            index = e;
                        }
                    }

                    if (nConnections[index] > 2) {
                        if (q != 0) {
                            std::vector<int>::iterator iVectorIt;
                            iVectorIt = std::find(nN3s.begin(), nN3s.end(), index);
                            if (iVectorIt == nN3s.end()) {
                                curAtom = index;
                                nN3 = index;
                                newN3 = true;
                                nN3s.push_back(index);
                                ss << " adding to curPath: "<< block[index]->getFileID() << std::endl;
                                curPath.push_back(block[index]);
                                break;
                            }
                            ss << " adding to curPath: "<< block[index]->getFileID() << std::endl;
                            curPath.push_back(block[index]);
                        } else {
                            ss << " adding to curPath: "<< block[index]->getFileID() << std::endl;
                            curPath.push_back(block[index]);
                        }
                    } else {
                        ss << " adding to curPath: "<< block[index]->getFileID() << std::endl;
                        curPath.push_back(block[index]);
                    }
                }
                if (!newN3) done = true;
            }
        }
    }
    
    Logger::Instance()->writeToLogFile(ss.str());
}

// ============================================================
// Function : numberRingsInBlock()
// ------------------------------------------------------------
//
// ============================================================

int RingContainer::numberRingsInBlock(std::vector<Atom*> block) {
    std::vector<Bond*> blockBonds;
    std::vector<int> nConnections;

    int numberRings = 0;
    int N3 = 0;
    int N4 = 0;

    this->getBonds(block, cyclicBonds, blockBonds, nConnections); // dangerous use of cyclicBonds
    for (unsigned int j = 0; j < nConnections.size(); j++) {
        if (nConnections[j] == 3) N3++;
        if (nConnections[j] == 4) N4++;
    }
    numberRings = 1 + N3 / 2 + N4;
    return numberRings;
}

// ============================================================
// Function : getNewPath()
// ------------------------------------------------------------
//
// ============================================================

void RingContainer::getNewPath(std::vector<Atom*> &path, std::vector<Atom*> curPath, bool &done) {
    std::stringstream ss;
    std::vector<Atom*> newPath;
    int lastPoint = 0;
    bool error = false;
    for (unsigned int i = 0; i < curPath.size(); i++) {
        newPath.push_back(curPath[i]);
        lastPoint = i;
    }
    bool record = false;
    for (unsigned int i = 0; i < path.size(); i++) {
        if (record) {
            if (pBondContainer->isBonded(path[i], newPath[newPath.size() - 1])) {
                newPath.push_back(path[i]);
            } else {
                // not in sequence
                error = true;
                break;
            }
        }
        if (path[i] == newPath[lastPoint]) {
            record = true;
        }
    }
    if (error) return;

    if (newPath.size() < path.size()) {
        path.clear();
        path = newPath;
    }
    //else {
    //  done = true;
    //}

    ss << " getNewPath: path:";
    for (unsigned int cv = 0; cv < path.size(); cv++) {
      ss << " " << path[cv]->getFileID();
    }
    ss << " ....done = " << done << std::endl;
    
    Logger::Instance()->writeToLogFile(ss.str());
}

// ============================================================
// Function : checkPath()
// ------------------------------------------------------------
//
// ============================================================

void RingContainer::checkPath(std::vector<Atom*> &path, bool &ok) {
    std::stringstream ss;    
    ok = true;
    Bond* pRingBond;
    for (unsigned int i = 0; i < path.size(); i++) {
        //#ifdef DEBUG
        ss << " rings:checkPath ->" << path[i] << " "<< path[i]->getFileID() << std::endl;
        //#endif
        if (i != path.size() - 1) {
            pRingBond = pBondContainer->getBond(path[i], path[i + 1]);
            if (!pRingBond) {
                ok = false;
            }
        } else {
            pRingBond = pBondContainer->getBond(path[i], path[0]);
            if (!pRingBond) {
                ok = false;
            }
        }
    }
    Logger::Instance()->writeToLogFile(ss.str());
}

// ============================================================
// Function : eliminateReducibleAtoms()
// ------------------------------------------------------------
//
// ============================================================

void RingContainer::eliminateReducibleAtoms(std::vector<Atom*> block, std::vector<Atom*> path) {

}

// ============================================================
// Function : dfs_visit()
// ------------------------------------------------------------
//
// ============================================================

void RingContainer::dfs_visit(int curAtom, int rootAtom,
        std::vector<Atom*> &vertexes, std::vector<Bond*> &edges,
        std::vector< std::vector<int> > &neighbors,
        std::vector<int>& vertexesColor, std::vector<int>& edgesColor,
        bool& loop, int& t,
        std::vector<Atom*>& curPath,
        std::vector< std::vector<Atom*> >& paths) {
    vertexesColor[curAtom] = 1; // grey
    std::vector<int> curNeighbors = neighbors[curAtom];
    int curNeighbor;
    int ec = 0;
    int curEdge;

    t++;
    if (!loop) {

        typedef std::vector<int>::iterator iVectorIt;
        iVectorIt start, end;
        start = curNeighbors.begin();
        end = curNeighbors.end();
        std::random_shuffle(start, end);

        for (unsigned int i = 0; i < curNeighbors.size(); i++) {
            curNeighbor = curNeighbors[i];

            for (unsigned int j = 0; j < edges.size(); j++) {
                pBond = edges[j];
                if ((pBond->getAtom1() == vertexes[curAtom] && pBond->getAtom2() == vertexes[curNeighbor]) ||
                        (pBond->getAtom2() == vertexes[curAtom] && pBond->getAtom1() == vertexes[curNeighbor])) {
                    curEdge = j;
                    ec = edgesColor[j];
                    edgesColor[j] = 2; // black
                }
            }
            if (vertexesColor[curNeighbor] == 0) {
                curPath.push_back(vertexes[curNeighbor]);
                this->dfs_visit(curNeighbor, rootAtom, vertexes, edges, neighbors,
                        vertexesColor, edgesColor, loop, t, curPath, paths);
            }

            if (t > 1) {
                if (ec == 0 and vertexesColor[curNeighbor] == 2) {
                    if (pBondContainer->isBonded(vertexes[rootAtom], vertexes[curNeighbor])) {
                        loop = true;
                        paths.push_back(curPath);
                    }
                }
            }
        }
        vertexesColor[curAtom] = 2;
    }
}

// ============================================================
// Function : dfs_visitNEW()
// ------------------------------------------------------------
//
// ============================================================

void RingContainer::dfs_visitNEW(int curAtom, int rootAtom, bool first, 
        bool& loop, std::vector<Atom*>& vertexes, std::vector<Bond*>& edges, 
        std::vector<std::vector<int> >& neighbors, std::vector<int>& vertexesColor, 
        std::vector<int>& edgesColor, std::vector<Atom*>& path){
    vertexesColor[curAtom] = 1; // black
    std::vector<int> curNeighbors = neighbors[curAtom];
    int curNeighbor;
    int curEdge = 0;
    int bondIndex = 0;
    
    std::stringstream ss; 

    if (loop) return;
    //std::cout << " \ncurAtom = " << vertexes[curAtom]->getFileID() << std::endl;

    typedef std::vector<int>::iterator iVectorIt;
    iVectorIt start, end;
    start = curNeighbors.begin();
    end = curNeighbors.end();
    std::random_shuffle(start, end);

    for (unsigned int i = 0; i < curNeighbors.size(); i++) {
        curNeighbor = curNeighbors[i];
        ss << " curNeighbor = " << vertexes[curNeighbor]->getFileID() << std::endl;
        int ec = 0;
        for (unsigned int j = 0; j < edges.size(); j++) {
            pBond = edges[j];
            if (((pBond->getAtom1() == vertexes[curAtom] and pBond->getAtom2() == vertexes[curNeighbor]) or
                    (pBond->getAtom2() == vertexes[curAtom] and pBond->getAtom1() == vertexes[curNeighbor])) and !loop) {
                curEdge = j;
                ss << " edgesColor = " << edgesColor[j] << std::endl;
                ec = edgesColor[j];
                edgesColor[j] = 1; // black
            }
            if ((pBond->getAtom1() == vertexes[rootAtom] and pBond->getAtom2() == vertexes[curNeighbor]) or
                    (pBond->getAtom2() == vertexes[rootAtom] and pBond->getAtom1() == vertexes[curNeighbor])) {
                bondIndex = j;
            }
        }
        if (vertexesColor[curNeighbor] == 0 and !loop) {
            path.push_back(vertexes[curNeighbor]);
            ss << " pushing back1 = "
                      << vertexes[rootAtom]->getFileID() << " "
                      << vertexes[curNeighbor]->getFileID() << " "
//                      << pParent->hasBond(vertexes[rootAtom],vertexes[curNeighbor])
                      << std::endl;

            if (!pBondContainer->isBonded(vertexes[rootAtom], vertexes[curNeighbor])) {
                this->dfs_visitNEW(curNeighbor, rootAtom, first, loop, vertexes, edges, neighbors, vertexesColor, edgesColor, path);
            } else {
                edgesColor[bondIndex] = 1;
                vertexesColor[curNeighbor] = 1; // black
            }
        }

        if (ec == 0 and vertexesColor[curNeighbor] == 1 and !loop) {
            if (first) {
                ss << " rootAtom = " << vertexes[rootAtom]->getFileID() << std::endl;
                ss << " curAtom = " << vertexes[curAtom]->getFileID() << std::endl;
                ss << " curNeighbor = " << vertexes[curNeighbor]->getFileID() << std::endl;
                if (pBondContainer->isBonded(vertexes[rootAtom], vertexes[curNeighbor]) and edgesColor[bondIndex] == 1) {
                    edgesColor[bondIndex] = 1;
                    loop = true;
                } else {
                    edgesColor[curEdge] = 0;
                }
            } else {
                loop = true;
                atomIterator = std::find(path.begin(), path.end(), vertexes[curNeighbor]);
                if (atomIterator == path.end()) {// and vertexesColor[curNeighbor] == 0) {
                    path.push_back(vertexes[curNeighbor]);
                    ss << " pushing back2 = " << vertexes[curNeighbor]->getFileID() << std::endl;
                }
            }
        }
    }
    Logger::Instance()->writeToLogFile(ss.str());
}

// ============================================================
// Function : kekulize()
// ------------------------------------------------------------
// Determine if ring is aromatic or not
// ============================================================
void RingContainer::kekulize() {

    for(unsigned int i=0; i < ringList.size(); ++i){
        this->kekulize(ringList[i]);
    }
}


void RingContainer::kekulize(Ring* r) {
    std::stringstream ss;
    std::vector<Atom*> ringAtoms=r->getAtomList();
//    int ringSize = ringAtoms.size();
    std::vector<Atom*> bondedAtoms;
    double dTorsion = 0.0;
    int planar = 0;
    double largestTorsion = 0.0;
    int huckel = 0;
    std::string symbol = "";
    int formalCharge = 0;

    // Check if planar
    for (unsigned int i = 0; i < ringAtoms.size() - 3; i++) {
        dTorsion = torsion(*(ringAtoms[i]->getCoords()),
                *(ringAtoms[i + 1]->getCoords()),
                *(ringAtoms[i + 2]->getCoords()),
                *(ringAtoms[i + 3]->getCoords()));

        // make all torsion positive (from 0 to 360 degrees)
        if (dTorsion < 0) dTorsion = dTorsion + 2 * PI;
        if (dTorsion > largestTorsion) largestTorsion = dTorsion;

        // 15 degree cutoff (was 10 degrees, change 12 to 18 below)
        if ((std::abs(dTorsion - 0) < PI / 12) or
                (std::abs(dTorsion - PI) < PI / 12) or
                (std::abs(dTorsion - 2 * PI) < PI / 12)) {
            
            ss << ringAtoms[i]->getFileID()   << "-"
                      << ringAtoms[i+1]->getFileID() << "-"
                      << ringAtoms[i+2]->getFileID() << "-"
                      << ringAtoms[i+3]->getFileID() << " :"
                      << dTorsion * RAD2DEG << std::endl;
             
            planar = 1;
            r->setPlanar(true);
        } else {
            planar = 0;
            r->setPlanar(false);
            break;
        }
    }
    ss << "   RingContainer::kekulize, largest ring torsion [15 degree cut-off] =  "
              << largestTorsion * RAD2DEG << std::endl;

    // check to see if ring is nonaromatic
    //  1) no double bonds
    int nonaromatic1 = 1;
    for (unsigned int i = 0; i < ringAtoms.size(); i++) {
        if (i != ringAtoms.size() - 1) {
            pBond = pBondContainer->getBond(ringAtoms[i], ringAtoms[i + 1]);
        } else {
            pBond = pBondContainer->getBond(ringAtoms[i], ringAtoms[0]);
        }
        if (pBond) {
            if (pBond->getAtom1()->getHybridization() == 3 and pBond->getAtom2()->getHybridization()) {
                //        if ((pBond->type == 2) or (pBond->type == 4)) {
                nonaromatic1 = 0;
            }
        }
    }

    //  2) contains quaternary atom (bonded to 4 other heavy atoms)
    int nonaromatic2 = 0;
    int nSaturatedCarbons = 0;

    for (unsigned int i = 0; i < ringAtoms.size(); i++) {
        pAtom = ringAtoms[i];
        bondedAtoms.clear();
        pBondContainer->getBondAtoms(pAtom, bondedAtoms);
        if (bondedAtoms.size() == 4) {
            bool hFound = false;
            for (unsigned int j = 0; j < bondedAtoms.size(); j++) {
                if (bondedAtoms[j]->getSymbol() == "H") hFound = true;
                if (bondedAtoms[j]->getSymbol() == "C") nSaturatedCarbons++;
            }
            if (!hFound) nonaromatic2 = 1;
        }
    }

    //  3) contains more than one saturated carbon
    int nonaromatic3 = 0;
    if (nSaturatedCarbons > 1) {
        nonaromatic3 = 1;
    }

    //  4) contains a monoradical
    int nonaromatic4 = 0;

    ///////////////// THIS CODE DOESN'T WORK ////////////////////

    //  5) contains sulfoxide or sulfone
    /*
         check for:
              sulfoxide  or  sulfone
                  O          O   O
                 ||          \\ //
                 S             S
                / \           / \
         if true then ring cannot be aromatic
     */
    int nonaromatic5 = 0;
    bool ringMember = false;
    for (unsigned int i = 0; i < ringAtoms.size(); i++) {
        if (ringAtoms[i]->getSymbol() == "S") {
            pAtom = ringAtoms[i];
            bondedAtoms.clear();
            pBondContainer->getBondAtoms(pAtom, bondedAtoms);            
            for (unsigned int j = 0; j < bondedAtoms.size(); j++) {
                for (unsigned int x = 0; x < ringAtoms.size(); x++) {
                    if (ringAtoms[x] == bondedAtoms[j]) {
                        ringMember = true;
                    }
                }
                if (!ringMember) {
                    pBond = pBondContainer->getBond(ringAtoms[i], bondedAtoms[j]);
                }
                ringMember = false;
            }
        }
    }
    //////////////////

    int nonaromatic = nonaromatic1 + nonaromatic2 + nonaromatic3 + nonaromatic4 + nonaromatic5;

    
    ss << "   RingContainer::kekulize: nonaromatic1 = " << nonaromatic1 << std::endl;
    ss << "   RingContainer::kekulize: nonaromatic2 = " << nonaromatic2 << std::endl;
    ss << "   RingContainer::kekulize: nonaromatic3 = " << nonaromatic3 << std::endl;
    ss << "   RingContainer::kekulize: nonaromatic5 = " << nonaromatic5 << std::endl;
    ss << "   RingContainer::kekulize: nonaromatic : " << nonaromatic << std::endl;
    ss << "   RingContainer::kekulize: planar : " << planar << std::endl;
    ss << "   RingContainer::kekulize: ringSize : " << ringAtoms.size() << std::endl;
     
    Logger::Instance()->writeToLogFile(ss.str());
    // If its already aromatic, then there is no need to test for aromaticity
    bool doHuckelTest = true;
    unsigned int numAro = 0;
    for (unsigned int i = 0; i < ringAtoms.size(); i++) {
        if (i != ringAtoms.size() - 1) {
            pBond = pBondContainer->getBond(ringAtoms[i], ringAtoms[i + 1]);
            if (pBond) {
                if (pBond->getType() == 4) numAro++;
            }
        } else {
            pBond = pBondContainer->getBond(ringAtoms[i], ringAtoms[0]);
            if (pBond) {
                if (pBond->getType() == 4) numAro++;
            }
        }
    }
    if (numAro == ringAtoms.size()) {
        doHuckelTest = false;
        huckel = 1;
    }


    // if planar, count the number of pi electrons
    if ((planar) and (!nonaromatic) and (ringAtoms.size() > 4) and (doHuckelTest)) {
        int huckelNumbers[8] = {2, 6, 10, 14, 18, 22, 26, 30};
        //int nAromatic = 2 + 4 * ( (int) ( (double)(ringSize-2) * 0.25 + 0.5) );
        int numberPiElectrons = 0;
        for (unsigned int i = 0; i < ringAtoms.size(); i++) {
            symbol = ringAtoms[i]->getSymbol();
            formalCharge = ringAtoms[i]->getFormalCharge();

            if (symbol == "C") {
                if (formalCharge == 1) { // cationic
                    // add zero to numberPiElectrons
                } else if (formalCharge == -1) { // anionic
                    numberPiElectrons += 2;
                } else { // neutral
                    numberPiElectrons++;
                }
            }

            if (symbol == "O") {
                numberPiElectrons += 2;
            }

            if (symbol == "S") {
                numberPiElectrons += 2;
            }

            if (symbol == "N") {
                Bond* pBond1 = 0;
                Bond* pBond2 = 0;
                if (i == 0) {
                    pBond1 = pBondContainer->getBond(ringAtoms[i], ringAtoms[ringAtoms.size() - 1]);
                } else {
                    pBond1 = pBondContainer->getBond(ringAtoms[i], ringAtoms[i - 1]);
                }
                if (i == ringAtoms.size() - 1) {
                    pBond2 = pBondContainer->getBond(ringAtoms[i], ringAtoms[0]);
                } else {
                    pBond2 = pBondContainer->getBond(ringAtoms[i], ringAtoms[i + 1]);
                }
                if (pBond1 and pBond2) {
                    // non-basic, lone pair of electrons delocalized into the pi system, e.g. pyrrole
                    if ((pBond1->getType() == 1) and (pBond2->getType() == 1)) {
                        numberPiElectrons += 2;
                    }                            // basic, lone pair doesn't contribute to pi system, e.g. imidazole
                    else {
                        numberPiElectrons++;
                    }
                }
            }
#ifdef DEBUG
            ss << "   ring::kekulize: numberPiElectrons = " << numberPiElectrons << std::endl;
#endif
        }

        ////// TEST CODE
        // Check for exocyclic pi bonds
        for (unsigned int i = 0; i < ringAtoms.size(); i++) {
            symbol = ringAtoms[i]->getSymbol();
            pAtom = ringAtoms[i];
            bondedAtoms.clear();
            pBondContainer->getBondAtoms(pAtom, bondedAtoms);            
//            bondedAtoms = ringAtoms[i]->getBondedAtoms();
            for (unsigned int j = 0; j < bondedAtoms.size(); j++) {
                for (unsigned int x = 0; x < ringAtoms.size(); x++) {
                    if (ringAtoms[x] == bondedAtoms[j]) {
                        ringMember = true;
                    }
                }
                if (!ringMember) {
                    pBond = pBondContainer->getBond(ringAtoms[i], bondedAtoms[j]);
                    if ((pBond->getType() == 2) and (symbol == "C")) {
                        std::string exoAtom = bondedAtoms[j]->getSymbol();
                        if ((exoAtom == "O") or (exoAtom == "S")) {
                            numberPiElectrons--;
                        }
                    }
                }
                ringMember = false;
            } 
        }
        ////////
        for (int j = 0; j < 8; j++) {
            if (numberPiElectrons == huckelNumbers[j]) {
                huckel = 1;
            }
        }
    }

    /*
        ss << "   huckel = " << huckel << std::endl;
     */

    // if number of pi electrons = 4n+2 => 2,6,10,14,... then its aromatic
    if (planar and huckel) {
        r->setAromatic(true);
        for (unsigned int i = 0; i < ringAtoms.size(); i++) {
            if (i != ringAtoms.size() - 1) {
                pBond = pBondContainer->getBond(ringAtoms[i], ringAtoms[i + 1]);
            } else {
                pBond = pBondContainer->getBond(ringAtoms[i], ringAtoms[0]);
            }

            if (pBond) {
                if (pBond->getType()==1) {
                    pBond->setType(6); // Single or Aromatic
                } else if (pBond->getType() == 2) {
                    pBond->setType(7); // Double or Aromatic
                }
                pBond->setTopology(1); // ring
            } else {
                ss << "RingContainer::kekulize" << " Can't find bond" << std::endl;
                std::stringstream ss;
                ss << "rings::kekulize" << " Can't find bond";
                throw LBindException(ss.str());
            }
            ringAtoms[i]->setType(6); // Aromatic Ring Heavy Atom
            ringAtoms[i]->setHybridization(3); // sp2
        }
    }

    if (planar and !huckel) {
        // if ring contains hetro atoms then make that atom sp2
        for (unsigned int i = 0; i < ringAtoms.size(); i++) {
            symbol = ringAtoms[i]->getSymbol();
            if (symbol != "C") {
                ringAtoms[i]->setHybridization(3);
            }
            if (ringAtoms[i]->getType() != 6) {
                ringAtoms[i]->setType(5);
            }
        }
        r->setAromatic(false);
    }

    if (!planar and !huckel) {
        for (unsigned int i = 0; i < ringAtoms.size(); i++) {
            ringAtoms[i]->setType(5);
        }
        r->setAromatic(false);
    }
    
    if(planar){
        r->setAromatic(true);
    }
//    pParent->bBondTypes2Assigned = true;
    Logger::Instance()->writeToLogFile(ss.str());
}

//// ============================================================
//// Function : calcCentroid()
//// ------------------------------------------------------------
//// Determine the center of the ring
//// ============================================================
//
//void RingContainer::calcCentroid(Ring* r) {
////    unsigned int ringSize = ringAtoms.size();
//    std::vector<Atom*> ringAtoms=r->getAtomList();
//    r->centroid.resize(3);
//
//    for (unsigned int i = 0; i < r->centroid.size(); i++) {
//        r->centroid(i) = 0.0;
//    }
//
//    for (unsigned int i = 0; i < ringAtoms.size(); i++) {
//        Atom* at = ringAtoms[i];
//        Coor3d* coord = at->getCoords();
//        for (unsigned int j = 0; j < 3; j++) {
//            r->centroid(j) += (*coord)[j];
//        }
//    }
//    for (unsigned int i = 0; i < r->centroid.size(); i++) {
//        r->centroid(i) /= double(ringSize);
//    }
//}
//
//// ============================================================
//// Function : getPlaneNormal()
//// ------------------------------------------------------------
//// Determine the plane and normal of the ring
//// ============================================================
//
//int RingContainer::getPlaneNormal(Ring* r) {
//    this->calcCentroid(r);
//
//    unsigned int ringSize = r->atoms.size();
//
//    ublas::matrix<double, ublas::column_major> coordMatrix(ringSize, 3); // rows, columns
//    r->planeNormal.resize(3, 3);
//    ublas::vector<double> R(3);
//
//    // size1 == rows
//    // size2 == columns
//    for (unsigned int i = 0; i < coordMatrix.size1(); i++) {
//        for (unsigned int j = 0; j < coordMatrix.size2(); j++) {
//            coordMatrix(i, j) = 0.0;
//        }
//    }
//
//    for (unsigned int i = 0; i < 3; i++) {
//        R(i) = 0.0;
//        for (unsigned int j = 0; j < 3; j++) {
//            r->planeNormal(i, j) = 0.0;
//        }
//    }
//
//    for (unsigned int i = 0; i < coordMatrix.size1(); i++) { // rows
//        atom* at = r->atoms[i];
//        vector3d* coord = at->getCoords();
//        for (unsigned int j = 0; j < coordMatrix.size2(); j++) { // columns
//            coordMatrix(i, j) = (*coord)[j] - r->centroid(j);
//            ;
//        }
//    }
//
//    r->planeNormal = ublas::prod(ublas::trans(coordMatrix), coordMatrix);
//    int result = diagonalize(r->planeNormal, R);
//    if (result != 0) return 1;
//
//    eigenValueSort(r->planeNormal, R, 1);
//
//#ifdef DEBUG
//    std::string errMessage = "\n RING: ";
//    for (unsigned int i = 0; i < coordMatrix.size1(); i++) { // rows
//        atom* at = r->atoms[i];
//        errMessage += i2s(at->getIndex()) + " ";
//    }
//    errMessage += "\n CENTROID Centroid_" + pParent->getName() + "_" + i2s(r->index);
//    for (unsigned int i = 0; i < r->centroid.size(); i++) {
//        errMessage += " " + d2s(r->centroid(i));
//    }
//    errMessage += " \n";
//
//    /*
//       printing using mat(j,i):
//       eigenvectors:
//        EV1_x EV1_y EV1_z
//        EV2_x EV2_y EV2_z
//        EV3_x EV3_z EV3_z
//
//       eigenvalues:
//        V1 V2 V3
//     */
//    for (unsigned int i = 0; i < 2; i++) {
//        errMessage += " PLANE P_" + pParent->getName() + "_" + i2s(r->index) + "_" + i2s(i + 1);
//        for (unsigned int j = 0; j < 3; j++) {
//            errMessage += " " + d2s(r->planeNormal(j, i) + r->centroid(j));
//        }
//        errMessage += " \n";
//    }
//
//    errMessage += " PLANE N_" + pParent->getName() + "_" + i2s(r->index);
//    for (unsigned int j = 0; j < 3; j++) {
//        errMessage += " " + d2s(r->planeNormal(j, 2) + r->centroid(j));
//    }
//    errMessage += " ";
//    errorLogger.throwError("rings::getPlaneNormal", errMessage, 4);
//#endif
//
//    return 0;
//}

std::vector<Ring*> RingContainer::getRingList(){
    return this->ringList;
}

std::vector<Atom*> RingContainer::getAcyclicAtoms(){
    return this->acyclicAtoms;
}

std::vector<Atom*> RingContainer::getHydrogens(){
    return this->hydrogens;
}

void RingContainer::printRingList(){
    for(unsigned int i=0; i<ringList.size(); ++i){
        ringList[i]->printRing();
        if(ringList[i]->getAromatic()) std::cout << "This is an aromatic ring." << std::endl;
    }
}

void RingContainer::loggingRingList(){
    std::stringstream ss;
    
    for(unsigned int i=0; i<ringList.size(); ++i){
        ringList[i]->loggingRing();
        if(ringList[i]->getAromatic()) ss << "This is an aromatic ring." << std::endl;
    }
    
    Logger::Instance()->writeToLogFile(ss.str());
}
}//namespace LBIND



