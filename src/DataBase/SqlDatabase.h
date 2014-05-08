/* 
 * File:   SqlDatabase.h
 * Author: zhang
 *
 * Created on May 7, 2014, 11:21 AM
 */

#ifndef SQLDATABASE_H
#define	SQLDATABASE_H

#include <sqlite3.h>
#include <string>
#include "SqlCommon.h"


namespace Sql {

class Database {
public:
    Database(void);
    ~Database(void);

public:
    sqlite3* getHandle();
    std::string errMsg();
    bool open(std::string filename);
    void close();
    bool isOpen();

public:
    bool transactionBegin();
    bool transactionCommit();
    bool transactionRollback();
private:
    sqlite3* _db;
    std::string _err_msg;
    int _result_open;
};
}//namespace Sql 

#endif	/* SQLDATABASE_H */

