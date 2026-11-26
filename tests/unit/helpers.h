/**
 * @file helpers.h
 * @brief Shared fixtures and builders for the unit-test suite.
 *
 * Unit tests build tiny models either directly in memory (ModelArena) or from
 * a few lines of model text written to a temporary file (parseModelText), so
 * every expected value can be checked by hand next to the assertion.
 */

#ifndef UNIT_HELPERS_H
#define UNIT_HELPERS_H

#include "gtest/gtest.h"

#include "Types.hpp"
#include "GlobalModelGenerator.hpp"
#include "Verification.hpp"
#include "ModelParser.hpp"
#include "StrategyParser.hpp"
#include "Utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <climits>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <unistd.h>
#include <vector>

extern Cfg config;

/// @brief Base fixture: every test starts with a zeroed global configuration.
class UnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = Cfg{};
    }
};

/// @brief Writes @p text to a fresh temporary file and returns its path.
/// @param text File contents.
/// @param suffix Optional file-name suffix (e.g. ".txt").
/// @return Absolute path of the created file. The caller removes it.
inline std::string writeTempFile(const std::string& text, const std::string& suffix = ".txt") {
    std::string pattern = "/tmp/stv_unit_XXXXXX" + suffix;
    std::vector<char> path(pattern.begin(), pattern.end());
    path.push_back('\0');
    int fd = mkstemps(path.data(), static_cast<int>(suffix.size()));
    if (fd == -1) {
        throw std::runtime_error("could not create temporary file");
    }
    FILE* file = fdopen(fd, "w");
    fputs(text.c_str(), file);
    fclose(file);
    return std::string(path.data());
}

/// @brief Creates a fresh temporary directory and returns its path (without a trailing slash).
inline std::string makeTempDir() {
    char pattern[] = "/tmp/stv_unit_dir_XXXXXX";
    char* dir = mkdtemp(pattern);
    if (dir == nullptr) {
        throw std::runtime_error("could not create temporary directory");
    }
    return std::string(dir);
}

/// @brief Reads a whole file into a string ("" if it does not exist).
inline std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

/// @brief True if a file exists at @p path.
inline bool fileExists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

/// @brief Parsed model kept together so the Formula's agent pointers stay valid.
struct ParsedModel {
    LocalModels models;
    Formula formula;

    /// @brief Finds an agent by name (agent order depends on the parser's pointer-ordered set).
    Agent* agent(const std::string& name) {
        for (auto agent : models.agents) {
            if (agent->name == name) {
                return agent;
            }
        }
        return nullptr;
    }
};

/// @brief Parses model text through ModelParser via a temporary file.
/// @param text Model description in the regular input format.
/// @return Parsed local models and formula.
inline ParsedModel parseModelText(const std::string& text) {
    std::string path = writeTempFile(text);
    ModelParser parser;
    ParsedModel result;
    try {
        std::tie(result.models, result.formula) = parser.parse(path);
    } catch (...) {
        std::remove(path.c_str());
        throw;
    }
    std::remove(path.c_str());
    return result;
}

/// @brief Finds a local state of @p agent by name ("" -> nullptr).
inline LocalState* stateNamed(Agent* agent, const std::string& name) {
    for (auto state : agent->localStates) {
        if (state->name == name) {
            return state;
        }
    }
    return nullptr;
}

/// @brief Finds the global state whose local-state names are exactly @p names (agent order).
inline GlobalState* globalStateNamed(GlobalModel* model, const std::vector<std::string>& names) {
    for (auto globalState : model->globalStates) {
        bool same = globalState->localStatesProjection.size() == names.size();
        for (size_t i = 0; same && i < names.size(); i++) {
            same = globalState->localStatesProjection[i]->name == names[i];
        }
        if (same) {
            return globalState;
        }
    }
    return nullptr;
}

/// @brief Owns hand-built model objects and deletes them when the test ends.
///
/// The model structs use raw pointers everywhere, so the arena keeps every
/// allocation alive for the duration of a test and frees it afterwards.
class ModelArena {
public:
    /// @brief Creates an agent with no states.
    Agent* agent(int id, const std::string& name) {
        agents.emplace_back(new Agent(id, name));
        return agents.back().get();
    }

    /// @brief Adds a local state to @p owner with the next free id. The first state becomes the initial state.
    LocalState* state(Agent* owner, const std::string& name, const std::map<std::string, int>& environment = {}) {
        states.emplace_back(new LocalState());
        LocalState* state = states.back().get();
        state->id = static_cast<uint32_t>(owner->localStates.size());
        state->name = name;
        state->environment = environment;
        state->agent = owner;
        owner->localStates.push_back(state);
        if (owner->initState == nullptr) {
            owner->initState = state;
        }
        return state;
    }

    /// @brief Adds a local transition @p from -> @p to and registers it on the agent and the source state.
    LocalTransition* transition(LocalState* from, LocalState* to, const std::string& name, const std::string& localName = "",
                                bool isShared = false, int sharedCount = -1, float probability = 1.0f) {
        transitions.emplace_back(new LocalTransition());
        LocalTransition* transition = transitions.back().get();
        Agent* owner = from->agent;
        transition->id = static_cast<int>(owner->localTransitions.size());
        transition->name = name;
        transition->localName = localName.empty() ? name : localName;
        transition->isShared = isShared;
        transition->sharedCount = sharedCount;
        transition->probability = probability;
        transition->agent = owner;
        transition->from = from;
        transition->to = to;
        owner->localTransitions.push_back(transition);
        from->localTransitions.insert(transition);
        return transition;
    }

    /// @brief Creates a variable of @p owner and adds it to the agent's variable set.
    Var* var(Agent* owner, const std::string& name, bool persistent, int initialValue = 0) {
        vars.emplace_back(new Var());
        Var* var = vars.back().get();
        var->name = name;
        var->persistent = persistent;
        var->initialValue = initialValue;
        var->agent = owner;
        owner->vars.insert(var);
        return var;
    }

    /// @brief Creates a condition "var op value" (no expression).
    Condition* condition(Var* var, ConditionOperator op, int value) {
        conditions.emplace_back(new Condition());
        Condition* condition = conditions.back().get();
        condition->var = var;
        condition->conditionOperator = op;
        condition->comparedValue = value;
        return condition;
    }

    /// @brief Creates a condition backed by an expression tree.
    Condition* condition(ExprNode* expression) {
        conditions.emplace_back(new Condition());
        Condition* condition = conditions.back().get();
        condition->expression = expression;
        return condition;
    }

    /// @brief Creates a free-standing global state from the given local states (hash "id;id;...").
    GlobalState* globalState(const std::vector<LocalState*>& localStates) {
        globalStates.emplace_back(new GlobalState());
        GlobalState* state = globalStates.back().get();
        state->localStatesProjection = localStates;
        for (auto localState : localStates) {
            state->hash += std::to_string(localState->id) + ";";
        }
        return state;
    }

    /// @brief Creates a global transition between two global states, made of @p parts.
    GlobalTransition* globalTransition(GlobalState* from, GlobalState* to, const std::set<LocalTransition*>& parts) {
        globalTransitions.emplace_back(new GlobalTransition());
        GlobalTransition* transition = globalTransitions.back().get();
        transition->isInvalidDecision = false;
        transition->from = from;
        transition->to = to;
        transition->localTransitions = parts;
        if (from != nullptr) {
            from->globalTransitions.insert(transition);
        }
        return transition;
    }

    /// @brief Creates an epistemic class with the given hash.
    EpistemicClass* epistemicClass(const std::string& hash) {
        epistemicClasses.emplace_back(new EpistemicClass());
        epistemicClasses.back()->hash = hash;
        return epistemicClasses.back().get();
    }

private:
    std::vector<std::unique_ptr<Agent>> agents;
    std::vector<std::unique_ptr<LocalState>> states;
    std::vector<std::unique_ptr<LocalTransition>> transitions;
    std::vector<std::unique_ptr<Var>> vars;
    std::vector<std::unique_ptr<Condition>> conditions;
    std::vector<std::unique_ptr<GlobalState>> globalStates;
    std::vector<std::unique_ptr<GlobalTransition>> globalTransitions;
    std::vector<std::unique_ptr<EpistemicClass>> epistemicClasses;
};

/// @brief Exposes protected GlobalModelGenerator members to the tests.
struct GeneratorProbe : public GlobalModelGenerator {
    using GlobalModelGenerator::computeGlobalStateHash;
    using GlobalModelGenerator::computeEpistemicClassHash;
    using GlobalModelGenerator::findOrCreateEpistemicClass;
    using GlobalModelGenerator::isCoalitionTransitionCompatibleWithStrategy;
    using GlobalModelGenerator::checkLocalStates;
    using GlobalModelGenerator::generateStateFromLocalStates;
};

/// @brief Exposes protected Verification members to the tests.
struct VerificationProbe : public Verification {
    explicit VerificationProbe(GlobalModelGenerator* generator) : Verification(generator) {}

    using Verification::verifyLocalStates;
    using Verification::isGlobalTransitionControlledByCoalition;
    using Verification::isGlobalTransitionCompatibleWithCoalitionDecisions;
    using Verification::addCoalitionDecisionHistory;
    using Verification::isAgentInCoalition;
    using Verification::getEpistemicClassForGlobalState;
    using Verification::areGlobalStatesInTheSameEpistemicClass;
    using Verification::addHistoryDecision;
    using Verification::addHistoryStateStatus;
    using Verification::addHistoryContext;
    using Verification::addHistoryMarkDecisionAsInvalid;
    using Verification::addHistoryUncontrolledDecision;
    using Verification::newHistoryMarkDecisionAsInvalid;
    using Verification::undoLastHistoryEntry;
    using Verification::undoHistoryUntil;
    using Verification::equivalentGlobalTransitions;
    using Verification::globalStateToValueBits;
    using Verification::reduceStrategy;
    using Verification::increaseProbability;
    using Verification::lowerProbability;

    HistoryEntry* start() { return historyStart; }
    HistoryEntry* end() { return historyEnd; }
    const vector<string>& names() { return variableNames; }
    short variableLimit() { return strategyVariableLimit; }

    /// @brief Number of entries after the sentinel start entry.
    int historyLength() {
        int count = 0;
        for (auto entry = historyStart->next; entry != nullptr; entry = entry->next) {
            count++;
        }
        return count;
    }
};

/// @brief A parsed model with an initialised generator, ready for generator or verification tests.
struct GeneratedModel {
    ParsedModel parsed;
    GeneratorProbe generator;

    explicit GeneratedModel(const std::string& text) : parsed(parseModelText(text)) {
        generator.initModel(&parsed.models, &parsed.formula);
    }

    GlobalModel* model() { return generator.getCurrentGlobalModel(); }
    Agent* agent(const std::string& name) { return parsed.agent(name); }
};

/// @brief Builds an argv-style array from strings (argv[0] is the program name).
class ArgvBuilder {
public:
    explicit ArgvBuilder(const std::vector<std::string>& args) : storage(args) {
        storage.insert(storage.begin(), "stv");
        for (auto& arg : storage) {
            pointers.push_back(&arg[0]);
        }
        pointers.push_back(nullptr);
    }
    int argc() const { return static_cast<int>(storage.size()); }
    char** argv() { return pointers.data(); }

private:
    std::vector<std::string> storage;
    std::vector<char*> pointers;
};

#endif // UNIT_HELPERS_H
