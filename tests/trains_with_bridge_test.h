#include "gtest/gtest.h"

#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "TestParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

TEST(TrainsWithBridgeTest, 1Train1ControllerOnBridge)
{
    config.fname = "../tests/examples/trains/1Train1ControllerOnBridge.txt";
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

    generator->expandAllStates();

    auto verification = new Verification(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, false);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 3);
}

TEST(TrainsWithBridgeTest, 2Trains1ControllerOnBridge)
{
    config.fname = "../tests/examples/trains/2Trains1ControllerOnBridge.txt";
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

    generator->expandAllStates();

    auto verification = new Verification(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, false);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 8);
}