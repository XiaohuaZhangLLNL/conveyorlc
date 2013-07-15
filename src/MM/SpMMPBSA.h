/* 
 * File:   SpMMPBSA.h
 * Author: zhang30
 *
 * Created on August 31, 2012, 4:01 PM
 */

#ifndef SPMMPBSA_H
#define	SPMMPBSA_H

#include <string>
#include <vector>

namespace LBIND{

// ! Single-Point MM-PB(GB)SA rescoring of vina docking poses.    
class SpMMPBSA {
public:
    SpMMPBSA();
    SpMMPBSA(bool calcPBSA);
    SpMMPBSA(const SpMMPBSA& orig);
    virtual ~SpMMPBSA();

    /**
     * \breif run run the whole re-scoring for one receptor together with one ligand
     * \param dir the directory/receptor name under the WORKDIR
     * \param ligand the ligand name under the ligLibDir
     * 
     */    
    void run(const std::string& dir, const std::string& ligand);
    
    void getbindGB(std::vector<double>& bindgb);
    
    void getbindPB(std::vector<double>& bindpb);
  
private:
    /**
     * \breif recRun prepare pdbqt, tleap and run GB/PB minimization for receptor
     * \param dir the directory/receptor name under the WORKDIR
     * 
     */    
    void recRun(const std::string& dir);
    /**
     * \breif ligRun prepare parameter files and run GB/PB minimization for ligand
     * \param ligand the ligand name under the ligLibDir
     * 
     */    
    void ligRun(const std::string& ligand);
    /**
     * \breif comRun prepare tleap and run GB/PB minimization for complex
     * \param ligand the ligand name under the ligLibDir
     * \param poseID the pose ID (Model number in Vina pdbqt output)
     * 
     */    
    void comRun(const std::string& ligand, int poseID); 
    /**
     * 
     */
    bool energy(const std::string& dir,const std::string& ligand);  
    /**
     * \breif calBind calculate binding energies for poses
     */
    void calBind();
    
    /**
     * \param WORKDIR is the working dierctory containing a list of receptor names;
     * \param ligLibDir directory (WORKDIR/../ligLib) to save ligand parameter files
     * \param calcPB flag to switch between PB and GB calculation
     * 
     */     
    bool calcPB;
    std::string WORKDIR;
    std::string ligLibDir;
    
   /**
    Energy data
    */
    
    double recGBen;
    double recPBen;
    double ligGBen;
    double ligPBen;
    std::vector<double> comGBen;
    std::vector<double> comPBen;    
    std::vector<double> bindGBen;
    std::vector<double> bindPBen;      
};

}//namespace LBIND

#endif	/* SPMMPBSA_H */

