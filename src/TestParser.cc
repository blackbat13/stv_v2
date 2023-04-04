/**
 * @file TextParser.cc
 * @brief Model parser.
 * A parser for converting a text file into a model.
 */

#include "TestParser.hpp"
#include "reader/nodes.hpp"
#include <stdio.h>

using namespace std;

extern int yyparse();
extern void yyrestart(FILE*);

set<AgentTemplate*>* modelDescription;

/// @brief TestParser constructor.
TestParser::TestParser() {
}

/// @brief TestParser destructor.
TestParser::~TestParser() {
}

/// @brief Parses a file with given name into a usable model.
/// @param fileName Name of the file to be converted into a model.
/// @return Pointer to a model created from a given file.
LocalModels * TestParser::parse(string fileName) {
   // otwórz plik wejściowy
   FILE *f=fopen(fileName.c_str(), "r");
   // zamapuj go jako wejście dla Fleksa
   yyrestart(f);
   // uruchom parsowanie
   yyparse();
   // na modelDescription jest opis modeli agentów
   
   // w pętli przetwórz wszystkie modele i wygeneruj docelowe modele lokalne
   LocalModels *models = new LocalModels;
   
   int i = 0;
   for(set<AgentTemplate*>::iterator it=modelDescription->begin(); it != modelDescription->end(); it++, i++) {
      models->agents.push_back((*it)->generateAgent(i));
      // jeszcze wskaźniki do zmiennych - raczej zbędne
      for(set<Var*>::iterator varit=models->agents[i]->vars.begin(); varit != models->agents[i]->vars.end(); varit++) {
         models->vars[(*varit)->name] = *varit;
      }
   }
   return models;
}
