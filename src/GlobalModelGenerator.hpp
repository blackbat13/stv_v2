#ifndef SELENE_GLOBAL_MODEL_GENERATOR
#define SELENE_GLOBAL_MODEL_GENERATOR

#include "Constants.hpp"
#include "Types.hpp"

using namespace std;

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
    LocalModels* localModels;
    Formula* formula;
    GlobalModel* globalModel;
    GlobalState* generateInitState();
    GlobalState* generateStateFromLocalStates(set<LocalState*>* localStates, set<LocalTransition*>* viaLocalTransitions, GlobalState* prevGlobalState);
    void generateGlobalTransitions(GlobalState* fromGlobalState, set<LocalTransition*> localTransitions, map<Agent*, vector<LocalTransition*>> transitionsByAgent);
    bool checkLocalTransitionConditions(LocalTransition* localTransition, GlobalState* globalState);
    string computeEpistemicClassHash(set<LocalState*>* localStates, Agent* agent);
    string computeGlobalStateHash(set<LocalState*>* localStates);
    EpistemicClass* findOrCreateEpistemicClass(set<LocalState*>* localStates, Agent* agent);
    GlobalState* findGlobalStateInEpistemicClass(set<LocalState*>* localStates, EpistemicClass* epistemicClass);
};

#endif // SELENE_GLOBAL_MODEL_GENERATOR
