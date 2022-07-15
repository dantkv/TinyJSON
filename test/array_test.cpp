#if 1
#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"


TEST(array_Test, null_test){
    tiny_value v;
    tiny_init(&v);
    EXPECT_EQ(JSON_PARSE_OK, tiny_parse("[]", &v));
    EXPECT_EQ(TINY_ARRAY, get_type(&v));
    EXPECT_EQ(0, get_array_size(&v));
    tiny_free(&v);
}

TEST(array_Test, mutil_type_test){
    tiny_value v;
    tiny_init(&v);
    EXPECT_EQ(JSON_PARSE_OK, tiny_parse("[ null , false , true , 123 , \"abc\" ]", &v));
    EXPECT_EQ(TINY_ARRAY, get_type(&v));
    EXPECT_EQ(5, get_array_size(&v));
    EXPECT_EQ(TINY_NULL, get_array_element(&v, 0)->value_type);
    EXPECT_EQ(TINY_FALSE, get_array_element(&v, 1)->value_type);
    EXPECT_EQ(TINY_TRUE, get_array_element(&v, 2)->value_type);
    tiny_value * e = get_array_element(&v, 3);
    EXPECT_EQ(TINY_NUMBER, e->value_type);
    EXPECT_EQ(123, e->number);
    e = get_array_element(&v, 4);
    EXPECT_EQ(TINY_STRING, e->value_type);
    EXPECT_EQ(0, strcmp("abc", e->str));
    EXPECT_EQ(3, e->str_len);
    tiny_free(&v);
}

TEST(array_Test, mutil_array_test){
    tiny_value v;
    tiny_init(&v);
    EXPECT_EQ(JSON_PARSE_OK, tiny_parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", &v));
    EXPECT_EQ(TINY_ARRAY, get_type(&v));
    EXPECT_EQ(4, get_array_size(&v));
    tiny_free(&v);
}

#define array_test_error(err, json) \
    do{\
        tiny_value node; \
        tiny_init(&node);\
        EXPECT_EQ(err, tiny_parse(json, &node));\
        tiny_free(&node);\
    }while(0);

TEST(array_Test, niss_char_array_test){
    array_test_error(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    array_test_error(JSON_PARSE_INVALID_VALUE, "[1, ]");
    array_test_error(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    array_test_error(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    array_test_error(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}
#endif //ARRAY_TEST_STATUS