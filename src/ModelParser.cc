/**
 * @file ModelParser.cc
 * @brief A model parser.
 * A parser for converting a text file into a model.
 */

#include "ModelParser.hpp"
#include "reader/nodes.hpp"
#include "GlobalModelGenerator.hpp"
#include <stdio.h>
#include <string.h>

#include <tuple>
#include <iostream>

using namespace std;

extern int yyparse();
extern void yyrestart(FILE*);
void set_input_string(const char* in);

set<AgentTemplate*>* modelDescription;
FormulaTemplate formulaDescription;

/// @brief ModelParser constructor.
ModelParser::ModelParser() {
}

/// @brief ModelParser destructor.
ModelParser::~ModelParser() {
}

/// @brief Parses a file with given name into a usable model.
/// @param fileName Name of the file to be converted into a model.
/// @return Pointer to a model created from a given file.
tuple<LocalModels, Formula> ModelParser::parse(string fileName) {
   // otwórz plik wejściowy
   FILE *f=fopen(fileName.c_str(), "r");
   if (f == nullptr) {
      throw std::runtime_error("Failed to open model file: " + fileName);
   }
   // zamapuj go jako wejście dla Fleksa
   yyrestart(f);
   // uruchom parsowanie
   yyparse();
   // na modelDescription jest opis modeli agentów
   
   // w pętli przetwórz wszystkie modele i wygeneruj docelowe modele lokalne
   LocalModels models;
   
   if (config.cone_of_influence) {
       for (auto agentTemplate : *modelDescription) {
           agentTemplate->checkConeOfInfluence(config.cone_radius);
       }
   }

   int i = 0;
   for (set<AgentTemplate*>::iterator it=modelDescription->begin(); it != modelDescription->end(); it++, i++) {
      models.agents.push_back((*it)->generateAgent(i));
   }

   if (config.merge_coalition) {
      mergeCoalition(formulaDescription.coalition, &models);
   }

   Formula formula;
   formula.p = formulaDescription.formula;
   formula.isF = formulaDescription.isF;
   formula.isCTL = false;
   if (formulaDescription.probability != NULL) {
      formula.probability = formulaDescription.probability->eval();
      if (formulaDescription.probabilitySign == "==") {
         formula.probabilitySign = ProbabilitySign::EQ;
      } else if (formulaDescription.probabilitySign == "!=") {
         formula.probabilitySign = ProbabilitySign::NE;
      } else if (formulaDescription.probabilitySign == ">") {
         formula.probabilitySign = ProbabilitySign::GT;
      } else if (formulaDescription.probabilitySign == ">=") {
         formula.probabilitySign = ProbabilitySign::GE;
      } else if (formulaDescription.probabilitySign == "<") {
         formula.probabilitySign = ProbabilitySign::LT;
      } else if (formulaDescription.probabilitySign == "<=") {
         formula.probabilitySign = ProbabilitySign::LE;
      } 
   }

   // cout << "Formula probability: " << formula.probabilitySign << " " << formula.probability << endl;

   if (formula.p != nullptr) {
      for (const auto agent : models.agents) {
         if (formulaDescription.coalition->count(agent->name)>0) {
            formula.coalition.insert(agent);
         }
      }
   }
   if (formula.coalition.size() != formulaDescription.coalition->size()) {
      throw std::runtime_error("Incorrect agent name");
   }

   if (formulaDescription.coalition->size() == 0) {
      formula.isCTL = true;
      for (const auto agent : models.agents) {
         formula.coalition.insert(agent);
      }
   }

   fclose(f);
   return tuple<LocalModels, Formula>{models, formula};
}

/// @brief Parses a text file in the given file path and internally replaces the verification formula with the given one.
/// @param fileName Path to the text file.
/// @param s New formula.
/// @return Returns a new model generation structure with a replaced formula.
tuple<LocalModels, Formula> ModelParser::parseAndOverwriteFormula(string fileName, string s) {
   // otwórz plik wejściowy
   FILE *f=fopen(fileName.c_str(), "r");
   // zamapuj go jako wejście dla Fleksa
   yyrestart(f);
   // uruchom parsowanie
   yyparse();
   // na modelDescription jest opis modeli agentów
   
   // w pętli przetwórz wszystkie modele i wygeneruj docelowe modele lokalne
   LocalModels models;
   
   int i = 0;
   for(set<AgentTemplate*>::iterator it=modelDescription->begin(); it != modelDescription->end(); it++, i++) {
      models.agents.push_back((*it)->generateAgent(i));
   }

   if (config.merge_coalition) {
      mergeCoalition(formulaDescription.coalition, &models);
   }
   
   fclose(f);

   char* ch = strdup(s.c_str());
   set_input_string(ch);
   yyparse();

   Formula formula;
   formula.p = formulaDescription.formula;
   formula.isF = formulaDescription.isF;
   
   for (const auto agent : models.agents) {
      if (formulaDescription.coalition->count(agent->name)>0) {
         formula.coalition.insert(agent);
         break;
      }
   }

   free(ch);
   
   return tuple<LocalModels, Formula>{models, formula};
}

/// @brief Builds a key identifying local states (order-dependent).
/// @param tuple Local states, one per coalition agent, in a fixed agent order.
/// @return A string key uniquely identifying that combination.
string ModelParser::localStateTupleKey(const vector<LocalState*>& tuple) {
    string key;
    for (const auto* state : tuple) {
        key += to_string(state->id) + ";";
    }
    return key;
}

/// @brief Merges the environments (variable maps) of a tuple of local states into one combined environment.
/// @param tuple Local states, one per coalition agent, to combine.
/// @return Union of all environments (variable names are assumed to not clash across agents).
map<string, int> ModelParser::mergeEnvironment(const vector<LocalState*>& tuple) {
    map<string, int> environment;
    for (const auto* state : tuple) {
        environment.insert(state->environment.begin(), state->environment.end());
    }
    return environment;
}

/// @brief Builds the combined display name "(name1, name2, ...)" for a tuple of local states.
/// @param tuple Local states, one per coalition agent, to combine.
/// @return Combined name.
string ModelParser::mergedStateName(const vector<LocalState*>& tuple) {
    string name = "(";
    for (size_t i = 0; i < tuple.size(); i++) {
        if (i > 0) {
            name += ", ";
        }
        name += tuple[i]->name;
    }
    name += ")";
    return name;
}

/// @brief Finds the merged LocalState for a tuple of per-agent local states, creating it (and queuing it for further expansion) if it doesn't exist yet.
/// @param mergedAgent The new merged agent that will own the merged state.
/// @param tuple Local states, one per coalition agent, to merge.
/// @param mergedStatesByKey Map of existing merged states by their identifying key.
/// @param tupleOfMergedState Map of merged states to their original tuples.
/// @param pendingStates Queue of merged states that still need to be expanded (their outgoing transitions need to be generated).
/// @return The merged LocalState corresponding to the given tuple.
LocalState* ModelParser::findOrCreateMergedState(Agent* mergedAgent, const vector<LocalState*>& tuple, map<string, LocalState*>& mergedStatesByKey, map<LocalState*, vector<LocalState*>>& tupleOfMergedState, queue<LocalState*>& pendingStates) {
   string key = localStateTupleKey(tuple);
   auto found = mergedStatesByKey.find(key);
   if (found != mergedStatesByKey.end()) {
      return found->second;
   }
   LocalState* mergedState = new LocalState();
   mergedState->id = mergedAgent->localStates.size();
   mergedState->name = mergedStateName(tuple);
   mergedState->environment = mergeEnvironment(tuple);
   mergedState->agent = mergedAgent;
   mergedAgent->localStates.push_back(mergedState);
   mergedStatesByKey[key] = mergedState;
   tupleOfMergedState[mergedState] = tuple;
   pendingStates.push(mergedState);
   return mergedState;
}

/// @brief Merges a coalition of agents into a single Agent, replacing them in the given local models. The merged agent's states are tuples of the original agents' local states, and it keeps every outgoing transition each original state had (unchanged, including shared/sharedCount), just rehomed onto the corresponding merged states.
/// @param coalitionNames Names of the agents that should be merged into a single agent.
/// @param localModels Local models whose agents (and their local states/transitions) will be merged in place; the original coalition agents are deleted once merging is complete.
void ModelParser::mergeCoalition(set<string>* coalitionNames, LocalModels* localModels) {
   // No need to merge if there's only one or zero agents in the coalition.
   if (coalitionNames->size() <= 1) {
      return;
   }

   // Preserve the existing order of agents in the model for a stable, deterministic tuple order.
   vector<Agent*> coalitionAgents;
   for (const auto& agent : localModels->agents) {
      if (coalitionNames->count(agent->name) > 0) {
         coalitionAgents.push_back(agent);
      }
   }
   string concatenatedName = "";
   for (const auto& agent : coalitionAgents) {
      concatenatedName += concatenatedName.empty() ? agent->name : ("_" + agent->name);
   }

   // Create the new merged agent and reassign all variables from the original agents to it.
   Agent* mergedAgent = new Agent(localModels->agents.size(), concatenatedName);
   for (const auto& agent : coalitionAgents) {
      for (auto* var : agent->vars) {
         var->agent = mergedAgent;
         mergedAgent->vars.insert(var);
      }
   }

   // BFS over tuples of per-agent local states to create merged states and their transitions.
   vector<LocalState*> initTuple;
   for (const auto& agent : coalitionAgents) {
      initTuple.push_back(agent->initState);
   }

   map<string, LocalState*> mergedStatesByKey;
   map<LocalState*, vector<LocalState*>> tupleOfMergedState;
   queue<LocalState*> pendingStates;

   mergedAgent->initState = findOrCreateMergedState(mergedAgent, initTuple, mergedStatesByKey, tupleOfMergedState, pendingStates);

   while (!pendingStates.empty()) {
      LocalState* mergedState = pendingStates.front();
      pendingStates.pop();
      vector<LocalState*> tuple = tupleOfMergedState[mergedState];

      // Every outgoing transition of every original sub-state becomes an outgoing transition of the merged state, unmodified except for the endpoints it now connects.
      for (size_t i = 0; i < tuple.size(); i++) {
         for (auto* localTransition : tuple[i]->localTransitions) {
            vector<LocalState*> targetTuple = tuple;
            targetTuple[i] = localTransition->to;
            LocalState* mergedTargetState = findOrCreateMergedState(mergedAgent, targetTuple, mergedStatesByKey, tupleOfMergedState, pendingStates);

            LocalTransition* mergedTransition = new LocalTransition(*localTransition);
            mergedTransition->id = mergedAgent->localTransitions.size();
            mergedTransition->agent = mergedAgent;
            mergedTransition->from = mergedState;
            mergedTransition->to = mergedTargetState;

            mergedAgent->localTransitions.push_back(mergedTransition);
            mergedState->localTransitions.insert(mergedTransition);
         }
      }

      // If the merged state has no outgoing transitions, check if we should add an epsilon self-loop to avoid deadlock.
      GlobalModelGenerator* tempGenerator = new GlobalModelGenerator();
      if (config.add_epsilon_transitions && tempGenerator->hasDeadlockCombination(&tuple)) {
         Agent* agent = mergedAgent;
         LocalState* epsilonLocalState = mergedState;

         LocalTransition* transition = new LocalTransition;
         transition->id = -1;
         transition->isShared = 0;
         transition->name = "ɛ";
         transition->localName = "ɛ";
         transition->sharedCount = 0;
         transition->agent = agent;
         transition->from = epsilonLocalState;
         transition->to = epsilonLocalState;
         transition->probability = 1.0;
         mergedState->localTransitions.insert(transition);

         agent->localTransitions.push_back(transition);
      }
      delete tempGenerator;

      // Propagate the reduced sharedCount to every other agent's transitions sharing that name, so the requirement stays consistent for whoever still needs to synchronize with them.
      map<string, int>* sharedCountMapToChange = resolveSharedTransitions(mergedAgent, mergedState, tuple, mergedStatesByKey, tupleOfMergedState, pendingStates);
      
      if (!sharedCountMapToChange->empty()) {
         for (auto& nameAndCount : *sharedCountMapToChange) {
            for (auto* model : localModels->agents) {
               for (auto* transition : model->localTransitions) {
                  if (transition->name == nameAndCount.first) {
                     transition->sharedCount = nameAndCount.second;
                  }
               }
            }
         }
      }
      delete sharedCountMapToChange;
   }

   // Replace the coalition agents with the merged agent and clear the now unnecessary original data.
   vector<Agent*> remainingAgents;
   for (const auto& agent : localModels->agents) {
      if (coalitionNames->count(agent->name) == 0) {
         remainingAgents.push_back(agent);
      }
   }
   remainingAgents.push_back(mergedAgent);
   for (size_t i = 0; i < remainingAgents.size(); i++) {
      remainingAgents[i]->id = i;
   }
   localModels->agents = remainingAgents;

   // Delete the original coalition agents and their local states/transitions to avoid memory leaks.
   for (const auto& agent : coalitionAgents) {
      for (auto* localTransition : agent->localTransitions) {
         delete localTransition;
      }
      for (auto* localState : agent->localStates) {
         delete localState;
      }
      delete agent;
   }

   // Reflect the merge in the formula's coalition names so later name-based lookups keep working.
   coalitionNames->clear();
   coalitionNames->insert(concatenatedName);
}

map<string, int>* ModelParser::resolveSharedTransitions(Agent* mergedAgent, LocalState* mergedState, const vector<LocalState*>& fromTuple, map<string, LocalState*>& mergedStatesByKey, map<LocalState*, vector<LocalState*>>& tupleOfMergedState, queue<LocalState*>& pendingStates) {
   set<LocalTransition*>* transitionsFromMergedState = &mergedState->localTransitions;
   map<string, set<LocalTransition*>> sharedMap;
   map<string, int>* sharedCountMapToChange = new map<string, int>();
   for (auto* transition : *transitionsFromMergedState) {
      if (transition->isShared) {
         sharedMap[transition->name].insert(transition);
      }
   }
   // Look for shared transitions that have the same name and local name
   for (const auto& pair : sharedMap) {
      const auto& transitionSet = pair.second;
      if (transitionSet.size() > 1) {
         // Check if there is a transition with the same local name as the global name
         LocalTransition* representative = nullptr;
         for (auto* transition : transitionSet) {
            if (transition->localName == transition->name) {
               representative = transition;
               break;
            }
         }
         // If no such transition exists, just pick the first one as representative
         if (representative == nullptr) {
            representative = *transitionSet.begin();
         }
         // Each duplicate only moved its own agent's slot; combine all of those moves into one joint target state instead of discarding the ones we're about to remove.
         vector<LocalState*> jointTargetTuple = fromTuple;
         for (auto* transition : transitionSet) {
            const vector<LocalState*>& targetTuple = tupleOfMergedState[transition->to];
            for (size_t i = 0; i < fromTuple.size(); i++) {
               if (targetTuple[i] != fromTuple[i]) {
                  jointTargetTuple[i] = targetTuple[i];
               }
            }
         }
         representative->to = findOrCreateMergedState(mergedAgent, jointTargetTuple, mergedStatesByKey, tupleOfMergedState, pendingStates);
         // Remove all other transitions from the set and delete them
         int removedCount = 0;
         for (auto* transition : transitionSet) {
            if (transition != representative) {
               representative->sharedCount -= 1;
               if(representative->sharedCount == 1) {
                  representative->isShared = false;
               }
               removedCount++;
               transitionsFromMergedState->erase(transition);
               delete transition;
            }
         }
         sharedCountMapToChange->insert({representative->name, representative->sharedCount});
      }
   }
   return sharedCountMapToChange;
}
