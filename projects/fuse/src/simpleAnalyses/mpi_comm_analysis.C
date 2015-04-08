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
  DEBUG_LEVEL(mpiCommAnalysisDebugLevel, 3);

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
    MPIOpAbsCallSitePtr moacs_p = dynamicPtrCast<MPIOpAbsCallSite>(that_p);
    assert(moacs_p);
    if(op < moacs_p->op) return true;
    if(op == moacs_p->op) return callsite < moacs_p->callsite;
    return false;
  }

  bool MPIOpAbsCallSite::operator==(const MPIOpAbsPtr& that_p) const {
    MPIOpAbsCallSitePtr moacs_p = dynamicPtrCast<MPIOpAbsCallSite>(that_p);
    assert(moacs_p);
    if(op == moacs_p->op) return callsite == moacs_p->callsite;
    return false;
  }

  string MPIOpAbsCallSite::str(string indent) const {
    ostringstream oss;
    oss << "[MPIOpAbsCallSite: " << callsite->str() << "]";
    return oss.str();
  }

  MPIOpAbsPtr createMPIOpAbs(PartPtr callsite) {
    // Callsite should be a SgFunctionCallExp
    set<CFGNode> callExpNodes;

    // Not really a requirement that all CFGNodes are SgFunctionCallExp
    // For simplicity assume there is only one CFGNode that corresponds
    // to the MPI SgFunctionCallExp
    ROSE_ASSERT(callsite->mustOutgoingFuncCall(callExpNodes));
    ROSE_ASSERT(callExpNodes.size() == 1);

    SgFunctionCallExp* mpicall = isSgFunctionCallExp(callExpNodes.begin()->getNode());
    Function mpif(mpicall);
    return makePtr<MPIOpAbsCallSite>(mpif, callsite);
  }

  /******************
   * MPICommContext *
   ******************/
  MPICommContext::MPICommContext(MPIOpAbsPtr mpiopabs_p)
  : CommContext(), mpiopabs_p(mpiopabs_p) { }

  MPICommContext::MPICommContext(const MPICommContext& that)
  : CommContext(that), mpiopabs_p(that.mpiopabs_p) { }

  //! Returns a list of PartContextPtr objects that denote more detailed context information about
  //! this PartContext's internal contexts. If there aren't any, the function may just return a list containing
  //! this PartContext itself.
  list<PartContextPtr> MPICommContext::getSubPartContexts() const {
    list<PartContextPtr> listOfMe;
    listOfMe.push_back(makePtr<MPICommContext>(mpiopabs_p));
    return listOfMe;
  }

  //!\param that Argument is either CompSharedPtr<MPICommContext> or CompSharedPtr<NonMPICommContext>
  bool MPICommContext::operator<(const PartContextPtr& that) const {
    // Check if that is NonMPICommContextPtr
    // If that is NonMPICommContext then it is ordered before this
    NonMPICommContextPtr thatnmcc_p = dynamicPtrCast<NonMPICommContext>(that);
    if(thatnmcc_p.get()) return false;

    MPICommContextPtr thatmcc_p = dynamicPtrCast<MPICommContext>(that);
    assert(thatmcc_p.get());
    // Differentiate two MPICommContext using the MPI operation abstraction
    return mpiopabs_p < thatmcc_p->mpiopabs_p;
  }

  //!\param that Argument is either CompSharedPtr<MPICommContext> or CompSharedPtr<NonMPICommContext>
  bool MPICommContext::operator==(const PartContextPtr& that) const {
    // Check if that is NonMPICommContextPtr
    // If that is NonMPICommContext then they are not equal
    NonMPICommContextPtr thatnmcc_p = dynamicPtrCast<NonMPICommContext>(that);
    if(thatnmcc_p.get()) return false;

    MPICommContextPtr thatmcc_p = dynamicPtrCast<MPICommContext>(that);
    assert(thatmcc_p.get());
    // Differentiate two MPICommContext using the MPI operation abstraction
    return mpiopabs_p == thatmcc_p->mpiopabs_p;
  }

  string MPICommContext::str(string indent) const {
    return "MPI";
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
    MPICommContextPtr thatmcc_p = dynamicPtrCast<MPICommContext>(that);
    if(thatmcc_p) return true;

    NonMPICommContextPtr thatnmc_p = dynamicPtrCast<NonMPICommContext>(that);
    assert(thatnmc_p);
    return parentContext_p < thatnmc_p->parentContext_p;
  }

  //!\param that Argument is either CompSharedPtr<MPICommContext> or CompSharedPtr<NonMPICommContext>
  bool NonMPICommContext::operator==(const PartContextPtr& that) const {
    // Check if that argument is MPICommContextPtr
    // If so then then they are not equal
    MPICommContextPtr thatmcc_p = dynamicPtrCast<MPICommContext>(that);
    if(thatmcc_p) return false;

    NonMPICommContextPtr thatnmc_p = dynamicPtrCast<NonMPICommContext>(that);
    assert(thatnmc_p);
    return parentContext_p == thatnmc_p->parentContext_p;
  }

  string NonMPICommContext::str(string indent) const {
    return "NonMPI";
  }

  /***************
   * CommATSPart *
   ***************/
  CommATSPart::CommATSPart(PartPtr base, MPICommAnalysis* analysis, CommContextPtr context)
  : Part(analysis, base),
    base(base),
    mpicommanalysis_p(analysis),
    context(context) {
  }
  CommATSPart::CommATSPart(const CommATSPart& that)
  : Part(that),
    base(that.base),
    mpicommanalysis_p(that.mpicommanalysis_p),
    context(that.context) {
  }

  CommATSPartPtr CommATSPart::get_shared_this() {
    return dynamicPtrCast<CommATSPart>(makePtrFromThis(shared_from_this()));
  }

  CommContextLattice* CommATSPart::getCommContextLatticeAbove(PartPtr part, PartEdgePtr pedge) {
    NodeState* state_p = NodeState::getNodeState(mpicommanalysis_p, part);
    CommContextLattice* ccl_p = dynamic_cast<CommContextLattice*>(state_p->getLatticeAbove(mpicommanalysis_p, pedge, 0));
    ROSE_ASSERT(ccl_p);
    return ccl_p;
  }

  CommContextLattice* CommATSPart::getCommContextLatticeBelow(PartPtr part, PartEdgePtr pedge) {
    NodeState* state_p = NodeState::getNodeState(mpicommanalysis_p, part);
    CommContextLattice* ccl_p = dynamic_cast<CommContextLattice*>(state_p->getLatticeBelow(mpicommanalysis_p, pedge, 0));
    ROSE_ASSERT(ccl_p);
    return ccl_p;
  }

  bool CommATSPart::contextFromNonMPItoNonMPI(CommContextLattice* edgefromccl_p, 
                                              CommContextLattice* edgetoccl_p) {
    if(edgefromccl_p->isCCLatElemNonMPI() && edgetoccl_p->isCCLatElemNonMPI()) return true;
    return false;
  }

  bool CommATSPart::contextFromNonMPItoMPI(CommContextLattice* edgefromccl_p, 
                                           CommContextLattice* edgetoccl_p) {
    if(edgefromccl_p->isCCLatElemNonMPI() && edgetoccl_p->isCCLatElemMPI()) return true;
    return false;
  }

  bool CommATSPart::contextFromMPItoNonMPI(CommContextLattice* edgefromccl_p, 
                                           CommContextLattice* edgetoccl_p) {
    if(edgefromccl_p->isCCLatElemMPI() && edgetoccl_p->isCCLatElemNonMPI()) return true;
    return false;
  }

  bool CommATSPart::contextFromMPItoMPI(CommContextLattice* edgefromccl_p, 
                                        CommContextLattice* edgetoccl_p) {
    if(edgefromccl_p->isCCLatElemMPI() && edgetoccl_p->isCCLatElemMPI()) return true;
    return false;
  }

  list<PartEdgePtr> CommATSPart::outEdges() {
    scope reg("CommATSPart::outEdges()", scope::low, attrGE("mpiCommAnalysisDebugLevel", 3)); 
    list<PartEdgePtr> caOutEdges;

    list<PartEdgePtr> oedges = base->outEdges();
    list<PartEdgePtr>::iterator oe = oedges.begin();
    
    CommContextLattice* edgefromccl_p = getCommContextLatticeAbove(base, base->inEdgeFromAny());
    for( ; oe != oedges.end(); ++oe) {
      PartPtr target = (*oe)->target();
      CommContextLattice* edgetoccl_p = getCommContextLatticeBelow(base, *oe);
      CommContextPtr targetContext;
      if(contextFromNonMPItoNonMPI(edgefromccl_p, edgetoccl_p)) {
        targetContext = makePtr<NonMPICommContext>(base->getPartContext());
      }
      else if(contextFromNonMPItoMPI(edgefromccl_p, edgetoccl_p)) {
        PartPtr calleePart = base;
        MPIOpAbsPtr mpiopabs = createMPIOpAbs(calleePart);
        targetContext = makePtr<MPICommContext>(mpiopabs);
      }
      else assert(0);
      CommATSPartPtr caTargetPart = makePtr<CommATSPart>(target, mpicommanalysis_p, targetContext);
      CommATSPartEdgePtr caOutEdge = makePtr<CommATSPartEdge>(*oe, mpicommanalysis_p, makePtrFromThis(shared_from_this()), caTargetPart);
      dbg << "CommATSPartEdge=" << caOutEdge->str() << endl;
      caOutEdges.push_back(caOutEdge);
    }
    return caOutEdges;
  } 

  list<PartEdgePtr> CommATSPart::inEdges() {
    // Look up predMap in MPICommAnalysis to find the predecessor of this CommATSPart
    // Create CommATSPartEdge between this part and all the successors
    assert(0);
    return base->inEdges();
  }

  // this analysis does not refine any CFGNodes within a part
  set<CFGNode> CommATSPart::CFGNodes() const {
    return base->CFGNodes();
  }

  set<PartPtr> CommATSPart::matchingCallParts() const {
    assert(0);
    return base->matchingCallParts();
  }

  PartEdgePtr CommATSPart::outEdgeToAny() {
    PartEdgePtr baseOutEdgeToAny = base->outEdgeToAny();
    return makePtr<CommATSPartEdge>(baseOutEdgeToAny, mpicommanalysis_p, get_shared_this(), NULLPart);
  }

  PartEdgePtr CommATSPart::inEdgeFromAny() {
    PartEdgePtr baseInEdgeFromAny = base->inEdgeFromAny();
    return makePtr<CommATSPartEdge>(baseInEdgeFromAny, mpicommanalysis_p, NULLPart, get_shared_this());
  }

  bool CommATSPart::equal(const PartPtr& that) const {
    const CommATSPartPtr thatcaPart = dynamicConstPtrCast<CommATSPart>(that);
    assert(thatcaPart.get());
    if(base == thatcaPart->base && context == thatcaPart->context) return true;
    else return false;  
  }

  bool CommATSPart::less(const PartPtr& that) const {
    const CommATSPartPtr thatcaPart = dynamicConstPtrCast<CommATSPart>(that);
    assert(thatcaPart.get());
    if(base < thatcaPart->base) return true;
    else if(base == thatcaPart->base && context < thatcaPart->context) return true;
    else return false;
  }

  string CommATSPart::str(string indent) const {
    ostringstream oss;
    oss << "[CommATSPart: " << context->str() << " " << base->str() << "]";
    return oss.str();
  }

  /*******************
   * CommATSPartEdge *
   *******************/
  CommATSPartEdge::CommATSPartEdge(PartEdgePtr base, MPICommAnalysis* analysis, CommATSPartPtr src, CommATSPartPtr tgt)
  : PartEdge(analysis, base),
    base(base),
    src(src),
    tgt(tgt) {
  }

  CommATSPartEdge::CommATSPartEdge(const CommATSPartEdge& that)
  : PartEdge(that),
    base(that.base),
    src(that.src),
    tgt(that.tgt) {
  }

  PartPtr CommATSPartEdge::source() const {
    return src;
  }

  PartPtr CommATSPartEdge::target() const {
    return tgt;
  }

  list<PartEdgePtr> CommATSPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand) {
    list<PartEdgePtr> baseOpPartEdges = base->getOperandPartEdge(anchor, operand);
    assert(0);
  }

  map<CFGNode, shared_ptr<SgValueExp> > CommATSPartEdge::getPredicateValue() {
    assert(0);
    return parent->getPredicateValue();
  }

  bool CommATSPartEdge::equal(const PartEdgePtr& that) const {
    const CommATSPartEdgePtr thatcaEdge = dynamicConstPtrCast<CommATSPartEdge>(that);
    assert(thatcaEdge.get());
    if(base == thatcaEdge->base &&
       src == thatcaEdge->source() &&
       tgt == thatcaEdge->target()) return true;
    return false;
  }

  bool CommATSPartEdge::less(const PartEdgePtr& that) const {
    const CommATSPartEdgePtr thatcaEdge = dynamicConstPtrCast<CommATSPartEdge>(that);
    assert(thatcaEdge.get());
    if(base < thatcaEdge->base) return true;
    else if(base == thatcaEdge->base && src < thatcaEdge->source()) return true;
    else if(base == thatcaEdge->base &&
            src == thatcaEdge->source() &&
            tgt < thatcaEdge->target()) return true;
    else return false;
  }

  string CommATSPartEdge::str(string indent) const {
    ostringstream oss;
    oss << (src ? src->str() : "*");
    oss << " ==> ";
    oss << (tgt ? tgt->str() : "*");
    return oss.str();
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
    assert(ccl);
    elem = ccl->getCCLatElem();
  }

  bool CommContextLattice::copy(CommContextLattice* that) {
    if(isFullLat()) return false;
    ROSE_ASSERT(that);
    if(elem != that->getCCLatElem()) {
      elem = that->getCCLatElem();
      return true;
    }
    return false;
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
  MPICommAnalysis::MPICommAnalysis() {
  }

  void MPICommAnalysis::initAnalysis(set<PartPtr>& startingParts) {
    scope reg("MPICommAnalysis::initAnalysis", scope::low, attrGE("mpiCommAnalysisDebugLevel", 2));
    set<PartPtr>::iterator s = startingParts.begin();
    for( ; s != startingParts.end(); ++s) {
      PartPtr part = *s;
      dbg << "Part=" << part->str() << endl;

      // Get the lattice associated with all incoming edges to starting parts
      NodeState* state = NodeState::getNodeState(this, part);
      assert(state);
      Lattice* l = state->getLatticeAbove(this, part->inEdgeFromAny(), 0);
      assert(l);
      CommContextLattice* ccl_p = dynamic_cast<CommContextLattice*>(l);
      assert(ccl_p);

      // Set the lattice to NonMPICommContext
      ccl_p->setCCLatElemNonMPI();

      dbg << "state=" << state->str() << endl;
    }
  }

  void MPICommAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    CommContextLattice* ccl_p = new CommContextLattice(pedge);
    initLattices.push_back(ccl_p);
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
    scope reg("MPICommAnalysis::transfer", scope::low, attrGE("mpiCommAnalysisDebugLevel", 1));
    bool modified=false;

    // Incoming lattice information
    CommContextLattice* cclat_p = dynamic_cast<CommContextLattice*>(dfInfo[part->inEdgeFromAny()][0]);
    assert(cclat_p);

    if(mpiCommAnalysisDebugLevel() >= 2) {
      dbg << "Lattice IN:" << cclat_p->str() << endl;
    }

    // Context for descendants is decided based on this parts position on the
    // function boundaries
    // If this is an outgoing MPI Call mark the edge as MPICommContext
    if(part->isOutgoingFuncCall(cn) && isMPIFuncCall(isSgFunctionCallExp(cn.getNode()))) {
      modified = cclat_p->setCCLatElemMPI();
    }
    // If this an call return from MPI mark the edge as NonMPICommContext
    else if(part->isIncomingFuncCall(cn) && isMPIFuncCall(isSgFunctionCallExp(cn.getNode()))) {
      modified = cclat_p->setCCLatElemNonMPI();
    }
    // If its not function boundary set the outgoing edge to
    // same context as incoming edge
    else {
      PartEdgePtr iedge = part->inEdgeFromAny();
      Lattice* lin = state.getLatticeAbove(this, iedge, 0); assert(lin);
      CommContextLattice* cclin_p = dynamic_cast<CommContextLattice*>(lin); assert(cclin_p);
      modified = cclat_p->copy(cclin_p);
    }

    if(mpiCommAnalysisDebugLevel() >= 2) {
      dbg << "Lattice OUT:" << cclat_p->str() << endl;
    }
    return modified;
  }

  set<PartPtr> MPICommAnalysis::GetStartAStates_Spec() {
    set<PartPtr> sParts = composer->GetStartAStates(this);
    set<PartPtr> sCommParts;
    set<PartPtr>::iterator s=sParts.begin();
    for( ; s != sParts.end(); ++s) {
      PartPtr spart = *s;
      NodeState* state = NodeState::getNodeState(this, spart);
      CommContextLattice* ccl_p = dynamic_cast<CommContextLattice*>(state->getLatticeAbove(this, 
                                                                                           spart->inEdgeFromAny(), 0));
      ROSE_ASSERT(ccl_p);
      // Startings must have NonMPICommContext
      assert(ccl_p->isCCLatElemNonMPI());
      // Create NonMPICommContext based on parent context
      NonMPICommContextPtr nmcc = makePtr<NonMPICommContext>(spart->getPartContext());
      CommATSPartPtr cap = makePtr<CommATSPart>(spart, this, nmcc);
      sCommParts.insert(cap);      
    }
    return sCommParts;
  }

  set<PartPtr> MPICommAnalysis::GetEndAStates_Spec() {
    set<PartPtr> eParts = composer->GetEndAStates(this);
    set<PartPtr> eCommParts;
    set<PartPtr>::iterator e=eParts.begin();
    for( ; e != eParts.end(); ++e) {
      PartPtr epart = *e;
      NodeState* state = NodeState::getNodeState(this, epart);
      CommContextLattice* ccl_p = dynamic_cast<CommContextLattice*>(state->getLatticeAbove(this, 
                                                                                           epart->inEdgeFromAny(), 0));
      ROSE_ASSERT(ccl_p);
      // Ending must have NonMPICommContext
      assert(ccl_p->isCCLatElemNonMPI());
      // Create NonMPICommContext based on parent context
      NonMPICommContextPtr nmcc = makePtr<NonMPICommContext>(epart->getPartContext());
      CommATSPartPtr cap = makePtr<CommATSPart>(epart, this, nmcc);
      eCommParts.insert(cap);      
    }
    return eCommParts;
  }

  string MPICommAnalysis::str(string indent) const {
    return "MPICommAnalysis";
  }

}; // end namespace

//  LocalWords:  MPICommAnalysis MPICommATSPartEdge CommATSPart
