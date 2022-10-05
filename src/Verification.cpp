#include "Verification.hpp"

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

void dbgVerifStatus(string prefix, GlobalState* gs, GlobalStateVerificationStatus st, string reason) {
    #if VERBOSE
        string prevStatus = verStatusToStr(gs->verificationStatus);
        string newStatus = verStatusToStr(st);
        printf("%sset verifStatus of %s: %s -> %s (%s)\n", prefix.c_str(), gs->hash.c_str(), prevStatus.c_str(), newStatus.c_str(), reason.c_str());
    #endif
}

void dbgHistEnt(string prefix, HistoryEntry *h) {
    #if VERBOSE
        printf("%s%s\n", prefix.c_str(), h->toString().c_str());
    #endif
}

HistoryDbg::HistoryDbg() {
}

HistoryDbg::~HistoryDbg() {
    for (auto &kvp : this->entries) {
        if (kvp.second == '-') {
            delete kvp.first;
        }
    }
}

void HistoryDbg::addEntry(HistoryEntry* entry) {
    this->entries.push_back(make_pair(entry, ' '));
}

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

void HistoryDbg::print(string prefix) {
    printf("%svvvvvvvvvv History:\n", prefix.c_str());
    for (auto pair : this->entries) {
        printf("%s    [%c] %s\n", prefix.c_str(), pair.second, pair.first->toString().c_str());
    }
    printf("%s^^^^^^^^^^\n", prefix.c_str());
}

HistoryEntry* HistoryDbg::cloneEntry(HistoryEntry* entry) {
    for (auto &kvp : this->entries) {
        if (kvp.first == entry) {
            kvp.first = new HistoryEntry(*entry);
            return kvp.first;
        }
    }
    return nullptr;
}





Verification::Verification(GlobalModelGenerator* generator) {
    this->generator = generator;
    this->seleneFormula = new SeleneFormula1();
    
    this->historyStart = new HistoryEntry();
    this->historyStart->globalState = nullptr;
    this->historyStart->decision = nullptr;
    this->historyStart->prev = nullptr;
    this->historyStart->next = nullptr;
    
    this->historyEnd = this->historyStart;
}

Verification::~Verification() {
}

bool Verification::verify() {
    this->mode = Mode::NORMAL;
    this->revertToGlobalState = nullptr;
    const auto initState = this->generator->getCurrentGlobalModel()->initState;
    return this->verifyGlobalState(initState, 0);
}

bool Verification::verifyLocalStates(set<LocalState*>* localStates) {
    #if MODEL_ID == 2
        return this->seleneFormula->verifyLocalStates(localStates);
    #endif
    #if MODEL_ID == 3
        auto localState = this->seleneFormula->getLocalStateForAgent("Coercer1", localStates);
	    auto localState2 = this->seleneFormula->getLocalStateForAgent("Voter1",localStates);
        if (localState == nullptr) {
            return false;
        }
        if (localState2 == nullptr) {
            return false;
        }
        int Coercer1_pun1 = this->seleneFormula->getLocalStateVar("Coercer1_pun1", localState);
        int Coercer1_npun1 = this->seleneFormula->getLocalStateVar("Coercer1_npun1", localState);
        int Coercer1_Voter1_vote = this->seleneFormula->getLocalStateVar("Coercer1_Voter1_vote", localState);
        int Voter1_vote = this->seleneFormula->getLocalStateVar("Voter1_vote", localState2);
        //bool result = (Coercer1_pun1 != 1);
        //bool result = (Coercer1_Voter1_vote != 1 || Coercer1_pun1 != 1) && (Voter1_vote != 2 || Coercer1_npun1 !=1);
        //bool result = (Voter1_vote != 1 || Coercer1_pun1 != 1) && (Voter1_vote != 2 || Coercer1_npun1 !=1);
        bool result = ((Voter1_vote != 2) || (Coercer1_npun1 == 0)) && ((Voter1_vote != 2) || (Coercer1_pun1 == 0));
	return result;
    #endif
    #if MODEL_ID == 101
        auto localState = this->seleneFormula->getLocalStateForAgent("Coercer1", localStates);
        if (localState == nullptr) {
            return false;
        }
        int Coercer1_pun1 = this->seleneFormula->getLocalStateVar("Coercer1_pun1", localState);
        bool result = (Coercer1_pun1 != 1);
        return result;
    #endif
    #if MODEL_ID == 102
        auto localState = this->seleneFormula->getLocalStateForAgent("Agt", localStates);
        if (localState == nullptr) {
            return false;
        }
        int Agt1_var1 = this->seleneFormula->getLocalStateVar("Agt_fail", localState);
        bool result = (Agt1_var1 != 1);
        return result;
    #endif
    #if MODEL_ID == 103 || MODEL_ID == 104
        auto localState = this->seleneFormula->getLocalStateForAgent("Agt2", localStates);
        if (localState == nullptr) {
            return false;
        }
        int Agt1_var1 = this->seleneFormula->getLocalStateVar("Agt2_fail", localState);
        bool result = (Agt1_var1 != 1);
        return result;
    #endif
    // <<Train1>>G(Train1_pos=3)
    // OR:
    // <<Train1>>G(Train1_pos=1 || Train1_pos=2 || Train1_pos=3)
    for (const auto localState : *localStates) {
        if (localState->agent->name == "Train1") {
            #if MODEL_ID == 0
                for (const auto var : localState->vars) {
                    if (var.first->name == "Train1_pos") {
                        // return var.second == 3;
                        return var.second == 1 || var.second == 2 || var.second == 3;
                    }
                }
            #elif MODEL_ID == 1
                for (const auto var : localState->environment) {
                    if (var.first == "Train1_pos") {
                        // return var.second == 3;
                        return var.second == 1 || var.second == 2 || var.second == 3;
                    }
                }
            #endif
        }
    }
    return false;
}

bool Verification::verifyGlobalState(GlobalState* globalState, int depth) {
    string prefix = string(depth * 4, ' ');
    #if VERBOSE
        if (globalState->verificationStatus == GlobalStateVerificationStatus::UNVERIFIED) {
            printf("%s> verify globalState: hash=%s (unverified)\n", prefix.c_str(), globalState->hash.c_str());
        }
        else if (globalState->verificationStatus == GlobalStateVerificationStatus::PENDING) {
            printf("%s> skipping verification of globalState: hash=%s (verification pending)\n", prefix.c_str(), globalState->hash.c_str());
        }
        else if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_OK) {
            printf("%s> skipping verification of globalState: hash=%s (verified OK)\n", prefix.c_str(), globalState->hash.c_str());
        }
        else if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_ERR) {
            printf("%s> skipping verification of globalState: hash=%s (verified ERR)\n", prefix.c_str(), globalState->hash.c_str());
        }
    #endif
    if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_ERR) {
        return false;
    }
    else if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_OK) {
        return true;
    }
    else if (globalState->verificationStatus == GlobalStateVerificationStatus::PENDING) {
        return true;
    }
    if (this->historyEnd != nullptr && this->historyEnd->type == HistoryEntryType::STATE_STATUS && this->historyEnd->globalState == globalState && this->historyEnd->prevStatus == GlobalStateVerificationStatus::PENDING && this->historyEnd->newStatus == GlobalStateVerificationStatus::UNVERIFIED) {
        dbgVerifStatus(prefix, globalState, GlobalStateVerificationStatus::PENDING, "entered state w/ undo");
        this->undoLastHistoryEntry(true);
    }
    else {
        this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::PENDING);
        dbgVerifStatus(prefix, globalState, GlobalStateVerificationStatus::PENDING, "entered state");
        globalState->verificationStatus = GlobalStateVerificationStatus::PENDING;
    }
    
    // 1) verify localStates that the globalState is composed of
    if (!this->verifyLocalStates(&globalState->localStates)) {
        this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_ERR);
        dbgVerifStatus(prefix, globalState, GlobalStateVerificationStatus::VERIFIED_ERR, "localStates verification");
        globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
        return false;
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
        if (this->isGlobalTransitionControlledByCoalition(globalTransition)) {
            if (fixedGlobalTransition == nullptr) {
                controlledGlobalTransitions.insert(globalTransition);
            }
            else if (this->areGlobalStatesInTheSameEpistemicClass(fixedGlobalTransition->to, globalTransition->to)) {
                // controlled transition that is fixed should be treated as an uncontrolled transition 
                #if VERBOSE
                    printf("%streat controlled as uncontrolled: %s -> %s\n", prefix.c_str(), globalState->hash.c_str(), globalTransition->to->hash.c_str());
                #endif
                uncontrolledGlobalTransitions.insert(globalTransition);
            }
            else {
                // omit controlled transition that is != fixedGlobalTransition
                #if VERBOSE
                    printf("%somit controlled: %s -> %s\n", prefix.c_str(), globalState->hash.c_str(), globalTransition->to->hash.c_str());
                #endif
                hasOmittedTransitions = true;
            }
        }
        else {
            uncontrolledGlobalTransitions.insert(globalTransition);
        }
    }
    
    // 4) verify paths controlled by the coalition (no controlled transitions || at least one is OK)
    if (controlledGlobalTransitions.size() > 0) {
        bool hasValidControlledTransition = false;
        for (const auto globalTransition : controlledGlobalTransitions) {
            if (this->mode == Mode::RESTORE) {
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
                bool matches = entry->type == HistoryEntryType::CONTEXT && entry->globalState == globalState && entry->depth == depth && entry->decision == globalTransition && entry->globalTransitionControlled == true;
                #if VERBOSE
                    if (matches) {
                        printf("%srestore: matched %s -> %s (controlled)\n", prefix.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
                    }
                    else {
                        printf("%srestore: ignoring %s -> %s (controlled)\n", prefix.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
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
                    this->mode = Mode::NORMAL;
                    #if VERBOSE
                        printf("%sset mode=NORMAL\n", prefix.c_str());
                    #endif
                }
                
                // Skip loop iterations performed before the one from historyToRestore
                // Won't affect iterations to perform after, because mode would have been changed back to NORMAL by then (possibly in recursive verifyGlobalM<odel() calls)
                if (!matches) {
                    continue;
                }
            }
            
            // Ensure that the transtiion wasn't marked as invalid as a part of the RESTORE-REVERT procedure
            if (globalTransition->isInvalidDecision) { 
                #if VERBOSE
                    printf("%sIGNORE invalidDecision in %s -> %s\n", prefix.c_str(), globalState->hash.c_str(), globalTransition->to->hash.c_str());
                #endif
                continue;
            }
            
            auto prevHistoryEnd = this->historyEnd;
            
            if (epistemicClass && fixedGlobalTransition == nullptr) {
                epistemicClass->fixedCoalitionTransition = globalTransition; 
                #if VERBOSE
                    printf("%sDECIDE %s->%s\n", prefix.c_str(), globalState->hash.c_str(), globalTransition->to->hash.c_str());
                #endif
                this->addHistoryDecision(globalState, globalTransition);
            }
            
            // About to go deeper - add history entry with type=CONTEXT
            this->addHistoryContext(globalState, depth, globalTransition, true);
            
            #if VERBOSE
                printf("%senter controlled %s -> %s\n", prefix.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
            #endif
            hasValidControlledTransition = this->verifyGlobalState(globalTransition->to, depth + 1);
            if (this->mode == Mode::REVERT) {
                // Recursive verifyGlobalState caused REVERT mode, just exit
                return false;
            }
            if (epistemicClass && fixedGlobalTransition == nullptr && !hasValidControlledTransition) {
                this->undoHistoryUntil(prevHistoryEnd, false, depth); 
                #if VERBOSE
                    printf("%sundoHistoryUntil (inside %s)\n", prefix.c_str(), globalState->hash.c_str());
                #endif
            }
            if (hasValidControlledTransition) {
                break;
            }
        }
        if (!hasValidControlledTransition) {
            this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_ERR);
            dbgVerifStatus(prefix, globalState, GlobalStateVerificationStatus::VERIFIED_ERR, "!hasValidControlledTransition");
            globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
            // A different decision could've been made above if transition/decision hadn't been fixed somewhere else
            bool reverted = this->revertLastDecision(depth); 
            #if VERBOSE
                if (reverted) {
                    printf("%srevertLastDecision to %s (inside %s)\n", prefix.c_str(), this->revertToGlobalState->hash.c_str(), globalState->hash.c_str());
                }
                else {
                    printf("%srevertLastDecision failed (inside %s)\n", prefix.c_str(), globalState->hash.c_str());
                }
            #endif
            return false;
        }
    }
    
    // 5) verify paths not controlled by the coalition (all must be OK)
    for (const auto globalTransition : uncontrolledGlobalTransitions) {
        if (this->mode == Mode::RESTORE) {
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
            bool matches = entry->type == HistoryEntryType::CONTEXT && entry->globalState == globalState && entry->depth == depth && entry->decision == globalTransition && entry->globalTransitionControlled == false;
            #if VERBOSE
                if (matches) {
                    printf("%srestore: matched %s -> %s (uncontrolled)\n", prefix.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
                }
                else {
                    printf("%srestore: ignoring %s -> %s (uncontrolled)\n", prefix.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
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
                this->mode = Mode::NORMAL;
                #if VERBOSE
                    printf("%sset mode=NORMAL\n", prefix.c_str());
                #endif
            }
            
            // Skip loop iterations performed before the one from historyToRestore
            // Won't affect iterations to perform after, because mode would have been changed back to NORMAL by then (possibly in recursive verifyGlobalM<odel() calls)
            if (!matches) {
                continue;
            }
        }
        
        // About to go deeper - add history entry with type=CONTEXT
        this->addHistoryContext(globalState, depth, globalTransition, false);
        
        #if VERBOSE
            printf("%senter UNcontrolled %s -> %s\n", prefix.c_str(), globalTransition->from->hash.c_str(), globalTransition->to->hash.c_str());
        #endif
        auto isTransitionValid = this->verifyGlobalState(globalTransition->to, depth + 1);
        if (this->mode == Mode::REVERT) {
            // Recursive verifyGlobalState caused REVERT mode
            if (globalState == this->revertToGlobalState) {
                // This is the "top" state (first Y in selene-ver2.png) from which recursion should be rebuilt
                this->revertToGlobalState = nullptr;
                if (this->historyToRestore.empty()) {
                    this->mode = Mode::NORMAL;
                    #if VERBOSE
                        printf("%sset mode=NORMAL\n", prefix.c_str());
                    #endif
                }
                else {
                    this->mode = Mode::RESTORE;
                    #if VERBOSE
                        printf("%sset mode=RESTORE\n", prefix.c_str());
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
                        printf("%srevertLastDecision to %s (inside %s)\n", prefix.c_str(), this->revertToGlobalState->hash.c_str(), globalState->hash.c_str());
                    }
                    else {
                        printf("%srevertLastDecision failed (inside %s)\n", prefix.c_str(), globalState->hash.c_str());
                    }
                #endif
                return false;// this->verifyGlobalState(globalState, depth);
            }
            this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_ERR);
            dbgVerifStatus(prefix, globalState, GlobalStateVerificationStatus::VERIFIED_ERR, "!isTransitionValid (uncontrolled)");
            globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
            return false;
        }
    }
    
    // 6) all passed
    this->addHistoryStateStatus(globalState, globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_OK);
    dbgVerifStatus(prefix, globalState, GlobalStateVerificationStatus::VERIFIED_OK, "all passed");
    globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;
    return true;
}

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

bool Verification::isAgentInCoalition(Agent* agent) {
    const auto coalition = &this->generator->getFormula()->coalition;
    return coalition->find(agent) != coalition->end();
}

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

bool Verification::areGlobalStatesInTheSameEpistemicClass(GlobalState* globalState1, GlobalState* globalState2) {
    auto epiCls1 = this->getEpistemicClassForGlobalState(globalState1);
    auto epiCls2 = this->getEpistemicClassForGlobalState(globalState2);
    if (epiCls1 == nullptr || epiCls2 == nullptr) {
        return false;
    }
    return epiCls1 == epiCls2;
}

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

HistoryEntry* Verification::newHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision) {
    auto newHistoryEntry = new HistoryEntry();
    newHistoryEntry->type = HistoryEntryType::MARK_DECISION_AS_INVALID;
    newHistoryEntry->globalState = globalState;
    newHistoryEntry->decision = decision;
    newHistoryEntry->next = nullptr;
    return newHistoryEntry;
}

void Verification::addHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision) {
    auto newHistoryEntry = this->newHistoryMarkDecisionAsInvalid(globalState, decision);
    newHistoryEntry->prev = this->historyEnd;
    this->historyEnd->next = newHistoryEntry;
    this->historyEnd = newHistoryEntry;
}

bool Verification::revertLastDecision(int depth) {
    // History: (based on selene-ver2.png)
    // X -> Y -> T<decision> -> Y -> Z<ERR>
    // shallowestGlobalState = Y.globalState
    // invalidDecisionGlobalState = T.globalState
    auto prefix = string(depth * 4, ' ');
    #if VERBOSE
        printf("%srevertLastDecision():\n", prefix.c_str());
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
        printf("%sset mode=REVERT\n", prefix.c_str());
    #endif
    this->mode = Mode::REVERT;
    #if VERBOSE
        histDbg.print(prefix);
        for (auto he : dbgHEsToDelete) {
            delete he;
        }
    #endif
    this->revertToGlobalState = shallowestGlobalState;
    return true;
}

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

void Verification::undoHistoryUntil(HistoryEntry* historyEntry, bool inclusive, int depth) {
    #if VERBOSE
        auto prefix = string(depth * 4, ' ');
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
        histDbg.print(prefix);
    #endif
}

void Verification::printCurrentHistory(int depth) {
    auto prefix = string(depth * 4, ' ');
    printf("%sCURRENT HISTORY:\n", prefix.c_str());
    auto histDbg = HistoryDbg();
    auto x = this->historyStart->next;
    while (x != nullptr) {
        histDbg.addEntry(x);
        x = x->next;
    }
    histDbg.print(prefix);
}
