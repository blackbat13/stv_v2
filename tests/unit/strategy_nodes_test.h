/**
 * @file strategy_nodes_test.h
 * @brief Unit tests for strategyReader/strategyNodes.* (ActionTemplate, StrategyCollectionTemplate).
 */

#include "helpers.h"
#include "strategyReader/strategyNodes.hpp"

TEST(ActionTemplate, HashJoinsStatesWithSemicolons) {
    ActionTemplate action(new vector<string>{"q0", "q1"}, "vote");
    EXPECT_EQ(action.hash, "q0;q1;");
    EXPECT_EQ(action.actionName, "vote");
    EXPECT_EQ(*action.states, (vector<string>{"q0", "q1"}));
}

TEST(ActionTemplate, EmptyStateListGivesEmptyHash) {
    ActionTemplate action(new vector<string>{}, "idle");
    EXPECT_EQ(action.hash, "");
}

TEST(StrategyCollectionTemplate, AddActionIndexesByHash) {
    StrategyCollectionTemplate strategy;
    EXPECT_TRUE(strategy.selectedStrategy.empty());
    strategy.addAction(new vector<string>{"q0"}, "a");
    strategy.addAction(new vector<string>{"q1", "q2"}, "b");
    ASSERT_EQ(strategy.selectedStrategy.size(), 2u);
    EXPECT_EQ(strategy.selectedStrategy.at("q0;").actionName, "a");
    EXPECT_EQ(strategy.selectedStrategy.at("q1;q2;").actionName, "b");
}

TEST(StrategyCollectionTemplate, FirstActionWinsOnDuplicateHash) {
    StrategyCollectionTemplate strategy;
    strategy.addAction(new vector<string>{"q0"}, "first");
    strategy.addAction(new vector<string>{"q0"}, "second");
    ASSERT_EQ(strategy.selectedStrategy.size(), 1u);
    EXPECT_EQ(strategy.selectedStrategy.at("q0;").actionName, "first");
}
