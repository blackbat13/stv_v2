/**
 * @file Verification.hpp
 */
#define STRATEGY_BITS 64

#ifndef SELENE_VERIFICATION
#define SELENE_VERIFICATION

#include <stack>
#include "Types.hpp"
#include "TypesDependency.hpp"
#include "GlobalModelGenerator.hpp"
#include <bitset>

string verStatusToStr(GlobalStateVerificationStatus status);

/// @brief Structure used to save model traversal history.
struct HistoryEntry {
    /// @brief Type of the history record.
    HistoryEntryType type;
    /// @brief Saved global state.
    GlobalState* globalState = nullptr;
    /// @brief Selected transition.
    GlobalTransition* decision = nullptr;
    /// @brief Agent whose local transition was selected.
    Agent* decisionAgent = nullptr;
    /// @brief Local transition selected for the agent's current local state.
    LocalTransition* localDecision = nullptr;
    /// @brief Is the transition controlled by an agent in coalition.
    bool globalTransitionControlled = false;
    /// @brief Previous model verification state.
    GlobalStateVerificationStatus prevStatus = GlobalStateVerificationStatus::UNVERIFIED;
    /// @brief Next model verification state.
    GlobalStateVerificationStatus newStatus = GlobalStateVerificationStatus::UNVERIFIED;
    /// @brief Recursion depth.
    int depth = 0;
    /// @brief Holds currently processed strategy for the current state.
    StrategyEntry strategy;
    /// @brief Pointer to the previous HistoryEntry.
    HistoryEntry* prev = nullptr;
    /// @brief Pointer to the next HistoryEntry.
    HistoryEntry* next = nullptr;
    /// @brief Converts HistoryEntry to string.
    /// @return A string with the descriprion of this history record.
    string toString() {
        char buff[1024] = { 0 };
        if (this->type == HistoryEntryType::DECISION) {
            if (this->decisionAgent != nullptr && this->localDecision != nullptr) {
                snprintf(buff, sizeof(buff), "decision by %s in local state %s: %s", this->decisionAgent->name.c_str(), this->localDecision->from->name.c_str(), this->localDecision->localName.c_str());
            } else {
                snprintf(buff, sizeof(buff), "decision in %s: to %s", this->globalState->hash.c_str(), this->decision->to->hash.c_str());
            }
        }
        else if (this->type == HistoryEntryType::STATE_STATUS) {
            snprintf(buff, sizeof(buff), "stateVerifStatus of %s: %s -> %s", this->globalState->hash.c_str(), verStatusToStr(this->prevStatus).c_str(), verStatusToStr(this->newStatus).c_str());
        }
        else if (this->type == HistoryEntryType::CONTEXT) {
            snprintf(buff, sizeof(buff), "context in %s at depth %i: to %s (%s)", this->globalState->hash.c_str(), this->depth, this->decision->to->hash.c_str(), this->globalTransitionControlled ? "controlled" : "uncontrolled");
        }
        else if (this->type == HistoryEntryType::MARK_DECISION_AS_INVALID) {
            snprintf(buff, sizeof(buff), "markInvalid in %s: to %s", this->globalState->hash.c_str(), this->decision->to->hash.c_str());
        }
        else if (this->type == HistoryEntryType::UNCONTROLLED_DECISION) {
            snprintf(buff, sizeof(buff), "uncontrolledDecision in %s at depth %i: to %s (%s)", this->globalState->hash.c_str(), this->depth, this->decision->to->hash.c_str(), this->globalTransitionControlled ? "controlled" : "uncontrolled");
        }
        return string(buff);
    };
};

/// @brief Stores history and allows displaying it to the console.
class HistoryDbg {
public:
    /// @brief A pair of history entries and a char marking history type.
    vector<pair<HistoryEntry*, char>> entries;
    HistoryDbg();
    ~HistoryDbg();
    void addEntry(HistoryEntry* entry);
    void markEntry(HistoryEntry* entry, char chr);
    void print(string prefix);
    HistoryEntry* cloneEntry(HistoryEntry* entry);
};

// On-the-fly traversal mode
/// @brief Current model traversal mode.
enum TraversalMode {
    NORMAL, ///< Normal model traversal.
    REVERT, ///< Backtracking through recursion with state rollback.
    RESTORE, ///< Backtracking through recursion.
};

/// @brief A class that verifies if the model fulfills the formula. Also can do some operations on decision history.
class Verification {
public:
    Verification(GlobalModelGenerator* generator);
    ~Verification();
    bool verify();
    bool fixpointVerify();
    void historyDecisionsERR();
    map<bitset<STRATEGY_BITS>, string, StrategyBitsComparator> getNaturalStrategy();
    vector<tuple<vector<tuple<bool, string>>, string>> getReducedStrategy();
    int getStrategyComplexity();
    Result verifyStrategy();
    Result verifyMDP();
protected:
    /// @brief Current mode of model traversal.
    TraversalMode mode;
    /// @brief Global state to which revert will rollback to.
    GlobalState* revertToGlobalState;
    /// @brief A history of decisions to be rolled back.
    stack<HistoryEntry*> historyToRestore;
    /// @brief Holds current model and formula.
    GlobalModelGenerator* generator;
    /// @brief States currently being verified on the active recursion path.
    set<GlobalState*> activeGlobalStates;
    /// @brief Pointer to the start of model traversal history.
    HistoryEntry* historyStart;
    /// @brief Pointer to the end of model traversal history.
    HistoryEntry* historyEnd;
    /// @brief A table of actions paired vith globalState internal variables state for natural strategy construction.
    map<bitset<STRATEGY_BITS>, string, StrategyBitsComparator> naturalStrategy;
    /// @brief Max strategy variables found.
    short strategyVariableLimit;
    /// @brief Easily readable variable names for natural strategy generation.
    vector<string> variableNames;
    /// @brief Natural strategy complexity before reduction
    int reductionComplexityBefore;

    bool verifyLocalStates(vector<LocalState*>* localStates, GlobalState* globalState);
    bool verifyGlobalState(GlobalState* globalState, int depth);
    bool isGlobalTransitionControlledByCoalition(GlobalTransition* globalTransition);
    bool isGlobalTransitionCompatibleWithCoalitionDecisions(GlobalTransition* globalTransition);
    bool hasUnfixedCoalitionDecision(GlobalTransition* globalTransition);
    void addCoalitionDecisionHistory(GlobalState* globalState, GlobalTransition* decision);
    bool isAgentInCoalition(Agent* agent);
    EpistemicClass* getEpistemicClassForGlobalState(GlobalState* globalState);
    bool areGlobalStatesInTheSameEpistemicClass(GlobalState* globalState1, GlobalState* globalState2);
    void addHistoryDecision(GlobalState* globalState, GlobalTransition* decision, Agent* decisionAgent = nullptr, LocalTransition* localDecision = nullptr);
    void addHistoryStateStatus(GlobalState* globalState, GlobalStateVerificationStatus prevStatus, GlobalStateVerificationStatus newStatus);
    bool addHistoryContext(GlobalState* globalState, int depth, GlobalTransition* decision, bool globalTransitionControlled);
    void addHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision);
    void addHistoryUncontrolledDecision(GlobalState* globalState, GlobalTransition* decision);
    HistoryEntry* newHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision);
    bool revertLastDecision(int depth);
    void undoLastHistoryEntry(bool freeMemory);
    void undoHistoryUntil(HistoryEntry* historyEntry, bool inclusive, int depth);
    void printCurrentHistory(int depth);
    bool equivalentGlobalTransitions(GlobalTransition* globalTransition1, GlobalTransition* globalTransition2);
    bool checkUncontrolledSet(const set<GlobalTransition*>& uncontrolledGlobalTransitions, GlobalState* globalState, int depth, bool hasOmittedTransitions, bool mixed = false);
    bool verifyTransitionSets(set<GlobalTransition*> controlledGlobalTransitions, const set<GlobalTransition*>& uncontrolledGlobalTransitions, const set<GlobalTransition*>& mandatoryOpponentTransitions, GlobalState* globalState, int depth, bool hasOmittedTransitions, bool isFMode, bool mixed = false);
    bool restoreHistory(GlobalState* globalState, GlobalTransition* globalTransition, int depth, bool controlled);
    bool minFixpointVerify();
    bool maxFixpointVerify();
    bitset<STRATEGY_BITS> globalStateToValueBits(GlobalState* globalState);
    vector<tuple<vector<tuple<bool, string>>, string>> reduceStrategy(vector<tuple<vector<tuple<bool, string>>, string>> strategyEntries, short lockedColumn = 0, bool upperHalf = false);
    void increaseProbability(GlobalState* currentState, GlobalTransition* decision);
    void lowerProbability(GlobalState* currentStateFrom, GlobalState* currentStateTo, set<LocalTransition*> decision);
};

#endif // SELENE_VERIFICATION
