#include "SeleneFormula.hpp"





SeleneFormula::SeleneFormula() {
}

SeleneFormula::~SeleneFormula() {
}

LocalState* SeleneFormula::getLocalStateForAgent(string agentName, set<LocalState*>* localStates) {
    for (const auto localState : *localStates) {
        if (localState->agent->name == agentName) {
            return localState;
        }
    }
    return nullptr;
}

int SeleneFormula::getLocalStateVar(string varName, LocalState* localState) {
    auto result = localState->environment.find(varName);
    if (result == localState->environment.end()) {
        return -1;
    }
    auto kvp = *result;
    return kvp.second;
}

inline bool SeleneFormula::implication(bool left, bool right) {
    if (left && !right) {
        return false;
    }
    return true;
}





SeleneFormula1::SeleneFormula1() : SeleneFormula() {
}

SeleneFormula1::~SeleneFormula1() {
}

bool SeleneFormula1::verifyLocalStates(set<LocalState*>* localStates) {
    // <<Coercer1>> G
    // (
    //     // firstImplication:
    //     (
    //         (Coercer1_finish == 1 && Coercer1_VoterC1_vote == 1)
    //         =>
    //         VoterC1_punish == 0
    //     )
    //     &&
    //     // secondImplication:
    //     (
    //         (Coercer1_finish == 1 && VoterC1 != 1)
    //         =>
    //         VoterC1_punish == 1
    //     )
    // )
    
    // Find local state
    auto localState = this->getLocalStateForAgent("Coercer1", localStates);
    if (localState == nullptr) {
        return false;
    }
    
    // Get var values
    int Coercer1_finish = this->getLocalStateVar("Coercer1_finish", localState);
    int Coercer1_VoterC1_vote = this->getLocalStateVar("Coercer1_VoterC1_vote", localState);
    int VoterC1_vote = this->getLocalStateVar("VoterC1_vote", localState);
    int VoterC1_punish = this->getLocalStateVar("VoterC1_punish", localState);
    int Coercer1_VoterC1_revote = this->getLocalStateVar("Coercer1_Voter1_revote", localState);
    
    // Compute formula
    bool firstImplicationLeft = Coercer1_finish == 1 && Coercer1_VoterC1_vote == 1;
    bool firstImplicationRight = VoterC1_punish == 0;
    bool secondImplicationLeft = Coercer1_finish == 1 && VoterC1_vote != 1 && Coercer1_VoterC1_revote == 1;
    bool secondImplicationRight = VoterC1_punish == 1;
    bool firstImplication = this->implication(firstImplicationLeft, firstImplicationRight);
    bool secondImplication = this->implication(secondImplicationLeft, secondImplicationRight);
    bool thirdImplication = this->implication(Coercer1_finish==1,VoterC1_vote != -1);
    //bool result = firstImplication && secondImplication;
    bool result = thirdImplication;
    
    return result;
}

