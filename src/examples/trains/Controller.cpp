#include "../../Types.hpp"
#include "Utils.cpp"

Agent* createController(LocalModels* localModels) {
    Agent* Controller = new Agent(localModels->agents.size(), "Controller");
    
    // States
    map<string, LocalState*> localStates;
    localStates["green"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "green", {  }, {  }, Controller });
    localStates["red"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "red", {  }, {  }, Controller });
    Controller->initState = localStates["green"];
    copyLocalStatesToAgent(&localStates, Controller);
    
    // Transitions
    VarAssignment* fakeVarAssignment = new VarAssignment({ nullptr, VarAssignmentType::FromValue, nullptr, -1 });
    Condition* fakeCondition = new Condition({ nullptr, ConditionOperator::Equals, -1 });
    addTransition(new LocalTransition({
        static_cast<int>(Controller->localTransitions.size()), // id
        "in_Train1", // name
        "in_Train1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Controller, // agent
        localStates["green"], // from
        localStates["red"], // to
    }), Controller);
    addTransition(new LocalTransition({
        static_cast<int>(Controller->localTransitions.size()), // id
        "out_Train1", // name
        "out_Train1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Controller, // agent
        localStates["red"], // from
        localStates["green"], // to
    }), Controller);
    addTransition(new LocalTransition({
        static_cast<int>(Controller->localTransitions.size()), // id
        "in_Train2", // name
        "in_Train2", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Controller, // agent
        localStates["green"], // from
        localStates["red"], // to
    }), Controller);
    addTransition(new LocalTransition({
        static_cast<int>(Controller->localTransitions.size()), // id
        "out_Train2", // name
        "out_Train2", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Controller, // agent
        localStates["red"], // from
        localStates["green"], // to
    }), Controller);
    
    // Remove fake var assignments and conditions
    for (const auto& t : Controller->localTransitions) {
        t->varAsssignments.erase(fakeVarAssignment);
        t->conditions.erase(fakeCondition);
    }
    
    return Controller;
}
