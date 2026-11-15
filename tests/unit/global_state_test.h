/**
 * @file global_state_test.h
 * @brief Unit tests for GlobalState and GlobalTransition.
 */

#include "helpers.h"

class GlobalStateTest : public UnitTest {
protected:
    ModelArena arena;
};

TEST_F(GlobalStateTest, ConstructorDefaults) {
    GlobalState state;
    EXPECT_FALSE(state.isExpanded);
    EXPECT_EQ(state.verificationStatus, GlobalStateVerificationStatus::UNVERIFIED);
    EXPECT_EQ(state.stateVerifResult, VerifResult::NOT_VERIFIED);
    EXPECT_FALSE(state.goodState);
    EXPECT_EQ(state.probabilityLoop, nullptr);
    EXPECT_TRUE(state.globalTransitions.empty());
}

TEST_F(GlobalStateTest, InitialProbabilityDependsOnProbabilisticMode) {
    config.probability = false;
    EXPECT_FLOAT_EQ(GlobalState().probability, 1.0f);
    config.probability = true;
    EXPECT_FLOAT_EQ(GlobalState().probability, 0.0f);
}

TEST_F(GlobalStateTest, EnvironmentMergesAllLocalEnvironments) {
    Agent* a = arena.agent(0, "A");
    Agent* b = arena.agent(1, "B");
    GlobalState* state = arena.globalState({arena.state(a, "s", {{"x", 1}}), arena.state(b, "t", {{"y", 2}, {"z", 3}})});
    EXPECT_EQ(state->getGlobalStateEnvironment(), (map<string, int>{{"x", 1}, {"y", 2}, {"z", 3}}));
    EXPECT_TRUE(GlobalState().getGlobalStateEnvironment().empty());
}

TEST_F(GlobalStateTest, ToStringNestsLocalStates) {
    Agent* a = arena.agent(0, "A");
    GlobalState* state = arena.globalState({arena.state(a, "s", {{"x", 1}})});
    EXPECT_EQ(state->toString(), "hash:\"0;\",\nlocalStates:[\n\tid:0,\n\tname:\"s\",\n\tagent:A,\n\tvars:[\n\t\tx=1\n\t]\n]");
}

TEST(GlobalTransition, IdsAreUniqueAndIncreasing) {
    GlobalTransition first;
    GlobalTransition second;
    GlobalTransition third;
    EXPECT_EQ(second.id, first.id + 1);
    EXPECT_EQ(third.id, second.id + 1);
}

class GlobalTransitionTest : public UnitTest {
protected:
    ModelArena arena;
    Agent* a = arena.agent(0, "A");
    Agent* b = arena.agent(1, "B");
    LocalState* sa = arena.state(a, "s");
    LocalState* sb = arena.state(b, "t");
};

TEST_F(GlobalTransitionTest, JoinLocalTransitionNamesEndsWithSeparator) {
    GlobalTransition* transition = arena.globalTransition(nullptr, nullptr, {arena.transition(sa, sa, "only")});
    EXPECT_EQ(transition->joinLocalTransitionNames(), "only;");
    EXPECT_EQ(transition->joinLocalTransitionNames(','), "only,");

    GlobalTransition* pair = arena.globalTransition(nullptr, nullptr, {arena.transition(sa, sa, "x"), arena.transition(sb, sb, "x")});
    EXPECT_EQ(pair->joinLocalTransitionNames(), "x;x;");

    GlobalTransition* empty = arena.globalTransition(nullptr, nullptr, {});
    EXPECT_EQ(empty->joinLocalTransitionNames(), "");
}

TEST_F(GlobalTransitionTest, ProbabilityIsTheProductOfLocalProbabilities) {
    GlobalTransition* transition = arena.globalTransition(nullptr, nullptr, {
        arena.transition(sa, sa, "p", "", true, 2, 0.5f),
        arena.transition(sb, sb, "p", "", true, 2, 0.25f)});
    EXPECT_FLOAT_EQ(transition->getProbability(), 0.125f);
    EXPECT_FLOAT_EQ(arena.globalTransition(nullptr, nullptr, {})->getProbability(), 1.0f);
}
