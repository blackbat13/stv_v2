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
    char* fname;
    int stv_mode;
    bool output_local_models;
    bool output_global_model;
    bool output_dot_files;
    int model_id; // <-- this is temporary member (used in Verification.cpp for a hardcoded formula)
    std::string dotdir;
};

#endif 