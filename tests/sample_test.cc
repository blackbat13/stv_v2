#include "gtest/gtest.h"

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

TEST(GeneralCorrectness, TrainsTest)
{
    config.fname = "../src/examples/trains/Trains.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = new TestParser();
    
    tuple<LocalModels*, Formula*> desc = tp->parse(config.fname);
    auto localModels = get<0>(desc);
    auto formula = get<1>(desc);

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    if(config.stv_mode=='0') {}
    else if(config.stv_mode=='1') {
        generator->expandAllStates();
    }
    else if(config.stv_mode=='2') {
        if(config.output_global_model) {
            generator->expandAllStates();
        }
        auto verification = new Verification(generator);
        
        result = verification->verify();
    }

    EXPECT_EQ(result, true);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 8);
}

TEST(GeneralCorrectness, SelectVoteRevoting)
{
    config.fname = "../src/examples/ssvr/Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = new TestParser();
    
    tuple<LocalModels*, Formula*> desc = tp->parse(config.fname);
    auto localModels = get<0>(desc);
    auto formula = get<1>(desc);

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    if(config.stv_mode=='0') {}
    else if(config.stv_mode=='1') {
        generator->expandAllStates();
    }
    else if(config.stv_mode=='2') {
        if(config.output_global_model) {
            generator->expandAllStates();
        }
        auto verification = new Verification(generator);
        
        result = verification->verify();
    }

    EXPECT_EQ(result, false);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 1);
}

TEST(GeneralCorrectness, SimpleVoting)
{
    config.fname = "../src/examples/svote/Simple_voting.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = new TestParser();
    
    tuple<LocalModels*, Formula*> desc = tp->parse(config.fname);
    auto localModels = get<0>(desc);
    auto formula = get<1>(desc);

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    if(config.stv_mode=='0') {}
    else if(config.stv_mode=='1') {
        generator->expandAllStates();
    }
    else if(config.stv_mode=='2') {
        if(config.output_global_model) {
            generator->expandAllStates();
        }
        auto verification = new Verification(generator);
        
        result = verification->verify();
    }

    EXPECT_EQ(result, false);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 1);
}

int main() {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}