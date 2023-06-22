#include "gtest/gtest.h"

#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

TEST(GeneralCorrectness, Trains)
{
    config.fname = "../tests/examples/trains/Trains.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = make_shared<ModelParser>();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = make_shared<LocalModels>((get<0>(desc)));
    auto formula = make_shared<Formula>((get<1>(desc)));

    shared_ptr<GlobalModelGenerator> generator = make_shared<GlobalModelGenerator>();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = make_shared<Verification>(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, true);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 8);
}

TEST(GeneralCorrectness, SelectVoteRevoting)
{
    config.fname = "../tests/examples/ssvr/SelectVoteRevoting.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = make_shared<ModelParser>();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = make_shared<LocalModels>((get<0>(desc)));
    auto formula = make_shared<Formula>((get<1>(desc)));

    shared_ptr<GlobalModelGenerator> generator = make_shared<GlobalModelGenerator>();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = make_shared<Verification>(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, false);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 38530);
}

TEST(GeneralCorrectness, SimpleVoting)
{
    config.fname = "../tests/examples/svote/SimpleVoting.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = make_shared<ModelParser>();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = make_shared<LocalModels>((get<0>(desc)));
    auto formula = make_shared<Formula>((get<1>(desc)));

    shared_ptr<GlobalModelGenerator> generator = make_shared<GlobalModelGenerator>();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = make_shared<Verification>(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, false);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 15);
}