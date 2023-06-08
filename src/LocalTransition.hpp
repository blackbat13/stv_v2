#ifndef LOCALTRANSITION_H
#define LOCALTRANSITION_H

#include "Common.hpp"

/// @brief Represents a single local transition, containing id, global name, local name, is shared and count of the appearances.
struct LocalTransition {
    // Data

    /// @brief Identifier of the transition.
    int id;

    /// @brief Name of the transition (global).
    string name;

    /// @brief Name of the transition (local).
    string localName;
    // if empty => same as name

    /// @brief Is the transition appearing somewhere else, true if yes, false if no.
    bool isShared;

    /// @brief Count of recurring appearances of this transition.
    int sharedCount;

    /// @brief Conditions that have to be fulfilled for the transition to be avaliable.
    set<Condition*> conditions;
    
    // Bindings

    /// @brief Binding to an Agent.
    Agent* agent;

    /// @brief Binding to a LocalState from which this transition goes from.
    LocalState* from;

    /// @brief Binding to a LocalState from which this transition goes to.
    LocalState* to;
};


#endif // LOCALTRANSITION_H