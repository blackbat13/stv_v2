#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(HartleyDetectionTest, HartleyDetectionTest1)
{
    TestVerif verify("../tests/examples/hartley/HartleyDetection1.txt", true);

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.hartley, "Coercer1");
    EXPECT_EQ(verify.le, false);
    EXPECT_EQ(verify.hCoeff, 1);
}

TEST(HartleyDetectionTest, HartleyDetectionTest2)
{
    TestVerif verify("../tests/examples/hartley/HartleyDetection2.txt", true);

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.hartley, "Voter1");
    EXPECT_EQ(verify.le, true);
    EXPECT_EQ(verify.hCoeff, 2);
}

TEST(HartleyDetectionTest, HartleyDetectionTest3)
{
    TestVerif verify("../tests/examples/hartley/HartleyDetection3.txt", true);

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.hartley, "Coercer1");
    EXPECT_EQ(verify.le, true);
    EXPECT_EQ(verify.hCoeff, 1);
}

TEST(HartleyVerificationTest, HartleyOK)
{
    TestVerif verify("../tests/examples/hartley/HartleyVerificationOK.txt", true);

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.hartley, "Coercer1");
}

TEST(HartleyVerificationTest, HartleyOK2)
{
    TestVerif verify("../tests/examples/hartley/HartleyVerificationOK2.txt", true);

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.hartley, "Coercer1");
}

TEST(HartleyVerificationTest, HartleyOK3)
{
    TestVerif verify("../tests/examples/hartley/HartleyVerificationOK3.txt", true);

    EXPECT_EQ(verify.result, true);
    EXPECT_EQ(verify.hartley, "Coercer1");
}

TEST(HartleyVerificationTest, HartleyERR)
{
    TestVerif verify("../tests/examples/hartley/HartleyVerificationERR.txt", true);

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.hartley, "Coercer1");
}

TEST(HartleyVerificationTest, HartleyERR2)
{
    TestVerif verify("../tests/examples/hartley/HartleyVerificationERR2.txt", true);

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.hartley, "Coercer1");
}

TEST(HartleyVerificationTest, HartleyERR3)
{
    TestVerif verify("../tests/examples/hartley/HartleyVerificationERR3.txt", true);

    EXPECT_EQ(verify.result, false);
    EXPECT_EQ(verify.hartley, "Coercer1");
}