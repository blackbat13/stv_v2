#include "../../Types.hpp"
#include "Utils.cpp"

Agent* createEA1(LocalModels* localModels) {
    Agent* EA1 = new Agent();
    EA1->id = localModels->agents.size();
    EA1->name = "EA1";
    
    // States
    map<string, LocalState*> localStates;
    localStates["prepare"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "prepare", {  }, EA1 });
    localStates["start"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "start", {  }, EA1 });
    localStates["generate"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "generate", {  }, EA1 });
    localStates["voting"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "voting", {  }, EA1 });
    localStates["finish"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "finish", {  }, EA1 });
    localStates["check"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "check", {  }, EA1 });
    EA1->initState = localStates["prepare"];
    copyLocalStatesToAgent(&localStates, EA1);
    
    // Transitions
    VarAssignment* fakeVarAssignment = new VarAssignment({ nullptr, VarAssignmentType::FromValue, nullptr, -1 });
    Condition* fakeCondition = new Condition({ nullptr, ConditionOperator::Equals, -1 });
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "is_ready", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["prepare"], // from
        localStates["start"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "gen_trackers1", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("EA1_tracker1", 1, "", localModels), mkVarAssignment("EA1_tracker2", 2, "", localModels) }, // varAssignments
        EA1, // agent
        localStates["start"], // from
        localStates["generate"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "gen_trackers2", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("EA1_tracker1", 2, "", localModels), mkVarAssignment("EA1_tracker2", 1, "", localModels) }, // varAssignments
        EA1, // agent
        localStates["start"], // from
        localStates["generate"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "start_voting", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["generate"], // from
        localStates["voting"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_vote_Voter1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker1", ConditionOperator::Equals, 1, localModels), fakeCondition }, // conditions
        { mkVarAssignment("EA1_tracker1_vote", -1, "Voter1_vote", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["voting"], // from
        localStates["voting"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_vote_Voter1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker2", ConditionOperator::Equals, 1, localModels), fakeCondition }, // conditions
        { mkVarAssignment("EA1_tracker2_vote", -1, "Voter1_vote", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["voting"], // from
        localStates["voting"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_vote_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker1", ConditionOperator::Equals, 2, localModels), fakeCondition }, // conditions
        { mkVarAssignment("EA1_tracker1_vote", -1, "VoterC1_vote", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["voting"], // from
        localStates["voting"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_fvote_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker1", ConditionOperator::Equals, 2, localModels), fakeCondition }, // conditions
        { mkVarAssignment("EA1_tracker1_vote", -1, "VoterC1_vote", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["voting"], // from
        localStates["voting"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_vote_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker2", ConditionOperator::Equals, 2, localModels), fakeCondition }, // conditions
        { mkVarAssignment("EA1_tracker2_vote", -1, "VoterC1_vote", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["voting"], // from
        localStates["voting"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_fvote_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker2", ConditionOperator::Equals, 2, localModels), fakeCondition }, // conditions
        { mkVarAssignment("EA1_tracker2_vote", -1, "VoterC1_vote", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["voting"], // from
        localStates["voting"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "finish_voting", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["voting"], // from
        localStates["finish"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_tracker_Voter1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker1", ConditionOperator::Equals, 1, localModels), fakeCondition }, // conditions
        { mkVarAssignment("Voter1_tracker", 1, "", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["finish"], // from
        localStates["finish"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_tracker_Voter1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker2", ConditionOperator::Equals, 1, localModels), fakeCondition }, // conditions
        { mkVarAssignment("Voter1_tracker", 2, "", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["finish"], // from
        localStates["finish"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_tracker_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker1", ConditionOperator::Equals, 2, localModels), fakeCondition }, // conditions
        { mkVarAssignment("VoterC1_tracker", 1, "", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["finish"], // from
        localStates["finish"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "send_tracker_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        { mkCondition("EA1_tracker2", ConditionOperator::Equals, 2, localModels), fakeCondition }, // conditions
        { mkVarAssignment("VoterC1_tracker", 2, "", localModels), fakeVarAssignment }, // varAssignments
        EA1, // agent
        localStates["finish"], // from
        localStates["finish"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "finish_sending_trackers", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["finish"], // from
        localStates["check"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "check_tracker1_Voter1", // name
        "check_tracker_Voter1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["check"], // from
        localStates["check"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "check_tracker1_VoterC1", // name
        "check_tracker_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["check"], // from
        localStates["check"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "check_tracker1_Coercer1", // name
        "check_tracker_Coercer1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["check"], // from
        localStates["check"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "check_tracker2_Voter1", // name
        "check_tracker_Voter1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["check"], // from
        localStates["check"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "check_tracker2_VoterC1", // name
        "check_tracker_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["check"], // from
        localStates["check"], // to
    }), EA1);
    addTransition(new LocalTransition({
        static_cast<int>(EA1->localTransitions.size()), // id
        "check_tracker2_Coercer1", // name
        "check_tracker_Coercer1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        EA1, // agent
        localStates["check"], // from
        localStates["check"], // to
    }), EA1);
    
    // Remove fake var assignments and conditions
    for (const auto& t : EA1->localTransitions) {
        t->varAsssignments.erase(fakeVarAssignment);
        t->conditions.erase(fakeCondition);
    }
    
    return EA1;
}
