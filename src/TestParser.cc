#include "TestParser.hpp"
#include "reader/nodes.hpp"
#include <stdio.h>

using namespace std;

extern int yyparse();
extern void yyrestart(FILE*);

set<AgentTemplate*>* modelDescription;

TestParser::TestParser() {
}

TestParser::~TestParser() {
}

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
