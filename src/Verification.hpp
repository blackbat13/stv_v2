/**
 * @file Verification.hpp
 */

#ifndef SELENE_VERIFICATION
#define SELENE_VERIFICATION

#include <stack>
#include "Types.hpp"
#include "GlobalModelGenerator.hpp"

/// @brief HistoryEntry entry type.
enum HistoryEntryType {
    DECISION, ///< Made the decision to go to a state using a transition.
    STATE_STATUS, ///< Changed verification status.
    CONTEXT, ///< Recursion has gone deeper.
    MARK_DECISION_AS_INVALID, ///< Marking a transition as invalid.
};

string verStatusToStr(GlobalStateVerificationStatus status);

/// @brief Structure used to save model traversal history.
struct HistoryEntry {
    /// @brief Type of the history record.
    HistoryEntryType type;
    /// @brief Saved global state.
    shared_ptr<GlobalState> globalState;
    /// @brief Selected transition.
    shared_ptr<GlobalTransition> decision;
    /// @brief Is the transition controlled by an agent in coalition.
    bool globalTransitionControlled;
    /// @brief Previous model verification state.
    GlobalStateVerificationStatus prevStatus;
    /// @brief Next model verification state.
    GlobalStateVerificationStatus newStatus;
    /// @brief Recursion depth.
    int depth;
    /// @brief Pointer to the previous HistoryEntry.
    HistoryEntry* prev;
    /// @brief Pointer to the next HistoryEntry.
    HistoryEntry* next;
    /// @brief Converts HistoryEntry to string.
    /// @return A string with the descriprion of this history record.
    string toString() {
        char buff[1024] = { 0 };
        if (this->type == HistoryEntryType::DECISION) {
            snprintf(buff, sizeof(buff), "decision in %s: to %s", this->globalState->hash.c_str(), this->decision->to->hash.c_str());
        }
        else if (this->type == HistoryEntryType::STATE_STATUS) {
            snprintf(buff, sizeof(buff), "stateVerStatus of %s: %s -> %s", this->globalState->hash.c_str(), verStatusToStr(this->prevStatus).c_str(), verStatusToStr(this->newStatus).c_str());
        }
        else if (this->type == HistoryEntryType::CONTEXT) {
            snprintf(buff, sizeof(buff), "context in %s at depth %i: to %s (%s)", this->globalState->hash.c_str(), this->depth, this->decision->to->hash.c_str(), this->globalTransitionControlled ? "controlled" : "uncontrolled");
        }
        else if (this->type == HistoryEntryType::MARK_DECISION_AS_INVALID) {
            snprintf(buff, sizeof(buff), "markInvalid in %s: to %s", this->globalState->hash.c_str(), this->decision->to->hash.c_str());
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
    Verification(shared_ptr<GlobalModelGenerator> generator);
    ~Verification();
    bool verify();
protected:
    /// @brief Current mode of model traversal.
    TraversalMode mode;
    /// @brief Global state to which revert will rollback to.
    shared_ptr<GlobalState> revertToGlobalState;
    /// @brief A history of decisions to be rolled back.
    stack<HistoryEntry*> historyToRestore;
    /// @brief Holds current model and formula.
    shared_ptr<GlobalModelGenerator> generator;
    /// @brief Pointer to the start of model traversal history.
    HistoryEntry* historyStart;
    /// @brief Pointer to the end of model traversal history.
    HistoryEntry* historyEnd;
    bool verifyLocalStates(vector<shared_ptr<LocalState>>* localStates);
    bool verifyGlobalState(shared_ptr<GlobalState> globalState, int depth);
    bool isGlobalTransitionControlledByCoalition(shared_ptr<GlobalTransition> globalTransition);
    bool isAgentInCoalition(Agent* agent);
    shared_ptr<EpistemicClass> getEpistemicClassForGlobalState(shared_ptr<GlobalState> globalState);
    bool areGlobalStatesInTheSameEpistemicClass(shared_ptr<GlobalState> globalState1, shared_ptr<GlobalState> globalState2);
    void addHistoryDecision(shared_ptr<GlobalState> globalState, shared_ptr<GlobalTransition> ecision);
    void addHistoryStateStatus(shared_ptr<GlobalState> globalState, GlobalStateVerificationStatus prevStatus, GlobalStateVerificationStatus newStatus);
    void addHistoryContext(shared_ptr<GlobalState> globalState, int depth, shared_ptr<GlobalTransition> decision, bool globalTransitionControlled);
    void addHistoryMarkDecisionAsInvalid(shared_ptr<GlobalState> globalState, shared_ptr<GlobalTransition> decision);
    HistoryEntry* newHistoryMarkDecisionAsInvalid(shared_ptr<GlobalState> globalState, shared_ptr<GlobalTransition> decision);
    bool revertLastDecision(int depth);
    void undoLastHistoryEntry(bool freeMemory);
    void undoHistoryUntil(HistoryEntry* historyEntry, bool inclusive, int depth);
    void printCurrentHistory(int depth);
    bool equivalentGlobalTransitions(shared_ptr<GlobalTransition> globalTransition1, shared_ptr<GlobalTransition> globalTransition2);
    bool checkUncontrolledSet(set<shared_ptr<GlobalTransition>> uncontrolledGlobalTransitions, shared_ptr<GlobalState> globalState, int depth, bool hasOmittedTransitions);
    bool verifyTransitionSets(set<shared_ptr<GlobalTransition>> controlledGlobalTransitions, set<shared_ptr<GlobalTransition>> uncontrolledGlobalTransitions, shared_ptr<GlobalState> globalState, int depth, bool hasOmittedTransitions);
    bool restoreHistory(shared_ptr<GlobalState> globalState, shared_ptr<GlobalTransition> globalTransition, int depth, bool controlled);
};

#endif // SELENE_VERIFICATION
