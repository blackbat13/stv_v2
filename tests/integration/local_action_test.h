#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(LocalActionTest, CoalitionCantKeepOutOfOpponentSynchronizations)
{
    // C has only the shared actions a and b and both lose; whichever C chooses, O can complete it,
    // so the other shared action is no way out
    TestVerif onTheFly("../tests/integration/examples/localActions/SharedChoicesAllLose.txt", 0);
    EXPECT_FALSE(onTheFly.result);

    TestVerif full("../tests/integration/examples/localActions/SharedChoicesAllLose.txt", 1);
    EXPECT_FALSE(full.result);
}

TEST(LocalActionTest, ProbabilisticCoalitionChoosesWithItsOwnLocalNames)
{
    // the Voter's local names differ from the global ones, yet it chooses askHelp; the Clerk answers both with its one local action
    TestVerif verify("../tests/integration/examples/localActions/ProbabilisticChoiceWithOwnLocalNames.txt", 4);
    EXPECT_TRUE(verify.result);
}

TEST(LocalActionTest, ProbabilisticOpponentPicksTheOutcomeOfALocalAction)
{
    // the Voter's pn_Voter1 turns into pun_Voter1 or npun_Voter1 as the Coercer picks; each gives npun1 with 0.7, which is below 0.9
    TestVerif verify("../tests/integration/examples/probability/NewProbTest1.txt", 4);
    EXPECT_FALSE(verify.result);
}

TEST(LocalActionTest, RefusableChoiceWinsWhenEveryAnswerIsSafe)
{
    // C chooses a: O either completes it, which is safe, or refuses it, and then only P's harmless move or nothing happens
    TestVerif onTheFly("../tests/integration/examples/localActions/SharedChoicesOneWins.txt", 0);
    EXPECT_TRUE(onTheFly.result);

    TestVerif full("../tests/integration/examples/localActions/SharedChoicesOneWins.txt", 1);
    EXPECT_TRUE(full.result);
}
