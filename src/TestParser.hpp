#ifndef __TESTPARSER_HPP
#define __TESTPARSER_HPP

#include "Types.hpp"
#include <tuple>

using namespace std;

class TestParser {
public:
    TestParser();
    ~TestParser();
    tuple<LocalModels*, Formula*> parse(string fileName);
    
protected:
    // @internal
};

#endif
