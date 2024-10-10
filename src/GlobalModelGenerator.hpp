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

#include <unordered_set>
#include <unordered_map>
#include <sstream>

using namespace std;

/// @brief Stores the local models, formula and a global model.
class GlobalModelGenerator {
public:
    GlobalModelGenerator();
    ~GlobalModelGenerator();
    GlobalState* initModel(LocalModels* localModels, Formula* formula);
    void expandState(GlobalState* state);
    vector<GlobalState*> expandStateAndReturn(GlobalState* state, bool returnAnyway = false);
    void expandAllStates();
    void expandAndReduceAllStates();
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
    struct GlobalStateTupleHash {
        size_t operator()(const tuple<GlobalState*, int>& t) const {
            auto gsHash = hash<string>()(get<0>(t)->hash);
            auto deHash = hash<int>()(get<1>(t));
            return gsHash ^ deHash;
        }
    };
    /// @brief LocalModels used in initModel.
    LocalModels* localModels;
    /// @brief Formula used in initModel.
    Formula* formula;
    /// @brief GlobalModel created in initModel.
    GlobalModel* globalModel;
    /// @brief Flag holding info if model is actually correct.
    bool correctModel;
    /// @brief Lookup map containing global states and the depths that the global state is contained in. Used for reductions.
    unordered_map<GlobalState*, unordered_set<int>> candidateStateDepths;
    /// @brief Candidate states to be used in reductions. Used for reductions.
    stack<GlobalState*> globalModelCandidates;
    /// @brief Saved depths of states added to statesToExpand. Used for reductions.
    stack<int> stateDepths;
    /// @brief States that were added to a stack of states. Used for reductions.
    unordered_set<GlobalState*> addedStates;
    GlobalState* generateInitState();
    GlobalState* generateStateFromLocalStates(vector<LocalState*>* localStates, set<LocalTransition*>* viaLocalTransitions, GlobalState* prevGlobalState);
    void generateGlobalTransitions(GlobalState* fromGlobalState, set<LocalTransition*> localTransitions, map<Agent*, vector<LocalTransition*>> transitionsByAgent);
    string computeEpistemicClassHash(vector<LocalState*>* localStates, Agent* agent);
    string computeGlobalStateHash(vector<LocalState*>* localStates);
    EpistemicClass* findOrCreateEpistemicClass(vector<LocalState*>* localStates, Agent* agent);
    GlobalState* findGlobalStateInEpistemicClass(vector<LocalState*>* localStates, EpistemicClass* epistemicClass);
};

#endif // SELENE_GLOBAL_MODEL_GENERATOR
