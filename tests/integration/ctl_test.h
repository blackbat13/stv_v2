#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(CTLTest, GOK)
{
    TestVerif verify("../tests/integration/examples/ctl/CTLSyntaxTestOK.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(CTLTest, GERR)
{
    TestVerif verify("../tests/integration/examples/ctl/CTLSyntaxTestERR.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(CTLTest, FOK)
{
    TestVerif verify("../tests/integration/examples/ctl/CTLSyntaxTestOK2.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(CTLTest, FERR)
{
    TestVerif verify("../tests/integration/examples/ctl/CTLSyntaxTestERR2.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(CTLTest, DeeperPathERR)
{
    TestVerif verify("../tests/integration/examples/ctl/CTLSyntaxTestERR3.txt", true);

    EXPECT_EQ(verify.result, false);
}