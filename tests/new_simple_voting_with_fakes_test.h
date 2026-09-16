#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(NewSimpleVotingWithFakesTest, 1Voter1Coercer)
{
    IterativeVerif verify("../tests/examples/svotewithfakes/1Voter1CoercerWithFakes.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingWithFakesTest, 2Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svotewithfakes/2Voters1CoercerWithFakes.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingWithFakesTest, 3Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svotewithfakes/3Voters1CoercerWithFakes.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingWithFakesTest, 4Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svotewithfakes/4Voters1CoercerWithFakes.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingWithFakesNotGiveTest, 1Voter1Coercer)
{
    IterativeVerif verify("../tests/examples/svotewithfakes/1Voter1CoercerWithFakesNG.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingWithFakesNotGiveTest, 2Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svotewithfakes/2Voters1CoercerWithFakesNG.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingWithFakesNotGiveTest, 3Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svotewithfakes/3Voters1CoercerWithFakesNG.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingWithFakesNotGiveTest, 4Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svotewithfakes/4Voters1CoercerWithFakesNG.txt", 0);

    EXPECT_EQ(verify.result, true);
}