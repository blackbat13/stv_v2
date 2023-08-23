/**
 * @file Agent.cpp
 * @brief Class of an agent.
 * Class of an agent.
 */

#include "Agent.hpp"
#include "LocalState.hpp"
#include "LocalTransition.hpp"

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

Agent* Agent::clone(){
	Agent* a = new Agent(*this);
	for(int i=0; i<a->localStates.size(); i++){
		a->localStates[i] = new LocalState(*a->localStates[i]);
		a->localStates[i]->agent = a;
	}
	for(int i=0; i<a->localTransitions.size(); i++){
		LocalTransition t = *this->localTransitions[i];
		a->localTransitions[i] = &t;
		a->localTransitions[i]->agent = a;
		a->localTransitions[i]->from = a->localStates[a->localTransitions[i]->from->id];
		a->localTransitions[i]->to = a->localStates[a->localTransitions[i]->to->id];
	}
	for(int i=0; i<a->localStates.size(); i++){
		vector<LocalTransition*> lsltset(a->localStates[i]->localTransitions.begin(), a->localStates[i]->localTransitions.end());
		a->localStates[i]->localTransitions.clear();
		for(int j=0; j<lsltset.size(); j++){
			LocalTransition tls = *a->localTransitions[lsltset[j]->id];
			a->localStates[i]->localTransitions.insert(&tls);
		}
	}
	
	return a;
}
