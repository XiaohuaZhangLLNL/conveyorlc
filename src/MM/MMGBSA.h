/* 
 * File:   MMGBSA.h
 * Author: zhang
 *
 * Created on April 22, 2014, 4:27 PM
 */

#ifndef MMGBSA_H
#define	MMGBSA_H

#include <string>
#include <vector>

namespace LBIND{

class MMGBSA {
public:
    MMGBSA(const std::string& dir, const std::string& ligand, const std::string& workDir);
    MMGBSA(const std::string& dir, const std::string& ligand, std::vector<std::string>& nonStdRes, const std::string& workDir);
    MMGBSA(const MMGBSA& orig);
    virtual ~MMGBSA();

    /**
     * \breif run run the whole re-scoring for one receptor together with one ligand
     * \param dir the directory/receptor name under the WORKDIR
     * \param ligand the ligand name under the ligLibDir
     * 
     */    
    void run(std::string& poseID, bool restart);
    
    double getbindGB();
    double getScore();
  
private:
 
//    void ligRun(const std::string& ligand);
    /**
     * \breif comRun prepare tleap and run GB/PB minimization for complex
     * \param ligand the ligand name under the ligLibDir
     * \param poseID the pose ID (Model number in Vina pdbqt output)
     * 
     */    
//    void comRun(int poseID); 
    
//    /**
//     * 
//     */
////    bool energy(const std::string& dir,const std::string& ligand);  
//    /**
//     * \breif calBind calculate binding energies for poses
//     */
//    void calBind();
    
    /**
     * \param WORKDIR is the working dierctory containing a list of receptor names;
     * \param ligLibDir directory (WORKDIR/../ligLib) to save ligand parameter files
     * \param calcPB flag to switch between PB and GB calculation
     * 
     */     
    std::string WORKDIR;
    std::string recID;
    std::string ligID;
    std::vector<std::string> nonRes;
    
   /**
    Energy data
    */
    
    double ligGBen;
    double bindGBen;
    double score;

};

}//namespace LBIND
#endif	/* MMGBSA_H */

