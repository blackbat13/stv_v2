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
#include "Verification.hpp"
#include <string>
#include <cmath>
#include <queue>
#include <algorithm>

using namespace std;

//void mockKBC(GlobalModel *const gm);
//void mockLocalKBC(Agent *const a);
void KBCprojection(GlobalModel *const gm, int agent_id);
Agent* KBCexpansion(GlobalModel *const gm, int agent_id);
GlobalModel* cloneGlobalModel(LocalModels* localModels, Formula* formula);