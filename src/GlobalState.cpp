/**
 * @file GlobalState.cpp
 * @brief Struct representing a global state.
 * Struct representing a global state.
 */

#include "GlobalState.hpp"

atomic_uint32_t GlobalState::next_id;

GlobalState::GlobalState(){
   id = next_id++;
   isExpanded = false;
   verificationStatus = GlobalStateVerificationStatus::UNVERIFIED;
}
