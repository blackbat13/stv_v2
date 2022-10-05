#ifndef SELENE_EXAMPLES_SSVR_UTILS
#define SELENE_EXAMPLES_SSVR_UTILS

#include "../../Types.hpp"

void addTransition(LocalTransition* localTransition, Agent* agent) {
    agent->localTransitions.push_back(localTransition);
    localTransition->from->localTransitions.insert(localTransition);
}

Var* getVar(string varName, LocalModels* localModels) {
    if (localModels->vars.count(varName) == 0) {
        localModels->vars[varName] = new Var({ varName, -1, false, -1 });
    }
    return localModels->vars[varName];
}

VarAssignment *mkVarAssignment(string varName, int varValue, string srcVarName, LocalModels* localModels) {
    Var* dstVar = getVar(varName, localModels);
    Var* srcVar = srcVarName.length() > 0 ? getVar(srcVarName, localModels) : nullptr;
    VarAssignmentType type = srcVarName.length() > 0 ? VarAssignmentType::FromVar : VarAssignmentType::FromValue;
    VarAssignment* varAssignment = new VarAssignment({ dstVar, type, srcVar, varValue });
    return varAssignment;
}

Condition *mkCondition(string varName, ConditionOperator conditionOperator, int comparedValue, LocalModels* localModels) {
    Var* var = getVar(varName, localModels);
    Condition* condition = new Condition({ var, conditionOperator, comparedValue });
    return condition;
}

void copyLocalStatesToAgent(map<string, LocalState*>* localStates, Agent* agent) {
    agent->localStates.resize(localStates->size());
    for (auto localStatePair: *localStates) {
        agent->localStates[localStatePair.second->id] = localStatePair.second;
    }
}

#endif // SELENE_EXAMPLES_SSVR_UTILS
