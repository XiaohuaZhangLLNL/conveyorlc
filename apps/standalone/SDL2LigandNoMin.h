//
// Created by Zhang, Xiaohua on 5/24/20.
//

#ifndef CONVEYORLC_SDL2LIGANDNOMIN_H
#define CONVEYORLC_SDL2LIGANDNOMIN_H

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
        ar & ambVersion;
        ar & minimizeFlg;
        ar & score_only;
        ar & intDiel;
        ar & dirBuffer;
        ar & sdfBuffer;
        ar & cmpName;
        ar & ligCdtFile;
    }

    int ambVersion;
    bool minimizeFlg;
    bool score_only;
    double intDiel;
    std::string dirBuffer;
    std::string sdfBuffer;
    std::string cmpName;
    std::string ligCdtFile;
};

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
        ar & ligID;
        ar & ligName;
        ar & message;
        ar & pdbStr;
        ar & pdbqtStr;
    }

    bool error;
    std::string ligID;
    std::string ligName;
    std::string message;
    std::string pdbStr;
    std::string pdbqtStr;

};

#endif //CONVEYORLC_SDL2LIGANDNOMIN_H
