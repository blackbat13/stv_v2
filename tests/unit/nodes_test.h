/**
 * @file nodes_test.h
 * @brief Unit tests for reader/nodes.* (Assignment, TransitionTemplate, AgentTemplate).
 */

#include "helpers.h"
#include "DotGraph.hpp"

#include <algorithm>

namespace {
/// @brief A set of assignments built from (variable, expression) pairs.
set<Assignment*>* assigns(std::initializer_list<std::pair<std::string, ExprNode*>> items) {
    auto result = new set<Assignment*>();
    for (const auto& item : items) {
        result->insert(new Assignment(item.first, item.second));
    }
    return result;
}

/// @brief A non-shared transition template with probability 1.
TransitionTemplate* plain(const std::string& name, const std::string& from, const std::string& to,
                          ExprNode* condition = nullptr, set<Assignment*>* assignments = nullptr) {
    return new TransitionTemplate(-1, name, name, from, to, condition, assignments, new ProbConst(1));
}

/// @brief Collects the names of all transitions leaving @p state.
std::multiset<std::string> outgoingNames(LocalState* state) {
    std::multiset<std::string> names;
    for (auto transition : state->localTransitions) {
        names.insert(transition->name);
    }
    return names;
}
}

class AgentTemplateTest : public UnitTest {};

TEST(Assignment, AssignsEvaluatedValueIntoEnvironment) {
    Environment env{{"x", 4}};
    Assignment(  "y", new ExprAdd(new ExprIdent("x"), new ExprConst(1))).assign(env);
    EXPECT_EQ(env["y"], 5);
    Assignment("x", new ExprConst(0)).assign(env);  // overwrite
    EXPECT_EQ(env["x"], 0);
}

TEST(TransitionTemplate, ConstructorStoresAllFields) {
    auto condition = new ExprConst(1);
    auto assignments = new set<Assignment*>();
    auto probability = new ProbConst(0.5f);
    TransitionTemplate transition(2, "vote", "vote_local", "s0", "s1", condition, assignments, probability);
    EXPECT_EQ(transition.shared, 2);
    EXPECT_EQ(transition.patternName, "vote");
    EXPECT_EQ(transition.matchName, "vote_local");
    EXPECT_EQ(transition.startState, "s0");
    EXPECT_EQ(transition.endState, "s1");
    EXPECT_EQ(transition.condition, condition);
    EXPECT_EQ(transition.assignments, assignments);
    EXPECT_EQ(transition.probability, probability);
}

TEST_F(AgentTemplateTest, SettersReturnTheSameObjectForChaining) {
    AgentTemplate agentTemplate;
    EXPECT_EQ(&agentTemplate.setIdent("A"), &agentTemplate);
    EXPECT_EQ(&agentTemplate.setInitState("s0"), &agentTemplate);
    EXPECT_EQ(&agentTemplate.addLocal(nullptr), &agentTemplate);
    EXPECT_EQ(&agentTemplate.addPersistent(nullptr), &agentTemplate);
    EXPECT_EQ(&agentTemplate.addInitial(nullptr), &agentTemplate);
    EXPECT_EQ(&agentTemplate.addTransition(plain("t", "s0", "s0")), &agentTemplate);
}

TEST_F(AgentTemplateTest, AddTransitionRegistersBothEndpointsAsLocations) {
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A");
    agentTemplate.addTransition(plain("go", "s0", "s1"));
    agentTemplate.addTransition(plain("back", "s1", "s0"));
    agentTemplate.addTransition(plain("stop", "s1", "end"));

    // DotGraph is a friend of AgentTemplate and shows its locations and edges.
    DotGraph graph(&agentTemplate);
    EXPECT_EQ(graph.graphName, "A");
    std::sort(graph.nodes.begin(), graph.nodes.end());
    EXPECT_EQ(graph.nodes, (vector<string>{"end[label=\"end\"]", "s0[label=\"s0\"]", "s1[label=\"s1\"]"}));
    EXPECT_EQ(graph.edges.size(), 3u);
}

TEST_F(AgentTemplateTest, GenerateAgentBuildsVariablesAndInitialState) {
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("Voter").setInitState("s0");
    agentTemplate.addLocal(new set<string>{"tmp"});
    agentTemplate.addPersistent(new set<string>{"vote"});
    agentTemplate.addInitial(assigns({{"vote", new ExprConst(3)}}));
    agentTemplate.addTransition(plain("t", "s0", "s0"));

    Agent* agent = agentTemplate.generateAgent(4);
    EXPECT_EQ(agent->id, 4);
    EXPECT_EQ(agent->name, "Voter");
    ASSERT_NE(agent->initState, nullptr);
    EXPECT_EQ(agent->initState->id, 0u);
    EXPECT_EQ(agent->initState->name, "s0");
    EXPECT_EQ(agent->initState->agent, agent);
    EXPECT_EQ(agent->initState->environment, (map<string, int>{{"tmp", 0}, {"vote", 3}}));  // tmp has no INITIAL value: 0

    map<string, bool> persistence;
    for (auto var : agent->vars) {
        EXPECT_EQ(var->agent, agent);
        persistence[var->name] = var->persistent;
    }
    EXPECT_EQ(persistence, (map<string, bool>{{"tmp", false}, {"vote", true}}));
}

TEST_F(AgentTemplateTest, GenerateAgentExploresStatesBreadthFirst) {
    // s0 -a-> s1 -b-> s2, s0 -c-> s2, s2 -d-> s0 : three distinct states.
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A").setInitState("s0");
    agentTemplate.addTransition(plain("a", "s0", "s1"));
    agentTemplate.addTransition(plain("b", "s1", "s2"));
    agentTemplate.addTransition(plain("c", "s0", "s2"));
    agentTemplate.addTransition(plain("d", "s2", "s0"));

    Agent* agent = agentTemplate.generateAgent(0);
    ASSERT_EQ(agent->localStates.size(), 3u);
    for (size_t i = 0; i < agent->localStates.size(); i++) {
        EXPECT_EQ(agent->localStates[i]->id, i);          // ids follow insertion order
        EXPECT_EQ(agent->localStates[i]->agent, agent);
    }
    EXPECT_EQ(agent->localStates[0]->name, "s0");
    EXPECT_EQ(agent->localTransitions.size(), 4u);        // the revisit of s2/s0 is deduplicated
    for (size_t i = 0; i < agent->localTransitions.size(); i++) {
        EXPECT_EQ(agent->localTransitions[i]->id, static_cast<int>(i));
    }
    EXPECT_EQ(outgoingNames(stateNamed(agent, "s0")), (std::multiset<string>{"[0]_a", "[0]_c"}));
}

TEST_F(AgentTemplateTest, PersistentVariablesSurviveAndLocalVariablesGoBackToInitialValue) {
    // s0 [p:=1, l:=1] -> s1 -> s2 : p is persistent and stays 1, l is not persistent and goes back to its initial 7.
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A").setInitState("s0");
    agentTemplate.addLocal(new set<string>{"l"});
    agentTemplate.addPersistent(new set<string>{"p"});
    agentTemplate.addInitial(assigns({{"p", new ExprConst(0)}, {"l", new ExprConst(7)}}));
    agentTemplate.addTransition(plain("set", "s0", "s1", nullptr, assigns({{"p", new ExprConst(1)}, {"l", new ExprConst(1)}})));
    agentTemplate.addTransition(plain("next", "s1", "s2"));

    Agent* agent = agentTemplate.generateAgent(0);
    EXPECT_EQ(stateNamed(agent, "s1")->environment, (map<string, int>{{"l", 1}, {"p", 1}}));
    EXPECT_EQ(stateNamed(agent, "s2")->environment, (map<string, int>{{"l", 7}, {"p", 1}}));
}

TEST_F(AgentTemplateTest, AssignmentsAreEvaluatedInTheSourceState) {
    // counter: c := c + 1 until c == 2, so states c=0, c=1, c=2.
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A").setInitState("s");
    agentTemplate.addPersistent(new set<string>{"c"});
    agentTemplate.addInitial(assigns({{"c", new ExprConst(0)}}));
    agentTemplate.addTransition(plain("inc", "s", "s", new ExprLt(new ExprIdent("c"), new ExprConst(2)),
                                      assigns({{"c", new ExprAdd(new ExprIdent("c"), new ExprConst(1))}})));

    Agent* agent = agentTemplate.generateAgent(0);
    ASSERT_EQ(agent->localStates.size(), 3u);
    EXPECT_EQ(agent->localStates[1]->environment.at("c"), 1);
    EXPECT_EQ(agent->localStates[2]->environment.at("c"), 2);
    // The condition c<2 is false in the last state, so it has no outgoing transition.
    EXPECT_TRUE(agent->localStates[2]->localTransitions.empty());
    // Every generated transition remembers its condition.
    for (auto transition : agent->localTransitions) {
        ASSERT_EQ(transition->conditions.size(), 1u);
        EXPECT_NE((*transition->conditions.begin())->expression, nullptr);
    }
}

TEST_F(AgentTemplateTest, SharedAndPrivateTransitionNames) {
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A").setInitState("s0");
    agentTemplate.addTransition(new TransitionTemplate(2, "sync", "sync_local", "s0", "s1", nullptr, nullptr, new ProbConst(1)));
    agentTemplate.addTransition(new TransitionTemplate(-1, "solo", "solo", "s1", "s0", nullptr, nullptr, new ProbConst(1)));

    Agent* agent = agentTemplate.generateAgent(3);
    ASSERT_EQ(agent->localTransitions.size(), 2u);
    LocalTransition* shared = agent->localTransitions[0];
    LocalTransition* solo = agent->localTransitions[1];

    EXPECT_TRUE(shared->isShared);
    EXPECT_EQ(shared->sharedCount, 2);
    EXPECT_EQ(shared->name, "sync");
    EXPECT_EQ(shared->localName, "sync_local");

    EXPECT_FALSE(solo->isShared);
    EXPECT_EQ(solo->name, "[3]_solo");   // private names are prefixed with the agent id
    EXPECT_EQ(solo->localName, "[3]_solo");
    EXPECT_EQ(solo->agent, agent);
    EXPECT_EQ(solo->from->name, "s1");
    EXPECT_EQ(solo->to->name, "s0");
}

TEST_F(AgentTemplateTest, EmptyTransitionBecomesSelfLoop) {
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A").setInitState("s0");
    agentTemplate.addTransition(new TransitionTemplate(999999, "wait", "wait", "s0", "s0", nullptr, nullptr, new ProbConst(1)));

    Agent* agent = agentTemplate.generateAgent(1);
    ASSERT_EQ(agent->localTransitions.size(), 1u);
    LocalTransition* transition = agent->localTransitions[0];
    EXPECT_TRUE(transition->isShared);
    EXPECT_EQ(transition->name, "[1]_wait");
    EXPECT_EQ(transition->from, agent->initState);
    EXPECT_EQ(transition->to, agent->initState);
    ASSERT_NE(transition->emptyTransition, nullptr);
    EXPECT_EQ(transition->emptyTransition->name, "wait");
    EXPECT_EQ(transition->emptyTransition->from, agent->initState);
}

TEST_F(AgentTemplateTest, RestProbabilityIsOneMinusPreviousBranches) {
    // heads: s0 -> 0.3 : s1
    //               &REST : s2      (parser encodes &REST as 1000 and leaves the header empty)
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A").setInitState("s0");
    agentTemplate.addTransition(new TransitionTemplate(-1, "heads", "heads", "s0", "s1", nullptr, nullptr, new ProbConst(0.3f)));
    agentTemplate.addTransition(new TransitionTemplate(0, "", "", "", "s2", new ExprConst(1), nullptr, new ProbConst(1000)));

    Agent* agent = agentTemplate.generateAgent(0);
    ASSERT_EQ(agent->localTransitions.size(), 2u);
    map<string, float> probabilityByTarget;
    for (auto transition : agent->localTransitions) {
        EXPECT_EQ(transition->name, "[0]_heads");       // the continuation inherits the header
        EXPECT_EQ(transition->from, agent->initState);
        probabilityByTarget[transition->to->name] = transition->probability;
    }
    EXPECT_FLOAT_EQ(probabilityByTarget["s1"], 0.3f);
    EXPECT_FLOAT_EQ(probabilityByTarget["s2"], 0.7f);
}

TEST_F(AgentTemplateTest, ConditionAndAssignmentVariables) {
    AgentTemplate agentTemplate;
    TransitionTemplate withBoth(-1, "t", "t", "s0", "s1",
                                new ExprAnd(new ExprEq(new ExprIdent("a"), new ExprConst(1)), new ExprIdent("b")),
                                assigns({{"x", new ExprIdent("a")}, {"y", new ExprConst(0)}}), new ProbConst(1));
    TransitionTemplate withNone(-1, "t", "t", "s0", "s1", nullptr, nullptr, new ProbConst(1));

    EXPECT_EQ(agentTemplate.getConditionVars(&withBoth), (set<string>{"a", "b"}));
    EXPECT_EQ(agentTemplate.getAssignmentVars(&withBoth), (set<string>{"x", "y"}));
    EXPECT_TRUE(agentTemplate.getConditionVars(&withNone).empty());
    EXPECT_TRUE(agentTemplate.getAssignmentVars(&withNone).empty());
}

TEST_F(AgentTemplateTest, RecommendReductionVariablesCountsAssignmentConditionPairs) {
    config.recommend_reduction_variables = true;
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A").setInitState("s0");
    // y is assigned under a condition on x (twice) and on z (once).
    agentTemplate.addTransition(plain("t1", "s0", "s1", new ExprEq(new ExprIdent("x"), new ExprConst(0)), assigns({{"y", new ExprConst(1)}})));
    agentTemplate.addTransition(plain("t2", "s1", "s0", new ExprAnd(new ExprIdent("x"), new ExprIdent("z")), assigns({{"y", new ExprConst(2)}})));

    Agent* agent = agentTemplate.generateAgent(0);
    EXPECT_EQ(agent->recommendedReductionVariableCounts, (map<string, map<string, int>>{{"y", {{"x", 2}, {"z", 1}}}}));
}

TEST_F(AgentTemplateTest, RecommendReductionVariablesOffByDefault) {
    AgentTemplate agentTemplate;
    agentTemplate.setIdent("A").setInitState("s0");
    agentTemplate.addTransition(plain("t1", "s0", "s1", new ExprIdent("x"), assigns({{"y", new ExprConst(1)}})));
    EXPECT_TRUE(agentTemplate.generateAgent(0)->recommendedReductionVariableCounts.empty());
}

namespace {
/// @brief Chain s0 -[x==0]-> s1 [y:=1] -> s2 [z:=1] -> s3 [w:=1]; only the first step has a condition.
void buildConeChain(AgentTemplate& agentTemplate) {
    agentTemplate.setIdent("A").setInitState("s0");
    agentTemplate.addTransition(plain("t0", "s0", "s1", new ExprEq(new ExprIdent("x"), new ExprConst(0)), assigns({{"y", new ExprConst(1)}})));
    agentTemplate.addTransition(plain("t1", "s1", "s2", nullptr, assigns({{"z", new ExprConst(1)}})));
    agentTemplate.addTransition(plain("t2", "s2", "s3", nullptr, assigns({{"w", new ExprConst(1)}})));
}
}

TEST_F(AgentTemplateTest, ConeOfInfluenceGrowsWithRadius) {
    using Counts = map<string, map<string, int>>;
    AgentTemplate agentTemplate;
    buildConeChain(agentTemplate);

    agentTemplate.checkConeOfInfluence(1);
    EXPECT_EQ(agentTemplate.generateAgent(0)->recommendedReductionVariableCounts, (Counts{{"y", {{"x", 1}}}}));

    agentTemplate.checkConeOfInfluence(2);
    EXPECT_EQ(agentTemplate.generateAgent(0)->recommendedReductionVariableCounts, (Counts{{"y", {{"x", 1}}}, {"z", {{"x", 1}}}}));

    agentTemplate.checkConeOfInfluence(10);  // larger than the chain: every later assignment is reached once
    EXPECT_EQ(agentTemplate.generateAgent(0)->recommendedReductionVariableCounts,
              (Counts{{"w", {{"x", 1}}}, {"y", {{"x", 1}}}, {"z", {{"x", 1}}}}));
}

TEST_F(AgentTemplateTest, ConeOfInfluenceWithNonPositiveRadiusClearsCounts) {
    config.recommend_reduction_variables = true;  // pre-fill the counts via addTransition
    AgentTemplate agentTemplate;
    buildConeChain(agentTemplate);
    ASSERT_FALSE(agentTemplate.generateAgent(0)->recommendedReductionVariableCounts.empty());

    agentTemplate.checkConeOfInfluence(0);
    EXPECT_TRUE(agentTemplate.generateAgent(0)->recommendedReductionVariableCounts.empty());
}
