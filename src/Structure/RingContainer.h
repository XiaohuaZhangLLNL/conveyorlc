/* 
 * File:   RingContainer.h
 * Author: zhang
 *
 * Created on August 23, 2010, 1:19 PM
 */

#ifndef RINGCONTAINER_H
#define	RINGCONTAINER_H
#include <vector>
#include <map>
#include <fstream>

namespace LBIND{
    class Ring;
    class Atom;
    class Bond;
    class Molecule;
    class Fragment;
    class BondContainer;

class RingContainer {
public:
    RingContainer(Molecule* parent);
//    RingContainer(Fragment* parent);
    RingContainer(const RingContainer& orig);
    virtual ~RingContainer();
    
    //! Determine all rings in the molecule
    void determine();

    //! Kekulize the rings.
    void kekulize();
    /*!
       \brief Determine if ring is aromatic
       \param r ring pointer
    */
    void kekulize(Ring* r);

    /*!
       \brief Determine the plane and normal of the ring
       \param r ring pointer
    */
    int getPlaneNormal(Ring* r);

    /*!
       \brief Determine the centroid of the ring
       \param r ring pointer
    */
    void calcCentroid(Ring* r);   
    
    std::vector<Ring*> getRingList();
    
    std::vector<Atom*> getAcyclicAtoms();
    
    std::vector<Atom*> getHydrogens();
        
    void printRingList();
    void loggingRingList();
    
protected:
    void deleteHydrogen();
    void deleteOpenAcyclic();
    void deleteOpenAcyclicBonds();
    void findNeighbors();
    /*! 
       \brief Find all neighbouring atoms
       \param a vector of atoms
       \param b vector of vector of ints
    */
    void findNeighbors(std::vector<Atom*> a, std::vector< std::vector<int> > &b);

    /*! 
       \brief Find all bonds between atoms in a from all possible in b
       \param a vector of atoms
       \param b vector of bonds
       \param c vector of bonds
       \param d vector of int
    */
    void getBonds(std::vector<Atom*> a, std::vector<Bond*> b, std::vector<Bond*> &c, std::vector<int> &d);

    //! Removes all CLOSED ACYCLIC NODES (atom joining ring systems)
    void deleteClosedAcyclic();

    /*!
      \brief Pick Root Atom
      \param a vector of atoms
      \return index of atom with greatest connectivity
    */
    int pickRootAtom(std::vector<Atom*> a);

    /*!
       \brief Separate cyclic atoms into blocks
    */
    void separateBlocks();

    /*!
       \brief Find the smallest set of smallest rings (SSSR)
    */
    void findSSSR();
    
    void addRing(std::vector<Atom*> r);

    void decomposeBlock(std::vector<Atom*> block);
    void getIrreducibleClosedPath(std::vector<Atom*> block, int root, std::vector<Atom*>& path);
    int numberRingsInBlock(std::vector<Atom*> block);
    void getNewPath(std::vector<Atom*> &path, std::vector<Atom*> curPath, bool &done);
    void eliminateReducibleAtoms(std::vector<Atom*> block, std::vector<Atom*> path);
    void checkPath(std::vector<Atom*> &path, bool &ok);

    /*!
       \brief Depth-First-Search of the molecular graph
       \param curAtom search NODE
       \param rootAtom ROOT NODE
       \param vertexes vector of atom pointers or vertexes
       \param edges vector of Bond pointers or edges
       \param neighbours NODES linking each other
       \param vertexesColor vertex colors used to traverse the molecular graph
       \param edgesColor edge colors used to traverse the molecular graph

       \param loop delete
       \param t delete

       \param curPath storage of the current closed cycle
       \param paths storage for all the paths found
    */ 
    void dfs_visit(int curAtom, int rootAtom,
                   std::vector<Atom*> &vertexes, std::vector<Bond*> &edges,
                   std::vector< std::vector<int> > &neighbours,
                   std::vector<int> &vertexesColor, std::vector<int> &edgesColor,
                   bool &loop,int &t,
                   std::vector<Atom*> &curPath,
                   std::vector< std::vector<Atom*> > &paths);

    void dfs_visitNEW(int curAtom, int rootAtom, bool first, bool &loop,
                      std::vector<Atom*> &vertexes, std::vector<Bond*> &edges,
                      std::vector< std::vector<int> > &neighbors,
                      std::vector<int> &vertexesColor, std::vector<int> &edgesColor,
                      std::vector<Atom*> &path);
    
    
private:
    Molecule* pParent;

    Atom* pAtom;
    Atom* pAtom2;
    
    Bond* pBond;
    Ring* pRing;
    
    BondContainer* pBondContainer;
    
    std::vector<Bond*> bondList;
     
    std::vector<Atom*>::iterator atomIterator;
    // Bond iterator
    std::vector<Bond*>::iterator bondIterator;
            
    std::vector<Atom*> atomList;

    //! Hydrogen atoms
    std::vector<Atom*> hydrogens;

    //! Open and closed acyclic atoms
    std::vector<Atom*> acyclicAtoms;

    //! cyclic atoms
    std::vector<Atom*> cyclicAtoms;

    //! Cyclic Bonds
    std::vector<Bond*> cyclicBonds;
    
    //! Neighboring atoms
    std::vector< std::vector<int> >    neighbors;

    //! vector of blocks
    std::vector< std::vector<Atom*> >  blocks;    
    
    std::vector<Atom*> finalPath;

    std::vector<Ring*> ringList;

};

}//namespace LBIND
#endif	/* RINGCONTAINER_H */

