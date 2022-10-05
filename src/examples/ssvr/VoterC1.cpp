#include "../../Types.hpp"
#include "Utils.cpp"

Agent* createVoterC1(LocalModels* localModels) {
    Agent* VoterC1 = new Agent();
    VoterC1->id = localModels->agents.size();
    VoterC1->name = "VoterC1";
    
    // States
    map<string, LocalState*> localStates;
    localStates["start"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "start", {  }, VoterC1 });
    localStates["coerced"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "coerced", {  }, VoterC1 });
    localStates["prepared"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "prepared", {  }, VoterC1 });
    localStates["ready"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "ready", {  }, VoterC1 });
    localStates["voting"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "voting", {  }, VoterC1 });
    localStates["vote"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "vote", {  }, VoterC1 });
    localStates["send"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "send", {  }, VoterC1 });
    localStates["votingf"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "votingf", {  }, VoterC1 });
    localStates["sendf"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "sendf", {  }, VoterC1 });
    localStates["finish"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "finish", {  }, VoterC1 });
    localStates["tracker"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "tracker", {  }, VoterC1 });
    localStates["trackers_sent"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "trackers_sent", {  }, VoterC1 });
    localStates["interact"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "interact", {  }, VoterC1 });
    localStates["check"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "check", {  }, VoterC1 });
    localStates["end"] = new LocalState(LocalState{ static_cast<int>(localStates.size()), "end", {  }, VoterC1 });
    VoterC1->initState = localStates["start"];
    copyLocalStatesToAgent(&localStates, VoterC1);
    
    // Transitions
    VarAssignment* fakeVarAssignment = new VarAssignment({ nullptr, VarAssignmentType::FromValue, nullptr, -1 });
    Condition* fakeCondition = new Condition({ nullptr, ConditionOperator::Equals, -1 });
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "coerce1_VoterC1", // name
        "coerce_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_required", 1, "", localModels), fakeVarAssignment }, // varAssignments
        VoterC1, // agent
        localStates["start"], // from
        localStates["coerced"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "coerce2_VoterC1", // name
        "coerce_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_required", 2, "", localModels), fakeVarAssignment }, // varAssignments
        VoterC1, // agent
        localStates["start"], // from
        localStates["coerced"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "coerce3_VoterC1", // name
        "coerce_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_required", 3, "", localModels), fakeVarAssignment }, // varAssignments
        VoterC1, // agent
        localStates["start"], // from
        localStates["coerced"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "select_vote1", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_vote", 1, "", localModels), mkVarAssignment("VoterC1_prep_vote", 1, "", localModels) }, // varAssignments
        VoterC1, // agent
        localStates["coerced"], // from
        localStates["prepared"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "select_vote2", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_vote", 2, "", localModels), mkVarAssignment("VoterC1_prep_vote", 2, "", localModels) }, // varAssignments
        VoterC1, // agent
        localStates["coerced"], // from
        localStates["prepared"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "select_vote3", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_vote", 3, "", localModels), mkVarAssignment("VoterC1_prep_vote", 3, "", localModels) }, // varAssignments
        VoterC1, // agent
        localStates["coerced"], // from
        localStates["prepared"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "is_ready", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["prepared"], // from
        localStates["ready"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "start_voting", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["ready"], // from
        localStates["voting"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "VoterC1_vote", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["voting"], // from
        localStates["vote"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "send_vote_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["vote"], // from
        localStates["send"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "revote_vote_1", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        { mkCondition("VoterC1_revote", ConditionOperator::Equals, 1, localModels), fakeCondition }, // conditions
        { mkVarAssignment("VoterC1_vote", -1, "VoterC1_required", localModels), mkVarAssignment("VoterC1_revote", 2, "", localModels) }, // varAssignments
        VoterC1, // agent
        localStates["send"], // from
        localStates["voting"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "skip_revote_1", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        { mkCondition("VoterC1_revote", ConditionOperator::Equals, 1, localModels), fakeCondition }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["send"], // from
        localStates["votingf"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "revote_vote_2", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        { mkCondition("VoterC1_revote", ConditionOperator::Equals, 2, localModels), fakeCondition }, // conditions
        { mkVarAssignment("VoterC1_vote", -1, "VoterC1_required", localModels), mkVarAssignment("VoterC1_revote", 3, "", localModels) }, // varAssignments
        VoterC1, // agent
        localStates["send"], // from
        localStates["voting"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "skip_revote_2", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        { mkCondition("VoterC1_revote", ConditionOperator::Equals, 2, localModels), fakeCondition }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["send"], // from
        localStates["votingf"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "final_vote", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        { mkCondition("VoterC1_revote", ConditionOperator::Equals, 3, localModels), fakeCondition }, // conditions
        { mkVarAssignment("VoterC1_vote", -1, "VoterC1_prep_vote", localModels), fakeVarAssignment }, // varAssignments
        VoterC1, // agent
        localStates["send"], // from
        localStates["votingf"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "skip_final", // name
        "", // localName
        false, // isShared
        0, // sharedCount
        { mkCondition("VoterC1_revote", ConditionOperator::Equals, 3, localModels), fakeCondition }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["send"], // from
        localStates["votingf"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "send_fvote_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["votingf"], // from
        localStates["sendf"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "finish_voting", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["sendf"], // from
        localStates["finish"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "send_tracker_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["finish"], // from
        localStates["tracker"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "finish_sending_trackers", // name
        "", // localName
        true, // isShared
        3, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["tracker"], // from
        localStates["trackers_sent"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "give1_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["trackers_sent"], // from
        localStates["interact"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "give2_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["trackers_sent"], // from
        localStates["interact"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "not_give_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        {  }, // varAssignments
        VoterC1, // agent
        localStates["trackers_sent"], // from
        localStates["interact"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "punish_VoterC1", // name
        "interact_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_punish", 1, "", localModels), fakeVarAssignment }, // varAssignments
        VoterC1, // agent
        localStates["interact"], // from
        localStates["check"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "not_punish_VoterC1", // name
        "interact_VoterC1", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_punish", 0, "", localModels), fakeVarAssignment }, // varAssignments
        VoterC1, // agent
        localStates["interact"], // from
        localStates["check"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "check_tracker1_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_tracker1", -1, "EA1_tracker1_vote", localModels), fakeVarAssignment }, // varAssignments
        VoterC1, // agent
        localStates["check"], // from
        localStates["end"], // to
    }), VoterC1);
    addTransition(new LocalTransition({
        static_cast<int>(VoterC1->localTransitions.size()), // id
        "check_tracker2_VoterC1", // name
        "", // localName
        true, // isShared
        2, // sharedCount
        {  }, // conditions
        { mkVarAssignment("VoterC1_tracker2", -1, "EA1_tracker2_vote", localModels), fakeVarAssignment }, // varAssignments
        VoterC1, // agent
        localStates["check"], // from
        localStates["end"], // to
    }), VoterC1);
    
    // Remove fake var assignments and conditions
    for (const auto& t : VoterC1->localTransitions) {
        t->varAsssignments.erase(fakeVarAssignment);
        t->conditions.erase(fakeCondition);
    }
    
    return VoterC1;
}
