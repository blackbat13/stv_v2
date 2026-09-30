/**
 * @file Agent.cpp
 * @brief Class of an agent.
 * Class of an agent.
 */

#include "Agent.hpp"
#include "LocalState.hpp"
#include "LocalTransition.hpp"
#include "Types.hpp"

#include <unordered_map>
#include <unordered_set>
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

/// @brief Partially reduces the model by forgetting specified variables.
/// @param variableNames A vector of variable names to be forgotten.
void Agent::partialReduceModel(const vector<string>& variableNames) {
   if (variableNames.empty() || localStates.empty()) {
      return;
   }

   // Track forgotten variable names for fast membership checks.
   unordered_set<string> forgottenVariables(variableNames.begin(), variableNames.end());

   // Keep the agent variable registry consistent with reduced states.
   // After reduction these variables should no longer appear in LOCAL/PERSISTENT sections.
   for (auto it = vars.begin(); it != vars.end();) {
      if (forgottenVariables.find((*it)->name) != forgottenVariables.end()) {
         it = vars.erase(it);
      } else {
         ++it;
      }
   }

   // Forget selected variables in all local states.
   for (auto state : localStates) {
      for (const auto& variableName : variableNames) {
         state->environment.erase(variableName);
      }
      for (auto transition : state->localTransitions) {
         for (auto it = transition->conditions.begin(); it != transition->conditions.end();) {
            Condition* condition = *it;
            bool removeCondition = false;

            if (condition->expression != nullptr) {
               set<string> vars = condition->expression->getVariableNames();
               for (const auto& varName : vars) {
                  if (forgottenVariables.find(varName) != forgottenVariables.end()) {
                     removeCondition = true;
                     break;
                  }
               }
            } else if (condition->var != nullptr) {
               removeCondition = forgottenVariables.find(condition->var->name) != forgottenVariables.end();
            }

            if (removeCondition) {
               it = transition->conditions.erase(it);
            } else {
               ++it;
            }
         }
      }
   }

   // Build representative states for equivalent states after forgetting variables.
   unordered_map<string, LocalState*> representativeByKey;
   unordered_map<LocalState*, LocalState*> representativeOf;
   vector<LocalState*> reducedStates;
   reducedStates.reserve(localStates.size());

   // Map each state to its representative based on name and environment hash.
   for (auto state : localStates) {
      string key = state->name + "|" + state->environmentHash();
      auto it = representativeByKey.find(key);
      if (it == representativeByKey.end()) {
         representativeByKey[key] = state;
         representativeOf[state] = state;
         reducedStates.push_back(state);
      } else {
         representativeOf[state] = it->second;
      }
   }

   // Keep initState valid in the reduced model.
   if (representativeOf.find(initState) != representativeOf.end()) {
      initState = representativeOf[initState];
   }

   // Rebuild outgoing transition sets from scratch using merged states.
   for (auto state : localStates) {
      state->localTransitions.clear();
   }

   // Deduplicate transitions that become identical after state merging.
   unordered_set<string> seenTransitions;
   vector<LocalTransition*> reducedTransitions;
   reducedTransitions.reserve(localTransitions.size());

   for (auto transition : localTransitions) {
      // Skip transitions that point to states that were not mapped.
      auto fromIt = representativeOf.find(transition->from);
      auto toIt = representativeOf.find(transition->to);
      if (fromIt == representativeOf.end() || toIt == representativeOf.end()) {
         continue;
      }

      // Redirect transition endpoints to representative states.
      transition->from = fromIt->second;
      transition->to = toIt->second;

      // Signature used for duplicate detection after reduction.
      string transitionKey = transition->from->name + "|" + to_string(transition->from->id) + "|" +
                             transition->to->name + "|" + to_string(transition->to->id) + "|" +
                             transition->name + "|" + transition->localName + "|" +
                             to_string(static_cast<int>(transition->isShared)) + "|" +
                             to_string(transition->sharedCount) + "|" +
                             to_string(transition->probability);

      if (!seenTransitions.insert(transitionKey).second) {
         continue;
      }

      reducedTransitions.push_back(transition);
      transition->from->localTransitions.insert(transition);
   }

   // Reassign contiguous IDs for consistency with existing assumptions.
   for (size_t i = 0; i < reducedStates.size(); ++i) {
      reducedStates[i]->id = static_cast<uint32_t>(i);
   }

   for (size_t i = 0; i < reducedTransitions.size(); ++i) {
      reducedTransitions[i]->id = static_cast<uint32_t>(i);
   }

   // Replace original storage with reduced state/transition collections.
   localStates = move(reducedStates);
   localTransitions = move(reducedTransitions);
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
