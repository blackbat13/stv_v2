#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(ClockTest, Clock6Hours15Minutes)
{
    TestVerif verify("../tests/integration/examples/clock/Clock6Hours15Minutes.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 90);
}

TEST(ClockTest, Clock6Hours15MinutesAndDays)
{
    TestVerif verify("../tests/integration/examples/clock/Clock6Hours15MinutesAndDays.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 1155);
}

TEST(ClockTestF, Clock6Hours15Minutes)
{
    TestVerif verify("../tests/integration/examples/clock/Clock6Hours15MinutesF.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(ClockTestF, Clock6Hours15MinutesAndDays)
{
    TestVerif verify("../tests/integration/examples/clock/Clock6Hours15MinutesAndDaysF.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(ClockTestF, Clock6Hours15MinutesAndDays2)
{
    TestVerif verify("../tests/integration/examples/clock/Clock6Hours15MinutesAnd4DaysFFalse.txt", true);

    EXPECT_EQ(verify.result, false);
}