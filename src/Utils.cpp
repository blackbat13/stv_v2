/**
 * @file Utils.cpp
 * @brief Utility functions.
 * A collection of utility functions to use in the project.
 */

#include "Utils.hpp"
#include <fstream>
#include "GlobalModelGenerator.hpp"
#include <map>
#include <algorithm>

#include <iostream> 
#include <cstdio>

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
    for (const auto& kvp : env) {
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
    return res;
}

/// @brief Prints the whole GlobalModel into the console. Contains global states with hashes, local states, variables inside those states, global variables, global transitions, local transitions and epistemic classes of agents.
/// @param globalModel Pointer to a GlobalModel to print into the console.
void outputGlobalModel(GlobalModel* globalModel) {
    printf("\n\nGlobal model:\n");
    for (const auto globalState : globalModel->globalStates) {
        printf(
            "%sGlobalState: hash=\"%s\"\n",
            globalModel->initState == globalState ? "(initial) " : "",
            globalState->hash.c_str()
        );
        for (const auto localState : globalState->localStatesProjection) {
            printf("    LocalState %i.%i (%s.%s)", localState->agent->id, localState->id, localState->agent->name.c_str(), localState->name.c_str());
            for (const auto var : localState->environment) {
                printf(" [%s=%i]", var.first.c_str(), var.second);
            }
            printf("\n");
        }
        for (const auto globalTransition : globalState->globalTransitions) {
            printf("    GlobalTransition id=%u to GlobalState %s\n", globalTransition->id, (globalTransition->to ? globalTransition->to->hash.c_str() : "-1"));
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
                } else {
                    printf(" not shared;");
                }
                for (const auto condition : localTransition->conditions) {
                    printf(" <if %s%s%i>", condition->var->name.c_str(), condition->conditionOperator == ConditionOperator::Equals ? "==" : "!=", condition->comparedValue);
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
                printf("        GlobalState: hash=\"%s\"\n", member.second->hash.c_str());
            }
        }
    }
}

// [YK]: todo - confirm the correctness of these measures
unsigned long getMemCap() {
    int pid = getpid();
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
    std::string fname = "/proc/" + std::to_string(pid) + "/statm";
    std::ifstream status(fname);
    unsigned long s, r, sh, txt, lib, dat, dt;
    status >> s >> r >> sh >> txt >> lib >> dat >> dt;
    status.close();

    return 4 * dat;
}


void loadConfigFromFile(string filename) {
    // read default values from config.txt
    ifstream ifs(filename, ifstream::in);
    if (ifs.is_open() == true) {
        string line;
        while (getline(ifs, line)) {
            if (line.empty() || line[0] == ';') {
                continue;
            }
            auto delimPos = line.find("=");
            auto key = line.substr(0, delimPos);
            auto val = line.substr(delimPos + 1);
            if (key == "MODEL_ID") {
                config.model_id = val[0] - '0';
                if (val == "1") {
                    config.fname = "../examples/trains/Trains2Controller1.txt";
                } else if (val == "2") {
                    config.fname = "../examples/ssvr/Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share.txt";
                } else if (val == "3") {
                    config.fname = "../examples/svote/Voters2Candidates2.txt";
                }
            } else if (key == "OUTPUT_LOCAL_MODELS") {
                config.output_local_models = (val == "1");
            } else if (key == "OUTPUT_GLOBAL_MODEL") {
                config.output_global_model = (val == "1");
            } else if (key == "MODE") {
                config.stv_mode = (val[0] - '0');
            } else if (key == "OUTPUT_DOT_FILES") {
                config.output_dot_files = (val == "1");
            } else if (key == "DOT_DIR") {
                config.dotdir = val;
            }
        }
    } else {
        printf("Could not open the config file \"%s\"...", filename.c_str());
    }
};

void loadConfigFromArgs(int argc, char** argv) {
    // overwrite the default config values (if provided on the input)
    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            string arg = argv[i];
            if ((arg == "-f") || (arg == "--file")) {
                if (i + 1 < argc) {
                    config.fname = argv[++i];
                } else {
                    printf("ERR: no filename was specified!\n");
                }
            } else if ((arg == "-m") || (arg == "--mode")) {
                if (i + 1 < argc) {
                    config.stv_mode = (argv[++i])[0] - '0';
                } else {
                    printf("ERR: no stv_mode was specified!\n");
                }
            } else if (arg == "-OUTPUT_GLOBAL_MODEL" || arg == "--OUTPUT_GLOBAL_MODEL") {
                config.output_global_model = 1;
            } else if (arg == "-OUTPUT_LOCAL_MODELS" || arg == "--OUTPUT_LOCAL_MODELS") {
                config.output_local_models = 1;
            } else if (arg == "-OUTPUT_DOT_FILES" || arg == "--OUTPUT_DOT_FILES") {
                config.output_dot_files = 1;
            } else if (arg == "-ADD_EPSILON_TRANSITIONS" || arg == "--ADD_EPSILON_TRANSITIONS") {
                config.add_epsilon_transitions = 1;
            } else if (arg == "-OVERWRITE_FORMULA" || arg == "--OVERWRITE_FORMULA") {
                config.formula_from_parameter = 1;
                if (i + 1 < argc) {
                    config.formula = argv[++i];
                } else {
                    printf("ERR: no formula was specified!\n");
                }
            } else if (arg == "-COUNTEREXAMPLE" || arg == "--COUNTEREXAMPLE") {
                config.counterexample = 1;
            } else if (arg == "-REDUCE" || arg == "--REDUCE") {
                config.reduce = 1;
                if (i + 1 < argc) {
                    config.reduce_args = argv[++i];
                } else {
                    printf("ERR: no variables were specified!\n");
                }
            } else if (arg == "-REDUCE_ALL" || arg == "--REDUCE_ALL") {
                config.reduce = 1;
                config.reduce_all = 1;
                if (i + 1 < argc) {
                    config.reduce_args = argv[++i];
                } else {
                    printf("ERR: no variables were specified!\n");
                }
            }
        }
    }
};










/// @brief Utility function for SCC-computatation
/// @param v - current vertex
/// @param dindex - vertex.index (alt. vertex.num)
/// @param lowlink - vertex.lowlink (by df. lowest dindex in the same scc reachable from vertex using tree edges followed by at most one back/cross edge)
/// @param stack - holds candidates for SCC
/// @param onstack - used as condition for back/cross-edge case
/// @param depth - next available (discovery) index
/// @param comp - result of SCC partitioning
void tarjanVisit(LocalState* v, map<int, int>* dindex, map<int, int>* lowlink, stack<LocalState*>* stack, map<int, bool>* onstack, int* depth, vector<set<LocalState*>>* comp) {
    (*dindex)[v->id] = (*depth);    // assign current depth
    (*lowlink)[v->id] = (*depth);   // start with its own dindex
    (*depth)++;                     // increment the (unused) depth

    (*stack).push(v);               // push on stack
    (*onstack)[v->id] = true;

    for (const auto& t : v->localTransitions) {
        if (dindex->count(t->to->id) <= 0) {
            tarjanVisit(t->to, dindex, lowlink, stack, onstack, depth, comp);
            (*lowlink)[v->id] = min((*lowlink)[v->id], (*lowlink)[t->to->id]);
        } else if ((*onstack)[t->to->id]) {
            (*lowlink)[v->id] = min((*lowlink)[v->id], (*dindex)[t->to->id]);
        }
    }

    // if v is a base vertex, then pop the stack to get SCC
    if ((*lowlink)[v->id] == (*dindex)[v->id]) {
        set<LocalState*> currComp;
        while (true) {
            LocalState* w = (*stack).top(); // pop w from the stack
            (*stack).pop();
            (*onstack)[w->id] = false;
            currComp.insert(w);             // add w to curr SCC
            if (v->id == w->id)break;
        }
        comp->push_back(currComp);
    }

}

/// @brief a quick implementation of a Tarjan SCC algorithm (based on DFS)
/// @param agt - an agent whose local graph will be inspected
/// @return localStates partition in a form of the vector, where each set correponds to a SCC
vector<set<LocalState*>> getLocalStatesSCC(Agent* agt) {
    vector<set<LocalState*>> comp;

    // [YK]: these 3 maps below could be replaced with int array (if localstate ids are always conseq. numbers starting from 0)
    map<int, int> dindex; // discovered/depth index
    map<int, int> lowlink;
    map<int, bool> onstack;

    stack<LocalState*> stack;

    int depth = 0;
    for (const auto& v : agt->localStates) {
        if (dindex.count(v->id) <= 0) {
            tarjanVisit(v, &dindex, &lowlink, &stack, &onstack, &depth, &comp);
        }
    }

    return comp;
}


map<LocalState*, vector<GlobalState*>> getContextModel(Formula* formula, LocalModels* localModels, Agent* agt) {
    // partition local states into SCC
    vector<set<LocalState*>> scc = getLocalStatesSCC(agt);        // Set of 
    map<LocalState*, vector<GlobalState*>> res;                    // Loc_i -> St_i
    //
    set<LocalState*> St_i;
    set<LocalTransition*> Edges_i;
    map<LocalState*, set<string>> Loops_i;
    //

    vector<const set<LocalState*>*> openComp;   // components that should be processed

    // find SCC of initial local state
    for (auto const& c : scc) {
        if (c.count(agt->initState)) {
            openComp.push_back(&c);             // push initial component pointer 
            break;
        }
    }

    // global model will store St - current subset of global states 
    GlobalModelGenerator generator = GlobalModelGenerator();
    // compute s_0 in St
    GlobalState* initState = generator.initModel(localModels, formula);
    GlobalModel* gm = generator.getCurrentGlobalModel();      // pointer to a current global model

    const set<LocalState*>* currComp;          // pointer to a const set<LocalState*>
    auto agtInd = generator.agentIndex[agt];
    stack<GlobalState*> q;

    while (!openComp.empty()) {
        // todo: openComp as priority queue (wrt #unprocessed parents)
        currComp = openComp.back();             // component C
        openComp.pop_back();

#if DEBUG_ON  
        cout << "Current component: [" << endl;
        bool comma = false;
        for (const auto& l : *currComp) {
            if (comma) {
                cout << ',' << endl;
                comma = false;
            }
            cout << l->toString("\t");
            comma = true;
        }
        cout << "\n]\n";
#endif

//--------------------------------------------------
// expand the states while contain an appropriate projection to currComp and within oneStepClosure
#if DEBUG_ON  
        cout << "Computing the oneStepClosure" << endl;
#endif
        for (const auto& s : gm->globalStates) {
            if (currComp->count(s->localStatesProjection[agtInd])) {
                q.push(s);
            }
        }

        while (!q.empty()) {
            GlobalState* state = q.top();
            q.pop();
#if DEBUG_ON  
            cout << "\tExpanding state: " << state->hash << endl;
#endif

            vector<GlobalState*> newStates = generator.expandStateAndReturn(state);
            for (auto targetState : newStates) {
                if (currComp->count(targetState->localStatesProjection[agtInd])) {
#if DEBUG_ON  
                    cout << "\t\tEnqueue state: " << targetState->hash << "[witness: " << targetState->localStatesProjection[agtInd]->id << "]" << endl;
#endif
                    q.push(targetState);    // enqueue (to expand) states with i-component in currComp
                }
            }
        }
        //--------------------------------------------------


        // extract local states and add them to St_i
        for (const auto& s : gm->globalStates) {
            if (currComp->count(s->localStatesProjection[agtInd])) {
                res[s->localStatesProjection[agtInd]].push_back(s);     // for debug
                // todo: extend (local) env with observable (global) variables (if possible)
                St_i.insert(s->localStatesProjection[agtInd]);          // from pseudocde
            }
        }

        //--------------------------------------------------


        /*
        [YK]: this is a "working" and HIGHLY unoptimized solution
            - processing each globalState is unnecessary
              (it would suffice to check all pairs of appearing proj[i] x repertoire[i])
            - further optimization can be achieved by modifying
              globalState or globalTransition attributes declarations
            - for a given alpha visitting globalState once should be enough
        */

#if DEBUG_ON  
        cout << "Computing =>_i" << endl;
#endif
        for (const auto& s : gm->globalStates) {
            if (currComp->count(s->localStatesProjection[agtInd])) {
                for (auto localTrn : s->localStatesProjection[agtInd]->localTransitions) {
                    string alpha = localTrn->localName; 
                    stack<GlobalState*> dfsAlphaQ;  // restricted to alpha-edge traversal
                    set<GlobalState*> dfsAlphaVisited;
                    dfsAlphaQ.push(s);
                    while (!dfsAlphaQ.empty()) {
                        GlobalState* curr = dfsAlphaQ.top();
                        dfsAlphaQ.pop();
                        dfsAlphaVisited.insert(curr);

                        for (auto globalTrn : curr->globalTransitions) {              // = globalTransitions.filter(hasAlpha)
                            for (auto compTrn : globalTrn->localTransitions) {
                                if (compTrn->agent == agt && compTrn->localName == alpha) {
                                    // cout  << compTrn->name << " with " << compTrn->localName << endl;
                                    if (globalTrn->to) {
                                        if (currComp->count(globalTrn->to->localStatesProjection[agtInd])) {
                                            if (!dfsAlphaVisited.count(globalTrn->to)) {
                                                dfsAlphaQ.push(globalTrn->to);
                                            } else {
                                                // add alpha-loop for globalTrn->to->localStatesProjection[agtInd]
                                                Loops_i[globalTrn->to->localStatesProjection[agtInd]].insert(alpha);
#if DEBUG_ON  
                                                cout << "\tA cycle was found for " << globalTrn->to->hash << " (adding loop for " << globalTrn->to->localStatesProjection[agtInd]->id << "-[" << compTrn->name << "/" << alpha << "]->" << globalTrn->to->localStatesProjection[agtInd]->id << endl;
#endif
                                            }
                                        } else {
                                            // add alpha-outedge 
                                            Edges_i.insert(compTrn);
#if DEBUG_ON  
                                            cout << "\tAn C-out edge was found for " << globalTrn->from->hash << "-> " << globalTrn->to->hash << "(adding edge for " << globalTrn->from->localStatesProjection[agtInd]->id << "-[" << compTrn->name << "/" << alpha << "]->" << globalTrn->to->localStatesProjection[agtInd]->id << endl;
#endif
                                        }
                                    }
                                }
                            }

                        }
                    }
                }
            }
        }



        // remove St|C from St
        gm->globalStates.erase( // [YK]: does not call destructor... (TOFIX)
            std::remove_if(
                gm->globalStates.begin(),
                gm->globalStates.end(),
                [&](auto s) {
                    for (const auto& l : *currComp) {
                        if (s->localStatesProjection[agtInd] == l) {
#if VERBOSE  
                            cout << "will remove " << s->hash << " which has local " << l->id << endl;
#endif
// generator->expandState(s);
                            return true;
                        }
                    };
                    return false;
                }
            ),
            gm->globalStates.end()
        );// todo: add filter/remove of GlobalStates in "other" places with refs to that...

        // for(auto it = gm->globalStates.begin(); it!=gm->globalStates.end();){
        //     bool cond = false;
        //     for(const auto& l : *currComp){     
        //         if((*it)->localStatesProjection[agtInd]==l){
        //             cond = true;
        //             break;
        //         }
        //     };
        //     if (cond){
        //         it++;
        //     }else{
        //         auto temp = *it;
        //         delete temp;
        //         it = gm->globalStates.erase(it);
        //     }
        // }

        // todo - delete processed globalTransitions
        for (const auto& s : gm->globalStates) {
            for (auto it = s->globalTransitions.begin(); it != s->globalTransitions.end();) {
                // if trn target is not occuring in the globalStates list, then delete the transition
                if (find(gm->globalStates.begin(), gm->globalStates.end(), (*it)->from) != gm->globalStates.end()) {
                    it++;
                } else {
                    cout << "remove: " << (*it)->from->hash << " -> " << "smth" << endl;
                    it = s->globalTransitions.erase(it);
                }
            }
        }// todo: add filter/remove of GlobalTransitions in "other" places with refs to that...

        openComp.clear();
        for (auto const& c : scc) {
            for (const auto& s : gm->globalStates) {
                if (c.count(s->localStatesProjection[agtInd])) {
                    openComp.push_back(&c);
                    break;
                }
            }
        }
    }

    cout << "List of => loops :" << endl;
    for(auto x : Loops_i){
        for( auto it = x.second.begin(); it!=x.second.end();it++){
            cout << "\t" << x.first->id << "-[" <<  *it << "]-> "<< x.first->id << endl;
        }
    }
    cout << "List of => edges :" << endl;
    for( auto it = Edges_i.begin(); it!=Edges_i.end();it++){
        cout << "\t" << (*it)->from->id << "-[" <<  (*it)->localName << "]-> "<< (*it)->to->id << endl;
    }

    // getchar();

    cout << "List of local states approximation as <locationId> : [<stateId>...]" << endl;
    for (const auto& x : res) {
        cout << x.first->id << ": \n";
        for (const auto& s : x.second) {
            cout << s->toString("\t") << endl;
        }
        cout << endl;
    }
    cout << "==================================" << endl;

    return res;
}