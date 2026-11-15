/**
 * @file strategy_parser_test.h
 * @brief Unit tests for StrategyParser (strategy file -> StrategyCollection).
 */

#include "helpers.h"

class StrategyParserTest : public UnitTest {};

TEST_F(StrategyParserTest, ParsesOneActionPerLine) {
    string path = writeTempFile("# comment\nqa0 -> vote\nqa1, qa2 -> give\n");
    StrategyParser parser;
    StrategyCollection* strategy = parser.parse(path);
    std::remove(path.c_str());

    auto selected = strategy->getStrategy();
    ASSERT_EQ(selected.count("qa0;"), 1u);
    EXPECT_EQ(selected.at("qa0;").actionName, "vote");
    EXPECT_EQ(selected.at("qa0;").hash, "qa0;");
    ASSERT_EQ(selected.count("qa1;qa2;"), 1u);
    EXPECT_EQ(selected.at("qa1;qa2;").actionName, "give");
    EXPECT_EQ(*selected.at("qa1;qa2;").states, (vector<string>{"qa1", "qa2"}));
}

TEST_F(StrategyParserTest, SecondParseDoesNotContainActionsOfTheFirst) {
    StrategyParser parser;
    string first = writeTempFile("qb0 -> first\n");
    parser.parse(first);
    std::remove(first.c_str());

    string second = writeTempFile("qc0 -> second\n");
    StrategyCollection* strategy = parser.parse(second);
    std::remove(second.c_str());

    auto selected = strategy->getStrategy();
    EXPECT_EQ(selected.size(), 1u);
    EXPECT_EQ(selected.count("qb0;"), 0u);
}

TEST_F(StrategyParserTest, MissingFileThrows) {
    StrategyParser parser;
    EXPECT_THROW(parser.parse("/nonexistent/stv/strategy.txt"), std::runtime_error);
}
