/**
 * @file ModelParser.cc
 * @brief A model parser.
 * A parser for converting a text file into a model.
 */

#include "ModelParser.hpp"
#include "reader/nodes.hpp"
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

   Formula formula;
   formula.p = formulaDescription.formula;
   formula.isF = formulaDescription.isF;
   formula.isCTL = false;

   if (formula.p != nullptr) {
      for (const auto agent : models.agents) {
         if (formulaDescription.coalition->count(agent->name)>0) {
            formula.coalition.insert(agent);
            break;
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
