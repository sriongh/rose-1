#define ANALYSIS_C
#include "sage3basic.h"

using namespace std;

#include "VirtualCFGIterator.h"
#include "cfgUtils.h"
#include "CallGraphTraverse.h"
#include "analysis.h"
#include "latticeFull.h"
#include "compose.h"
#include "abstract_object.h"
#include "graphIterator.h"
#include <vector>
#include <set>
#include <map>
#include <boost/make_shared.hpp>
#include "sight.h"

#ifndef DISABLE_SIGHT
using namespace sight;
#endif

/* GB 2012-10-23: DESIGN NOTE
 * At the start of an intra-procedural analysis of a given function the function's initial dataflow state is copied
 * from the special NodeState from inside the function's FunctionState object to the function's starting/ending Part.
 * To make our intra analyses simple we want to analyze each Part by propagating information from before it to after it
 * (fw: from incoming to outgoing edges; bw: from outgoing to incoming edges). This means that we need to ensure that
 * it is possible to create Lattices on the incoming edge of the starting Part and the outgoing edge of the ending Part
 * of a function. This is problematic because intra analyses are run upto the starting/ending Part but do not propagate
 * information to the other side (that would mean effectively falling off the edge of the function). This makes it
 * impossible to create Lattices on these "other-side" edges. We overcome this problem in
 * ContextInsensitiveInterProceduralDataflow (and should do the same in other analyses) by propagating the information
 * on the outgoing edges of the starting Parts / incoming edges of the ending Part one extra step to the incoming
 * edges of the starting Parts / outgoing edges of the ending Parts.
 */

namespace fuse {

#define analysisDebugLevel 0

/************************
 ***** AnaysisParts *****
 ************************/

// Returns the list of this Part's outgoing edges, focusing on all of the edge types
AnalysisPartEdgeLists AnalysisParts::outEdgesAll(NodeState2AllMapper& mapper) {
  assert(NodeState_valid_);
  return mapper.NodeState2All(NodeState()->outEdges());
}

AnalysisPartEdgeLists AnalysisParts::inEdgesAll(NodeState2AllMapper& mapper) {
  assert(NodeState_valid_);
  return mapper.NodeState2All(NodeState()->inEdges());
}

// Returns the list of this Part's outgoing edges, focusing on index and input edge types
AnalysisPartEdgeLists AnalysisParts::outEdgesIndexInput(NodeState2AllMapper& mapper) {
  assert(input_valid_);
  return mapper.Input2Index(input()->outEdges());
}

AnalysisPartEdgeLists AnalysisParts::inEdgesIndexInput(NodeState2AllMapper& mapper) {
  assert(input_valid_);
  return mapper.Input2Index(input()->outEdges());
}

// Returns a wildcard edge that denotes an incoming edge from any source Part, focusing on all of the edge types
AnalysisATS<PartEdgePtr> AnalysisParts::inEdgeFromAnyAll(NodeState2AllMapper& mapper) {
  assert(NodeState_valid_);
  return mapper.NodeState2All(NodeState()->inEdgeFromAny());
}

// Returns a wildcard edge that denotes an outgoing edge to any target Part, focusing on all or a subset of the edge types
AnalysisATS<PartEdgePtr> AnalysisParts::outEdgeToAnyAll(NodeState2AllMapper& mapper) {
  assert(NodeState_valid_);
  return mapper.NodeState2All(NodeState()->outEdgeToAny());
}

// Returns a wildcard edge that denotes an incoming edge from any source Part, focusing on all or a subset of the edge types
AnalysisATS<PartEdgePtr> AnalysisParts::inEdgeFromAnyIndexInput(NodeState2AllMapper& mapper) {
  assert(input_valid_);
  return mapper.Input2Index(NodeState()->inEdgeFromAny());
}

// Returns a wildcard edge that denotes an outgoing edge to any target Part, focusing on all or a subset of the edge types
AnalysisATS<PartEdgePtr> AnalysisParts::outEdgeToAnyIndexInput(NodeState2AllMapper& mapper) {
  assert(input_valid_);
  return mapper.Input2Index(NodeState()->outEdgeToAny());
}

/****************************
 ***** AnaysisPartEdges *****
 ****************************/

/*******************************
 ***** NodeState2AllMapper *****
 *******************************/

// Given a set of Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartSets object that contains all the sets of Parts relevant for analysis.
AnalysisPartSets NodeState2AllMapper::NodeState2All(const set<PartPtr>& parts, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
  AnalysisPartSets ret;
  for(set<PartPtr>::const_iterator p=parts.begin(); p!=parts.end(); ++p)
    ret.insert(NodeState2All(*p, NodeStates_valid, indexes_valid, inputs_valid));
  return ret;
}

// Given a set of PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartEdgeSets object that contains all the sets of PartEdges relevant for analysis.
AnalysisPartEdgeSets NodeState2AllMapper::NodeState2All(const set<PartEdgePtr>& pedges, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
  AnalysisPartEdgeSets ret;
  for(set<PartEdgePtr>::const_iterator edge=pedges.begin(); edge!=pedges.end(); ++edge)
    ret.insert(NodeState2All(*edge, NodeStates_valid, indexes_valid, inputs_valid));
  return ret;
}

// Given a list of Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartlists object that contains all the lists of Parts relevant for analysis.
AnalysisPartLists NodeState2AllMapper::NodeState2All(const list<PartPtr>& parts, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
  AnalysisPartLists ret;
  for(list<PartPtr>::const_iterator p=parts.begin(); p!=parts.end(); ++p)
    ret.push_back(NodeState2All(*p, NodeStates_valid, indexes_valid, inputs_valid));
  return ret;
}

// Given a list of PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartEdgelists object that contains all the lists of PartEdges relevant for analysis.
AnalysisPartEdgeLists NodeState2AllMapper::NodeState2All(const list<PartEdgePtr>& pedges, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
  AnalysisPartEdgeLists ret;
  for(list<PartEdgePtr>::const_iterator edge=pedges.begin(); edge!=pedges.end(); ++edge)
    ret.push_back(NodeState2All(*edge, NodeStates_valid, indexes_valid, inputs_valid));
  return ret;
}

// Given a set of Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartSets object that contains all the sets of Parts relevant for analysis.
AnalysisPartSets NodeState2AllMapper::Input2Index(const set<PartPtr>& parts, bool indexes_valid, bool inputs_valid) {
  AnalysisPartSets ret;
  for(set<PartPtr>::const_iterator p=parts.begin(); p!=parts.end(); ++p)
    ret.insert(Input2Index(*p, indexes_valid, inputs_valid));
  return ret;
}

// Given a set of PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartEdgeSets object that contains all the sets of PartEdges relevant for analysis.
AnalysisPartEdgeSets NodeState2AllMapper::Input2Index(const set<PartEdgePtr>& pedges, bool indexes_valid, bool inputs_valid) {
  AnalysisPartEdgeSets ret;
  for(set<PartEdgePtr>::const_iterator edge=pedges.begin(); edge!=pedges.end(); ++edge)
    ret.insert(Input2Index(*edge, indexes_valid, inputs_valid));
  return ret;
}

// Given a list of Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartlists object that contains all the lists of Parts relevant for analysis.
AnalysisPartLists NodeState2AllMapper::Input2Index(const list<PartPtr>& parts, bool indexes_valid, bool inputs_valid) {
  AnalysisPartLists ret;
  for(list<PartPtr>::const_iterator p=parts.begin(); p!=parts.end(); ++p)
    ret.push_back(Input2Index(*p, indexes_valid, inputs_valid));
  return ret;
}

// Given a list of PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartEdgelists object that contains all the lists of PartEdges relevant for analysis.
AnalysisPartEdgeLists NodeState2AllMapper::Input2Index(const list<PartEdgePtr>& pedges, bool indexes_valid, bool inputs_valid) {
  AnalysisPartEdgeLists ret;
  for(list<PartEdgePtr>::const_iterator edge=pedges.begin(); edge!=pedges.end(); ++edge)
    ret.push_back(Input2Index(*edge, indexes_valid, inputs_valid));
  return ret;
}

/***************************
 ***** AnaysisPartSets *****
 *************************** /

// Returns a human-readable string representation of this object
std::string AnaysisPartSets::str() const {
  ostringstream oss;

  oss << "[nodeStateParts="<<endl;
  for(set<PartPtr>::const_iterator p=nodeStateParts.begin(); p!=nodeStateParts.end(); ++p)
    oss << "  "<<(*p)->str()<<endl;

  oss << "indexParts="<<endl;
  for(set<PartPtr>::const_iterator p=indexParts.begin(); p!=indexParts.end(); ++p)
    oss << "  "<<(*p)->str()<<endl;

  oss << "inputParts="<<endl;
  for(set<PartPtr>::const_iterator p=inputParts.begin(); p!=inputParts.end(); ++p)
    oss << "  "<<(*p)->str()<<endl;
  oss << "]";

  return oss.str();
}

// Returns the set that contains the superset Parts of the Parts in partsSet
set<PartPtr> AnaysisPartSets::getSupersets(const set<PartPtr>& partsSet) {
  set<PartPtr> s;
  for(set<PartPtr>::iterator p=partsSet.begin(); p!=partsSet.end(); ++p) {
    s.insert((*p)->getSupersetPart());
  }
  return s;
}*/

/*******************************
 ***** AnaysisPartEdgeSets *****
 ******************************* /

// Returns a human-readable string representation of this object
std::string AnaysisPartEdgeSets::str() const {
  ostringstream oss;

  oss << "[nodeStatePartEdges="<<endl;
  for(set<PartEdgePtr>::const_iterator p=nodeStatePartEdges.begin(); p!=nodeStatePartEdges.end(); ++p)
    oss << "  "<<(*p)->str()<<endl;

  oss << "indexPartEdges="<<endl;
  for(set<PartEdgePtr>::const_iterator p=indexPartEdges.begin(); p!=indexPartEdges.end(); ++p)
    oss << "  "<<(*p)->str()<<endl;

  oss << "inputPartEdges="<<endl;
  for(set<PartEdgePtr>::const_iterator p=inputPartEdges.begin(); p!=inputPartEdges.end(); ++p)
    oss << "  "<<(*p)->str()<<endl;
  oss << "]";

  return oss.str();
}

// Returns the set that contains the superset PartEdges of the PartEdges in partEdgesSet
set<PartEdgePtr> AnaysisPartEdgeSets::getSupersets(const set<PartEdgePtr>& partEdgesSet) {
  set<PartEdgePtr> s;
  for(set<PartEdgePtr>::iterator p=partEdgesSet.begin(); p!=partEdgesSet.end(); ++p) {
    s.insert((*p)->getSupersetPartEdge());
  }
  return s;
}*/

/****************
 *** Analysis ***
 ****************/

void Analysis::runAnalysis() {
  /* GB: For some reason the compiler complains that SyntacticAnalysis doesn't implement this
         when it is implemented in its ancestor Dataflow. As such,
         a dummy implementation is provided here. */
  assert(0);
}

Analysis::~Analysis() {}

/************************************
 ***** UnstructuredPassAnalysis *****
 ************************************/

// Runs the intra-procedural analysis on the given function, returns true if
// the function's NodeState gets modified as a result and false otherwise
// state - the function's NodeState
void UnstructuredPassAnalysis::runAnalysis()
{
  SIGHT_VERB(dbg << "UnstructuredPassAnalysis::runAnalysis()"<<endl, 2, analysisDebugLevel)

  // Iterate over all the nodes in this function

  for(fw_graphEdgeIterator<PartEdgePtr, PartPtr> it(analysis->getComposer()->GetStartAStates(analysis),
                                                    /*incrementalGraph*/ false);
//          it!=fw_graphEdgeIterator<PartEdgePtr, PartPtr>::end();
          !it.isEnd();
          it++)
  {
    PartPtr p = it.getPart();
    NodeState* state = NodeState::getNodeState(analysis, p);
    visit(p, *state);
  }
}

} // namespace fuse;
