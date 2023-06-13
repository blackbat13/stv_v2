/**
 * @file GlobalState.cpp
 * @brief Struct representing a global state.
 * Struct representing a global state.
 */

#include "GlobalState.hpp"
#include "LocalState.hpp"

// atomic_uint32_t GlobalState::next_id;

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
