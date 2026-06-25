/**
 * @file nodes.cc
 * @brief Parser templates.
 * Class for setting up a new objects from a parser.
 */

#include "expressions.hpp"
#include "nodes.hpp"
#include <queue>
#include <fstream>
#include <cstring>


/* --------------------------------------------------------------------- */
// konstruktor bezparametrowy

/// @brief Constructor for an AgentTemplate.
AgentTemplate::AgentTemplate() {
   probabilityCache = 0.0;
   ident="";
   initState="";
   localVars = new set<string>;
   persistentVars = new set<string>;
   initialAssignments=new set<Assignment*>;
   transitions=new set<TransitionTemplate*>;
}
      
/* --------------------------------------------------------------------- */
// ustaw identyfikator agenta

/// @brief Sets the identifier of an agent.
/// @param _ident String with a new identifier.
/// @return Returns itself.
AgentTemplate& AgentTemplate::setIdent(string _ident) {
   ident = _ident;
   return *this;
}

/* --------------------------------------------------------------------- */
// ustaw identyfikator agenta

/// @brief Sets initial state of an agent.
/// @param _initState String with a new state.
/// @return Returns itself.
AgentTemplate& AgentTemplate::setInitState(string _initState) {
   initState = _initState;
   return *this;
}

/* --------------------------------------------------------------------- */
// dodaj zmienną/zmienne lokalne

/// @brief Adds local variables to the agent.
/// @param variables A pointer to a set of strings with the variables to be added.
/// @return Returns itself.
AgentTemplate& AgentTemplate::addLocal(set<string> *variables){
   if(variables != NULL) {
      // pętla po zawartości nowego zbioru
      for(set<string>::iterator it=variables->begin(); it != variables->end(); it++) {
         // każdą kolejną 
         localVars->insert(*it);
      }
   }
   return *this;
}

/* --------------------------------------------------------------------- */
// dodaj zmienne trwałe

/// @brief Adds persistent variables to the agent.
/// @param variables A pointer to a set of strings with the variables to be added.
/// @return Returns itself.
AgentTemplate& AgentTemplate::addPersistent(set<string> *variables) {
   if(variables != NULL) {
      // pętla po zawartości nowego zbioru
      for(set<string>::iterator it=variables->begin(); it != variables->end(); it++) {
         // każdą kolejną 
         persistentVars->insert(*it);
      }
   }
   return *this;
}

/* --------------------------------------------------------------------- */
// dodaj początkowe inicjacje

/// @brief Sets initial values of agent's variables.
/// @param assigns Set of variables to assign.
/// @return Returns itself.
AgentTemplate& AgentTemplate::addInitial(set<Assignment*> *assigns) {
   if(assigns != NULL) {
      // pętla po zawartości nowego zbioru
      for(set<Assignment*>::iterator it=assigns->begin(); it != assigns->end(); it++) {
         // każdą kolejną 
         initialAssignments->insert(*it);
      }
   }
   return *this;
}

/* --------------------------------------------------------------------- */
// dodaj tranzycję

/// @brief Adds a transition for the agent.
/// @param _transition Transition to be added.
/// @return Returns itself.
AgentTemplate& AgentTemplate::addTransition(TransitionTemplate *_transition) {
   if (strcmp(transitionCache.patternName.c_str(), "") == 0 || (strcmp(transitionCache.patternName.c_str(), _transition->patternName.c_str()) != 0 && strcmp(_transition->patternName.c_str(), "") != 0)) { 
      probabilityCache = 1.0;
      transitionCache = TransitionTemplate(*_transition);
   } else if (strcmp(_transition->patternName.c_str(), "") == 0) {
      _transition->shared = transitionCache.shared;
      _transition->patternName = transitionCache.patternName;
      _transition->matchName = transitionCache.matchName;
      _transition->startState = transitionCache.startState;
      _transition->condition = transitionCache.condition;
   }
   if (_transition->probability->eval() > 999.0) {
      _transition->probability = new ProbConst(probabilityCache);
   } else {
      probabilityCache -= _transition->probability->eval();
   }
   
   if (config.recommend_reduction_variables == true) {
      set<string> assignmentVars = getAssignmentVars(_transition);
      set<string> conditionVars = getConditionVars(_transition);

      for (auto assVar : assignmentVars) {
         if (recommendedReductionVariableCounts.count(assVar) == 0) {
            recommendedReductionVariableCounts[assVar] = map<string, int>();
         }
         for (auto condVar : conditionVars) {
            if (recommendedReductionVariableCounts[assVar].count(condVar) == 0) {
               recommendedReductionVariableCounts[assVar][condVar] = 0;
            }
            recommendedReductionVariableCounts[assVar][condVar]++;
         }
      }
   }

   transitions->insert(_transition);

   /* 
   [YK]: 
    - moved from `AgentTemplate::generateAgent`;
    - `AgentTemplate` does not exhibit delete, therefore `localStateTemplates` 
      can be computed incrementally (without refresh/update);
    - ideally localStateTemplates should be filled on demand, i.e 
      upon an instantiation of a new TransitionTemplate (provided parent Agent context);
      however, the performance difference would likely be insignificant and
      current "fix" should be sufficient
   */ 
  
   // Z tranzycji trzeba teraz wyciągnąć template'y stanów lokalnych

   // pomocnicze wskaźniki na stan początkowy i końcowy
   LocalStateTemplate *fromState, *toState;
   
   // sprawdź nazwę stanu początkowego, czy już jest w bazie
   if(localStateTemplates.count(_transition->startState) == 0) {
      // nie ma - utwórz obiekt stanu
      fromState = new LocalStateTemplate;
      fromState->name = _transition->startState;
      localStateTemplates[_transition->startState] = fromState;
   } else {
      fromState = localStateTemplates[_transition->startState];
   }
   // zapamietaj, że tranzycja "przynależy" do tego stanu
   fromState->transitions.insert(_transition);
   
   // to samo dla stanu końcowego
   if(localStateTemplates.count(_transition->endState) == 0) {
      // nie ma - utwórz obiekt stanu
      toState = new LocalStateTemplate;
      toState->name = _transition->endState;
      localStateTemplates[_transition->endState] = toState;
   }

   return *this;
}

/* --------------------------------------------------------------------- */
// wygeneruj kolejny stan zależnie od bieżącego stanu i schematu tranzycji

/// @brief Creates a new state and connects it to existing graph.
/// @param state Current local state.
/// @param trans Transitions from a local state to 
/// @return Returns a pointer to a newly created LocalState.
LocalState * AgentTemplate::genNextState(LocalState* state, TransitionTemplate* trans) {
   // utwórz obiekt dla nowego stanu
   LocalState *newState = new LocalState;
   newState->name = trans->endState;
   newState->agent = state->agent;
   // najpierw przepisz zmienne stałe
   for(set<string>::iterator it=persistentVars->begin(); it != persistentVars->end(); it++) {
      // wartości powinny być w środowisku, inaczej coś jest nie halo
      newState->environment[*it] = state->environment[*it];
   }
   // teraz wykonaj przypisania
   if (trans->assignments != nullptr) {
        for(set<Assignment*>::iterator it=trans->assignments->begin(); it != trans->assignments->end(); it++) {
            newState->environment[(*it)->ident] = (*it)->value->eval(state->environment);
        }
   }
   // zmienne ulotne automatycznie wyparowały
   // zwróć nowy stan
   return newState;
}

/* --------------------------------------------------------------------- */
// wygeneruj agenta do modelu

/// @brief Generates an agent for the model.
/// @param id Identifier of the new Agent.
/// @return Returns a pointer to a newly created Agent.
Agent * AgentTemplate::generateAgent(int id) {
   // zaalokuj obiekt będący wynikiem
   Agent *result = new Agent(id, ident);
   
   // utwórz zbiór zmiennych - aczkolwiek to wydaje mi się zbędne
   for(set<string>::iterator it=localVars->begin(); it != localVars->end(); it++) {
      Var *var = new Var;
      var->agent = result;
      var->name = *it;
      var->persistent = false;
      result->vars.insert(var);
   }
   for(set<string>::iterator it=persistentVars->begin(); it != persistentVars->end(); it++) {
      Var *var = new Var;
      var->agent = result;
      var->name = *it;
      var->persistent = true;
      result->vars.insert(var);
   }
     
   // na podstawie nazwy utwórz stan startowy
   result->initState = new LocalState;
   result->initState->id = 0;
   result->initState->name = initState;
   // result->localStates[0] = result->initState;
   result->localStates.push_back(result->initState);
   result->initState->agent = result;
   // wylicz mu wartości zmiennych na podstawie inicjalnego przypisania
   for(set<Assignment*>::iterator it = initialAssignments->begin(); it != initialAssignments->end(); it++) {
      // wylicz wartość i wstaw do środowiska. Posiłkuj się tym samym środowiskiem (zaślepka)
      result->initState->environment[(*it)->ident] = (*it)->value->eval(result->initState->environment);
   }
   
   // wstaw stan startowy do kolejki
   queue<LocalState*> pendingStates;
   pendingStates.push(result->initState);
   
   // główna pętla budująca model
   while(pendingStates.size() > 0) {
      // dopóki jest jakiś węzeł - weź go
      LocalState *state = pendingStates.front();
      pendingStates.pop();

      // wyciągnij template stanu
      LocalStateTemplate *temp=localStateTemplates[state->name];
      // dla każdej możliwej tranzycji
      for(set<TransitionTemplate*>::iterator it=temp->transitions.begin(); it != temp->transitions.end(); it++) {
         // sprawdź, czy tranzycja może zajść na podstawie warunku
         if((*it)->condition != nullptr && (*it)->condition->eval(state->environment) == 0) {
            // warunek jest fałszywy, przejdź do następnej tranzycji
            continue;
         }
         // wygeneruj następnik zależnie od schematu tranzycji
         LocalState *newState = genNextState(state, *it);
         // sprawdź, czy stanu jeszcze nie ma         
         LocalState *foundState=result->includesState(newState);
         if(foundState != NULL) {
            // owszem, zawiera
            // usuń nowy
            delete newState;
            // przepisz "stary" na "nowy"
            newState=foundState;
         } else {
            // w p.p. dodaj węzeł do struktur
            newState->id = result->localStates.size();
            // result->localStates[newState->id] = newState;
            result->localStates.push_back(newState);
            
            // wrzuć węzeł do kolejki węzłów do rozpatrzenia.
            pendingStates.push(newState);
         }
         // utwórz obiekt tranzycji
         LocalTransition *transition = new LocalTransition;
         transition->id = result->localTransitions.size();
        //  result->localTransitions[transition->id] = transition;
         result->localTransitions.push_back(transition);
         transition->isShared = ((*it)->shared > 0);
         transition->name = transition->isShared ? (*it)->patternName : "[" + to_string(result->id) + "]_" + (*it)->patternName;
         transition->localName = transition->isShared ? (*it)->matchName : "[" + to_string(result->id) + "]_" + (*it)->matchName;
         transition->sharedCount = (*it)->shared;
         transition->agent = result;
         transition->from = state;
         transition->to = newState;
         transition->probability = (*it)->probability->eval();
         if ((*it)->condition != nullptr) {
            Condition *condition = new Condition;
            condition->expression = (*it)->condition;
            transition->conditions.insert(condition);
         }
         
         // zapamiętaj w węźle
         state->localTransitions.insert(transition);
      }
   }
   // copy recommendedReductionVariableCounts to the generated Agent
   result->recommendedReductionVariableCounts = recommendedReductionVariableCounts;
   
   return result;
}

set<string> AgentTemplate::getConditionVars(TransitionTemplate *_transition) {
   set<string> vars;
   if (_transition->condition != nullptr) {
      vars = _transition->condition->getVariableNames();
   }
   return vars;
}

set<string> AgentTemplate::getAssignmentVars(TransitionTemplate *_transition) {
   set<string> vars;
   if (_transition->assignments != nullptr) {
      for(set<Assignment*>::iterator it=_transition->assignments->begin(); it != _transition->assignments->end(); it++) {
         vars.insert((*it)->ident);
      }
   }
   return vars;
}

/// @brief Checks the cone of influence for the agent template
/// @param radius The radius of the cone of influence
void AgentTemplate::checkConeOfInfluence(int radius) {
   recommendedReductionVariableCounts.clear();

   // if the radius is less than or equal to 0, we don't need to check anything
   if (radius <= 0) {
      return;
   }

   // find the initial state template
   map<string, LocalStateTemplate*>::iterator initIt = localStateTemplates.find(initState);
   if (initIt == localStateTemplates.end() || initIt->second == NULL) {
      return;
   }

   queue<LocalStateTemplate*> pendingStates;
   set<string> visitedStates;
   set<TransitionTemplate*> usedTransitions;

   pendingStates.push(initIt->second);
   visitedStates.insert(initState);

   while (!pendingStates.empty()) {
      LocalStateTemplate *stateTemplate = pendingStates.front();
      pendingStates.pop();

      // for each transition from the current state, check if it has been used before and process it if not
      for (set<TransitionTemplate*>::iterator trIt = stateTemplate->transitions.begin(); trIt != stateTemplate->transitions.end(); trIt++) {
         TransitionTemplate *transition = *trIt;
         if (transition == NULL) {
            continue;
         }

         // if the transition has not been used before, process it
         if (usedTransitions.insert(transition).second) {
            set<string> rootConditionVars = getConditionVars(transition);
            if (!rootConditionVars.empty()) {
               queue<pair<TransitionTemplate*, int>> coneQueue;
               set<TransitionTemplate*> coneUsedTransitions;
               set<TransitionTemplate*> coneQueuedTransitions;

               coneQueue.push(make_pair(transition, 1));
               coneQueuedTransitions.insert(transition);

               // while the cone queue is not empty, process each transition in the cone of influence
               while (!coneQueue.empty()) {
                  TransitionTemplate *coneTransition = coneQueue.front().first;
                  int coneDepth = coneQueue.front().second;
                  coneQueue.pop();

                  // skip transitions that were already used in this inner BFS
                  if (!coneUsedTransitions.insert(coneTransition).second) {
                     continue;
                  }

                  set<string> assignmentVars = getAssignmentVars(coneTransition);
                  // if the assignment variables are not empty, update the recommended reduction variable counts
                  if (!assignmentVars.empty()) {
                     for (set<string>::iterator assIt = assignmentVars.begin(); assIt != assignmentVars.end(); assIt++) {
                        if (recommendedReductionVariableCounts.count(*assIt) == 0) {
                           recommendedReductionVariableCounts[*assIt] = map<string, int>();
                        }

                        for (set<string>::iterator condIt = rootConditionVars.begin(); condIt != rootConditionVars.end(); condIt++) {
                           if (recommendedReductionVariableCounts[*assIt].count(*condIt) == 0) {
                              recommendedReductionVariableCounts[*assIt][*condIt] = 0;
                           }
                           recommendedReductionVariableCounts[*assIt][*condIt]++;
                        }
                     }
                  }

                  if (coneDepth >= radius) {
                     continue;
                  }
                  
                  // don't process the next transitions if the end state of the current transition is not in the localStateTemplates
                  map<string, LocalStateTemplate*>::iterator endStateIt = localStateTemplates.find(coneTransition->endState);
                  if (endStateIt == localStateTemplates.end() || endStateIt->second == NULL) {
                     continue;
                  }

                  // for each transition from the end state of the current transition, add it to the cone queue if it has not been visited before
                  LocalStateTemplate *endStateTemplate = endStateIt->second;
                  for (set<TransitionTemplate*>::iterator nextIt = endStateTemplate->transitions.begin(); nextIt != endStateTemplate->transitions.end(); nextIt++) {
                     TransitionTemplate *nextTransition = *nextIt;
                     if (nextTransition == NULL) {
                        continue;
                     }
                     
                     // if the next transition has not been used in this inner BFS and has not been queued before, add it to the cone queue
                     if (coneUsedTransitions.count(nextTransition) == 0 && coneQueuedTransitions.insert(nextTransition).second) {
                        coneQueue.push(make_pair(nextTransition, coneDepth + 1));
                     }
                  }
               }
            }
         }

         // if the end state of the transition has not been visited before, add it to the pending states queue
         if (visitedStates.count(transition->endState) == 0) {
            map<string, LocalStateTemplate*>::iterator nextStateIt = localStateTemplates.find(transition->endState);
            if (nextStateIt != localStateTemplates.end() && nextStateIt->second != NULL) {
               visitedStates.insert(transition->endState);
               pendingStates.push(nextStateIt->second);
            }
         }
      }
   }
}