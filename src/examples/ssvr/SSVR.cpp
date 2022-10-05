#include "../../Types.hpp"
#include "Utils.cpp"
#include "EA1.cpp"
#include "Voter1.cpp"
#include "VoterC1.cpp"
#include "Coercer1.cpp"

// SSVR = Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share.txt

LocalModels* createSSVR() {
    LocalModels* SSVR = new LocalModels();
    
    SSVR->agents.push_back(createEA1(SSVR));
    SSVR->agents.push_back(createVoter1(SSVR));
    SSVR->agents.push_back(createVoterC1(SSVR));
    SSVR->agents.push_back(createCoercer1(SSVR));
    
    // Initial var values
    map<string, int> initialVarValues = {
        { "VoterC1_revote", 1 }
    };
    for (const auto& p : initialVarValues) {
        auto var = getVar(p.first, SSVR);
        var->initialValue = p.second;
    }
    
    // Persistent vars
    vector<string> persistentVars = {
        "Coercer1_finish",
        "Voter1_vote",
        "Voter1_tracker",
        "VoterC1_vote",
        "Coercer1_VoterC1_vote",
        "VoterC1_tracker",
        "VoterC1_required",
        "Coercer1_VoterC1_tracker",
        "Coercer1_VoterC1_required",
        "VoterC1_revote",
        "Coercer1_VoterC1_revote",
        "VoterC1_prep_vote",
        "EA1_tracker1",
        "EA1_tracker1_vote",
        "EA1_tracker2",
        "EA1_tracker2_vote",
    };
    for (const auto& p : persistentVars) {
        auto var = getVar(p, SSVR);
        var->persistent = true;
    }
    
    for (auto agt1 : SSVR->agents) {
        for (auto agt2 : SSVR->agents) {
            if (agt1 == agt2) {
                continue;
            }
            for (auto tr1 : agt1->localTransitions) {
                if (!tr1->isShared) {
                    continue;
                }
                for (auto tr2 : agt2->localTransitions) {
                    if (!tr2->isShared) {
                        continue;
                    }
                    if (tr1->name != tr2->name) {
                        continue;
                    }
                    tr1->sharedLocalTransitions.insert(tr2);
                }
            }
        }
    }
    
    return SSVR;
}

string agentToString(Agent* agt) {
    string res = "";
    res += "Agent " + agt->name + ":\n";
    res += "init " + agt->initState->name + "\n";
    for (const auto& t : agt->localTransitions) {
        if (t->isShared) {
            res += "shared[" + to_string(t->sharedCount) + "] ";
        }
        res += t->name;
        if (t->isShared) {
            string localName = t->name;
            if (t->localName.length() > 0) {
                localName = t->localName;
            }
            res += "[" + localName + "]";
        }
        res += ": " + t->from->name + " -";
        if (t->conditions.size() > 0) {
            res += "[";
            bool first = true;
            for (auto _c = t->conditions.begin(); _c != t->conditions.end(); _c++) {
                if (!first) {
                    res += ", ";
                }
                first = false;
                auto c = *_c;
                res += c->var->name + (c->conditionOperator == ConditionOperator::Equals ? "==" : "!=") + to_string(c->comparedValue);
            }
            res += "]";
        }
        res += "> " + t->to->name;
        if (t->varAsssignments.size() > 0) {
            res += " [";
            bool first = true;
            for (auto _a = t->varAsssignments.begin(); _a != t->varAsssignments.end(); _a++) {
                if (!first) {
                    res += ", ";
                }
                first = false;
                auto a = *_a;
                res += a->dstVar->name + "=" + (a->type == VarAssignmentType::FromValue ? to_string(a->value) : ("?" + a->srcVar->name));
            }
            res += "]";
        }
        res += "\n";
    }
    return res;
}

string localModelsToString(LocalModels* lm) {
    string res = "";
    for (const auto& agt : lm->agents) {
        res += agentToString(agt);
        res += "\n";
    }
    bool isFirst = true;
    for (const auto& v : lm->vars) {
        if (v.second->initialValue != -1) {
            if (isFirst) {
                res += "INITIAL: [";
            }
            else {
                res += ", ";
            }
            isFirst = false;
            res += v.second->name +  "=" + to_string(v.second->initialValue);
        }
    }
    if (!isFirst) {
        res += "]\n";
    }
    isFirst = true;
    for (const auto& v : lm->vars) {
        if (v.second->persistent) {
            if (isFirst) {
                res += "PERSISTENT: [";
            }
            else {
                res += ", ";
            }
            isFirst = false;
            res += v.second->name;
        }
    }
    if (!isFirst) {
        res += "]\n";
    }
    return res;
}
