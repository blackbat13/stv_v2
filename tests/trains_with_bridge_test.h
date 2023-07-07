#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(TrainsWithBridgeTest, 1Train1ControllerOnBridge)
{
    TestVerif verify("../tests/examples/trains/1Train1ControllerOnBridge.txt");

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 3);
}

TEST(TrainsWithBridgeTest, 2Trains1ControllerOnBridge)
{
    TestVerif verify("../tests/examples/trains/2Trains1ControllerOnBridge.txt");

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 17);
}