#include "../../Types.hpp"
#include "Utils.cpp"

Agent* createCoercer1(LocalModels* localModels) {
    Agent* Coercer1 = new Agent();
    Coercer1->id = localModels->agents.size();
    Coercer1->name = "Coercer1";
    
    // States
    map<string, LocalState*> localStates;
    localStates["coerce"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "coerce", {  }, Coercer1 });
    localStates["voting"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "voting", {  }, Coercer1 });
    localStates["finish"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "finish", {  }, Coercer1 });
    localStates["trackers_sent"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "trackers_sent", {  }, Coercer1 });
    localStates["check"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "check", {  }, Coercer1 });
    localStates["interact"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "interact", {  }, Coercer1 });
    localStates["end"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "end", {  }, Coercer1 });
    Coercer1->initState = localStates["coerce"];
    copyLocalStatesToAgent(&localStates, Coercer1);
    
    // Transitions
    VarAssignment* fakeVarAssignment = new VarAssignment({ nullptr, VarAssignmentType::FromValue, nullptr, -1 });
    Condition* fakeCondition = new Condition({ nullptr, ConditionOperator::Equals, -1 });
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "coerce1_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_VoterC1_required", 1, "", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["coerce"], // from
        localStates["coerce"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "coerce2_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_VoterC1_required", 2, "", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["coerce"], // from
        localStates["coerce"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "coerce3_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_VoterC1_required", 3, "", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["coerce"], // from
        localStates["coerce"], // to
    }), Coercer1);
    
    
    
    
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "start_voting", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Coercer1, // agent
        localStates["coerce"], // from
        localStates["voting"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "VoterC1_vote", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_VoterC1_vote", -1, "VoterC1_vote", localModels), mkVarAssignment("Coercer1_VoterC1_revote", -1, "VoterC1_revote", localModels) }, // varAssignments
        Coercer1, // agent
        localStates["voting"], // from
        localStates["voting"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "finish_voting", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Coercer1, // agent
        localStates["voting"], // from
        localStates["finish"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "finish_sending_trackers", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Coercer1, // agent
        localStates["finish"], // from
        localStates["trackers_sent"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "give1_VoterC1", // name
        "interact_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_VoterC1_tracker", 1, "", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["trackers_sent"], // from
        localStates["trackers_sent"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "give2_VoterC1", // name
        "interact_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_VoterC1_tracker", 2, "", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["trackers_sent"], // from
        localStates["trackers_sent"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "not_give_VoterC1", // name
        "interact_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_VoterC1_tracker", 0, "", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["trackers_sent"], // from
        localStates["trackers_sent"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "to_check", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Coercer1, // agent
        localStates["trackers_sent"], // from
        localStates["check"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "check_tracker1_Coercer1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_tracker1", -1, "EA1_tracker1_vote", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["check"], // from
        localStates["check"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "check_tracker2_Coercer1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_tracker2", -1, "EA1_tracker2_vote", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["check"], // from
        localStates["check"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "to_interact", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Coercer1, // agent
        localStates["check"], // from
        localStates["interact"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "punish_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Coercer1, // agent
        localStates["interact"], // from
        localStates["interact"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "not_punish_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        Coercer1, // agent
        localStates["interact"], // from
        localStates["interact"], // to
    }), Coercer1);
    addTransition(new LocalTransition({
        static_cast<int>(Coercer1->localTransitions.size()), // id
        "finish", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("Coercer1_finish", 1, "", localModels), fakeVarAssignment }, // varAssignments
        Coercer1, // agent
        localStates["interact"], // from
        localStates["end"], // to
    }), Coercer1);
    
    // Remove fake var assignments and conditions
    for (const auto& t : Coercer1->localTransitions) {
        t->varAsssignments.erase(fakeVarAssignment);
        t->conditions.erase(fakeCondition);
    }
    
    return Coercer1;
}
