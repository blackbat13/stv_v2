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
