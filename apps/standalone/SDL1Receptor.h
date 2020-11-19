//
// Created by Zhang, Xiaohua on 2018-12-17.
//

#ifndef CONVEYORLC_SDL1RECEPTOR_H
#define CONVEYORLC_SDL1RECEPTOR_H


using namespace LBIND;

class JobInputData{

public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & protonateFlg;
        ar & minimizeFlg;
        ar & siteFlg;
        ar & sitebylig;
        ar & forceRedoFlg;
        ar & getPDBflg;
        ar & cutProt;
        ar & ambVersion;
        ar & surfSphNum;
        ar & gridSphNum;
        ar & radius;
        ar & spacing;
        ar & cutoffCoef;
        ar & boxExtend;
        ar & minVol;
        ar & cutRadius;
        ar & dirBuffer;
        ar & subRes;
        ar & recCdtFile;
        ar & keyRes;
        ar & nonRes;
        ar & dockBX;
    }

    bool protonateFlg;
    bool minimizeFlg;
    bool siteFlg;
    bool sitebylig;
    bool forceRedoFlg;
    bool getPDBflg;
    bool cutProt;
    int ambVersion;
    int surfSphNum;
    int gridSphNum;
    double radius;
    double spacing;
    double cutoffCoef;
    double boxExtend;
    double minVol;
    double cutRadius;
    std::string dirBuffer;
    std::string subRes;
    std::string recCdtFile;
    std::vector<std::string> keyRes;
    std::vector<std::string> nonRes;
    std::vector<std::string> dockBX; // User defined box info
};

//struct JobInputData{
//    bool getPDBflg;
//    char dirBuffer[100];
//};

class JobOutData{

public:
    friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & error;
        ar & cutProt;
        ar & clust;
        ar & volume;
        ar & gbEn;
        ar & centroid;
        ar & dimension;
        ar & pdbid;
        ar & pdbFilePath;
        ar & subRes;
        ar & recPath;
        ar & message;
        ar & nonRes;

    }

    bool error;
    bool cutProt;
    int clust;
    double volume;
    double gbEn;
    Coor3d centroid;
    Coor3d dimension;
    std::string pdbid;
    std::string pdbFilePath;
    std::string subRes;
    std::string recPath;
    std::string message;
    std::vector<std::string> nonRes;

};


struct RecData{
    std::string pdbFile;
    std::string subRes;
    std::vector<std::string> keyRes; // key residues to help locate binding site
    std::vector<std::string> nonRes; //non-standard residue list
    std::vector<std::string> dockBX; // User defined box info
};


#endif //CONVEYORLC_SDL1RECEPTOR_H
