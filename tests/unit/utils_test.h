/**
 * @file utils_test.h
 * @brief Unit tests for the free functions in Utils.cpp.
 */

#include "helpers.h"

class UtilsTest : public UnitTest {
protected:
    ModelArena arena;
};

TEST_F(UtilsTest, EnvToString) {
    EXPECT_EQ(envToString({}), "");
    EXPECT_EQ(envToString({{"x", 1}}), " (x=1)");
    EXPECT_EQ(envToString({{"b", -2}, {"a", 1}}), " (a=1, b=-2)");
}

TEST_F(UtilsTest, AgentToStringDescribesEveryTransition) {
    Agent* agent = arena.agent(0, "A");
    Var* x = arena.var(agent, "x", true);
    LocalState* s0 = arena.state(agent, "s0", {{"x", 0}});
    LocalState* s1 = arena.state(agent, "s1", {{"x", 1}});
    arena.transition(s0, s1, "[0]_go");
    LocalTransition* sync = arena.transition(s1, s0, "sync", "sync_A", true, 2);
    sync->conditions.insert(arena.condition(x, ConditionOperator::Equals, 1));
    LocalTransition* other = arena.transition(s1, s1, "other", "", true, 3);
    other->localName = "";  // falls back to the global name
    other->conditions.insert(arena.condition(x, ConditionOperator::NotEquals, 0));
    LocalTransition* expr = arena.transition(s0, s0, "[0]_wait");
    expr->conditions.insert(arena.condition(new ExprLt(new ExprIdent("x"), new ExprConst(5))));

    EXPECT_EQ(agentToString(agent),
              "Agent A:\n"
              "init s0\n"
              "[0]_go: s0 id=0 (x=0) -> s1 id=1 (x=1)\n"
              "shared[2] sync[sync_A]: s1 id=1 (x=1) -[x==1]> s0 id=0 (x=0)\n"
              "shared[3] other[other]: s1 id=1 (x=1) -[x!=0]> s1 id=1 (x=1)\n"
              "[0]_wait: s0 id=0 (x=0) -[x < 5]> s0 id=0 (x=0)\n");
}

TEST_F(UtilsTest, LocalModelsToStringJoinsAgents) {
    Agent* a = arena.agent(0, "A");
    arena.state(a, "s");
    Agent* b = arena.agent(1, "B");
    arena.state(b, "t");
    LocalModels models;
    models.agents = {a, b};
    EXPECT_EQ(localModelsToString(&models), "Agent A:\ninit s\n\nAgent B:\ninit t\n\n");
}

TEST_F(UtilsTest, OutputGlobalModelPrintsStatesTransitionsAndClasses) {
    GeneratedModel generated(R"(
Agent A:
LOCAL: [x]
PERSISTENT: [x]
INITIAL: [x:=0]
init s0
go: s0 [x==0] -> s1 [x:=1]

FORMULA: <<A>>[](x==0)
)");
    generated.generator.expandAllStates();
    testing::internal::CaptureStdout();
    outputGlobalModel(generated.model());
    string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("(initial) GlobalState: hash=\"0;\""), string::npos);
    EXPECT_NE(output.find("GlobalState: hash=\"1;\""), string::npos);
    EXPECT_NE(output.find("LocalState 0.0 (A.s0) [x=0]"), string::npos);
    EXPECT_NE(output.find("to GlobalState 1;"), string::npos);
    EXPECT_NE(output.find("LocalTransition 0 (globalName=[0]_go, localName=[0]_go) of Agent 0 (A); not shared; <if x == 0>"), string::npos);
    EXPECT_NE(output.find("Epistemic classes of Agent 0 (A):"), string::npos);
}

TEST_F(UtilsTest, MemoryCapIsPositive) {
    EXPECT_GT(getMemCap(), 0ul);
}

namespace {
/// @brief Converts an SCC partition into a set of sets of state names for easy comparison.
set<set<string>> namesOf(const vector<set<LocalState*>>& components) {
    set<set<string>> result;
    for (const auto& component : components) {
        set<string> names;
        for (auto state : component) {
            names.insert(state->name);
        }
        result.insert(names);
    }
    return result;
}
}

TEST_F(UtilsTest, SccOfAChainIsOneComponentPerState) {
    Agent* agent = arena.agent(0, "A");
    LocalState* a = arena.state(agent, "a");
    LocalState* b = arena.state(agent, "b");
    LocalState* c = arena.state(agent, "c");
    arena.transition(a, b, "ab");
    arena.transition(b, c, "bc");
    auto components = getLocalStatesSCC(agent);
    EXPECT_EQ(components.size(), 3u);
    EXPECT_EQ(namesOf(components), (set<set<string>>{{"a"}, {"b"}, {"c"}}));
}

TEST_F(UtilsTest, SccOfACycleIsOneComponent) {
    Agent* agent = arena.agent(0, "A");
    LocalState* a = arena.state(agent, "a");
    LocalState* b = arena.state(agent, "b");
    LocalState* c = arena.state(agent, "c");
    arena.transition(a, b, "ab");
    arena.transition(b, c, "bc");
    arena.transition(c, a, "ca");
    EXPECT_EQ(namesOf(getLocalStatesSCC(agent)), (set<set<string>>{{"a", "b", "c"}}));
}

TEST_F(UtilsTest, SccSeparatesTwoCyclesJoinedByABridge) {
    // a <-> b -> c <-> d, plus an isolated self-loop e
    Agent* agent = arena.agent(0, "A");
    LocalState* a = arena.state(agent, "a");
    LocalState* b = arena.state(agent, "b");
    LocalState* c = arena.state(agent, "c");
    LocalState* d = arena.state(agent, "d");
    LocalState* e = arena.state(agent, "e");
    arena.transition(a, b, "ab");
    arena.transition(b, a, "ba");
    arena.transition(b, c, "bc");
    arena.transition(c, d, "cd");
    arena.transition(d, c, "dc");
    arena.transition(e, e, "ee");
    auto components = getLocalStatesSCC(agent);
    EXPECT_EQ(namesOf(components), (set<set<string>>{{"a", "b"}, {"c", "d"}, {"e"}}));
    // Tarjan emits a component only after everything it reaches: {c,d} comes before {a,b}.
    EXPECT_EQ(namesOf({components[0]}), (set<set<string>>{{"c", "d"}}));
}

TEST_F(UtilsTest, ContextModelCoversEveryReachableLocalState) {
    ParsedModel parsed = parseModelText(R"(
Agent A:
LOCAL: [x]
PERSISTENT: [x]
INITIAL: [x:=0]
init s0
go: s0 -> s1
back: s1 -> s0
stop: s1 -> s2
loop: s2 -> s2

FORMULA: <<A>>[](x==0)
)");
    Agent* agent = parsed.agent("A");
    testing::internal::CaptureStdout();
    auto context = getContextModel(&parsed.formula, &parsed.models, agent);
    testing::internal::GetCapturedStdout();

    ASSERT_EQ(context.size(), 3u);
    for (auto state : agent->localStates) {
        ASSERT_EQ(context.count(state), 1u) << state->name;
        EXPECT_EQ(context[state].size(), 1u);  // single agent: one global state per local state
    }
}

// ---- configuration loading --------------------------------------------------

TEST_F(UtilsTest, LoadConfigFromFileReadsEveryKeyAndSkipsComments) {
    string path = writeTempFile(
        "; a comment line\n"
        "\n"
        "MODEL_ID=3\n"
        "OUTPUT_LOCAL_MODELS=1\n"
        "OUTPUT_GLOBAL_MODEL=0\n"
        "MODE=2\n"
        "OUTPUT_DOT_FILES=1\n"
        "DOT_DIR=out/dots\n");
    config.output_global_model = true;
    loadConfigFromFile(path);
    std::remove(path.c_str());

    EXPECT_EQ(config.model_id, 3);
    EXPECT_EQ(config.fname, "../examples/svote/Voters2Candidates2.txt");
    EXPECT_TRUE(config.output_local_models);
    EXPECT_FALSE(config.output_global_model);
    EXPECT_EQ(config.stv_mode, 2);
    EXPECT_TRUE(config.output_dot_files);
    EXPECT_EQ(config.dotdir, "out/dots");
}

TEST_F(UtilsTest, LoadConfigFromFileMapsModelIdsToExamples) {
    string path = writeTempFile("MODEL_ID=1\n");
    loadConfigFromFile(path);
    EXPECT_EQ(config.fname, "../examples/trains/Trains2Controller1.txt");
    std::remove(path.c_str());

    path = writeTempFile("MODEL_ID=2\n");
    loadConfigFromFile(path);
    EXPECT_EQ(config.fname, "../examples/ssvr/Selene_Select_Vote_Revoting_1v_1cv_3c_3rev_share.txt");
    std::remove(path.c_str());
}

TEST_F(UtilsTest, LoadConfigFromMissingFileKeepsConfigAndWarns) {
    config.stv_mode = 7;
    testing::internal::CaptureStdout();
    loadConfigFromFile("/nonexistent/stv/config.txt");
    string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(config.stv_mode, 7);
    EXPECT_NE(output.find("Could not open the config file"), string::npos);
}

TEST_F(UtilsTest, LoadConfigFromArgsFileAndMode) {
    ArgvBuilder args({"-f", "model.txt", "-m", "3"});
    loadConfigFromArgs(args.argc(), args.argv());
    EXPECT_EQ(config.fname, "model.txt");
    EXPECT_EQ(config.stv_mode, 3);

    ArgvBuilder longArgs({"--file", "other.txt", "--mode", "1"});
    loadConfigFromArgs(longArgs.argc(), longArgs.argv());
    EXPECT_EQ(config.fname, "other.txt");
    EXPECT_EQ(config.stv_mode, 1);
}

TEST_F(UtilsTest, LoadConfigFromArgsBooleanFlags) {
    ArgvBuilder args({"-OUTPUT_GLOBAL_MODEL", "--OUTPUT_LOCAL_MODELS", "-OUTPUT_DOT_FILES", "-ADD_EPSILON_TRANSITIONS",
                      "-COUNTEREXAMPLE", "-FIXPOINT", "-NATURAL_STRATEGY", "-SERIALIZE_MODEL",
                      "-RECOMMEND_REDUCTION_VARIABLES", "-MERGE_COALITION", "-ADD_LOCAL_EPSILON_TRANSITIONS"});
    loadConfigFromArgs(args.argc(), args.argv());
    EXPECT_TRUE(config.output_global_model);
    EXPECT_TRUE(config.output_local_models);
    EXPECT_TRUE(config.output_dot_files);
    EXPECT_TRUE(config.add_epsilon_transitions);
    EXPECT_TRUE(config.counterexample);
    EXPECT_TRUE(config.fixpoint);
    EXPECT_TRUE(config.natural_strategy);
    EXPECT_TRUE(config.serialize_model);
    EXPECT_TRUE(config.recommend_reduction_variables);
    EXPECT_TRUE(config.merge_coalition);
    EXPECT_TRUE(config.add_local_epsilon_transitions);
}

TEST_F(UtilsTest, LoadConfigFromArgsFlagsWithValues) {
    ArgvBuilder args({"-OVERWRITE_FORMULA", "<<A>>[](x==1)", "-STRATEGY_FROM_FILE", "strategy.txt",
                      "-CONE_OF_INFLUENCE", "2", "-PARTIAL_REDUCTION_AGENT", "Voter1", "-REDUCE", "x,y"});
    loadConfigFromArgs(args.argc(), args.argv());
    EXPECT_TRUE(config.formula_from_parameter);
    EXPECT_EQ(config.formula, "<<A>>[](x==1)");
    EXPECT_TRUE(config.verify_strategy);
    EXPECT_EQ(config.strategy_file_path, "strategy.txt");
    EXPECT_TRUE(config.cone_of_influence);
    EXPECT_EQ(config.cone_radius, 2);
    EXPECT_EQ(config.partial_reduction_agent, "Voter1");
    EXPECT_TRUE(config.reduce);
    EXPECT_FALSE(config.reduce_all);
    EXPECT_EQ(config.reduce_args, "x,y");

    ArgvBuilder reduceAll({"-REDUCE_ALL", "z"});
    loadConfigFromArgs(reduceAll.argc(), reduceAll.argv());
    EXPECT_TRUE(config.reduce_all);
    EXPECT_EQ(config.reduce_args, "z");
}

TEST_F(UtilsTest, LoadConfigFromArgsPartialReductionStopsAtNextFlag) {
    ArgvBuilder args({"-PARTIAL_REDUCTION", "a", "b", "-m", "3"});
    loadConfigFromArgs(args.argc(), args.argv());
    EXPECT_TRUE(config.partial_reduction);
    EXPECT_EQ(config.partial_reduction_args, (vector<string>{"a", "b"}));
    EXPECT_EQ(config.stv_mode, 3);
}

TEST_F(UtilsTest, LoadConfigFromArgsMissingValueKeepsDefaultAndWarns) {
    config.fname = "unchanged";
    ArgvBuilder args({"-f"});
    testing::internal::CaptureStdout();
    loadConfigFromArgs(args.argc(), args.argv());
    string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(config.fname, "unchanged");
    EXPECT_NE(output.find("no filename was specified"), string::npos);
}
