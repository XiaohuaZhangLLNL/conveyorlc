/* 
 * File:   Coor3d.h
 * Author: xiaohua
 *
 * Created on May 26, 2009, 10:44 AM
 */

#ifndef _VECTOR3D_H
#define	_VECTOR3D_H

#include <cmath>
#include <iostream>
#include "Constants.h"

#ifdef USE_MPI
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif

namespace LBIND{

class Coor3d {
public:
    Coor3d();
    Coor3d(double coor);
    Coor3d(double coorx, double coory, double coorz);
    Coor3d(const Coor3d& orig);
    virtual ~Coor3d();

    void set(const double coorx, const double coory, const double coorz);
    double getX();
    double getY();
    double getZ();
    double length();
    void normalize();
    double dist(const Coor3d* coord);
    double dist2(const Coor3d* coord);
    double dist2(const Coor3d& coord);
   inline void operator+=(const Coor3d &rhs) {
            x += rhs.z;
            y += rhs.y;
            z += rhs.z; }
   inline void operator-=(const Coor3d &rhs) {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z; }
   inline void operator*=(const double &value) {
            x *= value;
            y *= value;
            z *= value; }
   inline void operator/=(const double &value) {
            x /= value;
            y /= value;
            z /= value; }
    inline Coor3d& operator=(const double &value) {
        x = value;
        y = value;
        z = value;
        return *this; }

   inline friend Coor3d operator+(const Coor3d &lhs, const Coor3d &rhs) {
            return Coor3d( (lhs.x + rhs.x), (lhs.y + rhs.y), (lhs.z + rhs.z) ); }

    inline friend Coor3d operator-(const Coor3d &lhs, const Coor3d &rhs) {
            return Coor3d( (lhs.x - rhs.x), (lhs.y - rhs.y), (lhs.z - rhs.z) ); }

   inline friend Coor3d operator/(const Coor3d &lhs, const double &scalar) {
            return Coor3d( lhs.x/scalar, lhs.y/scalar, lhs.z/scalar ); }
   
   inline friend Coor3d operator*(const Coor3d &lhs, const double &scalar) {
            return Coor3d( scalar*lhs.x, scalar*lhs.y, scalar*lhs.z ); }

   inline friend Coor3d operator*(const double &scalar, const Coor3d &rhs) {
            return Coor3d( scalar*rhs.x, scalar*rhs.y, scalar*rhs.z) ; }

   inline friend double operator*(const Coor3d &lhs, const Coor3d &rhs) {
            return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z; }
   
   inline friend Coor3d cross(const Coor3d &lhs, const Coor3d &rhs) {
            return Coor3d( (lhs.y*rhs.z - rhs.y*lhs.z),
                             (lhs.z*rhs.x - rhs.z*lhs.x),
                             (lhs.x*rhs.y - rhs.x*lhs.y) ); 
   }

   inline friend double torsion(const Coor3d& a, const Coor3d& b,
                                const Coor3d& c, const Coor3d& d) {
            Coor3d ab = a - b;
            Coor3d cb = c - b;
            Coor3d cd = c - d;

            Coor3d p,q;
            p = cross(ab, cb);
            q = cross(cb, cd);

            p.normalize();
            q.normalize();

            double pXq = p*q;
            if (std::abs(pXq) > 1) {
              pXq = 1;
            }
            else if (pXq < - 1) {
              pXq = -1;
            }

            double ang = acos(pXq);
            double s   = cb*(cross(p, q));

            if (s < 0) ang = -ang;

            return ang;   
   }

   
    inline friend bool formRotMatrix(const Coor3d& a, const Coor3d& b, double angle, double rotMat[9]){
        //! coord b directly connect to the atoms that will rotate
        angle=angle*PI/180.0;
        double cosAng = cos(angle);
        double sinAng = sin(angle);
        double t = 1.0 - cosAng;
        Coor3d ab;
        ab=a-b;
//        printCoor3d(a);
//        printCoor3d(b);
//        printCoor3d(ab);
        ab.normalize();
//        printCoor3d(ab);

        // Form rotate matrix
        rotMat[0] = t*ab.getX()*ab.getX() + cosAng;
        rotMat[1] = t*ab.getX()*ab.getY() + sinAng*ab.getZ();
        rotMat[2] = t*ab.getX()*ab.getZ() - sinAng*ab.getY();

        rotMat[3] = t*ab.getX()*ab.getY() - sinAng*ab.getZ();
        rotMat[4] = t*ab.getY()*ab.getY() + cosAng;
        rotMat[5] = t*ab.getY()*ab.getZ() + sinAng*ab.getX();

        rotMat[6] = t*ab.getX()*ab.getZ() + sinAng*ab.getY();
        rotMat[7] = t*ab.getY()*ab.getZ() - sinAng*ab.getX();
        rotMat[8] = t*ab.getZ()*ab.getZ() + cosAng;
        return true;
    }

    inline friend bool formInvertMatrix(const Coor3d& n, const Coor3d& a,
                     const Coor3d& b, const Coor3d& c, double invertMat[9]){
        //! coord n is the nitrogen atom, a, b, c are atoms bond with nitrogen atom.
        //! coord c directly connect to the atoms that will invert
        //! v1 is

        Coor3d an=a-n;
        Coor3d bn=b-n;
        Coor3d cn=c-n;
        an.normalize();
        bn.normalize();
        cn.normalize();

        Coor3d ab;
        ab=a-b;
//        printCoor3d(ab);
        ab.normalize();
//        printCoor3d(ab);

        Coor3d v1=an+bn;
        v1 *= -1;
        v1.normalize();

        //angle=angle*PI/180.0;
        double angle= 2*acos(v1*cn);
        double cosAng = cos(angle);
        double sinAng = sin(angle);
        double t = 1.0 - cosAng;

        // Form rotate matrix
        invertMat[0] = t*ab.getX()*ab.getX() + cosAng;
        invertMat[1] = t*ab.getX()*ab.getY() + sinAng*ab.getZ();
        invertMat[2] = t*ab.getX()*ab.getZ() - sinAng*ab.getY();

        invertMat[3] = t*ab.getX()*ab.getY() - sinAng*ab.getZ();
        invertMat[4] = t*ab.getY()*ab.getY() + cosAng;
        invertMat[5] = t*ab.getY()*ab.getZ() + sinAng*ab.getX();

        invertMat[6] = t*ab.getX()*ab.getZ() + sinAng*ab.getY();
        invertMat[7] = t*ab.getY()*ab.getZ() - sinAng*ab.getX();
        invertMat[8] = t*ab.getZ()*ab.getZ() + cosAng;
        return true;
    }
    
    inline friend bool formFlapMatrix(const Coor3d& a, const Coor3d& cd, const Coor3d& ce,  double flapMat[9]){
        //! coord b directly connect to the atoms that will rotate
        Coor3d ab=a;
        double angle= 2*acos(cd*ce);
//        std::cout << "\t\tformFlapMatrix ... angle =" << angle*180/PI << " cos=" << cd*ce << std::endl;
        double cosAng = cos(angle);
        double sinAng = sin(angle);
        double t = 1.0 - cosAng;

        // Form rotate matrix
        flapMat[0] = t*ab.getX()*ab.getX() + cosAng;
        flapMat[1] = t*ab.getX()*ab.getY() + sinAng*ab.getZ();
        flapMat[2] = t*ab.getX()*ab.getZ() - sinAng*ab.getY();

        flapMat[3] = t*ab.getX()*ab.getY() - sinAng*ab.getZ();
        flapMat[4] = t*ab.getY()*ab.getY() + cosAng;
        flapMat[5] = t*ab.getY()*ab.getZ() + sinAng*ab.getX();

        flapMat[6] = t*ab.getX()*ab.getZ() + sinAng*ab.getY();
        flapMat[7] = t*ab.getY()*ab.getZ() - sinAng*ab.getX();
        flapMat[8] = t*ab.getZ()*ab.getZ() + cosAng;
        return true;
    }    

    void rotate(const double rotMat[9], const Coor3d& o);

    inline friend void printCoor3d(const Coor3d& coord){
        std::cout << "Coordinate: "
                << " x= " << coord.x
                << " y= " << coord.y
                << " z= " << coord.z << std::endl;
    }
   inline friend std::ostream& operator<< (std::ostream& os, Coor3d& coord) {
      os << coord.x << " " << coord.y  << " " << coord.z << std::endl;
      return os;
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
        ar & x;
        ar & y;
        ar & z;
    }
#endif
    double x;
    double y;
    double z;

};

}// namespace LBIND

#endif	/* _VECTOR3D_H */

