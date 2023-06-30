#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <algorithm>

#include "reader/nodes.hpp"
#include "DotGraph.hpp"

using namespace std;

Cfg config;

extern set<AgentTemplate*>* modelDescription;
extern FormulaTemplate formulaDescription;

int main(int argc, char* argv[]) {
    unsigned long mem1 = getMemCap();
    struct timeval tb, te;
    gettimeofday(&tb, NULL);
    loadConfigFromFile("config.txt");
    loadConfigFromArgs(argc,argv);

    auto tp = new ModelParser();
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = &(get<0>(desc));
    auto formula = &(get<1>(desc));

/* ------- Uncomment for the SCC compute test/debug ------- */
    // for (const auto& agt : localModels->agents) {
    //     cout << "SCC for agent " << agt->name << " are as follows:" << endl;
    //     auto res = getLocalStatesSCC(agt);
    //     for(const auto comp : res) {
    //         cout << "[ ";
    //         for(const auto& l: comp){
    //             cout << l->id << " ";
    //         }
    //         cout << "]" << endl;
    //     }  
    // }
/*----------------------------------------------------------*/
// /* ----------------------- YK tests ----------------------- */
    // map<LocalState*,vector<GlobalState*>> ctxmodel = getContextModel(formula, localModels, localModels->agents[1]);
    // // return 0;

    // ofstream ofs;
    // ofs.open("temp.txt");

    // for(const auto& x : ctxmodel){
    //     ofs << x.first->id << ": ";
    //     for(const auto& s : x.second){
    //         ofs << s->hash << " ";
    //     }
    //     ofs << endl;
    // }
    // ofs.close();
    // return 0;
//     GlobalModelGenerator* generator1 = new GlobalModelGenerator();
//     generator1->initModel(localModels, formula);
//     generator1->expandAllStates();
//     // stack<GlobalState*> q;
//     // q.push(generator1->initModel(localModels, formula));
//     // int upperBound = 0;
//     // // while(!q.empty() && upperBound<200){
//     // while(!q.empty()){
//     //     GlobalState* state = q.top();
//     //     q.pop();   
//     //     // cout << "expanding state " << state->hash << endl;  
//     //     vector<GlobalState*> newStates = generator1->expandStateAndReturn(state);
//     //     for(auto targetState : newStates){
//     //         q.push(targetState);
//     //         upperBound++;
//     //     }
//     // }
//     // cout << "total number of states = "  << generator1->getCurrentGlobalModel()->globalStates.size() << endl;
//     DotGraph dg = DotGraph(generator1->getCurrentGlobalModel(), true);
//     dg.saveToFile();
//     return 0;

//     // Generate and output global model
//     GlobalModelGenerator* generator = new GlobalModelGenerator();
//     generator->initModel(localModels, formula);
//     if(config.output_local_models){
//         printf("%s\n", localModelsToString(localModels).c_str());
//     }
    
//     // for(const auto& st: generator->getCurrentGlobalModel()->globalStates){
//     //     cout << "Global state id = " << st->id << endl;
//     //     for(const auto& loc: st->localStates){
//     //         cout << "\t" << loc->name << "(" << loc->id <<") of agent " << loc->agent->name << "(" << loc->agent->id <<")" << endl;
//     //     }
//     // }
//     // cout << "Current number of states = " << ((generator->getCurrentGlobalModel())->globalStates).size() << endl;
//     // return 0;
// // /*----------------------------------------------------------*/


    // Generate and output global model
    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);
    if(config.output_local_models){
        printf("%s\n", localModelsToString(localModels).c_str());
    }
    /* NOTE:
     * with this flag the whole global states gets generated,
     * whereas in Verification::verifyGlobalState (called by ::verify) those are expanded on demand (!)
    */
    if(config.output_global_model){
        generator->expandAllStates();
        outputGlobalModel(generator->getCurrentGlobalModel());
    }

    if(config.output_dot_files){
        // save AgentTemplates
        for(auto it:*modelDescription) {
            DotGraph(it).saveToFile();
        }
        // save LocalModels
        for (const auto& agt : localModels->agents) {
            DotGraph(agt, true).saveToFile();
        }
        // save GlobalModel
        generator->expandAllStates();   // todo: add allExpanded flag?
        DotGraph(generator->getCurrentGlobalModel(), true).saveToFile();
    }


    if(config.stv_mode & (1 << 0)){     // mode.binary = [0,1]*1 (generate)
        generator->expandAllStates();
    }
    
    if(config.stv_mode & (1 << 1)){     // mode.binary = [0,1]*1[0,1] (verify)
        auto verification = new Verification(generator);
        // Show verifications of vars in each global state; to use the following code, make verification->verifyLocalStates public and ensure that generator->expandAllStates() has been called
        // auto gm = generator->getCurrentGlobalModel();
        // for (auto state : gm->globalStates) {
        //     printf(">>>>>> %i; %s; %i\n", state->id, state->hash.c_str(), verification->verifyLocalStates(&state->localStates)?1:0);
        // }
        printf("Verification result: %s\n", verification->verify() ? "OK" : "ERR");
    }

    if(config.stv_mode & (1 << 2)){     // mode.binary = [0,1]*1[0,1]{2} (print metadata)
        // Print out model metadata
        for (const auto agent : localModels->agents) {
            // name: [states, transitions]
            cout << agent->name << ":  [" << agent->localStates.size() << ", " << agent->localTransitions.size() <<  "]"<< endl;
        }
        
        set<GlobalTransition*> globTrn;
        for(auto const st : (generator->getCurrentGlobalModel())->globalStates){
            for(auto tr: st->globalTransitions){
                globTrn.insert(tr);
            }
        }
        cout <<"Global:  [" << (generator->getCurrentGlobalModel())->globalStates.size() << ", " << (globTrn.size()) << "]" << endl;
        //
    }
    

    if(false){
        gettimeofday(&te, NULL);
        //unsigned long long czas = 1000000 * (te.tv_sec - tb.tv_sec) + (te.tv_usec - tb.tv_usec);
        unsigned long czas = (te.tv_sec - tb.tv_sec);
        unsigned long mem2=getMemCap();
        printf("\n\nczas = %lu sec\n\n",czas);
        printf("\n\n%lu - %lu = %lu\n\n",mem2,mem1,mem2-mem1);
        printf("\n\nNumber of global states: %i\n", ((generator->getCurrentGlobalModel())->globalStates).size());
    }

    return 0;
}


