/* 
 * File:   dockBMPI.h
 * Author: zhang30
 *
 * Created on August 14, 2012, 1:53 PM
 */

#ifndef DOCKING_H
#define	DOCKING_H

#include <string>
#include <vector>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>



class JobInputData{
    
public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & useScoreCF;
        ar & flexible;
        ar & randomize;
        ar & score_only;
        ar & local_only;
        ar & randomize_only;
        ar & cpu;
        ar & exhaustiveness;
        ar & num_modes;
        ar & seed;
        ar & scoreCF; 
        ar & energy_range;
        ar & min_rmsd;
        ar & granularity;
        ar & key;
        ar & recFile;
        ar & ligFile;
        ar & comFile;
        ar & outFile;
    }

    bool useScoreCF; //switch to turn on score cutoff
    bool flexible;
    bool randomize;
    bool score_only;
    bool local_only;
    bool randomize_only;
    int cpu;
    int exhaustiveness;
    int num_modes;
//    int mc_mult;
    int seed;
    double scoreCF;  // value for score cutoff     
    double energy_range;
    double min_rmsd;
    double granularity;
    std::string key;
    std::string recFile;
    std::string ligFile;
    std::string comFile;
    std::string outFile;
};

struct JobOutData{

public:
    bool error;
    int numPose;
    std::string pdbID;
    std::string ligID;
    std::string ligName;
    std::string dockDir;
    std::string mesg;
    std::vector<double> scores;
    std::string scorelog;
    std::string pdbqtfile;
};

void dockjob(JobInputData& jobInput, JobOutData& jobOut, std::string& workDir);

bool getScores(std::string& log, std::vector<double>& scores);

#endif	/* DOCKING_H */

