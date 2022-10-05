#ifndef SELENE_SELENE_FORMULA
#define SELENE_SELENE_FORMULA

#include "Types.hpp"





class SeleneFormula {
public:
    SeleneFormula();
    ~SeleneFormula();
    virtual bool verifyLocalStates(set<LocalState*>* localStates) = 0;
    LocalState* getLocalStateForAgent(string agentName, set<LocalState*>* localStates);
    int getLocalStateVar(string varName, LocalState* localState);
    inline bool implication(bool left, bool right);
};





class SeleneFormula1 : public SeleneFormula {
public:
    SeleneFormula1();
    ~SeleneFormula1();
    bool verifyLocalStates(set<LocalState*>* localStates);
protected:
};





#endif // SELENE_SELENE_FORMULA
