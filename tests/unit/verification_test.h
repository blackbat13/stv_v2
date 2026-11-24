/**
 * @file verification_test.h
 * @brief Unit tests for Verification, HistoryEntry, HistoryDbg and verStatusToStr.
 */

#include "helpers.h"

namespace {
/// @brief A single agent that chooses once between a good (x=1) and a bad (x=2) branch.
string goodBad(const string& formula) {
    return R"(
Agent A:
LOCAL: [x]
PERSISTENT: [x]
INITIAL: [x:=0]
init s0
good: s0 -> s1 [x:=1]
bad: s0 -> s2 [x:=2]
stay1: s1 -> s1
stay2: s2 -> s2

FORMULA: )" + formula + "\n";
}

/// @brief A has no enabled action; the opponent B chooses between y=1 and y=2.
string opponent(const string& formula, bool aCanWait = false) {
    return string(R"(
Agent A:
LOCAL: []
init a0
)") + (aCanWait ? "wait: a0 -> a0\n" : "wait: a0 [1==0] -> a0\n") + R"(
Agent B:
LOCAL: [y]
PERSISTENT: [y]
INITIAL: [y:=0]
init b0
good: b0 -> b1 [y:=1]
bad: b0 -> b2 [y:=2]
stay1: b1 -> b1
stay2: b2 -> b2

FORMULA: )" + formula + "\n";
}

const char* COIN_FLIP = R"(
Agent C:
LOCAL: [h]
PERSISTENT: [h]
INITIAL: [h:=0]
init s0
flip: s0 -> 0.5 : heads [h:=1]
         &REST : tails [h:=2]
stayH: heads -> heads
stayT: tails -> tails

FORMULA: <<C>><>&P[%SIGN%0.5](h==1)
)";

/// @brief The init-state transition whose single local transition has @p name.
GlobalTransition* transitionNamed(GlobalState* state, const string& name) {
    for (auto transition : state->globalTransitions) {
        for (auto local : transition->localTransitions) {
            if (local->name == name) {
                return transition;
            }
        }
    }
    return nullptr;
}

/// @brief The only local transition of a global transition.
LocalTransition* onlyLocal(GlobalTransition* transition) {
    return *transition->localTransitions.begin();
}

bool verifyText(const string& text) {
    GeneratedModel generated(text);
    return Verification(&generated.generator).verify();
}

bool fixpointText(const string& text) {
    GeneratedModel generated(text);
    return Verification(&generated.generator).fixpointVerify();
}

Result mdpText(const string& sign) {
    string text = COIN_FLIP;
    text.replace(text.find("%SIGN%"), 6, sign);
    GeneratedModel generated(text);
    generated.generator.expandAllStates(true);
    Verification verification(&generated.generator);
    generated.generator.createProbabilityStrategy();
    return verification.verifyMDP();
}

using StrategyTable = vector<tuple<vector<tuple<bool, string>>, string>>;
}

// ---- verStatusToStr, HistoryEntry, HistoryDbg ----------------------------------

TEST(VerStatusToStr, NamesEveryStatus) {
    EXPECT_EQ(verStatusToStr(GlobalStateVerificationStatus::UNVERIFIED), "unverified");
    EXPECT_EQ(verStatusToStr(GlobalStateVerificationStatus::PENDING), "pending");
    EXPECT_EQ(verStatusToStr(GlobalStateVerificationStatus::VERIFIED_OK), "OK");
    EXPECT_EQ(verStatusToStr(GlobalStateVerificationStatus::VERIFIED_ERR), "ERR");
    EXPECT_EQ(verStatusToStr(static_cast<GlobalStateVerificationStatus>(42)), "unknown");
}

class HistoryEntryTest : public UnitTest {
protected:
    ModelArena arena;
    Agent* agent = arena.agent(0, "A");
    LocalState* s0 = arena.state(agent, "s0");
    GlobalState* g0 = arena.globalState({s0});
    GlobalState* g1 = arena.globalState({s0});
    GlobalTransition* move = nullptr;

    void SetUp() override {
        UnitTest::SetUp();
        g0->hash = "g0";
        g1->hash = "g1";
        move = arena.globalTransition(g0, g1, {});
    }

    HistoryEntry entry(HistoryEntryType type) {
        HistoryEntry result;
        result.type = type;
        result.globalState = g0;
        result.decision = move;
        return result;
    }
};

TEST_F(HistoryEntryTest, ToStringDescribesEachEntryType) {
    EXPECT_EQ(entry(HistoryEntryType::DECISION).toString(), "decision in g0: to g1");
    EXPECT_EQ(entry(HistoryEntryType::MARK_DECISION_AS_INVALID).toString(), "markInvalid in g0: to g1");

    HistoryEntry status = entry(HistoryEntryType::STATE_STATUS);
    status.prevStatus = GlobalStateVerificationStatus::UNVERIFIED;
    status.newStatus = GlobalStateVerificationStatus::VERIFIED_OK;
    EXPECT_EQ(status.toString(), "stateVerifStatus of g0: unverified -> OK");

    HistoryEntry context = entry(HistoryEntryType::CONTEXT);
    context.depth = 3;
    context.globalTransitionControlled = true;
    EXPECT_EQ(context.toString(), "context in g0 at depth 3: to g1 (controlled)");

    HistoryEntry uncontrolled = entry(HistoryEntryType::UNCONTROLLED_DECISION);
    uncontrolled.depth = 2;
    EXPECT_EQ(uncontrolled.toString(), "uncontrolledDecision in g0 at depth 2: to g1 (uncontrolled)");

    EXPECT_EQ(entry(HistoryEntryType::PROBABILITY).toString(), "");
}

TEST_F(HistoryEntryTest, DecisionWithAgentNamesTheLocalChoice) {
    HistoryEntry decision = entry(HistoryEntryType::DECISION);
    decision.decisionAgent = agent;
    decision.localDecision = arena.transition(s0, s0, "[0]_go", "go");
    EXPECT_EQ(decision.toString(), "decision by A in local state s0: go");
}

TEST_F(HistoryEntryTest, HistoryDbgMarksClonesAndPrintsEntries) {
    HistoryEntry first = entry(HistoryEntryType::DECISION);
    HistoryEntry second = entry(HistoryEntryType::MARK_DECISION_AS_INVALID);
    HistoryDbg debug;
    debug.addEntry(&first);
    debug.addEntry(&second);
    ASSERT_EQ(debug.entries.size(), 2u);
    EXPECT_EQ(debug.entries[0].second, ' ');

    debug.markEntry(&first, 'R');
    EXPECT_EQ(debug.entries[0].first, &first);
    EXPECT_EQ(debug.entries[0].second, 'R');

    debug.markEntry(&second, '-');                  // '-' keeps a private copy (freed by HistoryDbg)
    EXPECT_NE(debug.entries[1].first, &second);
    EXPECT_EQ(debug.entries[1].first->toString(), second.toString());

    HistoryEntry absent = entry(HistoryEntryType::DECISION);
    EXPECT_EQ(debug.cloneEntry(&absent), nullptr);
    HistoryEntry* clone = debug.cloneEntry(&first);
    ASSERT_NE(clone, nullptr);
    EXPECT_NE(clone, &first);
    EXPECT_EQ(debug.entries[0].first, clone);

    testing::internal::CaptureStdout();
    debug.print(">");
    string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, ">vvvvvvvvvv History:\n>    [R] decision in g0: to g1\n>    [-] markInvalid in g0: to g1\n>^^^^^^^^^^\n");
    delete clone;
}

// ---- history list (through VerificationProbe) ---------------------------------

class VerificationHistoryTest : public UnitTest {
protected:
    std::unique_ptr<GeneratedModel> generated;
    std::unique_ptr<VerificationProbe> verification;
    GlobalState* init = nullptr;
    GlobalTransition* good = nullptr;
    GlobalTransition* bad = nullptr;
    EpistemicClass* initClass = nullptr;

    void SetUp() override {
        UnitTest::SetUp();
        generated.reset(new GeneratedModel(goodBad("<<A>>[](x!=2)")));
        generated->generator.expandAllStates();
        verification.reset(new VerificationProbe(&generated->generator));
        init = generated->model()->initState;
        good = transitionNamed(init, "[0]_good");
        bad = transitionNamed(init, "[0]_bad");
        initClass = init->epistemicClasses.at(generated->agent("A"));
    }
};

TEST_F(VerificationHistoryTest, StartsEmpty) {
    EXPECT_EQ(verification->start(), verification->end());
    EXPECT_EQ(verification->historyLength(), 0);
    verification->undoLastHistoryEntry(true);   // nothing to undo
    EXPECT_EQ(verification->start(), verification->end());
}

TEST_F(VerificationHistoryTest, EntriesAreAppendedToADoublyLinkedList) {
    verification->addHistoryDecision(init, good);
    verification->addHistoryStateStatus(init, GlobalStateVerificationStatus::UNVERIFIED, GlobalStateVerificationStatus::PENDING);
    EXPECT_TRUE(verification->addHistoryContext(init, 4, good, false));
    verification->addHistoryMarkDecisionAsInvalid(init, bad);
    verification->addHistoryUncontrolledDecision(init, bad);

    ASSERT_EQ(verification->historyLength(), 5);
    vector<HistoryEntryType> types;
    HistoryEntry* previous = verification->start();
    for (auto entry = verification->start()->next; entry != nullptr; entry = entry->next) {
        types.push_back(entry->type);
        EXPECT_EQ(entry->prev, previous);
        EXPECT_EQ(entry->globalState, init);
        previous = entry;
    }
    EXPECT_EQ(previous, verification->end());
    EXPECT_EQ(types, (vector<HistoryEntryType>{HistoryEntryType::DECISION, HistoryEntryType::STATE_STATUS, HistoryEntryType::CONTEXT,
                                               HistoryEntryType::MARK_DECISION_AS_INVALID, HistoryEntryType::UNCONTROLLED_DECISION}));
    EXPECT_EQ(verification->start()->next->decision, good);
    EXPECT_EQ(verification->end()->decision, bad);
}

TEST_F(VerificationHistoryTest, NewMarkDecisionAsInvalidIsNotLinked) {
    HistoryEntry* entry = verification->newHistoryMarkDecisionAsInvalid(init, bad);
    EXPECT_EQ(entry->type, HistoryEntryType::MARK_DECISION_AS_INVALID);
    EXPECT_EQ(entry->decision, bad);
    EXPECT_EQ(verification->historyLength(), 0);
    delete entry;
}

TEST_F(VerificationHistoryTest, UndoStateStatusRestoresThePreviousStatus) {
    verification->addHistoryStateStatus(init, GlobalStateVerificationStatus::UNVERIFIED, GlobalStateVerificationStatus::PENDING);
    init->verificationStatus = GlobalStateVerificationStatus::PENDING;
    verification->undoLastHistoryEntry(true);
    EXPECT_EQ(init->verificationStatus, GlobalStateVerificationStatus::UNVERIFIED);
    EXPECT_EQ(verification->historyLength(), 0);
}

TEST_F(VerificationHistoryTest, UndoMarkDecisionAsInvalidMakesItValidAgain) {
    bad->isInvalidDecision = true;
    verification->addHistoryMarkDecisionAsInvalid(init, bad);
    verification->undoLastHistoryEntry(true);
    EXPECT_FALSE(bad->isInvalidDecision);
}

TEST_F(VerificationHistoryTest, UndoAgentDecisionReleasesOnlyThatLocalChoice) {
    Agent* a = generated->agent("A");
    initClass->fixedCoalitionLocalTransition = onlyLocal(good);
    verification->addHistoryDecision(init, good, a, onlyLocal(good));
    verification->undoLastHistoryEntry(true);
    EXPECT_EQ(initClass->fixedCoalitionLocalTransition, nullptr);

    // An entry for a different local choice must not release the current one.
    initClass->fixedCoalitionLocalTransition = onlyLocal(bad);
    verification->addHistoryDecision(init, good, a, onlyLocal(good));
    verification->undoLastHistoryEntry(true);
    EXPECT_EQ(initClass->fixedCoalitionLocalTransition, onlyLocal(bad));
}

TEST_F(VerificationHistoryTest, UndoGlobalDecisionReleasesTheFixedTransition) {
    initClass->fixedCoalitionTransition = good;
    verification->addHistoryDecision(init, good);
    verification->undoLastHistoryEntry(true);
    EXPECT_EQ(initClass->fixedCoalitionTransition, nullptr);
}

TEST_F(VerificationHistoryTest, UndoContextInProbabilityModeRemovesTheAddedProbability) {
    config.probability = true;
    init->probability = 1.0f;
    good->to->probability = 1.0f;
    verification->addHistoryContext(init, 1, good, false);
    verification->undoLastHistoryEntry(true);
    EXPECT_FLOAT_EQ(good->to->probability, 0.0f);
}

TEST_F(VerificationHistoryTest, UndoHistoryUntilStopsAtOrRemovesTheGivenEntry) {
    verification->addHistoryMarkDecisionAsInvalid(init, good);
    HistoryEntry* first = verification->end();
    verification->addHistoryMarkDecisionAsInvalid(init, bad);
    HistoryEntry* second = verification->end();
    verification->addHistoryUncontrolledDecision(init, bad);

    verification->undoHistoryUntil(second, false, 0);
    EXPECT_EQ(verification->end(), second);
    EXPECT_EQ(verification->historyLength(), 2);

    verification->undoHistoryUntil(second, true, 0);
    EXPECT_EQ(verification->end(), first);
    EXPECT_EQ(verification->historyLength(), 1);
}

TEST_F(VerificationHistoryTest, NaturalStrategyRecordsOneActionPerValuation) {
    config.natural_strategy = true;
    EXPECT_TRUE(verification->addHistoryContext(init, 0, good, true));
    EXPECT_EQ(verification->end()->strategy.type, StrategyEntryType::ADDED);
    auto strategy = verification->getNaturalStrategy();
    ASSERT_EQ(strategy.size(), 1u);
    EXPECT_EQ(strategy.begin()->first.to_ulong(), 0u);          // x == 0 -> no bits set
    EXPECT_EQ(strategy.begin()->second, "[0]_good");

    EXPECT_TRUE(verification->addHistoryContext(init, 0, good, true));   // same choice again
    EXPECT_EQ(verification->end()->strategy.type, StrategyEntryType::NOT_MODIFIED);
    EXPECT_FALSE(verification->addHistoryContext(init, 0, bad, true));   // conflicting choice

    // Undoing the entries removes the action only together with the entry that added it.
    verification->undoLastHistoryEntry(true);
    verification->undoLastHistoryEntry(true);
    EXPECT_EQ(verification->getNaturalStrategy().size(), 1u);
    verification->undoLastHistoryEntry(true);
    EXPECT_TRUE(verification->getNaturalStrategy().empty());
}

// ---- coalition and epistemic helpers -------------------------------------------

class VerificationCoalitionTest : public UnitTest {
protected:
    ModelArena arena;
};

TEST_F(VerificationCoalitionTest, AgentAndTransitionControl) {
    GeneratedModel generated(opponent("<<A>>[](y!=2)", true));
    generated.generator.expandAllStates();
    VerificationProbe verification(&generated.generator);
    Agent* a = generated.agent("A");
    Agent* b = generated.agent("B");
    GlobalState* init = generated.model()->initState;

    EXPECT_TRUE(verification.isAgentInCoalition(a));
    EXPECT_FALSE(verification.isAgentInCoalition(b));

    string waitName = "[" + to_string(a->id) + "]_wait";
    string goodName = "[" + to_string(b->id) + "]_good";
    EXPECT_TRUE(verification.isGlobalTransitionControlledByCoalition(transitionNamed(init, waitName)));
    EXPECT_FALSE(verification.isGlobalTransitionControlledByCoalition(transitionNamed(init, goodName)));

    // A shared action fires only if every participant chooses its local action, so the coalition takes part whatever the local name.
    LocalTransition shared;
    shared.agent = a;
    shared.isShared = true;
    shared.name = "sync";
    shared.localName = "sync_A";
    GlobalTransition joint;
    joint.localTransitions = {&shared};
    EXPECT_TRUE(verification.isGlobalTransitionControlledByCoalition(&joint));
    shared.localName = "sync";
    EXPECT_TRUE(verification.isGlobalTransitionControlledByCoalition(&joint));

    // The epsilon self-loop only lets the system idle, it is no choice of the coalition.
    shared.isShared = false;
    shared.name = "ɛ";
    shared.localName = "ɛ";
    EXPECT_FALSE(verification.isGlobalTransitionControlledByCoalition(&joint));
}

TEST_F(VerificationCoalitionTest, CompatibilityWithFixedDecisions) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    generated.generator.expandAllStates();
    VerificationProbe verification(&generated.generator);
    GlobalState* init = generated.model()->initState;
    GlobalTransition* good = transitionNamed(init, "[0]_good");
    GlobalTransition* bad = transitionNamed(init, "[0]_bad");
    EpistemicClass* initClass = init->epistemicClasses.at(generated.agent("A"));

    EXPECT_TRUE(verification.isGlobalTransitionCompatibleWithCoalitionDecisions(good));

    initClass->fixedCoalitionLocalTransition = onlyLocal(good);
    EXPECT_TRUE(verification.isGlobalTransitionCompatibleWithCoalitionDecisions(good));
    EXPECT_FALSE(verification.isGlobalTransitionCompatibleWithCoalitionDecisions(bad));

    // A decision is a local action: another transition with the same local action stays possible.
    LocalTransition sameAction = *onlyLocal(good);
    GlobalTransition otherOutcome;
    otherOutcome.from = init;
    otherOutcome.localTransitions = {&sameAction};
    EXPECT_TRUE(verification.isGlobalTransitionCompatibleWithCoalitionDecisions(&otherOutcome));
}

TEST_F(VerificationCoalitionTest, MissingEpistemicClassIsALogicError) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    VerificationProbe verification(&generated.generator);
    LocalTransition local;
    local.agent = generated.agent("A");
    local.isShared = false;
    GlobalState orphan;   // has no epistemic classes
    GlobalTransition transition;
    transition.from = &orphan;
    transition.localTransitions = {&local};
    EXPECT_THROW(verification.isGlobalTransitionCompatibleWithCoalitionDecisions(&transition), std::logic_error);
    EXPECT_THROW(verification.addCoalitionDecisionHistory(&orphan, &transition), std::logic_error);
}

TEST_F(VerificationCoalitionTest, AddCoalitionDecisionHistoryFixesEachClassOnce) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    generated.generator.expandAllStates();
    VerificationProbe verification(&generated.generator);
    GlobalState* init = generated.model()->initState;
    GlobalTransition* good = transitionNamed(init, "[0]_good");
    EpistemicClass* initClass = init->epistemicClasses.at(generated.agent("A"));

    verification.addCoalitionDecisionHistory(init, good);
    EXPECT_EQ(initClass->fixedCoalitionLocalTransition, onlyLocal(good));
    ASSERT_EQ(verification.historyLength(), 1);
    EXPECT_EQ(verification.end()->decisionAgent, generated.agent("A"));
    EXPECT_EQ(verification.end()->localDecision, onlyLocal(good));

    verification.addCoalitionDecisionHistory(init, transitionNamed(init, "[0]_bad"));
    EXPECT_EQ(initClass->fixedCoalitionLocalTransition, onlyLocal(good));
    EXPECT_EQ(verification.historyLength(), 1);
}

TEST_F(VerificationCoalitionTest, EpistemicClassLookupAndComparison) {
    GeneratedModel generated(opponent("<<A>>[](y!=2)"));
    generated.generator.expandAllStates();
    VerificationProbe verification(&generated.generator);
    GlobalState* init = generated.model()->initState;
    GlobalState orphan;

    EXPECT_EQ(verification.getEpistemicClassForGlobalState(init), init->epistemicClasses.at(generated.agent("A")));
    EXPECT_EQ(verification.getEpistemicClassForGlobalState(&orphan), nullptr);
    // A never moves, so it cannot tell any two states apart.
    for (auto state : generated.model()->globalStates) {
        EXPECT_TRUE(verification.areGlobalStatesInTheSameEpistemicClass(init, state));
    }
    EXPECT_FALSE(verification.areGlobalStatesInTheSameEpistemicClass(init, &orphan));

    GeneratedModel choice(goodBad("<<A>>[](x!=2)"));
    choice.generator.expandAllStates();
    VerificationProbe choiceVerification(&choice.generator);
    GlobalState* afterGood = transitionNamed(choice.model()->initState, "[0]_good")->to;
    EXPECT_FALSE(choiceVerification.areGlobalStatesInTheSameEpistemicClass(choice.model()->initState, afterGood));
}

TEST_F(VerificationCoalitionTest, EquivalentGlobalTransitionsCompareCoalitionLocalNames) {
    GeneratedModel generated(opponent("<<A>>[](y!=2)"));
    VerificationProbe verification(&generated.generator);
    LocalTransition x, sameX, y, opponentMove;
    x.agent = sameX.agent = y.agent = generated.agent("A");
    opponentMove.agent = generated.agent("B");
    x.localName = sameX.localName = "x";
    y.localName = "y";
    opponentMove.localName = "o";
    GlobalTransition withX, withSameX, withY, onlyOpponent;
    withX.localTransitions = {&x};
    withSameX.localTransitions = {&sameX};
    withY.localTransitions = {&y};
    onlyOpponent.localTransitions = {&opponentMove};

    EXPECT_TRUE(verification.equivalentGlobalTransitions(&withX, &withSameX));
    EXPECT_FALSE(verification.equivalentGlobalTransitions(&withX, &withY));
    EXPECT_TRUE(verification.equivalentGlobalTransitions(&onlyOpponent, &withY));   // no coalition part to compare
}

TEST_F(VerificationCoalitionTest, EquivalentGlobalTransitionsChecksEveryCoalitionPart) {
    GeneratedModel generated(opponent("<<A>>[](y!=2)"));
    VerificationProbe verification(&generated.generator);
    // One array, so the set (ordered by address) visits the matching part first.
    LocalTransition parts[2];
    parts[0].agent = parts[1].agent = generated.agent("A");
    parts[0].localName = "x";
    parts[1].localName = "z";
    LocalTransition other;
    other.agent = generated.agent("A");
    other.localName = "x";
    GlobalTransition twoParts, onlyX;
    twoParts.localTransitions = {&parts[0], &parts[1]};
    onlyX.localTransitions = {&other};
    EXPECT_FALSE(verification.equivalentGlobalTransitions(&twoParts, &onlyX));   // "z" has no counterpart
}

TEST_F(VerificationCoalitionTest, VerifyLocalStatesEvaluatesTheFormula) {
    GeneratedModel generated(goodBad("<<A>>[](x==0)"));
    generated.generator.expandAllStates();
    VerificationProbe verification(&generated.generator);
    GlobalState* init = generated.model()->initState;
    GlobalState* afterGood = transitionNamed(init, "[0]_good")->to;
    EXPECT_TRUE(verification.verifyLocalStates(&init->localStatesProjection, init));
    EXPECT_FALSE(verification.verifyLocalStates(&afterGood->localStatesProjection, afterGood));
}

// A declared variable without an INITIAL value starts at 0, so the formula can always read it.
TEST_F(VerificationCoalitionTest, VerifyLocalStatesReadsDeclaredVariableWithoutInitialValueAsZero) {
    GeneratedModel generated(R"(
Agent A:
LOCAL: [x, unset]
PERSISTENT: [x]
INITIAL: [x:=0]
init s0
go: s0 -> s1 [unset:=1]

FORMULA: <<A>>[](unset==0)
)");
    VerificationProbe verification(&generated.generator);
    GlobalState* init = generated.model()->initState;
    EXPECT_TRUE(verification.verifyLocalStates(&init->localStatesProjection, init));
}

TEST_F(VerificationCoalitionTest, ProbabilityIsPropagatedAndRemoved) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    VerificationProbe verification(&generated.generator);
    Agent* agent = arena.agent(0, "P");
    LocalState* s = arena.state(agent, "s");
    GlobalState* from = arena.globalState({s});
    GlobalState* to = arena.globalState({s});
    from->probability = 0.5f;
    to->probability = 0.0f;
    GlobalTransition* step = arena.globalTransition(from, to, {arena.transition(s, s, "half", "", false, -1, 0.5f)});

    verification.increaseProbability(from, step);
    EXPECT_FLOAT_EQ(to->probability, 0.25f);
    verification.increaseProbability(from, step);
    EXPECT_FLOAT_EQ(to->probability, 0.5f);
    verification.lowerProbability(from, to, step->localTransitions);
    EXPECT_FLOAT_EQ(to->probability, 0.25f);
}

// ---- natural strategies ---------------------------------------------------------

TEST_F(VerificationCoalitionTest, GlobalStateToValueBitsUsesOneBitPerCoalitionVariable) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    VerificationProbe verification(&generated.generator);
    LocalState local;
    local.agent = generated.agent("A");
    local.environment = {{"a", 1}, {"b", 0}, {"c", 5}};   // non-zero counts as true
    GlobalState state;
    state.localStatesProjection = {&local};

    EXPECT_EQ(verification.globalStateToValueBits(&state).to_ulong(), 0b101u);
    EXPECT_EQ(verification.names(), (vector<string>{"a", "b", "c"}));
    EXPECT_EQ(verification.variableLimit(), 3);

    local.environment = {{"a", 0}, {"b", 7}, {"c", 0}};
    EXPECT_EQ(verification.globalStateToValueBits(&state).to_ulong(), 0b010u);
    EXPECT_EQ(verification.names(), (vector<string>{"a", "b", "c"}));   // captured only once
}

TEST_F(VerificationCoalitionTest, ReduceStrategyDropsColumnsWithOneValue) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    VerificationProbe verification(&generated.generator);
    // Column "a" is true in every row, so it cannot distinguish the actions.
    StrategyTable table = {
        {{{true, "a"}, {true, "b"}}, "X"},
        {{{true, "a"}, {false, "b"}}, "Y"}};
    StrategyTable expected = {
        {{{true, "b"}}, "X"},
        {{{false, "b"}}, "Y"}};
    EXPECT_EQ(verification.reduceStrategy(table), expected);
}

TEST_F(VerificationCoalitionTest, ReduceStrategySplitsOnTheFirstInformativeColumn) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    VerificationProbe verification(&generated.generator);
    // Split on "a": rows with a=true keep both columns, in the a=false half "a" is constant and dropped.
    StrategyTable table = {
        {{{true, "a"}, {true, "b"}}, "act1"},
        {{{false, "a"}, {true, "b"}}, "act2"},
        {{{true, "a"}, {false, "b"}}, "act1"},
        {{{false, "a"}, {false, "b"}}, "act2"}};
    StrategyTable expected = {
        {{{true, "a"}, {true, "b"}}, "act1"},
        {{{true, "a"}, {false, "b"}}, "act1"},
        {{{true, "b"}}, "act2"},
        {{{false, "b"}}, "act2"}};
    EXPECT_EQ(verification.reduceStrategy(table), expected);
}

TEST_F(VerificationCoalitionTest, ReducedStrategyIsEmptyBeforeVerification) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    Verification verification(&generated.generator);
    EXPECT_TRUE(verification.getReducedStrategy().empty());
    EXPECT_EQ(verification.getStrategyComplexity(), 0);
    EXPECT_TRUE(verification.getNaturalStrategy().empty());
}

TEST_F(VerificationCoalitionTest, NaturalStrategyAfterVerification) {
    config.natural_strategy = true;
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    generated.generator.expandAllStates();
    Verification verification(&generated.generator);
    ASSERT_TRUE(verification.verify());

    // In s0 (x == 0, no bits set) the winning action is "good".
    auto strategy = verification.getNaturalStrategy();
    ASSERT_EQ(strategy.count(bitset<STRATEGY_BITS>(0)), 1u);
    EXPECT_EQ(strategy.at(bitset<STRATEGY_BITS>(0)), "[0]_good");

    StrategyTable reduced = verification.getReducedStrategy();
    ASSERT_EQ(reduced.size(), strategy.size());
    // The last rule is the catch-all "otherwise" rule.
    EXPECT_EQ(get<0>(reduced.back()), (vector<tuple<bool, string>>{{true, "T"}}));
    EXPECT_GT(verification.getStrategyComplexity(), 0);
}

// ---- end-to-end verification on micro models -----------------------------------

class VerificationResultTest : public UnitTest {};

TEST_F(VerificationResultTest, GloballyWithAChoice) {
    EXPECT_TRUE(verifyText(goodBad("<<A>>[](x!=2)")));   // choose good
    EXPECT_TRUE(verifyText(goodBad("<<A>>[](x!=1)")));   // choose bad
    EXPECT_FALSE(verifyText(goodBad("<<A>>[](x==0)")));  // every choice changes x
}

TEST_F(VerificationResultTest, EventuallyWithAChoice) {
    EXPECT_TRUE(verifyText(goodBad("<<A>><>(x==1)")));
    EXPECT_TRUE(verifyText(goodBad("<<A>><>(x==2)")));
    EXPECT_FALSE(verifyText(goodBad("<<A>><>(x==3)")));  // unreachable
}

TEST_F(VerificationResultTest, OpponentChoicesMustAllBeSafe) {
    EXPECT_FALSE(verifyText(opponent("<<A>>[](y!=2)")));  // B may pick bad
    EXPECT_TRUE(verifyText(opponent("<<B>>[](y!=2)")));   // B itself avoids bad
    EXPECT_FALSE(verifyText(opponent("<<A>><>(y==1)")));  // B may never pick good
}

TEST_F(VerificationResultTest, FixpointAgreesWithOnTheFlyVerification) {
    const vector<string> models = {
        goodBad("<<A>>[](x!=2)"), goodBad("<<A>>[](x==0)"),
        goodBad("<<A>><>(x==1)"), goodBad("<<A>><>(x==3)"),
        opponent("<<A>>[](y!=2)"), opponent("<<B>>[](y!=2)")};
    for (const auto& model : models) {
        EXPECT_EQ(fixpointText(model), verifyText(model)) << model;
    }
}

TEST_F(VerificationResultTest, FixpointAgreesWhenCoalitionAndOpponentMovesAreMixed) {
    const string model = opponent("<<A>>[](y!=2)", true);
    EXPECT_EQ(fixpointText(model), verifyText(model));
}

TEST_F(VerificationResultTest, VerifyStrategyFollowsTheGivenActions) {
    config.verify_strategy = true;   // otherwise verifyStrategy ignores the strategy
    auto run = [](const string& actionAtStart) {
        GeneratedModel generated(goodBad("<<A>><>(x==1)"));
        Agent* a = generated.agent("A");
        auto id = [&](const string& name) { return to_string(stateNamed(a, name)->id) + ";"; };
        StrategyCollection strategy;
        for (auto entry : vector<pair<string, string>>{{id("s0"), actionAtStart}, {id("s1"), "stay1"}, {id("s2"), "stay2"}}) {
            Action action;
            action.states = nullptr;
            action.hash = entry.first;
            action.actionName = entry.second;
            strategy.addAction(action);
        }
        generated.generator.initStrategy(&strategy);
        Verification verification(&generated.generator);
        testing::internal::CaptureStdout();
        Result result = verification.verifyStrategy();
        testing::internal::GetCapturedStdout();
        return result.verificationResult;
    };
    EXPECT_TRUE(run("good"));
    EXPECT_FALSE(run("bad"));
}

TEST_F(VerificationResultTest, VerifyStrategyTerminatesForGloballyFormulas) {
    config.verify_strategy = true;
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    Agent* a = generated.agent("A");
    Action action;
    action.states = nullptr;
    action.hash = to_string(stateNamed(a, "s0")->id) + ";";
    action.actionName = "bad";
    StrategyCollection strategy(action.hash, action);
    generated.generator.initStrategy(&strategy);
    Verification verification(&generated.generator);
    testing::internal::CaptureStdout();
    Result result = verification.verifyStrategy();
    testing::internal::GetCapturedStdout();
    EXPECT_FALSE(result.verificationResult);
}

TEST_F(VerificationResultTest, VerifyMdpComparesTheBestProbabilityWithTheBound) {
    // P(eventually heads) = 0.5
    Result atLeastHalf = mdpText(">=");
    EXPECT_TRUE(atLeastHalf.verificationResult);
    EXPECT_NEAR(atLeastHalf.probabilityResult.probabilityTrue, 0.5f, 1e-3);
    EXPECT_NEAR(atLeastHalf.probabilityResult.probabilityFalse, 0.5f, 1e-3);

    EXPECT_FALSE(mdpText(">").verificationResult);
    EXPECT_TRUE(mdpText("<=").verificationResult);
    EXPECT_FALSE(mdpText("<").verificationResult);
}

TEST_F(VerificationResultTest, HistoryDecisionsErrPrintsThePathIntoErrorStates) {
    GeneratedModel generated(goodBad("<<A>>[](x!=2)"));
    generated.generator.expandAllStates();
    GlobalState* init = generated.model()->initState;
    transitionNamed(init, "[0]_bad")->to->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;

    Verification verification(&generated.generator);
    testing::internal::CaptureStdout();
    verification.historyDecisionsERR();
    string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output.rfind("States:\ns0;\n[A] s0 (x=0;)\n", 0), 0u);
    EXPECT_NE(output.find("Decisions:\n[0]_bad;\n[A] [0]_bad\n"), string::npos);
    EXPECT_NE(output.find("[A] s2 (x=2;)"), string::npos);
    EXPECT_EQ(output.find("s1"), string::npos);   // the good branch is not part of the error path
}
