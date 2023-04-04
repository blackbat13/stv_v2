/**
 * @file TextParser.hpp
 * @brief Model parser.
 * A parser for converting a text file into a model.
 */

#ifndef __TESTPARSER_HPP
#define __TESTPARSER_HPP

#include "Types.hpp"

using namespace std;

class TestParser {
public:
    TestParser();
    ~TestParser();
    LocalModels* parse(string fileName);
    
protected:
    // @internal
};

#endif
