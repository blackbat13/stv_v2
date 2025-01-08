#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(SimpleVotingRunTest, 1Voter1CoercerRun)
{
    TestVerif verify("../tests/examples/svoterun/1Voter1CoercerRun.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTest, 1Voter1CoercerNotRun)
{
    TestVerif verify("../tests/examples/svoterun/1Voter1CoercerNotRun.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTest, 2Voters1CoercerRun)
{
    TestVerif verify("../tests/examples/svoterun/2Voters1CoercerRun.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTest, 2Voters1CoercerNotRun)
{
    TestVerif verify("../tests/examples/svoterun/2Voters1CoercerNotRun.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTest, 1Voter2CoercersRun)
{
    TestVerif verify("../tests/examples/svoterun/1Voter2CoercersRun.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTest, 1Voter2CoercersNotRun)
{
    TestVerif verify("../tests/examples/svoterun/1Voter2CoercersNotRun.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTest, 1Voter2CoercersRunHide)
{
    TestVerif verify("../tests/examples/svoterun/1Voter2CoercersRunHide.txt", true);

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTestF, 2Voters1CoercerRun)
{
    TestVerif verify("../tests/examples/svoterun/2Voters1CoercerRunF.txt", true);

    EXPECT_EQ(verify.result, false);
}

TEST(SimpleVotingRunTestF, 2Voters1CoercerNotRun)
{
    TestVerif verify("../tests/examples/svoterun/2Voters1CoercerNotRunF.txt", true);

    EXPECT_EQ(verify.result, true);
}
