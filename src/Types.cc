#include "Types.hpp"

/* Funkcja porównująca dwa stany.
 * Pod uwagę brana jest nazwa oraz wartości zmiennych.
 * Wszystko musi się zgadzać, czyli musi być dokładnie ta sama nazwa (wzorca)
 * oraz tyle samo takich samych zmiennych o takich samych wartościach.
 */
bool LocalState::compare(LocalState* state) {
   // agent musi być ten sam (sanity check)
   if(agent != state->agent) return false;

   // nie porównujemy identyfikatora bo nie ma sensu
   // porównaj nazwy (w zasadzie template'y
   if(name != state->name) return false;
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
LocalState* Agent::includesState(LocalState* state) {
   // pętla po stanach już obecnych w modelu
   for(size_t i=0; i<localStates.size(); i++) {
      // jeśli jest zgodność, zwróć fałsz
      if(localStates[i]->compare(state)) return localStates[i];
   }
   return NULL;
}
