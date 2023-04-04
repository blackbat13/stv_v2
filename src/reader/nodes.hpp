/**
 * @file nodes.hpp
 * @brief Parser templates.
 * Class for setting up a new objects from a parser.
 */

#ifndef __NODES_H
#define __NODES_H

#include <string>
#include <set>
#include <map>
#include "expressions.hpp"

#include "../Types.hpp"

using namespace std;

/* Klasa reprezentująca przypisanie */
/// @brief Represents an assingment.
class Assignment {
   public:
      /// @brief To what we should assign a value.
      string ident;
      // do czego przypisujemy

      /// @brief A value to be assigned.
      ExprNode *value;
      // co przypisujemy

      /// @brief Constructor for an Assignment class.
      /// @param _ident To what we should assign a value.
      /// @param _exp A value to be assigned.
      Assignment(string _ident, ExprNode *_exp): ident(_ident), value(_exp) {};
      
      // wykonaj przypisanie w danym środowisku
      virtual void assign(Environment &env) {
         env[ident]=value->eval(env);
      };
};

/* Klasa reprezentująca meta-tranzycję */
class TransitionTemplate {
   public:
      // jeśli -1 to nie ma dzielenia, w p.p. wartość określa łączną liczbę wymaganych agentów
      int shared;
      // nazwa wzorca
      string patternName;
      // nazwa do wyszukiwania dla shared
      string matchName;
      // nazwa stanu początkowego i końcowego
      string startState;
      string endState;
      // wyrażenie warunkowe
      ExprNode *condition;
      // lista przypisań wartości
      set<Assignment*> *assignments;

      TransitionTemplate(int _shared, string _patternName, string _matchName, string _startState, string _endState, ExprNode *_cond, set<Assignment*> *_assign): 
            shared(_shared), patternName(_patternName), matchName(_matchName),
            startState(_startState), endState(_endState), condition(_cond), assignments(_assign) {};
            
};

class LocalStateTemplate {
   public:
      string name;
      set<TransitionTemplate*> transitions;
};

/* Klasa reprezentująca pojedynczego agenta po wczytaniu jego opisu z pliku */
class AgentTemplate {
      // identyfikator agenta
      /// @brief Agent identifier.
      string ident;
      // stan startowy
      /// @brief Agent initial state.
      string initState;
      // zbiór zmiennych lokalnych (local)
      /// @brief A set of local variables.
      set<string>* localVars;
      // zbiór zmiennych trwałych (persistent)
      /// @brief A set of persistent variables.
      set<string>* persistentVars;
      // początkowa inicjacja
      set<Assignment*>* initialAssignments;
      // zbiór tranzycji
      set<TransitionTemplate*>* transitions;
   
      // mapa stanów lokalnych potrzebna do wygenerowania modelu
      map<string,LocalStateTemplate*> localStateTemplates;
            
      // metoda wyznaczająca węzeł kolejny do danego, zależnie od tranzycji
      virtual LocalState* genNextState(LocalState *state, TransitionTemplate *trans);
      
   public:
      AgentTemplate();
      
      // ustaw identyfikator agenta
      virtual AgentTemplate& setIdent(string _ident);
      // ustaw identyfikator agenta
      virtual AgentTemplate& setInitState(string _startState);
      // dodaj zmienną/zmienne lokalne
      virtual AgentTemplate& addLocal(set<string> *variables);
      // dodaj zmienne trwałe
      virtual AgentTemplate& addPersistent(set<string> *variables);
      // dodaj początkowe inicjacje
      virtual AgentTemplate& addInitial(set<Assignment*> *assigns);
      // dodaj tranzycję
      virtual AgentTemplate& addTransition(TransitionTemplate *_transition);
      
      // wygeneruj agenta do modelu
      virtual Agent* generateAgent(int id) ;
};

#endif
