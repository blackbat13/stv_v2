#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(CTLTest, GOK)
{
    TestVerif verify("../tests/examples/ctl/CTLSyntaxTestOK.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(CTLTest, GERR)
{
    TestVerif verify("../tests/examples/ctl/CTLSyntaxTestERR.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(CTLTest, FOK)
{
    TestVerif verify("../tests/examples/ctl/CTLSyntaxTestOK2.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(CTLTest, FERR)
{
    TestVerif verify("../tests/examples/ctl/CTLSyntaxTestERR2.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(CTLTest, DeeperPathERR)
{
    TestVerif verify("../tests/examples/ctl/CTLSyntaxTestERR3.txt", true);

    EXPECT_EQ(verify.result, false);
}