#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(TreeTest, 5Choices3Options)
{
    TestVerif verify("../tests/big_examples/tree/5Choices3Options.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(TreeTest, 5Choices4Options)
{
    TestVerif verify("../tests/big_examples/tree/5Choices4Options.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(TreeTest, 5Choices5Options)
{
    TestVerif verify("../tests/big_examples/tree/5Choices5Options.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(TreeTest, 5Choices6Options)
{
    TestVerif verify("../tests/big_examples/tree/5Choices6Options.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(TreeTest, 6Choices3Options)
{
    TestVerif verify("../tests/big_examples/tree/6Choices3Options.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(TreeTest, 7Choices3Options)
{
    TestVerif verify("../tests/big_examples/tree/7Choices3Options.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(TreeTest, 8Choices3Options)
{
    TestVerif verify("../tests/big_examples/tree/8Choices3Options.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(TreeTestF, 5Choices3Options)
{
    TestVerif verify("../tests/big_examples/tree/5Choices3OptionsF.txt");

    EXPECT_EQ(verify.result, true);
}