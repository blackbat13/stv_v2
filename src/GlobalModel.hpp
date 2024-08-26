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

    ~GlobalModel(){
        //delete initState;
        delete formula;
        for(auto item : globalStates) {
            delete item;
        }
        for(auto item : epistemicClasses) {
            for(auto const& [key, val] : item.second) {
                delete val;
            }
        }
    }

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

    /// @brief Map of Agent pointers to a map of EpistemicClass for graph traversal.
    map<Agent*, map<string, EpistemicClass*>> epistemicClasses;

    /// @brief Map of Agent pointers to a map of EpistemicClass for knowledge checks.
    map<Agent*, map<string, set<GlobalState*>>> epistemicClassesKnowledge;
    // Agent* => (EpistemicClass->hash => EpistemicClass*)
};


#endif // GLOBALMODEL_H