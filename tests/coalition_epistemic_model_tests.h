#ifndef COALITION_EPISTEMIC_MODEL_TESTS_H
#define COALITION_EPISTEMIC_MODEL_TESTS_H

#include "gtest/gtest.h"
#include "config.h"

static Agent* coalitionTestFindAgent(GlobalModel* model, const string& name) {
    for (auto* agent : model->agents) {
        if (agent->name == name) {
            return agent;
        }
    }
    return nullptr;
}

static GlobalState* coalitionTestFindState(GlobalModel* model, const map<string, string>& localStateNames) {
    for (auto* state : model->globalStates) {
        bool matches = state->localStatesProjection.size() == localStateNames.size();
        for (auto* localState : state->localStatesProjection) {
            auto expected = localStateNames.find(localState->agent->name);
            if (expected == localStateNames.end() || expected->second != localState->name) {
                matches = false;
                break;
            }
        }
        if (matches) {
            return state;
        }
    }
    return nullptr;
}

TEST(CoalitionEpistemicClasses, coalitionEpistemicClasses1) {
    TestVerif verify("../tests/examples/coalitionEpistemicClasses/coalitionEpistemicClasses1.txt", 1);
    EXPECT_TRUE(verify.result);

    auto* model = verify.generator->getCurrentGlobalModel();
    EXPECT_NE(coalitionTestFindState(model, {{"CoalitionA", "a2"}, {"CoalitionB", "b2"}, {"Outside", "c1"}}), nullptr);
    ASSERT_EQ(model->agents.size(), 3u);
    auto* initial = model->initState;
    auto* first = coalitionTestFindAgent(model, "CoalitionA");
    auto* second = coalitionTestFindAgent(model, "CoalitionB");
    auto* outside = coalitionTestFindAgent(model, "Outside");
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(outside, nullptr);

    ASSERT_EQ(initial->epistemicClasses.size(), 2u);
    EXPECT_NE(initial->epistemicClasses.count(first), 0u);
    EXPECT_NE(initial->epistemicClasses.count(second), 0u);
    EXPECT_EQ(initial->epistemicClasses.count(outside), 0u);
    EXPECT_NE(initial->epistemicClasses[first], initial->epistemicClasses[second]);
}

TEST(CoalitionEpistemicClasses, coalitionEpistemicClasses2) {
    TestVerif verify("../tests/examples/coalitionEpistemicClasses/coalitionEpistemicClasses2.txt", 1);
    EXPECT_TRUE(verify.result);

    auto* model = verify.generator->getCurrentGlobalModel();
    EXPECT_NE(coalitionTestFindState(model, {{"CoalitionA", "a2"}, {"CoalitionB", "b3"}, {"Outside", "c1"}}), nullptr);
    auto* initial = model->initState;
    auto* secondMoved = coalitionTestFindState(model, {{"CoalitionA", "a0"}, {"CoalitionB", "b1"}, {"Outside", "c0"}});
    auto* first = coalitionTestFindAgent(model, "CoalitionA");
    auto* second = coalitionTestFindAgent(model, "CoalitionB");
    ASSERT_NE(secondMoved, nullptr);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(secondMoved->epistemicClasses[first], initial->epistemicClasses[first]);
    EXPECT_NE(secondMoved->epistemicClasses[second], initial->epistemicClasses[second]);
    EXPECT_EQ(initial->epistemicClasses[first]->globalStates.count(secondMoved->hash), 1u);
}

TEST(CoalitionEpistemicClasses, coalitionEpistemicClasses3) {
    TestVerif verify("../tests/examples/coalitionEpistemicClasses/coalitionEpistemicClasses3.txt", 1);
    EXPECT_TRUE(verify.result);

    auto* model = verify.generator->getCurrentGlobalModel();
    EXPECT_NE(coalitionTestFindState(model, {{"CoalitionA", "a3"}, {"CoalitionB", "b2"}, {"Outside", "c1"}}), nullptr);
    auto* initial = model->initState;
    auto* firstMoved = coalitionTestFindState(model, {{"CoalitionA", "a1"}, {"CoalitionB", "b0"}, {"Outside", "c0"}});
    auto* first = coalitionTestFindAgent(model, "CoalitionA");
    auto* second = coalitionTestFindAgent(model, "CoalitionB");
    ASSERT_NE(firstMoved, nullptr);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(firstMoved->epistemicClasses[first], initial->epistemicClasses[first]);
    EXPECT_EQ(firstMoved->epistemicClasses[second], initial->epistemicClasses[second]);
    EXPECT_EQ(initial->epistemicClasses[second]->globalStates.count(firstMoved->hash), 1u);
}

TEST(CoalitionEpistemicClasses, coalitionEpistemicClasses4) {
    TestVerif verify("../tests/examples/coalitionEpistemicClasses/coalitionEpistemicClasses4.txt", 1);
    EXPECT_TRUE(verify.result);

    auto* model = verify.generator->getCurrentGlobalModel();
    EXPECT_NE(coalitionTestFindState(model, {{"CoalitionA", "a2"}, {"CoalitionB", "b1"}, {"Outside", "c1"}}), nullptr);
    auto* initial = model->initState;
    auto* outsideMoved = coalitionTestFindState(model, {{"CoalitionA", "a0"}, {"CoalitionB", "b0"}, {"Outside", "c1"}});
    auto* first = coalitionTestFindAgent(model, "CoalitionA");
    auto* second = coalitionTestFindAgent(model, "CoalitionB");
    auto* outside = coalitionTestFindAgent(model, "Outside");
    ASSERT_NE(outsideMoved, nullptr);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(outside, nullptr);

    EXPECT_EQ(outsideMoved->epistemicClasses[first], initial->epistemicClasses[first]);
    EXPECT_EQ(outsideMoved->epistemicClasses[second], initial->epistemicClasses[second]);
    EXPECT_EQ(outsideMoved->epistemicClasses.count(outside), 0u);
}

TEST(CoalitionEpistemicClasses, coalitionEpistemicClasses5) {
    TestVerif verify("../tests/examples/coalitionEpistemicClasses/coalitionEpistemicClasses5.txt", 1);
    EXPECT_TRUE(verify.result);

    auto* model = verify.generator->getCurrentGlobalModel();
    EXPECT_NE(coalitionTestFindState(model, {{"CoalitionA", "a1"}, {"CoalitionB", "b2"}, {"Outside", "c1"}}), nullptr);
    auto* merged = coalitionTestFindState(model, {{"CoalitionA", "a1"}, {"CoalitionB", "b1"}, {"Outside", "c0"}});
    auto* first = coalitionTestFindAgent(model, "CoalitionA");
    auto* second = coalitionTestFindAgent(model, "CoalitionB");
    ASSERT_NE(merged, nullptr);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(merged->preimage.count(coalitionTestFindState(model, {{"CoalitionA", "a1"}, {"CoalitionB", "b0"}, {"Outside", "c0"}})), 0u);
    EXPECT_NE(merged->preimage.count(coalitionTestFindState(model, {{"CoalitionA", "a0"}, {"CoalitionB", "b1"}, {"Outside", "c0"}})), 0u);
    EXPECT_EQ(merged->epistemicClasses[first]->globalStates.count(merged->hash), 1u);
    EXPECT_EQ(merged->epistemicClasses[second]->globalStates.count(merged->hash), 1u);
}

#endif // COALITION_EPISTEMIC_MODEL_TESTS_H