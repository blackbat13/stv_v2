/**
 * @file TextModelParser.hpp
 * @brief Model parser.
 * A parser for converting a text file into a model.
 */

#ifndef __TESTPARSER_HPP
#define __TESTPARSER_HPP

#include "Types.hpp"
#include <tuple>

using namespace std;

/// @brief A parser for converting a text file into a model.
class ModelParser {
public:
    ModelParser();
    ~ModelParser();
    tuple<LocalModels, Formula> parse(string fileName);
    tuple<LocalModels, Formula> parseAndOverwriteFormula(string fileName, string s);
    
protected:
    // @internal
};

#endif
