#define STRATEGY_BITS 64

#ifndef TYPES_DEPENDENCY
#define TYPES_DEPENDENCY

#include <bitset>

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
};

#endif // TYPES_DEPENDENCY