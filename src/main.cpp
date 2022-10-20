#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "TestParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {

    unsigned long mem1=getMemCap();
    struct timeval tb, te;
    gettimeofday(&tb, NULL);
    
    string agentName = "";
    // Model
    #if MODEL_ID == 1
        string f = "../src/examples/trains/Trains.txt";
        auto tp = new TestParser();
        auto localModels = tp->parse(f);
    #elif MODEL_ID == 2
        string f = "../src/examples/ssvr/Selene_Select_Vote_Revoting_2v_1cv_3c_3rev_share.txt";
        auto tp = new TestParser();
        auto localModels = tp->parse(f);
        agentName = "Coercer1";
    #elif MODEL_ID == 3
        string f = "../src/examples/svote/Simple_voting.txt";
        auto tp = new TestParser();
        auto localModels = tp->parse(f);
        agentName = "Coercer1";
    #endif
    #if OUTPUT_LOCAL_MODELS
        printf("%s\n", localModelsToString(localModels).c_str());
    #endif

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
    #if MODE == 1
        generator->expandAllStates();
        #if OUTPUT_GLOBAL_MODEL
            outputGlobalModel(generator->getCurrentGlobalModel());
        #endif
    #elif MODE == 2
        #if OUTPUT_GLOBAL_MODEL
            generator->expandAllStates();
            outputGlobalModel(generator->getCurrentGlobalModel());
        #endif
        auto verification = new Verification(generator);
        
        // Show verifications of vars in each global state; to use the following code, make verification->verifyLocalStates public and ensure that generator->expandAllStates() has been called
        // auto gm = generator->getCurrentGlobalModel();
        // for (auto state : gm->globalStates) {
        //     printf(">>>>>> %i; %s; %i\n", state->id, state->hash.c_str(), verification->verifyLocalStates(&state->localStates)?1:0);
        // }
        
        printf("\nVerification result: %s\n", verification->verify() ? "OK" : "ERR");
    #endif

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


