/**
 * @file agent_test.h
 * @brief Unit tests for Agent.
 */

#include "helpers.h"

class AgentTest : public UnitTest {
protected:
    ModelArena arena;
};

TEST_F(AgentTest, ConstructorSetsIdAndName) {
    Agent agent(3, "Voter");
    EXPECT_EQ(agent.id, 3);
    EXPECT_EQ(agent.name, "Voter");
    EXPECT_EQ(agent.initState, nullptr);
    EXPECT_TRUE(agent.localStates.empty());
}

TEST_F(AgentTest, IncludesStateFindsAnEquivalentState) {
    Agent* agent = arena.agent(0, "A");
    LocalState* s0 = arena.state(agent, "s", {{"x", 0}});
    LocalState* s1 = arena.state(agent, "s", {{"x", 1}});

    LocalState probe;
    probe.agent = agent;
    probe.name = "s";
    probe.environment = {{"x", 1}};
    EXPECT_EQ(agent->includesState(&probe), s1);
    probe.environment = {{"x", 0}};
    EXPECT_EQ(agent->includesState(&probe), s0);
    probe.environment = {{"x", 2}};
    EXPECT_EQ(agent->includesState(&probe), nullptr);
}

TEST_F(AgentTest, PartialReduceWithNoVariablesIsANoOp) {
    Agent* agent = arena.agent(0, "A");
    LocalState* s0 = arena.state(agent, "s0", {{"x", 0}});
    LocalState* s1 = arena.state(agent, "s1", {{"x", 1}});
    arena.transition(s0, s1, "go");

    agent->partialReduceModel({});
    EXPECT_EQ(agent->localStates.size(), 2u);
    EXPECT_EQ(agent->localTransitions.size(), 1u);
    EXPECT_EQ(s1->environment, (map<string, int>{{"x", 1}}));
}

namespace {
/// @brief Agent with persistent x, y. Forgetting x merges the two "s1" states.
///
///   s0{x=0,y=0} -a-> s1{x=0,y=1} -b[x==0]-> s0
///   s0{x=0,y=0} -a-> s1{x=1,y=1} -b[x==1]-> s0
///   s0          -c[y==0]-> s0
struct ReducibleAgent {
    Agent* agent;
    LocalState *s0, *s1a, *s1b;
    LocalTransition *a1, *a2, *b1, *b2, *c;
    Var *x, *y;

    explicit ReducibleAgent(ModelArena& arena) {
        agent = arena.agent(0, "A");
        x = arena.var(agent, "x", true);
        y = arena.var(agent, "y", true);
        s0 = arena.state(agent, "s0", {{"x", 0}, {"y", 0}});
        s1a = arena.state(agent, "s1", {{"x", 0}, {"y", 1}});
        s1b = arena.state(agent, "s1", {{"x", 1}, {"y", 1}});
        a1 = arena.transition(s0, s1a, "a");
        a2 = arena.transition(s0, s1b, "a");
        b1 = arena.transition(s1a, s0, "b");
        b2 = arena.transition(s1b, s0, "b");
        c = arena.transition(s0, s0, "c");
        b1->conditions.insert(arena.condition(x, ConditionOperator::Equals, 0));
        b2->conditions.insert(arena.condition(new ExprEq(new ExprIdent("x"), new ExprConst(1))));
        c->conditions.insert(arena.condition(new ExprEq(new ExprIdent("y"), new ExprConst(0))));
    }
};
}

TEST_F(AgentTest, PartialReduceMergesStatesThatDifferOnlyInForgottenVariables) {
    ReducibleAgent model(arena);
    model.agent->partialReduceModel({"x"});

    ASSERT_EQ(model.agent->localStates.size(), 2u);
    EXPECT_EQ(model.agent->localStates[0], model.s0);
    EXPECT_EQ(model.agent->localStates[1], model.s1a);     // first state of each group is kept
    EXPECT_EQ(model.s0->environment, (map<string, int>{{"y", 0}}));
    EXPECT_EQ(model.s1a->environment, (map<string, int>{{"y", 1}}));
    EXPECT_EQ(model.agent->initState, model.s0);
    // ids are renumbered contiguously
    EXPECT_EQ(model.agent->localStates[0]->id, 0u);
    EXPECT_EQ(model.agent->localStates[1]->id, 1u);
}

TEST_F(AgentTest, PartialReduceRedirectsAndDeduplicatesTransitions) {
    ReducibleAgent model(arena);
    model.agent->partialReduceModel({"x"});

    // a1/a2 and b1/b2 become identical after the merge; one of each pair survives.
    ASSERT_EQ(model.agent->localTransitions.size(), 3u);
    EXPECT_EQ(model.agent->localTransitions[0], model.a1);
    EXPECT_EQ(model.agent->localTransitions[1], model.b1);
    EXPECT_EQ(model.agent->localTransitions[2], model.c);
    for (size_t i = 0; i < model.agent->localTransitions.size(); i++) {
        EXPECT_EQ(model.agent->localTransitions[i]->id, static_cast<int>(i));
    }
    EXPECT_EQ(model.a1->to, model.s1a);
    EXPECT_EQ(model.s0->localTransitions, (set<LocalTransition*>{model.a1, model.c}));
    EXPECT_EQ(model.s1a->localTransitions, (set<LocalTransition*>{model.b1}));
}

TEST_F(AgentTest, PartialReduceDropsConditionsAndVariablesThatMentionForgottenVariables) {
    ReducibleAgent model(arena);
    model.agent->partialReduceModel({"x"});

    EXPECT_TRUE(model.b1->conditions.empty());             // var-based condition on x
    EXPECT_TRUE(model.b2->conditions.empty());             // expression condition on x
    EXPECT_EQ(model.c->conditions.size(), 1u);             // condition on y is kept
    ASSERT_EQ(model.agent->vars.size(), 1u);
    EXPECT_EQ((*model.agent->vars.begin())->name, "y");
}
