#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(NewSimpleVotingTestButFaster, 1Voter1Coercer)
{
    IterativeVerif verify("../tests/examples/svote/1Voter1Coercer.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(NewSimpleVotingTestButFaster, 2Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svote/2Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(NewSimpleVotingTestButFaster, 3Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svote/3Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(NewSimpleVotingTestButFaster, 4Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svote/4Voters1Coercer.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(NewSimpleVotingTestFButFaster, 1Voter1Coercer)
{
    IterativeVerif verify("../tests/examples/svote/1Voter1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingTestFButFaster, 2Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svote/2Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingTestFButFaster, 3Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svote/3Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingTestFButFaster, 4Voters1Coercer)
{
    IterativeVerif verify("../tests/examples/svote/4Voters1CoercerF.txt");

    EXPECT_EQ(verify.result, true);
}