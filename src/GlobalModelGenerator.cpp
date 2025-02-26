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
#include <sstream>

extern Cfg config;

/// @brief Constructor for GlobalModelGenerator class.
GlobalModelGenerator::GlobalModelGenerator() {
    while(!this->stateDepths.empty()){
        this->stateDepths.pop();
    }
    this->candidateStateDepths.clear();
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

    globalModelCandidates.push(this->globalModel->initState);
    auto candidateDepthPtr = candidateStateDepths.find(this->globalModel->initState);
    if(candidateDepthPtr == candidateStateDepths.end()) {
        candidateStateDepths.insert({this->globalModel->initState, unordered_set<int>({0})});
    } else {
        candidateDepthPtr->second.insert(0);
    }

    Agent* a;
    for (auto agt : globalModel->agents) {
        a = agt;
        set<GlobalState*>* states = this->findOrCreateEpistemicClassForKnowledge(&this->globalModel->initState->localStatesProjection, this->globalModel->initState, a);
        this->globalModel->initState->epistemicClassesAllAgents[a] = states;
    }

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
            targetState->preimage.insert(state);
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
vector<GlobalState*> GlobalModelGenerator::expandStateAndReturn(GlobalState* state, bool returnAnyway) {
    size_t cardinalityBefore;
    vector<GlobalState*> res;
    if (state->isExpanded) {
        #if VERBOSE
            printf("\nState %s was already expanded\n", state->hash.c_str());
        #endif
        if(returnAnyway) {
            res.push_back(state);
        }
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

/// @brief Expands and reduces the states starting from the initial GlobalState using a DFS-POR algorithm and continues until there are no more states to expand.
/// @param depth Current depth of the recursive generation of all states.
void GlobalModelGenerator::expandAndReduceAllStates() {
    int depth;
    bool reexplore = false;
    GlobalState* g = globalModelCandidates.top(); //1
    auto findIfCandidateExists = candidateStateDepths.find({g});
    if(findIfCandidateExists->second.size() >= 2) { //2
        depth = stateDepths.top(); //3
        int anotherDepth = -1;
        for(auto item : findIfCandidateExists->second){
            if(item != globalModelCandidates.size() - 1) {
                anotherDepth = item;
                break;
            }
        }
        if(anotherDepth > depth) { //4
            reexplore = true;
        } else {
            globalModelCandidates.pop();
            candidateStateDepths.find(g)->second.erase(globalModelCandidates.size());
            return;
        }
    } //5

    if(!reexplore) { //6
        if(this->addedStates.find(g) != addedStates.end()) {
            globalModelCandidates.pop();
            candidateStateDepths.find(g)->second.erase(globalModelCandidates.size());
            return;    
        }
    } else {
        globalModelCandidates.pop();
        candidateStateDepths.find(g)->second.erase(globalModelCandidates.size());
        return;
    }
    this->addedStates.insert(g); //7
    set<GlobalTransition*> allAvaliableTransitions;
    set<GlobalTransition*> selectedTransitions;

    allAvaliableTransitions.insert(g->globalTransitions.begin(), g->globalTransitions.end());

    stringstream ss(config.reduce_args);
    string s;
    set<string> reductionImportantVariables; //temp
    while(getline(ss, s, ' ')) {
        cout << s << endl;
        reductionImportantVariables.insert(s);
    } //getting lost here
    
    bool isOk = true;
    set<Agent*> agentsInTransition;
    set<Agent*> agentsInTransition2;
    set<Agent*> intersectResult;

    if(allAvaliableTransitions.size() != 0) { //8
        if(reexplore == false) { //9
            for(auto transitionCandidate : allAvaliableTransitions) { //10
                isOk = true;
                cout << transitionCandidate->from->hash << endl;
                agentsInTransition.clear();
                agentsInTransition2.clear();
                intersectResult.clear();
                for(auto localTransitionsInCandidate : transitionCandidate->localTransitions) { //11
                    cout << "candidate " << localTransitionsInCandidate->from->name << " => " << localTransitionsInCandidate->to->name << endl;
                    auto coalitionAgents = this->getFormula()->coalition;
                    for(auto agent : coalitionAgents) { //11.2 (Checks if any agent from coalition didn't change the place)
                        if(agent->name == localTransitionsInCandidate->agent->name) { // to change when there's more agents in a coalition
                            string stateFrom = localTransitionsInCandidate->from->name;
                            string stateTo = localTransitionsInCandidate->to->name;
                            if(stateFrom != stateTo) {
                                isOk = false;
                                cout << "[!]state changed[!] ";
                                cout << agent->name << ": " << stateFrom << " -> " << stateTo << endl;
                                break;
                            }
                        }
                    }
                    if(!isOk) {
                        break;
                    }

                    map<string, int> environmentFrom = localTransitionsInCandidate->from->environment; //11.1 (Checks if the variable for the reduction wasn't changed in the local transition)
                    map<string, int> environmentTo = localTransitionsInCandidate->to->environment;
                    for(auto firstKey : environmentFrom) {
                        if(reductionImportantVariables.find(firstKey.first) != reductionImportantVariables.end()) {
                            int secondValue = environmentTo.find(firstKey.first)->second;
                            if(firstKey.second != secondValue) {
                                isOk = false;
                                cout << "[!]value changed[!] ";
                                cout << firstKey.first << " = " << firstKey.second << " | " << secondValue << endl;
                                break;
                            }
                        }
                    }
                    
                    if(!isOk) {
                        break;
                    }

                    agentsInTransition.insert(localTransitionsInCandidate->agent);
                }
                cout << "----------" << endl;
                if(!isOk) {
                    continue;
                }
                //11.3
                set<GlobalTransition*> globalTransitionsCopy;
                globalTransitionsCopy.insert(allAvaliableTransitions.begin(), allAvaliableTransitions.end());
                globalTransitionsCopy.erase(transitionCandidate);
                for(auto transitionCandidate2 : globalTransitionsCopy) {
                    for(auto localTransitionsInCandidate : transitionCandidate2->localTransitions) {
                        agentsInTransition2.insert(localTransitionsInCandidate->agent);
                    }
                }
                set_intersection(agentsInTransition.begin(), agentsInTransition.end(), agentsInTransition2.begin(), agentsInTransition2.end(), inserter(intersectResult, intersectResult.begin()));
                if(!intersectResult.empty()) {
                    cout << "not empty" << endl;
                    continue;
                }
                cout << (*transitionCandidate->localTransitions.begin())->from->name << " >> " << (*transitionCandidate->localTransitions.begin())->to->name << endl;
                selectedTransitions.insert(transitionCandidate);
                if(!config.reduce_all) {
                    break;
                }
            } //12
        } //13
        if(selectedTransitions.size() == 0) { //14
            selectedTransitions.clear();
            selectedTransitions.insert(allAvaliableTransitions.begin(), allAvaliableTransitions.end());
        }
        if(selectedTransitions.size() == allAvaliableTransitions.size()) { //15
            stateDepths.push(globalModelCandidates.size());
        }
        g->globalTransitions.clear();
        g->globalTransitions.insert(selectedTransitions.begin(), selectedTransitions.end());
        expandState(g);
        unordered_set<GlobalState*> createdStates;
        for(auto item : g->globalTransitions) {
            createdStates.insert(item->to);
            // cout << item->from->hash << endl;
            // cout << item->to->hash << endl;
            // cout << "here1" << endl;
            // vector<GlobalState*> tempStates = expandStateAndReturn(g, true);
            // cout << "newly created states (" << tempStates.size() << ")" << endl;
            // createdStates.insert(tempStates.begin(), tempStates.end());

        }
        //16 (Or maybe we don't have to connect the states after all?)
        for(auto newGlobalState : createdStates) {
            globalModelCandidates.push(newGlobalState);
            auto candidateDepthPtr = candidateStateDepths.find(newGlobalState);
            if(candidateDepthPtr == candidateStateDepths.end()) {
                candidateStateDepths.insert({newGlobalState, unordered_set<int>({globalModelCandidates.size() - 1})});
            } else {
                candidateDepthPtr->second.insert(globalModelCandidates.size() - 1);
            }
            expandAndReduceAllStates();
        }
    } //17

    depth = stateDepths.top(); //18
    if(depth == globalModelCandidates.size()) { //19
        stateDepths.pop();
    }
    globalModelCandidates.pop(); //20
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

    // Agent* a;
    // for (auto agt : globalModel->agents) {
    //     a = agt;
    //     set<GlobalState*>* states;
    //     states->insert(initState);
    //     initState->epistemicClassesAllAgents[a] = states;
    // }
    // cout << "----------" << endl;

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

    Agent* a;
    for (auto agt : globalModel->agents) {
        a = agt;
        set<GlobalState*>* states = this->findOrCreateEpistemicClassForKnowledge(localStates, globalState, a);
        globalState->epistemicClassesAllAgents[a] = states;
    }
    
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