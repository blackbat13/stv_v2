/**
 * @file local_state_test.h
 * @brief Unit tests for LocalState.
 */

#include "helpers.h"

class LocalStateTest : public UnitTest {
protected:
    ModelArena arena;
    Agent* agent = arena.agent(0, "A");
    Agent* other = arena.agent(1, "B");
};

TEST_F(LocalStateTest, CompareAcceptsSameNameAndEnvironment) {
    LocalState* a = arena.state(agent, "s", {{"x", 1}, {"y", 2}});
    LocalState* b = arena.state(agent, "s", {{"y", 2}, {"x", 1}});
    EXPECT_TRUE(a->compare(b));
    EXPECT_TRUE(b->compare(a));
}

TEST_F(LocalStateTest, CompareIgnoresTheId) {
    LocalState* a = arena.state(agent, "s");
    LocalState* b = arena.state(agent, "s");
    ASSERT_NE(a->id, b->id);
    EXPECT_TRUE(a->compare(b));
}

TEST_F(LocalStateTest, CompareRejectsDifferences) {
    LocalState* base = arena.state(agent, "s", {{"x", 1}});
    EXPECT_FALSE(base->compare(arena.state(other, "s", {{"x", 1}})));          // different agent
    EXPECT_FALSE(base->compare(arena.state(agent, "t", {{"x", 1}})));          // different name
    EXPECT_FALSE(base->compare(arena.state(agent, "s", {{"x", 2}})));          // different value
    EXPECT_FALSE(base->compare(arena.state(agent, "s", {{"z", 1}})));          // different variable
    EXPECT_FALSE(base->compare(arena.state(agent, "s", {{"x", 1}, {"y", 0}}))); // different size
    EXPECT_FALSE(base->compare(arena.state(agent, "s")));                      // empty environment
}

TEST_F(LocalStateTest, ToStringListsIdNameAgentAndVariables) {
    LocalState* state = arena.state(agent, "voting", {{"a", 1}, {"b", 2}});
    EXPECT_EQ(state->toString(), "id:0,\nname:\"voting\",\nagent:A,\nvars:[\n\ta=1,\n\tb=2\n]");
    EXPECT_EQ(state->toString(">"), ">id:0,\n>name:\"voting\",\n>agent:A,\n>vars:[\n\t>a=1,\n\t>b=2\n>]");
}

TEST_F(LocalStateTest, ToStringWithEmptyEnvironmentKeepsBrackets) {
    LocalState* state = arena.state(agent, "idle");
    EXPECT_EQ(state->toString(), "id:0,\nname:\"idle\",\nagent:A,\nvars:[\n]");
}

TEST_F(LocalStateTest, EnvironmentHashListsValuesInKeyOrder) {
    LocalState* state = arena.state(agent, "s", {{"b", 2}, {"a", 1}});
    EXPECT_EQ(state->environmentHash(), "a=1;b=2;");
    EXPECT_EQ(arena.state(agent, "s")->environmentHash(), "");
}

TEST_F(LocalStateTest, EnvironmentHashIncludesVariableNames) {
    LocalState* state = arena.state(agent, "s", {{"b", 2}, {"a", 1}});
    EXPECT_EQ(state->environmentHash(), "a=1;b=2;");
    EXPECT_NE(arena.state(agent, "s", {{"a", 1}})->environmentHash(),
              arena.state(agent, "s", {{"b", 1}})->environmentHash());
}
