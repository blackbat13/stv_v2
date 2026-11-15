#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(GeneralCorrectness, Trains)
{
    TestVerif verify("../tests/integration/examples/trains/Trains.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 8);
}

TEST(GeneralCorrectness, SelectVoteRevoting)
{
    TestVerif verify("../tests/integration/examples/ssvr/SelectVoteRevoting.txt", true);

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 38530);
}

TEST(GeneralCorrectness, SimpleVoting)
{
    TestVerif verify("../tests/integration/examples/svote/SimpleVoting.txt", true);

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 15);
}