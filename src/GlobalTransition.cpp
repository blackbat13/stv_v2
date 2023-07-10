/**
 * @file GlobalTransition.cpp
 * @brief Struct representing a global transition.
 * Struct representing a global transition.
 */

#include "GlobalTransition.hpp"
#include "LocalTransition.hpp"

atomic_uint32_t GlobalTransition::next_id;

GlobalTransition::GlobalTransition(){
   id = next_id++;
}

string GlobalTransition::joinLocalTransitionNames(char sep){
   string res = "";
   for(const auto& trn: localTransitions){
      res += trn->name + sep;
   }
   return res;
}