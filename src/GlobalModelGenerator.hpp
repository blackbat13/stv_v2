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
    shared_ptr<GlobalState> initModel(shared_ptr<LocalModels> localModels, shared_ptr<Formula> formula);
    void expandState(shared_ptr<GlobalState> state);
    vector<shared_ptr<GlobalState>> expandStateAndReturn(shared_ptr<GlobalState> state);
    void expandAllStates();
    shared_ptr<GlobalModel> getCurrentGlobalModel();
    shared_ptr<Formula> getFormula();

    /// @brief auxiliary variable mapping Agent pointer to its index (replace size_t with  if needed later)
    map<shared_ptr<Agent>, size_t> agentIndex;

protected:
    /// @brief LocalModels used in initModel.
    shared_ptr<LocalModels> localModels;
    /// @brief Formula used in initModel.
    shared_ptr<Formula> formula;
    /// @brief GlobalModel created in initModel.
    shared_ptr<GlobalModel> globalModel;
    shared_ptr<GlobalState> generateInitState();
    shared_ptr<GlobalState> generateStateFromLocalStates(shared_ptr<vector<shared_ptr<LocalState>>> localStates, shared_ptr<set<shared_ptr<LocalTransition>>> viaLocalTransitions, shared_ptr<GlobalState> prevGlobalState);
    void generateGlobalTransitions(shared_ptr<GlobalState> fromGlobalState, set<shared_ptr<LocalTransition>> localTransitions, map<shared_ptr<Agent>, vector<shared_ptr<LocalTransition>>> transitionsByAgent);
    string computeEpistemicClassHash(shared_ptr<vector<shared_ptr<LocalState>>> localStates, shared_ptr<Agent> agent);
    string computeGlobalStateHash(shared_ptr<vector<shared_ptr<LocalState>>> localStates);
    shared_ptr<EpistemicClass> findOrCreateEpistemicClass(shared_ptr<vector<shared_ptr<LocalState>>> localStates, shared_ptr<Agent> agent);
    shared_ptr<GlobalState> findGlobalStateInEpistemicClass(shared_ptr<vector<shared_ptr<LocalState>>> localStates, shared_ptr<EpistemicClass> epistemicClass);
};

#endif // SELENE_GLOBAL_MODEL_GENERATOR
