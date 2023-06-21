/**
 * @file GlobalModelGenerator.cpp
 * @brief Generator of a global model.
 * Class for initializing and generating a global model.
 */

#include "GlobalModelGenerator.hpp"
#include "Types.hpp"


#include <algorithm>
#include <iostream>

/// @brief Constructor for GlobalModelGenerator class.
GlobalModelGenerator::GlobalModelGenerator() {
}

/// @brief Initializes a global model from local models and a formula.
/// @param localModels Pointer to LocalModels that will construct a global model.
/// @param formula Pointer to a Formula to include into the model.
/// @return Returns a pointer to initial state of the global model.
shared_ptr<GlobalState> GlobalModelGenerator::initModel(shared_ptr<LocalModels> localModels, shared_ptr<Formula> formula) {
    this->localModels = localModels;
    this->formula = formula;
    this->globalModel = make_shared<GlobalModel>();
    this->globalModel->agents = localModels->agents;
    this->globalModel->initState = this->generateInitState();

    for (auto it = begin (this->globalModel->agents); it != end (this->globalModel->agents); ++it) {
        this->agentIndex[*it]=std::distance(this->globalModel->agents.begin(), it);
    }
    return this->globalModel->initState;
}

/// @brief Goes through all GlobalTransition in a given GlobalState and creates new GlobalStates connected to the given one.
/// @param state A state from which the expansion should start.
void GlobalModelGenerator::expandState(shared_ptr<GlobalState> state) {
    if (state->isExpanded) {
        return;
    }
    for (const auto globalTransition : state->globalTransitions) {
        if (globalTransition->to == nullptr) {
            vector<shared_ptr<LocalState>> localStates = state->localStatesProjection;
            for (const auto localTransition : globalTransition->localTransitions) {
                #if VERBOSE
                    cout << "expandState: " << localTransition->name << " : " << localTransition->from << " -> " << localTransition->to << endl;
                #endif

                auto it = agentIndex[localTransition->from->agent];
                // if(it<0)cout << "ERR" << endl;
                localStates[it]=localTransition->to;
                // localStates.erase(localTransition->from);
                // localStates.insert(localTransition->to);
            }            
            auto targetState = this->generateStateFromLocalStates(make_shared<vector<shared_ptr<LocalState>>>(&localStates), make_shared<set<shared_ptr<LocalTransition>>>(&globalTransition->localTransitions), state);
            globalTransition->to = targetState;
        }
    }
    state->isExpanded = true;
}

/// @brief GlobalModelGenerator::expandState that also additionally returns a vector of newly created states
/// @param state A state from which the expansion should start.
vector<shared_ptr<GlobalState>> GlobalModelGenerator::expandStateAndReturn(shared_ptr<GlobalState> state) {
    size_t cardinalityBefore;
    vector<shared_ptr<GlobalState>> res;
    if (state->isExpanded) {
        return res;
    }
    for (const auto globalTransition : state->globalTransitions) {
        if (globalTransition->to == nullptr) {
            vector<shared_ptr<LocalState>> localStates = state->localStatesProjection;
            for (const auto localTransition : globalTransition->localTransitions) {
                localStates[agentIndex[localTransition->from->agent]]=localTransition->to;
                // localStates.erase(localTransition->from);
                // localStates.insert(localTransition->to);
            }
            cardinalityBefore = this->globalModel->globalStates.size();
            // either returns a new state OR an existing one
            auto targetState = this->generateStateFromLocalStates(make_shared<vector<shared_ptr<LocalState>>>(&localStates), make_shared<set<shared_ptr<LocalTransition>>>(globalTransition->localTransitions), state);
            // when new state was indeed created - append that to result
            if(cardinalityBefore!=this->globalModel->globalStates.size()){   
                res.push_back(targetState);
            }
            globalTransition->to = targetState;
        }
    }
    state->isExpanded = true;
    return res;
}

/// @brief Expands the states starting from the initial GlobalState and continues until there are no more states to expand.
void GlobalModelGenerator::expandAllStates() {
    set<shared_ptr<GlobalState>> statesToExpand;
    statesToExpand.insert(this->globalModel->initState);
    while (!statesToExpand.empty()) {
        auto globalState = *statesToExpand.begin();
        statesToExpand.erase(globalState);
        #if VERBOSE
            printf("\nExpanding %s (%s)\n", globalState->hash, globalState->hash.c_str());
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
shared_ptr<GlobalModel> GlobalModelGenerator::getCurrentGlobalModel() {
    return this->globalModel;
}

/// @brief Get for the Formula used in initialization.
/// @return Returns a pointer to the formula structure.
shared_ptr<Formula> GlobalModelGenerator::getFormula() {
    return this->formula;
}

/// @brief Generates initial state of the model from GlobalModel in memory.
/// @return Returns a pointer to an initial GlobalState.
shared_ptr<GlobalState> GlobalModelGenerator::generateInitState() {
    vector<shared_ptr<LocalState>> localStates;
    for (const auto agt : this->globalModel->agents) {
        localStates.push_back(agt->initState);
    }
    auto initState = this->generateStateFromLocalStates(make_shared<vector<shared_ptr<LocalState>>>(&localStates), nullptr, nullptr);
    
    return initState;
}

/// @brief Creates a new GlobalState using some of the internally known model data and given local states, transitions that were uset to get there and the previous global state.
/// @param localStates LocalStates from which the new GlobalState will be built.
/// @param viaLocalTransitions Pointer to a set of pointers to LocalTransition from which the changes in variables, as a result of traversing through the transition, will be made in a new GlobalState.
/// @param prevGlobalState Pointer to GlobalState from which all persistent variables will be copied over from to the new GlobalState.
/// @return Returns a pointer to a new or already existing in the same epistemic class GlobalModel.
shared_ptr<GlobalState> GlobalModelGenerator::generateStateFromLocalStates(shared_ptr<vector<shared_ptr<LocalState>>> localStates, shared_ptr<set<shared_ptr<LocalTransition>>> viaLocalTransitions, shared_ptr<GlobalState> prevGlobalState) {
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
    auto globalState = make_shared<GlobalState>();
    // Reserve vector capacity
    globalState->localStatesProjection.reserve(localStates->size());
    
    // globalState->localStates: copy from localStates argument
    for (const auto localState : *localStates) {
        globalState->localStatesProjection.push_back(localState);
    }
    globalState->hash = this->computeGlobalStateHash(localStates);
    
    // Bind globalState with epistemicClass
    epistemicClass->globalStates.insert({ globalState->hash, globalState });
    globalState->epistemicClasses[agent] = epistemicClass;
    
    // globalState->globalTransitions:
    // 1) group transitions by name from localStates (only those that can be executed (check sharedCount, check conditions))
    map<string, map<shared_ptr<Agent>, vector<shared_ptr<LocalTransition>>>> transitionsByName;
    for (const auto localState : *localStates) {
        for (const auto localTransition : localState->localTransitions) {
            if (transitionsByName.count(localTransition->name) == 0) {
               transitionsByName[localTransition->name] = map<shared_ptr<Agent>, vector<shared_ptr<LocalTransition>>>(); 
            }
            auto transitionsByAgent = &transitionsByName[localTransition->name];
            if (transitionsByAgent->count(localTransition->agent) == 0) {
                transitionsByAgent->insert({ localTransition->agent, vector<shared_ptr<LocalTransition>>() });
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
        this->generateGlobalTransitions(globalState, set<shared_ptr<LocalTransition>>(), transitionsByAgent);
    }
    
    this->globalModel->globalStates.push_back(globalState);
    return globalState;
}

/// @brief Adds all shared global transitions to a GlobalState.
/// @param fromGlobalState Global state to add transitions to.
/// @param localTransitions Initially empty, avaliable local transitions by each agent from transitionsByAgent.
/// @param transitionsByAgent Mapped transitions to an agent, only with transitions avaliable for the agent at this moment.
void GlobalModelGenerator::generateGlobalTransitions(shared_ptr<GlobalState> fromGlobalState, set<shared_ptr<LocalTransition>> localTransitions, map<shared_ptr<Agent>, vector<shared_ptr<LocalTransition>>> transitionsByAgent) {
    auto agentWithTransitions = *transitionsByAgent.begin();
    map<shared_ptr<Agent>, vector<shared_ptr<LocalTransition>>> transitionsByOtherAgents = transitionsByAgent;
    transitionsByOtherAgents.erase(agentWithTransitions.first);
    bool hasOtherAgents = transitionsByOtherAgents.size() > 0;
    for (const auto transition : agentWithTransitions.second) {
        auto currentLocalTransitions = localTransitions;
        currentLocalTransitions.insert(transition);
        if (hasOtherAgents) {
            this->generateGlobalTransitions(fromGlobalState, currentLocalTransitions, transitionsByOtherAgents);
        }
        else {
            auto globalTransition = make_shared<GlobalTransition>();
            // globalTransition->id = this->globalModel->globalTransitions.size();
            globalTransition->isInvalidDecision = false;
            globalTransition->from = fromGlobalState;
            globalTransition->to = nullptr;
            globalTransition->localTransitions = currentLocalTransitions;
            fromGlobalState->globalTransitions.insert(globalTransition);
        }
    }
}

/// @brief Creates a hash from a set of LocalState and an Agent.
/// @param localStates Pointer to a vector of pointers of LocalState and pointer to and Agent to turn into a hash.
/// @return Returns a string with a hash.
string GlobalModelGenerator::computeEpistemicClassHash(shared_ptr<vector<shared_ptr<LocalState>>> localStates, shared_ptr<Agent> agent) {
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
/// @param localStates Pointer to a vector of pointers of LocalState to turn into a hash.
/// @return Returns a string with a hash.
string GlobalModelGenerator::computeGlobalStateHash(shared_ptr<vector<shared_ptr<LocalState>>> localStates) {
    string hash = "";
    for (const auto localState : *localStates) {
        hash.append(to_string(localState->id) + ";");
    }
    return hash;
}

/// @brief Checks if a vector of LocalState is already an epistemic class for a given Agent, if not, creates a new one.
/// @param localStates Local states from agent.
/// @param agent Agent for which to check the existence of an epistemic class.
/// @return A pointer to a new or existing EpistemicClass.
shared_ptr<EpistemicClass> GlobalModelGenerator::findOrCreateEpistemicClass(shared_ptr<vector<shared_ptr<LocalState>>> localStates, shared_ptr<Agent> agent) {
    string hash = this->computeEpistemicClassHash(localStates, agent);
    if (this->globalModel->epistemicClasses.find(agent) == this->globalModel->epistemicClasses.end()) {
        this->globalModel->epistemicClasses.insert({ agent, map<string, shared_ptr<EpistemicClass>>() });
    }
    auto epistemicClassesForAgent = &this->globalModel->epistemicClasses[agent];
    if (epistemicClassesForAgent->find(hash) == epistemicClassesForAgent->end()) {
        shared_ptr<EpistemicClass> epistemicClass = make_shared<EpistemicClass>();
        epistemicClass->hash = hash;
        epistemicClass->fixedCoalitionTransition = nullptr;
        epistemicClassesForAgent->insert({ hash, epistemicClass });
    }
    return epistemicClassesForAgent->at(hash);
}

/// @brief Gets a GlobalState from an EpistemicClass if it exists in that episcemic class.
/// @param localStates Pointer to a vector of pointers to LocalState, from which will be generated a global state hash.
/// @param epistemicClass Epistemic class in which to check if a GlobalState exists.
/// @return Returns a pointer to a GlobalState if it exists in given epistemic class, otherwise returns nullptr.
shared_ptr<GlobalState> GlobalModelGenerator::findGlobalStateInEpistemicClass(shared_ptr<vector<shared_ptr<LocalState>>> localStates, shared_ptr<EpistemicClass> epistemicClass) {
    string hash = this->computeGlobalStateHash(localStates);
    if (epistemicClass->globalStates.count(hash) == 0) {
        return nullptr;
    }
    return epistemicClass->globalStates[hash];
}
