/**
 * @file Types.hpp
 * @brief Custom data structures.
 * Data structures and classes containing model data.
 */

#ifndef SELENE_TYPES
#define SELENE_TYPES

#include <map>
#include <set>

#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "LocalState.hpp"
#include "LocalTransition.hpp"

#include "GlobalState.hpp"
#include "GlobalTransition.hpp"
#include "GlobalModel.hpp"

#include "EpistemicClass.hpp"
#include "Agent.hpp"

#include "reader/expressions.hpp"

#include <atomic> //std::atomic_uint32_t

using namespace std;



/// @brief Represents a variable in the model, containing name, initial value and persistence.
struct Var {
    /// @brief Variable name.
    string name;

    /// @brief Initial value of the variable.
    int initialValue;

    /// @brief True if variable is persistent, i.e. it should appear in all states in the model, false otherwise.
    bool persistent;

    /// @brief Reference to an agent, to which this variable belongs to.
    shared_ptr<Agent> agent;
};

/// @brief Represents a condition for LocalTransition.
struct Condition {
    /// @brief Pointer to a variable.
    shared_ptr<Var> var;

    /// @brief Conditional operator for the variable.
    ConditionOperator conditionOperator;

    /// @brief Condition value to be met.
    int comparedValue;
};

/// @brief Contains a template for coalition of Agent as string from the formula. 
struct FormulaTemplate{
   shared_ptr<set<string>> coalition; // this will be replaced by an Agent pointer upon instantiation of a formula
   shared_ptr<ExprNode> formula;
};

struct Formula {
    /// @brief Coalition of Agent from the formula.
    set<shared_ptr<Agent>> coalition;
    shared_ptr<ExprNode> p; // [YK]: temporary solution to encode <<coalution>> G p
};

/// @brief Represents a single local model, contains all agents and variables.
struct LocalModels {
    /// @brief A vector of agents for the current model.
    vector<shared_ptr<Agent>> agents;
    // agents[i].id == i
};


#endif // SELENE_TYPES
