/**
 * @file ModelSerializer.cpp
 * @brief Serializes an in-memory model back to the text input format.
 */

#include "ModelSerializer.hpp"
#include "Agent.hpp"
#include "LocalState.hpp"
#include "LocalTransition.hpp"
#include "reader/expressions.hpp"
#include "enums/ConditionOperator.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// ---- helpers ----------------------------------------------------------------

string ModelSerializer::stateName(LocalState* state) {
    return state->name;
}

string ModelSerializer::baseActionName(LocalTransition* t) {
    if (!t->isShared) {
        string prefix = "[" + to_string(t->agent->id) + "]_";
        if (t->name.size() > prefix.size() && t->name.compare(0, prefix.size(), prefix) == 0) {
            return t->name.substr(prefix.size());
        }
    }
    return t->name;
}

static string environmentSignature(const map<string, int>& environment) {
    string result;
    for (const auto& item : environment) {
        result += item.first + "=" + to_string(item.second) + ";";
    }
    return result;
}

static string formatProbability(float probability) {
    stringstream ss;
    ss << fixed << setprecision(6) << probability;
    string result = ss.str();
    while (!result.empty() && result.back() == '0') {
        result.pop_back();
    }
    if (!result.empty() && result.back() == '.') {
        result.pop_back();
    }
    return result;
}

string ModelSerializer::assignmentString(LocalTransition* t, const set<string>& persistentVarNames) {
    vector<string> parts;

    for (const auto& kv : t->to->environment) {
        const string& varName = kv.first;
        int toVal = kv.second;

        if (persistentVarNames.count(varName)) {
            // Persistent variable: only emit if it changed.
            auto fromIt = t->from->environment.find(varName);
            int fromVal = (fromIt != t->from->environment.end()) ? fromIt->second : 0;
            if (fromVal != toVal) {
                parts.push_back(varName + ":=" + to_string(toVal));
            }
        } else {
            // Local variable: appears in destination only when explicitly assigned.
            parts.push_back(varName + ":=" + to_string(toVal));
        }
    }

    if (parts.empty()) return "";
    string result = "[";
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) result += ", ";
        result += parts[i];
    }
    result += "]";
    return result;
}

static string transitionSignature(LocalTransition* t, const set<string>& persistentVarNames) {
    stringstream signature;
    signature << (t->isShared ? "shared" : "local") << "|";
    if (!t->isShared) {
        string prefix = "[" + to_string(t->agent->id) + "]_";
        if (t->name.size() > prefix.size() && t->name.compare(0, prefix.size(), prefix) == 0) {
            signature << t->name.substr(prefix.size());
        } else {
            signature << t->name;
        }
    } else {
        signature << t->name;
    }
    signature << t->from->name << "|" << environmentSignature(t->from->environment) << "|";
    signature << t->to->name << "|" << environmentSignature(t->to->environment) << "|";

    vector<string> parts;
    for (const auto& kv : t->to->environment) {
        const string& varName = kv.first;
        int toVal = kv.second;
        if (persistentVarNames.count(varName)) {
            auto fromIt = t->from->environment.find(varName);
            int fromVal = (fromIt != t->from->environment.end()) ? fromIt->second : 0;
            if (fromVal != toVal) {
                parts.push_back(varName + ":=" + to_string(toVal));
            }
        } else {
            parts.push_back(varName + ":=" + to_string(toVal));
        }
    }
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) signature << ",";
        signature << parts[i];
    }
    return signature.str();
}

// ---- agent serialization ----------------------------------------------------

string ModelSerializer::serializeAgent(Agent* agent) {
    stringstream ss;

    // Separate local and persistent variable names.
    set<string> localVarNames, persistentVarNames;
    for (auto var : agent->vars) {
        if (var->persistent) {
            persistentVarNames.insert(var->name);
        }
        else {
            localVarNames.insert(var->name);
        }
    }

    // Header
    ss << "Agent " << agent->name << ":\n";

    // LOCAL
    ss << "LOCAL: [";
    bool first = true;
    for (const auto& n : localVarNames) {
        if (!first) ss << ", ";
        ss << n;
        first = false;
    }
    ss << "]\n";

    // PERSISTENT
    ss << "PERSISTENT: [";
    first = true;
    for (const auto& n : persistentVarNames) {
        if (!first) ss << ", ";
        ss << n;
        first = false;
    }
    ss << "]\n";

    // INITIAL — only emit variables whose initial value differs from the default (0).
    ss << "INITIAL: [";
    first = true;
    for (const auto& kv : agent->initState->environment) {
        if (kv.second != 0) {
            if (!first) ss << ", ";
            ss << kv.first << ":=" << kv.second;
            first = false;
        }
    }
    ss << "]\n";

    // Initial state declaration
    ss << "init " << stateName(agent->initState) << "\n";

    // Group transitions by action and source state so probabilistic bundles stay together.
    map<string, vector<LocalTransition*>> groupedTransitions;
    for (auto t : agent->localTransitions) {
        string prefix;
        if (t->isShared) {
            prefix = "shared[" + to_string(t->sharedCount) + "] " + t->name + "[" + t->localName + "]";
        } else {
            prefix = baseActionName(t);
        }
        string groupKey = prefix + "|" + stateName(t->from);
        groupedTransitions[groupKey].push_back(t);
    }

    // Sort groups by the lowest transition id in each group for deterministic output.
    // This ensures a stable, reproducible order across runs.
    vector<vector<LocalTransition*>> sortedGroups;
    sortedGroups.reserve(groupedTransitions.size());
    for (auto& entry : groupedTransitions) {
        sortedGroups.push_back(entry.second);
    }
    sort(sortedGroups.begin(), sortedGroups.end(),
         [](const vector<LocalTransition*>& a, const vector<LocalTransition*>& b) {
             return a.front()->id < b.front()->id;
         });

    // Emit each action group (action + source state).
    for (auto& transitions : sortedGroups) {
        // Deduplicate identical branches within the group.
        // Branches are identical if they have the same:
        //   - destination state name
        //   - assignments (as a string)
        //   - probability value
        // This handles cases where model generation creates internal duplicates.
        vector<LocalTransition*> uniqueTransitions;
        uniqueTransitions.reserve(transitions.size());
        set<string> seenBranches;
        for (auto transition : transitions) {
            string branchKey = transition->to->name + "|" + assignmentString(transition, persistentVarNames) + "|" + formatProbability(transition->probability);
            if (seenBranches.insert(branchKey).second) {
                uniqueTransitions.push_back(transition);
            }
        }

        if (uniqueTransitions.empty()) {
            continue;
        }

        LocalTransition* first_t = uniqueTransitions[0];
        // Probabilistic means this action has multiple distinct outcome branches.
        bool probabilistic = uniqueTransitions.size() > 1;

        // If all branches have probability 1, they're not truly probabilistic outcomes—
        // they're likely artifacts from model generation (e.g., one with assignments, one without).
        // In this case, prefer the transition with assignments; if none have assignments, use the first.
        if (probabilistic && uniqueTransitions.size() > 0) {
            bool allProb1 = true;
            for (auto* t : uniqueTransitions) {
                if (t->probability != 1.0f) {
                    allProb1 = false;
                    break;
                }
            }
            if (allProb1) {
                // All have probability 1: find the one with the most/best assignments.
                LocalTransition* best = uniqueTransitions[0];
                string bestAssigns = assignmentString(best, persistentVarNames);
                for (size_t i = 1; i < uniqueTransitions.size(); ++i) {
                    string assigns = assignmentString(uniqueTransitions[i], persistentVarNames);
                    if (!assigns.empty() && bestAssigns.empty()) {
                        best = uniqueTransitions[i];
                        bestAssigns = assigns;
                    }
                }
                first_t = best;
                probabilistic = false;  // Treat as single non-probabilistic action.
            }
        }

        // Build the action prefix: either "shared[N] global[local]" or just "actionName".
        string prefix;
        if (first_t->isShared) {
            prefix = "shared[" + to_string(first_t->sharedCount) + "] "
                   + first_t->name + "[" + first_t->localName + "]";
        } else {
            prefix = baseActionName(first_t);
        }

        string fromName = stateName(first_t->from);

        string conditionStr = "";
        if (first_t->conditions.size() > 0) {
            for (const auto& c : first_t->conditions) {
                if (!conditionStr.empty()) {
                    conditionStr += ", ";
                }
                if (c->expression != nullptr) {
                    conditionStr += c->expression->toString(false);
                } else if (c->var != nullptr) {
                    conditionStr += c->var->name + (c->conditionOperator == ConditionOperator::Equals ? "==" : "!=") + to_string(c->comparedValue);
                }
            }
        }

        if (!probabilistic) {
            // Non-probabilistic: single outcome, single line.
            string assigns = assignmentString(first_t, persistentVarNames);
            string line = prefix + ": " + fromName + (conditionStr.empty() ? "" : " [" + conditionStr + "]") + " -> " + stateName(first_t->to);
            if (!assigns.empty()) line += " " + assigns;
            ss << line << "\n";
        } else {
            // Probabilistic: multiple branches (outcomes) in a single action group.
            // Format:
            //   action: source -> 0.5 : outcome1 [assigns]
            //                     0.3 : outcome2 [assigns]
            //                     &REST : outcome3 [assigns]    <-- last branch uses &REST
            // The indent aligns continuation lines under the first "->".
            string indent(prefix.size() + fromName.size() + (conditionStr.empty() ? 0 : conditionStr.size() + 3) + 6, ' ');

            for (size_t i = 0; i < uniqueTransitions.size(); ++i) {
                auto* ti = uniqueTransitions[i];
                string assignsi = assignmentString(ti, persistentVarNames);

                if (i == 0) {
                    // First branch: full action header with explicit probability.
                    ss << prefix << ": " << fromName << (conditionStr.empty() ? "" : " [" + conditionStr + "]") << " -> "
                       << formatProbability(ti->probability) << "   : "
                       << stateName(ti->to);
                } else {
                    // Continuation branches: indented to align under "->".
                    ss << indent;
                    if (i + 1 == uniqueTransitions.size()) {
                        // Last branch: use &REST instead of explicit probability.
                        ss << "&REST : ";
                    } else {
                        // Middle branches: explicit probability.
                        ss << formatProbability(ti->probability) << "   : ";
                    }
                    ss << stateName(ti->to);
                }
                if (!assignsi.empty()) ss << " " << assignsi;
                ss << "\n";
            }
        }
    }

    return ss.str();
}

// ---- formula serialization --------------------------------------------------

string ModelSerializer::serializeFormula(Formula* formula) {
    stringstream ss;
    ss << "FORMULA: ";

    // Coalition — omitted for CTL (no coalition in the original file).
    if (!formula->isCTL && !formula->coalition.empty()) {
        ss << "<<";
        bool first = true;
        for (auto agent : formula->coalition) {
            if (!first) ss << ", ";
            ss << agent->name;
            first = false;
        }
        ss << ">>";
    }

    // Temporal modality: [] (G) or <> (F).
    ss << (formula->isF ? "<>" : "[]");

    // Optional probability wrapper: &P[sign value].
    if (formula->probabilitySign != ProbabilitySign::NONE) {
        string signStr;
        switch (formula->probabilitySign) {
            case ProbabilitySign::EQ: signStr = "=="; break;
            case ProbabilitySign::NE: signStr = "!="; break;
            case ProbabilitySign::GT: signStr = ">";  break;
            case ProbabilitySign::GE: signStr = ">="; break;
            case ProbabilitySign::LT: signStr = "<";  break;
            case ProbabilitySign::LE: signStr = "<="; break;
            default: break;
        }
        ss << "&P[" << signStr << formula->probability << "]";
    }

    // Formula body from the ExprNode tree.
    if (formula->p != nullptr && !formula->p->empty()) {
        if (formula->p->size() == 1) {
            ss << "(" << (*formula->p)[0]->toString(false) << ")";
        } else {
            // Multiple top-level expressions — join with &&.
            ss << "(";
            for (size_t i = 0; i < formula->p->size(); ++i) {
                if (i > 0) ss << " && ";
                ss << (*formula->p)[i]->toString(false);
            }
            ss << ")";
        }
    }

    return ss.str();
}

// ---- public interface -------------------------------------------------------

void ModelSerializer::print(LocalModels* localModels, Formula* formula) {
    for (size_t i = 0; i < localModels->agents.size(); ++i) {
        if (i > 0) cout << "\n";
        cout << serializeAgent(localModels->agents[i]);
    }
    cout << "\n" << serializeFormula(formula) << "\n";
}
