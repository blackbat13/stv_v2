/**
 * @file GlobalModelGenerator.cpp
 * @brief Generator of a global model.
 * Class for initializing and generating a global model.
 */

#include "GlobalModelGenerator.hpp"
#include "Types.hpp"
#include "Constants.hpp"

#include <algorithm>
#include <string.h>
#include <iostream>

extern Cfg config;

/// @brief Constructor for GlobalModelGenerator class.
GlobalModelGenerator::GlobalModelGenerator() {
}

/// @brief Destructor for GlobalModelGenerator class.
GlobalModelGenerator::~GlobalModelGenerator() {
    delete(this->globalModel);
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

    for (auto it = begin (this->globalModel->agents); it != end (this->globalModel->agents); ++it) {
        this->agentIndex[*it]=std::distance(this->globalModel->agents.begin(), it);
    }

    this->globalModel->initState = this->generateInitState();
    this->globalModel->epistemicClassesKnowledge.clear();
    this->correctModel = true;

    return this->globalModel->initState;
}

/// @brief Goes through all GlobalTransition in a given GlobalState and creates new GlobalStates connected to the given one.
/// @param state A state from which the expansion should start.
void GlobalModelGenerator::expandState(GlobalState* state) {
    if (state->isExpanded) {
        #if VERBOSE
            printf("\nState %s was already expanded\n", state->hash.c_str());
        #endif
        return;
    }
    for (const auto globalTransition : state->globalTransitions) {
        if (globalTransition->to == nullptr) {
            vector<LocalState*> localStates = state->localStatesProjection;
            for (const auto localTransition : globalTransition->localTransitions) {
                #if VERBOSE
                    cout << "expandState via (localTransition) " << localTransition->name << " : " << localTransition->from->id << " -> " << localTransition->to->id << endl;
                #endif
                auto it = agentIndex[localTransition->from->agent];
                localStates[it]=localTransition->to;
            }            
            auto targetState = this->generateStateFromLocalStates(&localStates, &globalTransition->localTransitions, state);
            globalTransition->to = targetState;
        }
    }
    // add optional epsilon transition if no more states to expand to
    if (config.add_epsilon_transitions && state->globalTransitions.size() == 0) {
        set<LocalTransition*> epsilon;
        Agent* agent = *this->getFormula()->coalition.begin();
        LocalState* localState;
        for (auto state : state->localStatesProjection) {
            if (state->agent->name == agent->name) {
                localState = state;
            }
        }

        LocalTransition* transition = new LocalTransition;
        transition->id = -1;
        transition->isShared = 0;
        transition->name = "ɛ";
        transition->localName = "ɛ";
        transition->sharedCount = 0;
        transition->agent = agent;
        transition->from = localState;
        transition->to = localState;
        epsilon.insert(transition);

        auto globalTransition = new GlobalTransition();
        // globalTransition->id = this->globalModel->globalTransitions.size();
        globalTransition->isInvalidDecision = false;
        globalTransition->from = state;
        globalTransition->to = state;
        globalTransition->localTransitions = epsilon;
        state->globalTransitions.insert(globalTransition);
        // printf("added a state!\n");
    }
    state->isExpanded = true;
}

/// @brief GlobalModelGenerator::expandState that also additionally returns a vector of newly created states
/// @param state A state from which the expansion should start.
vector<GlobalState*> GlobalModelGenerator::expandStateAndReturn(GlobalState* state) {
    size_t cardinalityBefore;
    vector<GlobalState*> res;
    if (state->isExpanded) {
        #if VERBOSE
            printf("\nState %s was already expanded\n", state->hash.c_str());
        #endif
        return res;
    }
    for (const auto globalTransition : state->globalTransitions) {
        if (globalTransition->to == nullptr) {
            vector<LocalState*> localStates = state->localStatesProjection;
            for (const auto localTransition : globalTransition->localTransitions) {
                localStates[agentIndex[localTransition->from->agent]]=localTransition->to;
                // localStates.erase(localTransition->from);
                // localStates.insert(localTransition->to);
            }
            cardinalityBefore = this->globalModel->globalStates.size();
            // either returns a new state OR an existing one
            auto targetState = this->generateStateFromLocalStates(&localStates, &globalTransition->localTransitions, state);
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
    if(this->globalModel->initState->isExpanded){
        #if VERBOSE
            printf("\nInitial state %s was already expanded\n", this->globalModel->initState->hash.c_str());
        #endif
        return;
    }
    set<GlobalState*> statesToExpand;
    statesToExpand.insert(this->globalModel->initState);
    while (!statesToExpand.empty()) {
        auto globalState = *statesToExpand.begin();
        statesToExpand.erase(globalState);
        #if VERBOSE
            printf("\nExpanding %s\n", globalState->hash.c_str());
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

/// @brief Get size of the Formula used in initialization.
/// @return Returns the formula size.
int GlobalModelGenerator::getFormulaSize() {
    return this->formula->p->size();
}

/// @brief Generates initial state of the model from GlobalModel in memory.
/// @return Returns a pointer to an initial GlobalState.
GlobalState* GlobalModelGenerator::generateInitState() {
    vector<LocalState*> localStates;
    for (const auto agt : this->globalModel->agents) {
        localStates.push_back(agt->initState);
    }
    auto initState = this->generateStateFromLocalStates(&localStates, nullptr, nullptr);

    return initState;
}

/// @brief Creates a new GlobalState using some of the internally known model data and given local states, transitions that were uset to get there and the previous global state.
/// @param localStates LocalStates from which the new GlobalState will be built.
/// @param viaLocalTransitions Pointer to a set of pointers to LocalTransition from which the changes in variables, as a result of traversing through the transition, will be made in a new GlobalState.
/// @param prevGlobalState Pointer to GlobalState from which all persistent variables will be copied over from to the new GlobalState.
/// @return Returns a pointer to a new or already existing in the same epistemic class GlobalModel.
GlobalState* GlobalModelGenerator::generateStateFromLocalStates(vector<LocalState*>* localStates, set<LocalTransition*>* viaLocalTransitions, GlobalState* prevGlobalState) {   
    // Find/create EpistemicClass, check if an identical GlobalState is already present in that EpistemicClass
    auto agent = *this->formula->coalition.begin();
    auto epistemicClass = this->findOrCreateEpistemicClass(localStates, agent);
    auto identicalGlobalState = this->findGlobalStateInEpistemicClass(localStates, epistemicClass);
    if (identicalGlobalState != nullptr) {
        // The state already exists: return GlobalState that was created earlier
        #if VERBOSE
        cout << "GMG: globalState " << this->computeGlobalStateHash(localStates) << " already exists" << endl;
        #endif
        return identicalGlobalState;
    }
    #if VERBOSE
    cout << "GMG: genState " << this->computeGlobalStateHash(localStates) << endl;
    #endif
    
    // Create a new GlobalState
    auto globalState = new GlobalState();
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
    map<string, map<Agent*, vector<LocalTransition*>>> transitionsByName;
    for (const auto localState : *localStates) {
        for (const auto localTransition : localState->localTransitions) {
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

    Agent* a;
    for (auto agt : globalModel->agents) {
        a = agt;
        this->findOrCreateEpistemicClassForKnowledge(localStates, globalState, a);
    }

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
            if (config.add_epsilon_transitions == 1) {
                for (auto agentCheck : transitionsByOtherAgents) {
                    auto state = **find(agentCheck.first->localStates.begin(), agentCheck.first->localStates.end(), agentCheck.second[0]->from);
                    if (state.localTransitions.size() > 1) {
                        for (auto possibility : state.localTransitions) {
                            if (transition->name != possibility->name) {
                                set<LocalTransition*> sumTransitions = currentLocalTransitions;
                                sumTransitions.insert(possibility);
                                auto globalTransition = new GlobalTransition();
                                // globalTransition->id = this->globalModel->globalTransitions.size();
                                globalTransition->isInvalidDecision = false;
                                globalTransition->from = fromGlobalState;
                                globalTransition->to = fromGlobalState;
                                globalTransition->localTransitions = sumTransitions;
                                fromGlobalState->globalTransitions.insert(globalTransition);
                            }
                        }
                    }
                }
            }
            this->generateGlobalTransitions(fromGlobalState, currentLocalTransitions, transitionsByOtherAgents);
        }
        else {
            auto globalTransition = new GlobalTransition();
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
string GlobalModelGenerator::computeEpistemicClassHash(vector<LocalState*>* localStates, Agent* agent) {
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
string GlobalModelGenerator::computeGlobalStateHash(vector<LocalState*>* localStates) {
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
EpistemicClass* GlobalModelGenerator::findOrCreateEpistemicClass(vector<LocalState*>* localStates, Agent* agent) {
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

/// @brief Checks if a vector of LocalState is already an epistemic class for a given Agent, if not, creates a new one.
/// @param localStates Local states from agent.
/// @param agent Agent for which to check the existence of an epistemic class.
/// @return A pointer to a new or existing EpistemicClass.
set<GlobalState*>* GlobalModelGenerator::findOrCreateEpistemicClassForKnowledge(vector<LocalState*>* localStates, GlobalState* global, Agent* agent) {
    string hash = this->computeEpistemicClassHash(localStates, agent);
    if (this->globalModel->epistemicClassesKnowledge.find(agent) == this->globalModel->epistemicClassesKnowledge.end()) {
        this->globalModel->epistemicClassesKnowledge.insert({ agent, map<string, set<GlobalState*>>() });
    }
    auto epistemicClassesForAgent = &this->globalModel->epistemicClassesKnowledge[agent];
    if (epistemicClassesForAgent->find(hash) == epistemicClassesForAgent->end()) {
        set<GlobalState*> epistemicClass;
        epistemicClass.insert(global);
        epistemicClassesForAgent->insert({ hash, epistemicClass });
    }
    else {
        auto s = epistemicClassesForAgent->find(hash);
        s->second.insert(global);
    }
    return &epistemicClassesForAgent->at(hash);
}

/// @brief Gets a GlobalState from an EpistemicClass if it exists in that episcemic class.
/// @param localStates Pointer to a vector of pointers to LocalState, from which will be generated a global state hash.
/// @param epistemicClass Epistemic class in which to check if a GlobalState exists.
/// @return Returns a pointer to a GlobalState if it exists in given epistemic class, otherwise returns nullptr.
GlobalState* GlobalModelGenerator::findGlobalStateInEpistemicClass(vector<LocalState*>* localStates, EpistemicClass* epistemicClass) {
    string hash = this->computeGlobalStateHash(localStates);
    if (epistemicClass->globalStates.count(hash) == 0) {
        return nullptr;
    }
    return epistemicClass->globalStates[hash];
}

/// @brief Get a pointer to an agent by using its name.
/// @param agentName Name of an Agent that we want to get its instance.
/// @return Pointer to an Agent.
Agent* GlobalModelGenerator::getAgentInstanceByName(string agentName) {
    Agent* agentInstance = nullptr;
    auto globalModelAgents = globalModel->agents;
    for (auto globalAgent : globalModelAgents) {
        if (globalAgent->name == agentName) {
            agentInstance = globalAgent;
            break;
        }
    }
    return agentInstance;
}

/// @brief Sets formula to an incorrectly written state.
void GlobalModelGenerator::markFormulaAsIncorrect() {
    correctModel = false;
}

/// @brief Gets formula status.
/// @return True if formula is written correctly, false otherwise.
bool GlobalModelGenerator::getFormulaCorectness() {
    return correctModel;
}