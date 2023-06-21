#include "gtest/gtest.h"

#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

TEST(TreeTest, 5Choices3Options)
{
    config.fname = "../tests/big_examples/tree/5Choices3Options.txt";
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
}

TEST(TreeTest, 5Choices4Options)
{
    config.fname = "../tests/big_examples/tree/5Choices4Options.txt";
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
}

TEST(TreeTest, 5Choices5Options)
{
    config.fname = "../tests/big_examples/tree/5Choices5Options.txt";
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
}

TEST(TreeTest, 5Choices6Options)
{
    config.fname = "../tests/big_examples/tree/5Choices6Options.txt";
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
}

TEST(TreeTest, 6Choices3Options)
{
    config.fname = "../tests/big_examples/tree/6Choices3Options.txt";
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
}

TEST(TreeTest, 7Choices3Options)
{
    config.fname = "../tests/big_examples/tree/7Choices3Options.txt";
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
}

TEST(TreeTest, 8Choices3Options)
{
    config.fname = "../tests/big_examples/tree/8Choices3Options.txt";
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
}