#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(RecursionTest, 2Counters11States)
{
    TestVerif verify("../tests/big_examples/recursion/2Counters11States.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(RecursionTest, 3Counters11States)
{
    TestVerif verify("../tests/big_examples/recursion/3Counters11States.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(RecursionTest, 4Counters11States)
{
    TestVerif verify("../tests/big_examples/recursion/4Counters11States.txt");

    EXPECT_EQ(verify.result, true);
}