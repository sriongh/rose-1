#pragma once 

#include "widgets/graph/graph_layout.h"

namespace fuse {

class atsGraphLayoutHandlerInstantiator : layoutHandlerInstantiator {
  public:
  atsGraphLayoutHandlerInstantiator();
};
extern atsGraphLayoutHandlerInstantiator atsGraphLayoutHandlerInstan;
  
  
/**************************************
 *** ATS Graph Visualizer for Sight ***
 **************************************/

class atsGraph : public sight::layout::graph {
  static std::list<atsGraph*> gStack;
  
  std::ostringstream dot;
  std::string indent;
  public:
  atsGraph(sight::properties::iterator props);
  ~atsGraph();
  
  static void* enterATSSubGraph(sight::properties::iterator props);
  static void  exitATSSubGraph(void* obj);
  static void* enterATSGraphEdge(sight::properties::iterator props);
  static void* enterATSGraphAnchor(sight::properties::iterator props);
};

}; // namespace fuse
