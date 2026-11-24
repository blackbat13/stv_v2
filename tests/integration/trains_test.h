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

TEST(TrainsTest, FailedRevertKeepsControllerGreenDecision)
{
    // Controller sees only green/red, so at green it lets in either Train1 or Train2, never both.
    // A failed revertLastDecision used to wipe the fixed green decision, letting the DFS pick both.
    TestVerif onTheFly("../tests/integration/examples/trains/Trains2ControllerUniformGreen.txt", 0);
    EXPECT_FALSE(onTheFly.result);

    TestVerif full("../tests/integration/examples/trains/Trains2ControllerUniformGreen.txt", 1);
    EXPECT_FALSE(full.result);
}
