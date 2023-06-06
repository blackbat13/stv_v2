/**
 * @file Agent.cpp
 * @brief Class of an agent.
 * Class of an agent.
 */

#include "Agent.hpp"
#include "LocalState.hpp"

/* Sprawdzenie, czy w modelu nie ma juz (równoważnego) stanu.
 * Jeśli jest - zwróć go, w p.p. NULL
 */
/// @brief Checks if there is an equivalent LocalState in the model to the one passed as an argment.
/// @param state A pointer to LocalState to be checked.
/// @return Returns a pointer to an equivalent LocalState if such exists, otherwise returns NULL.
LocalState* Agent::includesState(LocalState* state) {
   // pętla po stanach już obecnych w modelu
   for(size_t i=0; i<localStates.size(); i++) {
      // jeśli jest zgodność, zwróć fałsz
      if(localStates[i]->compare(state)) return localStates[i];
   }
   return NULL;
}