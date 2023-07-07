#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(ClockTest, Clock6Hours15Minutes)
{
    TestVerif verify("../tests/examples/clock/Clock6Hours15Minutes.txt");

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 90);
}

TEST(ClockTest, Clock6Hours15MinutesAndDays)
{
    TestVerif verify("../tests/examples/clock/Clock6Hours15MinutesAndDays.txt");

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 1155);
}

TEST(ClockTestF, Clock6Hours15Minutes)
{
    TestVerif verify("../tests/examples/clock/Clock6Hours15MinutesF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(ClockTestF, Clock6Hours15MinutesAndDays)
{
    TestVerif verify("../tests/examples/clock/Clock6Hours15MinutesAndDaysF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(ClockTestF, Clock6Hours15MinutesAndDays2)
{
    TestVerif verify("../tests/examples/clock/Clock6Hours15MinutesAndDaysFFalse.txt");

    EXPECT_EQ(verify.result, false);
}