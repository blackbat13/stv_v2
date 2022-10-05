#include "../../Types.hpp"
#include "Utils.cpp"

Agent* createTrain1(LocalModels* localModels) {
    Agent* Train1 = new Agent(localModels->agents.size(), "Train1");
    
    // States
    map<string, LocalState*> localStates;
    auto Train1_pos = getVar("Train1_pos", localModels);
    localStates["idle_1"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "idle", { { Train1_pos, 1 } }, {  }, Train1 });
    localStates["idle_2"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "idle", { { Train1_pos, 2 } }, {  }, Train1 });
    localStates["idle_3"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "idle", { { Train1_pos, 3 } }, {  }, Train1 });
    Train1->initState = localStates["idle_1"];
    copyLocalStatesToAgent(&localStates, Train1);
    
    // Transitions
    VarAssignment* fakeVarAssignment = new VarAssignment({ nullptr, VarAssignmentType::FromValue, nullptr, -1 });
    Condition* fakeCondition = new Condition({ nullptr, ConditionOperator::Equals, -1 });
    addTransition(new LocalTransition({
        static_cast<int>(Train1->localTransitions.size()), // id
        "in_Train1", // name
        "in_Train1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Train1_pos", 2, "", localModels), fakeVarAssignment }, // varAssignments
        Train1, // agent
        localStates["idle_1"], // from
        localStates["idle_2"], // to
    }), Train1);
    addTransition(new LocalTransition({
        static_cast<int>(Train1->localTransitions.size()), // id
        "out_Train1", // name
        "out_Train1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Train1_pos", 3, "", localModels), fakeVarAssignment }, // varAssignments
        Train1, // agent
        localStates["idle_2"], // from
        localStates["idle_3"], // to
    }), Train1);
    // addTransition(new LocalTransition({
    //     static_cast<int>(Train1->localTransitions.size()), // id
    //     "return", // name
    //     "return", // localName
    //     false, // isShared
    //     0, // sharedCount
    //     {  }, // conditions
    //     { mkVarAssignment("Train1_pos", 1, "", localModels), fakeVarAssignment }, // varAssignments
    //     Train1, // agent
    //     localStates["idle_2"], // from
    //     localStates["idle_1"], // to
    // }), Train1);
    addTransition(new LocalTransition({
        static_cast<int>(Train1->localTransitions.size()), // id
        "return", // name
        "return", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Train1_pos", 1, "", localModels), fakeVarAssignment }, // varAssignments
        Train1, // agent
        localStates["idle_3"], // from
        localStates["idle_1"], // to
    }), Train1);
    
    // Remove fake var assignments and conditions
    for (const auto& t : Train1->localTransitions) {
        t->varAsssignments.erase(fakeVarAssignment);
        t->conditions.erase(fakeCondition);
    }
    
    return Train1;
}
