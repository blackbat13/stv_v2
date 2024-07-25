#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(SimpleVotingTestButFaster, 1Voter1Coercer)
{
    TestVerif verify("../tests/examples/svote/1Voter1Coercer.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(SimpleVotingTestButFaster, 2Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/2Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(SimpleVotingTestButFaster, 3Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/3Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(SimpleVotingTestButFaster, 4Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/4Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(SimpleVotingTestFButFaster, 1Voter1Coercer)
{
    TestVerif verify("../tests/examples/svote/1Voter1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingTestFButFaster, 2Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/2Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingTestFButFaster, 3Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/3Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingTestFButFaster, 4Voters1Coercer)
{
    TestVerif verify("../tests/examples/svote/4Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}