/* 
 * File:   TestSqlField.cpp
 * Author: zhang
 *
 * Created on May 7, 2014, 3:59 PM
 */

#include <iostream>

#include "src/DataBase/SqlField.h"



using namespace Sql;

void FieldKeySetup() {
    Field f(FIELD_KEY);

    std::cout << "Test FieldKeySetup ..." << std::endl;

    if (f.getDefinition() == "_ID INTEGER PRIMARY KEY") {
        std::cout << "f.getDefinition() pass!" << std::endl;
    } else {
        std::cout << "f.getDefinition() fail!" << std::endl;
    }
    //	CHECK_EQUAL("_ID INTEGER PRIMARY KEY", f.getDefinition());
    if (f.getName() == "_ID") {
        std::cout << "f.getName() pass!" << std::endl;
    } else {
        std::cout << "f.getName() fail!" << std::endl;
    }
    //	CHECK_EQUAL("_ID", f.getName());
    //	CHECK_EQUAL(type_int, f.getType());
    if (f.getType() == type_int) {
        std::cout << "f.getType() pass!" << std::endl;
    } else {
        std::cout << "f.getType() fail!" << std::endl;
    }
    //	CHECK_EQUAL("INTEGER", f.getTypeStr());
    if (f.getTypeStr() == "INTEGER") {
        std::cout << "f.getTypeStr() pass!" << std::endl;
    } else {
        std::cout << "f.getTypeStr() fail!" << std::endl;
    }
    //	CHECK(!f.isEndingField());
    if (!f.isEndingField()) {
        std::cout << "f.isEndingField() pass!" << std::endl;
    } else {
        std::cout << "f.isEndingField() fail!" << std::endl;
    }
    //	CHECK(f.isKeyIdField());
    if (f.isKeyIdField()) {
        std::cout << "f.isKeyIdField() pass!" << std::endl;
    } else {
        std::cout << "f.isKeyIdField() fail!" << std::endl;
    }
    //	CHECK(f.isPrimaryKey());
    if (f.isPrimaryKey()) {
        std::cout << "f.isPrimaryKey() pass!" << std::endl;
    } else {
        std::cout << "f.isPrimaryKey() fail!" << std::endl;
    }
}

void FieldEndSetup() {
    std::cout << "Test FieldEndSetup ..." << std::endl;
    Field f(DEFINITION_END);

    //	CHECK_EQUAL("", f.getDefinition());	
    //	CHECK_EQUAL("", f.getName());
    //	CHECK_EQUAL(type_undefined, f.getType());
    //	CHECK_EQUAL("", f.getTypeStr());
    //	CHECK(f.isEndingField());

    if (f.getDefinition() == "") {
        std::cout << "f.getDefinition() pass!" << std::endl;
    } else {
        std::cout << "f.getDefinition() fail!" << std::endl;
    }
    if (f.getName() == "") {
        std::cout << "f.getName() pass!" << std::endl;
    } else {
        std::cout << "f.getName() fail!" << std::endl;
    }
    if (f.getType() == type_undefined) {
        std::cout << "f.getType() pass!" << std::endl;
    } else {
        std::cout << "f.getType() fail!" << std::endl;
    }
    if (f.getTypeStr() == "") {
        std::cout << "f.getTypeStr() pass!" << std::endl;
    } else {
        std::cout << "f.getTypeStr() fail!" << std::endl;
    }
    if (f.isEndingField()) {
        std::cout << "f.isEndingField() pass!" << std::endl;
    } else {
        std::cout << "f.isEndingField() fail!" << std::endl;
    }

}

void FieldsDefinition() {
    std::cout << "Test FieldsDefinition ..." << std::endl;
    Field def[] ={
        Field(FIELD_KEY),
        Field("name", type_text, flag_not_null),
        Field("valueInt", type_int),
        Field("valueDbl", type_float),
        Field("valueTxt", type_text),
        Field("valueBol", type_bool, flag_not_null),
        Field("valueTme", type_time),
        Field(DEFINITION_END),
    };

    //	CHECK_EQUAL("name TEXT NOT NULL", def[1].getDefinition());
    //	CHECK_EQUAL("valueInt INTEGER", def[2].getDefinition());
    //	CHECK_EQUAL("valueDbl REAL", def[3].getDefinition());
    //	CHECK_EQUAL("valueTxt TEXT", def[4].getDefinition());
    //	CHECK_EQUAL("valueBol INTEGER NOT NULL", def[5].getDefinition());
    //	CHECK_EQUAL("valueTme INTEGER", def[6].getDefinition());
    if (def[1].getDefinition() == "name TEXT NOT NULL") {
        std::cout << "def[1].getDefinition() pass!" << std::endl;
    } else {
        std::cout << "def[1].getDefinition() fail!" << std::endl;
    }

    if (def[2].getDefinition() == "valueInt INTEGER") {
        std::cout << "def[2].getDefinition() pass!" << std::endl;
    } else {
        std::cout << "def[2].getDefinition() fail!" << std::endl;
    }

    if (def[3].getDefinition() == "valueDbl REAL") {
        std::cout << "def[3].getDefinition() pass!" << std::endl;
    } else {
        std::cout << "def[3].getDefinition() fail!" << std::endl;
    }

    if (def[4].getDefinition() == "valueTxt TEXT") {
        std::cout << "def[4].getDefinition() pass!" << std::endl;
    } else {
        std::cout << "def[4].getDefinition() fail!" << std::endl;
    }

    if (def[5].getDefinition() == "valueBol INTEGER NOT NULL") {
        std::cout << "def[5].getDefinition() pass!" << std::endl;
    } else {
        std::cout << "def[5].getDefinition() fail!" << std::endl;
    }

    if (def[6].getDefinition() == "valueTme INTEGER") {
        std::cout << "def[6].getDefinition() pass!" << std::endl;
    } else {
        std::cout << "def[6].getDefinition() fail!" << std::endl;
    }
}

void FieldCreation() {
    std::cout << "Test FieldCreation ..." << std::endl;
    Field f(FIELD_KEY);

    //	CHECK_EQUAL("_ID INTEGER PRIMARY KEY", f.getDefinition());	
    if (f.getDefinition() == "_ID INTEGER PRIMARY KEY") {
        std::cout << "f.getDefinition() pass!" << std::endl;
    } else {
        std::cout << "f.getDefinition() fail!" << std::endl;
    }

    Field f1(f);

    //	CHECK_EQUAL("_ID INTEGER PRIMARY KEY", f1.getDefinition());
    if (f1.getDefinition() == "_ID INTEGER PRIMARY KEY") {
        std::cout << "f1.getDefinition() pass!" << std::endl;
    } else {
        std::cout << "f1.getDefinition() fail!" << std::endl;
    }
}

void FieldCreationFromString() {
    std::cout << "Test FieldCreationFromString ..." << std::endl;
    Field* f = Field::createFromDefinition("_ID INTEGER PRIMARY KEY");

    //	CHECK(f != NULL);
    if (f != NULL) {
        std::cout << "Field::createFromDefinition pass!" << std::endl;
    } else {
        std::cout << "Field::createFromDefinition fail!" << std::endl;
    }
    if (f) {
        //		CHECK_EQUAL("_ID INTEGER PRIMARY KEY", f->getDefinition());
        if (f->getDefinition() == "_ID INTEGER PRIMARY KEY") {
            std::cout << "f->getDefinition() pass!" << std::endl;
        } else {
            std::cout << "f->getDefinition() fail!" << std::endl;
        }
        delete f;
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    FieldKeySetup();
    FieldEndSetup();
    FieldsDefinition();
    FieldCreation();
    FieldCreationFromString();

    return 0;
}

