#ifndef SELENE_TYPES
#define SELENE_TYPES

#include <map>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

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

enum ConditionOperator {
    Equals,
    NotEquals,
};

enum GlobalStateVerificationStatus {
    UNVERIFIED,
    PENDING,
    VERIFIED_OK,
    VERIFIED_ERR,
};

enum VarAssignmentType {
    FromVar,
    FromValue,
};

/// @brief Represents variable in the model
struct Var {
    /// @brief Variable name
    string name;

    /// @brief Initial value of the variable
    int initialValue;

    /// @brief Variable current value
    int currentValue;

    /// @brief True if variable is persistent, i.e. it should appear in all states in the model, false otherwise
    bool persistent;

    /// @brief Reference to an agent, to which this variable belongs to
    Agent *agent;
};

/// @brief Represents condition
struct Condition {
    Var* var;
    ConditionOperator conditionOperator;
    int comparedValue;
};

struct Formula {
    set<Agent*> coalition;
};

class Agent {
    public:
        int id;
        string name;
        set<Var*> vars;
        Agent(int _id, string _name):id(_id), name(_name) {};
        
        LocalState* initState;
        vector<LocalState*> localStates; // localStates[i].id == i
        vector<LocalTransition*> localTransitions; // localTransitions[i].id == i
        
        // sprawdź, czy stan nie został już wygenerowany
        LocalState* includesState(LocalState *state);
};

class LocalState {
    public:
        // Data
        int id;
        string name;
        map<Var*, int> vars;
        // alternatywna wersja - może wystarczy
        map<string, int> environment;
     
        // komparator
        bool compare(LocalState *state);
        
        // Bindings
        Agent* agent;
        set<LocalTransition*> localTransitions;
};

// to jest zbędne
struct VarAssignment {
    Var* dstVar;
    VarAssignmentType type; // zbędne
    Var* srcVar;
    int value;
};

struct LocalTransition {
    // Data
    int id;
    string name;
    string localName; // if empty => same as name
    bool isShared;
    int sharedCount;
    set<Condition*> conditions;
    set<VarAssignment*> varAsssignments;
    
    // Bindings
    Agent* agent;
    LocalState* from;
    LocalState* to;
    set<LocalTransition*> sharedLocalTransitions;
};

struct LocalModels {
    vector<Agent*> agents; // agents[i].id == i
    map<string, Var*> vars; // vars[str].name == str
};

struct GlobalModel {
    // Data
    vector<Agent*> agents; // agents[i].id == i
    Formula* formula;
    
    // Bindings
    GlobalState* initState;
    vector<GlobalState*> globalStates; // globalStates[i].id == i
    vector<GlobalTransition*> globalTransitions; // globalTransitions[i].id == i
    map<Agent*, map<string, EpistemicClass*>> epistemicClasses; // Agent* => (EpistemicClass->hash => EpistemicClass*)
};

struct GlobalState {
    // Data
    int id;
    string hash;
    map<Var*, int> vars;
    map<Agent*, EpistemicClass*> epistemicClasses;
    bool isExpanded;
    GlobalStateVerificationStatus verificationStatus;
    
    // Bindings
    set<GlobalTransition*> globalTransitions;
    set<LocalState*> localStates;
};

struct GlobalTransition {
    // Data
    int id;
    bool isInvalidDecision;
    
    // Bindings
    GlobalState* from;
    GlobalState* to;
    set<LocalTransition*> localTransitions;
};

struct EpistemicClass {
    string hash;
    map<string, GlobalState*> globalStates; // GlobalState->hash => GlobalState*
    GlobalTransition* fixedCoalitionTransition;
};

#endif // SELENE_TYPES
