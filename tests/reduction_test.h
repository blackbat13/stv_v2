#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(ReductionTest, 2Trains1ControllerOnBridgeReduced)
{
    TestVerif reducedVerify("../tests/examples/trains/2Trains1ControllerOnBridge.txt", true);

    EXPECT_EQ(reducedVerify.result, false);

    EXPECT_EQ((reducedVerify.generator->getCurrentGlobalModel())->globalStates.size(), 17);
}