/**
 * @file Common.hpp
 * @brief Contains all commonly used classes and structs.
 * Contains all commonly used classes and structs.
 */

#ifndef COMMON
#define COMMON

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
    std::string dotdir;           ///< pathprefix for .dot files export
    int model_id; // <-- this is temporary member (used in Verification.cpp for a hardcoded formula); has lower priority than `fname`
    bool add_epsilon_transitions; ///< add epsilon transitions to the states in the model when it's blocked for some reason
    bool formula_from_parameter;      ///< set if formula is provided directly via command-line, not from file
    std::string formula;              ///< the formula string to verify (from parameter, if specified)
    bool counterexample;              ///< output a counterexample if verification fails
    bool reduce;                      ///< enable state space reduction during model generation
    bool reduce_all;                  ///< apply reduction to all states (stronger reduction)
    std::string reduce_args;          ///< variable names for reduction (passed as argument)
    bool fixpoint;                    ///< use fixpoint-based verification algorithm
    bool natural_strategy;            ///< enable natural strategy mode (affects strategy reduction/printing)
    bool probability = false;         ///< enable probabilistic model checking
    bool verify_strategy;             ///< verify a strategy loaded from file
    std::string strategy_file_path;   ///< path to strategy file for verification
    bool partial_reduction = false;   ///< enable partial reduction (reduce only specified variables)
    std::vector<std::string> partial_reduction_args; ///< variable names for partial reduction
    std::string partial_reduction_agent; ///< agent name for partial reduction
    bool serialize_model = false;     ///< serialize the model to a file (for debugging or analysis)
    bool recommend_reduction_variables = false; ///< recommend variables for reduction based on analysis
    bool cone_of_influence = false;   ///< enable cone of influence analysis
    int cone_radius = 0;              ///< radius for cone of influence analysis
};

#endif 