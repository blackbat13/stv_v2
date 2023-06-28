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

void loadConfig(int argc, char** argv){
    // read default values from config.txt
    ifstream ifs("config.txt", ifstream::in);
    if (ifs.is_open() == true) {
        string line;
        while(getline(ifs,line)){
            if(line.empty() || line[0]==';'){
                continue;
            }
            auto delimPos = line.find("=");
            auto key = line.substr(0,delimPos);
            auto val = line.substr(delimPos+1);
            if(key=="MODEL_ID"){
                config.model_id=val[0]-'0';
                if(val=="1"){
                    config.fname = "../examples/trains/Trains.txt";
                }else if(val=="2"){
                    config.fname = "../examples/ssvr/Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share.txt";
                }else if(val=="3"){
                    config.fname = "../examples/svote/Simple_voting.txt";
                }
            }else if(key=="OUTPUT_LOCAL_MODELS"){
                config.output_local_models = (val=="1");
            }else if(key=="OUTPUT_GLOBAL_MODEL"){
                config.output_global_model = (val=="1");
            }else if(key=="MODE"){
                config.stv_mode = val[0];
            }else if(key=="OUTPUT_DOT_FILES"){
                config.output_dot_files = (val=="1");
            }
        }
    }else{
        printf("Could not open the config file...");
    }

    // overwrite the default config values (if provided on the input)
    if(argc>=2){
        for(int i=1;i<argc;i++){
            string arg = argv[i];
            if( (arg == "-f") || (arg == "--file")){
                if(i+1<argc){
                    config.fname = argv[++i];
                }else{
                    printf("ERR: no filename was specified!\n");
                }
            }else if((arg == "-m") || (arg == "--mode")){
                if(i+1<argc){
                    config.stv_mode = argv[i++][0];
                }else{
                    printf("ERR: no stv_mode was specified!\n");
                }
            }else if(arg == "-OUTPUT_GLOBAL_MODEL"){
                config.output_global_model = 1;
            }else if(arg == "-OUTPUT_LOCAL_MODELS"){
                config.output_local_models = 1;
            }else if(arg == "-OUTPUT_DOT_FILES"){
                config.output_dot_files = 1;
            }
        }
    }
}



int main(int argc, char* argv[]) {
    unsigned long mem1 = getMemCap();
    struct timeval tb, te;
    gettimeofday(&tb, NULL);
    
    loadConfig(argc,argv); 
    auto tp = new ModelParser();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = &(get<0>(desc));
    auto formula = &(get<1>(desc));

    for (const auto agent : localModels->agents) {
        cout << agent->name << " local states: " << agent->localStates.size() << endl;
        cout << agent->name << " local transitions: " << agent->localTransitions.size() << endl;
    }
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

    if(config.stv_mode=='0'){           // 0 - atm redundant (can be used as special, debug mode)
    }else if(config.stv_mode=='1'){     // 1 - ignore verification
        generator->expandAllStates();
    }else if(config.stv_mode=='2'){     // 2 - read, generate and verify
        // generator->expandAllStates();
        auto verification = new Verification(generator);
        // Show verifications of vars in each global state; to use the following code, make verification->verifyLocalStates public and ensure that generator->expandAllStates() has been called
        // auto gm = generator->getCurrentGlobalModel();
        // for (auto state : gm->globalStates) {
        //     printf(">>>>>> %i; %s; %i\n", state->id, state->hash.c_str(), verification->verifyLocalStates(&state->localStates)?1:0);
        // }
        printf("\nVerification result: %s\n", verification->verify() ? "OK" : "ERR");
    }

    gettimeofday(&te, NULL);
    //unsigned long long czas = 1000000 * (te.tv_sec - tb.tv_sec) + (te.tv_usec - tb.tv_usec);
    unsigned long czas = (te.tv_sec - tb.tv_sec);
    unsigned long mem2=getMemCap();

    printf("\n\nczas = %lu sec\n\n",czas);

    printf("\n\n%lu - %lu = %lu\n\n",mem2,mem1,mem2-mem1);
    //cout << mem2 << " - " << mem1 << " = " << mem2-mem1 << endl;

    printf("\n\nNumber of global states: %i\n", ((generator->getCurrentGlobalModel())->globalStates).size());

    return 0;
}


