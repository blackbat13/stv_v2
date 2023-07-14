#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(SimpleVotingTest, 1Voter1Coercer)
{
    TestVerif verify("../tests/examples/svote/1Voter1Coercer.txt");

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 15);
}

TEST(SimpleVotingTest, 2Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/2Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 133);
}

TEST(SimpleVotingTest, 3Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/3Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 1071);
}

TEST(SimpleVotingTest, 4Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/4Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 8461);
}

TEST(SimpleVotingTestF, 1Voter1Coercer)
{
    TestVerif verify("../tests/examples/svote/1Voter1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingTestF, 2Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/2Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingTestF, 3Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/3Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingTestF, 4Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/4Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}