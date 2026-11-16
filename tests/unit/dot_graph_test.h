/**
 * @file dot_graph_test.h
 * @brief Unit tests for DotGraph (graphviz export).
 */

#include "helpers.h"
#include "DotGraph.hpp"

class DotGraphTest : public UnitTest {
protected:
    ModelArena arena;
    Agent* a = arena.agent(0, "A");
    Agent* b = arena.agent(1, "B");
};

TEST_F(DotGraphTest, DefaultGraphIsEmpty) {
    DotGraph graph;
    EXPECT_EQ(graph.graphName, "");
    EXPECT_TRUE(graph.nodes.empty());
    EXPECT_TRUE(graph.edges.empty());
}

TEST_F(DotGraphTest, LocalModelNodesAndEdges) {
    LocalState* s0 = arena.state(a, "s0", {{"x", 0}});
    LocalState* s1 = arena.state(a, "s1", {{"x", 1}});
    arena.transition(s0, s1, "[0]_go");
    arena.transition(s1, s0, "sync", "", true, 2);

    DotGraph graph(a);
    EXPECT_EQ(graph.graphName, "A");
    EXPECT_EQ(graph.graphBase, DotGraphBase::LOCAL_MODEL);
    EXPECT_EQ(graph.nodes, (vector<string>{"0[label=\"s0\"]", "1[label=\"s1\"]"}));
    EXPECT_EQ(graph.edges, (vector<string>{"0->1[label=\"[0]_go\"]", "1->0[label=\"sync\", color=\"blue\"]"}));
}

TEST_F(DotGraphTest, ExtendedLocalModelUsesRecordLabelsWithVariables) {
    arena.state(a, "s0", {{"x", 0}, {"y", 1}});
    DotGraph graph(a, true);
    EXPECT_EQ(graph.nodes, (vector<string>{"0[label=\"{{s0|0}|x=0\\ny=1}\", shape=\"record\"]"}));
}

TEST_F(DotGraphTest, GlobalModelNodesAndEdges) {
    LocalState* s0 = arena.state(a, "s0");
    LocalState* s1 = arena.state(a, "s1");
    LocalState* t0 = arena.state(b, "t0");
    GlobalModel model;
    model.agents = {a, b};
    GlobalState* g0 = arena.globalState({s0, t0});
    GlobalState* g1 = arena.globalState({s1, t0});
    model.globalStates = {g0, g1};
    arena.globalTransition(g0, g1, {arena.transition(s0, s1, "[0]_go")});
    arena.globalTransition(g1, g0, {arena.transition(s1, s0, "sync", "", true, 2), arena.transition(t0, t0, "sync", "", true, 2)});
    arena.globalTransition(g1, nullptr, {arena.transition(s1, s1, "[0]_pending")});  // not expanded yet: no edge

    DotGraph graph(&model);
    EXPECT_EQ(graph.graphName, "A||B");
    EXPECT_EQ(graph.graphBase, DotGraphBase::GLOBAL_MODEL);
    EXPECT_EQ(graph.nodes, (vector<string>{"\"0;0;\"[label=\"s0,t0\"]", "\"1;0;\"[label=\"s1,t0\"]"}));
    ASSERT_EQ(graph.edges.size(), 2u);
    EXPECT_EQ(graph.edges[0], "\"0;0;\"->\"1;0;\"[label=\"[0]_go\"]");
    EXPECT_EQ(graph.edges[1], "\"1;0;\"->\"0;0;\"[label=\"sync,sync\", color=\"blue\"]");
}

TEST_F(DotGraphTest, GlobalModelShowsProbabilitiesAndExtendedLabels) {
    LocalState* s0 = arena.state(a, "s", {{"x", 1}});
    LocalState* t0 = arena.state(b, "t");
    GlobalModel model;
    model.agents = {a, b};
    GlobalState* g0 = arena.globalState({s0, t0});
    g0->probability = 1.0f;
    GlobalState* g1 = arena.globalState({s0, t0});
    g1->hash = "half";
    g1->probability = 0.5f;
    model.globalStates = {g0, g1};
    arena.globalTransition(g0, g1, {arena.transition(s0, s0, "[0]_flip", "", false, -1, 0.5f)});

    DotGraph graph(&model, true);
    ASSERT_EQ(graph.nodes.size(), 2u);
    EXPECT_EQ(graph.nodes[0], "\"0;0;\"[label=\"{{s,t|0;0;}|{/* lid = 0*/\\nx=1}|{/* lid = 0*/}}\", shape=\"record\"]");
    EXPECT_EQ(graph.nodes[1], "\"half\"[label=\"{{s,t|half}|{/* lid = 0*/\\nx=1}|{/* lid = 0*/}|{(p=0.5)}}\", shape=\"record\"]");
    EXPECT_EQ(graph.edges, (vector<string>{"\"0;0;\"->\"half\"[label=\"[0]_flip,(p=0.5)\"]"}));
}

TEST_F(DotGraphTest, CorrectFlagColoursEdgesBetweenVerifiedStatesRed) {
    LocalState* s0 = arena.state(a, "s0");
    LocalState* s1 = arena.state(a, "s1");
    GlobalModel model;
    model.agents = {a};
    GlobalState* g0 = arena.globalState({s0});
    GlobalState* g1 = arena.globalState({s1});
    model.globalStates = {g0, g1};
    arena.globalTransition(g0, g1, {arena.transition(s0, s1, "go")});
    arena.globalTransition(g1, g0, {arena.transition(s1, s0, "back")});
    g0->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;
    g1->verificationStatus = GlobalStateVerificationStatus::VERIFIED_OK;

    DotGraph correct(&model, false, true);
    EXPECT_EQ(correct.edges[0], "\"0;\"->\"1;\"[label=\"go\", color=\"red\"]");

    g1->verificationStatus = GlobalStateVerificationStatus::VERIFIED_ERR;
    DotGraph partlyCorrect(&model, false, true);
    EXPECT_EQ(partlyCorrect.edges[0], "\"0;\"->\"1;\"[label=\"go\"]");
}

TEST_F(DotGraphTest, SaveToFileWarnsWhenTheFileCannotBeOpened) {
    LocalState* s0 = arena.state(a, "s0");
    arena.transition(s0, s0, "loop");
    testing::internal::CaptureStderr();
    DotGraph(a).saveToFile("/nonexistent/stv/dir", "model-");
    string output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("Warning: could not open '/nonexistent/stv/dir/model-LocalModel_A.dot' for writing"), string::npos) << output;
}

TEST_F(DotGraphTest, SaveToFileUsesGraphTypeInFileName) {
    LocalState* s0 = arena.state(a, "s0");
    arena.transition(s0, s0, "loop");
    string dir = makeTempDir();

    DotGraph(a).saveToFile(dir + "/", "model-");   // trailing slash is accepted
    string path = dir + "/model-LocalModel_A.dot";
    ASSERT_TRUE(fileExists(path));
    string content = readFile(path);
    EXPECT_EQ(content.rfind("digraph \"A\"{\n\tlabel=\"LocalModel of A\"\n", 0), 0u);
    EXPECT_NE(content.find(DotGraph::styleString), string::npos);
    EXPECT_NE(content.find("\t0[label=\"s0\"]\n"), string::npos);
    EXPECT_NE(content.find("\t0->0[label=\"loop\"]\n"), string::npos);
    EXPECT_EQ(content.back(), '}');
    std::remove(path.c_str());

    DotGraph(a).saveToFile(dir, "", "custom.dot");
    EXPECT_TRUE(fileExists(dir + "/custom.dot"));
    std::remove((dir + "/custom.dot").c_str());

    GlobalModel model;
    model.agents = {a};
    model.globalStates = {arena.globalState({s0})};
    DotGraph(&model).saveToFile(dir, "m-");
    EXPECT_TRUE(fileExists(dir + "/m-GlobalModel.dot"));
    std::remove((dir + "/m-GlobalModel.dot").c_str());
    rmdir(dir.c_str());
}

TEST_F(DotGraphTest, SaveToFileSkipsGraphsWithoutName) {
    string dir = makeTempDir();
    DotGraph().saveToFile(dir, "x-", "empty.dot");
    EXPECT_FALSE(fileExists(dir + "/empty.dot"));
    rmdir(dir.c_str());
}
