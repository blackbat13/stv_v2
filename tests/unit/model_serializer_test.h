/**
 * @file model_serializer_test.h
 * @brief Unit tests for ModelSerializer (model -> input-format text).
 */

#include "helpers.h"
#include "ModelSerializer.hpp"

namespace {
/// @brief Serializes a parsed model and returns what ModelSerializer printed.
string serialize(ParsedModel& parsed) {
    ModelSerializer serializer;
    testing::internal::CaptureStdout();
    serializer.print(&parsed.models, &parsed.formula);
    return testing::internal::GetCapturedStdout();
}

/// @brief Counts states and transitions per agent name.
map<string, pair<size_t, size_t>> shapeOf(ParsedModel& parsed) {
    map<string, pair<size_t, size_t>> shape;
    for (auto agent : parsed.models.agents) {
        shape[agent->name] = {agent->localStates.size(), agent->localTransitions.size()};
    }
    return shape;
}

/// go assigns a persistent and a local variable; sync is shared. The sync step
/// leads to a second "s0" state (p=3), so "go" appears twice from states named s0.
const char* SIMPLE_MODEL = R"(
Agent A:
LOCAL: [l]
PERSISTENT: [p]
INITIAL: [p:=2]
init s0
go: s0 -> s1 [p:=3, l:=1]
shared[2] sync[sync_A]: s1 -> s0

FORMULA: <<A>>[](p==2)
)";

const char* COIN_MODEL = R"(
Agent C:
LOCAL: [r]
PERSISTENT: [r]
INITIAL: [r:=0]
init s0
flip: s0 -> 0.25 : h [r:=1]
         &REST : t [r:=2]

FORMULA: <<C>><>&P[>=0.5](r==1)
)";
}

class ModelSerializerTest : public UnitTest {};

TEST_F(ModelSerializerTest, PrintsAgentBlockAndFormula) {
    ParsedModel parsed = parseModelText(SIMPLE_MODEL);
    EXPECT_EQ(serialize(parsed),
              "Agent A:\n"
              "LOCAL: [l]\n"
              "PERSISTENT: [p]\n"
              "INITIAL: [p:=2]\n"
              "init s0\n"
              "go: s0 -> s1 [l:=1, p:=3]\n"           // "[0]_" prefix stripped, duplicate branch dropped
              "shared[2] sync[sync_A]: s1 -> s0\n"     // p does not change, so no assignment
              "\n"
              "FORMULA: <<A>>[](p == 2)\n");
}

TEST_F(ModelSerializerTest, PrintsConditionsAndOmitsZeroInitialValues) {
    ParsedModel parsed = parseModelText(R"(
Agent A:
LOCAL: [x, y]
PERSISTENT: [x, y]
INITIAL: [x:=0, y:=1]
init s0
go: s0 [x==0] -> s1 [x:=1]

FORMULA: <<A>><>(x==1)
)");
    string output = serialize(parsed);
    EXPECT_NE(output.find("INITIAL: [y:=1]\n"), string::npos);
    EXPECT_NE(output.find("go: s0 [x == 0] -> s1 [x:=1]\n"), string::npos);
    EXPECT_NE(output.find("FORMULA: <<A>><>(x == 1)\n"), string::npos);
}

TEST_F(ModelSerializerTest, ProbabilisticBranchesEndWithRest) {
    ParsedModel parsed = parseModelText(COIN_MODEL);
    string output = serialize(parsed);
    // Branch order follows the generated transitions; either order is a valid model.
    bool headsFirst = output.find("flip: s0 -> 0.25   : h [r:=1]\n            &REST : t [r:=2]\n") != string::npos;
    bool tailsFirst = output.find("flip: s0 -> 0.75   : t [r:=2]\n            &REST : h [r:=1]\n") != string::npos;
    EXPECT_TRUE(headsFirst || tailsFirst) << output;
    EXPECT_NE(output.find("FORMULA: <<C>><>&P[>=0.5](r == 1)\n"), string::npos);
}

TEST_F(ModelSerializerTest, FormulaRendersEveryProbabilitySign) {
    ParsedModel parsed = parseModelText(COIN_MODEL);
    const vector<pair<ProbabilitySign, string>> signs = {
        {ProbabilitySign::EQ, "=="}, {ProbabilitySign::NE, "!="}, {ProbabilitySign::GT, ">"},
        {ProbabilitySign::GE, ">="}, {ProbabilitySign::LT, "<"}, {ProbabilitySign::LE, "<="}};
    for (const auto& sign : signs) {
        parsed.formula.probabilitySign = sign.first;
        parsed.formula.probability = 0.25f;
        EXPECT_NE(serialize(parsed).find("<<C>><>&P[" + sign.second + "0.25](r == 1)"), string::npos) << sign.second;
    }
}

TEST_F(ModelSerializerTest, CtlFormulaOmitsCoalitionAndJoinsConditions) {
    ParsedModel parsed = parseModelText(R"(
Agent A:
LOCAL: [x]
PERSISTENT: [x]
INITIAL: [x:=0]
init s0
go: s0 -> s0

FORMULA: [](x==0), (x<1)
)");
    ASSERT_TRUE(parsed.formula.isCTL);
    EXPECT_NE(serialize(parsed).find("FORMULA: [](x == 0 && x < 1)\n"), string::npos);
}

TEST_F(ModelSerializerTest, SerializedModelParsesBackToTheSameShape) {
    for (const char* text : {SIMPLE_MODEL, COIN_MODEL}) {
        ParsedModel original = parseModelText(text);
        ParsedModel reparsed = parseModelText(serialize(original));
        EXPECT_EQ(shapeOf(reparsed), shapeOf(original));
        EXPECT_EQ(reparsed.formula.isF, original.formula.isF);
        EXPECT_EQ(reparsed.formula.probabilitySign, original.formula.probabilitySign);
    }
}
