/**
 * @file Types.cc
 * @brief Custom data structures.
 * Data structures and classes containing model data.
 */
#include "Types.hpp"

/* Funkcja porównująca dwa stany.
 * Pod uwagę brana jest nazwa oraz wartości zmiennych.
 * Wszystko musi się zgadzać, czyli musi być dokładnie ta sama nazwa (wzorca)
 * oraz tyle samo takich samych zmiennych o takich samych wartościach.
 */
/// @brief Function comparing two states.
/// @param state A pointer to LocalState to which this state should be compared to.
/// @return Returns true if the current LocalState is the same as the passed one, otherwise false.
bool LocalState::compare(LocalState* state) {
   // agent musi być ten sam (sanity check)
   if(agent != state->agent) return false;

   // nie porównujemy identyfikatora bo nie ma sensu
   // porównaj nazwy (w zasadzie template'y
   if(name != state->name) return false;

   // if(environment.size() != state->environment.size() || vars.size()!=state->vars.size()){
   //    printf("===========================================\n");
   //    printf("test on env: %d\n",environment.size()-state->environment.size());
   //    printf("test on var: %d\n",vars.size()-state->vars.size());
   //    printf("===========================================\n");
   // }

   // porównaj środowiska na rozmiar
   if(environment.size() != state->environment.size()) return false;
   // porównaj wartości zmiennych
   for(map<string,int>::iterator it=environment.begin(); it != environment.end(); it++) {
      // jeśli nie ma zmiennej
      if(state->environment.count(it->first) == 0) return false;
      // zmienna jest, czy wartości się zgadzają
      if(state->environment[it->first] != it->second) return false;
   }
   // stany są takie same - czyli to ten sam stan   
   return true;
}

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

atomic_uint32_t GlobalState::next_id;
atomic_uint32_t GlobalTransition::next_id;

GlobalState::GlobalState(){
   id = next_id++;
   isExpanded = false;
   verificationStatus = GlobalStateVerificationStatus::UNVERIFIED;
}

GlobalTransition::GlobalTransition(){
   id = next_id++;
}