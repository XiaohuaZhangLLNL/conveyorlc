/* 
 * File:   SqlRecord.h
 * Author: zhang
 *
 * Created on May 7, 2014, 11:59 AM
 */

#ifndef SQLRECORD_H
#define	SQLRECORD_H

#include <vector>
#include "SqlCommon.h"
#include "SqlValue.h"
#include "SqlFieldSet.h"


namespace Sql {

class Record {
private:
    FieldSet* _fields;
    std::vector<Value> _values;

public:
    Record(FieldSet* fields);
    Record(Record* record);
    Record(const Record& record);

private:
    friend class RecordSet;

    void initColumnCount(int columns);
    void initColumnValue(int column_index, char* value, field_type type);

public:
    int columnCount();
    Value* getValue(int column_index);
    Value* getValue(string fieldName);
    Value* getKeyIdValue();
    Field* fieldByName(string fieldName);
    FieldSet* fields();
    string toString();
    string toSql();
    bool equalsColumnValue(Record* record, string fieldName);
    bool equalsValues(Record* record);

public:
    string toSqlInsert(string tableName);
    string toSqlUpdate(string tableName);

public:
    void setNull(int index);
    void setString(int index, string value);
    void setInteger(int index, integer value);
    void setDouble(int index, double value);
    void setBool(int index, bool value);
    void setTime(int index, time value);

public:
    void setNull(string fieldName);
    void setString(string fieldName, string value);
    void setInteger(string fieldName, integer value);
    void setDouble(string fieldName, double value);
    void setBool(string fieldName, bool value);
    void setTime(string fieldName, time value);

};



}; //namespace Sql


#endif	/* SQLRECORD_H */

