#define STRATEGY_BITS 64

#ifndef TYPES_DEPENDENCY
#define TYPES_DEPENDENCY

#include <bitset>
#include <queue>

using namespace std;

enum VerifResult {
    NOT_VERIFIED = 0,
    TRUE = 1,
    FALSE = 2
};

/// @brief StrategyEntry entry type.
enum StrategyEntryType {
    NOT_MODIFIED, ///< Entry didn't have to get modified.
    ADDED, ///< Entry got added to the map.
};

struct StrategyEntry {
    StrategyEntryType type = NOT_MODIFIED;
    bitset<STRATEGY_BITS> globalValues = 0;
    string* actionName;
};

/// @brief A comparator for two bitsets containing values for the global states.
struct StrategyBitsComparator {
    bool operator() (const bitset<STRATEGY_BITS> &b1, const bitset<STRATEGY_BITS> &b2) const {
        return b1.to_ulong() < b2.to_ulong();
    }
};

/// @brief HistoryEntry entry type.
enum HistoryEntryType {
    DECISION, ///< Made the decision to go to a state using a transition.
    STATE_STATUS, ///< Changed verification status.
    CONTEXT, ///< Recursion has gone deeper.
    MARK_DECISION_AS_INVALID, ///< Marking a transition as invalid.
    UNCONTROLLED_DECISION, ///< One uncontrolled choice from a set, from which all of them has to be OK.
    PROBABILITY, ///< There's a change in probability.
    ANSWER_PROBABILITY ///< There's a change in the answer probability.
};

enum ProbabilityCalculationType {
    SUM_PROBABILITY,
    MIN_PROBABILITY,
    NONE_PROBABILITY
};

struct ProbabilityTrueFalse {
    float probabilityTrue = 0.0;
    float probabilityFalse = 0.0;
};

class ProbabilityEntry {
    public:
        ProbabilityEntry() {} ;
        ProbabilityEntry(ProbabilityCalculationType mode) { probabilityCalculationType = mode; };
        ~ProbabilityEntry() {};
        void changeVerificationMode(ProbabilityCalculationType mode) { probabilityCalculationType = mode; }
        ProbabilityCalculationType getVerificationMode() { return probabilityCalculationType; }
        void changeSumProbabilityTrue(float changeProbabilityBy) { sumProbability.probabilityTrue += changeProbabilityBy; }
        void changeSumProbabilityFalse(float changeProbabilityBy) { sumProbability.probabilityFalse += changeProbabilityBy; }
        ProbabilityTrueFalse getSumProbability() { return sumProbability; }
        void setSumProbability(ProbabilityTrueFalse newSumProbability ) { sumProbability = newSumProbability; }
        void changeMinProbabilityTrue(float newProbability) { (minProbability.probabilityTrue > newProbability ? minProbability.probabilityTrue = newProbability : 0); }
        void changeMinProbabilityFalse(float newProbability) { (minProbability.probabilityFalse < newProbability ? minProbability.probabilityFalse = newProbability : 0); }
        ProbabilityTrueFalse getMinProbability() { return minProbability; }
        void setMinProbability(ProbabilityTrueFalse newMinProbability ) { minProbability = newMinProbability; }
        ProbabilityTrueFalse returnProbability() { return (probabilityCalculationType == ProbabilityCalculationType::MIN_PROBABILITY ? minProbability : sumProbability); }
    private:
        ProbabilityCalculationType probabilityCalculationType = ProbabilityCalculationType::SUM_PROBABILITY;
        ProbabilityTrueFalse sumProbability = {0.0, 0.0};
        ProbabilityTrueFalse minProbability = {1.0, 0.0};
};

struct StateVerificationInfo {
    GlobalState* globalState = nullptr;
    StateVerificationInfo* fromState = nullptr;
    queue<GlobalTransition*> controlledTransitionsLeftToProcess;
    queue<GlobalTransition*> uncontrolledTransitionsLeftToProcess;
    map<string, set<GlobalTransition*>> controlledProbabilisticTransitionsLeftToProcess;
    map<string, set<GlobalTransition*>> uncontrolledProbabilisticTransitionsLeftToProcess;
    int depth = 0;
    bool processed = false;
    VerifResult verifResult = VerifResult::NOT_VERIFIED;
    bool controlled = false;
    bool uncontrolled = false;
    bool hasControlledProbabilistic = false;
    bool hasUncontrolledProbabilistic = false;
    bool hasValidControlledTransition = false;
    bool hasValidUncontrolledTransition = true;
    bool isControlledByCoalition = false;
    bool gotThroughPreselectedTransition = false;
    bool gotResponseFromOtherState = false;
};

#endif // TYPES_DEPENDENCY