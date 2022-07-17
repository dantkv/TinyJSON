#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

#define ROUNDTRIP_TEST(str, v_type, number)\
    do{\
        tiny_value v;\
        tiny_init(&v);\
        size_t len;\
        char *json;\
        EXPECT_EQ(JSON_PARSE_OK, tiny_parse(str, &v));\
        EXPECT_EQ(v_type, get_type(&v));\
        EXPECT_EQ(JSON_STRINGIFY_OK, tiny_stringify(&v, &json, &len));\
        EXPECT_EQ(number, get_number(&v));\
        tiny_free(&v);\
        free(json);\
    }while(0);

TEST(stringify_number_test, roundtrip_test){
    ROUNDTRIP_TEST("1.23", TINY_NUMBER, 1.23);
    ROUNDTRIP_TEST("1e9", TINY_NUMBER, 1e9);
    ROUNDTRIP_TEST("0.0001", TINY_NUMBER, 0.0001);
    ROUNDTRIP_TEST("-0.0", TINY_NUMBER, 0);
    ROUNDTRIP_TEST("1.0000000000000002", TINY_NUMBER, 1.0000000000000002);
    ROUNDTRIP_TEST("1.23e-010", TINY_NUMBER, 1.23e-010);
    ROUNDTRIP_TEST("-0.0", TINY_NUMBER, 0);
}