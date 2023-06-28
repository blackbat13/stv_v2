#ifndef SELENE_CONSTANTS
#define SELENE_CONSTANTS

#define VERBOSE 0
// #define OUTPUT_LOCAL_MODELS 1
// #define OUTPUT_GLOBAL_MODEL 0 // warning: it will call expandAllStates()
// #define MODE 2 // 1 = only generate; 2 = verify

// Model id
// 1 = /examples/trains/Trains.txt
// 2 = /examples/ssvr/Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share.txt
// 3 = /examples/svote/Simple_voting.txt
// #define MODEL_ID 1

struct Cfg{
    char* fname;
    char stv_mode;
    bool output_local_models;
    bool output_global_model;
    bool output_dot_files;
    int model_id; // <-- this is temporary member (used in Verification.cpp for a hardcoded formula)
};

#endif // SELENE_CONSTANTS
