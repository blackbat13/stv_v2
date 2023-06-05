#ifndef GLOBAL_STATE
#define GLOBAL_STATE

#include "Common.hpp"

/// @brief Represents a single global state.
struct GlobalState {
    static atomic_uint32_t next_id; // [YK]: ideally it should be a private member (and possibly of a type size_t), but for now this should work just as well
    GlobalState();
    // Data
    
    /// @brief Identifier of the global state.
    uint32_t id;

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
    set<LocalState*> localStates;
};

#endif