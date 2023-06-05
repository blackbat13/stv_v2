/**
 * @file GlobalModelGenerator.hpp
 * @brief Generator of a global model.
 * Class for initializing and generating a global model.
 */

#ifndef SELENE_GLOBAL_MODEL_GENERATOR
#define SELENE_GLOBAL_MODEL_GENERATOR

#include "Constants.hpp"
#include "GlobalState.hpp"
#include "GlobalTransition.hpp"
#include "Agent.hpp"

using namespace std;

/// @brief Stores the local models, formula and a global model.
class GlobalModelGenerator {
public:
    GlobalModelGenerator();
    ~GlobalModelGenerator();
    GlobalState* initModel(LocalModels* localModels, Formula* formula);
    void expandState(GlobalState* state);
    void expandAllStates();
    GlobalModel* getCurrentGlobalModel();
    Formula* getFormula();
    
protected:
    /// @brief LocalModels used in initModel.
    LocalModels* localModels;
    /// @brief Formula used in initModel.
    Formula* formula;
    /// @brief GlobalModel created in initModel.
    GlobalModel* globalModel;
    GlobalState* generateInitState();
    GlobalState* generateStateFromLocalStates(set<LocalState*>* localStates, set<LocalTransition*>* viaLocalTransitions, GlobalState* prevGlobalState);
    void generateGlobalTransitions(GlobalState* fromGlobalState, set<LocalTransition*> localTransitions, map<Agent*, vector<LocalTransition*>> transitionsByAgent);
    string computeEpistemicClassHash(set<LocalState*>* localStates, Agent* agent);
    string computeGlobalStateHash(set<LocalState*>* localStates);
    EpistemicClass* findOrCreateEpistemicClass(set<LocalState*>* localStates, Agent* agent);
    GlobalState* findGlobalStateInEpistemicClass(set<LocalState*>* localStates, EpistemicClass* epistemicClass);
};

#endif // SELENE_GLOBAL_MODEL_GENERATOR
