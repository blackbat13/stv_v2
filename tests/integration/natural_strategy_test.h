#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(NaturalStrategyTest, Test1)
{
    TestVerif strategyVerify("../tests/integration/examples/naturalStrategy/naturalStrategyTest.txt", 3);

    EXPECT_EQ(strategyVerify.result, true);
}

TEST(NaturalStrategyTest, Test2)
{
    TestVerif strategyVerify("../tests/integration/examples/naturalStrategy/naturalStrategyTest2.txt", 3);

    EXPECT_EQ(strategyVerify.result, true);
}

TEST(NaturalStrategyTest, Test3)
{
    // the Voter picks one day and the Calendar can pick another one, so no sync fires: a deadlock loses the F goal
    TestVerif strategyVerify("../tests/integration/examples/naturalStrategy/naturalStrategyTest3.txt", 3);

    EXPECT_EQ(strategyVerify.result, false);
}

TEST(NaturalStrategyTest, Test4)
{
    // every day leads outside, but the days are different local actions of the Voter: the Calendar can pick another day than the Voter, so no sync fires and the deadlock loses the F goal
    TestVerif strategyVerify("../tests/integration/examples/naturalStrategy/naturalStrategyTest4.txt", 3);

    EXPECT_EQ(strategyVerify.result, false);
}

TEST(NaturalStrategyTest, Test5)
{
    TestVerif strategyVerify("../tests/integration/examples/naturalStrategy/naturalStrategyTest5.txt", 3);

    EXPECT_EQ(strategyVerify.result, true);
}

TEST(NaturalStrategyTest, DifferentPaths)
{
    TestVerif strategyVerify("../tests/integration/examples/naturalStrategy/naturalStrategyDifferentPaths.txt", 3);

    EXPECT_EQ(strategyVerify.result, true);
}