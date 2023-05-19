#include "gtest/gtest.h"

#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "TestParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

TEST(SimpleVotingRunTest, 1Voter1CoercerRun)
{
    config.fname = "../tests/examples/svoterun/1Voter1CoercerRun.txt";
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

    EXPECT_EQ(result, true);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 19);
}

TEST(SimpleVotingRunTest, 1Voter1CoercerNotRun)
{
    config.fname = "../tests/examples/svoterun/1Voter1CoercerNotRun.txt";
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

    EXPECT_EQ(result, true);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 19);
}


TEST(SimpleVotingRunTest, 2Voters1Coercer)
{
    config.fname = "../tests/examples/svoterun/2Voters1Coercer.txt";
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

    EXPECT_EQ(result, true);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 63);
}
