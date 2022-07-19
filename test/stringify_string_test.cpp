#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

#define ROUNDTRIP_TEST(json, v_type)\
    do{\
        tiny_value v;\
        tiny_init(&v);\
        size_t len;\
        char *j;\
        EXPECT_EQ(JSON_PARSE_OK, tiny_parse(json, &v));\
        EXPECT_EQ(v_type, get_type(&v));\
        EXPECT_EQ(JSON_STRINGIFY_OK, tiny_stringify(&v, &j, &len));\
        EXPECT_EQ(strcmp(json, j), 0);\
        tiny_free(&v);\
        free(j);\
    }while(0);

TEST(stringify_string_test, roundtrip_test){
    ROUNDTRIP_TEST("\"hello\"",  TINY_STRING);
    ROUNDTRIP_TEST("\"\"",  TINY_STRING);
    ROUNDTRIP_TEST("\"Hello\\nWorld\"", TINY_STRING);
    ROUNDTRIP_TEST("\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"", TINY_STRING);
}

