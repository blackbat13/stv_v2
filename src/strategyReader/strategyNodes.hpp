/**
 * @file strategyNodes.hpp
 * @brief Strategy parser templates.
 * Class for setting up a new objects from a parser.
 */

#ifndef __STRATEGY_NODES_H
#define __STRATEGY_NODES_H

#include <string>
#include <set>
#include <map>

#include "../Types.hpp"

using namespace std;

/// @brief Represents a selected action.
class ActionTemplate {
   public:
      /// @brief Constructor for the ActionTemplate
      /// @param _states Vector of states from which the action has to be executed from.
      /// @param _actionName Action executed from the aforementioned states.
      ActionTemplate(vector<string>* _states, string _actionName);
      /// @brief Vector of states from which the action has to be executed from.
      vector<string> *states;
      /// @brief Contains the states in a 
      string hash;
      /// @brief 
      string actionName;
};

/// @brief 
class StrategyCollectionTemplate {
   public:
      /// @brief 
      StrategyCollectionTemplate();
      /// @brief 
      map<string, ActionTemplate> selectedStrategy;
      /// @brief 
      /// @param newAction 
      void addAction(vector<string>* _states, string _actionName);
};

#endif
