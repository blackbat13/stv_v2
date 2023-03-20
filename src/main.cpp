#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "TestParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    unsigned long mem1 = getMemCap();
    struct timeval tb, te;
    gettimeofday(&tb, NULL);

    char* fname;
    
    char stv_mode;
    bool output_local_models = false;
    bool output_global_model = false;

    string agentName = "";

    if(argc<2){ // use default const values
        printf("No arguments were provided - using the default ones.\n");
        // Model
        #if MODEL_ID == 1
            fname = "../src/examples/trains/Trains.txt";
        #elif MODEL_ID == 2
            fname = "../src/examples/ssvr/Selene_Select_Vote_Revoting_2v_1cv_3c_3rev_share.txt";
            agentName = "Coercer1";
        #elif MODEL_ID == 3
            fname = "../src/examples/svote/Simple_voting.txt";
            agentName = "Coercer1";
        #endif

        #if OUTPUT_LOCAL_MODELS == 1
            output_local_models = true;
        #endif        

        #if OUTPUT_GLOBAL_MODEL == 1
            output_global_model = true;
        #endif

        #if MODE == 1
            stv_mode = '1';
        #endif
        #if MODE == 2
            stv_mode = '2';
        #endif
    }else{
        for(int i=1;i<argc;i++){
            string arg = argv[i];
            if( (arg == "-f") || (arg == "--file")){
                if(i+1<argc){
                    fname = argv[++i];
                }else{
                    printf("ERR: no filename was specified!\n");
                }
            }else if((arg == "-m") || (arg == "--mode")){
                if(i+1<argc){
                    stv_mode = argv[i++][0];
                }else{
                    printf("ERR: no stv_mode was specified!\n");
                }
            }else if(arg == "-OUTPUT_GLOBAL_MODEL"){
                output_global_model =1;
            }else if(arg == "-OUTPUT_LOCAL_MODELS"){
                output_local_models = 1;
            }
        }
        // printf("filename = %s\n",fname);
    }

    auto tp = new TestParser();
    auto localModels = tp->parse(fname);
    
    if(output_local_models){
        printf("%s\n", localModelsToString(localModels).c_str());
    }
    
    // Formula
    Formula* formula = new Formula();
    for (const auto agent : localModels->agents) {
        if (agent->name == agentName) {
            formula->coalition.insert(agent);
            break;
        }
    }

    // Generate and output global model
    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);
    


    if(stv_mode=='0'){ 
        // do nothing
    }else if(stv_mode=='1'){ // read and generate
        generator->expandAllStates();
        if(output_global_model){
            outputGlobalModel(generator->getCurrentGlobalModel());
        }
    }else if(stv_mode=='2'){ // read, generate and verify
        if(output_global_model){
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


