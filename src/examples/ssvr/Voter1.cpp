#include "../../Types.hpp"
#include "Utils.cpp"

Agent* createVoter1(LocalModels* localModels) {
    Agent* Voter1 = new Agent();
    Voter1->id = localModels->agents.size();
    Voter1->name = "Voter1";
    
    // States
    map<string, LocalState*> localStates;
    localStates["start"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "start",  {  }, Voter1 });
    localStates["voting"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "voting",  {  }, Voter1 });
    localStates["vote"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "vote",  {  }, Voter1 });
    localStates["send"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "send",  {  }, Voter1 });
    localStates["finish"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "finish",  {  }, Voter1 });
    localStates["tracker"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "tracker",  {  }, Voter1 });
    localStates["check"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "check",  {  }, Voter1 });
    localStates["end"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "end",  {  }, Voter1 });
    Voter1->initState = localStates["start"];
    copyLocalStatesToAgent(&localStates, Voter1);
    
    // Transitions
    VarAssignment* fakeVarAssignment = new VarAssignment({ nullptr, VarAssignmentType::FromValue, nullptr, -1 });
    Condition* fakeCondition = new Condition({ nullptr, ConditionOperator::Equals, -1 });
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "start_voting", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Voter1, // agent
        localStates["start"], // from
        localStates["voting"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "vote1", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Voter1_vote", 1, "", localModels), fakeVarAssignment }, // varAssignments
        Voter1, // agent
        localStates["voting"], // from
        localStates["vote"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "vote2", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Voter1_vote", 2, "", localModels), fakeVarAssignment }, // varAssignments
        Voter1, // agent
        localStates["voting"], // from
        localStates["vote"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "vote3", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Voter1_vote", 3, "", localModels), fakeVarAssignment }, // varAssignments
        Voter1, // agent
        localStates["voting"], // from
        localStates["vote"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "send_vote_Voter1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Voter1, // agent
        localStates["vote"], // from
        localStates["send"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "finish_voting", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Voter1, // agent
        localStates["send"], // from
        localStates["finish"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "send_tracker_Voter1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Voter1, // agent
        localStates["finish"], // from
        localStates["tracker"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "finish_sending_trackers", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Voter1, // agent
        localStates["tracker"], // from
        localStates["check"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "check_tracker1_Voter1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Voter1_tracker1", -1, "EA1_tracker1_vote", localModels), fakeVarAssignment }, // varAssignments
        Voter1, // agent
        localStates["check"], // from
        localStates["end"], // to
    }), Voter1);
    addTransition(new LocalTransition({
        static_cast<int>(Voter1->localTransitions.size()), // id
        "check_tracker2_Voter1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Voter1_tracker2", -1, "EA1_tracker2_vote", localModels), fakeVarAssignment }, // varAssignments
        Voter1, // agent
        localStates["check"], // from
        localStates["end"], // to
    }), Voter1);
    
    // Remove fake var assignments and conditions
    for (const auto& t : Voter1->localTransitions) {
        t->varAsssignments.erase(fakeVarAssignment);
        t->conditions.erase(fakeCondition);
    }
    
    return Voter1;
}
