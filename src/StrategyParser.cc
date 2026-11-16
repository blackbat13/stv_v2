/**
 * @file StrategyParser.cc
 * @brief A strategy file parser.
 * A parser for converting a text file into a strategy.
 */

#include "StrategyParser.hpp"
#include "strategyReader/strategyNodes.hpp"
#include <stdio.h>
#include <string.h>

#include <tuple>
#include <iostream>
#include <stdexcept>

using namespace std;

extern int stratparse();
extern void stratrestart(FILE*);
extern int stratlineno;
void set_input_string_strat(const char* in);

StrategyCollectionTemplate strategyCollectionTemplate;

/// @brief StrategyParser constructor.
StrategyParser::StrategyParser() {
}

/// @brief ModelParser destructor.
StrategyParser::~StrategyParser() {
}

/// @brief Parses a file with given name into a usable strategy.
/// @param fileName Name of the file to be converted into a strategy.
/// @return Pointer to a model created from a given file.
StrategyCollection* StrategyParser::parse(string fileName) {
   // otwórz plik wejściowy
   FILE *f=fopen(fileName.c_str(), "r");
   if (f == nullptr) {
      throw std::runtime_error("Failed to open strategy file '" + fileName + "'");
   }
   // the template is global, so drop the actions of a previously parsed file
   strategyCollectionTemplate.selectedStrategy.clear();
   // zamapuj go jako wejście dla Fleksa
   stratrestart(f);
   stratlineno = 1;
   // uruchom parsowanie
   try {
      stratparse();
   } catch (...) {
      fclose(f);
      throw;
   }

   StrategyCollection* strategyCollection = new StrategyCollection();

   for (auto it = strategyCollectionTemplate.selectedStrategy.begin(); it != strategyCollectionTemplate.selectedStrategy.end(); it++) {
      Action action;
      action.actionName = it->second.actionName;
      action.hash = it->second.hash;
      action.states = it->second.states;
      // cout << action.hash << " " << action.actionName << endl;
      strategyCollection->addAction(action);
   }

   fclose(f);
   return strategyCollection;
}
