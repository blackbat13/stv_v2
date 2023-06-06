/**
 * @file GlobalModel.hpp
 * @brief Struct of a global model.
 * Struct of a global model.
 */

#ifndef GLOBALMODEL_H
#define GLOBALMODEL_H

#include "Common.hpp"

/// @brief Represents a global model, containing agents and a formula.
struct GlobalModel {
    // Data

    /// @brief Pointers to all agents in a model.
    vector<Agent*> agents;
    // agents[i].id == i

    /// @brief A pointer to a Formula.
    Formula* formula;
    
    // Bindings

    /// @brief Pointer to the initial state of the model.
    GlobalState* initState;

    /// @brief Every GlobalState in the model.
    vector<GlobalState*> globalStates;
    // globalStates[i].id == i

    /// @brief Map of Agent pointers to a map of EpistemicClass.
    map<Agent*, map<string, EpistemicClass*>> epistemicClasses;
    // Agent* => (EpistemicClass->hash => EpistemicClass*)
};


#endif // GLOBALMODEL_H