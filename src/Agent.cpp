/**
 * @file Agent.cpp
 * @brief Class of an agent.
 * Class of an agent.
 */

#include "Agent.hpp"
#include "LocalState.hpp"
#include "LocalTransition.hpp"
#include <iostream>

using namespace::std;

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
	Agent* a = new Agent(id, name);
	
	for(LocalState* l : localStates){
		LocalState* lc = new LocalState();
		lc->id = l->id;
		lc->name.assign(l->name);
		lc->environment = l->environment;
		lc->agent = a;
		a->localStates.push_back(lc);
	}
	a->initState = a->localStates[initState->id];
	//asm("INT3");
	//a->localTransitions.clear();
	for(int i=0; i<localTransitions.size(); i++){
		LocalTransition* t = new LocalTransition();
		t->id = i;
		t->name.assign(localTransitions[i]->name);
		t->localName.assign(localTransitions[i]->localName);
		t->isShared = localTransitions[i]->isShared;
		t->sharedCount = localTransitions[i]->sharedCount;
		for(Condition* c : localTransitions[i]->conditions){
			Condition* cc;
			cc->var->name = c->var->name;
			cc->var->initialValue = c->var->initialValue;
			cc->var->persistent = c->var->persistent;
			cc->var->agent = a;
			cc->conditionOperator = c->conditionOperator;
			cc->comparedValue = c->comparedValue;
			t->conditions.insert(cc);
		}
		t->agent = a;
		t->from = a->localStates[localTransitions[i]->from->id];
		t->to = a->localStates[localTransitions[i]->to->id];
		a->localTransitions.push_back(t);
		a->localStates[t->from->id]->localTransitions.insert(t);
	}
	return a;
}
