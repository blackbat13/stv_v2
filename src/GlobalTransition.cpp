/**
 * @file GlobalTransition.cpp
 * @brief Struct representing a global transition.
 * Struct representing a global transition.
 */

#include "GlobalTransition.hpp"
#include "LocalTransition.hpp"

atomic_uint32_t GlobalTransition::next_id;

/// @brief Constructor for GlobalTransition.
GlobalTransition::GlobalTransition(){
   id = next_id++;
}

/// @brief Concatenates local transition names into a string.
/// @param sep Separator used for separating the transition names.
/// @return String of transition names joined with the given separator and ending on the separator.
string GlobalTransition::joinLocalTransitionNames(char sep){
   string res = "";
   for(const auto& trn: localTransitions){
      res += trn->name + sep;
   }
   return res;
}

float GlobalTransition::getProbability() {
   float prob = 1.0;
   for (LocalTransition* transition : localTransitions) {
      prob *= transition->probability;
   }
   return prob;
}