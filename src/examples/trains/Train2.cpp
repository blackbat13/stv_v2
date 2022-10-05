#include "../../Types.hpp"
#include "Utils.cpp"

Agent* createTrain2(LocalModels* localModels) {
    Agent* Train2 = new Agent(localModels->agents.size(), "Train2");
    
    // States
    map<string, LocalState*> localStates;
    auto Train2_pos = getVar("Train2_pos", localModels);
    localStates["idle_1"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "idle", { { Train2_pos, 1 } }, {  }, Train2 });
    localStates["idle_2"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "idle", { { Train2_pos, 2 } }, {  }, Train2 });
    localStates["idle_3"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "idle", { { Train2_pos, 3 } }, {  }, Train2 });
    Train2->initState = localStates["idle_1"];
    copyLocalStatesToAgent(&localStates, Train2);
    
    // Transitions
    VarAssignment* fakeVarAssignment = new VarAssignment({ nullptr, VarAssignmentType::FromValue, nullptr, -1 });
    Condition* fakeCondition = new Condition({ nullptr, ConditionOperator::Equals, -1 });
    addTransition(new LocalTransition({
        static_cast<int>(Train2->localTransitions.size()), // id
        "in_Train2", // name
        "in_Train2", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Train2_pos", 2, "", localModels), fakeVarAssignment }, // varAssignments
        Train2, // agent
        localStates["idle_1"], // from
        localStates["idle_2"], // to
    }), Train2);
    addTransition(new LocalTransition({
        static_cast<int>(Train2->localTransitions.size()), // id
        "out_Train2", // name
        "out_Train2", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Train2_pos", 3, "", localModels), fakeVarAssignment }, // varAssignments
        Train2, // agent
        localStates["idle_2"], // from
        localStates["idle_3"], // to
    }), Train2);
    // addTransition(new LocalTransition({
    //     static_cast<int>(Train2->localTransitions.size()), // id
    //     "return", // name
    //     "return", // localName
    //     false, // isShared
    //     0, // sharedCount
    //     {  }, // conditions
    //     { mkVarAssignment("Train2_pos", 1, "", localModels), fakeVarAssignment }, // varAssignments
    //     Train2, // agent
    //     localStates["idle_2"], // from
    //     localStates["idle_1"], // to
    // }), Train2);
    addTransition(new LocalTransition({
        static_cast<int>(Train2->localTransitions.size()), // id
        "return", // name
        "return", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Train2_pos", 1, "", localModels), fakeVarAssignment }, // varAssignments
        Train2, // agent
        localStates["idle_3"], // from
        localStates["idle_1"], // to
    }), Train2);
    
    // Remove fake var assignments and conditions
    for (const auto& t : Train2->localTransitions) {
        t->varAsssignments.erase(fakeVarAssignment);
        t->conditions.erase(fakeCondition);
    }
    
    return Train2;
}
