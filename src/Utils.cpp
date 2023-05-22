/**
 * @file Utils.cpp
 * @brief Utility functions.
 * A collection of utility functions to use in the project.
 */

#include "Utils.hpp"

extern Cfg config;

/// @brief Converts a map of string and int to a string.
/// @param env Map to be converted into a string.
/// @return Returns string " (first_name, second_name, ..., last_name=int_value)"
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

/// @brief Converts pointer to an Agant into a string containing name of the agent, its initial state, transitions with their local and global names, shared count and conditions. 
/// @param agt Pointer to an Agent to parse into a string.
/// @return String containing all of Agent data.
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

/// @brief Converts pointer to the LocalModels into a string cointaining all Agent instances from the model, initial values of the variables and names of the persistent values.
/// @param lm Pointer to the local model to parse into a string.
/// @return String containing all of LocalModels data.
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

/// @brief Prints the whole GlobalModel into the console. Contains global states with hashes, local states, variables inside those states, global variables, global transitions, local transitions and epistemic classes of agents.
/// @param globalModel Pointer to a GlobalModel to print into the console.
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
                printf(" [%s=%i]", var.first->name.c_str(), var.second);
            }
            if(config.model_id != 0){
                for (const auto var : localState->environment) {
                    printf("\n");
                    printf("        Var %s = %i", var.first.c_str(), var.second);
                }
            }
            printf("\n");
        }
        if(config.model_id == 0){
            for (const auto var : globalState->vars) {
                printf("    Var %s = %i\n", var.first->name.c_str(), var.second);
            }
        }
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

// [YK]: todo - confirm the correctness of these measures
unsigned long getMemCap() {
   int pid=getpid();
/*
    From https://man7.org/linux/man-pages/man5/proc.5.html
    /proc/[pid]/statm
        Provides information about memory usage, measured in pages.  The columns are:
            size       (1) total program size
                        (same as VmSize in /proc/[pid]/status)
            resident   (2) resident set size
                        (inaccurate; same as VmRSS in /proc/[pid]/status)
            shared     (3) number of resident shared pages (i.e., backed by a file)
                        (inaccurate; same as RssFile+RssShmem in /proc/[pid]/status)
            text       (4) text (code)
            lib        (5) library (unused since Linux 2.6; always 0)
            data       (6) data + stack
            dt         (7) dirty pages (unused since Linux 2.6; always 0)

        Some of these values are inaccurate because of a kernel-internal scalability optimization.  
        If accurate values are required, use /proc/[pid]/smaps or /proc/[pid]/smaps_rollup instead, 
        which are much slower but provide accurate, detailed information.
*/
   std::string fname="/proc/"+std::to_string(pid)+"/statm";
   std::ifstream status(fname);
   unsigned long s, r, sh, txt, lib, dat, dt;
   status >> s >> r >> sh >> txt >> lib >> dat >> dt;
   status.close();

   return 4*dat;
}


// [YK]: todo - extract to a separate file as static class with .dot parameters (e.g., font, colours, etc)
/// @brief Parses the pointed local model to a dot-formatted files for basic visualisation
/// @return 
void localModelsToDotFile(LocalModels* lm){
    const int _edgeLabelFontSize = 10;

    ofstream ofs;
    for(const auto &agt: lm->agents){
        string fileName = "lts_of_" + agt->name + ".dot";
        ofs.open(fileName);
        ofs << "digraph \"" << agt->name << "\"{\n";
        ofs << "\tlabel=\"LTS of " << agt->name << "\"\n";                                 // figure label

        ofs << "\tedge[fontsize=\"" << to_string(_edgeLabelFontSize) << "\"]\n";    // font size for the edge-labels
        ofs << "\tnode [\n"
               "\t\tshape=circle,\n"
               "\t\tfixedsize=true,\n"
               "\t\twidth=auto,\n"
               "\t\tcolor=\"black\",\n"
               "\t\tfillcolor=\"#eeeeee\",\n"
               "\t\tstyle=\"filled,solid\",\n"
               "\t\tfontsize=8,\n"
               "\t\tfontname=\"Roboto\"\n"
               "\t]\n";                                                               // node settings
        ofs << "\tfontname=Consolas\n";                                             // font-family (will be inherited)
        ofs << "\tlayout=dot\n";                                                    // layout engine name

        // parse local states
        for (const auto& s : agt->localStates){
            ofs << "\t" << to_string(s->id) << "[label=\"" << s->name << "\"]\n";
        }

        // parse local transitions
        for (const auto& t : agt->localTransitions) {
            ofs << "\t" << to_string(t->from->id) << "->" << to_string(t->to->id);
            ofs << "[label=\"" << t->name << "\"";
            if (t->isShared) {
                ofs << ", color=\"blue\"";
            }
            ofs << "]\n";
        }

        ofs << "}";
        ofs.close();
    }
}

/// @brief Utility function for SCC-computatation
/// @param v - current vertex
/// @param dindex - vertex.index (alt. vertex.num)
/// @param lowlink - vertex.lowlink (by df. lowest dindex in the same scc reachable from vertex using tree edges followed by at most one back/cross edge)
/// @param stack - holds candidates for SCC
/// @param onstack - used as condition for back/cross-edge case
/// @param depth - next available (discovery) index
/// @param comp - result of SCC partitioning
void tarjanVisit(LocalState* v, map<int,int>* dindex, map<int,int>* lowlink, stack<LocalState*>* stack, map<int,bool>* onstack, int* depth, vector<set<LocalState*>>* comp){
    (*dindex)[v->id] = (*depth);    // assign current depth
    (*lowlink)[v->id] = (*depth);   // start with its own dindex
    (*depth)++;                     // increment the (unused) depth
    
    (*stack).push(v);               // push on stack
    (*onstack)[v->id] = true;     

    for(const auto& t : v->localTransitions){
        if(dindex->count(t->to->id)<=0){
            tarjanVisit(t->to, dindex, lowlink, stack, onstack, depth, comp);
            (*lowlink)[v->id] = min((*lowlink)[v->id],(*lowlink)[t->to->id]);
        }else if( (*onstack)[t->to->id]) {
            (*lowlink)[v->id] = min((*lowlink)[v->id],(*dindex)[t->to->id]); 
        }
    }

    // if v is a base vertex, then pop the stack to get SCC
    if( (*lowlink)[v->id] == (*dindex)[v->id] ){ 
        set<LocalState*> currComp;
        while(true){
            LocalState* w = (*stack).top(); // pop w from the stack
            (*stack).pop();
            (*onstack)[w->id] = false;
            currComp.insert(w);             // add w to curr SCC
            if( v->id == w->id )break;
        }
        comp->push_back(currComp);
    }

}

/// @brief a quick implementation of a Tarjan SCC algorithm (based on DFS)
/// @param agt - an agent whose local graph will be inspected
/// @return localStates partition in a form of the vector, where each set correponds to a SCC
vector<set<LocalState*>> getSCC(Agent* agt){
    vector<set<LocalState*>> comp;
    
    // [YK]: these 3 maps below could be replaced with int array (if localstate ids are always conseq. numbers starting from 0)
    map<int,int> dindex; // discovered/depth index
    map<int,int> lowlink;
    map<int,bool> onstack; 

    stack<LocalState*> stack;

    int depth = 0;
    for(const auto& v : agt->localStates){
        if(dindex.count(v->id)<=0){
            tarjanVisit(v, &dindex, &lowlink, &stack, &onstack, &depth, &comp);
        }
    }

    return comp;
}