/**
 * @file ModelSerializer.hpp
 * @brief Serializes an in-memory model back to the text input format.
 */

#ifndef MODEL_SERIALIZER_HPP
#define MODEL_SERIALIZER_HPP

#include "Types.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

/// @brief Reconstructs and prints a model definition file from LocalModels and Formula.
///
/// Output format matches the .txt model files accepted by the parser.
///
/// Limitations:
/// - Assignment expressions that depend on variable values at runtime
///   (e.g. score:=score+1) are serialized as concrete integers because
///   the original expression tree is not stored after model generation.
/// - Transition conditions (e.g. [Voter1_vote==1]) are not stored in
///   LocalTransition after model generation; states that were split by
///   conditions appear as distinct states with disambiguated names instead.
class ModelSerializer {
public:
    /// @brief Prints the full model description to stdout.
    /// @param localModels Local models to serialize.
    /// @param formula Formula to serialize.
    void print(LocalModels* localModels, Formula* formula);

private:
    // Returns the state name as it appears in the local model.
    string stateName(LocalState* state);

    // Strips the "[agentId]_" prefix added during model generation for non-shared transitions.
    string baseActionName(LocalTransition* t);

    // Builds the "[var:=value, ...]" assignment string by comparing from/to environments.
    // Only outputs variables whose values changed (persistent) or appeared (local).
    string assignmentString(LocalTransition* t, const set<string>& persistentVarNames);

    // Serializes a single agent block.
    string serializeAgent(Agent* agent);

    // Serializes the FORMULA line.
    string serializeFormula(Formula* formula);
};

#endif // MODEL_SERIALIZER_HPP
