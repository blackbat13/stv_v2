/**
 * @file KnowledgeBasedConstruction.hpp
 * @brief Blah
 * Blah blah
 */

#include "Types.hpp"
#include "Utils.hpp"
#include "Common.hpp"
#include "reader/nodes.hpp"
#include "GlobalModelGenerator.hpp"
#include <string>
#include <cmath>

using namespace std;

//void mockKBC(GlobalModel *const gm);
//void mockLocalKBC(Agent *const a);
void KBCprojection(GlobalModel *const gm, int agent_id);
bool checkAgent(GlobalState* s, int agent_id);
GlobalModel* cloneGlobalModel(LocalModels* localModels, Formula* formula);