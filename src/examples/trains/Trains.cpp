#include "../../Types.hpp"
#include "Utils.cpp"
#include "Train1.cpp"
#include "Train2.cpp"
#include "Controller.cpp"

// Trains.txt

LocalModels* createTrains() {
    LocalModels* Trains = new LocalModels();
    
    Trains->agents.push_back(createTrain1(Trains));
    Trains->agents.push_back(createTrain2(Trains));
    Trains->agents.push_back(createController(Trains));
    
    // Initial var values
    map<string, int> initialVarValues = {
        { "Train1_pos", 1 },
        { "Train2_pos", 1 },
    };
    for (const auto& p : initialVarValues) {
        auto var = getVar(p.first, Trains);
        var->initialValue = p.second;
    }
    
    // Persistent vars
    vector<string> persistentVars = {
    };
    for (const auto& p : persistentVars) {
        auto var = getVar(p, Trains);
        var->persistent = true;
    }
    
    for (auto agt1 : Trains->agents) {
        for (auto agt2 : Trains->agents) {
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
    
    return Trains;
}

string envToString(map<string, int> env) {
    if (env.size() == 0) {
        return "";
    }
    string res = "";
    res += " (";
    bool isFirst = true;
    for (const auto &kvp : env) {
        if (!isFirst) {
            res += ", ";
        }
        isFirst = false;
        res += kvp.first + "=";
        res += to_string(kvp.second);
    }
    res += ")";
    return res;
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
        res += ": " + t->from->name + " id=" + to_string(t->from->id);
        res += envToString(t->from->environment);
        res += " -";
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
        res += "> " + t->to->name + " id=" + to_string(t->to->id);
        res += envToString(t->to->environment);
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
