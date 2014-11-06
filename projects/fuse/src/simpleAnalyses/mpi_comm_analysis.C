/****************************************
 * Author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "sage3basic.h"
#include "mpi_comm_analysis.h"
#include "latticeFull.h"

using namespace std;
using namespace sight;
using namespace boost;

namespace fuse {
  DEBUG_LEVEL(mpiCommAnalysisDebugLevel, 2);

  /******************
   * MPICommATSPart *
   ******************/
  MPICommATSPart::MPICommATSPart(PartPtr base, MPICommAnalysis* analysis) 
    : Part(analysis, base),
      parent(base), 
      mpicommanalysis(analysis) {
  }

  list<PartEdgePtr> MPICommATSPart::outEdges() {
    assert(0);
    return parent->outEdges();
  }

  list<PartEdgePtr> MPICommATSPart::inEdges() {
    assert(0);
    return parent->inEdges();
  }

  set<CFGNode> MPICommATSPart::CFGNodes() const {
    assert(0);
    return parent->CFGNodes();
  }

  set<PartPtr> MPICommATSPart::matchingCallParts() const {
    assert(0);
    return parent->matchingCallParts();
  }

  PartEdgePtr MPICommATSPart::outEdgeToAny() {
    assert(0);
    return parent->outEdgeToAny();
  }

  PartEdgePtr MPICommATSPart::inEdgeFromAny() {
    assert(0);
    return parent->inEdgeFromAny();
  }

  bool MPICommATSPart::equal(const PartPtr& that) const {
    const MPICommATSPartPtr mcap_p = dynamicConstPtrCast<MPICommATSPart>(that);
    assert(0);
    return parent->equal(mcap_p->parent);
  }

  bool MPICommATSPart::less(const PartPtr& that) const {
    const MPICommATSPartPtr mcap_p = dynamicConstPtrCast<MPICommATSPart>(that);
    // assert(0);
    return parent->less(mcap_p->parent);
  }

  string MPICommATSPart::str(string indent) const {
    ostringstream oss;
    oss << "[MPICommATSPart:" << parent->str() << "]";
    return oss.str();
  }

  /**********************
   * MPICommATSPartEdge *
   **********************/
  MPICommATSPartEdge::MPICommATSPartEdge(MPICommAnalysis* analysis, PartEdgePtr base)
    : PartEdge(analysis, base),
      parent(base),
      mpicommanalysis(analysis) {
  }

  PartPtr MPICommATSPartEdge::source() const {
    assert(0);
    return parent->source();
  }

  PartPtr MPICommATSPartEdge::target() const {
    assert(0);
    return parent->target();
  }

  list<PartEdgePtr> MPICommATSPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand) {
    assert(0);
    return parent->getOperandPartEdge(anchor, operand);
  }

  map<CFGNode, shared_ptr<SgValueExp> > MPICommATSPartEdge::getPredicateValue() {
    assert(0);
    return parent->getPredicateValue();
  }

  bool MPICommATSPartEdge::equal(const PartEdgePtr& that) const {
    const MPICommATSPartEdgePtr mcape_p = dynamicConstPtrCast<MPICommATSPartEdge>(that);
    assert(0);
    return parent->equal(mcape_p->parent);
  }

  bool MPICommATSPartEdge::less(const PartEdgePtr& that) const {
    const MPICommATSPartEdgePtr mcape_p = dynamicConstPtrCast<MPICommATSPartEdge>(that);
    assert(0);
    return parent->less(mcape_p->parent);
  }

  string MPICommATSPartEdge::str(string indent) const {
    return "[MPICommATSPartEdge]";
  }
    
 
  /*******************
   * MPICommAnalysis *
   *******************/
  void MPICommAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    initLattices.push_back(new BoolAndLattice(true, pedge));
  }

  bool MPICommAnalysis::transfer(PartPtr part, CFGNode cn, NodeState& state, 
                                 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    scope reg("MPICommAnalysis::transfer", scope::medium, attrGE("mpiCommAnalysisDebugLevel", 1));
    bool modified=false;

    MPICommATSPartPtr mcap_p = makePtr<MPICommATSPart, PartPtr, MPICommAnalysis*>(part, this);
    if(mpiCommAnalysisDebugLevel() >= 2) {
      dbg << mcap_p->str() << endl;
    }

    pair<Part2MCAPartMap::iterator, bool> retVal = p2mcapMap.insert(Part2MCAPartMapElement(part, mcap_p));
    modified = retVal.second;

    list<PartEdgePtr> inedges = part->inEdges();
    list<PartEdgePtr> outedges = part->outEdges();
    list<PartEdgePtr>::iterator inEdgeIt, outEdgeIt;

    MCAPartSet& inEdgeSet = predMap[mcap_p];
    for(inEdgeIt=inedges.begin(); inEdgeIt != inedges.end(); ++inEdgeIt) {
      PartPtr src = (*inEdgeIt)->source();
      MPICommATSPartPtr smcap_p = makePtr<MPICommATSPart, PartPtr, MPICommAnalysis*>(src, this);      
      pair<MCAPartSet::iterator, bool> retVal = inEdgeSet.insert(smcap_p);
      modified = retVal.second || modified;
    }

    MCAPartSet& outEdgeSet = succMap[mcap_p];
    for(outEdgeIt=outedges.begin(); outEdgeIt != outedges.end(); ++outEdgeIt) {
      PartPtr tgt = (*outEdgeIt)->target();
      MPICommATSPartPtr tmcap_p = makePtr<MPICommATSPart, PartPtr, MPICommAnalysis*>(tgt, this);
      pair<MCAPartSet::iterator, bool> retVal = outEdgeSet.insert(tmcap_p);
      modified = retVal.second || modified;
    }

    // if(mpiCommAnalysisDebugLevel() >= 2) {
    //   dbg << str(succMap) << endl;
    // }

    return modified;
  }

  std::set<PartPtr> MPICommAnalysis::GetStartAStates_Spec() {
    assert(0);
    return composer->GetStartAStates(this);
  }

  std::set<PartPtr> MPICommAnalysis::GetEndAStates_Spec() {
    assert(0);
    return composer->GetEndAStates(this);
  }

  string MPICommAnalysis::str(const Part2MCAPartMap& p2mcapMap) const {
    ostringstream oss;
    oss << "<u>Part2MCAPartMap: </u>";
    oss << "<table border=1><tr><td>Key</td><td>Value</td>";
    Part2MCAPartMap::const_iterator it = p2mcapMap.begin();
    for(; it != p2mcapMap.end(); ++it) {
      oss << "<tr><td>#"  << (it->first)->str();
      oss << "</td><td>#" << (it->second)->str();
      oss << "</td></tr>";
    }
    oss << "</table>";
    return oss.str();
  }

  string MPICommAnalysis::str(const MCAPart2MCAPartsMap& mcap2mcapMap) const {
    ostringstream oss;
    oss << "<u>MCAPart2MCAPartsMap: </u>";
    oss << "<table border=1><tr><td>Key</td><td>Value</td>";
    MCAPart2MCAPartsMap::const_iterator it = mcap2mcapMap.begin();
    for(; it != mcap2mcapMap.end(); ++it) {
      oss << "<tr><td>#"  << (it->first)->str();
      oss << "</td><td>#" << str(it->second);
      oss << "</td></tr>";
    }
    oss << "</table>";
    return oss.str();
  }

  string MPICommAnalysis::str(const MCAPartSet& mcapSet) const {
    ostringstream oss;
    oss << "<u>MCAPartSet: </u>";
    oss << "<dl>";
    MCAPartSet::const_iterator it = mcapSet.begin();
    for( ; it != mcapSet.end(); ++it) {
      oss << "<dt>" << it->str() << "</dt>";
    }
    oss << "</dl>";
    return oss.str();
  }
      
}; // end namespace

//  LocalWords:  MPICommAnalysis MPICommATSPartEdge
