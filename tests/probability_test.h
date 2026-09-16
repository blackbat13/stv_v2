#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(ProbabilityTest, ParseTestF)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest.txt", 4);

    EXPECT_EQ(verify.result, false);
}

TEST(ProbabilityTest, ParseTestG)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest2.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, SplitProbability)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest3.txt", 4);

    EXPECT_EQ(verify.result, false);
}

TEST(ProbabilityTest, JoinProbability)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest4.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, PassProbability)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest4a.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, GVerification)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest5.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, GVerificationFurther)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest6.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, FVerificationDistinctStates)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest7.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, FVerificationFurther2ndChoice)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest7a.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, Verification2ndChoice)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest7a.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, VerificationTooHighProbability)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest8.txt", 4);

    EXPECT_EQ(verify.result, false);
}

TEST(ProbabilityTest, VerificationOkProbability)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest8a.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, VerificationMultipleChoice)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest8b.txt", 4);

    EXPECT_EQ(verify.result, true);
}

TEST(ProbabilityTest, FVerificationEndEarly)
{
    TestVerif verify("../tests/examples/probability/SimpleVotingProbTest8c.txt", 4);

    EXPECT_EQ(verify.result, true);
}