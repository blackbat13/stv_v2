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
void localModelsToDotString(LocalModels* lm);
vector<set<LocalState*>> getSCC(Agent* agt);

#endif // STV_TYPES