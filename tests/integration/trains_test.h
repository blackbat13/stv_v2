#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(TrainsTest, 1Train1Controller)
{
    TestVerif verify("../tests/integration/examples/trains/1Train1Controller.txt", true);

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 3);
}

TEST(TrainsTest, 2Trains1Controller)
{
    TestVerif verify("../tests/integration/examples/trains/2Trains1Controller.txt", true);

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 8);
}

TEST(TrainsTest, MultiAgentCoalitionDecisionsAreLocalStateBased)
{
    TestVerif verify("../tests/integration/examples/trains/Test.txt", 0);

    EXPECT_FALSE(verify.result);
}

TEST(TrainsTest, CoalitionChoiceDisablesIncompatibleOpponentSynchronization)
{
    TestVerif verify("../tests/integration/examples/trains/CoalitionOpponentCompatibility.txt", 0);

    EXPECT_TRUE(verify.result);
}

TEST(TrainsTest, ActiveControllerCanForceTrain2IntoTunnel)
{
    // Controller performs every sync and the trains only wait, so it can keep Train1 out at green_3 and let Train2 in.
    TestVerif verify("../tests/integration/examples/trains/Trains2ControllerActiveSync.txt", 0);

    EXPECT_TRUE(verify.result);
}

TEST(TrainsTest, FailedOpponentMoveIsNotCompensatedByOpponentSynchronization)
{
    // Controller only waits for syncs, so Train1 can loop in/out/return forever; a winning
    // in_Train2 sync must not hide the losing return_1 move of Train1.
    TestVerif verify("../tests/integration/examples/trains/Trains2ControllerPassiveSync.txt", 0);

    EXPECT_FALSE(verify.result);
}
