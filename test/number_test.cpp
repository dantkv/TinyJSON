#define NUMBER_TEST 1
#if NUMBER_TEST
#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

#define test_number(err, json, type, number) \
    do{\
        tiny_value node; \
        node.value_type = TINY_NULL; \
        EXPECT_EQ(err, tiny_parse(json, &node));\
        EXPECT_EQ(type, get_type(&node));\
        EXPECT_EQ(number, get_number(&node));\
    }while(0);

#if 1
TEST(number_Test, number_ok_TEST){
    test_number(JSON_PARSE_OK, "0", TINY_NUMBER, 0.0);
    test_number(JSON_PARSE_OK, "-0", TINY_NUMBER, 0.0);
    test_number(JSON_PARSE_OK, "-0.0", TINY_NUMBER, 0.0);
    test_number(JSON_PARSE_OK, "0.0", TINY_NUMBER, 0.0);
    test_number(JSON_PARSE_OK, "1", TINY_NUMBER, 1.0);
    test_number(JSON_PARSE_OK, "-1.0", TINY_NUMBER, -1.0);
    test_number(JSON_PARSE_OK, "3.123", TINY_NUMBER, 3.123);
    test_number(JSON_PARSE_OK, "2e3", TINY_NUMBER, 2e3);
    test_number(JSON_PARSE_OK, "2E-3", TINY_NUMBER, 2E-3);
    test_number(JSON_PARSE_OK, "-1e10", TINY_NUMBER, -1e10);
    test_number(JSON_PARSE_OK, "1e+3", TINY_NUMBER, 1e+3);
    test_number(JSON_PARSE_OK, "1.23e3", TINY_NUMBER, 1.23e3);
    test_number(JSON_PARSE_OK, "1e-10000000", TINY_NUMBER, 0.0);
    test_number(JSON_PARSE_OK, "1.23e-10", TINY_NUMBER, 1.23e-10);
    test_number(JSON_PARSE_OK, "1.23e-010", TINY_NUMBER, 1.23e-10);

    test_number(JSON_PARSE_OK, "0.0", TINY_NUMBER, 1e-10000);

    test_number(JSON_PARSE_OK, "1.0000000000000002", TINY_NUMBER, 1.0000000000000002);/* the smallest number > 1 */
    test_number(JSON_PARSE_OK, "4.9406564584124654e-324", TINY_NUMBER, 4.9406564584124654e-324);
    test_number(JSON_PARSE_OK, "2.2250738585072009e-308", TINY_NUMBER, 2.2250738585072009e-308);
    test_number(JSON_PARSE_OK, "-2.2250738585072009e-308", TINY_NUMBER, -2.2250738585072009e-308);
    test_number(JSON_PARSE_OK, "2.2250738585072014e-308", TINY_NUMBER, 2.2250738585072014e-308);
    test_number(JSON_PARSE_OK, "-2.2250738585072014e-308", TINY_NUMBER, -2.2250738585072014e-308);
    test_number(JSON_PARSE_OK, "1.7976931348623157e+308", TINY_NUMBER, 1.7976931348623157e+308);
    test_number(JSON_PARSE_OK, "-1.7976931348623157e+308", TINY_NUMBER, -1.7976931348623157e+308);
}
#endif

#define test_number_error(err, json) \
    do{\
        tiny_value node; \
        node.value_type = TINY_NULL; \
        EXPECT_EQ(err, tiny_parse(json, &node));\
    }while(0);
#if 1
TEST(number_Test, number_Test_invalid_value){
    /* at least one digit before '.' */
    test_number_error(JSON_PARSE_INVALID_VALUE, ".123");
    /* at least one digit after '.' */
    test_number_error(JSON_PARSE_INVALID_VALUE, "-6.");
    test_number_error(JSON_PARSE_INVALID_VALUE, "+1");
    test_number_error(JSON_PARSE_INVALID_VALUE, "INF");
    test_number_error(JSON_PARSE_INVALID_VALUE, "inf");
}
#endif

#if 1
TEST(number_Test, number_Test_not_singular){
    /* invalid number */
    test_number_error(JSON_PARSE_ROOT_NOT_SINGULAR, "0123");/* after zero should be '.' , 'E' , 'e' or nothing */
    test_number_error(JSON_PARSE_ROOT_NOT_SINGULAR, "0x0");
    test_number_error(JSON_PARSE_ROOT_NOT_SINGULAR, "0x0123");
}
#endif

#if 1
TEST(number_Test, number_too_big){
    test_number_error(JSON_PARSE_NUMBER_OUT_OF_RANGE, "1e309");
    test_number_error(JSON_PARSE_NUMBER_OUT_OF_RANGE, "-1e309");
}
#endif

#endif// NUMBER_TEST