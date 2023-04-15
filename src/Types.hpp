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
#include "reader/expressions.hpp"

using namespace std;


class Agent;
class LocalState;

struct Condition;
struct EpistemicClass;
struct Formula;
struct GlobalModel;
struct GlobalState;
struct GlobalTransition;
struct LocalTransition;
struct LocalModels;
struct Var;
struct VarAssignment;

/// @brief Conditional operator for the variable.
enum ConditionOperator {
    Equals, ///< Variable should be equal to the value.
    NotEquals, ///< Variable should be not equal to the value.
};

/// @brief Verification status of a GlobalState.
enum GlobalStateVerificationStatus {
    UNVERIFIED, ///< State is not verified.
    PENDING, ///< Entered the state but it is not verified as correct or incorrect yet.
    VERIFIED_OK, ///< The state has been verified and is correct.
    VERIFIED_ERR, /// < The state has been verified and is incorrect.
};

/// @brief Handles if the Var value is from srcVar or from value.
enum VarAssignmentType {
    FromVar, ///< Take value from srcVar.
    FromValue, ///< Take value from value.
};

/// @brief Represents a variable in the model, containing name, initial value and persistence.
struct Var {
    /// @brief Variable name.
    string name;

    /// @brief Initial value of the variable.
    int initialValue;

    /// @brief True if variable is persistent, i.e. it should appear in all states in the model, false otherwise.
    bool persistent;

    /// @brief Reference to an agent, to which this variable belongs to.
    Agent *agent;
};

/// @brief Represents a condition for LocalTransition.
struct Condition {
    /// @brief Pointer to a variable.
    Var* var;

    /// @brief Conditional operator for the variable.
    ConditionOperator conditionOperator;

    /// @brief Condition value to be met.
    int comparedValue;
};

// [YK]: template, as here coalition is just a string

/// @brief Contains a template for coalition of Agent as string from the formula.
struct FormulaTemplate{
   set<string>* coalition;
   ExprNode* formula;
};

struct Formula {
    /// @brief Coalition of Agent from the formula.
    set<Agent*> coalition;
    ExprNode* p; // [YK]: temporary solution to encode <<coalution>> G p
};

/// @brief Contains all data for a single Agent, including id, name and all of the agents' variables.
class Agent {
    public:
        /// @brief Identifier of the agent.
        int id;

        /// @brief Name of the agent.
        string name;

        /// @brief Variable names for the agent.
        set<Var*> vars;

        /// @brief Constructor for the Agent class, assigning it an id and name.
        /// @param _id Identifier of the new agent.
        /// @param _name Name of the new agent.
        Agent(int _id, string _name):id(_id), name(_name) {};
        
        /// @brief Initial state of the agent.
        LocalState* initState;

        /// @brief Local states for this agent.
        vector<LocalState*> localStates; // localStates[i].id == i

        /// @brief Local transitions for this agent.
        vector<LocalTransition*> localTransitions; // localTransitions[i].id == i
        
        // sprawdź, czy stan nie został już wygenerowany.
        
        LocalState* includesState(LocalState *state);
};

/// @brief Represents a single LocalState, containing id, name and internal variables.
class LocalState {
    public:
        // Data

        /// @brief State identifier.
        int id;

        /// @brief State name.
        string name;

        /// @brief Local variables and their values.
        map<Var*, int> vars;

        // alternatywna wersja - może wystarczy

        /// @brief Local variables as a name and their current values.
        map<string, int> environment; 
     
        // komparator
        
        bool compare(LocalState *state);
        
        // Bindings

        /// @brief Binding to an Agent.
        Agent* agent;

        /// @brief Binding to the set of LocalTransition.
        set<LocalTransition*> localTransitions;
};

// to jest zbędne
struct VarAssignment {
    Var* dstVar;
    VarAssignmentType type; // zbędne
    Var* srcVar;
    int value;
};

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

    /// @brief Values to be set as a result of the traversal.
    set<VarAssignment*> varAsssignments;
    
    // Bindings

    /// @brief Binding to an Agent.
    Agent* agent;

    /// @brief Binding to a LocalState from which this transition goes from.
    LocalState* from;

    /// @brief Binding to a LocalState from which this transition goes to.
    LocalState* to;

    /// @brief Stores shared transitions from different models.
    set<LocalTransition*> sharedLocalTransitions;
};

/// @brief Represents a single local model, contains all agents and variables.
struct LocalModels {
    /// @brief A vector of agents for the current model.
    vector<Agent*> agents;
    // agents[i].id == i

    /// @brief A map of variable names to Var.
    map<string, Var*> vars;
    // vars[str].name == str
};

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

    /// @brief Every GlobalTransition in the model.
    vector<GlobalTransition*> globalTransitions;
    // globalTransitions[i].id == i

    /// @brief Map of Agent pointers to a map of EpistemicClass.
    map<Agent*, map<string, EpistemicClass*>> epistemicClasses;
    // Agent* => (EpistemicClass->hash => EpistemicClass*)
};

/// @brief Represents a single global state.
struct GlobalState {
    // Data

    /// @brief Identifier of the global state.
    int id;

    /// @brief Hash of the global state used in quick checks if the states are in the same epistemic class.
    string hash;

    /// @brief Map of model variables and their current values.
    map<Var*, int> vars;

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

/// @brief Represents a single global transition.
struct GlobalTransition {
    // Data

    /// @brief Identifier of the transition.
    int id;

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

#endif // SELENE_TYPES
