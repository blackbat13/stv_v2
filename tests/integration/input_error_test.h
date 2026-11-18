#include "gtest/gtest.h"

#ifndef CONFIG
#define CONFIG
#include "config.h"
#endif

TEST(InputErrorTest, IncorrectVariable)
{
    EXPECT_THROW({
        try
        {
            TestVerif verify("../tests/integration/examples/inputErrors/IncorrectVariable.txt", true);
        }
        catch( const std::runtime_error& e )
        {
            EXPECT_STREQ("Incorrect variable name 'A' in the formula", e.what());
            throw;
        }
    }, std::runtime_error);
}

TEST(InputErrorTest, IncorrectAgent)
{
    EXPECT_THROW({
        try
        {
            TestVerif verify("../tests/integration/examples/inputErrors/IncorrectAgent.txt", true);
        }
        catch( const std::runtime_error& e )
        {
            EXPECT_STREQ("Incorrect agent name 'C' in the formula coalition", e.what());
            throw;
        }
    }, std::runtime_error);
}

TEST(InputErrorTest, IncorrectAgentAndVariable)
{
    EXPECT_THROW({
        try
        {
            TestVerif verify("../tests/integration/examples/inputErrors/IncorrectAgentAndVariable.txt", true);
        }
        catch( const std::runtime_error& e )
        {
            EXPECT_STREQ("Incorrect agent name 'C' in the formula coalition", e.what());
            throw;
        }
    }, std::runtime_error);
}
#include "StrategyParser.hpp"

namespace {
const string INPUT_ERRORS = "../tests/integration/examples/inputErrors/";

/// @brief Verifies the model and returns the std::runtime_error message it throws, or "" if none.
string verificationError(const string& model) {
    try {
        TestVerif verify(INPUT_ERRORS + model, true);
    } catch (const std::runtime_error& e) {
        return e.what();
    }
    return "";
}

/// @brief Verifies the model and returns what it printed to stderr.
string verificationWarnings(const string& model) {
    testing::internal::CaptureStderr();
    TestVerif verify(INPUT_ERRORS + model, true);
    return testing::internal::GetCapturedStderr();
}

/// @brief Parses the strategy file and returns the std::runtime_error message it throws, or "" if none.
string strategyError(const string& path) {
    try {
        StrategyParser().parse(path);
    } catch (const std::runtime_error& e) {
        return e.what();
    }
    return "";
}

/// @brief Parses the model with a replaced formula and returns the std::runtime_error message, or "" if none.
string overwriteFormulaError(const string& formula) {
    try {
        ModelParser().parseAndOverwriteFormula("../tests/integration/examples/svote/1Voter1Coercer.txt", formula);
    } catch (const std::runtime_error& e) {
        return e.what();
    }
    return "";
}
}

// ---- model file: reading and syntax ------------------------------------------

TEST(InputErrorTest, MissingModelFile)
{
    EXPECT_EQ(verificationError("DoesNotExist.txt"), "Failed to open model file '" + INPUT_ERRORS + "DoesNotExist.txt'");
}

TEST(InputErrorTest, SyntaxError)
{
    EXPECT_EQ(verificationError("SyntaxError.txt"), "Syntax error at line 4 near '->'");
}

TEST(InputErrorTest, SyntaxErrorInCompleteModel)
{
    // svote/1Voter1Coercer.txt with the ':' after the action name missing on line 23
    EXPECT_EQ(verificationError("SyntaxErrorInCompleteModel.txt"), "Syntax error at line 23 near 'q1'");
}

TEST(InputErrorTest, UnexpectedEndOfFile)
{
    EXPECT_EQ(verificationError("UnexpectedEndOfFile.txt"), "Syntax error at line 4 at the end of the file");
}

TEST(InputErrorTest, UnexpectedCharacter)
{
    EXPECT_EQ(verificationError("UnexpectedCharacter.txt"), "Unexpected character '=' at line 4");
}

// ---- model file: structure ---------------------------------------------------

TEST(InputErrorTest, NoAgents)
{
    EXPECT_EQ(verificationError("NoAgents.txt"), "The model does not define any agents");
}

TEST(InputErrorTest, NoFormula)
{
    EXPECT_EQ(verificationError("NoFormula.txt"), "The model has no FORMULA");
}

TEST(InputErrorTest, NoInitialState)
{
    EXPECT_EQ(verificationError("NoInitialState.txt"), "The agent 'Voter' has no initial state (missing 'init <state>')");
}

TEST(InputErrorTest, InitialStateNotUsed)
{
    EXPECT_EQ(verificationError("InitialStateNotUsed.txt"), "No transition of agent 'Voter' starts in its initial state 'q0'");
}

TEST(InputErrorTest, DuplicateAgent)
{
    EXPECT_EQ(verificationError("DuplicateAgent.txt"), "Agent 'Voter' is defined more than once");
}

TEST(InputErrorTest, SharedWithOneAgent)
{
    EXPECT_EQ(verificationError("SharedWithOneAgent.txt"),
              "Shared action 'give' of agent 'Voter' is declared as shared[1], but a shared action needs at least 2 agents");
}

TEST(InputErrorTest, InconsistentSharedCount)
{
    // the agents are checked in no fixed order, so either one can be named first
    const string message = verificationError("InconsistentSharedCount.txt");
    EXPECT_TRUE(message == "Shared action 'give' is declared as shared[2] in agent 'Voter' but as shared[3] in agent 'Coercer'" ||
                message == "Shared action 'give' is declared as shared[3] in agent 'Coercer' but as shared[2] in agent 'Voter'") << message;
}

TEST(InputErrorTest, SharedActionPerformedTwice)
{
    // a bare action name and name[name] both mean the agent performs the sync
    EXPECT_EQ(verificationError("SharedActionPerformedTwice.txt"),
              "Shared action 'give' is performed by more than one agent ('Coercer', 'Voter'): only one agent may perform it, written as give or give[give]; the agents that only wait for it must write give[localName] with a different local name");
}

TEST(InputErrorTest, AmbiguousFormulaVariable)
{
    EXPECT_EQ(verificationError("AmbiguousFormulaVariable.txt"),
              "Variable 'win' in the formula is declared by more than one agent ('Controller', 'Train'), so it is ambiguous: give each agent's variable a unique name");
}

TEST(InputErrorTest, ProbabilityOutOfRange)
{
    EXPECT_EQ(verificationError("ProbabilityOutOfRange.txt"),
              "Probability -0.3 of action 'flip' in state 'q0' of agent 'Coin' is not between 0 and 1");
}

TEST(InputErrorTest, ProbabilitiesDoNotSumToOne)
{
    EXPECT_EQ(verificationError("ProbabilitiesDoNotSumToOne.txt"),
              "Probabilities of action 'flip' in state 'q0' of agent 'Coin' add up to 0.9 instead of 1");
}

// ---- formula -----------------------------------------------------------------

TEST(InputErrorTest, UnknownKnowledgeAgent)
{
    EXPECT_EQ(verificationError("UnknownKnowledgeAgent.txt"), "Incorrect agent name 'Ghost' in the &K_ operator");
}

TEST(InputErrorTest, UnknownHartleyAgent)
{
    EXPECT_EQ(verificationError("UnknownHartleyAgent.txt"), "Incorrect agent name 'Ghost' in the &H_ operator");
}

TEST(InputErrorTest, VariableWithoutValueInTheState)
{
    // 'receipt' is declared, but has no value in the initial state
    EXPECT_EQ(verificationError("UnsetVariable.txt"), "Variable 'receipt' of the formula has no value in the checked state");
}

TEST(InputErrorTest, InvalidOverwriteFormula)
{
    EXPECT_EQ(overwriteFormulaError("FORMULA: <<Voter1>>[](Voter1_vote==)"),
              "Invalid formula given with --OVERWRITE_FORMULA: Syntax error at line 1 near ')'");
    EXPECT_EQ(overwriteFormulaError("<<Voter1>>[](Voter1_vote==1)"),
              "Invalid formula given with --OVERWRITE_FORMULA: Syntax error at line 1 near '<'");
    EXPECT_EQ(overwriteFormulaError(""),
              "The formula given with --OVERWRITE_FORMULA is empty (it must start with 'FORMULA:')");
    EXPECT_EQ(overwriteFormulaError("FORMULA: <<Voter2>>[](Voter1_vote==1)"),
              "Incorrect agent name 'Voter2' in the formula coalition");
    EXPECT_EQ(overwriteFormulaError("FORMULA: <<Voter1>>[](Voter1_vot==1)"),
              "Incorrect variable name 'Voter1_vot' in the formula");
}

// ---- warnings (the model is still verified) -----------------------------------

TEST(InputErrorTest, ReadsUndeclaredVariableWarns)
{
    EXPECT_NE(verificationWarnings("ReadsUndeclaredVariable.txt").find(
                  "Warning: agent 'Voter' reads variable 'Coercer_demand', which it never declares or assigns, so it always evaluates to -1"),
              string::npos);
}

TEST(InputErrorTest, SharedActionCannotFireWarns)
{
    EXPECT_NE(verificationWarnings("SharedActionCannotFire.txt").find(
                  "Warning: shared action 'give' needs 2 agents, but only 1 agent(s) define it, so it can never fire"),
              string::npos);
}

// ---- strategy file -----------------------------------------------------------

TEST(InputErrorTest, MissingStrategyFile)
{
    EXPECT_EQ(strategyError(INPUT_ERRORS + "DoesNotExist.txt"), "Failed to open strategy file '" + INPUT_ERRORS + "DoesNotExist.txt'");
}

TEST(InputErrorTest, StrategySyntaxError)
{
    EXPECT_EQ(strategyError(INPUT_ERRORS + "StrategySyntaxError.txt"), "Syntax error in strategy file at line 3 near '->'");
}

TEST(InputErrorTest, StrategyUnexpectedCharacter)
{
    EXPECT_EQ(strategyError(INPUT_ERRORS + "StrategyUnexpectedCharacter.txt"), "Unexpected character ';' in strategy file at line 2");
}
