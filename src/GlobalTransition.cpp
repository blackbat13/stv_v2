/**
 * @file GlobalTransition.cpp
 * @brief Struct representing a global transition.
 * Struct representing a global transition.
 */

#include "GlobalTransition.hpp"

atomic_uint32_t GlobalTransition::next_id;

GlobalTransition::GlobalTransition(){
   id = next_id++;
}