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

  /*********
   * MPIOp *
   *********/
  MPIOp::MPIOp(const Function& mpif) {
    string mpifunc = mpif.get_name().getString();
    if(mpifunc.compare("MPI_Send")==0) op = MPIOp::SEND;
    else if(mpifunc.compare("MPI_Recv") == 0) op = MPIOp::RECV;
    else if(mpifunc.compare("MPI_ISend") == 0) op = MPIOp::ISEND;
    else if(mpifunc.compare("MPI_IRecv") == 0) op = MPIOp::IRECV;
    else if(mpifunc.compare("MPI_Barrier") == 0) op = MPIOp::BARRIER;
    else if(mpifunc.find("MPI_", 0) == 0) op = MPIOp::NOOP;
    else {
      dbg << "Unhandled MPI function\n";
      assert(0);
    }
  }

  MPIOp::MPIOp(const MPIOp& that) : op(that.op) { }

  bool MPIOp::operator<(const MPIOp& that) const {
    return op < that.op;
  }

  bool MPIOp::operator==(const MPIOp& that) const {
    return op == that.op;
  }

  /****************
   * MPIOpAbsType *
   ****************/
  // MPIOpAbsType::MPIOpAbsType(const Function& mpif) : MPIOpAbs(*this), op(mpif) { }
  
  // MPIOpAbsType::MPIOpAbsType(const MPIOpAbsType& that) : MPIOpAbs(that), op(that.op) { }

  // bool MPIOpAbsType::operator<(const MPIOpAbsPtr& that_p) const {
  //   MPIOpAbsTypePtr moat_p = boost::dynamic_pointer_cast<MPIOpAbsType>(that_p);
  //   assert(moat_p);
  //   return op < moat_p->op;
  // }

  // bool MPIOpAbsType::operator==(const MPIOpAbsPtr& that_p) const {
  //   MPIOpAbsTypePtr moat_p = boost::dynamic_pointer_cast<MPIOpAbsType>(that_p);
  //   assert(moat_p);
  //   return op == moat_p->op;
  // }

  /********************
   * MPIOpAbsCallSite *
   ********************/

  MPIOpAbsCallSite::MPIOpAbsCallSite(const Function& mpif, PartPtr pCallSite)
    : MPIOpAbs(*this), op(mpif), callsite(pCallSite) { }

  MPIOpAbsCallSite::MPIOpAbsCallSite(const MPIOpAbsCallSite& that)
    : MPIOpAbs(that), op(that.op), callsite(that.callsite) { }

  //! Order the operations by type first
  //! If two operations are of same type 
  //! order them using the callsite pointer (SgFunctionCallExp*)
  bool MPIOpAbsCallSite::operator<(const MPIOpAbsPtr& that_p) const {
    MPIOpAbsCallSitePtr moacs_p = dynamicPtrCast<MPIOpAbsCallSite, MPIOpAbs>(that_p);
    assert(moacs_p);
    if(op < moacs_p->op) return true;
    if(op == moacs_p->op) return callsite < moacs_p->callsite;
    return false;
  }

  bool MPIOpAbsCallSite::operator==(const MPIOpAbsPtr& that_p) const {
    MPIOpAbsCallSitePtr moacs_p = dynamicPtrCast<MPIOpAbsCallSite, MPIOpAbs>(that_p);
    assert(moacs_p);
    if(op == moacs_p->op) return callsite == moacs_p->callsite;
    return false;
  }

  // MPIOpAbsPtr createMPIOpAbs(const Function& mpif) {
  //   return boost::make_shared<MPIOpAbsType>(mpif);
  // }

  MPIOpAbsPtr createMPIOpAbs(const Function& mpif, PartPtr callsite) {
    return make_ptr<MPIOpAbsCallSite>(mpif, callsite);
  }

  /**********************
   * CommATSPartContext *
   **********************/
  //! For two parts that are equal both parts have NonMPIContexts or MPICallContexts.
  //! This is due to association of MPICallContext only with MPI functions.
  //! The cases where two parts are equal and one having a NonMPIContext and
  //! the other having MPICallContext is rare or impossible.
  //! However such a case should arise for two equal parts order its NonMPIContext before its MPICallContext.
  bool CommATSPartContext::less(const PartContextPtr& that) const {
    // If this is MPICallContext and that is NonMPIContext
    if(dynamic_cast<MPICallContext>(this) && 
       dynamicPtrCast<NonMPIContext, PartContext>(that)) return false;
    // If this is NonMPIContext and that is MPICallContext
    else if(dynamic_cast<NonMPIContext>(this) && 
            dynamicPtrCast<MPICallContext, PartContext>(that)) return true;
    // If both are either NonMPIContext or MPICallContext use their respective implementation of less
    else return less(that);
  }

  //! For two parts that are equal both parts have NonMPIContexts or MPICallContexts.
  //! This is due to association of MPICallContext only with MPI functions.
  //! The cases where two parts are equal and one having a NonMPIContext and
  //! the other having MPICallContext is rare or impossible.
  //! However such a case should arise for two equal parts order its NonMPIContext before its MPICallContext.
  bool CommATSPartContext::equals(const PartContextPtr& that) const {
    // If this is MPICallContext and that is NonMPIContext
    if(dynamic_cast<MPICallContext>(this) && 
       dynamicPtrCast<NonMPICallContext, PartContext>(that)) return false;
    // If this is NonMPIContext and that is MPICallContext
    else if(dynamic_cast<NonMPICallContext>(this) && 
            dynamicPtrCast<MPICallContext, PartContext>(that)) return false;
    // If both are either NonMPIContext or MPICallContext use their respective implementation of less
    else return equals(that);
  }

  /******************
   * MPICallContext *
   ******************/
  MPICallContext::MPICallContext(MPIOpAbsPtr mpiopabs_p, PartContextPtr calleeContext_p)
    : CommATSPartContext(), this->mpiopabs_p(mpiopabs_p), this->calleeContext_p(calleeContext_p) { }

  MPICallContext::MPICallContext(const MPICallContext& that)
    : CommATSPartContext(that), this->mpiopabs_p(that.mpiopabs_p), this->calleeContext_p(that.calleeContext_p) { }

  list<PartContextPtr> MPICallContext::getSubPartContexts() const {
    assert(false);
  }

  bool MPICallContext::less(const PartContextPtr& that) const {
    MPICallContextPtr thatmcc_p = dynamicPtrCast<MPICallContext, PartContext>(that);
    assert(thatmcc_p);
    // Differentiate two MPICallContext using the MPI operation abstraction
    return mpiopabs_p < that.mpiopabs_p;
  }
   
  bool MPICallContext::equals(const PartContextPtr& that) const {
    MPICallContextPtr thatmcc_p = dynamicPtrCast<MPICallContext, PartContext>(that);
    assert(thatmcc_p);
    // Differentiate two MPICallContext using the MPI operation abstraction
    return mpiopabs_p == that.mpiopabs_p;
  }

  /*****************
   * NonMPIContext *
   *****************/
  NonMPIContext::NonMPIContext(PartContextPtr parentContext_p) 
    : CommATSPartContext(), this->parentContext_p(parentContext_p) { }
  NonMPIContext::NonMPIContext(const NonMPIContext& that)
    : CommATSPartContext(that), this->parentContext_p(that.parentContext_p) { }

  list<PartContextPtr> NonMPIContext::getSubPartContexts() const {
    assert(false);
  }

  bool NonMPIContext::less(const PartContextPtr& that) const {
    NonMPIContextPtr thatnmc_p = dynamicPtrCast<NonMPIContext, PartContext>(that);
    assert(thatnmc_p);
    return parentContext_p < thatnmc_p.parentContext_p;
  }

  bool NonMPIContext::equals(const PartContextPtr& that) const {
    NonMPIContextPtr thatnmc_p = dynamicPtrCast<NonMPIContext, PartContext>(that);
    assert(thatnmc_p);
    return parentContext_p == thatnmc_p.parentContext_p;
  }

  /******************
   * CommATSPart *
   ******************/
  CommATSPart::CommATSPart(PartPtr base, MPICommAnalysis* analysis) 
    : Part(analysis, base),
      parent(base), 
      mpicommanalysis(analysis) {
  }

  list<PartEdgePtr> CommATSPart::outEdges() {
    // Look up succMap in MPICommAnalysis to find the successor of this CommATSPart
    // Create CommATSPartEdge between this part and all the successors
  }

  list<PartEdgePtr> CommATSPart::inEdges() {
    // Look up predMap in MPICommAnalysis to find the predecessor of this CommATSPart
    // Create CommATSPartEdge between this part and all the successors
    return parent->inEdges();
  }

  set<CFGNode> CommATSPart::CFGNodes() const {
    assert(0);
    return parent->CFGNodes();
  }

  set<PartPtr> CommATSPart::matchingCallParts() const {
    assert(0);
    return parent->matchingCallParts();
  }

  PartEdgePtr CommATSPart::outEdgeToAny() {
    assert(0);
    return parent->outEdgeToAny();
  }

  PartEdgePtr CommATSPart::inEdgeFromAny() {
    assert(0);
    return parent->inEdgeFromAny();
  }

  bool CommATSPart::equal(const PartPtr& that) const {
    const CommATSPartPtr mcap_p = dynamicConstPtrCast<CommATSPart>(that);
    assert(0);
    return parent->equal(mcap_p->parent);
  }

  bool CommATSPart::less(const PartPtr& that) const {
    const CommATSPartPtr mcap_p = dynamicConstPtrCast<CommATSPart>(that);
    // assert(0);
    return parent->less(mcap_p->parent);
  }

  string CommATSPart::str(string indent) const {
    ostringstream oss;
    oss << "[CommATSPart:" << parent->str() << "]";
    return oss.str();
  }

  /**********************
   * CommATSPartEdge *
   **********************/
  CommATSPartEdge::CommATSPartEdge(MPICommAnalysis* analysis, PartEdgePtr base)
    : PartEdge(analysis, base),
      parent(base),
      mpicommanalysis(analysis) {
  }

  PartPtr CommATSPartEdge::source() const {
    assert(0);
    return parent->source();
  }

  PartPtr CommATSPartEdge::target() const {
    assert(0);
    return parent->target();
  }

  list<PartEdgePtr> CommATSPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand) {
    assert(0);
    return parent->getOperandPartEdge(anchor, operand);
  }

  map<CFGNode, shared_ptr<SgValueExp> > CommATSPartEdge::getPredicateValue() {
    assert(0);
    return parent->getPredicateValue();
  }

  bool CommATSPartEdge::equal(const PartEdgePtr& that) const {
    const CommATSPartEdgePtr mcape_p = dynamicConstPtrCast<CommATSPartEdge>(that);
    assert(0);
    return parent->equal(mcape_p->parent);
  }

  bool CommATSPartEdge::less(const PartEdgePtr& that) const {
    const CommATSPartEdgePtr mcape_p = dynamicConstPtrCast<CommATSPartEdge>(that);
    assert(0);
    return parent->less(mcape_p->parent);
  }

  string CommATSPartEdge::str(string indent) const {
    return "[CommATSPartEdge]";
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

    CommATSPartPtr mcap_p = makePtr<CommATSPart, PartPtr, MPICommAnalysis*>(part, this);
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
      CommATSPartPtr smcap_p = makePtr<CommATSPart, PartPtr, MPICommAnalysis*>(src, this);      
      pair<MCAPartSet::iterator, bool> retVal = inEdgeSet.insert(smcap_p);
      modified = retVal.second || modified;
    }

    MCAPartSet& outEdgeSet = succMap[mcap_p];
    for(outEdgeIt=outedges.begin(); outEdgeIt != outedges.end(); ++outEdgeIt) {
      PartPtr tgt = (*outEdgeIt)->target();
      CommATSPartPtr tmcap_p = makePtr<CommATSPart, PartPtr, MPICommAnalysis*>(tgt, this);
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
