#include "gtest/gtest.h"

#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

TEST(SimpleVotingWithFakesTest, 1Voter1Coercer)
{
    config.fname = "../tests/examples/svotewithfakes/1Voter1CoercerWithFakes.txt";
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

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 21);
}

TEST(SimpleVotingWithFakesTest, 2Voters1Coercer)
{
    config.fname = "../tests/examples/svotewithfakes/2Voters1CoercerWithFakes.txt";
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

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 135);
}

TEST(SimpleVotingWithFakesTest, 3Voters1Coercer)
{
    config.fname = "../tests/examples/svotewithfakes/3Voters1CoercerWithFakes.txt";
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

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 837);
}

TEST(SimpleVotingWithFakesTest, 4Voters1Coercer)
{
    config.fname = "../tests/examples/svotewithfakes/4Voters1CoercerWithFakes.txt";
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

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 5103);
}

TEST(SimpleVotingWithFakesNotGiveTest, 1Voter1Coercer)
{
    config.fname = "../tests/examples/svotewithfakes/1Voter1CoercerWithFakesNG.txt";
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

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 21);
}

TEST(SimpleVotingWithFakesNotGiveTest, 2Voters1Coercer)
{
    config.fname = "../tests/examples/svotewithfakes/2Voters1CoercerWithFakesNG.txt";
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

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 135);
}

TEST(SimpleVotingWithFakesNotGiveTest, 3Voters1Coercer)
{
    config.fname = "../tests/examples/svotewithfakes/3Voters1CoercerWithFakesNG.txt";
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

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 837);
}

TEST(SimpleVotingWithFakesNotGiveTest, 4Voters1Coercer)
{
    config.fname = "../tests/examples/svotewithfakes/4Voters1CoercerWithFakesNG.txt";
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

    EXPECT_EQ((generator->getCurrentGlobalModel())->globalStates.size(), 5103);
}