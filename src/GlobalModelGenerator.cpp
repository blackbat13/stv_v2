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
#include <functional>

#include "craam/RMDP.hpp"
#include "craam/algorithms/values.hpp"
#include "craam/modeltools.hpp"

using namespace craam;

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

    if(this->formula->probabilitySign != ProbabilitySign::NONE) {
        config.probability = true;
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
void GlobalModelGenerator::expandAllStates(bool additionalProbSplit) {
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
            // sort transitions into two buckets for probability model generation
            if (additionalProbSplit) {
                // // transition switches from a FALSE state to a TRUE state or is from a state that was somewhere correct along the line
                // bool goodTransition = false;
                // if (transition->from->goodState || (checkLocalStates(&transition->from->localStatesProjection, transition->from) == false && checkLocalStates(&transition->to->localStatesProjection, transition->to) == true)) {
                //     goodTransition = true;
                //     // create a new state here?
                //     transition->to->goodState = true;
                // }

                bool isControlled = false;
                for (const auto localTransition : transition->localTransitions) {
                    if (formula->coalition.find(localTransition->agent) != formula->coalition.end()) {
                        if (!localTransition->isShared || localTransition->name == localTransition->localName) {
                            isControlled = true;
                            break;
                        }
                    }
                }
                if (isControlled) {
                    coalitionTransitions[globalState->hash][transition->joinLocalTransitionNames()].insert(transition);
                } else {
                    opponentsTransitions[globalState->hash][transition->joinLocalTransitionNames()].insert(transition);
                }
            }
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
    initState->probability = 1.0;
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

/// @brief Initiates the strategy with a given StrategyCollection.
/// @param strat StrategyCollection to initialize the strategy.
void GlobalModelGenerator::initStrategy(StrategyCollection* strat)
{
    this->strategyCollection = strat;
}

/// @brief Prepares the strategy generating structures for the probabilistic verification.
/// @param localModels LocalModels to generate the coalition transition buckets from.
void GlobalModelGenerator::createIterativeStrategy(LocalModels* localModels)
{
    ProbabilityStrategyDecisions newProbabilityStrategy;
    Agent* agent = *this->formula->coalition.begin();
    auto localModel = localModels->agents[agent->id]->localTransitions;
    for (auto trans : localModel) {
      Action action;
      action.actionName = trans->localName;
      action.hash = trans->from->name + ";";
      action.states = new vector<string>(1, trans->from->name);

      // add each strategy to a good bucket
      cout << "Adding action " << action.actionName << " for state " << action.hash << endl;
      newProbabilityStrategy.allCoalitionTransitions[agent][action.hash].push_back(StrategyCollection(action.hash, action));
      cout << "Bucket size: " << newProbabilityStrategy.allCoalitionTransitions[agent][action.hash].size() << endl;
      newProbabilityStrategy.currentStrategyPermutation[action.hash] = 0;
      // cout << action.hash << " " << action.actionName << endl;
      //strategyCollection->addAction(action);
    }
    this->probabilityStrategy = newProbabilityStrategy;
    // create the first strategy
    StrategyCollection* strat = new StrategyCollection();
    // add the first actions from allCoalitionTransitions to strat
    for (auto item : this->probabilityStrategy.currentStrategyPermutation) {
        auto action = this->probabilityStrategy.allCoalitionTransitions[agent][item.first][0].getStrategy().begin()->second;
        strat->addAction(action);
    }
    this->strategyCollection = strat;
}

bool GlobalModelGenerator::checkLocalStates(vector<LocalState*>* localStates, GlobalState* globalState) {
    map<string, int> currEnv;

    for (const auto localState : *localStates) {
        for(auto it = localState->environment.begin(); it!=localState->environment.end(); ++it){
            currEnv[it->first] = it->second;
        }
    }
    auto val = *formula->p;

    if (val[0]->eval(currEnv, this, globalState)==1) {
        return true;
    }
    return false;
}

/// @brief Prepares the strategy generating structures for the probabilistic verification.
/// @param localModels LocalModels to generate the coalition transition buckets from.
set<set<tuple<string, string>>> GlobalModelGenerator::createProbabilityStrategy(LocalModels* localModels)
{
    set<Agent*> coalition = formula->coalition;
    set<Agent*> opponents;
    for (Agent* agent : localModels->agents) {
        if (coalition.find(agent) == coalition.end()) {
            opponents.emplace(agent);
        }
    }
    // for (auto transitions : coalitionTransitions) {
    //     cout << "Controlled: " << transitions.first << ":" << endl;
    //     for (auto transitionName : transitions.second) {
    //         cout << "\t" << transitionName.first << endl;
    //         for (auto transition : transitionName.second) {
    //             cout << "\t\t" << transition->from->hash << " -> " << transition->to->hash << " P=(" << transition->getProbability() << ") " << checkLocalStates(&transition->to->localStatesProjection, transition->to) << endl;
    //         }
    //     }
    // }
    // for (auto transitions : opponentsTransitions) {
    //     cout << "Uncontrolled: " << transitions.first << ":" << endl;
    //     for (auto transitionName : transitions.second) {
    //         cout << "\t" << transitionName.first << endl;
    //         for (auto transition : transitionName.second) {
    //             cout << "\t\t" << transition->from->hash << " -> " << transition->to->hash << " P=(" << transition->getProbability() << ") " << checkLocalStates(&transition->to->localStatesProjection, transition->to) << endl;
    //         }
    //     }
    // }
    this->coalitionStrategy = getAllPossiblePaths(coalitionTransitions, opponentsTransitions, this->globalModel->initState->hash);
    
    return this->coalitionStrategy;
}

/// @brief Generates all possible paths through the transition graph.
/// @param coalitionTransitions Map of coalition-controlled transitions grouped by state hash and action name. (state, actionName, actual transitions)
/// @param opponentsTransitions Map of opponent-controlled transitions grouped by state hash and action name. (state, actionName, actual transitions)
/// @param initialHash Starting state hash for path exploration.
/// @return Set of all unique paths, where each path is a set of tuples (state_hash, action_name).
set<set<tuple<string, string>>> GlobalModelGenerator::getAllPossiblePaths(map<string, map<string, set<GlobalTransition*>>>& coalitionTransitions, map<string, map<string, set<GlobalTransition*>>>& opponentsTransitions, string initialHash) {
    //set of tuples
    set<string> visitedStatesStack; // track current recursion stack for cycle detection

    // DFS returns the set of paths (each path is a set of (state_hash, action_name) tuples) reachable from stateHash
    function<set<set<tuple<string, string>>>(const string&, int)> dfs = [&](const string& stateHash, int depth) -> set<set<tuple<string, string>>> {
        if (visitedStatesStack.count(stateHash)) {
            // cycle encountered -> treat as terminal for this branch (return one empty path so callers can add their transition if needed)
            return set<set<tuple<string, string>>>{ set<tuple<string, string>>() };
        }

        visitedStatesStack.insert(stateHash);
        bool hasOutgoingTransitions = false;
        set<set<tuple<string, string>>> resultPaths;

        // Coalition-controlled transitions: for each action name, union results of all transitions for that action,
        // then add the (state_hash, action_name) tuple to each path returned for that action.
        auto coalitionIterator = coalitionTransitions.find(stateHash);
        if (coalitionIterator != coalitionTransitions.end()) {
            for (auto& actionPair : coalitionIterator->second) {
                const string& actionName = actionPair.first;
                // cout << string(depth*4, ' ') << actionName << endl;
                vector<set<set<tuple<string, string>>>> actionUnionResults; // union of results for all transitions with this action
                for (auto* transition : actionPair.second) {
                    // Always consider every probabilistic branch for the same action.
                    // Previously we skipped branches that did not change coalition local states,
                    // which lost outcomes like 0;0; -> 1;2; for action A.
                    hasOutgoingTransitions = true;
                    auto subPaths = dfs(transition->to->hash, depth+1);
                    actionUnionResults.push_back(set<set<tuple<string, string>>>());
                    for (auto subPath : subPaths) {
                        subPath.insert(make_tuple(transition->from->hash, actionName));
                        actionUnionResults[actionUnionResults.size() - 1].insert(subPath);
                    }
                }
                // add the (state_hash, action_name) tuple to each path in the union and merge into resultPaths
                // compute cartesian product of sets in actionUnionResults:
                // start with one empty accumulation set
                vector<set<tuple<string, string>>> cartesianProduct;
                cartesianProduct.emplace_back();

                for (const auto &choiceSet : actionUnionResults) {
                    vector<set<tuple<string, string>>> nextProduct;
                    for (const auto &accumulator : cartesianProduct) {
                        for (const auto &selectedPath : choiceSet) {
                            set<tuple<string, string>> mergedPaths = accumulator;
                            mergedPaths.insert(selectedPath.begin(), selectedPath.end());
                            nextProduct.push_back(std::move(mergedPaths));
                        }
                    }
                    cartesianProduct.swap(nextProduct);
                }

                // insert all combined results into resultPaths
                for (const auto &combinedPath : cartesianProduct) {
                    resultPaths.insert(combinedPath);
                }
            }
        }

        // Opponent-controlled transitions: explore outcomes and union results (do not add action names)
        auto opponentIterator = opponentsTransitions.find(stateHash);
        if (opponentIterator != opponentsTransitions.end()) {
            for (auto& actionPair : opponentIterator->second) {
                for (auto* transition : actionPair.second) {
                    hasOutgoingTransitions = true;
                    auto subPaths = dfs(transition->to->hash, depth+1);
                    for (auto& subPath : subPaths) {
                        if (subPath.size() > 0) {
                            resultPaths.insert(subPath);
                            // cout << string(depth*4, ' ') << "been here" << endl;
                        }
                    }
                }
            }
        }

        // If no outgoing transitions, this is a terminal state: return a single empty path (callers may add actions)
        if (!hasOutgoingTransitions) {
            resultPaths.insert(set<tuple<string, string>>());
        }

        visitedStatesStack.erase(stateHash);
        return resultPaths;
    };

    // Get all paths from initial state
    set<set<tuple<string, string>>> allPaths = dfs(initialHash, 0);

    // Print resulting paths
    // for (const auto& path : allPaths) {
    //     for (const auto& tuple : path) cout << "(" << get<0>(tuple) << "," << get<1>(tuple) << ") ";
    //     cout << endl;
    // }
    currentStratID = 0;
    return allPaths;
}

MDP GlobalModelGenerator::generateNextMDP(bool makeOpponentGoMax) {
    MDP newMdp(0);
    int actionId = 0;

    if (currentStratID == -1) {
        return newMdp;
    }

    // map of state hash to action name that appear in coalitionStrategy (allowed coalition actions)
    map<string, string> allowedCoalitionActions;
    auto strategyIt = coalitionStrategy.begin(); // first strategy
    advance(strategyIt, currentStratID);
    for (const auto &tuple : *strategyIt) {
        cout << get<0>(tuple) << "-" << get<1>(tuple) << " ";
        allowedCoalitionActions[get<0>(tuple)] = get<1>(tuple); // map state hash to action name
    }
    cout << endl;

    // for (const auto &entry : allowedCoalitionActions) {
    //     cout << "Allowed: " << entry.first << " -> " << entry.second << endl;
    // }

    // Start with only the initial state in the id map and expand states on the fly
    map<string, int> stateIdMap;
    string initHash = this->globalModel->initState->hash;
    stateIdMap[initHash] = 0;
    int nextNewStateId = 1;

    // BFS/queue of raw state hashes to process (may include 'T' suffixed terminal duplicates)
    queue<string> q;
    q.push(initHash);
    unordered_set<string> processed; // avoid reprocessing same raw hash

    while (!q.empty()) {
        string rawStateHash = q.front();
        q.pop();
        // skip already processed states
        if (processed.find(rawStateHash) != processed.end()) {
            continue;
        }
        processed.insert(rawStateHash);

        // check if state is in a chain where the answer was correct somewhere down the line and strip the additional marking if it is the case
        bool stateWasT = false;
        string stateHash = rawStateHash;
        if (!stateHash.empty() && stateHash.back() == 'T') {
            stateWasT = true;
            // cout << stateHash << " was T!" << endl;
            stateHash.pop_back();
        }
        string baseHash = stateHash;

        auto coalitionIt = coalitionTransitions.find(baseHash);
        auto opponentsIt  = opponentsTransitions.find(baseHash);

        // gather action names available in this state (based on baseHash)
        set<string> actionNames;
        if (coalitionIt != coalitionTransitions.end()) {
            for (const auto &p : coalitionIt->second) actionNames.insert(p.first);
        }
        if (opponentsIt != opponentsTransitions.end()) {
            for (const auto &p : opponentsIt->second) actionNames.insert(p.first);
        }

        // ensure fromStateId exists (we populate stateIdMap as we enqueue)
        auto fromIt = stateIdMap.find(rawStateHash);
        if (fromIt == stateIdMap.end()) {
            // Shouldn't happen because we push only mapped states, but guard anyway
            stateIdMap[rawStateHash] = nextNewStateId++;
            fromIt = stateIdMap.find(rawStateHash);
        }
        int fromStateId = fromIt->second;

        for (const auto &actionName : actionNames) {
            // if this is a coalition action, require it to be present in some strategy path with proper state mapping
            bool isCoalitionAction = (coalitionIt != coalitionTransitions.end() && coalitionIt->second.count(actionName));
            if (isCoalitionAction) {
                auto allowedIt = allowedCoalitionActions.find(baseHash);
                if (allowedIt == allowedCoalitionActions.end() || allowedIt->second != actionName) {
                    continue;
                }
            }

            // collect all transitions (coalition + opponents) for this action at this state (baseHash used)
            vector<GlobalTransition*> transitionList;
            if (coalitionIt != coalitionTransitions.end()) {
                auto coalitionActionIt = coalitionIt->second.find(actionName);
                if (coalitionActionIt != coalitionIt->second.end()) {
                    for (auto *t : coalitionActionIt->second) {
                        if (t && t->to) {
                            transitionList.push_back(t);
                        }
                    }
                }
            }
            if (opponentsIt != opponentsTransitions.end()) {
                auto opponentActionIt = opponentsIt->second.find(actionName);
                if (opponentActionIt != opponentsIt->second.end()) {
                    for (auto *t : opponentActionIt->second) {
                        if (t && t->to) {
                            transitionList.push_back(t);
                        }
                    }
                }
            }
            if (transitionList.empty()) continue;

            // add all transitions under a single action id
            bool transitionsAdded = false;
            for (auto *transition : transitionList) {
                auto toHash = transition->to->hash;
                bool checkToStateResult = checkLocalStates(&transition->to->localStatesProjection, transition->to);
                if (formula->isF == false) {
                    checkToStateResult = !checkToStateResult;
                }
                // If both the source raw state and the target would be the same 'T'-suffixed hash, skip this transition.
                // This avoids adding trivial T->T self-transitions.
                if (stateWasT) {
                    if (toHash + string("T") == rawStateHash && checkToStateResult == true) {
                        continue;
                    }
                }

                // ensure target state has an id and is scheduled for processing (if unseen)
                if (stateIdMap.find(toHash) == stateIdMap.end() && checkToStateResult == false) {
                    stateIdMap[toHash] = nextNewStateId++;
                    // cout << "Pushed " << toHash << endl;
                    q.push(toHash);
                }
                int toStateId = stateIdMap[toHash];
                double prob = transition->getProbability();

                // skip self-loops with probability 0 or 1 that won't change the state to TRUE
                if (baseHash == transition->to->hash && (prob == 0.0 || prob == 1.0) && checkToStateResult == false) {
                    continue;
                }

                bool isTerminalByCheck = false;
                // If the currently processed state had a trailing 'T', assume terminal (no need to call checkLocalStates)
                if (stateWasT) {
                    isTerminalByCheck = true;
                } else {
                    if (checkToStateResult == true) {
                        isTerminalByCheck = true;
                    }
                }

                double reward = 0.0;
                if (isTerminalByCheck) {
                    // duplicate target state with 'T' appended and map it if not present
                    string toHashT = toHash + "T";
                    if (stateIdMap.find(toHashT) == stateIdMap.end()) {
                        stateIdMap[toHashT] = nextNewStateId++;
                        // cout << "T-Pushed " << toHashT << endl;
                        q.push(toHashT);
                    }
                    toStateId = stateIdMap[toHashT];
                    // if the current processed state had 'T', reward must be 0.0
                    reward = (!stateWasT) ? ((makeOpponentGoMax || opponentsTransitions.size() == 0) ? 1.0 : -1.0) : 0.0;
                }

                add_transition(newMdp, fromStateId, actionId, toStateId, prob, reward);
                // cout << "(" << actionId << ") " << rawStateHash << " -> " << (isTerminalByCheck ? toHash + "T" : toHash) << " p=" << reward << " " << isTerminalByCheck << endl;
                transitionsAdded = true;
            }
            if (transitionsAdded) {
                ++actionId;
            }
        }
    }

    if (currentStratID != -1) {
        currentStratID++;
    }
    if (currentStratID >= coalitionStrategy.size()) {
        currentStratID = -1;
    }

    // auto&& re = algorithms::solve_mpi(newMdp, 1, numvec(0), indvec(0), 100, SOLPREC, 100, SOLPREC/2, false);
    // cout << "p=" << -re.valuefunction[0] << endl;

    return newMdp;
}

/// @brief Generates next set strategy from the memorized coalition transitions for the probabilistic verification.
/// @return Returns true if the strategy was generated properly, false if it is the last permutation.
bool GlobalModelGenerator::nextIterativeStrategy()
{
    cout << this->probabilityStrategy.currentStrategyPermutation.rbegin()->first << " " << this->probabilityStrategy.currentStrategyPermutation.rbegin()->second << " / " << endl;
    // increase rightmost counter by 1, if overflow -> increase the previous counter by 1 recursively
    this->probabilityStrategy.currentStrategyPermutation.rbegin()->second += 1;
    auto it = this->probabilityStrategy.currentStrategyPermutation.rbegin();
    while (it != this->probabilityStrategy.currentStrategyPermutation.rend()) {
        cout << it->first << " " << it->second << " / " << endl;
        auto actionBucket = this->probabilityStrategy.allCoalitionTransitions[*this->formula->coalition.begin()][it->first];
        cout << static_cast<int>(actionBucket.size()) << endl;
        for (auto item : actionBucket) {
            auto itemStrat = item.getStrategy().begin();
            cout << itemStrat->first << " " << itemStrat->second.actionName << endl;
        }
        if (it->second >= static_cast<int>(actionBucket.size())) {
            it->second = 0;
            it++;
            if (it != this->probabilityStrategy.currentStrategyPermutation.rend()) {
                it->second += 1;
            } else {
                break;
            }
        } else {                
            break;
        }
    }
    if (it == this->probabilityStrategy.currentStrategyPermutation.rend() && this->probabilityStrategy.currentStrategyPermutation.begin()->second == 0) {
        cout << "THIS" << endl;
        return false;
    }
    
    // create a new strategy from the current permutation and construct a strategy from selected parts
    StrategyCollection* strat = new StrategyCollection();
    for (auto item : this->probabilityStrategy.currentStrategyPermutation) {
        auto actionIt = this->probabilityStrategy.allCoalitionTransitions[*this->formula->coalition.begin()][item.first].begin();
        advance(actionIt, item.second);
        auto action = actionIt->getStrategy().begin()->second;
        strat->addAction(action);
    }
    this->strategyCollection = strat;
    return true;
}

/// @brief Gives next action's name from a given state.
/// @param state GlobalState from which an action name would be retrieved if a set strategy is present.
/// @return String containing the action name ending on a semicolon.
string GlobalModelGenerator::getActionNameFromStateInStrategy(GlobalState* state) {
    string coalitionLocalStateName = getCoalitionIdentifier(&(state->localStatesProjection));
    try {
        auto match = this->strategyCollection->getStrategy()[coalitionLocalStateName];
        return match.actionName + ";";
    }
    catch (const std::out_of_range& e) {
        cout << "No action name found for coalition local state: " << coalitionLocalStateName << endl;
        return "";
    }
    return "";
}

/// @brief Returns a concatenated name of all given LocalStates.
/// @param localStates Vector of LocalStates to concatenate into a single string.
/// @return LocalState names separated with semicolons, ending on a semicolon.
string GlobalModelGenerator::getCoalitionIdentifier(vector<LocalState*>* localStates) {
    string hash = "";
    for (const auto localState : *localStates) {
        if (formula->coalition.find(localState->agent) != formula->coalition.end()) {
            hash += localState->name + ";";
        }
    }
    return hash;
}
