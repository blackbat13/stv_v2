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

/// @brief 
struct DecisionEntry {
    /// @brief Type of the history record.
    HistoryEntryType type;
    /// @brief Saved global state.
    GlobalState* globalState;
    /// @brief Selected transition.
    GlobalTransition* decision;
    /// @brief Is the transition controlled by an agent in coalition.
    bool globalTransitionControlled;
    /// @brief Previous model verification state.
    GlobalStateVerificationStatus prevStatus;
    /// @brief Next model verification state.
    GlobalStateVerificationStatus newStatus;
    /// @brief Holds currently processed strategy for the current state.
    StrategyEntry strategy;
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
        bool addHistoryContext(GlobalState* globalState, int depth, GlobalTransition* decision, bool globalTransitionControlled);
        void addHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision);
        void addHistoryUncontrolledDecision(GlobalState* globalState, GlobalTransition* decision);
        DecisionEntry* newHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision);
        bool revertLastDecision(int depth);
        void undoLastHistoryEntry(bool freeMemory);
        void undoHistoryUntil(DecisionEntry* historyEntry, bool inclusive, int depth);
        
        bool equivalentGlobalTransitions(GlobalTransition* globalTransition1, GlobalTransition* globalTransition2);
        bitset<STRATEGY_BITS> globalStateToValueBits(GlobalState* globalState);
        vector<tuple<vector<tuple<bool, string>>, string>> reduceStrategy(vector<tuple<vector<tuple<bool, string>>, string>> strategyEntries, short lockedColumn = 0, bool upperHalf = false);
};

#endif // VERIFICATION_ITERATIVE
