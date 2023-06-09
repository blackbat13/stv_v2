#include "gtest/gtest.h"

#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

TEST(ClockTest, 2Counters11States)
{
    config.fname = "../tests/big_examples/2Counters11States.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = new ModelParser();
    
    tuple<LocalModels*, Formula*> desc = tp->parse(config.fname);
    auto localModels = get<0>(desc);
    auto formula = get<1>(desc);

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = new Verification(generator);
    
    result = verification->verify();

    free(tp);
    free(generator);

    EXPECT_EQ(result, true);
}

TEST(ClockTest, 4Counters11States)
{
    config.fname = "../tests/big_examples/4Counters11States.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = new ModelParser();
    
    tuple<LocalModels*, Formula*> desc = tp->parse(config.fname);
    auto localModels = get<0>(desc);
    auto formula = get<1>(desc);

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = new Verification(generator);
    
    result = verification->verify();

    free(tp);
    free(generator);

    EXPECT_EQ(result, true);
}

TEST(ClockTest, 5Counters11States)
{
    config.fname = "../tests/big_examples/5Counters11States.txt";
    config.output_local_models = false;
    config.output_global_model = false;
    config.stv_mode = '2';

    auto tp = new ModelParser();
    
    tuple<LocalModels*, Formula*> desc = tp->parse(config.fname);
    auto localModels = get<0>(desc);
    auto formula = get<1>(desc);

    GlobalModelGenerator* generator = new GlobalModelGenerator();
    generator->initModel(localModels, formula);

    bool result = false;

    generator->expandAllStates();

    auto verification = new Verification(generator);
    
    result = verification->verify();

    free(tp);
    free(generator);

    EXPECT_EQ(result, true);
}