/**
 * @file Utils.hpp
 */

#ifndef STV_TYPES
#define STV_TYPES

#include "Types.hpp"
#include "Constants.hpp"
#include <map>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>

using namespace std;

string envToString(map<string, int> env);
string agentToString(Agent* agt);
string localModelsToString(LocalModels* lm);
void outputGlobalModel(GlobalModel* globalModel);
unsigned long getMemCap();
vector<set<LocalState*>> getLocalStatesSCC(Agent* agt);
map<LocalState*,vector<GlobalState*>> getContextModel(Formula* formula, LocalModels* localModels, Agent* agt);
void loadConfigFromFile(string filename="config.txt");
void loadConfigFromArgs(int argc, char** argv);

#endif // STV_TYPES