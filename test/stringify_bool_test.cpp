#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

#define ROUNDTRIP_TEST(str, v_type, length)\
    do{\
        tiny_value v;\
        tiny_init(&v);\
        size_t len;\
        char *json;\
        EXPECT_EQ(JSON_PARSE_OK, tiny_parse(str, &v));\
        EXPECT_EQ(v_type, v.value_type);\
        EXPECT_EQ(JSON_STRINGIFY_OK, tiny_stringify(&v, &json, &len));\
        EXPECT_EQ(0, strcmp(json, str));\
        EXPECT_EQ(len, length);\
        tiny_free(&v);\
        free(json);\
    }while(0);

TEST(stringify_literal_test, roundtrip_test){
    ROUNDTRIP_TEST("null", TINY_NULL, 4);
    ROUNDTRIP_TEST("false", TINY_FALSE, 5);
    ROUNDTRIP_TEST("true", TINY_TRUE, 4);
}
