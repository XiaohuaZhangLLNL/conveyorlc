/* 
 * File:   TestSqlTable.cpp
 * Author: zhang
 *
 * Created on May 8, 2014, 2:47 PM
 */


#include <iostream>
#include "src/DataBase/SqlRecordSet.h"
#include "src/DataBase/SqlDatabase.h"
#include "src/DataBase/SqlTable.h"

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

static string strdef() {
    Sql::string strdef = "CREATE TABLE test (";
    strdef += "_ID INTEGER PRIMARY KEY, ";
    strdef += "name TEXT NOT NULL, ";
    strdef += "valueInt INTEGER, ";
    strdef += "valueDbl REAL, ";
    strdef += "valueTxt TEXT, ";
    strdef += "valueBol INTEGER NOT NULL, ";
    strdef += "valueTme INTEGER";
    strdef += ")";

    return strdef;
}

static void setValues(Record& r) {
    r.setString("name", "test_text1");
    r.setInteger("valueInt", 890);
    r.setDouble("valueDbl", 0.345);
    r.setString("valueTxt", "test_text2");
    r.setBool("valueBol", true);
    r.setTime("valueTme", 0);
}

void TableSetup() {
    std::cout << "Test TableSetup ..." << std::endl;

    Sql::Database db;

    try {
        db.open("UnitTests.db");

        Table tb(db.getHandle(), "test", definition());

        //		CHECK_EQUAL("test", tb.name());
        if (tb.name() == "test") {
            std::cout << "tb.name() pass!" << std::endl;
        } else {
            std::cout << "tb.name() fail!" << std::endl;
        }

        //		CHECK_EQUAL(strdef(), tb.getDefinition());
        if (tb.getDefinition() == strdef()) {
            std::cout << "tb.getDefinition() pass!" << std::endl;
        } else {
            std::cout << "tb.getDefinition() fail!" << std::endl;
        }
        if (tb.exists())
            tb.remove();

        tb.create();

        //		CHECK(tb.exists());
        if (tb.exists()) {
            std::cout << "tb.exists() pass!" << std::endl;
        } else {
            std::cout << "tb.exists() fail!" << std::endl;
        }
        Record r(tb.fields());

        setValues(r);

        //add 10 records
        for (int index = 0; index < 10; index++)
            tb.addRecord(&r);

        tb.open();

        //		CHECK_EQUAL(10, tb.recordCount());
        if (tb.recordCount() == 10) {
            std::cout << "tb.recordCount() pass!" << std::endl;
        } else {
            std::cout << "tb.recordCount() fail!" << std::endl;
        }
        //check _ID of record with index = 5
        //		CHECK_EQUAL(6, tb.getRecord(5)->getKeyIdValue()->asInteger());
        if (tb.getRecord(5)->getKeyIdValue()->asInteger() == 6) {
            std::cout << "tb.getRecord(5)->getKeyIdValue()->asInteger() pass!" << std::endl;
        } else {
            std::cout << "tb.getRecord(5)->getKeyIdValue()->asInteger() fail!" << std::endl;
        }
        //select 4 records
        tb.open("_ID >= 5 AND _ID <= 8");

        //		CHECK_EQUAL(4, tb.recordCount());
        if (tb.recordCount() == 4) {
            std::cout << "tb.open(\"_ID >= 5 AND _ID <= 8\") pass!" << std::endl;
        } else {
            std::cout << "tb.open(\"_ID >= 5 AND _ID <= 8\") fail!" << std::endl;
        }
        //update data of record with _ID = 5
        if (Record * record = tb.getRecordByKeyId(5)) {
            r.setString("name", "new_text5");
            r.setInteger("valueInt", 111);
            r.setDouble("valueDbl", 0.222);
            r.setString("valueTxt", "new_text8");
            r.setBool("valueBol", false);
            r.setTime("valueTme", 1);

            tb.updateRecord(record);
        } else {
            //			CHECK_EQUAL("*", "record not found");
            std::cout << "record not found!" << std::endl;
        }

        //get updated record
        tb.open("_ID = 5");

        //check returned result
        //		CHECK_EQUAL(1, tb.recordCount());
        if (tb.recordCount() == 1) {
            std::cout << "tb.open(\"_ID = 5\") pass!" << std::endl;
        } else {
            std::cout << "tb.open(\"_ID = 5\") fail!" << std::endl;
        }

        if (Record * record = tb.getRecord(0)) {
            //			CHECK_EQUAL("new_text5", r.getValue("name")->asString());
            //			CHECK_EQUAL(111, r.getValue("valueInt")->asInteger());
            //			CHECK_EQUAL(0.222, r.getValue("valueDbl")->asDouble());
            //			CHECK_EQUAL("new_text8", r.getValue("valueTxt")->asString());
            //			CHECK_EQUAL(false, r.getValue("valueBol")->asBool());
            //			CHECK_EQUAL(1, r.getValue("valueTme")->asTime().asInteger());
            if (r.getValue("name")->asString() == "new_text5") {
                std::cout << "r.getValue(\"name\")->asString() pass!" << std::endl;
            } else {
                std::cout << "r.getValue(\"name\")->asString() fail!" << std::endl;
            }
            if (r.getValue("valueInt")->asInteger() == 111) {
                std::cout << "r.getValue(\"valueInt\")->asInteger() pass!" << std::endl;
            } else {
                std::cout << "r.getValue(\"valueInt\")->asInteger() fail!" << std::endl;
            }

            if (r.getValue("valueDbl")->asDouble() == 0.222) {
                std::cout << "r.getValue(\"valueDbl\")->asDouble() pass!" << std::endl;
            } else {
                std::cout << "r.getValue(\"valueDbl\")->asDouble() fail!" << std::endl;
            }

            if (r.getValue("valueTxt")->asString() == "new_text8") {
                std::cout << "r.getValue(\"valueTxt\")->asString() pass!" << std::endl;
            } else {
                std::cout << "r.getValue(\"valueTxt\")->asString() fail!" << std::endl;
            }

            if (r.getValue("valueBol")->asBool() == false) {
                std::cout << "r.getValue(\"valueBol\")->asBool() pass!" << std::endl;
            } else {
                std::cout << "r.getValue(\"valueBol\")->asBool() fail!" << std::endl;
            }

            if (r.getValue("valueTme")->asTime().asInteger() == 1) {
                std::cout << "r.getValue(\"valueTme\")->asTime().asInteger() pass!" << std::endl;
            } else {
                std::cout << "r.getValue(\"valueTme\")->asTime().asInteger() fail!" << std::endl;
            }

        } else {
            //			CHECK_EQUAL("*", "record not found");
            std::cout << "record not found!" << std::endl;
        }

        //delete 7 records
        tb.deleteRecords("_ID < 4");
        tb.open();

        //		CHECK_EQUAL(7, tb.recordCount());
        if (tb.recordCount() == 7) {
            std::cout << "tb.deleteRecords() pass!" << std::endl;
        } else {
            std::cout << "tb.deleteRecords() fail!" << std::endl;
        }
        //remove all records
        tb.truncate();
        tb.open();

        //		CHECK_EQUAL(0, tb.recordCount());
        if (tb.recordCount() == 0) {
            std::cout << "tb.truncate() pass!" << std::endl;
        } else {
            std::cout << "tb.truncate() fail!" << std::endl;
        }

    } catch (Exception e) {
        //		CHECK_EQUAL("*", e.msg());
        std::cout << e.msg() << std::endl;
    }
}

void TableCreationFromString() {
    std::cout << "Test TableCreationFromString ..." << std::endl;
    Sql::Database db;

    try {
        db.open("UnitTests.db");

        Table tb(db.getHandle(), "test", definition());

        Sql::string strdef;
        strdef += "_ID INTEGER PRIMARY KEY, ";
        strdef += "name TEXT NOT NULL, ";
        strdef += "valueInt INTEGER, ";
        strdef += "valueDbl REAL, ";
        strdef += "valueTxt TEXT, ";
        strdef += "valueBol INTEGER NOT NULL, ";
        strdef += "valueTme INTEGER";

        if (Table * table = Table::createFromDefinition(db.getHandle(), "test1", strdef)) {
            //			CHECK_EQUAL(table->fields()->definitionHash(), tb.fields()->definitionHash());
            if (table->fields()->definitionHash() == tb.fields()->definitionHash()) {
                std::cout << "tb.fields()->definitionHash() pass!" << std::endl;
            } else {
                std::cout << "tb.fields()->definitionHash() fail!" << std::endl;
            }
        } else {
            //			CHECK_EQUAL("*", "table not created");
            std::cout << "table not created" << std::endl;
        }

    } catch (Exception e) {
        //		CHECK_EQUAL("*", e.msg());
        std::cout << e.msg() << std::endl;
    }
}

/*
 * 
 */
int main(int argc, char** argv) {

    TableSetup();
    TableCreationFromString();

    return 0;
}

