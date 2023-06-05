#ifndef EPISTEMICCLASS_H
#define EPISTEMICCLASS_H

#include "Common.hpp"

/// @brief Represents a single epistemic class.
struct EpistemicClass {
    /// @brief Hash of that epistemic class.
    string hash;

    /// @brief Map of GlobalState hashes to according GlobalState pointers bound to this epistemic class.
    map<string, GlobalState*> globalStates;
    // GlobalState->hash => GlobalState*

    /// @brief Transition that was already selected in this epistemic class. Model has to choose this transition if it is already set.
    GlobalTransition* fixedCoalitionTransition;
};

#endif // EPISTEMICCLASS_H