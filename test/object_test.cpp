#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

TEST(object__Test, get_object_key){
    tiny_value v;
    tiny_init(&v);
    int ret = tiny_parse("{\"name\":\"dank\"}", &v);
    EXPECT_EQ(ret, JSON_PARSE_OK);
    EXPECT_EQ(1, get_object_size(&v));
    EXPECT_EQ(strcmp(get_object_key(&v, 0), "name"), 0);
    EXPECT_EQ(4, get_object_key_length(&v, 0));
    tiny_value * tmp = get_object(&v, 0);
    EXPECT_EQ(TINY_STRING, get_type(tmp));
    EXPECT_EQ(strcmp(get_string(tmp), "dank"), 0);
    tiny_free(&v);
}

#define OBJECT_ERROR_TEST(str, err)\
    do{\
    tiny_value v;\
    tiny_init(&v);\
    int ret = tiny_parse(str, &v);\
    EXPECT_EQ(ret, err);\
    tiny_free(&v);\
    }while(0);

TEST(object__Test, err_miss_key_test){
    OBJECT_ERROR_TEST("{:\"dank\"}", JSON_PARSE_MISS_KEY)
    OBJECT_ERROR_TEST("{1:1,", JSON_PARSE_MISS_KEY)
    OBJECT_ERROR_TEST("{[]:1,", JSON_PARSE_MISS_KEY)
    OBJECT_ERROR_TEST("{\"a\":1,", JSON_PARSE_MISS_KEY)
}

TEST(object__Test, err_miss_COLON_test){
    OBJECT_ERROR_TEST("{\"name\":dank\"}", JSON_PARSE_MISS_COLON)
    OBJECT_ERROR_TEST("{\"a\",\"b\"}", JSON_PARSE_MISS_COLON)
    OBJECT_ERROR_TEST("{\"name\":}", JSON_PARSE_MISS_COLON)
}

TEST(object__Test, err_miss_COMMA_test){
    OBJECT_ERROR_TEST("{\"a\":1", JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET)
    OBJECT_ERROR_TEST("{\"a\":1)", JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET)
    OBJECT_ERROR_TEST("{\"a\":{}", JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET)
    OBJECT_ERROR_TEST("{\"a\":1 \"b\"", JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET)
}