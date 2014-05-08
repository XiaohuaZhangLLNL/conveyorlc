/* 
 * File:   TestSqlDatabase.cpp
 * Author: zhang
 *
 * Created on May 7, 2014, 3:03 PM
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

/*
 * 
 */
int main(int argc, char** argv) {
    Sql::Database db;

    try {
        db.open("UnitTests.db");

        Table tb(db.getHandle(), "test", definition());

        if (tb.exists())
            tb.remove();

        tb.create();

        tb.open();

        if (tb.recordCount() == 0) {
            std::cout << "tb.recordCount pass!" << std::endl;
        } else {
            std::cout << "tb.recordCount fail!" << std::endl;
        }
        //		CHECK_EQUAL(0, tb.recordCount());

        Record r(tb.fields());
        setValues(r);

        //test transaction commit
        tb.truncate();
        if (tb.totalRecordCount() == 0) {
            std::cout << "tb.totalRecordCount() pass!" << std::endl;
        } else {
            std::cout << "tb.totalRecordCount() fail!" << std::endl;
        }
        //		CHECK_EQUAL(0, tb.totalRecordCount());

        if (db.transactionBegin()) {
            tb.addRecord(&r);
            tb.addRecord(&r);
            tb.addRecord(&r);

            db.transactionCommit();
            //			CHECK_EQUAL(3, tb.totalRecordCount());

            if (tb.totalRecordCount() == 3) {
                std::cout << "tb.totalRecordCount() pass!" << std::endl;
            } else {
                std::cout << "tb.totalRecordCount() fail!" << std::endl;
            }
        } else {
            std::cout << "transaction begin fail!" << std::endl;
            //			CHECK_EQUAL("transaction begin fail", "");
        }

        //test transaction rollback
        tb.truncate();
        //		CHECK_EQUAL(0, tb.totalRecordCount());
        if (tb.totalRecordCount() == 0) {
            std::cout << "tb.totalRecordCount() pass!" << std::endl;
        } else {
            std::cout << "tb.totalRecordCount() fail!" << std::endl;
        }
        if (db.transactionBegin()) {
            tb.addRecord(&r);
            tb.addRecord(&r);
            tb.addRecord(&r);

            db.transactionRollback();
            //			CHECK_EQUAL(0, tb.totalRecordCount());
            if (tb.totalRecordCount() == 0) {
                std::cout << "tb.totalRecordCount() pass!" << std::endl;
            } else {
                std::cout << "tb.totalRecordCount() fail!" << std::endl;
            }

        } else {
            //			CHECK_EQUAL("transaction begin fail", "");
            std::cout << "transaction begin fail!" << std::endl;
        }

    } catch (Exception e) {
        //		CHECK_EQUAL("*", e.msg());
        std::cout << e.msg() << std::endl;
    }



    return 0;
}

