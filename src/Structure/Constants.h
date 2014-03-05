/* 
 * File:   Constants.h
 * Author: zhang
 *
 * Created on August 26, 2010, 3:02 PM
 */

#ifndef CONSTANTS_H
#define	CONSTANTS_H
namespace LBIND{

    const double BONDTOLERANCE=0.4;
    const double PI=3.1415926;
    const double RKCAL=1.9858775;//gas constant kcal·K−1·mol−1
    const double RT=0.591791494;   //1.9858775*298/1000 kcal/mol
    
    const double RAD2DEG=180.0/PI;

    const double NEARZERO = 1.e-6;

    // Hartree to kcal/mol
    const double H2KCAL=627.509;
    // conversion factor from cm^-1 to Hz
    const double CM2HZ = 2.99792458e10; // 10**6.0*2.99792458*10**4.0;
    // conversion factor from au in kg
    const double AU2KG = 1.66053873e-27;
    // Avogadro constant
    const double NA = 6.0221415e23;
    // conversion of kj/mol to H
    const double KJM2H = 3.80879838e-4;
    // Hartree to J
    const double H2J = 4.359744e-18;
    // conversion of m to Angstroem
    const double M2ANGS = 10e10;

    // Boltzmann constant J/K
    const double  KB = 1.38066e-23;
    // Planck's constant J s
    const double HPLANCK = 6.626218e-34;
    const double HBAR = HPLANCK/2.0/PI;

    const double BIGPOSITIVE=9999999.0;
    const double BIGNEGTIVE=-9999999.0;
//    const double ECONSTANT=2.71828182846;

}//namespace LBIND

#endif	/* CONSTANTS_H */

