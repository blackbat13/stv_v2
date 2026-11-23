/**
 * @file types_test.h
 * @brief Unit tests for the small value types in Types.hpp and TypesDependency.hpp.
 */

#include "helpers.h"

TEST(ProbabilityEntry, DefaultsToSumModeWithZeroProbabilities) {
    ProbabilityEntry entry;
    EXPECT_EQ(entry.getVerificationMode(), ProbabilityCalculationType::SUM_PROBABILITY);
    EXPECT_FLOAT_EQ(entry.getSumProbability().probabilityTrue, 0.0f);
    EXPECT_FLOAT_EQ(entry.getSumProbability().probabilityFalse, 0.0f);
    EXPECT_FLOAT_EQ(entry.getMinProbability().probabilityTrue, 1.0f);
    EXPECT_FLOAT_EQ(entry.getMinProbability().probabilityFalse, 0.0f);
}

TEST(ProbabilityEntry, ConstructorAndChangeSetTheMode) {
    ProbabilityEntry entry(ProbabilityCalculationType::MIN_PROBABILITY);
    EXPECT_EQ(entry.getVerificationMode(), ProbabilityCalculationType::MIN_PROBABILITY);
    entry.changeVerificationMode(ProbabilityCalculationType::NONE_PROBABILITY);
    EXPECT_EQ(entry.getVerificationMode(), ProbabilityCalculationType::NONE_PROBABILITY);
}

TEST(ProbabilityEntry, SumModeAccumulates) {
    ProbabilityEntry entry;
    entry.changeSumProbabilityTrue(0.25f);
    entry.changeSumProbabilityTrue(0.25f);
    entry.changeSumProbabilityFalse(0.125f);
    EXPECT_FLOAT_EQ(entry.getSumProbability().probabilityTrue, 0.5f);
    EXPECT_FLOAT_EQ(entry.getSumProbability().probabilityFalse, 0.125f);
    EXPECT_FLOAT_EQ(entry.returnProbability().probabilityTrue, 0.5f);

    entry.setSumProbability({0.1f, 0.9f});
    EXPECT_FLOAT_EQ(entry.getSumProbability().probabilityTrue, 0.1f);
    EXPECT_FLOAT_EQ(entry.getSumProbability().probabilityFalse, 0.9f);
}

TEST(ProbabilityEntry, MinModeKeepsLowestTrueAndHighestFalse) {
    ProbabilityEntry entry(ProbabilityCalculationType::MIN_PROBABILITY);
    entry.changeMinProbabilityTrue(0.4f);
    entry.changeMinProbabilityTrue(0.6f);   // ignored: not lower
    entry.changeMinProbabilityFalse(0.3f);
    entry.changeMinProbabilityFalse(0.1f);  // ignored: not higher
    EXPECT_FLOAT_EQ(entry.getMinProbability().probabilityTrue, 0.4f);
    EXPECT_FLOAT_EQ(entry.getMinProbability().probabilityFalse, 0.3f);
    // returnProbability follows the active mode
    EXPECT_FLOAT_EQ(entry.returnProbability().probabilityTrue, 0.4f);
    entry.changeVerificationMode(ProbabilityCalculationType::SUM_PROBABILITY);
    EXPECT_FLOAT_EQ(entry.returnProbability().probabilityTrue, 0.0f);

    entry.setMinProbability({0.7f, 0.2f});
    EXPECT_FLOAT_EQ(entry.getMinProbability().probabilityTrue, 0.7f);
    EXPECT_FLOAT_EQ(entry.getMinProbability().probabilityFalse, 0.2f);
}

namespace {
Action makeAction(const std::string& hash, const std::string& name) {
    Action action;
    action.states = nullptr;
    action.hash = hash;
    action.actionName = name;
    return action;
}
}

TEST(StrategyCollection, ConstructorStoresOneAction) {
    StrategyCollection empty;
    EXPECT_TRUE(empty.getStrategy().empty());
    StrategyCollection single("0;", makeAction("0;", "go"));
    ASSERT_EQ(single.getStrategy().size(), 1u);
    EXPECT_EQ(single.getStrategy().at("0;").actionName, "go");
}

TEST(StrategyCollection, AddActionOverwritesSameHash) {
    StrategyCollection strategy;
    strategy.addAction(makeAction("0;", "a"));
    strategy.addAction(makeAction("1;", "b"));
    strategy.addAction(makeAction("0;", "c"));
    auto selected = strategy.getStrategy();
    ASSERT_EQ(selected.size(), 2u);
    EXPECT_EQ(selected.at("0;").actionName, "c");
    EXPECT_EQ(selected.at("1;").actionName, "b");
}

TEST(StrategyCollection, AddStrategyMergesAndClearEmpties) {
    StrategyCollection strategy;
    strategy.addAction(makeAction("0;", "a"));
    StrategyCollection other;
    other.addAction(makeAction("0;", "override"));
    other.addAction(makeAction("2;", "new"));
    strategy.addStrategy(other);
    auto selected = strategy.getStrategy();
    ASSERT_EQ(selected.size(), 2u);
    EXPECT_EQ(selected.at("0;").actionName, "override");
    EXPECT_EQ(selected.at("2;").actionName, "new");

    strategy.clear();
    EXPECT_TRUE(strategy.getStrategy().empty());
}

TEST(StrategyBitsComparator, OrdersByNumericValue) {
    StrategyBitsComparator less;
    EXPECT_TRUE(less(bitset<STRATEGY_BITS>(1), bitset<STRATEGY_BITS>(2)));
    EXPECT_FALSE(less(bitset<STRATEGY_BITS>(2), bitset<STRATEGY_BITS>(1)));
    EXPECT_FALSE(less(bitset<STRATEGY_BITS>(3), bitset<STRATEGY_BITS>(3)));
}

TEST(TypeDefaults, StructsStartInANeutralState) {
    Condition condition;
    EXPECT_EQ(condition.expression, nullptr);
    EXPECT_EQ(condition.var, nullptr);
    EXPECT_EQ(condition.conditionOperator, ConditionOperator::Equals);
    EXPECT_EQ(condition.comparedValue, 0);

    Formula formula;
    EXPECT_FLOAT_EQ(formula.probability, 1.0f);
    EXPECT_EQ(formula.probabilitySign, ProbabilitySign::NONE);

    Result result;
    EXPECT_FALSE(result.verificationResult);
    EXPECT_FLOAT_EQ(result.probabilityResult.probabilityTrue, 0.0f);

    StrategyEntry strategyEntry;
    EXPECT_EQ(strategyEntry.type, StrategyEntryType::NOT_MODIFIED);
    EXPECT_EQ(strategyEntry.globalValues.to_ulong(), 0u);

    StateVerificationInfo info;
    EXPECT_EQ(info.globalState, nullptr);
    EXPECT_EQ(info.depth, 0);
    EXPECT_EQ(info.verifResult, VerifResult::NOT_VERIFIED);
    EXPECT_FALSE(info.processed);
    EXPECT_FALSE(info.hasValidControlledTransition);
    EXPECT_TRUE(info.hasValidUncontrolledTransition);

    EpistemicClass epistemicClass;
    EXPECT_EQ(epistemicClass.fixedCoalitionTransition, nullptr);
    EXPECT_EQ(epistemicClass.fixedCoalitionLocalTransition, nullptr);
}
