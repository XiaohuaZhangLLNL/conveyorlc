//
// Created by Zhang, Xiaohua on 2018-12-18.
//

#ifndef CONVEYORLC_UTILS_H
#define CONVEYORLC_UTILS_H

#include <ctime>
#include <string>

inline std::string timeStamp()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%m-%d-%Y %H:%M:%S",timeinfo);
    std::string str(buffer);

    return str;
}

#endif //CONVEYORLC_UTILS_H
