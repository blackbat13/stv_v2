/**
 * @file nodes.cc
 * @brief Parser templates.
 * Class for setting up a new objects from a parser.
 */

#include "strategyNodes.hpp"
#include <queue>
#include <fstream>
#include <cstring>

/// @brief Constructor for an ActionTemplate.
ActionTemplate::ActionTemplate(vector<string>* _states, string _actionName) {
   states = _states;
   hash = "";
   for (int i = 0; i < states->size(); i++) {
      hash.append(states->at(i));
      hash.append(";");
   }
   actionName = _actionName;
}

/// @brief 
StrategyCollectionTemplate::StrategyCollectionTemplate() {
   
}

void StrategyCollectionTemplate::addAction(vector<string> *_states, string _actionName) {
   ActionTemplate action(_states, _actionName);
   selectedStrategy.insert({action.hash, action});
}
