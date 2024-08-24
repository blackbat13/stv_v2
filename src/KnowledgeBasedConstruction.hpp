/**
 * @file KnowledgeBasedConstruction.hpp
 * @brief KBC implementation
 * @author Filip Jamroga
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

void ModelDotDump(GlobalModel *const gm, string prefix);
void KBCprojection(GlobalModel *const gm, int agent_id);
Agent* KBCexpansion(GlobalModel *const gm, int agent_id);
GlobalModel* cloneGlobalModel(LocalModels* localModels, Formula* formula);
