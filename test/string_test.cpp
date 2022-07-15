#if 1
#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

#if 1
TEST(string_Test, string_access_test){
    tiny_value v;
    tiny_init(&v);
    char c[] = "hello";
    set_string(&v, c, strlen(c));
    const char *s = get_string(&v);
    // 不能直接比较字符串地址，这两个变量地址一定是不一样的
    EXPECT_EQ(0, strncmp(c, s, strlen(c)));
    EXPECT_EQ(5, get_string_len(&v));
    char e[]= "";
    set_string(&v, e, strlen(e));
    s = get_string(&v);
    EXPECT_EQ(0, strncmp(e, s, strlen(e)));
    EXPECT_EQ(0, get_string_len(&v));
    set_bool(&v, 1);
    tiny_free(&v);
    EXPECT_EQ(TINY_NULL, v.value_type);
}
#endif

#if 1
#define test_string(err , json, type)  do{\
        tiny_value v;\
        tiny_init(&v);\
        int ret = tiny_parse(json, &v);\
        EXPECT_EQ(err, ret);\
        if (err == JSON_PARSE_OK){\
            EXPECT_EQ(get_string(&v), json);\
            EXPECT_EQ(get_string_len(&v), strlen(json));\
            EXPECT_EQ(type, get_type(&v));\
        }\
    }while(0);


TEST(string_Test, string_escape_Test){
    test_string(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"", TINY_NULL);// "\v"
    test_string(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"", TINY_NULL);
    test_string(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"", TINY_NULL);
    test_string(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"", TINY_NULL);
}

TEST(string_Test, string_char_Test){
    test_string(JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"", TINY_NULL);
    test_string(JSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"", TINY_NULL);
}
#endif

#if 1
#define test_string_unicode(str, json, err, type)  do{\
        tiny_value v;\
        tiny_init(&v);\
        int ret = tiny_parse(json, &v);\
        EXPECT_EQ(err, ret);\
        if (err == JSON_PARSE_OK){\
            EXPECT_EQ(0, strncmp(get_string(&v), str, strlen(str)) );\
            EXPECT_EQ(get_string_len(&v), sizeof(str)-1);\
            EXPECT_EQ(type, get_type(&v));\
        }\
        tiny_free(&v);\
    }while(0);

TEST(string_Test, string_unicode_test){
    test_string_unicode("", "\"\"", JSON_PARSE_OK, TINY_STRING);
    test_string_unicode("Hello", "\"Hello\"", JSON_PARSE_OK, TINY_STRING);
    test_string_unicode("Hello\nWorld", "\"Hello\\nWorld\"", JSON_PARSE_OK, TINY_STRING);
    test_string_unicode("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"", JSON_PARSE_OK, TINY_STRING);
    test_string_unicode("Hello\0World", "\"Hello\\u0000World\"", JSON_PARSE_OK, TINY_STRING);
    test_string_unicode("\x24", "\"\\u0024\"", JSON_PARSE_OK, TINY_STRING);         /* Dollar sign U+0024 */
    test_string_unicode("\xC2\xA2", "\"\\u00A2\"", JSON_PARSE_OK, TINY_STRING);     /* Cents sign U+00A2 */
    test_string_unicode("\xE2\x82\xAC", "\"\\u20AC\"", JSON_PARSE_OK, TINY_STRING); /* Euro sign U+20AC */
    test_string_unicode("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"", JSON_PARSE_OK, TINY_STRING);  /* G clef sign U+1D11E */
    test_string_unicode("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"", JSON_PARSE_OK, TINY_STRING);  /* G clef sign U+1D11E */
}
#endif

#endif //STR_TEST_STATUS