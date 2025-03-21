/**
 * @file Verification.cpp
 * @brief Class for verification of the formula on a model.
 * Class for verification of the specified formula on a specified model.
 */

#include "Verification.hpp"
#define DEPTH_PREFIX string(depth * 4, ' ')

#include <bits/stdc++.h>

extern Cfg config;

/// @brief Converts global verification status into a string.
/// @param status Enum value to be converted.
/// @return Verification status converted into a string.
string verStatusToStr(GlobalStateVerificationStatus status) {
    if (status == GlobalStateVerificationStatus::PENDING) {
        return "pending";
    }
    if (status == GlobalStateVerificationStatus::UNVERIFIED) {
        return "unverified";
    }
    if (status == GlobalStateVerificationStatus::VERIFIED_ERR) {
        return "ERR";
    }
    if (status == GlobalStateVerificationStatus::VERIFIED_OK) {
        return "OK";
    }
    return "unknown";
}

/// @brief Print a debug message of a verification status to the console.
/// @param prefix A prefix string to append to the front of every entry.
/// @param gs Pointer to a GlobalState.
/// @param st Enum with a verification status of a global state.
/// @param reason String with a reason why the function was called, e.g. "entered state", "all passed".
void dbgVerifStatus(string prefix, GlobalState* gs, GlobalStateVerificationStatus st, string reason) {
    #if VERBOSE
        string prevStatus = verStatusToStr(gs->verificationStatus);
        string newStatus = verStatusToStr(st);
        printf("%schange verifStatus of (%s): %s -> %s (%s)\n", prefix.c_str(), gs->hash.c_str(), prevStatus.c_str(), newStatus.c_str(), reason.c_str());
    #endif
}

/// @brief Print a single debug message with a history entry to the console.
/// @param prefix A prefix string to append to the front of the entry.
/// @param h A pointer to the HistoryEntry struct which will be printed out.
void dbgHistEnt(string prefix, HistoryEntry *h) {
    #if VERBOSE
        printf("%s%s\n", prefix.c_str(), h->toString().c_str());
    #endif
}

/// @brief A constructor for HistoryDbg.
HistoryDbg::HistoryDbg() {
}

/// @brief A destructor for HistoryDbg.
HistoryDbg::~HistoryDbg() {
    for (auto &kvp : this->entries) {
        if (kvp.second == '-') {
            delete kvp.first;
        }
    }
}

/// @brief Adds a HistoryEntry to the debug history.
/// @param entry A pointer to the HistoryEntry that will be added to the history.
void HistoryDbg::addEntry(HistoryEntry* entry) {
    this->entries.push_back(make_pair(entry, ' '));
}

/// @brief Marks an entry in the degug history with a char.
/// @param entry A pointer to a HistoryEntry that is supposed to be marked.
/// @param chr A char that will be made into a pair with a HistoryEntry.
void HistoryDbg::markEntry(HistoryEntry* entry, char chr) {
    // '-' - action that will be undone
    // 'Y' (type=context) - will be undone; corresponding to Y in selene-ver2.png
    // 'T' (type=decision) - will be undone; corresponding to T in selene-ver2.png
    // 'R' (type=context) - will be undone and added to historyToRestore
    // ' ' - won't be affected
    for (auto &kvp : this->entries) {
        if (kvp.first == entry) {
            kvp.second = chr;
            if (chr == '-') {
                kvp.first = new HistoryEntry(*entry);
            }
        }
    }
}

/// @brief Prints every entry from the algorithm's path.
/// @param prefix A prefix string to append to the front of every entry.
void HistoryDbg::print(string prefix) {
    printf("%svvvvvvvvvv History:\n", prefix.c_str());
    for (auto pair : this->entries) {
        printf("%s    [%c] %s\n", prefix.c_str(), pair.second, pair.first->toString().c_str());
    }
    printf("%s^^^^^^^^^^\n", prefix.c_str());
}

/// @brief Checks if the HistoryEntry pointer exists in the debug history.
/// @param entry A pointer to a HistoryEntry to be checked.
/// @return Identity function if the entry is in history, otherwise returns nullptr.
HistoryEntry* HistoryDbg::cloneEntry(HistoryEntry* entry) {
    for (auto &kvp : this->entries) {
        if (kvp.first == entry) {
            kvp.first = new HistoryEntry(*entry);
            return kvp.first;
        }
    }
    return nullptr;
}




/// @brief Constructor for Verification.
/// @param generator Pointer to GlobalModelGenerator
Verification::Verification(GlobalModelGenerator* generator) {
    this->generator = generator;
    
    this->historyStart = new HistoryEntry();
    this->historyStart->globalState = nullptr;
    this->historyStart->decision = nullptr;
    this->historyStart->prev = nullptr;
    this->historyStart->next = nullptr;
    
    this->historyEnd = this->historyStart;
}

/// @brief Destructor for Verification.
Verification::~Verification() {
}

/// @brief Starts the process of formula verification on a model.
/// @return Returns true if the verification is PENDING or VERIFIED_OK, otherwise returns false.
bool Verification::verify() {
    this->mode = TraversalMode::NORMAL;
    this->revertToGlobalState = nullptr;
    const auto initState = this->generator->getCurrentGlobalModel()->initState;
    return this->verifyGlobalState(initState, 0);
}

bool Verification::fixpointVerify() {
    this->generator->expandAllStates();

    if(this->generator->getFormula()->isF) {
        return this->minFixpointVerify();
    } else {
        return this->maxFixpointVerify();
    }
}

bool Verification::minFixpointVerify() {
    Agent* agent = *(this->generator->getFormula()->coalition.begin());
    set<GlobalState*> winningStates, previousStep;
    for(auto state : this->generator->getCurrentGlobalModel()->globalStates) {
        if(verifyLocalStates(&state->localStatesProjection, state)) {
            winningStates.insert(state);
        }
    }

    previousStep.insert(winningStates.begin(), winningStates.end());

    while(true) {
        set<GlobalState*> preimage, currentStep;
        for(auto state : previousStep) {
            preimage.insert(state->preimage.begin(), state->preimage.end());
        }

        for(auto state : preimage) {
            bool isOk = false;
            map<string, set<GlobalState*>> transitionResult;
            for(auto epistemicClassEl : state->epistemicClasses[agent]->globalStates) {
                auto globalState = epistemicClassEl.second;
                for(auto transition : globalState->globalTransitions) {
                    bool hasAction = false;
                    for(auto localTransition : transition->localTransitions) {
                        if(localTransition->from->agent == agent) {
                            transitionResult[localTransition->name].insert(transition->to);
                            hasAction = true;
                            break;
                        }
                    }

                    if(!hasAction) {
                        transitionResult[""].insert(transition->to);
                    }
                }
            }

            for(auto el : transitionResult) {
                bool transitionOk = true;
                for(auto nextState : el.second) {
                    if(winningStates.find(nextState) == winningStates.end()) {
                        transitionOk = false;
                        break;
                    }
                }

                if (transitionOk) {
                    isOk = true;
                    break;
                }
            }

            if(isOk) {
                currentStep.insert(state);
            }
        }

        previousStep = currentStep;
        set<GlobalState*> newWinningStates;
        newWinningStates.insert(winningStates.begin(), winningStates.end());
        newWinningStates.insert(currentStep.begin(), currentStep.end());
        if(newWinningStates.size() == winningStates.size()) {
            break;
        }

        winningStates = newWinningStates;
    }

    if(winningStates.find(this->generator->getCurrentGlobalModel()->initState) != winningStates.end()) {
        return true;
    }

    return false;
}

bool Verification::maxFixpointVerify() {
    Agent* agent = *(this->generator->getFormula()->coalition.begin());
    set<GlobalState*> winningStates, previousStep;
    for(auto state : this->generator->getCurrentGlobalModel()->globalStates) {
        if(verifyLocalStates(&state->localStatesProjection, state)) {
            winningStates.insert(state);
        }
    }

    previousStep.insert(winningStates.begin(), winningStates.end());

    while(true) {
        set<GlobalState*> toKeep;

        for(auto state : winningStates) {
            bool isOk = false;
            map<string, set<GlobalState*>> transitionResult;
            for(auto epistemicClassEl : state->epistemicClasses[agent]->globalStates) {
                auto globalState = epistemicClassEl.second;
                for(auto transition : globalState->globalTransitions) {
                    bool hasAction = false;
                    for(auto localTransition : transition->localTransitions) {
                        if(localTransition->from->agent == agent) {
                            transitionResult[localTransition->name].insert(transition->to);
                            hasAction = true;
                            break;
                        }
                    }

                    if(!hasAction) {
                        transitionResult[""].insert(transition->to);
                    }
                }
            }

            for(auto el : transitionResult) {
                bool transitionOk = true;
                for(auto nextState : el.second) {
                    if(winningStates.find(nextState) == winningStates.end()) {
                        transitionOk = false;
                        break;
                    }
                }

                if (transitionOk) {
                    isOk = true;
                    break;
                }
            }

            if(isOk) {
                toKeep.insert(state);
            }
        }

        if(toKeep.size() == winningStates.size()) {
            break;
        }

        winningStates = toKeep;
    }

    if(winningStates.find(this->generator->getCurrentGlobalModel()->initState) != winningStates.end()) {
        return true;
    }

    return false;
}

/// @brief Verifies a set of LocalState that a GlobalState is composed of with a hardcoded formula.
/// @param localStates A pointer to a set of pointers to LocalState.
/// @return Returns true if there is a LocalState with a specific set of values, fulfilling the criteria, otherwise returns false.
bool Verification::verifyLocalStates(vector<LocalState*>* localStates, GlobalState* globalState) {
    map<string, int> currEnv; // [YK]: temporary solution assuming that Agents environments are disjoint

    for (const auto localState : *localStates) {
        for(auto it = localState->environment.begin(); it!=localState->environment.end(); ++it){
            currEnv[it->first] = it->second;
        }
    }
    auto val = *this->generator->getFormula()->p;
    if (val[0]->eval(currEnv, generator, globalState)==1 && generator->getFormulaCorectness()) {
        return true;
    } else if (!generator->getFormulaCorectness()) {
        throw std::runtime_error("Incorrect variable name");
    }
    return false;
}

/// @brief Recursively verifies GlobalState 
/// @param globalState Pointer to a GlobalState of the model.
/// @param depth Current depth of the recursion.
/// @return Returns true if the verification is PENDING or VERIFIED_OK, otherwise returns false.
bool Verification::verifyGlobalState(GlobalState* globalState, int depth) {
    #if VERBOSE
        if (globalState->verificationStatus == GlobalStateVerificationStatus::UNVERIFIED) {
            printf("%s> verify globalState: hash=%s (unverified)\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str());
        }
        else if (globalState->verificationStatus == GlobalStateVerificationStatus::PENDING) {
            printf("%s> skipping verification of globalState: hash=%s (verification pending)\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str());
        }
        else if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_OK) {
            printf("%s> skipping verification of globalState: hash=%s (verified OK)\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str());
        }
        else if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_ERR) {
            printf("%s> skipping verification of globalState: hash=%s (verified ERR)\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str());
        }
    #endif

    bool isFMode = this->generator->getFormula()->isF;
    bool isCTLMode = this->generator->getFormula()->isCTL;

    if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_ERR) {
        return false;
    }
    else if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_OK) {
        return true;
    }
    else if (globalState->verificationStatus == GlobalStateVerificationStatus::PENDING) {
        if (isFMode) {
            return false;
        }
        else {
            return true;
        }
    }
    if (this->historyEnd != nullptr && this->historyEnd->type == HistoryEntryType::STATE_STATUS && this->historyEnd->globalState == globalState && this->historyEnd->prevStatus == GlobalStateVerificationStatus::PENDING && this->historyEnd->newStatus == GlobalStateVerificationStatus::UNVERIFIED) {
        dbgVerifStatus(DEPTH_PREFIX, globalState, GlobalStateVerificationStatus::PENDING, "entered state w/ undo");
        this->undoLastHistoryEntry(true);
    }
    else {
        this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::PENDING);
        dbgVerifStatus(DEPTH_PREFIX, globalState, GlobalStateVerificationStatus::PENDING, "entered state");
        globalState->verificationStatus = GlobalStateVerificationStatus::PENDING;
    }
    
    // 1) verify localStates that the globalState is composed of
    if (isFMode) { // F
        if (this->verifyLocalStates(&globalState->localStatesProjection, globalState)) {
            this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_OK);
            dbgVerifStatus(DEPTH_PREFIX, globalState, GlobalStateVerificationStatus::VERIFIED_OK, "all passed");
            globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;
            return true;
        }
    }
    else { // G
        if (!this->verifyLocalStates(&globalState->localStatesProjection, globalState)) {
            this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_ERR);
            dbgVerifStatus(DEPTH_PREFIX, globalState, GlobalStateVerificationStatus::VERIFIED_ERR, "localStates verification");
            globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
            return false;
        }
    }
    
    // 2) ensure that the state is expanded
    if (!globalState->isExpanded) {
        this->generator->expandState(globalState);
    }
    
    // 3) group transitions: controlled and uncontrolled by the coalition
    set<GlobalTransition*> controlledGlobalTransitions;
    set<GlobalTransition*> uncontrolledGlobalTransitions;
    auto epistemicClass = this->getEpistemicClassForGlobalState(globalState);
    auto fixedGlobalTransition = epistemicClass != nullptr ? epistemicClass->fixedCoalitionTransition : nullptr;
    bool hasOmittedTransitions = false;

    for (const auto globalTransition : globalState->globalTransitions) {
        // if CTL then treat everything as uncontrolled transitions
        if (isCTLMode) {
            uncontrolledGlobalTransitions.insert(globalTransition);
            continue;
        }

        if (this->isGlobalTransitionControlledByCoalition(globalTransition)) {
            if (fixedGlobalTransition == nullptr) {
                controlledGlobalTransitions.insert(globalTransition);
            }
            else if (this->areGlobalStatesInTheSameEpistemicClass(fixedGlobalTransition->to, globalTransition->to) && this->equivalentGlobalTransitions(fixedGlobalTransition, globalTransition)) {
                // controlled transition that is fixed should be treated as an uncontrolled transition 
                #if VERBOSE
                    printf("%streat controlled as uncontrolled: %s -> %s\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str(), globalTransition->to->hash.c_str());
                #endif
                uncontrolledGlobalTransitions.insert(globalTransition);
            }
            else {
                // omit controlled transition that is != fixedGlobalTransition
                #if VERBOSE
                    printf("%somit controlled: %s -> %s\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str(), globalTransition->to->hash.c_str());
                #endif
                hasOmittedTransitions = true;
            }
        }
        else {
            uncontrolledGlobalTransitions.insert(globalTransition);
        }
    }
    // solve the uncontrolled transition blocking the controlled transition, making it uncontrolled
    if (controlledGlobalTransitions.size() > 0 && uncontrolledGlobalTransitions.size() > 0) {
        set<Agent*> agents = generator->getFormula()->coalition;
        set<Agent*> brokenAgents;
        set<Agent*> potentiallyBrokenAgents;
        // find if there are uncontrolled actions with only agents that are not in coalition, if yes, then they can block the controlled actions if they are a participants
        for (const auto globalTransition : uncontrolledGlobalTransitions) {
            bool agentTest = false;
            for (auto glob : globalTransition->localTransitions) {
                for (auto agt : agents) {
                    if (glob->agent->name.c_str() == agt->name.c_str()) {
                        agentTest = true;
                        break;
                    }
                }
                if(agentTest) {
                    potentiallyBrokenAgents.clear();
                    break;
                }
                else {
                    potentiallyBrokenAgents.insert(glob->agent);
                }
            }
            // if the global transition can fire without any of the coalition agents, mark the agents participating as a definitely faulty ones
            if (!agentTest) {
                brokenAgents.insert(potentiallyBrokenAgents.begin(), potentiallyBrokenAgents.end());
                potentiallyBrokenAgents.clear();
            }
        }
        // if it turns out that there are bad blocking agents, turn every controlled action with those agents into an uncontrolled one
        if (brokenAgents.size() > 0) {
            stack<GlobalTransition*> transitionsToBeMoved;
            // mark bad controlled global transition as uncontrolled
            for (const auto globalTransition : controlledGlobalTransitions) {
                bool agentTest = true;
                for (auto localTrans : globalTransition->localTransitions) {
                    bool brokenAgentFound = false;
                    for (auto agt : brokenAgents) {
                        if(localTrans->agent->name.c_str() == agt->name.c_str()) {
                            brokenAgentFound = true;
                            break;
                        }
                    }
                    if(brokenAgentFound) {
                        agentTest = false;
                    }
                }
                if (!agentTest) {
                    transitionsToBeMoved.push(globalTransition);
                }
            }
            while (!transitionsToBeMoved.empty()) {
                GlobalTransition* tr = transitionsToBeMoved.top();
                transitionsToBeMoved.pop();
                uncontrolledGlobalTransitions.insert(tr);
                controlledGlobalTransitions.erase(tr);
            }
        }
    }

    if (!verifyTransitionSets(controlledGlobalTransitions, uncontrolledGlobalTransitions, globalState, depth, hasOmittedTransitions, isFMode)) {
        return false;
    }
    
    // 5) all passed
    this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_OK);
    dbgVerifStatus(DEPTH_PREFIX, globalState, GlobalStateVerificationStatus::VERIFIED_OK, "all passed");
    globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;
    return true;
}

/// @brief Checks if any of the LocalTransition in a given GlobalTransition has an Agent in a coalition in the formula.
/// @param globalTransition Pointer to a GlobalTransition in a model.
/// @return Returns true if the Agent is in coalition in the formula, otherwise returns false.
bool Verification::isGlobalTransitionControlledByCoalition(GlobalTransition* globalTransition) {
    bool isControlled = false;
    for (const auto localTransition : globalTransition->localTransitions) {
        if (this->isAgentInCoalition(localTransition->agent)) {
            if (!localTransition->isShared || localTransition->name == localTransition->localName) {
                isControlled = true;
                break;
            }
        }
    }
    return isControlled;
}

/// @brief Checks if the Agent is in a coalition based on the formula in a GlobalModelGenerator.
/// @param agent Pointer to an Agent that is to be checked.
/// @return Returns true if the Agent is in a coalition, otherwise returns false.
bool Verification::isAgentInCoalition(Agent* agent) {
    const auto coalition = &this->generator->getFormula()->coalition;
    return coalition->find(agent) != coalition->end();
}

/// @brief Gets the EpistemicClass for the agent in passed GlobalState, i.e. transitions from indistinguishable state from certain other states for an agent to other states.
/// @param globalState Pointer to a GlobalState of the model.
/// @return Pointer to the EpistemicClass that a coalition of agents from the formula belong to. If there is no such EpistemicClass, returns false.
EpistemicClass* Verification::getEpistemicClassForGlobalState(GlobalState* globalState) {
    const auto coalition = &this->generator->getFormula()->coalition;
    const auto agent = *coalition->begin();
    const auto findResult = globalState->epistemicClasses.find(agent);
    if (findResult == globalState->epistemicClasses.end()) {
        return nullptr;
    }
    const auto epistemicClass = (*findResult).second;
    return epistemicClass;
}

/// @brief Compares two GlobalState and checks if their EpistemicClass is the same.
/// @param globalState1 Pointer to the first GlobalState.
/// @param globalState2 Pointer to the second GlobalState.
/// @return Returns true if the EpistemicClass is the same for both of the GlobalState. Returns false if they are different or at least one of them has no EpistemicClass.
bool Verification::areGlobalStatesInTheSameEpistemicClass(GlobalState* globalState1, GlobalState* globalState2) {
    auto epiCls1 = this->getEpistemicClassForGlobalState(globalState1);
    auto epiCls2 = this->getEpistemicClassForGlobalState(globalState2);
    if (epiCls1 == nullptr || epiCls2 == nullptr) {
        return false;
    }
    return epiCls1 == epiCls2;
}

/// @brief Creates a HistoryEntry of the type DECISION and puts it on top of the stack of the decision history. 
/// @param globalState Pointer to a GlobalState of the model.
/// @param decision Pointer to a GlobalTransition that is to be recorded in the decision history.
void Verification::addHistoryDecision(GlobalState* globalState, GlobalTransition* decision) {
    auto newHistoryEntry = new HistoryEntry();
    newHistoryEntry->type = HistoryEntryType::DECISION;
    newHistoryEntry->globalState = globalState;
    newHistoryEntry->decision = decision;
    newHistoryEntry->prev = this->historyEnd;
    newHistoryEntry->next = nullptr;
    this->historyEnd->next = newHistoryEntry;
    this->historyEnd = newHistoryEntry;
}

/// @brief Creates a HistoryEntry of the type STATE_STATUS and puts it to the top of the decision history.
/// @param globalState Pointer to a GlobalState of the model.
/// @param prevStatus Previous GlobalStateVerificationStatus to be logged.
/// @param newStatus New GlobalStateVerificationStatus to be logged.
void Verification::addHistoryStateStatus(GlobalState* globalState, GlobalStateVerificationStatus prevStatus, GlobalStateVerificationStatus newStatus) {
    auto newHistoryEntry = new HistoryEntry();
    newHistoryEntry->type = HistoryEntryType::STATE_STATUS;
    newHistoryEntry->globalState = globalState;
    newHistoryEntry->prevStatus = prevStatus;
    newHistoryEntry->newStatus = newStatus;
    newHistoryEntry->prev = this->historyEnd;
    newHistoryEntry->next = nullptr;
    this->historyEnd->next = newHistoryEntry;
    this->historyEnd = newHistoryEntry;
}

/// @brief Creates a HistoryEntry of the type CONTEXT and puts it to the top of the decision history.
/// @param globalState Pointer to a GlobalState of the model.
/// @param depth Depth of the recursion of the validation algorithm.
/// @param decision Pointer to a transition GlobalTransition selected by the algorithm.
/// @param globalTransitionControlled True if the GlobalTransition is in the set of global transitions controlled by a coalition and it is not a fixed global transition.
void Verification::addHistoryContext(GlobalState* globalState, int depth, GlobalTransition* decision, bool globalTransitionControlled) {
    auto newHistoryEntry = new HistoryEntry();
    newHistoryEntry->type = HistoryEntryType::CONTEXT;
    newHistoryEntry->globalState = globalState;
    newHistoryEntry->depth = depth;
    newHistoryEntry->decision = decision;
    newHistoryEntry->globalTransitionControlled = globalTransitionControlled;
    newHistoryEntry->prev = this->historyEnd;
    newHistoryEntry->next = nullptr;
    this->historyEnd->next = newHistoryEntry;
    this->historyEnd = newHistoryEntry;
}

/// @brief Creates a HistoryEntry of the type MARK_DECISION_AS_INVALID and returns it.
/// @param globalState Pointer to a GlobalState of the model.
/// @param decision Pointer to a transition GlobalTransition selected by the algorithm.
/// @return Returns pointer to a new HistoryEntry.
HistoryEntry* Verification::newHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision) {
    auto newHistoryEntry = new HistoryEntry();
    newHistoryEntry->type = HistoryEntryType::MARK_DECISION_AS_INVALID;
    newHistoryEntry->globalState = globalState;
    newHistoryEntry->decision = decision;
    newHistoryEntry->next = nullptr;
    return newHistoryEntry;
}

/// @brief Creates a HistoryEntry of the type MARK_DECISION_AS_INVALID and puts it to the top of the decision history.
/// @param globalState Pointer to a GlobalState of the model.
/// @param decision Pointer to a transition GlobalTransition selected by the algorithm.
void Verification::addHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision) {
    auto newHistoryEntry = this->newHistoryMarkDecisionAsInvalid(globalState, decision);
    newHistoryEntry->prev = this->historyEnd;
    this->historyEnd->next = newHistoryEntry;
    this->historyEnd = newHistoryEntry;
}

/// @brief Reverts GlobalState and history to the previous decision state.
/// @param depth Integer that will be multiplied by 4 and appended as a prefix to the optional debug log.
/// @return Returns true if rollback is successful, otherwise returns false.
bool Verification::revertLastDecision(int depth) {
    // History: (based on selene-ver2.png)
    // X -> Y -> T<decision> -> Y -> Z<ERR>
    // shallowestGlobalState = Y.globalState
    // invalidDecisionGlobalState = T.globalState
    // auto prefix = string(depth * 4, ' ');
    #if VERBOSE
        printf("%srevertLastDecision():\n", DEPTH_PREFIX.c_str());
        auto histDbg = HistoryDbg();
        auto x = this->historyStart->next;
        while (x != nullptr) {
            histDbg.addEntry(x);
            x = x->next;
        }
    #endif
    
    // Undo Z->T (including Z, excluding T); find Y while doing that
    int shallowestDepth = 99999999;
    GlobalState* shallowestGlobalState = nullptr;
    while (this->historyEnd != this->historyStart && this->historyEnd->type != HistoryEntryType::DECISION) {
        auto entry = this->historyEnd;
        if (entry->type == HistoryEntryType::CONTEXT && entry->depth < shallowestDepth) {
            shallowestDepth = entry->depth;
            shallowestGlobalState = entry->globalState;
        }
        #if VERBOSE
            histDbg.markEntry(entry, '-');
        #endif
        this->undoLastHistoryEntry(true);
    }
    
    #if VERBOSE
        vector<HistoryEntry*> dbgHEsToDelete;
        for (auto &kvp : histDbg.entries) {
            if (kvp.first->globalState == shallowestGlobalState && kvp.first->type == HistoryEntryType::CONTEXT && kvp.second == '-') {
                kvp.second = 'Y';
                dbgHEsToDelete.push_back(kvp.first);
            }
        }
    #endif
    
    // Ensure all ok
    if (this->historyEnd == this->historyStart || this->historyEnd->type != HistoryEntryType::DECISION || shallowestGlobalState == nullptr) {
        return false;
    }
    
    // Undo T->Y (including T, excluding Y); build path to repeat while doing that
    auto invalidDecisionGlobalState = this->historyEnd->globalState;
    auto invalidDecision = this->historyEnd->decision;
    auto invalidDecisionHistoryEntry = this->historyEnd;
    while (!this->historyToRestore.empty()) {
        delete this->historyToRestore.top();
        this->historyToRestore.pop();
    }
    if (!invalidDecision->isInvalidDecision) {
        // Mark the decision (transition) as invalid
        auto newHistoryEntry = this->newHistoryMarkDecisionAsInvalid(invalidDecisionGlobalState, invalidDecision);
        this->historyToRestore.push(newHistoryEntry);
    }
    while (this->historyEnd != this->historyStart && this->historyEnd->globalState != shallowestGlobalState) {
        auto entry = this->historyEnd;
        this->undoLastHistoryEntry(false);
        if (entry->type == HistoryEntryType::CONTEXT) {
            this->historyToRestore.push(entry);
            #if VERBOSE
                histDbg.markEntry(entry, 'R');
            #endif
        }
        else if (entry->type == HistoryEntryType::MARK_DECISION_AS_INVALID) {
            this->historyToRestore.push(entry);
            #if VERBOSE
                histDbg.markEntry(entry, 'R');
            #endif
        }
        else {
            #if VERBOSE
                if (entry == invalidDecisionHistoryEntry) {
                    histDbg.markEntry(entry, 'T');
                    auto cloned = histDbg.cloneEntry(entry);
                    if (cloned != nullptr) {
                        dbgHEsToDelete.push_back(cloned);
                    }
                }
                else {
                    histDbg.markEntry(entry, '-');
                }
            #endif
            delete entry;
        }
    }
    
    // Ensure all ok
    if (this->historyEnd == this->historyStart || this->historyEnd->globalState != shallowestGlobalState) {
        while (!this->historyToRestore.empty()) {
            delete this->historyToRestore.top();
            this->historyToRestore.pop();
        }
        return false;
    }
    
    #if VERBOSE
        histDbg.markEntry(this->historyEnd, 'Y');
    #endif
    
    // Undo Y
    this->historyToRestore.push(this->historyEnd);
    this->undoLastHistoryEntry(false);
    
    // Enter revert mode
    #if VERBOSE
        printf("%sset mode=REVERT\n", DEPTH_PREFIX.c_str());
    #endif
    this->mode = TraversalMode::REVERT;
    #if VERBOSE
        histDbg.print(DEPTH_PREFIX);
        for (auto he : dbgHEsToDelete) {
            delete he;
        }
    #endif
    this->revertToGlobalState = shallowestGlobalState;
    return true;
}

/// @brief Removes the top entry of the history stack.
/// @param freeMemory True if the entry has to be removed from memory.
void Verification::undoLastHistoryEntry(bool freeMemory) {
    if (this->historyEnd == this->historyStart) {
        return;
    }
    if (this->historyEnd->type == HistoryEntryType::DECISION) {
        auto epistemicClass = this->getEpistemicClassForGlobalState(this->historyEnd->globalState);
        epistemicClass->fixedCoalitionTransition = nullptr;
    }
    else if (this->historyEnd->type == HistoryEntryType::STATE_STATUS) {
        this->historyEnd->globalState->verificationStatus = this->historyEnd->prevStatus;
    }
    else if (this->historyEnd->type == HistoryEntryType::MARK_DECISION_AS_INVALID) {
        this->historyEnd->decision->isInvalidDecision = false;
    }
    
    auto entry = this->historyEnd;
    this->historyEnd = this->historyEnd->prev;
    this->historyEnd->next = nullptr;
    if (freeMemory) {
        delete entry;
    }
}

/// @brief Rolls back the history entries up to the certain HistoryEntry.
/// @param historyEntry Pointer to a HistoryEntry that the history has to be rolled back to.
/// @param inclusive True if the rollback has to remove the specified entry too.
/// @param depth Integer that will be multiplied by 4 and appended as a prefix to the optional debug log.
void Verification::undoHistoryUntil(HistoryEntry* historyEntry, bool inclusive, int depth) {
    #if VERBOSE
        // auto prefix = string(depth * 4, ' ');
        auto histDbg = HistoryDbg();
        auto x = this->historyStart->next;
        while (x != nullptr) {
            histDbg.addEntry(x);
            x = x->next;
        }
    #endif
    while (this->historyEnd != this->historyStart && this->historyEnd != historyEntry) {
        #if VERBOSE
            histDbg.markEntry(this->historyEnd, '-');
        #endif
        this->undoLastHistoryEntry(true);
    }
    if (inclusive && this->historyEnd == historyEntry) {
        #if VERBOSE
            histDbg.markEntry(this->historyEnd, '-');
        #endif
        this->undoLastHistoryEntry(true);
    }
    #if VERBOSE
        histDbg.print(DEPTH_PREFIX);
    #endif
}

/// @brief Prints current history to the console.
/// @param depth Integer that will be multiplied by 4 and appended as a prefix to the optional debug log.
void Verification::printCurrentHistory(int depth) {
    printf("%sCURRENT HISTORY:\n", DEPTH_PREFIX.c_str());
    auto histDbg = HistoryDbg();
    auto x = this->historyStart->next;
    while (x != nullptr) {
        histDbg.addEntry(x);
        x = x->next;
    }
    histDbg.print(DEPTH_PREFIX);
}

/// @brief Checks if two global transitions are made up of the same local transitions
/// @param globalTransition1 First global transition to compare.
/// @param globalTransition2 Second global transition to compare.
/// @return True if the two global transitions have the same local transitions, false otherwise.
bool Verification::equivalentGlobalTransitions(GlobalTransition* globalTransition1, GlobalTransition* globalTransition2) {
    bool isEquivalent = true;
    bool isMatchFound = false;
    for (const auto localTransition1 : globalTransition1->localTransitions) {
        if (isAgentInCoalition(localTransition1->agent)) {
            for (const auto localTransition2 : globalTransition2->localTransitions) {
                if (isAgentInCoalition(localTransition2->agent) && localTransition1->localName == localTransition2->localName) {
                    isMatchFound = true;
                    break;
                }
            }
            if (!isMatchFound) {
                isEquivalent = false;
                break;
            }
        }
    }
    return isEquivalent;
}

/// @brief Verifies if each transition from a given state yields a correct result.
/// @param uncontrolledGlobalTransitions A set of global transitions to be checked.
/// @param globalState Currently processed global state.
/// @param depth Current recursion depth.
/// @param hasOmittedTransitions Flag with the information about skipped unneeded transitions.
/// @return Returns true if every transition yields a correct result, false otherwise.
bool Verification::checkUncontrolledSet(set<GlobalTransition*> uncontrolledGlobalTransitions, GlobalState* globalState, int depth, bool hasOmittedTransitions) {
    for (const auto globalTransition : uncontrolledGlobalTransitions) {
        if (this->mode == TraversalMode::RESTORE) {
            // Skip loop iterations performed before the one from historyToRestore
            // Won't affect iterations to perform after, because mode would have been changed back to NORMAL by then (possibly in recursive verifyGlobalM<odel() calls)
            if (!this->restoreHistory(globalState, globalTransition, depth, false)) {
                continue;
            }
        }
        
        // About to go deeper - add history entry with type=CONTEXT
        this->addHistoryContext(globalState, depth, globalTransition, false);
        
        #if VERBOSE
            printf("%senter UNcontrolled %s -> %s\n", DEPTH_PREFIX.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
        #endif
        auto isTransitionValid = this->verifyGlobalState(globalTransition->to, depth + 1);
        if (this->mode == TraversalMode::REVERT) {
            // Recursive verifyGlobalState caused REVERT mode
            if (globalState == this->revertToGlobalState) {
                // This is the "top" state (first Y in selene-ver2.png) from which recursion should be rebuilt
                this->revertToGlobalState = nullptr;
                if (this->historyToRestore.empty()) {
                    this->mode = TraversalMode::NORMAL;
                    #if VERBOSE
                        printf("%sset mode=NORMAL\n", DEPTH_PREFIX.c_str());
                    #endif
                }
                else {
                    this->mode = TraversalMode::RESTORE;
                    #if VERBOSE
                        printf("%sset mode=RESTORE\n", DEPTH_PREFIX.c_str());
                    #endif
                }
                this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::UNVERIFIED);
                globalState->verificationStatus = GlobalStateVerificationStatus::UNVERIFIED;
                return this->verifyGlobalState(globalState, depth); // Same state, same depth
            }
            else {
                return false;
            }
        }
        if (!isTransitionValid) {
            if (hasOmittedTransitions) {
                bool reverted = this->revertLastDecision(depth);
                #if VERBOSE
                    if (reverted) {
                        printf("%srevertLastDecision to %s (inside %s)\n", DEPTH_PREFIX.c_str(), this->revertToGlobalState->hash.c_str(), globalState->hash.c_str());
                    }
                    else {
                        printf("%srevertLastDecision failed (inside %s)\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str());
                    }
                #endif
                return false;// this->verifyGlobalState(globalState, depth);
            }
            this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_ERR);
            dbgVerifStatus(DEPTH_PREFIX, globalState, GlobalStateVerificationStatus::VERIFIED_ERR, "!isTransitionValid (uncontrolled)");
            globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
            return false;
        }
    }
    return true;
}

/// @brief Checks if given transition sets are able to fulfill the formula for its given epistemic class.
/// @param controlledGlobalTransitions Set of controlled transitions in the current global state.
/// @param uncontrolledGlobalTransitions Set of uncontrolled transitions in the current global state.
/// @param globalState Currently processed global state.
/// @param depth Current recursion depth.
/// @param hasOmittedTransitions Flag with the information about skipped unneeded transitions.
/// @return True if there is a correct choice for an agent to take, false otherwise.
bool Verification::verifyTransitionSets(set<GlobalTransition*> controlledGlobalTransitions, set<GlobalTransition*> uncontrolledGlobalTransitions, GlobalState* globalState, int depth, bool hasOmittedTransitions, bool isFMode) {
    auto epistemicClass = this->getEpistemicClassForGlobalState(globalState);
    auto fixedGlobalTransition = epistemicClass != nullptr ? epistemicClass->fixedCoalitionTransition : nullptr;
    bool isMixedControlTransitions = false;

    if (controlledGlobalTransitions.size() > 0 && uncontrolledGlobalTransitions.size() > 0) {
        isMixedControlTransitions = true;
    }

    // 1) verify paths controlled by the coalition (no controlled transitions || at least one is OK)
    if (controlledGlobalTransitions.size() > 0) {
        bool hasValidControlledTransition = false;
        bool hasValidChoiceTransition = false;
        for (const auto globalTransition : controlledGlobalTransitions) {
            if (this->mode == TraversalMode::RESTORE) {
                // Skip loop iterations performed before the one from historyToRestore
                // Won't affect iterations to perform after, because mode would have been changed back to NORMAL by then (possibly in recursive verifyGlobalM<odel() calls)
                if (!this->restoreHistory(globalState, globalTransition, depth, true)) {
                    continue;
                }
            }
            
            // Ensure that the transtiion wasn't marked as invalid as a part of the RESTORE-REVERT procedure
            if (globalTransition->isInvalidDecision) { 
                #if VERBOSE
                    printf("%sIGNORE invalidDecision in %s -> %s\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str(), globalTransition->to->hash.c_str());
                #endif
                continue;
            }
            
            auto prevHistoryEnd = this->historyEnd;
            
            if (epistemicClass && fixedGlobalTransition == nullptr) {
                epistemicClass->fixedCoalitionTransition = globalTransition; 
                #if VERBOSE
                    printf("%sDECIDE %s -[%s]-> %s\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str(), globalTransition->joinLocalTransitionNames().c_str(),globalTransition->to->hash.c_str());
                #endif
                this->addHistoryDecision(globalState, globalTransition);
            }
            
            // About to go deeper - add history entry with type=CONTEXT
            this->addHistoryContext(globalState, depth, globalTransition, true);
            
            #if VERBOSE
                printf("%senter controlled %s -> %s\n", DEPTH_PREFIX.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
            #endif
            hasValidControlledTransition = this->verifyGlobalState(globalTransition->to, depth + 1);
            if (this->mode == TraversalMode::REVERT && !isFMode) {
                // Recursive verifyGlobalState caused REVERT mode, just exit
                return false;
            }
            if (epistemicClass && fixedGlobalTransition == nullptr && !hasValidControlledTransition) {
                this->undoHistoryUntil(prevHistoryEnd, false, depth); 
                #if VERBOSE
                    printf("%sundoHistoryUntil (inside %s)\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str());
                #endif
            }
            if (hasValidControlledTransition) {
                break;
            }
        }
        // Maybe a controlled action is an option too
        if (uncontrolledGlobalTransitions.size() > 0) {
            hasValidChoiceTransition = true;
            // for (const auto globalTransition : uncontrolledGlobalTransitions) {
            //     set<Agent*> agents = generator->getFormula()->coalition;
            //     for (auto agt : agents) {
            //         cout << (*globalTransition->localTransitions.begin())->name << " " << (*globalTransition->localTransitions.begin())->agent->name.c_str() << " ? " << agt->name << endl;
            //         if ((*globalTransition->localTransitions.begin())->agent->name.c_str() != agt->name.c_str()) {
            //             cout << "uh oh" << endl;
            //             hasValidChoiceTransition = false;
            //         }
            //     }
            // }
            if (!this->checkUncontrolledSet(uncontrolledGlobalTransitions, globalState, depth, hasOmittedTransitions)) {
                hasValidChoiceTransition = false;
            }
            if (epistemicClass && fixedGlobalTransition == nullptr) {
                epistemicClass->fixedCoalitionTransition = *uncontrolledGlobalTransitions.begin();
            }
        }
        if (!hasValidControlledTransition && !hasValidChoiceTransition) {
            this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_ERR);
            dbgVerifStatus(DEPTH_PREFIX, globalState, GlobalStateVerificationStatus::VERIFIED_ERR, "!hasValidControlledTransition");
            globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
            // A different decision could've been made above if transition/decision hadn't been fixed somewhere else
            bool reverted = this->revertLastDecision(depth); 
            #if VERBOSE
                if (reverted) {
                    printf("%srevertLastDecision to %s (inside %s)\n", DEPTH_PREFIX.c_str(), this->revertToGlobalState->hash.c_str(), globalState->hash.c_str());
                }
                else {
                    printf("%srevertLastDecision failed (inside %s)\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str());
                }
            #endif
            return false;
        }
    }
    
    // 2) verify paths not controlled by the coalition (all must be OK)
    if (!isMixedControlTransitions && !this->checkUncontrolledSet(uncontrolledGlobalTransitions, globalState, depth, hasOmittedTransitions)) {
        return false;
    }
    return true;
}

/// @brief Restores the decisions made for a given global state and transition in current recursion depth.
/// @param globalState Currently processed global state.
/// @param globalTransition Previously selected global transition from the given state to mark as invalid.
/// @param depth Current recursion depth.
/// @param controlled Flag with the information about current type of transition. True if controlled, false if uncontrolled.
/// @return Returns true if current top of the history entires matches with 
bool Verification::restoreHistory(GlobalState* globalState, GlobalTransition* globalTransition, int depth, bool controlled) {
    // Check if top history entry matches this loop iteration
    auto entry = this->historyToRestore.top();
    if (entry->type == HistoryEntryType::MARK_DECISION_AS_INVALID) {
        entry->decision->isInvalidDecision = true;
        this->addHistoryMarkDecisionAsInvalid(entry->globalState, entry->decision);
        this->historyToRestore.pop();
        if (!this->historyToRestore.empty()) {
            delete entry;
            entry = this->historyToRestore.top();
        }
    }
    bool matches = entry->type == HistoryEntryType::CONTEXT && entry->globalState == globalState && entry->depth == depth && entry->decision == globalTransition && entry->globalTransitionControlled == controlled;
    #if VERBOSE
        if (matches) {
            if (controlled) {
                printf("%srestore: matched %s -> %s (controlled)\n", DEPTH_PREFIX.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
            }
            else {
                printf("%srestore: matched %s -> %s (uncontrolled)\n", DEPTH_PREFIX.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
            }
        }
        else {
            if (controlled) {
                printf("%srestore: ignoring %s -> %s (controlled)\n", DEPTH_PREFIX.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
            }
            else {
                printf("%srestore: ignoring %s -> %s (uncontrolled)\n", DEPTH_PREFIX.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
            }
        }
    #endif
    
    // Delete top history entry
    if (matches) {
        this->historyToRestore.pop();
        delete entry;
    }
    if (!this->historyToRestore.empty() && this->historyToRestore.top()->type == HistoryEntryType::MARK_DECISION_AS_INVALID) {
        entry = this->historyToRestore.top();
        entry->decision->isInvalidDecision = true;
        this->addHistoryMarkDecisionAsInvalid(entry->globalState, entry->decision);
        this->historyToRestore.pop();
        delete entry;
    }
    if (this->historyToRestore.empty()) {
        // Last entry to restore - exit restore mode
        this->mode = TraversalMode::NORMAL;
        #if VERBOSE
            printf("%sset mode=NORMAL\n", DEPTH_PREFIX.c_str());
        #endif
    }

    return matches;
}

/// @brief Prints out the first ERR path.
void Verification::historyDecisionsERR() {
    GlobalState* lastState = this->generator->getCurrentGlobalModel()->initState;
    set<string> visited;
    visited.insert(lastState->hash);
    bool changed = true;
    
    while (changed) {
        cout << "States:" << endl;
        for (auto local : lastState->localStatesProjection) {
            cout << local->name << ";";
        }
        cout << endl;
        for (auto local : lastState->localStatesProjection) {
            cout << "[" << local->agent->name << "] " << local->name << " (";
            for (auto val : local->environment) {
                cout << val.first << "=" << val.second << ";";
            }
            cout << ")" << endl;
        }
        cout << endl;

        changed = false;
        for (auto transition : lastState->globalTransitions) {
            if (transition->to->verificationStatus == GLOBAL_STATE_VERIFICATION_STATUS::VERIFIED_ERR) {
                lastState = transition->to;
                cout << "Decisions:\n" << transition->joinLocalTransitionNames().c_str() << endl;
                for (auto val : transition->localTransitions) {
                    cout << "[" << val->agent->name << "]" << " " << val->name << endl;
                }
                cout << endl;
                if (visited.find(transition->to->hash) == visited.end()) {
                    changed = true;
                }
                visited.insert(transition->to->hash);
                break;
            }
        }
    }
}