#ifndef EMPTY_TRANSITION_TEST_H
#define EMPTY_TRANSITION_TEST_H

#include "gtest/gtest.h"
#include "config.h"

TEST(EmptyTransition, KeepsExecutableActionAlongsideEmptyDeadlockChoice) {
    struct ConfigRestore {
        Cfg saved = config;
        ~ConfigRestore() { config = saved; }
    } restoreConfig;

    config.add_epsilon_transitions = true;
    config.add_local_epsilon_transitions = false;
    config.merge_coalition = false;
    config.cone_of_influence = false;

    ModelParser parser;
    auto parsedModel = parser.parse("../tests/examples/emptyTransition/emptyTransitionDeadlock.txt");
    auto* localModels = &std::get<0>(parsedModel);
    auto* formula = &std::get<1>(parsedModel);

    Agent* coalitionAgent = nullptr;
    for (auto* agent : localModels->agents) {
        if (agent->name == "Coalition") {
            coalitionAgent = agent;
            break;
        }
    }
    ASSERT_NE(coalitionAgent, nullptr);

    LocalTransition* emptyTransition = nullptr;
    LocalTransition* normalTransition = nullptr;
    for (auto* transition : coalitionAgent->initState->localTransitions) {
        if (transition->emptyTransition != nullptr) {
            emptyTransition = transition;
        } else if (transition->name == "move") {
            normalTransition = transition;
        }
    }
    ASSERT_NE(emptyTransition, nullptr);
    EXPECT_EQ(emptyTransition->emptyTransition->name, "wait");
    EXPECT_EQ(emptyTransition->from, emptyTransition->to);
    ASSERT_NE(normalTransition, nullptr);
    EXPECT_EQ(normalTransition->from->name, "c0");
    EXPECT_EQ(normalTransition->to->name, "c1");

    GlobalModelGenerator generator;
    GlobalState* initialState = generator.initModel(localModels, formula);
    ASSERT_NE(initialState, nullptr);

    GlobalTransition* moveTransition = nullptr;
    GlobalTransition* epsilonTransition = nullptr;
    for (auto* transition : initialState->globalTransitions) {
        for (auto* localTransition : transition->localTransitions) {
            if (localTransition->name == "move") {
                moveTransition = transition;
            } else if (localTransition->name == "ɛ") {
                epsilonTransition = transition;
            }
        }
    }

    ASSERT_NE(moveTransition, nullptr);
    ASSERT_EQ(moveTransition->localTransitions.size(), 2u);
    bool coalitionCanMove = false;
    bool outsideCanMove = false;
    for (auto* localTransition : moveTransition->localTransitions) {
        if (localTransition->agent->name == "Coalition") {
            coalitionCanMove = localTransition->from->name == "c0" && localTransition->to->name == "c1";
        } else if (localTransition->agent->name == "Outside") {
            outsideCanMove = localTransition->from->name == "o0" && localTransition->to->name == "o1";
        }
    }
    EXPECT_TRUE(coalitionCanMove);
    EXPECT_TRUE(outsideCanMove);

    ASSERT_NE(epsilonTransition, nullptr);
    ASSERT_EQ(epsilonTransition->localTransitions.size(), 1u);
    auto* epsilonLocalTransition = *epsilonTransition->localTransitions.begin();
    EXPECT_EQ(epsilonLocalTransition->name, "ɛ");
    EXPECT_EQ(epsilonTransition->to, initialState);

    generator.expandState(initialState);
    ASSERT_NE(moveTransition->to, nullptr);
    EXPECT_NE(moveTransition->to, initialState);
}

#endif
