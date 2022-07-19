#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

#define ROUNDTRIP_TEST(json, str, v_type)\
    do{\
        tiny_value v;\
        tiny_init(&v);\
        size_t len;\
        char *j;\
        EXPECT_EQ(JSON_PARSE_OK, tiny_parse(json, &v));\
        EXPECT_EQ(v_type, get_type(&v));\
        EXPECT_EQ(JSON_STRINGIFY_OK, tiny_stringify(&v, &j, &len));\
        EXPECT_EQ(memcmp(str, j, len), 0);\
        tiny_free(&v);\
        free(j);\
    }while(0);


TEST(stringify_array_test, roundtrip_test){
    ROUNDTRIP_TEST("[ null , false , true , 123 , \"abc\" ]", "[null,false,true,123,\"abc\"]", TINY_ARRAY);
    ROUNDTRIP_TEST("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", "[[],[0],[0,1],[0,1,2]]", TINY_ARRAY);
}