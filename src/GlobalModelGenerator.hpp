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
    vector<GlobalState*> expandStateAndReturn(GlobalState* state);
    void expandAllStates();
    GlobalModel* getCurrentGlobalModel();
    Formula* getFormula();
    int getFormulaSize();
    set<GlobalState*>* findOrCreateEpistemicClassForKnowledge(vector<LocalState*>* localStates, GlobalState* globalState, Agent* agent);
    Agent* getAgentInstanceByName(string agentName);
    void markFormulaAsIncorrect();
    bool getFormulaCorectness();

    /// @brief auxiliary variable mapping Agent pointer to its index (replace size_t with  if needed later)
    map<Agent*,size_t> agentIndex;

protected:
    /// @brief LocalModels used in initModel.
    LocalModels* localModels;
    /// @brief Formula used in initModel.
    Formula* formula;
    /// @brief GlobalModel created in initModel.
    GlobalModel* globalModel;
    /// @brief Flag holding info if model is actually correct.
    bool correctModel;
    GlobalState* generateInitState();
    GlobalState* generateStateFromLocalStates(vector<LocalState*>* localStates, set<LocalTransition*>* viaLocalTransitions, GlobalState* prevGlobalState);
    void generateGlobalTransitions(GlobalState* fromGlobalState, set<LocalTransition*> localTransitions, map<Agent*, vector<LocalTransition*>> transitionsByAgent);
    string computeEpistemicClassHash(vector<LocalState*>* localStates, Agent* agent);
    string computeGlobalStateHash(vector<LocalState*>* localStates);
    EpistemicClass* findOrCreateEpistemicClass(vector<LocalState*>* localStates, Agent* agent);
    GlobalState* findGlobalStateInEpistemicClass(vector<LocalState*>* localStates, EpistemicClass* epistemicClass);
};

#endif // SELENE_GLOBAL_MODEL_GENERATOR
