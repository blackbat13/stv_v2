/**
 * @file DotGraph.hpp
 * @brief Class for drawing graphs of input models.
 * Class used for making graph files of input models created by parsing the data and adding graphviz syntax.
 */

#ifndef DOT_PARSER
#define DOT_PARSER

#include "Types.hpp"
#include "Utils.hpp"
#include "reader/nodes.hpp"
#include <string>

using namespace std;

class DotGraph{
public:
    std::vector<std::string> nodes;
    std::vector<std::string> edges;
    std::string name;
    std::string caption;
    static string styleString;
    DotGraph();
    DotGraph(shared_ptr<GlobalModel const> gm, bool extended=false);
    DotGraph(shared_ptr<Agent const> ag, bool extended=false);
    DotGraph(shared_ptr<AgentTemplate const> at);
    void saveToFile(std::string filename="");
protected:
    void addNode(std::string id, std::string name);
    void addEdge(std::string src, std::string trg, std::string label);
};

#endif