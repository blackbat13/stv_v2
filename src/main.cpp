#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "examples/trains/Trains.cpp"
#include "TestParser.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>
#include <string>

using namespace std;

inline unsigned long getMemCap() {
   int pid=getpid();

   std::string fname="/proc/"+std::to_string(pid)+"/statm";
   std::ifstream status(fname);
   unsigned long s, r, sh, txt, lib, dat, dt;
   status >> s >> r >> sh >> txt >> lib >> dat >> dt;
   status.close();

   return 4*dat;
}

void outputGlobalModel(GlobalModel* globalModel);

int main() {

    unsigned long mem1=getMemCap();
    struct timeval tb, te;
    gettimeofday(&tb, NULL);
    
    string agentName = "";
    // Model
    #if MODEL_ID == 0
        auto localModels = createTrains();
        agentName = "Train1";
    #elif MODEL_ID == 1
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
