/**
 * @file nodes.cc
 * @brief Parser templates.
 * Class for setting up a new objects from a parser.
 */

#include "expressions.hpp"
#include "nodes.hpp"
#include <queue>
#include <fstream>


/* --------------------------------------------------------------------- */
// konstruktor bezparametrowy

/// @brief Constructor for an AgentTemplate.
AgentTemplate::AgentTemplate() {
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
         
         // zapamiętaj w węźle
         state->localTransitions.insert(transition);
      }
   }
   
   
   return result;
}

