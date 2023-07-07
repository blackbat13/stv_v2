#ifndef TEST_CONFIG
#define TEST_CONFIG
#include "gtest/gtest.h"

#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>

Cfg config;

class TestVerif
{
    public:
        GlobalModelGenerator* generator = new GlobalModelGenerator();
        bool result;

    TestVerif(string path) {
        result = verify(path, generator);
    }

    ~TestVerif() {
        delete(generator);
    }

    bool verify(string path, GlobalModelGenerator* generator)
    {
        config.fname = path.data();
        config.output_local_models = false;
        config.output_global_model = false;
        config.stv_mode = '2';

        auto tp = new ModelParser();
        
        tuple<LocalModels, Formula> desc = tp->parse(config.fname);
        auto localModels = &(get<0>(desc));
        auto formula = &(get<1>(desc));

        generator->initModel(localModels, formula);

        bool result = false;

        generator->expandAllStates();

        auto verification = new Verification(generator);
        
        result = verification->verify();
        
        return result;
    }
};

#endif