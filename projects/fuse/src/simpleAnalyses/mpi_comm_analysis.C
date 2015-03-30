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

  string MPIOpAbsCallSite::str(string indent) const {
    return "[MPIOpAbsCallSite]";
  }

  MPIOpAbsPtr createMPIOpAbs(const Function& mpif, PartPtr callsite) {
    return makePtr<MPIOpAbsCallSite>(mpif, callsite);
  }

  /******************
   * MPICommContext *
   ******************/
  MPICommContext::MPICommContext(MPIOpAbsPtr mpiopabs_p, PartContextPtr calleeContext_p)
  : CommContext(), mpiopabs_p(mpiopabs_p), calleeContext_p(calleeContext_p) { }

  MPICommContext::MPICommContext(const MPICommContext& that)
  : CommContext(that), mpiopabs_p(that.mpiopabs_p), calleeContext_p(that.calleeContext_p) { }

  //! Returns a list of PartContextPtr objects that denote more detailed context information about
  //! this PartContext's internal contexts. If there aren't any, the function may just return a list containing
  //! this PartContext itself.
  list<PartContextPtr> MPICommContext::getSubPartContexts() const {
    list<PartContextPtr> listOfMe;
    listOfMe.push_back(makePtr<MPICommContext>(mpiopabs_p, calleeContext_p));
    return listOfMe;
  }

  //!\param that Argument is either CompSharedPtr<MPICommContext> or CompSharedPtr<NonMPICommContext>
  bool MPICommContext::operator<(const PartContextPtr& that) const {
    // Check if that is NonMPICommContextPtr
    // If that is NonMPICommContext then it is ordered before this
    NonMPICommContextPtr thatnmcc_p = dynamicPtrCast<NonMPICommContext, PartContext>(that);
    if(thatnmcc_p) return false;

    MPICommContextPtr thatmcc_p = dynamicPtrCast<MPICommContext, PartContext>(that);
    assert(thatmcc_p);
    // Differentiate two MPICommContext using the MPI operation abstraction
    return mpiopabs_p < thatmcc_p->mpiopabs_p;
  }

  //!\param that Argument is either CompSharedPtr<MPICommContext> or CompSharedPtr<NonMPICommContext>
  bool MPICommContext::operator==(const PartContextPtr& that) const {
    // Check if that is NonMPICommContextPtr
    // If that is NonMPICommContext then they are not equal
    NonMPICommContextPtr thatnmcc_p = dynamicPtrCast<NonMPICommContext, PartContext>(that);
    if(thatnmcc_p) return false;

    MPICommContextPtr thatmcc_p = dynamicPtrCast<MPICommContext, PartContext>(that);
    assert(thatmcc_p);
    // Differentiate two MPICommContext using the MPI operation abstraction
    return mpiopabs_p == thatmcc_p->mpiopabs_p;
  }

  string MPICommContext::str(string indent) const {
    return "[CommContext:MPI]";
  }

  /*********************
   * NonMPICommContext *
   *********************/
  NonMPICommContext::NonMPICommContext(PartContextPtr parentContext_p) 
  : CommContext(), parentContext_p(parentContext_p) { }
  NonMPICommContext::NonMPICommContext(const NonMPICommContext& that)
  : CommContext(that), parentContext_p(that.parentContext_p) { }

  //! Returns a list of PartContextPtr objects that denote more detailed context information about
  //! this PartContext's internal contexts. If there aren't any, the function may just return a list containing
  //! this PartContext itself.
  list<PartContextPtr> NonMPICommContext::getSubPartContexts() const {
    list<PartContextPtr> listOfMe;
    listOfMe.push_back(makePtr<NonMPICommContext>(parentContext_p));
    return listOfMe;
  }

  //!\param that Argument is either CompSharedPtr<MPICommContext> or CompSharedPtr<NonMPICommContext>
  bool NonMPICommContext::operator<(const PartContextPtr& that) const {
    // Check if that argument is MPICommContextPtr
    // If so then return true as NonMPICommContext is ordered before MPICommContext
    MPICommContextPtr thatmcc_p = dynamicPtrCast<MPICommContext, PartContext>(that);
    if(thatmcc_p) return true;

    NonMPICommContextPtr thatnmc_p = dynamicPtrCast<NonMPICommContext, PartContext>(that);
    assert(thatnmc_p);
    return parentContext_p < thatnmc_p->parentContext_p;
  }

  //!\param that Argument is either CompSharedPtr<MPICommContext> or CompSharedPtr<NonMPICommContext>
  bool NonMPICommContext::operator==(const PartContextPtr& that) const {
    // Check if that argument is MPICommContextPtr
    // If so then then they are not equal
    MPICommContextPtr thatmcc_p = dynamicPtrCast<MPICommContext, PartContext>(that);
    if(thatmcc_p) return false;

    NonMPICommContextPtr thatnmc_p = dynamicPtrCast<NonMPICommContext, PartContext>(that);
    assert(thatnmc_p);
    return parentContext_p == thatnmc_p->parentContext_p;
  }

  string NonMPICommContext::str(string indent) const {
    return "[CommContext: NonMPI]";
  }

  /***************
   * CommATSPart *
   ***************/
  CommATSPart::CommATSPart(PartPtr base, MPICommAnalysis* analysis, CommContextPtr context)
  : Part(analysis, base),
    base_p(base),
    mpicommanalysis(analysis),
    context_p(context) {
  }
  CommATSPart::CommATSPart(const CommATSPart& that)
  : Part(that),
    base_p(that.base_p),
    mpicommanalysis(that.mpicommanalysis),
    context_p(that.context_p) {
  }

  list<PartEdgePtr> CommATSPart::outEdges() {
    // Look up succMap in MPICommAnalysis to find the successor of this CommATSPart
    // Create CommATSPartEdge between this part and all the successors
    assert(0);
    return parent->outEdges();
  }

  list<PartEdgePtr> CommATSPart::inEdges() {
    // Look up predMap in MPICommAnalysis to find the predecessor of this CommATSPart
    // Create CommATSPartEdge between this part and all the successors
    return parent->inEdges();
  }

  set<CFGNode> CommATSPart::CFGNodes() const {
    return base_p->CFGNodes();
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

  /*******************
   * CommATSPartEdge *
   *******************/
  CommATSPartEdge::CommATSPartEdge(PartEdgePtr base, MPICommAnalysis* analysis, CommATSPartPtr source, CommATSPartPtr target)
  : PartEdge(analysis, base),
    base_p(base),
    source_p(source),
    target_p(target) {
  }

  CommATSPartEdge::CommATSPartEdge(const CommATSPartEdge& that)
  : PartEdge(that),
    base_p(that.base_p),
    source_p(that.source_p),
    target_p(that.target_p) {
  }

  PartPtr CommATSPartEdge::source() const {
    return source_p;
  }

  PartPtr CommATSPartEdge::target() const {
    return target_p;
  }

  list<PartEdgePtr> CommATSPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand) {
    list<PartEdgePtr> baseOpPartEdges = base_p->getOperandPartEdge(anchor, operand);
    assert(0);
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

  /**********************
   * CommContextLattice *
   **********************/
  CommContextLattice::CommContextLattice(PartEdgePtr edge_p)
  : Lattice(edge_p), FiniteLattice(edge_p),
    elem(CommContextLattice::NOCONTEXT) {
  }

  CommContextLattice::CommContextLattice(const CommContextLattice& that)
  : Lattice(that.latPEdge), FiniteLattice(that.latPEdge),
    elem(that.elem) {
  }

  CommContextLattice::CommContextLatticeElem
  CommContextLattice::getCCLatElem() const {
    return elem;
  }

  void CommContextLattice::initialize() {
    elem = CommContextLattice::NOCONTEXT;
  }

  bool CommContextLattice::setCCLatElemMPI() {
    elem = CommContextLattice::MPICOMMCONTEXT;
    return true;
  }

  bool CommContextLattice::setCCLatElemNonMPI() {
    elem = CommContextLattice::NONMPICOMMCONTEXT;
    return true;
  }

  bool CommContextLattice::setToFull() {
    if(isFullLat()) return false;
    elem = CommContextLattice::UNKNOWN;
    return true;
  }

  bool CommContextLattice::setToEmpty() {
    if(isEmptyLat()) return false;
    elem = CommContextLattice::NOCONTEXT;
    return true;
  }

  Lattice* CommContextLattice::copy() const {
    return new CommContextLattice(*this);
  }

  void CommContextLattice::copy(Lattice* that) {
    CommContextLattice* ccl = dynamic_cast<CommContextLattice*>(that);
    ROSE_ASSERT(ccl);
    elem = ccl->getCCLatElem();
  }

  bool CommContextLattice::operator==(Lattice* that) {
    CommContextLattice* ccl = dynamic_cast<CommContextLattice*>(that);
    assert(ccl);
    if(elem == ccl->getCCLatElem()) return true;
    return false;
  }

  bool CommContextLattice::isCCLatElemMPI() const {
    if(elem == CommContextLattice::MPICOMMCONTEXT) return true;
    return false;
  }

  bool CommContextLattice::isCCLatElemNonMPI() const {
    if(elem == CommContextLattice::NONMPICOMMCONTEXT) return true;
    return false;
  }

  bool CommContextLattice::isEmptyLat() {
    if(elem == CommContextLattice::NOCONTEXT) return true;
    return false;
  }

  bool CommContextLattice::isFullLat() {
    if(elem == CommContextLattice::UNKNOWN) return true;
    return false;
  }

  bool CommContextLattice::meetUpdate(Lattice* that) {
    CommContextLattice* ccl = dynamic_cast<CommContextLattice*>(that);
    ROSE_ASSERT(ccl);
    // if this is full nothing to update
    if(isFullLat()) return false;
    // this is not full and if that is full set to full
    else if(ccl->isFullLat()) return setToFull();
    // both are not full
    // this is MPI and that is NonMPI
    else if(isCCLatElemMPI()&& ccl->isCCLatElemNonMPI())
      return setToFull();
    // this is NonMPI and that is MPI
    else if(isCCLatElemNonMPI() && ccl->isCCLatElemMPI())
      return setToFull();
    // this is empty
    // if that is not empty update this lattice value
    else if (!ccl->isEmptyLat()) {
      elem = ccl->getCCLatElem();
      return true;
    }
    // both are empty
    return false;
  }

  bool CommContextLattice::setMLValueToFull(MemLocObjectPtr ml_p) {
    return false;
  }

  string CommContextLattice::str(string indent) const {
    ostringstream oss;
    oss << "[CommContextLatticeVal: ";
    switch(elem) {
      case NOCONTEXT: oss << "NOCONTEXT"; break;
      case NONMPICOMMCONTEXT: oss << "NONMPICOMMCONTEXT"; break;
      case MPICOMMCONTEXT: oss << "MPICOMMCONTEXT"; break;
      default: assert(false); break;
    }
    oss << "]";
    return oss.str();
  }

  /*******************
   * MPICommAnalysis *
   *******************/
  void MPICommAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    CommContextLattice* ccl = new CommContextLattice(pedge);
    initLattices.push_back(ccl);
  }

  bool MPICommAnalysis::isMPIFuncCall(SgFunctionCallExp* sgn) {
    assert(sgn);
    Function func(sgn);
    if(func.get_name().getString().find("MPI_",0) == 0) return true;
    return false;
  }

  //! Clone MPI functions giving them context using CommContext
  bool MPICommAnalysis::transfer(PartPtr part, CFGNode cn, NodeState& state, 
                                 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    scope reg("MPICommAnalysis::transfer", scope::medium, attrGE("mpiCommAnalysisDebugLevel", 1));
    bool modified=false;

    // Incoming lattice information
    CommContextLattice* cclat = dynamic_cast<CommContextLattice*>(dfInfo[part->inEdgeFromAny()][0]);
    assert(cclat);

    if(mpiCommAnalysisDebugLevel() >= 2) dbg << cclat->str() << endl;

    // Context for descendants is decided based on this parts position on the
    // function boundaries
    if(part->isOutgoingFuncCall(cn) && isMPIFuncCall(isSgFunctionCallExp(cn.getNode()))) {
      modified = cclat->setCCLatElemMPI();
    }
    else if(part->isFuncExit(cn)) {
      assert(0);
    }
    else {
      // the context for the descendant is same as this Part's context
    }
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

  string MPICommAnalysis::str(string indent) const {
    return "MPICommAnalysis";
  }

}; // end namespace

//  LocalWords:  MPICommAnalysis MPICommATSPartEdge
