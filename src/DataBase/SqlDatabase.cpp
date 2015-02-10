/* 
 * File:   SqlDatabase.cpp
 * Author: zhang
 * 
 * Created on May 7, 2014, 11:21 AM
 */

#include "SqlDatabase.h"
#include "SqlRecordSet.h"
#include <time.h>


namespace Sql {

Database::Database(void) {
    _db = NULL;
    _result_open = SQLITE_ERROR;

    close();

    tzset();
}

Database::~Database(void) {
    close();
}

sqlite3* Database::getHandle() {
    return _db;
}

std::string Database::errMsg() {
    return _err_msg;
}

void Database::close() {
    if (_db) {
        sqlite3_close(_db);
        _db = NULL;
        _err_msg.clear();
        _result_open = SQLITE_ERROR;
    }
}

bool Database::isOpen() {
    return (_result_open == SQLITE_OK);
}

bool Database::open(std::string filename) {
    close();

    _result_open = sqlite3_open(filename.c_str(), &_db);

    if (isOpen()) {
        return true;
    } else {
        _err_msg = sqlite3_errmsg(_db);
    }

    THROW_EXCEPTION("Database::open: " + errMsg())

    return false;
}

bool Database::transactionBegin() {
    RecordSet rs(_db);

    if (rs.query("BEGIN TRANSACTION"))
        return true;

    return false;
}

bool Database::transactionCommit() {
    RecordSet rs(_db);

    if (rs.query("COMMIT TRANSACTION"))
        return true;

    return false;
}

bool Database::transactionRollback() {
    RecordSet rs(_db);

    if (rs.query("ROLLBACK TRANSACTION"))
        return true;

    return false;
}


}; //namespace Sql 


