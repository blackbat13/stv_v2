#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(SVoteVariableReductionTest, TwoVotersThreeCandidates)
{
    TestVerif verify("../tests/examples/svote_variable_reduction/svote_2voters_3cands.stv", true);

    EXPECT_EQ(verify.result, false);
}
