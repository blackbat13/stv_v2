#ifndef SELENE_CONSTANTS
#define SELENE_CONSTANTS

#define VERBOSE 1
#define OUTPUT_LOCAL_MODELS 1
#define OUTPUT_GLOBAL_MODEL 1 // warning: it will call expandAllStates()
#define MODE 2 // 1 = only generate; 2 = verify

// Model id
// 0 = hardcoded trains
// 1 = src/examples/trains/Trains.txt
// 2 = src/examples/ssvr/Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share.txt
// 3 = src/examples/svote/Simple_voting.txt
#define MODEL_ID 3

#endif // SELENE_CONSTANTS
