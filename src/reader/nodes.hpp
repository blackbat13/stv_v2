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

      /// @brief Make an assignment in a given environment.
      /// @param env Environment in which to make an assignment.
      virtual void assign(Environment &env) {
         env[ident]=value->eval(env);
      };
};

/* Klasa reprezentująca meta-tranzycję */

/// @brief Represents a meta-transition.
class TransitionTemplate {
   public:
      // jeśli -1 to nie ma dzielenia, w p.p. wartość określa łączną liczbę wymaganych agentów

      /// @brief Needed amound of needed agents. -1 if not shared.
      int shared;

      // nazwa wzorca

      /// @brief Name of the pattern.
      string patternName;

      // nazwa do wyszukiwania dla shared

      /// @brief Global name for shared transitions.
      string matchName;

      // nazwa stanu początkowego i końcowego

      /// @brief Start state name.
      string startState;

      /// @brief End state name.
      string endState;

      // wyrażenie warunkowe

      /// @brief Condition expression that has do be fulfilled in that transition.
      ExprNode *condition;

      // lista przypisań wartości

      /// @brief Set of assignments.
      set<Assignment*> *assignments;

      /// @brief TransitionTemplate constructor.
      /// @param _shared Needed amound of needed agents. -1 if not shared.
      /// @param _patternName Name of the pattern.
      /// @param _matchName Global name for shared transitions.
      /// @param _startState Start state name.
      /// @param _endState End state name.
      /// @param _cond Condition expression that has do be fulfilled in that transition.
      /// @param _assign Set of assignments.
      TransitionTemplate(int _shared, string _patternName, string _matchName, string _startState, string _endState, ExprNode *_cond, set<Assignment*> *_assign): 
            shared(_shared), patternName(_patternName), matchName(_matchName),
            startState(_startState), endState(_endState), condition(_cond), assignments(_assign) {};
            
};

/// @brief A template for the local state.
class LocalStateTemplate {
   public:
      /// @brief Name of the local state.
      string name;

      /// @brief Local transitions going out from this state.
      set<TransitionTemplate*> transitions;
};

/* Klasa reprezentująca pojedynczego agenta po wczytaniu jego opisu z pliku */

/// @brief Represents a single agent loaded from the description from a file.
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
      
      /// @brief Initial initialization.
      set<Assignment*>* initialAssignments;
      
      // zbiór tranzycji
      
      /// @brief A set of transitions.
      set<TransitionTemplate*>* transitions;
   
      // mapa stanów lokalnych potrzebna do wygenerowania modelu

      /// @brief Map of local states needed to generate a model.
      map<string,LocalStateTemplate*> localStateTemplates;
            
      // metoda wyznaczająca węzeł kolejny do danego, zależnie od tranzycji

      virtual LocalState* genNextState(LocalState *state, TransitionTemplate *trans);
      
   public:
      AgentTemplate();
      
      // ustaw identyfikator agenta

      /// @brief Set the identifier of an agent.
      /// @param _ident New agent identifier.
      /// @return Returns a pointer to self.
      virtual AgentTemplate& setIdent(string _ident);

      // ustaw identyfikator agenta

      /// @brief Set the initial state of the agent.
      /// @param _startState New inital agent state.
      /// @return Returns a pointer to self.
      virtual AgentTemplate& setInitState(string _startState);
      
      // dodaj zmienną/zmienne lokalne

      /// @brief Adds local variables to an agent.
      /// @param variables Set of variables to be added.
      /// @return Returns a pointer to self.
      virtual AgentTemplate& addLocal(set<string> *variables);

      // dodaj zmienne trwałe

      /// @brief Adds persistent variables to an agent.
      /// @param variables Set of variables to be added.
      /// @return Returns a pointer to self.
      virtual AgentTemplate& addPersistent(set<string> *variables);

      // dodaj początkowe inicjacje

      /// @brief Adds initial assignments.
      /// @param assigns Assignments to be added.
      /// @return Returns a pointer to self.
      virtual AgentTemplate& addInitial(set<Assignment*> *assigns);
      
      // dodaj tranzycję

      /// @brief Adds a transition to the agent.
      /// @param _transition Transition to be added.
      /// @return Returns a pointer to self.
      virtual AgentTemplate& addTransition(TransitionTemplate *_transition);
      
      // wygeneruj agenta do modelu

      /// @brief Generate a new agent for the model.
      /// @param id Identification number defining a new Agent.
      /// @return Returns a pointer to a new Agent.
      virtual Agent* generateAgent(int id) ;
};

#endif
