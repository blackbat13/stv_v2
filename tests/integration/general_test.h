#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(GeneralCorrectness, Trains)
{
    TestVerif verify("../tests/integration/examples/trains/Trains.txt", true);

    EXPECT_EQ(verify.result, true);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 8);
}

TEST(GeneralCorrectness, SelectVoteRevoting)
{
    TestVerif verify("../tests/integration/examples/ssvr/SelectVoteRevoting.txt", true);

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 38530);
}

TEST(GeneralCorrectness, SimpleVoting)
{
    TestVerif verify("../tests/integration/examples/svote/SimpleVoting.txt", true);

    EXPECT_EQ(verify.result, false);

    EXPECT_EQ((verify.generator->getCurrentGlobalModel())->globalStates.size(), 15);
}

TEST(GeneralCorrectness, DeadEndDoesNotReachFGoal)
{
    // the Dealer performs every sync and every path ends in a state without successors, so the coalition can't force win==1
    TestVerif verify("../tests/integration/examples/deadlock/DeadEndBeforeGoalF.txt", 0);

    EXPECT_FALSE(verify.result);
}

TEST(GeneralCorrectness, DeadEndStatesAreRecorded)
{
    // all four end states of the full model have no outgoing transitions; with epsilon transitions there are none
    struct ConfigRestore {
        Cfg saved = config;
        ~ConfigRestore() { config = saved; }
    } restoreConfig;

    config.add_epsilon_transitions = false;
    TestVerif verify("../tests/integration/examples/deadlock/DeadEndBeforeGoalF.txt", 1);
    EXPECT_EQ(verify.generator->getDeadEndStates().size(), 4u);
    for (auto* state : verify.generator->getDeadEndStates()) {
        EXPECT_TRUE(state->globalTransitions.empty());
    }

    config.add_epsilon_transitions = true;
    TestVerif withEpsilon("../tests/integration/examples/deadlock/DeadEndBeforeGoalF.txt", 1);
    EXPECT_TRUE(withEpsilon.generator->getDeadEndStates().empty());
}

TEST(GeneralCorrectness, DeadEndKeepsGHolding)
{
    // a state without successors stays put forever, so a G property that held there keeps holding
    TestVerif verify("../tests/integration/examples/deadlock/DeadEndKeepsG.txt", 0);

    EXPECT_TRUE(verify.result);
}

static LocalState* variablesTestFindLocalState(GlobalModel* model, const string& agentName, const string& stateName) {
    for (auto* agent : model->agents) {
        if (agent->name != agentName) {
            continue;
        }
        for (auto* localState : agent->localStates) {
            if (localState->name == stateName) {
                return localState;
            }
        }
    }
    return nullptr;
}

TEST(GeneralCorrectness, NonPersistentVariableGoesBackToInitialValue)
{
    // x is not persistent: set to 1 by 'set', back to its initial 5 after 'step'; y is persistent and keeps 1
    TestVerif verify("../tests/integration/examples/variables/NonPersistentReset.txt", 1);
    EXPECT_TRUE(verify.result);

    auto* model = verify.generator->getCurrentGlobalModel();
    auto* q0 = variablesTestFindLocalState(model, "A", "q0");
    auto* q1 = variablesTestFindLocalState(model, "A", "q1");
    auto* q2 = variablesTestFindLocalState(model, "A", "q2");
    ASSERT_NE(q0, nullptr);
    ASSERT_NE(q1, nullptr);
    ASSERT_NE(q2, nullptr);
    EXPECT_EQ(q0->environment, (map<string, int>{{"x", 5}, {"y", 0}}));
    EXPECT_EQ(q1->environment, (map<string, int>{{"x", 1}, {"y", 1}}));
    EXPECT_EQ(q2->environment, (map<string, int>{{"x", 5}, {"y", 1}}));
}

TEST(GeneralCorrectness, NonPersistentVariableHasValueInEveryState)
{
    // the Controller's win is only assigned by setC1, but it still has a value after setC0
    TestVerif verify("../tests/integration/examples/variables/NonPersistentInEveryState.txt", 1);
    EXPECT_TRUE(verify.result);

    for (auto* state : verify.generator->getCurrentGlobalModel()->globalStates) {
        EXPECT_EQ(state->getGlobalStateEnvironment().count("win"), 1u) << state->hash;
    }
}
