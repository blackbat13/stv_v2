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

enum DotGraphBase {
    LOCAL_MODEL, ///< (unfolded) local model = TS with states and transitions
    GLOBAL_MODEL, ///< (unfolded) global model = TS with states and transitions
    AGENT_TEMPLATE, ///< (folded/compact) agent graph = sets of locations and labelled edges
};

class DotGraph{
public:
    std::vector<std::string> nodes;
    std::vector<std::string> edges;
    std::string graphName;
    DotGraphBase graphBase;
    static string styleString;
    DotGraph();
    DotGraph(GlobalModel *const gm, bool extended=false);
    DotGraph(Agent *const ag, bool extended=false);
    DotGraph(AgentTemplate *const at);
    void saveToFile(std::string pathprefix, std::string basename="");
protected:
    void addNode(std::string id, std::string name);
    void addEdge(std::string src, std::string trg, std::string label);
};

#endif