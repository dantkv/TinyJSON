#if 1
#include<gtest/gtest.h>
#include<iostream>

#include "tiny_json.h"

TEST(func_Test, bool_test){
    tiny_value v;
    tiny_init(&v);
    set_bool(&v, 1);
    EXPECT_EQ(TINY_TRUE, get_bool(&v));
    set_bool(&v, 0);
    EXPECT_EQ(TINY_FALSE, get_bool(&v));
    tiny_free(&v);
    EXPECT_EQ(v.value_type, TINY_NULL);
}

TEST(func_Test, number_test){
    tiny_value v;
    tiny_init(&v);
    // 如果越界如何处理
    // set_number(&v, 1.2e100000000000000000000000000000000000000000000);
    set_number(&v, 1.2e2);
    EXPECT_EQ(120, get_number(&v));
    EXPECT_EQ(0, get_bool(&v));
}

#endif