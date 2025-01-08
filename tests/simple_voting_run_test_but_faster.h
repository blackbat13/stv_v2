#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(SimpleVotingRunTestButFaster, 1Voter1CoercerRun)
{
    TestVerif verify("../tests/examples/svoterun/1Voter1CoercerRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTestButFaster, 1Voter1CoercerNotRun)
{
    TestVerif verify("../tests/examples/svoterun/1Voter1CoercerNotRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTestButFaster, 2Voters1CoercerRun)
{
    TestVerif verify("../tests/examples/svoterun/2Voters1CoercerRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTestButFaster, 2Voters1CoercerNotRun)
{
    TestVerif verify("../tests/examples/svoterun/2Voters1CoercerNotRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTestButFaster, 1Voter2CoercersRun)
{
    TestVerif verify("../tests/examples/svoterun/1Voter2CoercersRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTestButFaster, 1Voter2CoercersNotRun)
{
    TestVerif verify("../tests/examples/svoterun/1Voter2CoercersNotRun.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTestButFaster, 1Voter2CoercersRunHide)
{
    TestVerif verify("../tests/examples/svoterun/1Voter2CoercersRunHide.txt");

    EXPECT_EQ(verify.result, true);
}

TEST(SimpleVotingRunTestFButFaster, 2Voters1CoercerRun)
{
    TestVerif verify("../tests/examples/svoterun/2Voters1CoercerRunF.txt");

    EXPECT_EQ(verify.result, false);
}

TEST(SimpleVotingRunTestFButFaster, 2Voters1CoercerNotRun)
{
    TestVerif verify("../tests/examples/svoterun/2Voters1CoercerNotRunF.txt");

    EXPECT_EQ(verify.result, true);
}
