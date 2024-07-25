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
        string knowledge;
        string hartley;
        int hCoeff;
        bool le;

    TestVerif(string path) {
        result = verify(path, generator);
        hCoeff = 0;
        le = false;
    }

    TestVerif(string path, bool ok) {
        if (!ok) {
            result = verify(path, generator);
        }
        else {
            result = verifyFull(path, generator);
        }
    }

    ~TestVerif() {
        delete(generator);
    }

    bool verify(string path, GlobalModelGenerator* generator)
    {
        config.fname = path.data();
        config.output_local_models = false;
        config.output_global_model = false;
        config.stv_mode = 2;

        auto tp = new ModelParser();
        
        tuple<LocalModels, Formula> desc = tp->parse(config.fname);
        auto localModels = &(get<0>(desc));
        auto formula = &(get<1>(desc));

        generator->initModel(localModels, formula);

        bool result = false;

        auto verification = new Verification(generator);
        
        result = verification->verify();
        
        return result;
    }

    bool verifyFull(string path, GlobalModelGenerator* generator)
    {
        config.fname = path.data();
        config.output_local_models = false;
        config.output_global_model = false;
        config.stv_mode = 3;

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