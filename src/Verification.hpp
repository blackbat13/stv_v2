#ifndef SELENE_VERIFICATION
#define SELENE_VERIFICATION

#include <stack>
#include "Types.hpp"
#include "GlobalModelGenerator.hpp"
#include "SeleneFormula.hpp"

string verStatusToStr(GlobalStateVerificationStatus status);

enum HistoryEntryType {
    DECISION,
    STATE_STATUS,
    CONTEXT,
    MARK_DECISION_AS_INVALID,
};
struct HistoryEntry {
    HistoryEntryType type;
    GlobalState* globalState;
    GlobalTransition* decision;
    bool globalTransitionControlled;
    GlobalStateVerificationStatus prevStatus;
    GlobalStateVerificationStatus newStatus;
    int depth;
    HistoryEntry* prev;
    HistoryEntry* next;
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

class HistoryDbg {
public:
    vector<pair<HistoryEntry*, char>> entries;
    HistoryDbg();
    ~HistoryDbg();
    void addEntry(HistoryEntry* entry);
    void markEntry(HistoryEntry* entry, char chr);
    void print(string prefix);
    HistoryEntry* cloneEntry(HistoryEntry* entry);
};

enum Mode {
    NORMAL,
    REVERT,
    RESTORE,
};

class Verification {
public:
    Verification(GlobalModelGenerator* generator);
    ~Verification();
    bool verify();
protected:
    Mode mode;
    GlobalState* revertToGlobalState;
    stack<HistoryEntry*> historyToRestore;
    GlobalModelGenerator* generator;
    SeleneFormula* seleneFormula;
    HistoryEntry* historyStart;
    HistoryEntry* historyEnd;
    bool verifyLocalStates(set<LocalState*>* localStates);
    bool verifyGlobalState(GlobalState* globalState, int depth);
    bool isGlobalTransitionControlledByCoalition(GlobalTransition* globalTransition);
    bool isAgentInCoalition(Agent* agent);
    EpistemicClass* getEpistemicClassForGlobalState(GlobalState* globalState);
    bool areGlobalStatesInTheSameEpistemicClass(GlobalState* globalState1, GlobalState* globalState2);
    void addHistoryDecision(GlobalState* globalState, GlobalTransition* ecision);
    void addHistoryStateStatus(GlobalState* globalState, GlobalStateVerificationStatus prevStatus, GlobalStateVerificationStatus newStatus);
    void addHistoryContext(GlobalState* globalState, int depth, GlobalTransition* decision, bool globalTransitionControlled);
    void addHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision);
    HistoryEntry* newHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision);
    bool revertLastDecision(int depth);
    void undoLastHistoryEntry(bool freeMemory);
    void undoHistoryUntil(HistoryEntry* historyEntry, bool inclusive, int depth);
    void printCurrentHistory(int depth);
};

#endif // SELENE_VERIFICATION
