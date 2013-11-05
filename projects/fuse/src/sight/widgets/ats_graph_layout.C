#include "sage3basic.h"
#include "ats_graph_layout.h"
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>
#include "sight_layout.h"

using namespace std;
using namespace sight;
using namespace sight::layout;

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

atsGraph::atsGraph(properties::iterator props) : graph(properties::next(props)) {
  dot << "digraph atsGraph {"<<endl;
  dot << "  compound=true;"<<endl;
  dot << "  rankdir="<<(properties::getInt(props, "dirAligned")? "TD": "DT")<<";"<<endl;
  
  indent += "    ";
}

atsGraph::~atsGraph() {
  dot << "}"<<endl;
  outputCanvizDotGraph(dot.str());
}

void* atsGraph::enterATSSubGraph(properties::iterator props) {
  assert(gStack.size()>0);
  assert(dynamic_cast<atsGraph*>(gStack.back()));
  ostringstream& dot = dynamic_cast<atsGraph*>(gStack.back())->dot;
  string& indent = dynamic_cast<atsGraph*>(gStack.back())->indent;
  
  dot << indent << "subgraph "<<properties::get(props, "name")<<" {"<<endl;
  dot << indent << "  color="<<(properties::getInt(props, "crossAnalysisBoundary")?"red":"black")<<";"<<endl;
  
  if(properties::get(props, "SgType")=="context")
    dot << indent << "  fillcolor=lightgrey;"<<endl;
  else if(properties::get(props, "SgType")=="context")
    dot << indent << "  fillcolor=lightsteelblue;"<<endl;
  
  if(properties::get(props, "SgType")=="callEdges")
    dot << indent << "  style=invis;"<<endl;
  else
    dot << indent << "  style=filled;"<<endl;
  
  if(properties::exists(props, "label"))
    dot << indent << "  label = \""<<properties::get(props, "label")<<"\";"<<endl;
  
  if(properties::exists(props, "source"))
    dot << indent << "      { rank=source; "<<properties::get(props, "source")<<" }"<<endl;
  
  if(properties::exists(props, "sink"))
    dot << indent << "      { rank=sink; "<<properties::get(props, "sink")<<" }"<<endl;
  
  if(properties::exists(props, "same"))
    dot << indent << "      { rank=same; "<<properties::get(props, "same")<<" }"<<endl;

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
               "[label=\""<<properties::get(props, "label")<<"\", "<<//\"a"<<a.getID()<<"\", "<<
                "color=\"" << nodeColor << "\", "<<
                "fillcolor=\"white\", "<<
                "style=\"" << nodeStyle << "\", "<<
                "shape=\"" << nodeShape << "\", "<<
                "href=\"javascript:"<<a.getLinkJS()<<"\"];\n";
  return NULL;
}

}; // namespace fuse
