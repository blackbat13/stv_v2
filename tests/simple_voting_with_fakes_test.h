#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(SimpleVotingWithFakesTest, 1Voter1Coercer)
{
    TestVerif verify("../tests/examples/svotewithfakes/1Voter1CoercerWithFakes.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 21);
}

TEST(SimpleVotingWithFakesTest, 2Voters1Coercer)
{
    TestVerif verify("../tests/examples/svotewithfakes/2Voters1CoercerWithFakes.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 279);
}

TEST(SimpleVotingWithFakesTest, 3Voters1Coercer)
{
    TestVerif verify("../tests/examples/svotewithfakes/3Voters1CoercerWithFakes.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 3429);
}

TEST(SimpleVotingWithFakesTest, 4Voters1Coercer)
{
    TestVerif verify("../tests/examples/svotewithfakes/4Voters1CoercerWithFakes.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 41391);
}

TEST(SimpleVotingWithFakesNotGiveTest, 1Voter1Coercer)
{
    TestVerif verify("../tests/examples/svotewithfakes/1Voter1CoercerWithFakesNG.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 21);
}

TEST(SimpleVotingWithFakesNotGiveTest, 2Voters1Coercer)
{
    TestVerif verify("../tests/examples/svotewithfakes/2Voters1CoercerWithFakesNG.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 279);
}

TEST(SimpleVotingWithFakesNotGiveTest, 3Voters1Coercer)
{
    TestVerif verify("../tests/examples/svotewithfakes/3Voters1CoercerWithFakesNG.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 3429);
}

TEST(SimpleVotingWithFakesNotGiveTest, 4Voters1Coercer)
{
    TestVerif verify("../tests/examples/svotewithfakes/4Voters1CoercerWithFakesNG.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 41391);
}