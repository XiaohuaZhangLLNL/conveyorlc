/* 
 * File:   Coordinates.h
 * Author: zhang
 *
 * Created on December 3, 2010, 2:21 PM
 */

#ifndef COORDINATES_H
#define	COORDINATES_H

#include <vector>
#include "Coor3d.h"

#ifdef USE_MPI
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif

namespace LBIND{
    
class Coordinates {
public:
    Coordinates();
    Coordinates(const Coordinates& orig);
    virtual ~Coordinates();

    void setDelFlg(bool delFlg);
    bool getDelFlg();

    void setEnergy(double energy);
    double getEnergy();

    void setFreeE(double freeE);
    double getFreeE();
    
    void setEnthalpy(double enthalpy);
    double getEnthalpy();
    
    void setEntropy(double entropy);
    double getEntropy();    

    void getCoordinates(std::vector<Coor3d*>& atmCoorList);
    void addCoor3d(Coor3d* pCoor);

//    friend std::ostream& operator<<(std::ostream& os, const Coordinates& p);

    friend std::ostream& operator <<(std::ostream& os, const Coordinates& p){
        os << "Energy: " <<  p.itsEnergy <<" Kcal/mol" << std::endl;
        for (unsigned int i = 0; i < p.itsCoordinates.size(); ++i)
        {
            os << " " << *(p.itsCoordinates[i]);
        }

        return os;  // for multiple << operators.
    }
private:
#ifdef USE_MPI
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & itsDelFlg;
        ar & itsEnergy;
        ar & itsFreeE;
        ar & itsEntropy;
        ar & itsEnthalpy;
        ar & itsCoordinates;
    }
#endif
    bool itsDelFlg;
    double itsEnergy;
    double itsFreeE;
    double itsEntropy;
    double itsEnthalpy;
    std::vector<Coor3d*> itsCoordinates;
    
};

}//namespace LBIND
#endif	/* COORDINATES_H */

