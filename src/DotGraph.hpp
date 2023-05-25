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
    DotGraph(GlobalModel *const gm, bool extended=false);
    DotGraph(Agent *const ag, bool extended=false);
    DotGraph(AgentTemplate *const at);
    void saveToFile(std::string filename="");
protected:
    void addNode(std::string id, std::string name);
    void addEdge(std::string src, std::string trg, std::string label);
};

#endif