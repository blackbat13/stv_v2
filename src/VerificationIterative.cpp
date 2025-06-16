/**
 * @file VerificationIterative.cpp
 * @brief Class for verification of the formula on a model.
 * Class for verification of the specified formula on a specified model.
 */
#include "VerificationIterative.hpp"
#define DEPTH_PREFIX string(depth * 4, ' ')

#include <bits/stdc++.h>

extern Cfg config;

// #define VERBOSE 1

/// @brief Converts global verification status into a string.
/// @param status Enum value to be converted.
/// @return Verification status converted into a string.
string verifStatusToStr(GlobalStateVerificationStatus status) {
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

/// @brief Constructor for Verification.
/// @param generator Pointer to GlobalModelGenerator
VerificationIterative::VerificationIterative(GlobalModelGenerator* generator) {
    this->generator = generator;
    
    this->strategyVariableLimit = 0;
    this->variableNames.clear();
}

/// @brief Destructor for Verification.
VerificationIterative::~VerificationIterative() {
}

/// @brief Creates a HistoryEntry of the type DECISION and puts it on top of the stack of the decision history. 
/// @param globalState Pointer to a GlobalState of the model.
/// @param decision Pointer to a GlobalTransition that is to be recorded in the decision history.
void VerificationIterative::addHistoryDecision(GlobalState* globalState, GlobalTransition* decision) {
    DecisionEntry newHistoryEntry;
    newHistoryEntry.type = HistoryEntryType::DECISION;
    newHistoryEntry.globalState = globalState;
    newHistoryEntry.decision = decision;
    history.emplace(newHistoryEntry);
}

/// @brief Creates a HistoryEntry of the type STATE_STATUS and puts it to the top of the decision history.
/// @param globalState Pointer to a GlobalState of the model.
/// @param prevStatus Previous GlobalStateVerificationStatus to be logged.
/// @param newStatus New GlobalStateVerificationStatus to be logged.
void VerificationIterative::addHistoryStateStatus(GlobalState* globalState, GlobalStateVerificationStatus prevStatus, GlobalStateVerificationStatus newStatus) {
    DecisionEntry newHistoryEntry;
    newHistoryEntry.type = HistoryEntryType::STATE_STATUS;
    newHistoryEntry.globalState = globalState;
    newHistoryEntry.prevStatus = prevStatus;
    newHistoryEntry.newStatus = newStatus;
    history.emplace(newHistoryEntry);
}

/// @brief Creates a HistoryEntry of the type CONTEXT and puts it to the top of the decision history.
/// @param globalState Pointer to a GlobalState of the model.
/// @param depth Depth of the recursion of the validation algorithm.
/// @param decision Pointer to a transition GlobalTransition selected by the algorithm.
/// @param globalTransitionControlled True if the GlobalTransition is in the set of global transitions controlled by a coalition and it is not a fixed global transition.
/// @return Returns true if the natural strategy is good for now.
bool VerificationIterative::addHistoryContext(GlobalState* globalState, int depth, GlobalTransition* decision, bool globalTransitionControlled) {
    DecisionEntry newHistoryEntry;
    newHistoryEntry.type = HistoryEntryType::CONTEXT;
    newHistoryEntry.globalState = globalState;
    newHistoryEntry.decision = decision;
    newHistoryEntry.globalTransitionControlled = globalTransitionControlled;
    newHistoryEntry.depth = depth;
    newHistoryEntry.strategy = StrategyEntry();
    
    if (!globalTransitionControlled) {
        history.emplace(newHistoryEntry);
        return true;
    }

    if (config.natural_strategy) {
        StrategyEntry strategyEntry = StrategyEntry();
        strategyEntry.type = ADDED;
        auto coalition = generator->getFormula()->coalition;
        string actionName;
        for(auto item : decision->localTransitions) {
            if(coalition.find(item->agent) != coalition.end()) {
                actionName = item->localName;
                break;
            }
        }
        bitset<STRATEGY_BITS> valueBits = globalStateToValueBits(globalState);
        auto existingStrategy = naturalStrategy.find(valueBits);
        if(existingStrategy != naturalStrategy.end()) {
            if((*existingStrategy).second != actionName) {
                history.emplace(newHistoryEntry);
                return false;
            }
            strategyEntry.type = NOT_MODIFIED;
        }
        naturalStrategy.insert({valueBits, actionName});
        existingStrategy = naturalStrategy.find(valueBits);

        strategyEntry.actionName = &((*existingStrategy).second);
        strategyEntry.globalValues = (*existingStrategy).first;
        newHistoryEntry.strategy = strategyEntry;
        history.emplace(newHistoryEntry);
    }
    return true;
}

void VerificationIterative::addHistoryProbability(GlobalTransition* decision, float* probabilityTrue, float* probabilityFalse) {
    if (decision->from == decision->to) {
        return;
    }
    DecisionEntry newHistoryEntry;
    newHistoryEntry.type = HistoryEntryType::PROBABILITY;
    newHistoryEntry.globalState = decision->to;
    newHistoryEntry.stateProbabilityPrevious = decision->to->probability;
    float globalTransitionProbability = 1.0;
    for (auto localProbability : decision->localTransitions) {
        globalTransitionProbability *= localProbability->probability;
    }
    decision->to->probability += (decision->from->probability * globalTransitionProbability);
    cout << "Changing " << decision->to->hash << " probability from " << newHistoryEntry.stateProbabilityPrevious << " to " << decision->to->probability << endl;
    history.emplace(newHistoryEntry);
    // fix probability if found an already processed state
    addHistoryAnswerProbability(decision->to, probabilityTrue, probabilityFalse, (decision->from->probability * globalTransitionProbability));

    if (decision->to->isExpanded && decision->to->verificationStatus != GlobalStateVerificationStatus::UNVERIFIED && decision->from != decision->to) {
        propagateProbability(decision->to, probabilityTrue, probabilityFalse);
    }
}

void VerificationIterative::addHistoryAnswerProbability(GlobalState* globalState, float *probabilityTrue, float *probabilityFalse, float probabilityChange)
{
    DecisionEntry newHistoryEntry;
    newHistoryEntry.type = HistoryEntryType::ANSWER_PROBABILITY;
    newHistoryEntry.globalState = globalState;
    if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_OK) {
        newHistoryEntry.probabilityTrueChange = probabilityChange;
        // todo: resolve things depending if it should be sum or min
        // *probabilityTrue += probabilityChange;
    } else if (globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_ERR) {
        newHistoryEntry.probabilityFalseChange = probabilityChange;
        // todo: resolve things depending if it should be sum or min
        // *probabilityFalse += probabilityChange;
    }
    cout << "Probability changed to T:" << *probabilityTrue << " F:" << *probabilityFalse << endl;
    history.emplace(newHistoryEntry);
}

void VerificationIterative::propagateProbability(GlobalState* fromState, float* probabilityTrue, float* probabilityFalse) {
    for (auto globalTransition : fromState->globalTransitions) {
        if (globalTransition->to->isExpanded && globalTransition->to->verificationStatus != GlobalStateVerificationStatus::UNVERIFIED) {
            addHistoryProbability(globalTransition, probabilityTrue, probabilityFalse);
        }
    }
}

// todo: propagate final probability upwards and add that to history

/// @brief Creates a HistoryEntry of the type MARK_DECISION_AS_INVALID and puts it to the top of the decision history.
/// @param globalState Pointer to a GlobalState of the model.
/// @param decision Pointer to a transition GlobalTransition selected by the algorithm.
void VerificationIterative::addHistoryMarkDecisionAsInvalid(GlobalState* globalState, GlobalTransition* decision) {
    DecisionEntry newHistoryEntry;
    newHistoryEntry.type = HistoryEntryType::MARK_DECISION_AS_INVALID;
    newHistoryEntry.globalState = globalState;
    newHistoryEntry.decision = decision;
    history.emplace(newHistoryEntry);
}

/// @brief Removes the top entry of the history stack.
void VerificationIterative::undoLastHistoryEntry() {
    if (this->history.empty()) {
        return;
    }
    if (this->history.top().type == HistoryEntryType::DECISION) {
        auto epistemicClass = this->getEpistemicClassForGlobalState(this->history.top().globalState);
        epistemicClass->fixedCoalitionTransition = nullptr;
    } else if (this->history.top().type == HistoryEntryType::STATE_STATUS) {
        this->history.top().globalState->verificationStatus = this->history.top().prevStatus;
        cout << "Reverting " << this->history.top().globalState->hash << " to " << this->history.top().prevStatus << endl;
    } else if (this->history.top().type == HistoryEntryType::MARK_DECISION_AS_INVALID) {
        this->history.top().decision->isInvalidDecision = false;
    } else if (this->history.top().type == HistoryEntryType::PROBABILITY) {
        this->history.top().globalState->probability = this->history.top().stateProbabilityPrevious;
    }

    // change probability here
    // todo

    if (this->history.top().strategy.type == StrategyEntryType::ADDED) {
        #if VERBOSE
            cout << "REMOVED " << this->historyEnd->strategy.type << " " << this->historyEnd->strategy.actionName->c_str() << " " << this->historyEnd->strategy.globalValues << endl;
        #endif
        this->naturalStrategy.erase(this->history.top().strategy.globalValues);
    }
    
    history.pop();
}

bool VerificationIterative::revertToLastDecision()
{
    // Undo Z->T (including Z, excluding T); find Y while doing that
    int shallowestDepth = 99999999;
    GlobalState* shallowestGlobalState = nullptr;
    while (!this->history.empty() && this->history.top().type != HistoryEntryType::DECISION) {
        // cout << "[!] " << this->history.top().depth << " " << this->history.top().globalState->hash << " " << this->history.top().type << endl;
        if (this->history.top().type == HistoryEntryType::CONTEXT && this->history.top().depth < shallowestDepth) {
            shallowestDepth = this->history.top().depth;
            shallowestGlobalState = this->history.top().globalState;
        }
        this->undoLastHistoryEntry();
    }
    if (!this->history.empty()) {
        // cout << "[found] " << this->history.top().depth << " " << this->history.top().globalState->hash << " " << this->history.top().type << endl;
        auto invalidDecisionGlobalState = this->history.top().globalState;
        auto invalidDecision = this->history.top().decision;
        auto invalidDecisionHistoryEntry = this->history.top();
        history.pop();
        if (!invalidDecision->isInvalidDecision) {
            // Mark the decision (transition) as invalid
            this->addHistoryMarkDecisionAsInvalid(invalidDecisionGlobalState, invalidDecision);
            invalidDecision->isInvalidDecision = true;
        }
    }
    
    if (this->history.empty() || this->history.top().type != HistoryEntryType::DECISION || shallowestGlobalState == nullptr) {
        return false;
    }
    
    // Undo T->Y (including T, excluding Y); build path to repeat while doing that
    auto invalidDecisionGlobalState = this->history.top().globalState;
    auto invalidDecision = this->history.top().decision;
    auto invalidDecisionHistoryEntry = this->history.top();
    // cout << "there2" << endl;

    if (!invalidDecision->isInvalidDecision) {
        // cout << "there4" << endl;
        // Mark the decision (transition) as invalid
        this->addHistoryMarkDecisionAsInvalid(invalidDecisionGlobalState, invalidDecision);
        invalidDecision->isInvalidDecision = true;
    }

    cout << shallowestGlobalState->hash << endl;
    while (!this->history.empty() && this->history.top().globalState != shallowestGlobalState) {
        // cout << "[!2] " << this->history.top().globalState->hash << " " << this->history.top().type << endl;
        this->undoLastHistoryEntry();
    }
    while (this->history.top().globalState != this->statesToProcess.top().globalState) {
        // cout << "[kick] " << statesToProcess.top().globalState->hash << endl;
        this->statesToProcess.pop();
    }

    // cout << "there3" << endl;
    return true;
}

/// @brief Verifies a set of LocalState that a GlobalState is composed of with a hardcoded formula.
/// @param localStates A pointer to a set of pointers to LocalState.
/// @return Returns true if there is a LocalState with a specific set of values, fulfilling the criteria, otherwise returns false.
bool VerificationIterative::verifyLocalStates(vector<LocalState*>* localStates, GlobalState* globalState) {
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

/// @brief Checks if any of the LocalTransition in a given GlobalTransition has an Agent in a coalition in the formula.
/// @param globalTransition Pointer to a GlobalTransition in a model.
/// @return Returns true if the Agent is in coalition in the formula, otherwise returns false.
bool VerificationIterative::isGlobalTransitionControlledByCoalition(GlobalTransition* globalTransition) {
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
bool VerificationIterative::isAgentInCoalition(Agent* agent) {
    const auto coalition = &this->generator->getFormula()->coalition;
    return coalition->find(agent) != coalition->end();
}

/// @brief Gets the EpistemicClass for the agent in passed GlobalState, i.e. transitions from indistinguishable state from certain other states for an agent to other states.
/// @param globalState Pointer to a GlobalState of the model.
/// @return Pointer to the EpistemicClass that a coalition of agents from the formula belong to. If there is no such EpistemicClass, returns false.
EpistemicClass* VerificationIterative::getEpistemicClassForGlobalState(GlobalState* globalState) {
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
bool VerificationIterative::areGlobalStatesInTheSameEpistemicClass(GlobalState* globalState1, GlobalState* globalState2) {
    auto epiCls1 = this->getEpistemicClassForGlobalState(globalState1);
    auto epiCls2 = this->getEpistemicClassForGlobalState(globalState2);
    if (epiCls1 == nullptr || epiCls2 == nullptr) {
        return false;
    }
    return epiCls1 == epiCls2;
}

/// @brief Checks if two global transitions are made up of the same local transitions
/// @param globalTransition1 First global transition to compare.
/// @param globalTransition2 Second global transition to compare.
/// @return True if the two global transitions have the same local transitions, false otherwise.
bool VerificationIterative::equivalentGlobalTransitions(GlobalTransition* globalTransition1, GlobalTransition* globalTransition2) {
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

/// @brief Changes globalState variables to a bitset used in natural strategy synthesis.
/// @param globalState State that we want to extractvariable values from.
/// @return Bitset of environment variable values.
bitset<STRATEGY_BITS> VerificationIterative::globalStateToValueBits(GlobalState* globalState) {
    LocalState* agentState;
    for(auto agentStates : globalState->localStatesProjection) {
        if(agentStates->agent->name == (*(this->generator->getFormula()->coalition.begin()))->name) {
            agentState = agentStates;
            break;
        }
    }
    bitset<STRATEGY_BITS> currentValues = 0;
    if (this->strategyVariableLimit == 0) {
        this->variableNames.clear();
    }
    int it = 0;
    for(auto variable : agentState->environment) {
        currentValues |= ((variable.second ? 1 : 0) << it);
        if (this->strategyVariableLimit == 0) {
            this->variableNames.push_back(variable.first);
        }
        it++;
    }
    if (this->strategyVariableLimit == 0) {
        strategyVariableLimit = it;
    }
    return currentValues;
}

/// @brief Get natural strategy map.
/// @return Map of variable values and action names.
map<bitset<STRATEGY_BITS>, string, StrategyBitsComparator> VerificationIterative::getNaturalStrategy() {
    return this->naturalStrategy;
}

/// @brief Takes natural strategy from memory and reduces it using reduceStrategy().
/// @return Vector of tuples <vector of tuples <variable value, variable name>, action name>.
vector<tuple<vector<tuple<bool, string>>, string>> VerificationIterative::getReducedStrategy() {
    vector<tuple<vector<tuple<bool, string>>, string>> result;
    for (auto item : naturalStrategy) {
        tuple<vector<tuple<bool, string>>, string> newItem = {vector<tuple<bool, string>>(strategyVariableLimit, {false, ""}) , ""};
        
        auto values = item.first.to_ulong();
        for (int i = 0; i < strategyVariableLimit; i++) {
            if (values & 1) {
                get<0>(get<0>(newItem)[i]) = true;
            }
            values = values >> 1;
            get<1>(get<0>(newItem)[i]) = variableNames[i];
        }
        get<1>(newItem) = item.second;
        result.push_back(newItem);
    }
    this->reductionComplexityBefore = 0;
    for (auto item : result) {
        this->reductionComplexityBefore += (strategyVariableLimit - 1);
        for (auto values : get<0>(item)) {
            this->reductionComplexityBefore += (get<0>(values) ? 1 : 2);
        }
    }
    auto reduceResult = reduceStrategy(result);
    auto temp = &get<0>(reduceResult[reduceResult.size() - 1]);
    tuple<bool, string> t = {true, "T"};
    temp->clear();
    temp->push_back(t);
    return reduceResult;
}

/// @brief Natural strategy reduction algorithm.
/// @param strategyEntries Vector of tuples <vector of tuples <variable value, variable name>, action name> containing not reduced natural strategy.
/// @param lockedColumn Index of the furthest leftmost locked column.
/// @param upperHalf Indicates if the first columns of the processed strategy entries can't be shuffled. True if they can't be shuffled, false otherwise.
/// @return Vector of tuples <vector of tuples <variable value, variable name>, action name>.
vector<tuple<vector<tuple<bool, string>>, string>> VerificationIterative::reduceStrategy(vector<tuple<vector<tuple<bool, string>>, string>> strategyEntries, short lockedColumn, bool upperHalf) {
    queue<short> toBeRemoved;
    short minSum = 999, minSumID = 0, maxSum = 0, maxSumID = 0, maxValue = strategyEntries.size();
    // remove unnecessary columns
    for (int i = (!upperHalf ? lockedColumn : lockedColumn + 1); i < get<0>(strategyEntries[0]).size(); i++) {
        short sum = 0;
        for (int j = 0; j < strategyEntries.size(); j++) {
            sum += (get<0>(get<0>(strategyEntries[j])[i]) ? 1 : 0);
        }
        if (sum == strategyEntries.size() || sum == 0) {
            toBeRemoved.push(i);
        }
        if (sum > 0 && sum < minSum) {
            minSum = sum;
            minSumID = i;
        }
        if (sum < strategyEntries.size() && sum > maxSum) {
            maxSum = sum;
            maxSumID = i;
        }
    }
    vector<tuple<vector<tuple<bool, string>>, string>> result(strategyEntries.size(), {vector<tuple<bool, string>>(), ""});

    for (int i = 0; i < strategyEntries.size(); i++) {
        get<1>(result[i]) = get<1>(strategyEntries[i]);
    }
    short totalOffsetMin = 0;
    short totalOffsetMax = 0;
    for (int i = 0; i < get<0>(strategyEntries[0]).size(); i++) {
        if (!toBeRemoved.empty() && toBeRemoved.front() == i) {
            toBeRemoved.pop();
            if (minSumID > i) {
                totalOffsetMin++;
            }
            if (maxSumID > i) {
                totalOffsetMax++;
            }
            continue;
        }
        for (int j = 0; j < strategyEntries.size(); j++) {
            get<0>(result[j]).push_back(get<0>(strategyEntries[j])[i]);
        }
    }
    minSumID -= totalOffsetMin;
    maxSumID -= totalOffsetMax;

    if (result.size() == 1 || get<0>(result[0]).size() == 0 || lockedColumn == get<0>(result[0]).size() - 1) {
        return result;
    }
    //get a division point, pick least valued weighted count of 0's and 1's 
    short winnerID = ((((maxValue - maxSum) * 2) < minSum) ? maxSumID : minSumID);

    bool toLookFor = true;
    if (minSumID != winnerID) {
        toLookFor = false;
    }
    // bring the defining column to the front
    for (int i = 0; i < result.size(); i++) {
        swap(get<0>(result[i])[(!upperHalf ? lockedColumn : lockedColumn + 1)], get<0>(result[i])[winnerID]);
    }

    //sort by defining value
    short swapSpot = 0;
    for (int i = 0; i < result.size(); i++) {
        if(get<0>(get<0>(result[i])[(!upperHalf ? lockedColumn : lockedColumn + 1)]) == toLookFor) {
            swap(result[i], result[swapSpot]);
            swapSpot++;
        }
    }
    
    vector<tuple<vector<tuple<bool, string>>, string>> finalResult;
    finalResult.reserve(result.size());
    vector<tuple<vector<tuple<bool, string>>, string>> upperPart = vector<tuple<vector<tuple<bool, string>>, string>>(result.begin(), result.begin() + swapSpot);
    vector<tuple<vector<tuple<bool, string>>, string>> lowerPart = vector<tuple<vector<tuple<bool, string>>, string>>(result.begin() + swapSpot, result.end());

    upperPart = reduceStrategy(upperPart, lockedColumn + 1, true);
    lowerPart = reduceStrategy(lowerPart, lockedColumn, false);

    finalResult.insert(finalResult.end(), upperPart.begin(), upperPart.end());
    finalResult.insert(finalResult.end(), lowerPart.begin(), lowerPart.end());

    return finalResult;
}

/// @brief Natural strategy complexity before reduction. Each variable, negation, and, or are treated as value 1.
/// @return Total sum of the natural strategy complexity before reduction.
int VerificationIterative::getStrategyComplexity() {
    return this->reductionComplexityBefore;
}

Result VerificationIterative::verify() {
    Result verificationResult;
    VerificationFormulaMode formulaMode = this->generator->getFormula()->isF ? VerificationFormulaMode::F : VerificationFormulaMode::G;
    StateVerificationInfo initState;
    StateVerificationInfo* currentState;
    initState.globalState = this->generator->getCurrentGlobalModel()->initState;
    initState.depth = 0;
    initState.globalState->probability = 1.0;
    initState.probabilityFalse = new float(0.0);
    initState.probabilityTrue = new float(0.0);
    
    statesToProcess.emplace(initState);
    int lastDepth = -1;
    
    while (!statesToProcess.empty()) {
        currentState = &statesToProcess.top();

        auto epistemicClass = this->getEpistemicClassForGlobalState(currentState->globalState);
        auto fixedGlobalTransition = epistemicClass != nullptr ? epistemicClass->fixedCoalitionTransition : nullptr;
        
        if (config.verify_strategy) {
            // cout << string(20, '=') << endl;
            // cout << "Stack size: " << statesToProcess.size() << endl;
            // cout << "Coalition is at: " << generator->getCoalitionIdentifier(&currentState->globalState->localStatesProjection) << endl;
            // cout << "Should execute: " << generator->getActionNameFromStateInStrategy(currentState->globalState) << endl;
        } else {
            // cout << string(20, '=') << endl;
            // cout << currentState->globalState->hash << " " << (currentState->isControlledByCoalition ? "CONTROLLED" : "UNCONTROLLED") << endl;
        }
        // cout << "Has uncontrolled transition 1:" << currentState->hasValidUncontrolledTransition << endl;
        // reached a previously visited state by going forward
        if (currentState->depth > lastDepth) {
            // cout << "New state" << endl;
            lastDepth = currentState->depth;
            if (currentState->globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_ERR) {
                if (currentState->fromState != nullptr) {
                    if (!currentState->isControlledByCoalition) { // if uncontrolled then mark the transition as not good
                        // cout << "FALSE here 3?" << endl;
                        currentState->fromState->hasValidUncontrolledTransition = false;
                    }
                    // cout << "ERR1" << endl;
                }
                // initiate rollback?
                statesToProcess.pop();
                continue;
            } else if (currentState->globalState->verificationStatus == GlobalStateVerificationStatus::VERIFIED_OK) {
                // go on
                if (currentState->isControlledByCoalition) { // if controlled then mark the transition as good
                    currentState->fromState->hasValidControlledTransition = true;
                }
                statesToProcess.pop();
                continue;
            } else if (currentState->globalState->verificationStatus == GlobalStateVerificationStatus::PENDING) {
                if (formulaMode == VerificationFormulaMode::F) {
                    if (currentState->fromState != nullptr) {
                        this->addHistoryStateStatus(currentState->fromState->globalState, currentState->fromState->globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_ERR);
                        currentState->fromState->globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
                        // temporary probability verification fix
                        addHistoryAnswerProbability(currentState->globalState, currentState->probabilityTrue, currentState->probabilityFalse, currentState->globalState->probability);
                        if (!currentState->isControlledByCoalition) { // if uncontrolled then mark the transition as not good
                            // cout << "FALSE here 2?" << endl;
                            currentState->fromState->hasValidUncontrolledTransition = false;
                        }
                        // cout << "ERR2" << endl;
                    }
                } else if (formulaMode == VerificationFormulaMode::G) {
                    if (currentState->isControlledByCoalition) { // if controlled then mark the transition as good
                        currentState->fromState->hasValidControlledTransition = true;
                    }
                }
                // initiate rollback?
                statesToProcess.pop();
                continue;
            }
        } else {
            // cout << "Went back to state" << endl;
        }
        lastDepth = currentState->depth;
        // cout << "Has uncontrolled transition 2:" << currentState->hasValidUncontrolledTransition << endl;

        // do some initial processing for the state
        if (currentState->globalState->verificationStatus == GlobalStateVerificationStatus::UNVERIFIED) {
            // mark state as pending if haven't visited it yet
            this->addHistoryStateStatus(currentState->globalState, currentState->globalState->verificationStatus, GlobalStateVerificationStatus::PENDING);
            currentState->globalState->verificationStatus = GlobalStateVerificationStatus::PENDING;
            // 1) verify localStates that the globalState is composed of
            if (formulaMode == VerificationFormulaMode::F) { // F
                if (currentState->globalState->stateVerifResult == VerifResult::TRUE || this->verifyLocalStates(&currentState->globalState->localStatesProjection, currentState->globalState)) {
                    this->addHistoryStateStatus(currentState->globalState, currentState->globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_OK);
                    currentState->globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;
                    // temporary probability verification fix
                    addHistoryAnswerProbability(currentState->globalState, currentState->probabilityTrue, currentState->probabilityFalse, currentState->globalState->probability);
                    // cache the result for later
                    currentState->globalState->stateVerifResult = VerifResult::TRUE;
                    cout << "OK" << endl;
                    if (currentState->fromState != nullptr) {
                        if (currentState->isControlledByCoalition) { // if controlled then mark the transition as good
                            currentState->fromState->hasValidControlledTransition = true;
                        }
                        statesToProcess.pop();
                        continue;
                    } else { // initial state is good already
                        verificationResult.verificationResult = true;
                        statesToProcess.pop();
                        continue;
                    }
                } else {
                    // cache the result for later
                    currentState->globalState->stateVerifResult = VerifResult::FALSE;
                }
            } else { // G
                if (currentState->globalState->stateVerifResult == VerifResult::FALSE || !this->verifyLocalStates(&currentState->globalState->localStatesProjection, currentState->globalState)) {
                    this->addHistoryStateStatus(currentState->globalState, currentState->globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_ERR);
                    currentState->globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
                    // temporary probability verification fix
                    addHistoryAnswerProbability(currentState->globalState, currentState->probabilityTrue, currentState->probabilityFalse, currentState->globalState->probability);
                    // cache the result for later
                    currentState->globalState->stateVerifResult = VerifResult::FALSE;
                    // cout << "ERR3" << endl;
                    if (currentState->fromState != nullptr) {
                        if (!currentState->isControlledByCoalition) { // if uncontrolled then mark the transition as not good
                            // cout << "FALSE here 1?" << endl;
                            currentState->fromState->hasValidUncontrolledTransition = false;
                        }
                        // todo: trigger revert until last decision
                        statesToProcess.pop();
                        revertToLastDecision();
                        continue;
                    } else { // initial state is bad already
                        verificationResult.verificationResult = false;
                        statesToProcess.pop();
                        continue;
                    }
                } else {
                    // cache the result for later
                    currentState->globalState->stateVerifResult = VerifResult::TRUE;
                }
            }
        }
        if (!currentState->processed) {
            // 2) ensure that the state is expanded
            if (!currentState->globalState->isExpanded) {
                this->generator->expandState(currentState->globalState);
            }

            // classify transition end global state
            for (auto globalTransition : currentState->globalState->globalTransitions) {
                if (globalTransition->isInvalidDecision) { 
                    continue;
                }
                // add CTL support here
                if (this->isGlobalTransitionControlledByCoalition(globalTransition)) {
                    if (!config.verify_strategy) {
                        if (fixedGlobalTransition == nullptr && !globalTransition->isInvalidDecision) {
                            currentState->controlledTransitionsLeftToProcess.emplace(globalTransition);
                        } else if (fixedGlobalTransition == nullptr) {
                            // the decision is invalid, skip it
                        } else if (this->areGlobalStatesInTheSameEpistemicClass(fixedGlobalTransition->to, globalTransition->to) && this->equivalentGlobalTransitions(fixedGlobalTransition, globalTransition)) {
                            // controlled transition that is fixed should be treated as an uncontrolled transition 
                            #if VERBOSE
                                printf("%streat controlled as uncontrolled: %s -> %s\n", DEPTH_PREFIX.c_str(), globalState->hash.c_str(), globalTransition->to->hash.c_str());
                            #endif
                            if (!config.probability) {
                                currentState->uncontrolledTransitionsLeftToProcess.emplace(globalTransition);
                            } else {
                                string globalTransitionName = globalTransition->joinLocalTransitionNames(',');
                                for (auto globalTransition2 : currentState->globalState->globalTransitions) {
                                    if (globalTransition2->joinLocalTransitionNames(',') == globalTransitionName) {
                                        currentState->uncontrolledTransitionsLeftToProcess.emplace(globalTransition2);
                                    }
                                }
                            }
                        }
                    } else if (generator->getActionNameFromStateInStrategy(currentState->globalState) != ";" && generator->getActionNameFromStateInStrategy(currentState->globalState) != "") {
                        if (globalTransition->joinLocalTransitionNames().find(generator->getActionNameFromStateInStrategy(currentState->globalState)) != string::npos) {
                            // cout << "Should use: " << globalTransition->joinLocalTransitionNames() << endl;
                            currentState->controlledTransitionsLeftToProcess.emplace(globalTransition);
                        } else {
                            // cout << "Shouldn't use: " << globalTransition->joinLocalTransitionNames() << endl;
                        }
                        // might need to fix it when there's multiple agents in a coalition
                    }
                } else {
                    currentState->uncontrolledTransitionsLeftToProcess.emplace(globalTransition);
                }
            }

            // prepare current state according to the presence of controlled and uncontrolled transitions
            if (!currentState->controlledTransitionsLeftToProcess.empty()) {
                currentState->controlled = true;
                currentState->hasValidControlledTransition = false;
            }
            if (!currentState->uncontrolledTransitionsLeftToProcess.empty()) {
                currentState->uncontrolled = true;
                currentState->hasValidUncontrolledTransition = true;
                // cout << "Has uncontrolled transition 3:" << currentState->hasValidUncontrolledTransition << endl;
            }

            // if (currentState->controlled && currentState->uncontrolled) {
            //     cout << "Mixed transitions" << endl;
            // } else if (currentState->controlled) {
            //     cout << "Controlled transitions" << endl;
            // } else if (currentState->uncontrolled) {
            //     cout << "Uncontrolled transitions" << endl;
            // }

            testForAndFixBadAgents(currentState);
            currentState->processed = true;
        }

        // got back to the state after some other states got taken off the stack and it breaks the state verification
        // cout << currentState->controlled << " " << currentState->uncontrolled << " " << (currentState->controlledTransitionsLeftToProcess.empty()) << " " << currentState->hasValidControlledTransition << " " << currentState->uncontrolled << " " << currentState->hasValidUncontrolledTransition << endl;
        if ((currentState->controlled && currentState->hasValidControlledTransition == true) || (currentState->uncontrolled && currentState->uncontrolledTransitionsLeftToProcess.empty() && currentState->hasValidUncontrolledTransition == true)) { // there is a controlled action that is ok or all uncontrolled actions are ok 
            // cout << "here 2" << endl;
            if (currentState->fromState != nullptr) {
                // go on
                if (currentState->isControlledByCoalition && (currentState->controlled && currentState->hasValidControlledTransition == true)) { // if controlled then mark the transition as good
                    currentState->fromState->hasValidControlledTransition = true;
                    this->addHistoryStateStatus(currentState->fromState->globalState, currentState->fromState->globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_OK);
                    currentState->globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;
                    // temporary probability verification fix
                    addHistoryAnswerProbability(currentState->globalState, currentState->probabilityTrue, currentState->probabilityFalse, currentState->globalState->probability);
                } else if (!currentState->isControlledByCoalition && (currentState->uncontrolled && currentState->uncontrolledTransitionsLeftToProcess.empty() && currentState->hasValidUncontrolledTransition == true)) {
                    currentState->fromState->hasValidUncontrolledTransition = true;
                    this->addHistoryStateStatus(currentState->fromState->globalState, currentState->fromState->globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_OK);
                    currentState->globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;
                    // temporary probability verification fix
                    addHistoryAnswerProbability(currentState->globalState, currentState->probabilityTrue, currentState->probabilityFalse, currentState->globalState->probability);
                }
                statesToProcess.pop();
                continue;
            } else { // got back to root state
                // cout << "here 3" << endl;
                verificationResult.verificationResult = true;
                this->addHistoryStateStatus(currentState->globalState, currentState->globalState->verificationStatus, GlobalStateVerificationStatus::VERIFIED_OK);
                currentState->globalState->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;
                statesToProcess.pop();
                continue;
            }
        } else if ((currentState->controlled && !currentState->uncontrolled && currentState->controlledTransitionsLeftToProcess.empty() && currentState->hasValidControlledTransition == false) || (currentState->uncontrolled && currentState->hasValidUncontrolledTransition == false)) {
            // cout << "here 1" << endl;
            if (currentState->fromState != nullptr) {
                if (!currentState->isControlledByCoalition) { // if uncontrolled then mark the transition as not good
                    // cout << "Has uncontrolled transition 4:" << currentState->hasValidUncontrolledTransition << endl;
                    currentState->fromState->hasValidUncontrolledTransition = false;
                }
                statesToProcess.pop();
                revertToLastDecision();
                continue;
            } else { // got back to root state
                verificationResult.verificationResult = false;
                statesToProcess.pop();
                continue;
            }
        }

        // push another state onto the queue, go back and continue or return a value
        if (!currentState->controlledTransitionsLeftToProcess.empty()) { // process controlled states, one by one
            StateVerificationInfo newState;
            newState.globalState = currentState->controlledTransitionsLeftToProcess.front()->to;
            newState.fromState = currentState;
            newState.depth = currentState->depth + 1;
            newState.isControlledByCoalition = true;
            newState.probabilityTrue = currentState->probabilityTrue;
            newState.probabilityFalse = currentState->probabilityFalse;
            if (epistemicClass && fixedGlobalTransition == nullptr) {
                epistemicClass->fixedCoalitionTransition = currentState->controlledTransitionsLeftToProcess.front(); 
                this->addHistoryDecision(currentState->globalState, currentState->controlledTransitionsLeftToProcess.front());
            }
            this->addHistoryContext(currentState->globalState, currentState->depth, currentState->controlledTransitionsLeftToProcess.front(), true);
            if (config.probability) {
                this->addHistoryProbability(currentState->controlledTransitionsLeftToProcess.front(), currentState->probabilityTrue, currentState->probabilityFalse);
            }
            statesToProcess.emplace(newState);
            currentState->controlledTransitionsLeftToProcess.pop();
        } else if (!currentState->uncontrolledTransitionsLeftToProcess.empty()) { // process uncontrolled states, one by one
            StateVerificationInfo newState;
            newState.globalState = currentState->uncontrolledTransitionsLeftToProcess.front()->to;
            newState.fromState = currentState;
            newState.depth = currentState->depth + 1;
            newState.probabilityTrue = currentState->probabilityTrue;
            newState.probabilityFalse = currentState->probabilityFalse;
            this->addHistoryContext(currentState->globalState, currentState->depth, currentState->uncontrolledTransitionsLeftToProcess.front(), false);
            if (config.probability) {
                this->addHistoryProbability(currentState->uncontrolledTransitionsLeftToProcess.front(), currentState->probabilityTrue, currentState->probabilityFalse);
            }
            statesToProcess.emplace(newState);
            currentState->uncontrolledTransitionsLeftToProcess.pop();
        }
    }

    // while (!history.empty()) {
    //     cout << history.top().toString() << endl;
    //     history.pop();
    // }

    return verificationResult;
}

bool VerificationIterative::testForAndFixBadAgents(StateVerificationInfo* stateVerificationInfo) {
    bool hasMergedTransitionsIntoUncontrolled = false;
    if (stateVerificationInfo->controlled && stateVerificationInfo->uncontrolled) {
        set<Agent*> agents = generator->getFormula()->coalition;
        set<Agent*> brokenAgents;
        set<Agent*> potentiallyBrokenAgents;

        queue<GlobalTransition*> uncontrolledGlobalTransitions(stateVerificationInfo->uncontrolledTransitionsLeftToProcess);
        queue<GlobalTransition*> controlledGlobalTransitions(stateVerificationInfo->controlledTransitionsLeftToProcess);

        // find if there are uncontrolled actions with only agents that are not in coalition, if yes, then they can block the controlled actions if they are a participants
        while (!uncontrolledGlobalTransitions.empty()) {
            GlobalTransition* globalTransition = uncontrolledGlobalTransitions.front();
            uncontrolledGlobalTransitions.pop();
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
            stateVerificationInfo->controlledTransitionsLeftToProcess = queue<GlobalTransition*>();
            // mark bad controlled global transition as uncontrolled
            while (!controlledGlobalTransitions.empty()) {
                GlobalTransition* globalTransition = controlledGlobalTransitions.front();
                controlledGlobalTransitions.pop();
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
                } else {
                    stateVerificationInfo->controlledTransitionsLeftToProcess.emplace(globalTransition);
                }
            }
            while (!transitionsToBeMoved.empty()) {
                GlobalTransition* tr = transitionsToBeMoved.top();
                transitionsToBeMoved.pop();
                stateVerificationInfo->uncontrolledTransitionsLeftToProcess.emplace(tr);
            }
            hasMergedTransitionsIntoUncontrolled = true;
        }
    }
    return hasMergedTransitionsIntoUncontrolled;
}