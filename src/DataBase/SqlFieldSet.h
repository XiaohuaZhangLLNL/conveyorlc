/* 
 * File:   SqlFieldSet.h
 * Author: zhang
 *
 * Created on May 7, 2014, 11:51 AM
 */

#ifndef SQLFIELDSET_H
#define	SQLFIELDSET_H

#include <map>
#include <vector>
#include "SqlField.h"


namespace Sql {

class FieldSet {
private:
    std::vector<Field> _vec;
    std::map<string, Field*> _map;

private:
    void copy(const std::vector<Field>& definition);

public:
    FieldSet(Field* definition);
    FieldSet(std::vector<Field>& definition);
    FieldSet(const FieldSet& source);

public:
    string toString();
    int count();
    Field* getByIndex(int index);
    Field* getByName(string name);

public:
    string definitionHash();
    string getDefinition();
    static FieldSet* createFromDefinition(string value);

};


}; //namespace Sql 


#endif	/* SQLFIELDSET_H */

