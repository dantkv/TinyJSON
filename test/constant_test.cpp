#if 1
#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

#define test_constant(err, json, t) \
    do{\
        tiny_value node; \
        node.value_type = TINY_NULL; \
        EXPECT_EQ(err, tiny_parse(json, &node));\
        EXPECT_EQ(t, get_type(&node));\
    }while(0);

TEST(constant_Test, parse_const_value_test){
    test_constant(JSON_PARSE_OK, "null", TINY_NULL);
    test_constant(JSON_PARSE_OK, "false", TINY_FALSE);
    test_constant(JSON_PARSE_OK, "true", TINY_TRUE);
}

TEST(constant_Test, parse_not_singular_Test){
    test_constant(JSON_PARSE_ROOT_NOT_SINGULAR, "null s ", TINY_NULL);
}

TEST(constant_Test, parse_invalid_value_Test){
    test_constant(JSON_PARSE_ROOT_NOT_SINGULAR, "nulla", TINY_NULL);
}

TEST(constant_Test, parse_expect_Test){
    test_constant(JSON_PARSE_EXPECT_VALUE, "", TINY_NULL);
    test_constant(JSON_PARSE_EXPECT_VALUE, " ", TINY_NULL);
}

#endif