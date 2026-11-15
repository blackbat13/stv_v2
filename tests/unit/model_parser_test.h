/**
 * @file model_parser_test.h
 * @brief Unit tests for ModelParser (model text -> LocalModels + Formula).
 *
 * Note: the parser stores agents in a pointer-ordered set, so agent order (and
 * the ids derived from it) is not guaranteed. Tests look agents up by name.
 */

#include "helpers.h"

namespace {
const char* TWO_AGENT_MODEL = R"(
Agent Voter:
LOCAL: [vote, tmp]
PERSISTENT: [vote]
INITIAL: [vote:=0]
init q0
choose: q0 -> q1 [vote:=1, tmp:=1]
shared[2] give[give_Voter]: q1 -> q2

Agent Coercer:
LOCAL: []
init c0
shared[2] give[give_Coercer]: c0 -> c1

FORMULA: <<Voter>><>(vote==1)
)";

/// @brief A one-agent model with the given formula line.
string withFormula(const string& formula) {
    return R"(
Agent A:
LOCAL: [x]
PERSISTENT: [x]
INITIAL: [x:=0]
init s0
go: s0 -> s1 [x:=1]

)" + formula + "\n";
}

/// @brief Names of the agents in @p parsed.
set<string> agentNames(ParsedModel& parsed) {
    set<string> names;
    for (auto agent : parsed.models.agents) {
        names.insert(agent->name);
    }
    return names;
}

/// @brief Names of the agents in @p formula's coalition.
set<string> coalitionNames(const Formula& formula) {
    set<string> names;
    for (auto agent : formula.coalition) {
        names.insert(agent->name);
    }
    return names;
}
}

class ModelParserTest : public UnitTest {};

TEST_F(ModelParserTest, ParsesAgentsVariablesAndStates) {
    ParsedModel parsed = parseModelText(TWO_AGENT_MODEL);
    ASSERT_EQ(agentNames(parsed), (set<string>{"Coercer", "Voter"}));

    Agent* voter = parsed.agent("Voter");
    map<string, bool> persistence;
    for (auto var : voter->vars) {
        persistence[var->name] = var->persistent;
    }
    // A variable listed in both LOCAL and PERSISTENT becomes two Var entries.
    EXPECT_EQ(voter->vars.size(), 3u);
    EXPECT_TRUE(persistence.at("vote"));
    EXPECT_FALSE(persistence.at("tmp"));
    EXPECT_EQ(voter->initState->name, "q0");
    EXPECT_EQ(voter->initState->environment, (map<string, int>{{"vote", 0}}));
    EXPECT_EQ(voter->localStates.size(), 3u);
    EXPECT_EQ(voter->localTransitions.size(), 2u);

    Agent* coercer = parsed.agent("Coercer");
    EXPECT_TRUE(coercer->vars.empty());
    EXPECT_EQ(coercer->localStates.size(), 2u);

    // ids are 0..n-1 in some order
    set<int> ids{voter->id, coercer->id};
    EXPECT_EQ(ids, (set<int>{0, 1}));
}

TEST_F(ModelParserTest, ParsesSharedTransitionNames) {
    ParsedModel parsed = parseModelText(TWO_AGENT_MODEL);
    LocalTransition* give = nullptr;
    for (auto transition : parsed.agent("Voter")->localTransitions) {
        if (transition->isShared) {
            give = transition;
        }
    }
    ASSERT_NE(give, nullptr);
    EXPECT_EQ(give->name, "give");
    EXPECT_EQ(give->localName, "give_Voter");
    EXPECT_EQ(give->sharedCount, 2);
}

TEST_F(ModelParserTest, ParsesStrategicFormula) {
    ParsedModel parsed = parseModelText(TWO_AGENT_MODEL);
    EXPECT_EQ(coalitionNames(parsed.formula), (set<string>{"Voter"}));
    EXPECT_TRUE(parsed.formula.isF);
    EXPECT_FALSE(parsed.formula.isCTL);
    EXPECT_EQ(parsed.formula.probabilitySign, ProbabilitySign::NONE);
    ASSERT_NE(parsed.formula.p, nullptr);
    ASSERT_EQ(parsed.formula.p->size(), 1u);
    EXPECT_EQ((*parsed.formula.p)[0]->toString(false), "vote == 1");
}

TEST_F(ModelParserTest, GloballyFormulaIsNotF) {
    ParsedModel parsed = parseModelText(withFormula("FORMULA: <<A>>[](x==0)"));
    EXPECT_FALSE(parsed.formula.isF);
}

TEST_F(ModelParserTest, FormulaWithoutCoalitionIsCtlOverAllAgents) {
    ParsedModel parsed = parseModelText(withFormula("FORMULA: [](x==0)"));
    EXPECT_TRUE(parsed.formula.isCTL);
    EXPECT_EQ(coalitionNames(parsed.formula), (set<string>{"A"}));

    ParsedModel emptyCoalition = parseModelText(withFormula("FORMULA: <<>><>(x==1)"));
    EXPECT_TRUE(emptyCoalition.formula.isCTL);
    EXPECT_TRUE(emptyCoalition.formula.isF);
}

TEST_F(ModelParserTest, ParsesEveryProbabilitySign) {
    const vector<pair<string, ProbabilitySign>> signs = {
        {"==", ProbabilitySign::EQ}, {"!=", ProbabilitySign::NE}, {">", ProbabilitySign::GT},
        {">=", ProbabilitySign::GE}, {"<", ProbabilitySign::LT}, {"<=", ProbabilitySign::LE}};
    for (const auto& sign : signs) {
        ParsedModel parsed = parseModelText(withFormula("FORMULA: <<A>><>&P[" + sign.first + "0.25](x==1)"));
        EXPECT_EQ(parsed.formula.probabilitySign, sign.second) << sign.first;
        EXPECT_FLOAT_EQ(parsed.formula.probability, 0.25f) << sign.first;
    }
}

TEST_F(ModelParserTest, MissingFileThrows) {
    ModelParser parser;
    EXPECT_THROW(parser.parse("/nonexistent/stv/model.txt"), std::runtime_error);
}

TEST_F(ModelParserTest, UnknownCoalitionAgentThrows) {
    EXPECT_THROW(parseModelText(withFormula("FORMULA: <<Nobody>>[](x==0)")), std::runtime_error);
}

TEST_F(ModelParserTest, ParseAndOverwriteFormulaKeepsAgentsAndReplacesFormula) {
    string path = writeTempFile(TWO_AGENT_MODEL);
    ModelParser parser;
    LocalModels models;
    Formula formula;
    std::tie(models, formula) = parser.parseAndOverwriteFormula(path, "FORMULA: <<Coercer>>[](vote==0)");
    std::remove(path.c_str());

    EXPECT_EQ(models.agents.size(), 2u);
    ASSERT_EQ(formula.coalition.size(), 1u);
    EXPECT_EQ((*formula.coalition.begin())->name, "Coercer");
    EXPECT_FALSE(formula.isF);
    EXPECT_EQ((*formula.p)[0]->toString(false), "vote == 0");
}

TEST_F(ModelParserTest, ParseAndOverwriteFormulaKeepsWholeCoalition) {
    string path = writeTempFile(TWO_AGENT_MODEL);
    ModelParser parser;
    LocalModels models;
    Formula formula;
    std::tie(models, formula) = parser.parseAndOverwriteFormula(path, "FORMULA: <<Voter, Coercer>>[](vote==0)");
    std::remove(path.c_str());
    EXPECT_EQ(formula.coalition.size(), 2u);
}

TEST_F(ModelParserTest, ConeOfInfluenceFillsRecommendedVariables) {
    config.cone_of_influence = true;
    config.cone_radius = 2;
    ParsedModel parsed = parseModelText(R"(
Agent A:
LOCAL: [x, y]
PERSISTENT: [x, y]
INITIAL: [x:=0, y:=0]
init s0
check: s0 [x==0] -> s1
set: s1 -> s2 [y:=1]

FORMULA: <<A>>[](y==0)
)");
    EXPECT_EQ(parsed.agent("A")->recommendedReductionVariableCounts, (map<string, map<string, int>>{{"y", {{"x", 1}}}}));
}

namespace {
/// @brief Splits a merged state name "(a, b)" into its parts.
set<string> tupleParts(const string& name) {
    set<string> parts;
    string inner = name.substr(1, name.size() - 2);
    size_t start = 0;
    while (true) {
        size_t comma = inner.find(", ", start);
        parts.insert(inner.substr(start, comma - start));
        if (comma == string::npos) {
            break;
        }
        start = comma + 2;
    }
    return parts;
}

Agent* mergedAgentOf(ParsedModel& parsed) {
    for (auto agent : parsed.models.agents) {
        if (agent->name == "A_B" || agent->name == "B_A") {
            return agent;
        }
    }
    return nullptr;
}
}

TEST_F(ModelParserTest, MergeCoalitionBuildsTheProductOfCoalitionAgents) {
    config.merge_coalition = true;
    ParsedModel parsed = parseModelText(R"(
Agent A:
LOCAL: [a]
PERSISTENT: [a]
INITIAL: [a:=0]
init a0
goA: a0 -> a1 [a:=1]

Agent B:
LOCAL: [b]
PERSISTENT: [b]
INITIAL: [b:=0]
init b0
goB: b0 -> b1 [b:=1]

Agent C:
LOCAL: []
init c0
idle: c0 -> c0

FORMULA: <<A, B>><>(a==1 && b==1)
)");
    ASSERT_EQ(parsed.models.agents.size(), 2u);
    Agent* merged = mergedAgentOf(parsed);
    ASSERT_NE(merged, nullptr);
    EXPECT_EQ(merged->id, 1);                       // merged agent is appended last
    EXPECT_EQ(parsed.agent("C")->id, 0);
    EXPECT_EQ(coalitionNames(parsed.formula), (set<string>{merged->name}));

    // 2 x 2 product of local states, each with the union of both environments.
    ASSERT_EQ(merged->localStates.size(), 4u);
    EXPECT_EQ(tupleParts(merged->initState->name), (set<string>{"a0", "b0"}));
    EXPECT_EQ(merged->initState->environment, (map<string, int>{{"a", 0}, {"b", 0}}));
    set<set<string>> tuples;
    for (auto state : merged->localStates) {
        EXPECT_EQ(state->agent, merged);
        tuples.insert(tupleParts(state->name));
    }
    EXPECT_EQ(tuples, (set<set<string>>{{"a0", "b0"}, {"a1", "b0"}, {"a0", "b1"}, {"a1", "b1"}}));

    // Each original transition is copied to every product state where it is enabled: 2 + 1 + 1.
    EXPECT_EQ(merged->localTransitions.size(), 4u);
    EXPECT_EQ(merged->initState->localTransitions.size(), 2u);
    for (auto var : merged->vars) {
        EXPECT_EQ(var->agent, merged);
    }
}

TEST_F(ModelParserTest, MergeCoalitionCollapsesSharedTransitionsBetweenMembers) {
    config.merge_coalition = true;
    ParsedModel parsed = parseModelText(R"(
Agent A:
LOCAL: []
init a0
shared[3] sync[sync_A]: a0 -> a1

Agent B:
LOCAL: []
init b0
shared[3] sync[sync_B]: b0 -> b1

Agent C:
LOCAL: []
init c0
shared[3] sync[sync]: c0 -> c1

FORMULA: <<A, B>><>(1==1)
)");
    Agent* merged = mergedAgentOf(parsed);
    ASSERT_NE(merged, nullptr);

    // From (a0, b0) both members take part in sync together: one transition, needing one partner less.
    ASSERT_EQ(merged->initState->localTransitions.size(), 1u);
    LocalTransition* sync = *merged->initState->localTransitions.begin();
    EXPECT_EQ(sync->name, "sync");
    EXPECT_EQ(sync->sharedCount, 2);
    EXPECT_EQ(tupleParts(sync->to->name), (set<string>{"a1", "b1"}));
    // The remaining partner's requirement is lowered to match.
    EXPECT_EQ(parsed.agent("C")->localTransitions[0]->sharedCount, 2);
}

TEST_F(ModelParserTest, MergeCoalitionWithSingleAgentChangesNothing) {
    config.merge_coalition = true;
    ParsedModel parsed = parseModelText(TWO_AGENT_MODEL);
    EXPECT_EQ(agentNames(parsed), (set<string>{"Coercer", "Voter"}));
}
