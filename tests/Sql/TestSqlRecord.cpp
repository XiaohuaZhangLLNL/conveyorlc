/* 
 * File:   TestSqlRecord.cpp
 * Author: zhang
 *
 * Created on May 8, 2014, 9:57 AM
 */


#include <iostream>
#include "src/DataBase/SqlRecord.h"


using namespace Sql;

void RecordSetup() {
    std::cout << "Test RecordSetup ..." << std::endl;
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

    FieldSet fields(def);

    Record r(&fields);

    //	CHECK(r.fieldByName("name"));
    //	CHECK(r.fieldByName("valueInt"));
    //	CHECK(r.fieldByName("valueDbl"));
    //	CHECK(r.fieldByName("valueTxt"));
    //	CHECK(r.fieldByName("valueBol"));
    //	CHECK(r.fieldByName("valueTme"));

    if (r.fieldByName("name")) {
        std::cout << "r.fieldByName(\"name\") pass!" << std::endl;
    } else {
        std::cout << "r.fieldByName(\"name\") fail!" << std::endl;
    }
    if (r.fieldByName("valueInt")) {
        std::cout << "r.fieldByName(\"valueInt\") pass!" << std::endl;
    } else {
        std::cout << "r.fieldByName(\"valueInt\") fail!" << std::endl;
    }
    if (r.fieldByName("valueDbl")) {
        std::cout << "r.fieldByName(\"valueDbl\") pass!" << std::endl;
    } else {
        std::cout << "r.fieldByName(\"valueDbl\") fail!" << std::endl;
    }
    if (r.fieldByName("valueTxt")) {
        std::cout << "r.fieldByName(\"valueTxt\") pass!" << std::endl;
    } else {
        std::cout << "r.fieldByName(\"valueTxt\") fail!" << std::endl;
    }
    if (r.fieldByName("valueBol")) {
        std::cout << "r.fieldByName(\"valueBol\") pass!" << std::endl;
    } else {
        std::cout << "r.fieldByName(\"valueBol\") fail!" << std::endl;
    }
    if (r.fieldByName("valueTme")) {
        std::cout << "r.fieldByName(\"valueTme\") pass!" << std::endl;
    } else {
        std::cout << "r.fieldByName(\"valueTme\") fail!" << std::endl;
    }

    //	CHECK_EQUAL(7, r.columnCount());

    if (r.columnCount() == 7) {
        std::cout << "r.columnCount() pass!" << std::endl;
    } else {
        std::cout << "r.columnCount() fail!" << std::endl;
    }

    if (r.columnCount() == 7) {
        //check if values by name and by index are the same
        //		CHECK_EQUAL(r.getValue("_ID"), r.getValue(0));
        //		CHECK_EQUAL(r.getValue("name"), r.getValue(1));
        //		CHECK_EQUAL(r.getValue("valueInt"), r.getValue(2));
        //		CHECK_EQUAL(r.getValue("valueDbl"), r.getValue(3));
        //		CHECK_EQUAL(r.getValue("valueTxt"), r.getValue(4));
        //		CHECK_EQUAL(r.getValue("valueBol"), r.getValue(5));
        //		CHECK_EQUAL(r.getValue("valueTme"), r.getValue(6));
        if (r.getValue("_ID") == r.getValue(0)) {
            std::cout << "r.getValue(\"_ID\") pass!" << std::endl;
        } else {
            std::cout << "r.getValue(\"name\") fail!" << std::endl;
        }
        if (r.getValue("name") == r.getValue(1)) {
            std::cout << "r.getValue(\"name\") pass!" << std::endl;
        } else {
            std::cout << "r.getValue(\"name\") fail!" << std::endl;
        }
        if (r.getValue("valueInt") == r.getValue(2)) {
            std::cout << "r.getValue(\"valueInt\") pass!" << std::endl;
        } else {
            std::cout << "r.getValue(\"valueInt\") fail!" << std::endl;
        }
        if (r.getValue("valueDbl") == r.getValue(3)) {
            std::cout << "r.getValue(\"valueDbl\") pass!" << std::endl;
        } else {
            std::cout << "r.getValue(\"valueDbl\") fail!" << std::endl;
        }
        if (r.getValue("valueTxt") == r.getValue(4)) {
            std::cout << "r.getValue(\"valueTxt\") pass!" << std::endl;
        } else {
            std::cout << "r.getValue(\"valueTxt\") fail!" << std::endl;
        }
        if (r.getValue("valueBol") == r.getValue(5)) {
            std::cout << "r.getValue(\"valueBol\") pass!" << std::endl;
        } else {
            std::cout << "r.getValue(\"valueBol\") fail!" << std::endl;
        }
        if (r.getValue("valueTme") == r.getValue(6)) {
            std::cout << "r.getValue(\"valueTme\") pass!" << std::endl;
        } else {
            std::cout << "r.getValue(\"valueTme\") fail!" << std::endl;
        }
    }
}

void RecordQueries() {
    std::cout << "Test RecordQueries ..." << std::endl;
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

    FieldSet fields(def);

    Record r(&fields);

    Record rnew(r);

    //	CHECK_EQUAL(rnew.toSql(), r.toSql());
    if (rnew.toSql() == r.toSql()) {
        std::cout << "rnew.toSql() pass!" << std::endl;
    } else {
        std::cout << "rnew.toSql() fail!" << std::endl;
    }
    //	CHECK_EQUAL("insert into test (_ID, name, valueInt, valueDbl, valueTxt, valueBol, valueTme) values (null, null, null, null, null, null, null)",
    //		r.toSqlInsert("test"));
    //
    //	CHECK_EQUAL("update test set name=null, valueInt=null, valueDbl=null, valueTxt=null, valueBol=null, valueTme=null where _ID = null",
    //		r.toSqlUpdate("test"));
    //
    //	CHECK_EQUAL("null, null, null, null, null, null, null",
    //		r.toSql());
    //
    //	CHECK_EQUAL("null|null|null|null|null|null|null",
    //		r.toString());
    Sql::string str1 = "insert into test (_ID, name, valueInt, valueDbl, valueTxt, valueBol, valueTme) values (null, null, null, null, null, null, null)";
    if (r.toSqlInsert("test") == str1) {
        std::cout << "r.toSqlInsert(\"test\") pass!" << std::endl;
    } else {
        std::cout << "r.toSqlInsert(\"test\") fail!" << std::endl;
    }

    Sql::string str2 = "update test set name=null, valueInt=null, valueDbl=null, valueTxt=null, valueBol=null, valueTme=null where _ID = null";
    if (r.toSqlUpdate("test") == str2) {
        std::cout << "r.toSqlUpdate(\"test\") pass!" << std::endl;
    } else {
        std::cout << "r.toSqlUpdate(\"test\") fail!" << std::endl;
    }
    Sql::string str3 = "null, null, null, null, null, null, null";

    if (r.toSql() == str3) {
        std::cout << "r.toSql() pass!" << std::endl;
    } else {
        std::cout << "r.toSql() fail!" << std::endl;
    }
    Sql::string str4 = "null|null|null|null|null|null|null";
    if (r.toString() == str4) {
        std::cout << "r.toString() pass!" << std::endl;
    } else {
        std::cout << "r.toString() fail!" << std::endl;
    }
}

void RecordSetValues() {
    std::cout << "Test RecordSetValues ..." << std::endl;
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

    FieldSet fields(def);

    Record r(&fields);

    r.setInteger("_ID", 123);
    r.setString("name", "test_text1");
    r.setInteger("valueInt", 890);
    r.setDouble("valueDbl", 0.345);
    r.setString("valueTxt", "test_text2");
    r.setBool("valueBol", true);
    r.setTime("valueTme", 0);

    //	CHECK_EQUAL(123, r.getKeyIdValue()->asInteger());
    if (r.getKeyIdValue()->asInteger() == 123) {
        std::cout << "r.getKeyIdValue()->asInteger() pass!" << std::endl;
    } else {
        std::cout << "r.getKeyIdValue()->asInteger() fail!" << std::endl;
    }
    //	CHECK_EQUAL("123|test_text1|890|0.34500000|test_text2|1|0", r.toString());
    Sql::string str1 = "123|test_text1|890|0.34500000|test_text2|1|0";
    if (r.toString() == str1) {
        std::cout << "r.toString() pass!" << std::endl;
    } else {
        std::cout << "r.toString() fail!" << std::endl;
    }
    //	CHECK_EQUAL("123, 'test_text1', 890, 0.34500000, 'test_text2', 1, 0", r.toSql());
    Sql::string str2 = "123, 'test_text1', 890, 0.34500000, 'test_text2', 1, 0";
    if (r.toSql() == str2) {
        std::cout << "r.toSql() pass!" << std::endl;
    } else {
        std::cout << "r.toSql() fail!" << std::endl;
    }
}

void RecordGetValues() {
    std::cout << "Test RecordGetValues ..." << std::endl;
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

    FieldSet fields(def);

    Record r(&fields);

    r.setInteger("_ID", 123);
    r.setString("name", "test_text1");
    r.setInteger("valueInt", 890);
    r.setDouble("valueDbl", 0.345);
    r.setString("valueTxt", "test_text2");
    r.setBool("valueBol", true);
    r.setTime("valueTme", 0);

    //	CHECK_EQUAL(123, r.getValue("_ID")->asInteger());
    //	CHECK_EQUAL("test_text1", r.getValue("name")->asString());
    //	CHECK_EQUAL(890, r.getValue("valueInt")->asInteger());
    //	CHECK_EQUAL(0.345, r.getValue("valueDbl")->asDouble());
    //	CHECK_EQUAL("test_text2", r.getValue("valueTxt")->asString());
    //	CHECK_EQUAL(true, r.getValue("valueBol")->asBool());
    //	CHECK_EQUAL(0, r.getValue("valueTme")->asTime().asInteger());
    if (r.getValue("_ID")->asInteger() == 123) {
        std::cout << "r.getValue(\"_ID\")->asInteger() pass!" << std::endl;
    } else {
        std::cout << "r.getValue(\"_ID\")->asInteger() fail!" << std::endl;
    }
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

/*
 * 
 */
int main(int argc, char** argv) {

    RecordSetup();
    RecordQueries();
    RecordSetValues();
    RecordGetValues();

    return 0;
}

