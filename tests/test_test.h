#include "gtest/gtest.h"

#include <string.h>

TEST(TestTest, EXPECT_EQ)
{
    EXPECT_EQ(0, 0);
}

TEST(TestTest, EXPECT_GT)
{
    EXPECT_GT(1, 0);
}

TEST(TestTest, EXPECT_LT)
{
    EXPECT_LT(-1, 0);
}

TEST(TestTest, EXPECT_TRUE)
{
    EXPECT_TRUE(true);
}

TEST(TestTest, EXPECT_FALSE)
{
    EXPECT_FALSE(false);
}

TEST(TestTest, EXPECT_STREQ)
{
    const char* s = "hello";
    EXPECT_STREQ("hello", s);
}