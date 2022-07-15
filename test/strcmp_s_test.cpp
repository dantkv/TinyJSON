#if 1
#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

extern int strcmp_s(const char *,const char*, size_t);

TEST(strfunc_Test, strcmp){
    EXPECT_EQ(strcmp_s("a","a",1), 1);
    EXPECT_EQ(strcmp_s("abc", "abd", 3), 0);
    EXPECT_EQ(strcmp_s("a", "abc", 3), 0);
    EXPECT_EQ(strcmp_s("", "", 0), 1);
    EXPECT_EQ(strcmp_s("123", "1", 1), 1);
}

TEST(strfunc_Test, json_strcmp){
    tiny_value v;
    tiny_init(&v);
    char s[] = "hello";
    set_string(&v, s, strlen(s));
    EXPECT_EQ(1, strcmp_s(get_string(&v), s, strlen(s)));
    tiny_free(&v);
}

#endif