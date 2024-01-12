/**
 * @file GlobalState.hpp
 * @brief Struct representing a global state.
 * Struct representing a global state.
 */

#ifndef GLOBAL_STATE
#define GLOBAL_STATE

#include "Common.hpp"

/// @brief Represents a single global state.
struct GlobalState {
    GlobalState();
    // Data

    /// @brief Hash of the global state used in quick checks if the states are in the same epistemic class.
    string hash;

    /// @brief Map of agents and the epistemic classes that belongs to the respective agent.
    map<Agent*, EpistemicClass*> epistemicClasses;

    /// @brief If false, the state can be still expanded, potentially creating new states, otherwise the expansion of the state already occured and is not necessary.
    bool isExpanded;

    /// @brief Current verifivation status of this state.
    GlobalStateVerificationStatus verificationStatus;
    
    // Bindings

    /// @brief Every GlobalTransition in the model.
    set<GlobalTransition*> globalTransitions;

    /// @brief Local states of each agent that define this global state.
    vector<LocalState*> localStatesProjection;

    /// @brief Debug information on the given GlobalState
    /// @param indent - optional indentation string 
    /// @return GlobalState data
    std::string toString(string indent="");

    /// @brief Get for the environment of a given global state.
    /// @return A map of variable names and their values for the current global state.
    map<string, int> getGlobalStateEnvironment();
};

#endif