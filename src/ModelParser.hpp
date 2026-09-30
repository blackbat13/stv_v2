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
private:
    string localStateTupleKey(const vector<LocalState*>& tuple);
    map<string, int> mergeEnvironment(const vector<LocalState*>& tuple);
    string mergedStateName(const vector<LocalState*>& tuple);
    LocalState *findOrCreateMergedState(Agent *mergedAgent, const vector<LocalState *> &tuple, map<string, LocalState *> &mergedStatesByKey, map<LocalState *, vector<LocalState *>> &tupleOfMergedState, queue<LocalState *> &pendingStates);
    void mergeCoalition(set<string>* coalitionNames, LocalModels * localModels);
    map<string, int> *resolveSharedTransitions(Agent* mergedAgent, LocalState* mergedState, const vector<LocalState*>& fromTuple, map<string, LocalState*>& mergedStatesByKey, map<LocalState*, vector<LocalState*>>& tupleOfMergedState, queue<LocalState*>& pendingStates);
};

#endif
