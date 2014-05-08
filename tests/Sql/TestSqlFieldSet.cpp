/* 
 * File:   TestSqlFieldSet.cpp
 * Author: zhang
 *
 * Created on May 7, 2014, 4:36 PM
 */

#include <iostream>
#include "src/DataBase/SqlField.h"
#include "src/DataBase/SqlFieldSet.h"


using namespace Sql;

static Field* definition() {
    static Field def[] ={
        Field(FIELD_KEY),
        Field("name", type_text, flag_not_null),
        Field("valueInt", type_int),
        Field("valueDbl", type_float),
        Field("valueTxt", type_text),
        Field("valueBol", type_bool, flag_not_null),
        Field("valueTme", type_time),
        Field(DEFINITION_END),
    };

    return &def[0];
}

void FieldSetTest() {
    std::cout << "Test FieldSet ..." << std::endl;
    Sql::FieldSet fields(definition());

    //	CHECK_EQUAL(7, fields.count());

    if (fields.count() == 7) {
        for (int index = 0; index < 7; index++) {
            //			CHECK_EQUAL(index, fields.getByIndex(index)->getIndex());
            if (fields.getByIndex(index)->getIndex() == index) {
                std::cout << "fields.getByIndex(" << index << ")->getIndex() pass!" << std::endl;
            } else {
                std::cout << "fields.getByIndex(" << index << ")->getIndex() fail!" << std::endl;
            }
        }
    }

    //	CHECK(fields.getByName("name"));
    //	CHECK(fields.getByName("valueInt"));
    //	CHECK(fields.getByName("valueDbl"));
    //	CHECK(fields.getByName("valueTxt"));
    //	CHECK(fields.getByName("valueBol"));
    //	CHECK(fields.getByName("valueTme"));
    if (fields.getByName("name")) {
        std::cout << "fields.getByName(\"name\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"name\") fail!" << std::endl;
    }
    if (fields.getByName("valueInt")) {
        std::cout << "fields.getByName(\"valueInt\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueInt\") fail!" << std::endl;
    }
    if (fields.getByName("valueDbl")) {
        std::cout << "fields.getByName(\"valueDbl\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueDbl\") fail!" << std::endl;
    }
    if (fields.getByName("valueTxt")) {
        std::cout << "fields.getByName(\"valueTxt\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueTxt\") fail!" << std::endl;
    }
    if (fields.getByName("valueBol")) {
        std::cout << "fields.getByName(\"valueBol\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueBol\") fail!" << std::endl;
    }
    if (fields.getByName("valueTme")) {
        std::cout << "fields.getByName(\"valueTme\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueTme\") fail!" << std::endl;
    }

    //	CHECK_EQUAL("name", (fields.getByName("name") ? fields.getByName("name")->getName() : "*"));
    //	CHECK_EQUAL("valueInt", (fields.getByName("valueInt") ? fields.getByName("valueInt")->getName() : "*"));
    //	CHECK_EQUAL("valueDbl", (fields.getByName("valueDbl") ? fields.getByName("valueDbl")->getName() : "*"));
    //	CHECK_EQUAL("valueTxt", (fields.getByName("valueTxt") ? fields.getByName("valueTxt")->getName() : "*"));
    //	CHECK_EQUAL("valueBol", (fields.getByName("valueBol") ? fields.getByName("valueBol")->getName() : "*"));
    //	CHECK_EQUAL("valueTme", (fields.getByName("valueTme") ? fields.getByName("valueTme")->getName() : "*"));

    Sql::string str1 = (fields.getByName("name") ? fields.getByName("name")->getName() : "*");
    Sql::string str2 = (fields.getByName("valueInt") ? fields.getByName("valueInt")->getName() : "*");
    Sql::string str3 = (fields.getByName("valueDbl") ? fields.getByName("valueDbl")->getName() : "*");
    Sql::string str4 = (fields.getByName("valueTxt") ? fields.getByName("valueTxt")->getName() : "*");
    Sql::string str5 = (fields.getByName("valueBol") ? fields.getByName("valueBol")->getName() : "*");
    Sql::string str6 = (fields.getByName("valueTme") ? fields.getByName("valueTme")->getName() : "*");

    if (str1 == "name") {
        std::cout << "fields.getByName(\"name\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"name\") fail!" << std::endl;
    }
    if (str2 == "valueInt") {
        std::cout << "fields.getByName(\"valueInt\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueInt\") fail!" << std::endl;
    }
    if (str3 == "valueDbl") {
        std::cout << "fields.getByName(\"valueDbl\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueDbl\") fail!" << std::endl;
    }
    if (str4 == "valueTxt") {
        std::cout << "fields.getByName(\"valueTxt\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueTxt\") fail!" << std::endl;
    }
    if (str5 == "valueBol") {
        std::cout << "fields.getByName(\"valueBol\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueBol\") fail!" << std::endl;
    }
    if (str6 == "valueTme") {
        std::cout << "fields.getByName(\"valueTme\") pass!" << std::endl;
    } else {
        std::cout << "fields.getByName(\"valueTme\") fail!" << std::endl;
    }
}

void FieldSetCreation() {
    std::cout << "Test FieldSetCreation ..." << std::endl;
    Sql::FieldSet fields(definition());

    Sql::string strdef;
    strdef += "_ID INTEGER PRIMARY KEY, ";
    strdef += "name TEXT NOT NULL, ";
    strdef += "valueInt INTEGER, ";
    strdef += "valueDbl REAL, ";
    strdef += "valueTxt TEXT, ";
    strdef += "valueBol INTEGER NOT NULL, ";
    strdef += "valueTme INTEGER";

    //	CHECK_EQUAL(strdef, fields.getDefinition());	
    //	CHECK_EQUAL("63f1ae61aad81ca9e98eb7c9c643c55197aab28d", fields.definitionHash());	

    if (fields.getDefinition() == strdef) {
        std::cout << "fields.getDefinition() pass!" << std::endl;
    } else {
        std::cout << "fields.getDefinition() fail!" << std::endl;
    }

    if (fields.definitionHash() == "63f1ae61aad81ca9e98eb7c9c643c55197aab28d") {
        std::cout << "fields.definitionHash() pass!" << std::endl;
    } else {
        std::cout << "fields.definitionHash() fail!" << std::endl;
    }
    Sql::FieldSet fields_copy(fields);

    //	CHECK_EQUAL(fields_copy.definitionHash(), fields.definitionHash());
    if (fields.definitionHash() == fields_copy.definitionHash()) {
        std::cout << "fields_copy.definitionHash() pass!" << std::endl;
    } else {
        std::cout << "fields_copy.definitionHash() fail!" << std::endl;
    }
}

void FieldSetCreationFromString() {
    std::cout << "Test FieldSetCreationFromString ..." << std::endl;
    Sql::string strdef;
    strdef += "_ID INTEGER PRIMARY KEY, ";
    strdef += "name TEXT NOT NULL, ";
    strdef += "valueInt INTEGER, ";
    strdef += "valueDbl REAL, ";
    strdef += "valueTxt TEXT, ";
    strdef += "valueBol INTEGER NOT NULL, ";
    strdef += "valueTme INTEGER";

    Sql::FieldSet* fields = FieldSet::createFromDefinition(strdef);

    //	CHECK(fields != NULL);
    if (fields != NULL) {
        std::cout << "Field::createFromDefinition pass!" << std::endl;
    } else {
        std::cout << "Field::createFromDefinition fail!" << std::endl;
    }
    if (fields) {
        //		CHECK_EQUAL(strdef, fields->getDefinition());
        //		CHECK_EQUAL("63f1ae61aad81ca9e98eb7c9c643c55197aab28d", fields->definitionHash());
        if (fields->getDefinition() == strdef) {
            std::cout << "fields->getDefinition() pass!" << std::endl;
        } else {
            std::cout << "fields->getDefinition() fail!" << std::endl;
        }

        if (fields->definitionHash() == "63f1ae61aad81ca9e98eb7c9c643c55197aab28d") {
            std::cout << "fields->definitionHash() pass!" << std::endl;
        } else {
            std::cout << "fields->definitionHash() fail!" << std::endl;
        }
        delete fields;
    }
}

void FieldSetFieldsOrder() {
    std::cout << "Test FieldSetFieldsOrder ..." << std::endl;
    Sql::FieldSet fields(definition());

    //	CHECK_EQUAL(7, fields.count());
    if (fields.count() == 7) {
        std::cout << "fields.count() pass!" << std::endl;
    } else {
        std::cout << "fields.count() fail!" << std::endl;
    }

    //check if fields by name and by index are the same
    //	CHECK_EQUAL(fields.getByName("_ID")->getName(), fields.getByIndex(0)->getName());
    //	CHECK_EQUAL(fields.getByName("name")->getName(), fields.getByIndex(1)->getName());
    //	CHECK_EQUAL(fields.getByName("valueInt")->getName(), fields.getByIndex(2)->getName());
    //	CHECK_EQUAL(fields.getByName("valueDbl")->getName(), fields.getByIndex(3)->getName());
    //	CHECK_EQUAL(fields.getByName("valueTxt")->getName(), fields.getByIndex(4)->getName());
    //	CHECK_EQUAL(fields.getByName("valueBol")->getName(), fields.getByIndex(5)->getName());
    //	CHECK_EQUAL(fields.getByName("valueTme")->getName(), fields.getByIndex(6)->getName());

    if (fields.getByName("_ID")->getName() == fields.getByIndex(0)->getName()) {
        std::cout << "fields.getByIndex(0) pass!" << std::endl;
    } else {
        std::cout << "fields.getByIndex(0) fail!" << std::endl;
    }
    if (fields.getByName("name")->getName() == fields.getByIndex(1)->getName()) {
        std::cout << "fields.getByIndex(1) pass!" << std::endl;
    } else {
        std::cout << "fields.getByIndex(1) fail!" << std::endl;
    }
    if (fields.getByName("valueInt")->getName() == fields.getByIndex(2)->getName()) {
        std::cout << "fields.getByIndex(2) pass!" << std::endl;
    } else {
        std::cout << "fields.getByIndex(2) fail!" << std::endl;
    }
    if (fields.getByName("valueDbl")->getName() == fields.getByIndex(3)->getName()) {
        std::cout << "fields.getByIndex(3) pass!" << std::endl;
    } else {
        std::cout << "fields.getByIndex(3) fail!" << std::endl;
    }
    if (fields.getByName("valueTxt")->getName() == fields.getByIndex(4)->getName()) {
        std::cout << "fields.getByIndex(4) pass!" << std::endl;
    } else {
        std::cout << "fields.getByIndex(4) fail!" << std::endl;
    }
    if (fields.getByName("valueBol")->getName() == fields.getByIndex(5)->getName()) {
        std::cout << "fields.getByIndex(5) pass!" << std::endl;
    } else {
        std::cout << "fields.getByIndex(5) fail!" << std::endl;
    }
    if (fields.getByName("valueTme")->getName() == fields.getByIndex(6)->getName()) {
        std::cout << "fields.getByIndex(6) pass!" << std::endl;
    } else {
        std::cout << "fields.getByIndex(6) fail!" << std::endl;
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    FieldSetTest();
    FieldSetCreation();
    FieldSetCreationFromString();
    FieldSetFieldsOrder();
    return 0;
}

