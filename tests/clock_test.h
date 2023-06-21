#include "gtest/gtest.h"

#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

TEST(ClockTest, Clock6Hours15Minutes)
{
    config.fname = "../tests/examples/clock/Clock6Hours15Minutes.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = make_shared<ModelParser>();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = &(get<0>(desc));
    auto formula = &(get<1>(desc));

    shared_ptr<GlobalModelGenerator> generator = make_shared<GlobalModelGenerator>();
    generator->initModel(make_shared<LocalModels>(localModels), make_shared<Formula>(formula));

    bool result = false;

    generator->expandAllStates();

    auto verification = make_shared<Verification>(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, true);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 90);
}

TEST(ClockTest, Clock6Hours15MinutesAndDays)
{
    config.fname = "../tests/examples/clock/Clock6Hours15MinutesAndDays.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = make_shared<ModelParser>();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = &(get<0>(desc));
    auto formula = &(get<1>(desc));

    shared_ptr<GlobalModelGenerator> generator = make_shared<GlobalModelGenerator>();
    generator->initModel(make_shared<LocalModels>(localModels), make_shared<Formula>(formula));

    bool result = false;

    generator->expandAllStates();

    auto verification = make_shared<Verification>(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, true);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 1155);
}