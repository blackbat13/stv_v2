#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <algorithm>
#include <sys/stat.h>

#include "reader/nodes.hpp"
#include "DotGraph.hpp"
#include "KnowledgeBasedConstruction.hpp"

using namespace std;

Cfg config;

extern set<AgentTemplate*>* modelDescription;
extern FormulaTemplate formulaDescription;

int main(int argc, char* argv[]) {
    unsigned long mem1 = getMemCap();
    struct timeval tb, te;
	struct stat buffer;
    gettimeofday(&tb, NULL);
    loadConfigFromFile("config.txt");
    loadConfigFromArgs(argc,argv);
	
	LocalModels* clonableLM;
	
	if(!(stat (config.fname.c_str(), &buffer) == 0)){
		std::cout << "ERROR: File " << config.fname << " not found!" << endl;
		return -1;
	}

    auto tp = new ModelParser();
    string fbasename = config.fname.substr(config.fname.find_last_of("/\\") + 1,config.fname.rfind('.')-config.fname.find_last_of("/\\")-1);
    tuple<LocalModels, Formula> desc;
    if (!config.formula_from_parameter) {
        desc = tp->parse(config.fname);
    }
    else {
        desc = tp->parseAndOverwriteFormula(config.fname, config.formula);
    }
    auto localModels = &(get<0>(desc));
    auto formula = &(get<1>(desc));

    // Generate and output global model
    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);
	//clonableLM->agents = cloneGlobalModel(localModels, formula)->agents;
	
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
            DotGraph(it).saveToFile(config.dotdir, fbasename+"-");
        }
        // save LocalModels
        for (const auto& agt : localModels->agents) {
            DotGraph(agt, true).saveToFile(config.dotdir, fbasename+"-");
        }
        // save GlobalModel
        if(!config.reduce) {
            generator->expandAllStates();   // todo: add allExpanded flag?
        }
        DotGraph(generator->getCurrentGlobalModel(), true).saveToFile(config.dotdir, fbasename+"-");
    }

    if(config.reduce) {
        generator->expandAndReduceAllStates();
        if(config.output_dot_files){
            // save GlobalModel solution
            DotGraph(generator->getCurrentGlobalModel(), true, true).saveToFile(config.dotdir, fbasename+"-");
        }
    } else if(config.stv_mode & (1 << 0)){     // mode.binary = /[0,1]*1/ (generate)
        generator->expandAllStates();
    }
    
    if(config.stv_mode & (1 << 1)){     // mode.binary = /[0,1]*1[0,1]/ (verify)
        auto verification = new Verification(generator);
        // Show verifications of vars in each global state; to use the following code, make verification->verifyLocalStates public and ensure that generator->expandAllStates() has been called
        // auto gm = generator->getCurrentGlobalModel();
        // for (auto state : gm->globalStates) {
        //     printf(">>>>>> %i; %s; %i\n", state->id, state->hash.c_str(), verification->verifyLocalStates(&state->localStates)?1:0);
        // }
        bool verifResult = verification->verify();
        printf("Verification result: %s\n", verifResult ? "TRUE" : "FALSE");
        if (!verifResult && config.counterexample) {
            verification->historyDecisionsERR();
        }
        if(config.output_dot_files && verifResult){
            // save GlobalModel solution
            DotGraph(generator->getCurrentGlobalModel(), true, true).saveToFile(config.dotdir, fbasename+"-");
        }
        delete verification;
    }

    if(config.stv_mode & (1 << 2)){     // mode.binary = /[0,1]*1[0,1]{2}/ (print metadata)
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

    if(config.stv_mode & (1 << 3)){
        ofstream ofs;

/* ------- Uncomment for the SCC compute test/debug ------- */
        ofs.open(fbasename+"-scc.txt");
        for (const auto& agt : localModels->agents) {
            // cout << "SCC for agent " << agt->name << " are as follows:" << endl;
            auto res = getLocalStatesSCC(agt);
            for(const auto comp : res) {
                ofs << "[ ";
                for(const auto& l: comp){
                    ofs << l->id << " ";
                }
                ofs << "]" << endl;
            }  
        }
        ofs.close();
/*----------------------------------------------------------*/

        map<LocalState*,vector<GlobalState*>> ctxmodel = getContextModel(formula, localModels, localModels->agents[1]);
        ofs.open(fbasename+"-"+localModels->agents[1]->name+"-LocalContext.txt");

        for(const auto& x : ctxmodel){
            ofs << x.first->id << ": ";
            for(const auto& s : x.second){
                ofs << s->hash << " ";
            }
            ofs << endl;
        }
        ofs.close();
    }
    
	if(config.kbc){//run Filip Jamroga's code (location to be changed)
        int maxKBCIterations = 1;
        int i=0;
        GlobalModel* gm = generator->getCurrentGlobalModel();

        // Generate the list of agents that KBC is applied to based on user config
        set<int> selectedAgents = {};
        if(config.kbc_coalition_only){
            for (auto itr = formula->coalition.begin(); itr != formula->coalition.end(); itr++) {
                selectedAgents.insert((*itr)->id);
            }
        }else{
            for(int i=0; i<gm->agents.size(); i++) selectedAgents.insert(i);
        }

        // Perform KBC the expected number of times
        for(i=0; i<maxKBCIterations; i++){
            //ModelDotDump(gm, "KBC-iter-"+to_string(i)+"-");
            cout << "Attempting KBC Iteration #" << i+1 << endl;
            LocalModels KBCdLM;

            // Perform KBC for the selected agents
            for(int j=0; j<gm->agents.size(); j++){
                if(selectedAgents.count(j)>0){
                    GlobalModel* cloneModel = cloneGlobalModel(localModels, formula);
                    KBCprojection(cloneModel, j);
                    Agent* a = KBCexpansion(cloneModel, j);
                    KBCdLM.agents.push_back(a);
                    //DotGraph(a).saveToFile("", "kbc-");
                    cout << "Created a KBC'd version of Agent #" << j << " " << a->name << endl;
                    //cout << "Stats for Agent #" << j << ": States: " << a->localStates.size() << " Transitions: " << a->localTransitions.size() << endl;
                    delete cloneModel;
                }else{
                    KBCdLM.agents.push_back(localModels->agents[j]->clone());
                    cout << "Copied Agent #" << j << " verbatim\n";
                }
            }cout << "Finished KBC iteration\n";
            GlobalModelGenerator* KBCdGenerator = new GlobalModelGenerator();
            //cout << ".\n";
            KBCdGenerator->initModel(&KBCdLM, formula);
            //cout << ".\n";
            KBCdGenerator->expandAllStates();

            if(config.output_dot_files){
                // save LocalModels
                for (const auto& agt : KBCdLM.agents) {
                    DotGraph(agt, true).saveToFile(config.dotdir, fbasename+"-KBC-");
                }
                // save GlobalModel
                //generator->expandAllStates();   // todo: add allExpanded flag?
                DotGraph(KBCdGenerator->getCurrentGlobalModel(), true).saveToFile(config.dotdir, fbasename+"-KBC-");
            }

            cout << "Attempting verification\n";
            auto KBCverif = new Verification(KBCdGenerator);
            printf("Post-KBC verification result: %s\n", KBCverif->verify() ? "TRUE" : "FALSE");
            delete gm;
            gm = KBCdGenerator->getCurrentGlobalModel();
            //cout << "Stats for Global Model after KBC iteration #" << i << ": States: " << gm->globalStates.size() << endl;
            delete KBCverif;
        }
        ModelDotDump(gm, "KBC-iter-"+to_string(i)+"-");
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

    //delete generator;

    return 0;
}


