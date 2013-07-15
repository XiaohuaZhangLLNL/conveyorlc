/* 
 * File:   AutoDock.h
 * Author: zhang30
 *
 * Created on January 9, 2012, 9:55 AM
 */

#ifndef AUTODOCK_H
#define	AUTODOCK_H

#include <string>
#include <Structure/Coor3d.h>

namespace LBIND {
    class Protein;
class AutoDock {
public:
    AutoDock();
    AutoDock(Protein* pProt);
    AutoDock(const AutoDock& orig);
    virtual ~AutoDock();
    
    void run();
    
    void prepLigands();
    void prepareLigand(std::string pdbid, std::string ligandFName);
    void prepareReceptor(std::string pdbFName);
//    void deprotonReceptor(std::string pdbFName);
//    void antechamber(std::string pdbFName);
    void dms(std::string pdbid);
    void sphgen();
    void sphere_selector(std::string ligname);
    void actSiteDimens();
    void ligandCenter(std::string ligname);
    void prepVinaInput(std::string pdbid, std::string ligName);
    void runVina();
    
private:

    std::string AUTODOCKPATH;
    Protein* pProtein;
    Coor3d actSiteSize;
    Coor3d actSiteCenter;    
};

}//namespace LBIND 
#endif	/* AUTODOCK_H */

