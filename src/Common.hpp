/**
 * @file Common.hpp
 * @brief Contains all commonly used classes and structs.
 * Contains all commonly used classes and structs.
 */

#ifndef COMMON
#define COMMON

#define EPSILON "&epsilon;"//"STV KBC RESERVED CONST 'Epsilon'"

#include <map>
#include <set>

#include <stack>
#include <string>
#include <utility>
#include <vector>
#include <atomic> //std::atomic_uint32_t

#include "reader/expressions.hpp"

#include "enums/GlobalStateVerificationStatus.hpp"
#include "enums/ConditionOperator.hpp"

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

struct Cfg{
    std::string fname;            ///< path to input file with system specification
    int stv_mode;                 ///< stv_code as sum/combination of (1 - expandAllStates, 2 - verify, 4 - print metadata, 8 - run experiments)
    bool output_local_models;     ///< (obsolete) print data on local model
    bool output_global_model;     ///< (obsolete) print data on local model
    bool output_dot_files;        ///< flag for .dot export (by default exports templates and local/global models)
    bool kbc;					  ///< flag for knowledge-based construction
    bool kbc_coalition_only;      ///< flag for targeting knowledge-based construction at agents in the formula coalition only; if unset, all agents are selected
    std::string dotdir;           ///< pathprefix for .dot files export
    int model_id; // <-- this is temporary member (used in Verification.cpp for a hardcoded formula); has lower priority than `fname`
    bool add_epsilon_transitions; ///< add epsilon transitions to the states in the model when it's blocked for some reason
    bool formula_from_parameter;
    std::string formula;
    bool counterexample;
};

#endif 
