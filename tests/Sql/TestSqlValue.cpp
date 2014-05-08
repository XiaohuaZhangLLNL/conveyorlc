/* 
 * File:   TestSqlValue.cpp
 * Author: zhang
 *
 * Created on May 8, 2014, 3:11 PM
 */

#include <iostream>
#include "src/DataBase/SqlRecordSet.h"
#include "src/DataBase/SqlDatabase.h"
#include "src/DataBase/SqlTable.h"
#include "src/DataBase/SqlValue.h"

using namespace Sql;

void ValueStore() {
    std::cout << "Test ValueStore ..." << std::endl;
    Value v_udf;
    Value v_int("123", type_int);
    Value v_txt("text", type_text);
    Value v_dbl("0.5678", type_float);
    Value v_bol("1", type_bool);
    Value v_tme("0", type_time);

    //	CHECK(v_udf.isNull());
    //	CHECK_EQUAL(123, v_int.asInteger());
    //	CHECK_EQUAL("text", v_txt.asString());
    //	CHECK_EQUAL(0.5678, v_dbl.asDouble());
    //	CHECK_EQUAL(true, v_bol.asBool());
    //	CHECK(v_tme.asTime() == sql::time(0));
    if (v_udf.isNull()) {
        std::cout << "v_udf.isNull() pass!" << std::endl;
    } else {
        std::cout << "v_udf.isNull() fail!" << std::endl;
    }
    if (v_int.asInteger() == 123) {
        std::cout << "v_int.asInteger() pass!" << std::endl;
    } else {
        std::cout << "v_int.asInteger() fail!" << std::endl;
    }
    if (v_txt.asString() == "text") {
        std::cout << "v_txt.asString() pass!" << std::endl;
    } else {
        std::cout << "v_txt.asString() fail!" << std::endl;
    }
    if (v_dbl.asDouble() == 0.5678) {
        std::cout << "v_dbl.asDouble() pass!" << std::endl;
    } else {
        std::cout << "v_dbl.asDouble() fail!" << std::endl;
    }
    if (v_bol.asBool() == true) {
        std::cout << "v_bol.asBool() pass!" << std::endl;
    } else {
        std::cout << "v_bol.asBool() fail!" << std::endl;
    }

    if (v_tme.asTime() == Sql::time(0)) {
        std::cout << "v_tme.asTime() pass!" << std::endl;
    } else {
        std::cout << "v_tme.asTime() fail!" << std::endl;
    }
}

void ValueCompare() {
    std::cout << "Test ValueCompare ..." << std::endl;
    Value v("test", type_text);

    Value v1(v);

    Value v2 = v;

    //	CHECK(v1.equals(v));
    //	CHECK(v2.equals(v));

    if (v1.equals(v)) {
        std::cout << "v1.equals(v) pass!" << std::endl;
    } else {
        std::cout << "v1.equals(v) fail!" << std::endl;
    }
    if (v2.equals(v)) {
        std::cout << "v2.equals(v) pass!" << std::endl;
    } else {
        std::cout << "v2.equals(v) fail!" << std::endl;
    }
}

void ValueConvert() {
    std::cout << "Test ValueConvert ..." << std::endl;
    Value v("678", type_int);

    //	CHECK_EQUAL(false, v.asBool());
    //	CHECK_EQUAL(678, v.asDouble()); 
    //	CHECK_EQUAL(678, v.asInteger());
    //	CHECK_EQUAL("678", v.asString());
    if (v.asBool() == false) {
        std::cout << "v.asBool() pass!" << std::endl;
    } else {
        std::cout << "v.asBool() fail!" << std::endl;
    }
    if (v.asDouble() == 678) {
        std::cout << "v.asDouble() pass!" << std::endl;
    } else {
        std::cout << "v.asDouble() fail!" << std::endl;
    }
    if (v.asInteger() == 678) {
        std::cout << "v.asInteger() pass!" << std::endl;
    } else {
        std::cout << "v.asInteger() fail!" << std::endl;
    }
    if (v.asString() == "678") {
        std::cout << "v.asString() pass!" << std::endl;
    } else {
        std::cout << "v.asString() fail!" << std::endl;
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    ValueStore();
    ValueCompare();
    ValueConvert();
    return 0;
}

