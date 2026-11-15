/**
 * @file global_model_generator_test.h
 * @brief Unit tests for GlobalModelGenerator.
 *
 * The models are small enough to enumerate by hand: two independent togglers
 * have exactly 2 x 2 = 4 global states, a shared step fires only when both
 * agents are ready, and so on.
 */

#include "helpers.h"

namespace {
/// Two independent agents, each flipping between two states (4 global states).
const char* TOGGLERS = R"(
Agent A:
LOCAL: []
init a0
goA: a0 -> a1
backA: a1 -> a0

Agent B:
LOCAL: []
init b0
goB: b0 -> b1
backB: b1 -> b0

FORMULA: <<A>>[](1==1)
)";

/// B must prepare before the shared step; A waits for it.
const char* SYNC = R"(
Agent A:
LOCAL: []
init a0
shared[2] sync[sync_A]: a0 -> a1

Agent B:
LOCAL: []
init b0
prepB: b0 -> b1
shared[2] sync[sync_B]: b1 -> b2

FORMULA: <<A>>[](1==1)
)";

/// A chooses left or right once, then loops forever.
const char* CHOICE = R"(
Agent A:
LOCAL: [x]
PERSISTENT: [x]
INITIAL: [x:=0]
init s0
left: s0 -> l [x:=1]
right: s0 -> r [x:=2]
stayL: l -> l
stayR: r -> r

FORMULA: <<A>><>&P[>=0.5](x==1)
)";

/// A single coin flip with probability 0.5 of heads.
const char* COIN = R"(
Agent C:
LOCAL: [h]
PERSISTENT: [h]
INITIAL: [h:=0]
init s0
flip: s0 -> 0.5 : heads [h:=1]
         &REST : tails [h:=2]
stayH: heads -> heads
stayT: tails -> tails

FORMULA: <<C>><>&P[>=0.5](h==1)
)";

/// @brief Local state of @p agent in @p state.
LocalState* localOf(GlobalModelGenerator& generator, GlobalState* state, Agent* agent) {
    return state->localStatesProjection[generator.agentIndex[agent]];
}
}

class GlobalModelGeneratorTest : public UnitTest {};

TEST_F(GlobalModelGeneratorTest, InitModelCreatesOnlyTheInitialState) {
    GeneratedModel generated(TOGGLERS);
    GlobalModel* model = generated.model();
    ASSERT_NE(model, nullptr);
    ASSERT_EQ(model->globalStates.size(), 1u);
    GlobalState* init = model->initState;
    EXPECT_EQ(model->globalStates[0], init);
    EXPECT_EQ(init->hash, "0;0;");
    EXPECT_FALSE(init->isExpanded);
    EXPECT_FLOAT_EQ(init->probability, 1.0f);
    // One global transition per enabled local action, not yet expanded.
    ASSERT_EQ(init->globalTransitions.size(), 2u);
    for (auto transition : init->globalTransitions) {
        EXPECT_EQ(transition->from, init);
        EXPECT_EQ(transition->to, nullptr);
        EXPECT_EQ(transition->localTransitions.size(), 1u);
    }
}

TEST_F(GlobalModelGeneratorTest, InitModelIndexesAgentsAndEpistemicClasses) {
    GeneratedModel generated(TOGGLERS);
    GlobalModel* model = generated.model();
    Agent* a = generated.agent("A");
    Agent* b = generated.agent("B");

    ASSERT_EQ(generated.generator.agentIndex.size(), 2u);
    for (size_t i = 0; i < model->agents.size(); i++) {
        EXPECT_EQ(generated.generator.agentIndex[model->agents[i]], i);
    }
    // Coalition epistemic classes exist only for the coalition agent A.
    ASSERT_EQ(model->epistemicClasses.size(), 1u);
    ASSERT_EQ(model->epistemicClasses.count(a), 1u);
    EpistemicClass* initClass = model->initState->epistemicClasses.at(a);
    EXPECT_EQ(initClass->hash, "0");
    EXPECT_EQ(initClass->globalStates.at("0;0;"), model->initState);
    // Knowledge classes exist for every agent.
    EXPECT_EQ(model->initState->epistemicClassesAllAgents.size(), 2u);
    EXPECT_EQ(model->initState->epistemicClassesAllAgents.at(b)->count(model->initState), 1u);
    EXPECT_FALSE(config.probability);
}

TEST_F(GlobalModelGeneratorTest, InitModelEnablesProbabilityModeForProbabilisticFormulas) {
    GeneratedModel generated(COIN);
    EXPECT_TRUE(config.probability);
    EXPECT_FLOAT_EQ(generated.model()->initState->probability, 1.0f);
}

TEST_F(GlobalModelGeneratorTest, ExpandStateCreatesSuccessorsOnce) {
    GeneratedModel generated(TOGGLERS);
    GlobalState* init = generated.model()->initState;
    generated.generator.expandState(init);

    EXPECT_TRUE(init->isExpanded);
    EXPECT_EQ(generated.model()->globalStates.size(), 3u);
    for (auto transition : init->globalTransitions) {
        ASSERT_NE(transition->to, nullptr);
        EXPECT_NE(transition->to, init);
        EXPECT_EQ(transition->to->preimage.count(init), 1u);
    }
    generated.generator.expandState(init);  // already expanded: nothing changes
    EXPECT_EQ(generated.model()->globalStates.size(), 3u);
}

TEST_F(GlobalModelGeneratorTest, ExpandStateAndReturnReturnsOnlyNewStates) {
    GeneratedModel generated(TOGGLERS);
    GlobalState* init = generated.model()->initState;
    vector<GlobalState*> created = generated.generator.expandStateAndReturn(init);
    ASSERT_EQ(created.size(), 2u);
    EXPECT_EQ(set<string>({created[0]->hash, created[1]->hash}), (set<string>{"1;0;", "0;1;"}));

    EXPECT_TRUE(generated.generator.expandStateAndReturn(init).empty());
    EXPECT_EQ(generated.generator.expandStateAndReturn(init, true), (vector<GlobalState*>{init}));

    // From (a1, b0) one successor is new (a1, b1) and one already exists (a0, b0).
    GlobalState* next = created[0];
    EXPECT_EQ(generated.generator.expandStateAndReturn(next).size(), 1u);
}

TEST_F(GlobalModelGeneratorTest, ExpandAllStatesBuildsTheWholeProduct) {
    GeneratedModel generated(TOGGLERS);
    generated.generator.expandAllStates();
    GlobalModel* model = generated.model();
    ASSERT_EQ(model->globalStates.size(), 4u);
    set<string> hashes;
    for (auto state : model->globalStates) {
        hashes.insert(state->hash);
        EXPECT_TRUE(state->isExpanded);
        EXPECT_EQ(state->globalTransitions.size(), 2u);
    }
    EXPECT_EQ(hashes, (set<string>{"0;0;", "0;1;", "1;0;", "1;1;"}));

    generated.generator.expandAllStates();  // idempotent
    EXPECT_EQ(model->globalStates.size(), 4u);
}

TEST_F(GlobalModelGeneratorTest, SharedTransitionFiresOnlyWhenAllParticipantsAreReady) {
    GeneratedModel generated(SYNC);
    generated.generator.expandAllStates();
    GlobalModel* model = generated.model();
    ASSERT_EQ(model->globalStates.size(), 3u);
    // Agent order in the vector is not fixed, so look states up via agentIndex.
    Agent* a = generated.agent("A");
    Agent* b = generated.agent("B");
    for (auto state : model->globalStates) {
        string where = localOf(generated.generator, state, a)->name + "," + localOf(generated.generator, state, b)->name;
        if (where == "a0,b0") {
            ASSERT_EQ(state->globalTransitions.size(), 1u);   // only B can prepare
            EXPECT_EQ((*state->globalTransitions.begin())->localTransitions.size(), 1u);
        } else if (where == "a0,b1") {
            ASSERT_EQ(state->globalTransitions.size(), 1u);   // the joint sync step
            EXPECT_EQ((*state->globalTransitions.begin())->localTransitions.size(), 2u);
        } else {
            EXPECT_EQ(where, "a1,b2");
            EXPECT_TRUE(state->globalTransitions.empty());
        }
    }
}

namespace {
/// After a joint start, A (coalition) and B move independently. In (a1, b1) B's
/// step touches only B, so the reduction may explore it alone:
/// full = 5 states, reduced = 4 states (it skips (a2, b1)).
const char* INDEPENDENT_AFTER_START = R"(
Agent A:
LOCAL: []
init a0
shared[2] start[start_A]: a0 -> a1
done: a1 -> a2

Agent B:
LOCAL: [y]
PERSISTENT: [y]
INITIAL: [y:=0]
init b0
shared[2] start[start_B]: b0 -> b1
work: b1 -> b2 [y:=1]

FORMULA: <<A>>[](y<2)
)";

/// @brief Runs reduced generation (it prints debug output) and returns the verdict.
bool reduceAndVerify(GeneratedModel& generated) {
    testing::internal::CaptureStdout();
    generated.generator.expandAndReduceAllStates();
    bool result = Verification(&generated.generator).verify();
    testing::internal::GetCapturedStdout();
    return result;
}
}

TEST_F(GlobalModelGeneratorTest, ReducedGenerationSkipsInterleavingsOfIndependentSteps) {
    GeneratedModel full(INDEPENDENT_AFTER_START);
    full.generator.expandAllStates();
    EXPECT_EQ(full.model()->globalStates.size(), 5u);
    bool fullResult = Verification(&full.generator).verify();

    GeneratedModel reduced(INDEPENDENT_AFTER_START);
    bool reducedResult = reduceAndVerify(reduced);

    EXPECT_TRUE(fullResult);
    EXPECT_EQ(reducedResult, fullResult);
    EXPECT_EQ(reduced.model()->globalStates.size(), 4u);
}

TEST_F(GlobalModelGeneratorTest, ReductionNeverSkipsStepsThatChangeImportantVariables) {
    config.reduce_args = "y";   // B's step changes y, so it must not be reordered
    GeneratedModel reduced(INDEPENDENT_AFTER_START);
    reduceAndVerify(reduced);
    EXPECT_EQ(reduced.model()->globalStates.size(), 5u);
}

TEST_F(GlobalModelGeneratorTest, ReductionWorksWhenTheInitialStateIsReducible) {
    GeneratedModel reduced(R"(
Agent A:
LOCAL: []
init a0
go: a0 -> a1

Agent B:
LOCAL: []
init b0
work: b0 -> b1

FORMULA: <<A>>[](1==1)
)");
    EXPECT_TRUE(reduceAndVerify(reduced));
    EXPECT_LE(reduced.model()->globalStates.size(), 4u);
}

TEST_F(GlobalModelGeneratorTest, FormulaAccessors) {
    GeneratedModel generated(TOGGLERS);
    EXPECT_EQ(generated.generator.getFormula(), &generated.parsed.formula);
    EXPECT_EQ(generated.generator.getFormulaSize(), 1);
    EXPECT_TRUE(generated.generator.getFormulaCorectness());
    generated.generator.markFormulaAsIncorrect();
    EXPECT_FALSE(generated.generator.getFormulaCorectness());
}

TEST_F(GlobalModelGeneratorTest, GetAgentInstanceByName) {
    GeneratedModel generated(TOGGLERS);
    EXPECT_EQ(generated.generator.getAgentInstanceByName("A"), generated.agent("A"));
    EXPECT_EQ(generated.generator.getAgentInstanceByName("B"), generated.agent("B"));
    EXPECT_EQ(generated.generator.getAgentInstanceByName("Nobody"), nullptr);
}

TEST_F(GlobalModelGeneratorTest, KnowledgeClassesGroupStatesWithTheSameLocalState) {
    GeneratedModel generated(TOGGLERS);
    generated.generator.expandAllStates();
    Agent* a = generated.agent("A");
    auto& classesOfA = generated.model()->epistemicClassesKnowledge.at(a);
    ASSERT_EQ(classesOfA.size(), 2u);                     // A is either in a0 or in a1
    for (const auto& entry : classesOfA) {
        EXPECT_EQ(entry.second.size(), 2u);                // ...whatever B does
        for (auto state : entry.second) {
            EXPECT_EQ(to_string(localOf(generated.generator, state, a)->id), entry.first);
        }
    }
    // Asking again for a member returns the same set without growing it.
    GlobalState* init = generated.model()->initState;
    auto knowledgeClass = generated.generator.findOrCreateEpistemicClassForKnowledge(&init->localStatesProjection, init, a);
    EXPECT_EQ(knowledgeClass, &classesOfA.at("0"));
    EXPECT_EQ(knowledgeClass->size(), 2u);
}

TEST_F(GlobalModelGeneratorTest, CoalitionIdentifiers) {
    GeneratedModel generated(TOGGLERS);
    generated.generator.expandAllStates();
    Agent* a = generated.agent("A");
    Agent* b = generated.agent("B");
    GlobalState* aMoved = nullptr;  // state (a1, b0)
    for (auto state : generated.model()->globalStates) {
        if (localOf(generated.generator, state, a)->id == 1 && localOf(generated.generator, state, b)->id == 0) {
            aMoved = state;
        }
    }
    ASSERT_NE(aMoved, nullptr);

    // Coalition {A}: only A's local state id.
    EXPECT_EQ(generated.generator.getCoalitionIdentifier(&aMoved->localStatesProjection), "1;");
    // Coalition {A, B}: ids ordered by agent index.
    generated.parsed.formula.coalition.insert(b);
    string expected = generated.generator.agentIndex[a] == 0 ? "1;0;" : "0;1;";
    EXPECT_EQ(generated.generator.getCoalitionIdentifier(&aMoved->localStatesProjection), expected);

    LocalState* a1 = stateNamed(a, "a1");
    EXPECT_EQ(generated.generator.getCoalitionLocalStateIdentifier(a, a1), to_string(generated.generator.agentIndex[a]) + ":1");
    Agent stranger(9, "Stranger");
    EXPECT_THROW(generated.generator.getCoalitionLocalStateIdentifier(&stranger, a1), std::logic_error);
}

TEST_F(GlobalModelGeneratorTest, CoalitionActionSignatureUsesLocalNamesOfCoalitionAgentsOnly) {
    GeneratedModel generated(SYNC);
    generated.generator.expandAllStates();
    Agent* a = generated.agent("A");
    string indexA = to_string(generated.generator.agentIndex[a]);
    GlobalTransition* sync = nullptr;
    GlobalTransition* prep = nullptr;
    for (auto state : generated.model()->globalStates) {
        for (auto transition : state->globalTransitions) {
            (transition->localTransitions.size() == 2 ? sync : prep) = transition;
        }
    }
    ASSERT_NE(sync, nullptr);
    ASSERT_NE(prep, nullptr);
    EXPECT_EQ(generated.generator.getCoalitionActionSignature(sync), indexA + ":sync_A");
    EXPECT_EQ(generated.generator.getCoalitionActionSignature(prep), "");   // B only, not in the coalition

    // With both agents in the coalition the parts are joined with the separator, ordered by index.
    Agent* b = generated.agent("B");
    generated.parsed.formula.coalition.insert(b);
    string indexB = to_string(generated.generator.agentIndex[b]);
    string expected = generated.generator.agentIndex[a] < generated.generator.agentIndex[b]
        ? indexA + ":sync_A|" + indexB + ":sync_B" : indexB + ":sync_B|" + indexA + ":sync_A";
    EXPECT_EQ(generated.generator.getCoalitionActionSignature(sync, '|'), expected);
}

class DeadlockTest : public UnitTest {
protected:
    ModelArena arena;
    GlobalModelGenerator generator;
    Agent* a = arena.agent(0, "A");
    Agent* b = arena.agent(1, "B");
    LocalState* a0 = arena.state(a, "a0");
    LocalState* b0 = arena.state(b, "b0");
};

TEST_F(DeadlockTest, AnyPrivateActionMeansNoDeadlock) {
    arena.transition(a0, a0, "solo");
    arena.transition(b0, b0, "x", "x", true, 2);
    vector<LocalState*> states{a0, b0};
    EXPECT_FALSE(generator.hasDeadlockCombination(&states));
}

TEST_F(DeadlockTest, NoTransitionsAtAllIsADeadlock) {
    vector<LocalState*> states{a0, b0};
    EXPECT_TRUE(generator.hasDeadlockCombination(&states));
}

TEST_F(DeadlockTest, SharedActionsWithoutEnoughPartnersDeadlock) {
    arena.transition(a0, a0, "x", "x", true, 2);
    arena.transition(b0, b0, "y", "y", true, 2);
    vector<LocalState*> states{a0, b0};
    EXPECT_TRUE(generator.hasDeadlockCombination(&states));
}

TEST_F(DeadlockTest, MatchingSharedActionsDoNotDeadlock) {
    arena.transition(a0, a0, "x", "x", true, 2);
    arena.transition(b0, b0, "x", "x", true, 2);
    vector<LocalState*> states{a0, b0};
    EXPECT_FALSE(generator.hasDeadlockCombination(&states));
}

TEST_F(DeadlockTest, OneBlockedCombinationIsEnough) {
    // A can pick x or y; B only x. Picking y leaves both actions without partners.
    arena.transition(a0, a0, "x", "x", true, 2);
    arena.transition(a0, a0, "y", "y", true, 2);
    arena.transition(b0, b0, "x", "x", true, 2);
    vector<LocalState*> states{a0, b0};
    EXPECT_TRUE(generator.hasDeadlockCombination(&states));
}

TEST_F(GlobalModelGeneratorTest, EpsilonTransitionIsAddedToDeadlockedStates) {
    config.add_epsilon_transitions = true;
    GeneratedModel generated(R"(
Agent A:
LOCAL: []
init a0
shared[2] x: a0 -> a1

Agent B:
LOCAL: []
init b0
shared[2] y: b0 -> b1

FORMULA: <<A>>[](1==1)
)");
    GlobalState* init = generated.model()->initState;
    ASSERT_EQ(init->globalTransitions.size(), 1u);
    GlobalTransition* epsilon = *init->globalTransitions.begin();
    EXPECT_EQ(epsilon->from, init);
    EXPECT_EQ(epsilon->to, init);
    ASSERT_EQ(epsilon->localTransitions.size(), 1u);
    EXPECT_EQ((*epsilon->localTransitions.begin())->name, "ɛ");
    EXPECT_EQ((*epsilon->localTransitions.begin())->agent, generated.agent("A"));
}

TEST_F(GlobalModelGeneratorTest, WithoutEpsilonOptionDeadlockedStatesHaveNoTransitions) {
    GeneratedModel generated(R"(
Agent A:
LOCAL: []
init a0
shared[2] x: a0 -> a1

Agent B:
LOCAL: []
init b0
shared[2] y: b0 -> b1

FORMULA: <<A>>[](1==1)
)");
    EXPECT_TRUE(generated.model()->initState->globalTransitions.empty());
}

TEST_F(GlobalModelGeneratorTest, StrategyLookupByCoalitionState) {
    GeneratedModel generated(TOGGLERS);
    generated.generator.expandAllStates();
    Action action;
    action.states = nullptr;
    action.hash = "0;";
    action.actionName = "goA";
    StrategyCollection strategy("0;", action);
    generated.generator.initStrategy(&strategy);

    GlobalState* init = generated.model()->initState;
    EXPECT_EQ(generated.generator.getActionNameFromStateInStrategy(init), "goA;");
    // A state without an entry yields ";" (map::operator[] inserts an empty action).
    // Verification::verifyStrategy relies on this value meaning "no action".
    GlobalState* aMoved = nullptr;
    for (auto state : generated.model()->globalStates) {
        if (localOf(generated.generator, state, generated.agent("A"))->id == 1) {
            aMoved = state;
        }
    }
    EXPECT_EQ(generated.generator.getActionNameFromStateInStrategy(aMoved), ";");
}

TEST_F(GlobalModelGeneratorTest, GetNextPathEnumeratesEveryCoalitionStrategyOnce) {
    GeneratedModel generated(CHOICE);
    generated.generator.expandAllStates(true);
    generated.generator.createProbabilityStrategy();
    Agent* a = generated.agent("A");
    string decisionAtStart = generated.generator.getCoalitionLocalStateIdentifier(a, stateNamed(a, "s0"));

    set<string> choicesAtStart;
    int strategies = 0;
    while (const map<string, string>* strategy = generated.generator.getNextPath()) {
        ASSERT_LT(++strategies, 10) << "strategy enumeration does not terminate";
        ASSERT_EQ(strategy->count(decisionAtStart), 1u);
        choicesAtStart.insert(strategy->at(decisionAtStart));
    }
    // s0 offers two actions; every other local state offers one: 2 strategies in total.
    EXPECT_EQ(strategies, 2);
    EXPECT_EQ(choicesAtStart, (set<string>{"[0]_left", "[0]_right"}));
    EXPECT_EQ(generated.generator.getNextPath(), nullptr);  // stays exhausted

    generated.generator.createProbabilityStrategy();        // restart
    EXPECT_NE(generated.generator.getNextPath(), nullptr);
}

TEST_F(GlobalModelGeneratorTest, GenerateNextMdpBuildsOneMdpPerStrategy) {
    GeneratedModel generated(COIN);
    generated.generator.expandAllStates(true);
    generated.generator.createProbabilityStrategy();

    MDP mdp = generated.generator.generateNextMDP();
    EXPECT_GE(mdp.state_count(), 2u);                         // at least the start and the goal
    EXPECT_EQ(generated.generator.generateNextMDP().state_count(), 0u);  // only one strategy exists
}

// ---- protected helpers (through GeneratorProbe) -------------------------------

TEST_F(GlobalModelGeneratorTest, StateAndClassHashes) {
    ModelArena arena;
    GeneratedModel generated(TOGGLERS);
    Agent* a = generated.agent("A");
    Agent* b = generated.agent("B");
    vector<LocalState*> states{stateNamed(a, "a1"), stateNamed(b, "b0")};
    EXPECT_EQ(generated.generator.computeGlobalStateHash(&states), "1;0;");
    EXPECT_EQ(generated.generator.computeEpistemicClassHash(&states, a), "1");
    EXPECT_EQ(generated.generator.computeEpistemicClassHash(&states, b), "0");
    Agent stranger(9, "Stranger");
    EXPECT_EQ(generated.generator.computeEpistemicClassHash(&states, &stranger), "");
}

TEST_F(GlobalModelGeneratorTest, FindOrCreateEpistemicClassIsIdempotent) {
    GeneratedModel generated(TOGGLERS);
    Agent* a = generated.agent("A");
    Agent* b = generated.agent("B");
    vector<LocalState*> first{stateNamed(a, "a1"), stateNamed(b, "b0")};
    vector<LocalState*> second{stateNamed(a, "a1"), stateNamed(b, "b1")};
    EpistemicClass* created = generated.generator.findOrCreateEpistemicClass(&first, a);
    EXPECT_EQ(created->hash, "1");
    EXPECT_EQ(created->fixedCoalitionTransition, nullptr);
    EXPECT_EQ(generated.generator.findOrCreateEpistemicClass(&second, a), created);  // B's state does not matter to A
    EXPECT_NE(generated.generator.findOrCreateEpistemicClass(&first, b), created);
}

TEST_F(GlobalModelGeneratorTest, GenerateStateFromLocalStatesReusesExistingStates) {
    GeneratedModel generated(TOGGLERS);
    Agent* a = generated.agent("A");
    Agent* b = generated.agent("B");
    vector<LocalState*> states(2);
    states[generated.generator.agentIndex[a]] = stateNamed(a, "a1");
    states[generated.generator.agentIndex[b]] = stateNamed(b, "b1");
    GlobalState* created = generated.generator.generateStateFromLocalStates(&states, nullptr, nullptr);
    EXPECT_EQ(generated.model()->globalStates.size(), 2u);
    EXPECT_EQ(generated.generator.generateStateFromLocalStates(&states, nullptr, nullptr), created);
    EXPECT_EQ(generated.model()->globalStates.size(), 2u);
}

TEST_F(GlobalModelGeneratorTest, CoalitionTransitionCompatibilityWithStrategy) {
    GeneratedModel generated(CHOICE);
    generated.generator.expandAllStates();
    Agent* a = generated.agent("A");
    GlobalState* init = generated.model()->initState;
    string decision = generated.generator.getCoalitionLocalStateIdentifier(a, stateNamed(a, "s0"));
    for (auto transition : init->globalTransitions) {
        string action = (*transition->localTransitions.begin())->localName;
        EXPECT_TRUE(generated.generator.isCoalitionTransitionCompatibleWithStrategy(transition, {{decision, action}}));
        EXPECT_FALSE(generated.generator.isCoalitionTransitionCompatibleWithStrategy(transition, {{decision, "other"}}));
        EXPECT_FALSE(generated.generator.isCoalitionTransitionCompatibleWithStrategy(transition, {}));
    }
}

TEST_F(GlobalModelGeneratorTest, CheckLocalStatesEvaluatesTheFirstFormula) {
    GeneratedModel generated(CHOICE);   // formula: x==1
    generated.generator.expandAllStates();
    Agent* a = generated.agent("A");
    for (auto state : generated.model()->globalStates) {
        bool expected = localOf(generated.generator, state, a)->name == "l";
        EXPECT_EQ(generated.generator.checkLocalStates(&state->localStatesProjection, state), expected) << state->hash;
    }
}
