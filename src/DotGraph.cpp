#include "DotGraph.hpp"
#include <algorithm>

/// @brief (temporary hard-coded) graphviz visual configuration
std::string DotGraph::styleString = 
    "\tedge[fontsize=\"10\"]\n"         // font size for the edge-labels
    "\tnode [\n"                        // node styles
    "\t\tshape=circle,\n"
    // "\t\tfixedsize=true,\n"
    "\t\twidth=auto,\n"
    "\t\tcolor=\"black\",\n"
    "\t\tfillcolor=\"#eeeeee\",\n"
    "\t\tstyle=\"filled,solid\",\n"
    "\t\tfontsize=8,\n"
    "\t\tfontname=\"Roboto\"\n"
    "\t]\n"                                                               
    "\tfontname=Consolas\n"             // font-family (will be inherited)
    "\tlayout=dot\n";                   // layout engine

/// @brief empty graph
DotGraph::DotGraph(){
    name="";
    caption="";
}

/* 
 * [YK]:
 * three constructors for each supported (graph) format;
 * hard-coded, yet arguable cleaner than juggling lambda functions (or proxy)
 */

/// @brief parses the edge/state templates into nodes/edges
DotGraph::DotGraph(AgentTemplate *const at){
    nodes.clear();
    edges.clear();
    name = at->ident;
    caption = "AgentTemplate of " + at->ident; 
    for (const auto& st : at->localStateTemplates){
        this->addNode(st.first, st.second->name);
        for(const auto* trn: st.second->transitions){
            this->addEdge(
                trn->startState,
                trn->endState, 
                trn->patternName + (trn->shared!=-1 ? "\", color=\"blue" : "")
            );
        }
    }
}

/// @brief parses the local transitions/states templates into nodes/edges
DotGraph::DotGraph(Agent *const ag, bool extended){
    nodes.clear();
    edges.clear();
    name = ag->name;
    caption = "LocalModel of " + ag->name;
    for (const auto& s : ag->localStates){
        if(extended){
            string tmp = "{{"+s->name+"|"+to_string(s->id)+"}|";
            // for(const auto v: s->vars){
            //     tmp+=v.first->name+"="+to_string(v.second)+"\n";
            // }
            for(const auto x: s->environment){
                tmp+=x.first+"="+to_string(x.second)+"\\n";
            }
            tmp.resize(tmp.size()-2);
            tmp+="}\", shape=\"record";
            this->addNode(to_string(s->id), tmp);
        }else{
            this->addNode(to_string(s->id), s->name);
        }
    }
    for (const auto& t : ag->localTransitions) {
        this->addEdge(
            to_string(t->from->id), 
            to_string(t->to->id), 
            t->name + (t->isShared ? "\", color=\"blue" : "")
        );
    }
}

/// @brief parses the transitions/states templates into nodes/edges
DotGraph::DotGraph(GlobalModel *const gm, bool extended){
    nodes.clear();
    edges.clear();
    const std::string sep="||";              // (async) agent names separator
    name = "";
    for (const auto &agt : gm->agents) name += agt->name + sep;
    name.resize(name.size()-sep.size());     // truncate last (extra) sep
    caption = "GlobalModel of " + name;

    for (const auto& s : gm->globalStates){
        std::string stateLabel = "";
        for(const auto& loc: s->localStates){
            stateLabel+=loc->name+",";
        }
        stateLabel.pop_back();  // truncate sep
    
        if(extended){
            stateLabel="{{"+stateLabel+"|"+to_string(s->id)+"}|";
            for(const auto& loc: s->localStates){
                // add var evals
                stateLabel+="{";
                // stateLabel+="{"+loc->agent->name+"|"+ loc->name+"}|{";
                stateLabel+="/* lid = "+to_string(loc->id)+"*/\\n";
                for(const auto x: loc->environment){
                    stateLabel+=x.first+"="+to_string(x.second)+"\\n";
                }
                stateLabel.pop_back();
                stateLabel.pop_back();
                stateLabel+="}|";    
            }
            stateLabel.pop_back();
            stateLabel+="}\", shape=\"record";
        }

        this->addNode(to_string(s->id), stateLabel);
    }
    for (const auto& t : gm->globalTransitions) {
        std::string transitionLabel = "";
        bool isShared = false;
        for(const auto& e: t->localTransitions){
            transitionLabel+=e->name+",";
            isShared=isShared || e->isShared;
        }
        transitionLabel.pop_back();  // truncate sep
        if(!t->from || !t->to)continue;
        this->addEdge(
            to_string(t->from->id), 
            to_string(t->to->id), 
            transitionLabel + (isShared ? "\", color=\"blue" : "")
        );
    }
}

/// @brief creates a node string in graphviz syntax
/// @param id unique internal node identifier
/// @param name displayed node label/name
void DotGraph::addNode(std::string id, std::string name){
    nodes.push_back(id+"[label=\""+name+"\"]");
};

/// @brief creates an edge string in graphviz syntax
/// @param src id of a source node
/// @param trg id of a target node
/// @param label edge label and, possibly, extra attributes
void DotGraph::addEdge(std::string src, std::string trg, std::string label){
    edges.push_back(src+"->"+trg+"[label=\""+label+"\"]");
};

/// @brief creates a `.dot` file
/// @param filename name of file (parent graph name if blank)
void DotGraph::saveToFile(std::string filename /*= ""*/){
    if(name.length()==0 && caption.length()==0)return;
    if(filename.length()==0){
        filename = caption.substr(0, caption.find(" "));
        if(filename!="GlobalModel"){
            filename+="_"+name;
        }
        filename+=".dot";
    }
    ofstream ofs;
    ofs.open(filename);
    ofs << "digraph \"" << name << "\"{\n";
    ofs << "\tlabel=\"" << caption << "\"\n";
    ofs << styleString;
    for(const auto n: this->nodes){
        ofs << "\t" + n + "\n";
    }
    for(const auto e: this->edges){
        ofs << "\t" + e + "\n";
    }
    ofs << "}";
    ofs.close();
};

