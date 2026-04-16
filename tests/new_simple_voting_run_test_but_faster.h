#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(NewSimpleVotingRunTestButFaster, 1Voter1CoercerRun)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter1CoercerRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestButFaster, 1Voter1CoercerNotRun)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter1CoercerNotRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestButFaster, 2Voters1CoercerRun)
{
    IterativeVerif verify("../tests/examples/svoterun/2Voters1CoercerRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestButFaster, 2Voters1CoercerNotRun)
{
    IterativeVerif verify("../tests/examples/svoterun/2Voters1CoercerNotRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestButFaster, 1Voter2CoercersRun)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter2CoercersRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestButFaster, 1Voter2CoercersNotRun)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter2CoercersNotRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestButFaster, 1Voter2CoercersRunHide)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter2CoercersRunHide.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestFButFaster, 2Voters1CoercerRun)
{
    IterativeVerif verify("../tests/examples/svoterun/2Voters1CoercerRunF.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestFButFaster, 2Voters1CoercerNotRun)
{
    IterativeVerif verify("../tests/examples/svoterun/2Voters1CoercerNotRunF.txt");

    EXPECT_EQ(verify.result, true);
}
