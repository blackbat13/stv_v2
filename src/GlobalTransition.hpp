/**
 * @file GlobalTransition.hpp
 * @brief Struct representing a global transition.
 * Struct representing a global transition.
 */

#ifndef GLOBALTRANSITION_H
#define GLOBALTRANSITION_H

#include "Common.hpp"

/// @brief Represents a single global transition.
struct GlobalTransition {
    static atomic_uint32_t next_id; // [YK]: ideally it should be a private member (and possibly of a type size_t), but for now this should work just as well
    GlobalTransition();
    // Data

    /// @brief Identifier of the transition.
    uint32_t id;

    /// @brief Marks if the transition is invalid, true if there is no point in traversing that transition, otherwise false.
    bool isInvalidDecision;
    
    // Bindings

    /// @brief Binding to a GlobalState from which this transition goes from.
    GlobalState* from;

    /// @brief Binding to a GlobalState from which this transition goes to.
    GlobalState* to;

    /// @brief Local transitions that define this global transition. A single transition or more in case of shared transitions.
    set<LocalTransition*> localTransitions;
};

#endif // GLOBALTRANSITION_H