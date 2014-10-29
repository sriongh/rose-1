#include "sage3basic.h"
#include "ats_graph_layout.h"
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>
#include "sight_common.h"
// #include "sight_layout.h"
// #include "widgets/graph/graph_structure.h"

using namespace std;
using namespace sight;
using namespace sight::layout;
using namespace sight::structure;

namespace fuse {

/**************************************
 *** ATS Graph Visualizer for Sight ***
 **************************************/

// Record the layout handlers in this file
void* atsGraphEnterHandler(properties::iterator props) { return new atsGraph(props); }
void  atsGraphExitHandler(void* obj) { atsGraph* g = static_cast<atsGraph*>(obj); delete g; }

atsGraphLayoutHandlerInstantiator::atsGraphLayoutHandlerInstantiator() {
  (*layoutEnterHandlers)["atsGraph"]       = &atsGraphEnterHandler;
  (*layoutExitHandlers )["atsGraph"]       = &atsGraphExitHandler;
  (*layoutEnterHandlers)["atsSubGraph"]    = &atsGraph::enterATSSubGraph;
  (*layoutExitHandlers )["atsSubGraph"]    = &atsGraph::exitATSSubGraph;
  (*layoutEnterHandlers)["atsGraphEdge"]   = &atsGraph::enterATSGraphEdge;
  (*layoutExitHandlers )["atsGraphEdge"]   = &defaultExitHandler;
  (*layoutEnterHandlers)["atsGraphAnchor"] = &atsGraph::enterATSGraphAnchor;
  (*layoutExitHandlers )["atsGraphAnchor"] = &defaultExitHandler;
}
atsGraphLayoutHandlerInstantiator atsGraphLayoutHandlerInstance;

std::list<atsGraph*> atsGraph::gStack;

atsGraph::atsGraph(properties::iterator props) : graph(properties::next(props)) {
  dot << "digraph atsGraph {"<<endl;
  dot << "  compound=true;"<<endl;
  dot << "  rankdir="<<(properties::getInt(props, "dirAligned")? "TD": "DT")<<";"<<endl;
  
  indent += "    ";

  gStack.push_back(this);
}

atsGraph::~atsGraph() {
  dot << "}"<<endl;
  outputCanvizDotGraph(dot.str());

  assert(gStack.back() == this);
  gStack.pop_back();
}

void* atsGraph::enterATSSubGraph(properties::iterator props) {
  assert(gStack.size()>0);
  assert(dynamic_cast<atsGraph*>(gStack.back()));
  ostringstream& dot = dynamic_cast<atsGraph*>(gStack.back())->dot;
  string& indent = dynamic_cast<atsGraph*>(gStack.back())->indent;
  
  dot << indent << "subgraph "<<props.get("name")<<" {"<<endl;
  dot << indent << "  color="<<(properties::getInt(props, "crossAnalysisBoundary")?"red":"black")<<";"<<endl;
  
  if(props.get("SgType")=="context")
    dot << indent << "  fillcolor=lightgrey;"<<endl;
  else if(props.get("SgType")=="context")
    dot << indent << "  fillcolor=lightsteelblue;"<<endl;
  
  if(props.get("SgType")=="callEdges")
    dot << indent << "  style=invis;"<<endl;
  else
    dot << indent << "  style=filled;"<<endl;
  
  if(props.exists("label"))
    dot << indent << "  label = \""<<props.get("label")<<"\";"<<endl;
  
  if(props.exists("source"))
    dot << indent << "      { rank=source; "<<props.get("source")<<" }"<<endl;
  
  if(props.exists("sink"))
    dot << indent << "      { rank=sink; "<<props.get("sink")<<" }"<<endl;
  
  if(props.exists("same"))
    dot << indent << "      { rank=same; "<<props.get("same")<<" }"<<endl;

  indent += "    ";
  
  return NULL;
}

void atsGraph::exitATSSubGraph(void* obj) {
  assert(gStack.size()>0);
  assert(dynamic_cast<atsGraph*>(gStack.back()));
  ostringstream& dot = dynamic_cast<atsGraph*>(gStack.back())->dot;
  string& indent = dynamic_cast<atsGraph*>(gStack.back())->indent;
  
  indent.erase(indent.length()-4);
  
  dot << indent << "}"<<endl;
}

void* atsGraph::enterATSGraphEdge(properties::iterator props) {
  assert(gStack.size()>0);
  assert(dynamic_cast<atsGraph*>(gStack.back()));
  ostringstream& dot = dynamic_cast<atsGraph*>(gStack.back())->dot;
  string& indent = dynamic_cast<atsGraph*>(gStack.back())->indent;
  
  string style = "solid";
  string color = "black";
  int weight=100;

  // If this edge crosses function boundaries, reduce its weight
  set<CFGNode> matchNodes;
  if(properties::getInt(props, "crossFunc")) weight = 1;
  /*
  // Create anchors for from and to to make sure that we update their anchorID to use the same 
  // ID for all anchors that point to the same location
  anchor fromA(properties::getInt(props, "from"));
  anchor toA  (properties::getInt(props, "to"));
  dot << indent << "a"<<fromA.getID() << " -> a" << toA.getID() << */
  dot << indent << "a"<<properties::getInt(props, "from") << " -> a" << (properties::getInt(props, "to")) << 
         " [style=\"" << style << "\", " << 
         " color=\"" << color << "\", weight="<<weight<<"];\n";

  return NULL;
}

void* atsGraph::enterATSGraphAnchor(properties::iterator props) {
  assert(gStack.size()>0);
  assert(dynamic_cast<atsGraph*>(gStack.back()));
  ostringstream& dot = dynamic_cast<atsGraph*>(gStack.back())->dot;
  string& indent = dynamic_cast<atsGraph*>(gStack.back())->indent;
  
  std::string nodeColor = "black";
  std::string nodeStyle = "solid";
  std::string nodeShape = "box";
  anchor a(properties::getInt(props, "anchorID"));
  dot << indent << "a"<<a.getID()<< " "<<
               "[label=\""<<props.get("label")<<"\", "<<//\"a"<<a.getID()<<"\", "<<
                "color=\"" << nodeColor << "\", "<<
                "fillcolor=\"white\", "<<
                "style=\"" << nodeStyle << "\", "<<
                "shape=\"" << nodeShape << "\", "<<
                "href=\"javascript:"<<a.getLinkJS()<<"\"];\n";
  return NULL;
}

}; // namespace fuse
