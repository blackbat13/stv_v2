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
string agentToString(shared_ptr<Agent> agt);
string localModelsToString(shared_ptr<LocalModels> lm);
void outputGlobalModel(shared_ptr<GlobalModel> globalModel);
unsigned long getMemCap();
vector<set<shared_ptr<LocalState>>> getLocalStatesSCC(shared_ptr<Agent> agt);
map<shared_ptr<LocalState>, vector<shared_ptr<GlobalState>>> getContextModel(shared_ptr<Formula> formula, shared_ptr<LocalModels> localModels, shared_ptr<Agent> agt);

#endif // STV_TYPES