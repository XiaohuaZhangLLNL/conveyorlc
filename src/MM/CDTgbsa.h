//
// Created by Zhang, Xiaohua on 2018-12-26.
//

#ifndef CONVEYORLC_CDTMMGBSA_H
#define CONVEYORLC_CDTMMGBSA_H

// It is ugly


namespace LBIND{


struct CDTmeta{
    int version;
    std::string dockInDir;
    std::string recFile;
    std::string ligFile;
    std::string ligName;

    std::string workDir;
    std::string localDir;
    std::string inputDir;
    std::string dataPath;

    std::string poseDir;

    std::string key;
    int procID;

    bool error;
    bool score_only;
    bool newapp;
    bool minimize;
    bool useScoreCF; //switch to turn on score cutoff
    double scoreCF;  // value for score cutoff
    double intDiel;
    double dockscore;
    double gbbind;
    std::string recID;
    std::string ligID;
    std::string poseID;
    std::string message;

    double ligGB;
    double recGB;
    double comGB;

    std::vector<std::string> nonRes;

};

class CDTgbsa {
public:
    CDTgbsa();

    CDTgbsa(const CDTgbsa& orig);
    virtual ~CDTgbsa();

    static void run(CDTmeta & cdtMeta);
    static void runNew(CDTmeta & cdtMeta);

private:
    static void getLigData(CDTmeta & cdtMeta);
    static void getRecData(CDTmeta & cdtMeta);
    static void getDockData(CDTmeta & cdtMeta);

    static void ligMinimize(CDTmeta &cdtMeta);

};

}//namespace LBIND

#endif //CONVEYORLC_CDTMMGBSA_H
