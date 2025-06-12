#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(NewSimpleVotingTest, 1Voter1Coercer)
{
    TestVerif verify("../tests/examples/svote/1Voter1Coercer.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(NewSimpleVotingTest, 2Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/2Voters1Coercer.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(NewSimpleVotingTest, 3Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/3Voters1Coercer.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(NewSimpleVotingTest, 4Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/4Voters1Coercer.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(NewSimpleVotingTestF, 1Voter1Coercer)
{
    TestVerif verify("../tests/examples/svote/1Voter1CoercerF.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingTestF, 2Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/2Voters1CoercerF.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingTestF, 3Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/3Voters1CoercerF.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingTestF, 4Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/4Voters1CoercerF.txt", true);

    EXPECT_EQ(verify.result, true);
}