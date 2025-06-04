/**
 * @file Verification.hpp
 */
#ifndef VERIFICATION_ITERATIVE
#define VERIFICATION_ITERATIVE

#include <stack>
#include "Types.hpp"
#include "TypesDependency.hpp"
#include "GlobalModelGenerator.hpp"
#include <bitset>

string verifStatusToStr(GlobalStateVerificationStatus status);

/// @brief 
struct DecisionEntry {
    /// @brief Type of the history record.
    HistoryEntryType type = HistoryEntryType::CONTEXT;
    /// @brief Saved global state.
    GlobalState* globalState = nullptr;
    /// @brief Selected transition.
    GlobalTransition* decision = nullptr;
    /// @brief Is the transition controlled by an agent in coalition.
    bool globalTransitionControlled = false;
    /// @brief Previous model verification state.
    GlobalStateVerificationStatus prevStatus = GlobalStateVerificationStatus::UNVERIFIED;
    /// @brief Next model verification state.
    GlobalStateVerificationStatus newStatus = GlobalStateVerificationStatus::UNVERIFIED;
    /// @brief Holds currently processed strategy for the current state.
    StrategyEntry strategy = StrategyEntry();
    string toString() {
        char buff[1024] = { 0 };
        if (this->type == HistoryEntryType::DECISION) {
            snprintf(buff, sizeof(buff), "decision in %s: to %s", this->globalState->hash.c_str(), this->decision->to->hash.c_str());
        }
        else if (this->type == HistoryEntryType::STATE_STATUS) {
            snprintf(buff, sizeof(buff), "stateVerifStatus of %s: %s -> %s", this->globalState->hash.c_str(), verifStatusToStr(this->prevStatus).c_str(), verifStatusToStr(this->newStatus).c_str());
        }
        else if (this->type == HistoryEntryType::CONTEXT) {
            snprintf(buff, sizeof(buff), "context in %s: to %s (%s)", this->globalState->hash.c_str(), this->decision->to->hash.c_str(), this->globalTransitionControlled ? "controlled" : "uncontrolled");
        }
        else if (this->type == HistoryEntryType::MARK_DECISION_AS_INVALID) {
            snprintf(buff, sizeof(buff), "markInvalid in %s: to %s", this->globalState->hash.c_str(), this->decision->to->hash.c_str());
        }
        else if (this->type == HistoryEntryType::UNCONTROLLED_DECISION) {
            snprintf(buff, sizeof(buff), "uncontrolledDecision in %s: to %s (%s)", this->globalState->hash.c_str(), this->decision->to->hash.c_str(), this->globalTransitionControlled ? "controlled" : "uncontrolled");
        }
        return string(buff);
    };
};

/// @brief A class that verifies if the model fulfills the formula. Also can do some operations on decision history.
class VerificationIterative {
    public:
        VerificationIterative(GlobalModelGenerator* generator);
        ~VerificationIterative();
        map<bitset<STRATEGY_BITS>, string, StrategyBitsComparator> getNaturalStrategy();
        vector<tuple<vector<tuple<bool, string>>, string>> getReducedStrategy();
        int getStrategyComplexity();
        Result verify();
    protected:
        /// @brief Holds current model and formula.
        GlobalModelGenerator* generator;
        /// @brief Stack of decisions made.
        stack<DecisionEntry> history;
        /// @brief A table of actions paired vith globalState internal variables state for natural strategy construction.
        map<bitset<STRATEGY_BITS>, string, StrategyBitsComparator> naturalStrategy;
        /// @brief Max strategy variables found.
        short strategyVariableLimit;
        /// @brief Easily readable variable names for natural strategy generation.
        vector<string> variableNames;
        /// @brief Natural strategy complexity before reduction
        int reductionComplexityBefore;

        bool verifyLocalStates(vector<LocalState*>* localStates, GlobalState* globalState);
        bool isGlobalTransitionControlledByCoalition(GlobalTransition* globalTransition);
        bool isAgentInCoalition(Agent* agent);
        EpistemicClass* getEpistemicClassForGlobalState(GlobalState* globalState);
        bool areGlobalStatesInTheSameEpistemicClass(GlobalState* globalState1, GlobalState* globalState2);
        
        // todo: fix history
        // add and take away from stack the StateVerificationInfo
        void addHistoryDecision(GlobalState* globalState, GlobalTransition* decision);
        void addHistoryStateStatus(GlobalState* globalState, GlobalStateVerificationStatus prevStatus, GlobalStateVerificationStatus newStatus);
        bool addHistoryContext(GlobalState* globalState, GlobalTransition* decision, bool globalTransitionControlled);
        void addHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision);
        bool revertLastDecision();
        void undoLastHistoryEntry(bool freeMemory);
        void undoHistoryUntil(DecisionEntry* historyEntry, bool inclusive);
        
        bool equivalentGlobalTransitions(GlobalTransition* globalTransition1, GlobalTransition* globalTransition2);
        bitset<STRATEGY_BITS> globalStateToValueBits(GlobalState* globalState);
        vector<tuple<vector<tuple<bool, string>>, string>> reduceStrategy(vector<tuple<vector<tuple<bool, string>>, string>> strategyEntries, short lockedColumn = 0, bool upperHalf = false);
        bool testForAndFixBadAgents(StateVerificationInfo* stateVerificationInfo);
};

#endif // VERIFICATION_ITERATIVE
