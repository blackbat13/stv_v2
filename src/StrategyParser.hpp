/**
 * @file StrategyParser.hpp
 * @brief Strategy file parser.
 * A parser for converting a text file into a strategy.
 */

#ifndef __STRATEGYPARSER_HPP
#define __STRATEGYPARSER_HPP

#include "Types.hpp"
#include <tuple>

using namespace std;

/// @brief A parser for converting a text file into a strategy.
class StrategyParser {
    public:
        StrategyParser();
        ~StrategyParser();
        StrategyCollection parse(string fileName);
    
    protected:
        // @internal
};

#endif
