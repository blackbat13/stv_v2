#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "TestParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

using namespace std;

Cfg config;

int main(int argc, char* argv[]) {
    unsigned long mem1 = getMemCap();
    struct timeval tb, te;
    gettimeofday(&tb, NULL);
    
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
                    config.fname = "../src/examples/trains/Trains.txt";
                }else if(val=="2"){
                    config.fname = "../src/examples/ssvr/Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share.txt";
                }else if(val=="3"){
                    config.fname = "../src/examples/svote/Simple_voting.txt";
                }
            }else if(key=="OUTPUT_LOCAL_MODELS"){
                config.output_local_models = (val=="1");
            }else if(key=="OUTPUT_GLOBAL_MODEL"){
                config.output_global_model = (val=="1");
            }else if(key=="MODE"){
                config.stv_mode = val[0];
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
                config.output_global_model =1;
            }else if(arg == "-OUTPUT_LOCAL_MODELS"){
                config.output_local_models = 1;
            }
        }
    }

    auto tp = new TestParser();
    
    tuple<LocalModels*, Formula*> desc = tp->parse(config.fname);
    auto localModels = get<0>(desc);
    auto formula = get<1>(desc);
    
    if(config.output_local_models){
        printf("%s\n", localModelsToString(localModels).c_str());
    }
    
    // // Formula
    // Formula* formula = new Formula();
    // for (const auto agent : localModels->agents) {
    //     if (agent->name == agentName) {
    //         formula->coalition.insert(agent);
    //         break;
    //     }
    // }

    // Generate and output global model
    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);
    


    if(config.stv_mode=='0'){ 
        // do nothing
    }else if(config.stv_mode=='1'){ // read and generate
        generator->expandAllStates();
        if(config.output_global_model){
            outputGlobalModel(generator->getCurrentGlobalModel());
        }
    }else if(config.stv_mode=='2'){ // read, generate and verify
        if(config.output_global_model){
            generator->expandAllStates(); // [YK]: why this was put under the printing-option body?
            outputGlobalModel(generator->getCurrentGlobalModel());
        }
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

    printf("\n\n czas = %lu sec\n\n",czas);

    printf("\n\n%lu - %lu = %lu\n\n",mem2,mem1,mem2-mem1);
    //cout << mem2 << " - " << mem1 << " = " << mem2-mem1 << endl;

    printf("\n\n Number of states: %i\n", ((generator->getCurrentGlobalModel())->globalStates).size());

    return 0;
}


