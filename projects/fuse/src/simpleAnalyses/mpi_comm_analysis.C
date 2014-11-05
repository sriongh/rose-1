/****************************************
 * Author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "sage3basic.h"
#include "mpi_comm_analysis.h"

using namespace std;
using namespace sight;
using namespace boost;

namespace fuse {
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
    const MPICommATSPartPtr mcatsp_p = dynamicConstPtrCast<MPICommATSPart>(that);
    assert(0);
    return parent->equal(mcatsp_p->parent);
  }

  bool MPICommATSPart::less(const PartPtr& that) const {
    const MPICommATSPartPtr mcatsp_p = dynamicConstPtrCast<MPICommATSPart>(that);
    assert(0);
    return parent->less(mcatsp_p->parent);
  }

  string MPICommATSPart::str(string indent) const {
    return "[MPICommATSPart]";
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
    const MPICommATSPartEdgePtr mcatspe_p = dynamicConstPtrCast<MPICommATSPartEdge>(that);
    assert(0);
    return parent->equal(mcatspe_p->parent);
  }

  bool MPICommATSPartEdge::less(const PartEdgePtr& that) const {
    const MPICommATSPartEdgePtr mcatspe_p = dynamicConstPtrCast<MPICommATSPartEdge>(that);
    assert(0);
    return parent->less(mcatspe_p->parent);
  }

  string MPICommATSPartEdge::str(string indent) const {
    return "[MPICommATSPartEdge]";
  }
    
 
  /*******************
   * MPICommAnalysis *
   *******************/
  void MPICommAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
  }

  bool MPICommAnalysis::transfer(PartPtr part, CFGNode cn, NodeState& state, 
                                 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
  }

  std::set<PartPtr> MPICommAnalysis::GetStartAStates_Spec() {
    assert(0);
    return composer->GetStartAStates(this);
  }

  std::set<PartPtr> MPICommAnalysis::GetEndAStates_Spec() {
    assert(0);
    return composer->GetEndAStates(this);
  }
      
}; // end namespace
