#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(NewClockTest, Clock6Hours15Minutes)
{
    IterativeVerif verify("../tests/examples/clock/Clock6Hours15Minutes.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewClockTest, Clock6Hours15MinutesAndDays)
{
    IterativeVerif verify("../tests/examples/clock/Clock6Hours15MinutesAndDays.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewClockTestF, Clock6Hours15Minutes)
{
    IterativeVerif verify("../tests/examples/clock/Clock6Hours15MinutesF.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewClockTestF, Clock6Hours15MinutesAndDays)
{
    IterativeVerif verify("../tests/examples/clock/Clock6Hours15MinutesAndDaysF.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewClockTestF, Clock6Hours15MinutesAndDays2)
{
    IterativeVerif verify("../tests/examples/clock/Clock6Hours15MinutesAndDaysFFalse.txt", 0);

    EXPECT_EQ(verify.result, false);
}