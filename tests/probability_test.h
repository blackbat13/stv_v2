#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(ProbabilityTest, 1Test)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest.txt", 4);

    EXPECT_EQ(verify.result, false);
}

TEST(ProbabilityTest, 2Test)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest2.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, 3Test)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest3.txt", 4);

    EXPECT_EQ(verify.result, false);
}

TEST(ProbabilityTest, 4Test)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest4.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, 5Test)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest5.txt", 4);

    EXPECT_EQ(verify.result, true);
}