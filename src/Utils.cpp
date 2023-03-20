#include "Utils.hpp"

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

void outputGlobalModel(GlobalModel* globalModel) {
    printf("\n\nGlobal model:\n");
    for (const auto globalState : globalModel->globalStates) {
        printf(
            "%sGlobalState: id=%i; hash=\"%s\"\n",
            globalModel->initState == globalState ? "(initial) " : "",
            globalState->id,
            globalState->hash.c_str()
        );
        for (const auto localState : globalState->localStates) {
            printf("    LocalState %i.%i (%s.%s)", localState->agent->id, localState->id, localState->agent->name.c_str(), localState->name.c_str());
            for (const auto var : localState->vars) {
                printf(" [%s=%i]", var.first->name.c_str(), var.second); // [YK]: what is the use of second (e.g., over Var class `currentValue` member)?
            }
            #if MODEL_ID != 0
            for (const auto var : localState->environment) {
                printf("\n");
                printf("        Var %s = %i", var.first.c_str(), var.second);
            }
            #endif
            printf("\n");
        }
        #if MODEL_ID == 0
        for (const auto var : globalState->vars) {
            printf("    Var %s = %i\n", var.first->name.c_str(), var.second);
        }
        #endif
        for (const auto globalTransition : globalState->globalTransitions) {
            printf("    GlobalTransition id=%i to GlobalState %i\n", globalTransition->id, globalTransition->to->id);
            for (const auto localTransition : globalTransition->localTransitions) {
                printf(
                    "        LocalTransition %i (globalName=%s, localName=%s) of Agent %i (%s);",
                    localTransition->id,
                    localTransition->name.c_str(),
                    localTransition->localName.c_str(),
                    localTransition->agent->id,
                    localTransition->agent->name.c_str()
                );
                if (localTransition->isShared) {
                    printf(" #shared=%i;", localTransition->sharedCount);
                }
                else {
                    printf(" not shared;");
                }
                for (const auto condition : localTransition->conditions) {
                    printf(" <if %s%s%i>", condition->var->name.c_str(), condition->conditionOperator == ConditionOperator::Equals ? "==" : "!=", condition->comparedValue);
                }
                for (const auto varAssignment : localTransition->varAsssignments) {
                    printf(" [set %s=%s]", varAssignment->dstVar->name.c_str(), varAssignment->type == VarAssignmentType::FromValue ? to_string(varAssignment->value).c_str() : varAssignment->srcVar->name.c_str());
                }
                printf("\n");
            }
        }
    }
    for (const auto epiClsAgtPair : globalModel->epistemicClasses) {
        printf("Epistemic classes of Agent %i (%s):\n", epiClsAgtPair.first->id, epiClsAgtPair.first->name.c_str());
        for (const auto epiClsPair : epiClsAgtPair.second) {
            auto epiCls = epiClsPair.second;
            printf("    EpistemicClass: hash=\"%s\"\n", epiCls->hash.c_str());
            for (const auto member : epiCls->globalStates) {
                printf("        GlobalState: id=%i; hash=\"%s\"\n", member.second->id, member.second->hash.c_str());
            }
        }
    }
}

unsigned long getMemCap() {
   int pid=getpid();

   std::string fname="/proc/"+std::to_string(pid)+"/statm";
   std::ifstream status(fname);
   unsigned long s, r, sh, txt, lib, dat, dt;
   status >> s >> r >> sh >> txt >> lib >> dat >> dt;
   status.close();

   return 4*dat;
}