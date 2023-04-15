/**
 * @file TextParser.hpp
 * @brief Model parser.
 * A parser for converting a text file into a model.
 */

#ifndef __TESTPARSER_HPP
#define __TESTPARSER_HPP

#include "Types.hpp"
#include <tuple>

using namespace std;

/// @brief A parser for converting a text file into a model.
class TestParser {
public:
    TestParser();
    ~TestParser();
    tuple<LocalModels*, Formula*> parse(string fileName);
    
protected:
    // @internal
};

#endif
