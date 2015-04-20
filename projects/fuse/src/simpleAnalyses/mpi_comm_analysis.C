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

  list<PartEdgePtr> CommATSPart::outEdges() {
    scope reg("CommATSPart::outEdges()", scope::low, attrGE("mpiCommAnalysisDebugLevel", 3)); 
    list<PartEdgePtr> caOutEdges;

    list<PartEdgePtr> oedges = base->outEdges();
    list<PartEdgePtr>::iterator oe = oedges.begin();

    assert(0);

    for( ; oe != oedges.end(); ++oe) {
      PartPtr baseTarget = (*oe)->target();
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
    set<PartPtr> baseMatchingParts;
    set<PartPtr> commATSMatchingParts;
    baseMatchingParts = base->matchingCallParts();
    set<PartPtr>::iterator bp = baseMatchingParts.begin();

    assert(0);

    for( ; bp != baseMatchingParts.end(); ++bp) {
    }
    return commATSMatchingParts;
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
    mpicommanalysis_p(analysis),
    src(src),
    tgt(tgt) {
  }

  CommATSPartEdge::CommATSPartEdge(const CommATSPartEdge& that)
  : PartEdge(that),
    base(that.base),
    mpicommanalysis_p(that.mpicommanalysis_p),
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
    list<PartEdgePtr> commATSOpPartEdges;
    list<PartEdgePtr>::iterator be = baseOpPartEdges.begin();


    assert(0);

    for( ; be != baseOpPartEdges.end(); ++be) {
    }
    return commATSOpPartEdges;
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
  : Lattice(edge_p), FiniteLattice(edge_p) {
  }

  CommContextLattice::CommContextLattice(const CommContextLattice& that)
  : Lattice(that.latPEdge), FiniteLattice(that.latPEdge),
    outgoing(that.outgoing), incoming(that.incoming) {
  }

  void CommContextLattice::initialize() {
    outgoing.clear();
    incoming.clear();
  }

  bool CommContextLattice::setToFull() {
    assert(0);
    return false;
  }

  bool CommContextLattice::setToEmpty() {
    bool modified = false;
    if(outgoing.size() > 0 || incoming.size() > 0) modified = true;
    initialize();
    return modified;
  }

  Lattice* CommContextLattice::copy() const {
    return new CommContextLattice(*this);
  }

  void CommContextLattice::copy(Lattice* that) {
    CommContextLattice* thatccl_p = dynamic_cast<CommContextLattice*>(that);
    ROSE_ASSERT(thatccl_p);
    outgoing = thatccl_p->outgoing;
    incoming = thatccl_p->incoming;
  }

  bool CommContextLattice::operator==(Lattice* that) {
    CommContextLattice* thatccl_p = dynamic_cast<CommContextLattice*>(that);
    ROSE_ASSERT(thatccl_p);
    return (outgoing == thatccl_p->outgoing &&
            incoming == thatccl_p->incoming);
  }

  bool CommContextLattice::isEmptyLat() {
    return (outgoing.size() == 0 &&
            incoming.size() == 0);
  }

  bool CommContextLattice::isFullLat() {
    assert(0);
    return false;
  }

  bool CommContextLattice::meetUpdate(Lattice* that) {
    CommContextLattice* ccl = dynamic_cast<CommContextLattice*>(that);
    ROSE_ASSERT(ccl);
    assert(0);
  }

  bool CommContextLattice::setMLValueToFull(MemLocObjectPtr ml_p) {
    return false;
  }

  string CommContextLattice::str(const CommATSPartSet& commATSPartSet) const {
    ostringstream oss;
    CommATSPartSet::const_iterator cs = commATSPartSet.begin();
    oss << "{ ";
    for( ; cs != commATSPartSet.end(); ) {
      oss << cs->str();
      ++cs;
      if(cs != commATSPartSet.end()) oss << cs->str() << ",\n";
    }
    oss << " }";
    return oss.str();
  }

  string CommContextLattice::str(string indent) const {
    ostringstream oss;
    oss << "<table border=\"1\">";
    oss << "<th> CommContextLatticeVal: </th>";
    oss << "<tr> <th>" << "outgoing(" << outgoing.size() << "):" << "</th> </tr>";
    CommATSPartMap::const_iterator o = outgoing.begin();
    for( ; o != outgoing.end(); ++o) {
      oss << "<tr>";
      oss << "<td>" << o->first->str() << "<\td>";
      oss << "<td>" << str(o->second) << "<\td>";
      oss << "<\tr>";
    }
    oss << "<tr> <th>" << "incoming(" << incoming.size() << "):" << "</th> </tr>";
    CommATSPartMap::const_iterator i = incoming.begin();
    for( ; i != incoming.end(); ++i) {
      oss << "<tr>";
      oss << "<td>" << i->first->str() << "<\td>";
      oss << "<td>" << str(i->second) << "<\td>";
      oss << "<\tr>";
    }
    oss << "</table>";
    return oss.str();
  }

  /*******************
   * MPICommAnalysis *
   *******************/
  MPICommAnalysis::MPICommAnalysis() {
  }

  void MPICommAnalysis::initAnalysis(set<PartPtr>& startingParts) {    
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

    assert(0);

    // Context for descendants is decided based on this parts position on the
    // function boundaries
    // If this is an outgoing MPI Call mark the edge as MPICommContext
    if(part->isOutgoingFuncCall(cn) && isMPIFuncCall(isSgFunctionCallExp(cn.getNode()))) {
    }
    // If this an call return from MPI mark the edge as NonMPICommContext
    else if(part->isIncomingFuncCall(cn) && isMPIFuncCall(isSgFunctionCallExp(cn.getNode()))) {      
    }
    // If its not function boundary set the outgoing edge to
    // same context as incoming edge
    else {
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

      assert(0);   
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
      assert(0);
    }
    return eCommParts;
  }

  string MPICommAnalysis::str(string indent) const {
    return "MPICommAnalysis";
  }

  CommContextLattice* MPICommAnalysis::getCommContextLatticeAbove(PartPtr part, PartEdgePtr pedge) {
    NodeState* state_p = NodeState::getNodeState(this, part);
    CommContextLattice* ccl_p = dynamic_cast<CommContextLattice*>(state_p->getLatticeAbove(this, pedge, 0));
    ROSE_ASSERT(ccl_p);
    return ccl_p;
  }

  CommContextLattice* MPICommAnalysis::getCommContextLatticeBelow(PartPtr part, PartEdgePtr pedge) {
    NodeState* state_p = NodeState::getNodeState(this, part);
    CommContextLattice* ccl_p = dynamic_cast<CommContextLattice*>(state_p->getLatticeBelow(this, pedge, 0));
    ROSE_ASSERT(ccl_p);
    return ccl_p;
  }

  // CommATSPartPtr MPICommAnalysis::buildCommATSPart(PartPtr base, PartEdgePtr efrom, PartEdgePtr eto) {
  //   CommContextLattice* efromccl_p = getCommContextLatticeAbove(base, efrom);
  //   CommContextLattice* etoccl_p = getCommContextLatticeBelow(base, eto);

  //   assert(0);
  //   CommATSPartPtr commATSPart = makePtr<CommATSPart>(base, this, commContext);
  //   return commATSPart;
  // }


  // CommATSPartEdgePtr MPICommAnalysis::buildCommATSPartEdge(PartEdgePtr baseEdge) {
  //   PartPtr baseSource = baseEdge->source();
  //   PartPtr baseTarget = baseEdge->target();
  //   CommATSPartPtr commATSSource = buildCommATSPart(baseSource, baseSource->inEdgeFromAny(), baseEdge);
  //   CommATSPartPtr commATSTarget = buildCommATSPart(baseTarget, baseEdge, baseTarget->outEdgeToAny());
    
  //   CommATSPartEdgePtr commATSEdge = makePtr<CommATSPartEdge>(baseEdge, this, commATSSource, commATSTarget);
  //   dbg << commATSEdge << endl;
  //   return commATSEdge;
  // }
}; // end namespace

//  LocalWords:  MPICommAnalysis MPICommATSPartEdge CommATSPart
