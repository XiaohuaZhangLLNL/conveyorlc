/* 
 * File:   SqlField.h
 * Author: zhang
 *
 * Created on May 7, 2014, 11:43 AM
 */

#ifndef SQLFIELD_H
#define	SQLFIELD_H

#include "SqlCommon.h"


namespace Sql {

class Field {
public:
    friend class FieldSet;

private:
    string _name;
    field_use _use;
    field_type _type;
    int _index;
    int _flags;

public:
    Field(field_use use);
    Field(string name, field_type type, int flags = flag_none);
    Field(const Field& value);

public:
    bool isKeyIdField();
    bool isEndingField();

public:
    int getIndex();
    string getName();
    string getTypeStr();
    field_type getType();
    bool isPrimaryKey();
    bool isNotNull();

public:
    string getDefinition();
    static Field* createFromDefinition(string value);

};


}; //namespace Sql 

#endif	/* SQLFIELD_H */

