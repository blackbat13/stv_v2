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
	//Create a new blank Agent
	Agent* a = new Agent(id, name);
	
	//Copy local states
	for(LocalState* l : localStates){
		LocalState* lc = new LocalState();
		lc->id = l->id;
		lc->name.assign(l->name);
		lc->environment = l->environment;
		lc->agent = a;
		a->localStates.push_back(lc);
	}
	a->initState = a->localStates[initState->id];

	//Copy variables
	for(Var* v : vars){
		Var *o = new Var;
		o->name.assign(""+v->name);
		o->initialValue = v->initialValue;
		o->persistent = v->persistent;
		o->agent = a;
		a->vars.insert(o);
	}

	//Copy local transitions
	for(int i=0; i<localTransitions.size(); i++){
		LocalTransition* t = new LocalTransition();
		t->id = localTransitions[i]->id;//i;
		t->name.assign(localTransitions[i]->name);
		t->localName.assign(localTransitions[i]->localName);
		t->isShared = localTransitions[i]->isShared;
		t->sharedCount = localTransitions[i]->sharedCount;

		//Copy condition data
		for(Condition* c : localTransitions[i]->conditions){
			Condition* cc;
			for(Var* v : a->vars){
				if(v->name == c->var->name){
					cc->var = v;
				}
			}
			/*cc->var->name = c->var->name;
			cc->var->initialValue = c->var->initialValue;
			cc->var->persistent = c->var->persistent;
			cc->var->agent = a;*/
			cc->conditionOperator = c->conditionOperator;
			cc->comparedValue = c->comparedValue;
			t->conditions.insert(cc);
		}

		//Link the transitions to the states in the agent
		t->agent = a;
		t->from = a->localStates[localTransitions[i]->from->id];
		t->to = a->localStates[localTransitions[i]->to->id];
		a->localTransitions.push_back(t);
		a->localStates[t->from->id]->localTransitions.insert(t);
	}

	//Return the finished Agent
	return a;
}
