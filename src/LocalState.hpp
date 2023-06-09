/**
 * @file LocalState.hpp
 * @brief Class representing a local state.
 * Class representing a local state.
 */

#ifndef LOCALSTATE_H
#define LOCALSTATE_H

#include "Common.hpp"

/// @brief Represents a single LocalState, containing id, name and internal variables.
class LocalState {
    public:
        // Data

        /// @brief State identifier.
        uint32_t id;

        /// @brief State name.
        string name;

        /// @brief Local variables as a name and their current values.
        map<string, int> environment; 
     
        // komparator
        
        bool compare(LocalState *state);
        
        // Bindings

        /// @brief Binding to an Agent.
        Agent* agent;

        /// @brief Binding to the set of LocalTransition.
        set<LocalTransition*> localTransitions;

        /// @brief Debug information on the given LocalState
        /// @param indent - optional indentation string 
        /// @return LocalState data
        string toString(string indent="");
};

#endif // LOCALSTATE_H