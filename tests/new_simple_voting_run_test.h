#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(NewSimpleVotingRunTest, 1Voter1CoercerRun)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter1CoercerRun.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTest, 1Voter1CoercerNotRun)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter1CoercerNotRun.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTest, 2Voters1CoercerRun)
{
    IterativeVerif verify("../tests/examples/svoterun/2Voters1CoercerRun.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTest, 2Voters1CoercerNotRun)
{
    IterativeVerif verify("../tests/examples/svoterun/2Voters1CoercerNotRun.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTest, 1Voter2CoercersRun)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter2CoercersRun.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTest, 1Voter2CoercersNotRun)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter2CoercersNotRun.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTest, 1Voter2CoercersRunHide)
{
    IterativeVerif verify("../tests/examples/svoterun/1Voter2CoercersRunHide.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestF, 2Voters1CoercerRun)
{
    IterativeVerif verify("../tests/examples/svoterun/2Voters1CoercerRunF.txt", 0);

    EXPECT_EQ(verify.result, true);
}

TEST(NewSimpleVotingRunTestF, 2Voters1CoercerNotRun)
{
    IterativeVerif verify("../tests/examples/svoterun/2Voters1CoercerNotRunF.txt", 0);

    EXPECT_EQ(verify.result, true);
}
