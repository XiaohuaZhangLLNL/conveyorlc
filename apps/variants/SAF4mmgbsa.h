//
// Created by Zhang, Xiaohua on 2018-12-25.
//

#ifndef CONVEYORLC_SAF4MMGBSA_H
#define CONVEYORLC_SAF4MMGBSA_H



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
        ar & key;
    }

    std::string key;

};


#endif //CONVEYORLC_SAF4MMGBSA_H
