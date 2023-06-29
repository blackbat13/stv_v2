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

    auto tp = new ModelParser();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = &(get<0>(desc));
    auto formula = &(get<1>(desc));

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = new Verification(generator);
    
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

    auto tp = new ModelParser();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = &(get<0>(desc));
    auto formula = &(get<1>(desc));

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = new Verification(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, true);

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 1155);
}

TEST(ClockTestF, Clock6Hours15Minutes)
{
    config.fname = "../tests/examples/clock/Clock6Hours15MinutesF.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = new ModelParser();
    
    tuple<LocalModels, Formula> desc = tp->parse(config.fname);
    auto localModels = &(get<0>(desc));
    auto formula = &(get<1>(desc));

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = new Verification(generator);
    
    result = verification->verify();

    EXPECT_EQ(result, true);
}