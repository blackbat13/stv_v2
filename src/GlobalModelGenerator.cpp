/**
 * @file GlobalModelGenerator.cpp
 * @brief Generator of a global model.
 * Class for initializing and generating a global model.
 */

#include "GlobalModelGenerator.hpp"
#include <iostream>

/// @brief Constructor for GlobalModelGenerator class.
GlobalModelGenerator::GlobalModelGenerator() {
}

/// @brief Destructor for GlobalModelGenerator class.
GlobalModelGenerator::~GlobalModelGenerator() {
}

/// @brief Initializes a global model from local models and a formula.
/// @param localModels Pointer to LocalModels that will construct a global model.
/// @param formula Pointer to a Formula to include into the model.
/// @return Returns a pointer to initial state of the global model.
GlobalState* GlobalModelGenerator::initModel(LocalModels* localModels, Formula* formula) {
    this->localModels = localModels;
    this->formula = formula;
    this->globalModel = new GlobalModel();
    this->globalModel->agents = localModels->agents;
    this->globalModel->initState = this->generateInitState();
    return this->globalModel->initState;
}

/// @brief Goes through all GlobalTransition in a given GlobalState and creates new GlobalStates connected to the given one.
/// @param state A state from which the expansion should start.
void GlobalModelGenerator::expandState(GlobalState* state) {
    if (state->isExpanded) {
        return;
    }
    for (const auto globalTransition : state->globalTransitions) {
        if (globalTransition->to == nullptr) {
            set<LocalState*> localStates = state->localStates;
            for (const auto localTransition : globalTransition->localTransitions) {
                #if VERBOSE
                    cout << "expandState: " << localTransition->name << " : " << localTransition->from << " -> " << localTransition->to << endl;
                #endif
                localStates.erase(localTransition->from);
                localStates.insert(localTransition->to);
            }            
            auto targetState = this->generateStateFromLocalStates(&localStates, &globalTransition->localTransitions, state);
            globalTransition->to = targetState;
        }
    }
    state->isExpanded = true;
}

/// @brief Expands the states starting from the initial GlobalState and continues until there are no more states to expand.
void GlobalModelGenerator::expandAllStates() {
    set<GlobalState*> statesToExpand;
    statesToExpand.insert(this->globalModel->initState);
    while (!statesToExpand.empty()) {
        auto globalState = *statesToExpand.begin();
        statesToExpand.erase(globalState);
        #if VERBOSE
            printf("\nExpanding %i (%s)\n", globalState->id, globalState->hash.c_str());
        #endif
        this->expandState(globalState);
        for (auto transition : globalState->globalTransitions) {
            auto targetGlobalState = transition->to;
            if (!targetGlobalState->isExpanded) {
                statesToExpand.insert(targetGlobalState);
            }
        }
    }
}

/// @brief Get for a GlobalModel used in initialization.
/// @return Returns a pointer to a global model.
GlobalModel* GlobalModelGenerator::getCurrentGlobalModel() {
    return this->globalModel;
}

/// @brief Get for the Formula used in initialization.
/// @return Returns a pointer to the formula structure.
Formula* GlobalModelGenerator::getFormula() {
    return this->formula;
}

/// @brief Generates initial state of the model from GlobalModel in memory.
/// @return Returns a pointer to an initial GlobalState.
GlobalState* GlobalModelGenerator::generateInitState() {
    set<LocalState*> localStates;
    for (const auto agt : this->globalModel->agents) {
        localStates.insert(agt->initState);
    }
    auto initState = this->generateStateFromLocalStates(&localStates, nullptr, nullptr);
    
    // vars: use initial values
    for (const auto varData : this->localModels->vars) {
        if (varData.second->initialValue != -1) {
            initState->vars[varData.second] = varData.second->initialValue;
        }
    }
    
    return initState;
}

/// @brief Creates a new GlobalState using some of the internally known model data and given local states, transitions that were uset to get there and the previous global state. ???
/// @param localStates LocalStates from which the new GlobalState will be built.
/// @param viaLocalTransitions Pointer to a set of pointers to LocalTransition from which the changes in variables, as a result of traversing through the transition, will be made in a new GlobalState. ???
/// @param prevGlobalState Pointer to GlobalState from which all persistent variables will be copied over from to the new GlobalState.
/// @return Returns a pointer to a new or already existing in the same epistemic class GlobalModel.
GlobalState* GlobalModelGenerator::generateStateFromLocalStates(set<LocalState*>* localStates, set<LocalTransition*>* viaLocalTransitions, GlobalState* prevGlobalState) {
    #if VERBOSE
        cout << "GMG:genState" << " : ";
        cout << this->computeGlobalStateHash(localStates) << endl;
    #endif
    
    // Find/create EpistemicClass, check if an identical GlobalState is already present in that EpistemicClass
    auto agent = *this->formula->coalition.begin();
    auto epistemicClass = this->findOrCreateEpistemicClass(localStates, agent);
    auto identicalGlobalState = this->findGlobalStateInEpistemicClass(localStates, epistemicClass);
    if (identicalGlobalState != nullptr) {
        // The state already exists: return GlobalState that was created earlier
        return identicalGlobalState;
    }
    
    // Create a new GlobalState
    auto globalState = new GlobalState();
    globalState->id = this->globalModel->globalStates.size();
    globalState->isExpanded = false;
    globalState->verificationStatus = GlobalStateVerificationStatus::UNVERIFIED;
    
    // globalState->localStates: copy from localStates argument
    for (const auto localState : *localStates) {
        globalState->localStates.insert(localState);
    }
    globalState->hash = this->computeGlobalStateHash(localStates);
    
    // Bind globalState with epistemicClass
    epistemicClass->globalStates.insert({ globalState->hash, globalState });
    globalState->epistemicClasses[agent] = epistemicClass;
    
    // globalState->vars:
    // 1) copy from prevVars (only persistent vars)
    if (prevGlobalState != nullptr) {
        for (const auto varData : prevGlobalState->vars) {
            if (varData.first->persistent) {
                globalState->vars[varData.first] = varData.second;
            }
        }
    }
    // 2) use values from varAssignments in viaLocalTransitions; for var-to-var assignment use values already set in this newly generated state
    if (viaLocalTransitions != nullptr) {
        for (const auto localTransition : *viaLocalTransitions) {
            for (const auto varAssignment : localTransition->varAsssignments) {
                if (varAssignment->type == VarAssignmentType::FromValue) {
                    globalState->vars[varAssignment->dstVar] = varAssignment->value;
                }
                else {
                    if (globalState->vars.count(varAssignment->srcVar) > 0) {
                        globalState->vars[varAssignment->dstVar] = globalState->vars[varAssignment->srcVar];
                    }
                    else {
                        globalState->vars[varAssignment->dstVar] = -1;
                    }
                }
            }
        }
    }
    
    // globalState->globalTransitions:
    // 1) group transitions by name from localStates (only those that can be executed (check sharedCount, check conditions))
    map<string, map<Agent*, vector<LocalTransition*>>> transitionsByName;
    for (const auto localState : *localStates) {
        for (const auto localTransition : localState->localTransitions) {
            if (!this->checkLocalTransitionConditions(localTransition, globalState)) {
                continue;
            }
            if (transitionsByName.count(localTransition->name) == 0) {
               transitionsByName[localTransition->name] = map<Agent*, vector<LocalTransition*>>(); 
            }
            auto transitionsByAgent = &transitionsByName[localTransition->name];
            if (transitionsByAgent->count(localTransition->agent) == 0) {
                transitionsByAgent->insert({ localTransition->agent, vector<LocalTransition*>() });
            }
            transitionsByAgent->at(localTransition->agent).push_back(localTransition);
        }
    }
    // 2) add transitions to globalState if 'shared' conditions are met (!shared or sharedCount ok)
    for (const auto trPair : transitionsByName) {
        const auto sampleTransition = (*trPair.second.begin()).second.at(0);
        if (sampleTransition->isShared && sampleTransition->sharedCount > static_cast<int>(trPair.second.size())) {
            continue;
        }
        auto transitionsByAgent = trPair.second;
        this->generateGlobalTransitions(globalState, set<LocalTransition*>(), transitionsByAgent);
    }
    
    this->globalModel->globalStates.push_back(globalState);
    return globalState;
}

/// @brief Adds all shared global transitions to a GlobalState.
/// @param fromGlobalState Global state to add transitions to.
/// @param localTransitions Initially empty, avaliable local transitions by each agent from transitionsByAgent.
/// @param transitionsByAgent Mapped transitions to an agent, only with transitions avaliable for the agent at this moment.
void GlobalModelGenerator::generateGlobalTransitions(GlobalState* fromGlobalState, set<LocalTransition*> localTransitions, map<Agent*, vector<LocalTransition*>> transitionsByAgent) {
    auto agentWithTransitions = *transitionsByAgent.begin();
    map<Agent*, vector<LocalTransition*>> transitionsByOtherAgents = transitionsByAgent;
    transitionsByOtherAgents.erase(agentWithTransitions.first);
    bool hasOtherAgents = transitionsByOtherAgents.size() > 0;
    for (const auto transition : agentWithTransitions.second) {
        auto currentLocalTransitions = localTransitions;
        currentLocalTransitions.insert(transition);
        if (hasOtherAgents) {
            this->generateGlobalTransitions(fromGlobalState, currentLocalTransitions, transitionsByOtherAgents);
        }
        else {
            auto globalTransition = new GlobalTransition();
            globalTransition->id = this->globalModel->globalTransitions.size();
            globalTransition->isInvalidDecision = false;
            globalTransition->from = fromGlobalState;
            globalTransition->to = nullptr;
            globalTransition->localTransitions = currentLocalTransitions;
            this->globalModel->globalTransitions.push_back(globalTransition);
            fromGlobalState->globalTransitions.insert(globalTransition);
        }
    }
}

/// @brief Checks if all conditions for a given local transition in a given global state are fulfilled.
/// @param localTransition Local transition to traverse.
/// @param globalState Current global state.
/// @return Returns true if all of the necessary conditions to use that transition are fulfilled, otherwise false.
bool GlobalModelGenerator::checkLocalTransitionConditions(LocalTransition* localTransition, GlobalState* globalState) {
    for (const auto condition : localTransition->conditions) {
        auto currentValue = globalState->vars[condition->var];
        if (condition->conditionOperator == ConditionOperator::Equals) {
            if (currentValue != condition->comparedValue) {
                return false;
            }
        }
        else if (condition->conditionOperator == ConditionOperator::NotEquals) {
            if (currentValue == condition->comparedValue) {
                return false;
            }
        }
    }
    return true;
}

/// @brief Creates a hash from a set of LocalState and an Agent.
/// @param localStates Pointer to a set of pointers of LocalState and pointer to and Agent to turn into a hash.
/// @return Returns a string with a hash.
string GlobalModelGenerator::computeEpistemicClassHash(set<LocalState*>* localStates, Agent* agent) {
    string hash = "";
    for (const auto localState : *localStates) {
        if (localState->agent == agent) {
            hash = to_string(localState->id);
            break;
        }
    }
    return hash;
}

/// @brief Creates a hash from a set of LocalState.
/// @param localStates Pointer to a set of pointers of LocalState to turn into a hash.
/// @return Returns a string with a hash.
string GlobalModelGenerator::computeGlobalStateHash(set<LocalState*>* localStates) {
    string hash = "";
    for (const auto localState : *localStates) {
        hash.append(to_string(localState->id) + ";");
    }
    return hash;
}

/// @brief Checks if a set of LocalState is already an epistemic class for a given Agent, if not, creates a new one.
/// @param localStates Local states from agent.
/// @param agent Agent for which to check the existence of an epistemic class.
/// @return A pointer to a new or existing EpistemicClass.
EpistemicClass* GlobalModelGenerator::findOrCreateEpistemicClass(set<LocalState*>* localStates, Agent* agent) {
    string hash = this->computeEpistemicClassHash(localStates, agent);
    if (this->globalModel->epistemicClasses.find(agent) == this->globalModel->epistemicClasses.end()) {
        this->globalModel->epistemicClasses.insert({ agent, map<string, EpistemicClass*>() });
    }
    auto epistemicClassesForAgent = &this->globalModel->epistemicClasses[agent];
    if (epistemicClassesForAgent->find(hash) == epistemicClassesForAgent->end()) {
        EpistemicClass* epistemicClass = new EpistemicClass();
        epistemicClass->hash = hash;
        epistemicClass->fixedCoalitionTransition = nullptr;
        epistemicClassesForAgent->insert({ hash, epistemicClass });
    }
    return epistemicClassesForAgent->at(hash);
}

/// @brief Gets a GlobalState from an EpistemicClass if it exists in that episcemic class.
/// @param localStates Pointer to a set of pointers to LocalState, from which will be generated a global state hash.
/// @param epistemicClass Epistemic class in which to check if a GlobalState exists.
/// @return Returns a pointer to a GlobalState if it exists in given epistemic class, otherwise returns nullptr.
GlobalState* GlobalModelGenerator::findGlobalStateInEpistemicClass(set<LocalState*>* localStates, EpistemicClass* epistemicClass) {
    string hash = this->computeGlobalStateHash(localStates);
    if (epistemicClass->globalStates.count(hash) == 0) {
        return nullptr;
    }
    return epistemicClass->globalStates[hash];
}
