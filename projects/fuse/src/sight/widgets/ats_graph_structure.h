#pragma once

#include "VirtualCFGIterator.h"
#include "cfgUtils.h"
#include "CallGraphTraverse.h"
#include "analysis.h"
#include "latticeFull.h"
#include "saveDotAnalysis.h"
#include "sight_structure.h"
extern int divAnalysisDebugLevel;

namespace fuse {
/**************************************
 *** ATS Graph Visualizer for Sight ***
 **************************************/

// Contains information required to visualize an individual Part
class partDotInfo_atsGraph : public partDotInfo {
  public:
  std::list<anchor> anchors; // All the anchors associated with this Part, in order
  partDotInfo_atsGraph(int partID, const std::list<anchor>& anchors) : partDotInfo(partID), anchors(anchors) {}
};
typedef boost::shared_ptr<partDotInfo_atsGraph> partDotInfo_atsGraphPtr;

class Ctxt2PartsMap_atsGraph : public Ctxt2PartsMap {
  public:
  Ctxt2PartsMap_atsGraph(bool crossAnalysisBoundary, Ctxt2PartsMap_GeneratorPtr mgen, Ctxt2PartsMap_Leaf_GeneratorPtr lgen);
  Ctxt2PartsMap_atsGraph(bool crossAnalysisBoundary, const std::list<std::list<PartContextPtr> >& key, PartPtr part,
                         Ctxt2PartsMap_GeneratorPtr mgen, Ctxt2PartsMap_Leaf_GeneratorPtr lgen);
  // Prints out the hierarchical sub-graphs of Parts denoted by this map to the given output stream
  // partInfo: maps Parts to the information required to display them in the dot graph
  // subgraphName: name of the subgraph that contains the current level in the map
  void map2dot(std::ostream& o, std::map<PartPtr, partDotInfoPtr>& partInfo, std::string subgraphName="cluster", std::string indent="") const;
};

class Ctxt2PartsMap_Leaf_atsGraph : public Ctxt2PartsMap_Leaf {
  public:
  // Prints out the hierarchical sub-graphs of Parts denoted by this map to the given output stream
  // partInfo: maps Parts to the information required to display them in the dot graph
  // subgraphName: name of the subgraph that contains the current level in the map
  void map2dot(std::ostream& o, std::map<PartPtr, partDotInfoPtr>& partInfo, std::string subgraphName="cluster", std::string indent="") const;
};

class atsGraph : public sight::structure::graph {
  std::map<PartPtr, partDotInfoPtr> partInfo;
  // Maps each anchor to the Part it is associated with
  std::map<anchor, PartPtr> anchor2Parts;

  std::string graphName;
  std::set<PartPtr> startParts;
  boost::shared_ptr<std::map<PartPtr, std::list<anchor> > > partAnchors;

  bool dirAligned;

  std::list<graphEdge> edges;

  public:
  // startPart(s) - the Parts from which the iteration of the ATS should start. The ATS graph performs a forward
  //    iteration through the ATS so these should be the entry Part(s).
  // partAnchors - maps each Part in the ATS to the anchors that point to blocks associated with it
  // dirAligned - true if the edges between anchors are pointing in the same direction as the ATS flow of control
  //    and false if they point in the opposite direcction
  atsGraph(PartPtr startPart,                    boost::shared_ptr<std::map<PartPtr, std::list<anchor> > > partAnchors, bool dirAligned,                        sight::properties* props=NULL);
  atsGraph(const std::set<PartPtr>& startParts,  boost::shared_ptr<std::map<PartPtr, std::list<anchor> > > partAnchors, bool dirAligned,                        sight::properties* props=NULL);
  atsGraph(const std::list<PartPtr>& startParts, boost::shared_ptr<std::map<PartPtr, std::list<anchor> > > partAnchors, bool dirAligned,                        sight::properties* props=NULL);
  atsGraph(PartPtr startPart,                    boost::shared_ptr<std::map<PartPtr, std::list<anchor> > > partAnchors, bool dirAligned, const attrOp& onoffOp, sight::properties* props=NULL);
  atsGraph(const std::set<PartPtr>& startParts,  boost::shared_ptr<std::map<PartPtr, std::list<anchor> > > partAnchors, bool dirAligned, const attrOp& onoffOp, sight::properties* props=NULL);
  atsGraph(const std::list<PartPtr>& startParts, boost::shared_ptr<std::map<PartPtr, std::list<anchor> > > partAnchors, bool dirAligned, const attrOp& onoffOp, sight::properties* props=NULL);
  ~atsGraph();

  static sight::properties* setProperties(PartPtr startPart,             boost::shared_ptr<std::map<PartPtr, std::list<anchor> > >& partAnchors, bool dirAligned, const attrOp* onoffOp, sight::properties* props);
  static sight::properties* setProperties(const std::set<PartPtr>& startParts, boost::shared_ptr<std::map<PartPtr, std::list<anchor> > >& partAnchors, bool dirAligned, const attrOp* onoffOp, sight::properties* props);

  // Generates and returns the dot graph code for this graph
  void genDotGraph();

  // Add a directed edge from the location of the from anchor to the location of the to anchor
  virtual void addDirEdge(anchor from, anchor to);

  // Add an undirected edge between the location of the a anchor and the location of the b anchor
  virtual void addUndirEdge(anchor a, anchor b);

  // Called to notify this block that a sub-block was started/completed inside of it.
  // Returns true of this notification should be propagated to the blocks
  // that contain this block and false otherwise.
  virtual bool subBlockEnterNotify(block* subBlock) { return false; }
  virtual bool subBlockExitNotify (block* subBlock) { return false; }
}; // class atsGraph

}; // namespace fuse
