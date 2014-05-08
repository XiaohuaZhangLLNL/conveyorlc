/* 
 * File:   TestSqlRecordSet.cpp
 * Author: zhang
 *
 * Created on May 8, 2014, 2:14 PM
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

static void setValues(Record& r) {
    r.setString("name", "test_text1");
    r.setInteger("valueInt", 890);
    r.setDouble("valueDbl", 0.345);
    r.setString("valueTxt", "test_text2");
    r.setBool("valueBol", true);
    r.setTime("valueTme", 0);
}

static void cmpValues(Record& r) {
    //	CHECK_EQUAL("test_text1", r.getValue("name")->asString());
    //	CHECK_EQUAL(890, r.getValue("valueInt")->asInteger());
    //	CHECK_EQUAL(0.345, r.getValue("valueDbl")->asDouble());
    //	CHECK_EQUAL("test_text2", r.getValue("valueTxt")->asString());
    //	CHECK_EQUAL(true, r.getValue("valueBol")->asBool());
    //	CHECK_EQUAL(0, r.getValue("valueTme")->asTime().asInteger());

    if (r.getValue("name")->asString() == "test_text1") {
        std::cout << "r.getValue(\"name\")->asString() pass!" << std::endl;
    } else {
        std::cout << "r.getValue(\"name\")->asString() fail!" << std::endl;
    }
    if (r.getValue("valueInt")->asInteger() == 890) {
        std::cout << "r.getValue(\"valueInt\")->asInteger() pass!" << std::endl;
    } else {
        std::cout << "r.getValue(\"valueInt\")->asInteger() fail!" << std::endl;
    }

    if (r.getValue("valueDbl")->asDouble() == 0.345) {
        std::cout << "r.getValue(\"valueDbl\")->asDouble() pass!" << std::endl;
    } else {
        std::cout << "r.getValue(\"valueDbl\")->asDouble() fail!" << std::endl;
    }

    if (r.getValue("valueTxt")->asString() == "test_text2") {
        std::cout << "r.getValue(\"valueTxt\")->asString() pass!" << std::endl;
    } else {
        std::cout << "r.getValue(\"valueTxt\")->asString() fail!" << std::endl;
    }

    if (r.getValue("valueBol")->asBool() == true) {
        std::cout << "r.getValue(\"valueBol\")->asBool() pass!" << std::endl;
    } else {
        std::cout << "r.getValue(\"valueBol\")->asBool() fail!" << std::endl;
    }

    if (r.getValue("valueTme")->asTime().asInteger() == 0) {
        std::cout << "r.getValue(\"valueTme\")->asTime().asInteger() pass!" << std::endl;
    } else {
        std::cout << "r.getValue(\"valueTme\")->asTime().asInteger() fail!" << std::endl;
    }

}

void RecordSetModify() {
    std::cout << "Test RecordSetModify ..." << std::endl;
    Sql::Database db;

    try {
        db.open("UnitTests.db");

        Table tb(db.getHandle(), "test", definition());

        if (tb.exists())
            tb.remove();

        tb.create();

        Record r(tb.fields());

        setValues(r);

        for (int index = 0; index < 10; index++)
            tb.addRecord(&r);

        tb.open();

        //		CHECK_EQUAL(10, tb.recordCount());
        if (tb.recordCount() == 10) {
            std::cout << "tb.recordCount() pass!" << std::endl;
        } else {
            std::cout << "tb.recordCount() fail!" << std::endl;
        }

        for (int index = 0; index < tb.recordCount(); index++)
            if (Record * record = tb.getRecord(index))
                cmpValues(*record);

    } catch (Exception e) {
        //		CHECK_EQUAL("*", e.msg());
        std::cout << e.msg() << std::endl;

    }
}

void RecordSetData() {
    Sql::Database db;

    try {
        db.open("UnitTests.db");

        Table tb(db.getHandle(), "test", definition());

        if (tb.exists())
            tb.remove();

        tb.create();

        Record r(tb.fields());

        setValues(r);

        tb.addRecord(&r);

        tb.open();

        //		CHECK_EQUAL(1, tb.recordCount());
        if (tb.recordCount() == 1) {
            std::cout << "tb.recordCount() pass!" << std::endl;
        } else {
            std::cout << "tb.recordCount() fail!" << std::endl;
        }

        if (Record * record = tb.getTopRecord()) {
            cmpValues(*record);
            //			CHECK_EQUAL("1|test_text1|890|0.345|test_text2|1|01-01-1970 01:00, Thu", record->toString());
            Sql::string str = "1|test_text1|890|0.345|test_text2|1|31-12-1969 16:00, Wed";
            std::cout << record->toString() << std::endl;
            if (record->toString() == str) {
                std::cout << "record->toString() pass!" << std::endl;
            } else {
                std::cout << "record->toString() fail!" << std::endl;
            }

        } else {
            //			CHECK_EQUAL("*", "record not found");
            std::cout << "record not found" << std::endl;
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

    RecordSetModify();
    RecordSetData();
    return 0;
}

