/**
 * @file GlobalState.cpp
 * @brief Struct representing a global state.
 * Struct representing a global state.
 */

#include "GlobalState.hpp"
#include "LocalState.hpp"

GlobalState::GlobalState(){
   isExpanded = false;
   verificationStatus = GlobalStateVerificationStatus::UNVERIFIED;
}

/// @brief Debug information on the given GlobalState
/// @return GlobalState data
std::string GlobalState::toString(string indent){
   string res = 
   indent + "hash:\"" + this->hash + "\",\n" +
   indent + "localStates:[";
   for(const auto &l : localStatesProjection){
      res += "\n" + l->toString(indent + "\t") + ",";
   }
   res.pop_back();
   res+="\n" + indent + "]";
   return res;
}

/// @brief Get for the environment of a given global state.
/// @return A map of variable names and their values for the current global state.
map<string, int> GlobalState::getGlobalStateEnvironment() {
   map<string, int> currEnv;

   for (const auto localState : localStatesProjection) {
      for(auto it = localState->environment.begin(); it!=localState->environment.end(); ++it){
            currEnv[it->first] = it->second;
      }
   }

   return currEnv;
}
