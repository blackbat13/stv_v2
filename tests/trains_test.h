#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(TrainsTest, 1Train1Controller)
{
    TestVerif verify("../tests/examples/trains/1Train1Controller.txt");

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 3);
}

TEST(TrainsTest, 2Trains1Controller)
{
    TestVerif verify("../tests/examples/trains/2Trains1Controller.txt");

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 8);
}