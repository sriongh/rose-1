/****************************************
 * Author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "sage3basic.h"
#include "mpi_comm_analysis.h"
#include "latticeFull.h"
#include <algorithm>

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

  CommContextPtr MPICommContext::copy() const {
    return makePtr<MPICommContext>(*this);
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

  CommContextPtr NonMPICommContext::copy() const {
    return makePtr<NonMPICommContext>(*this);
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

  CommContextPtr CommATSPart::getCommContext() const {
    return context;
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


  //! Merge the items of setfrom into setto
  bool CommContextLattice::mergeCommATSPartSets(CommATSPartSet& setto, CommATSPartSet& setfrom) {        
    bool modified = false;
    CommATSPartSet::iterator fsi = setfrom.begin(), tsi = setto.begin();
    while(fsi != setfrom.end() && tsi != setto.end()) {
      // If element is in setto but not in setfrom
      if(*tsi < *fsi) {
        ++tsi;
      }
      // If the element is in setfrom but not in setto
      else if(*tsi > *fsi) {
        // Insert the element
        // Find the pos after which the element needs to inserted
        // CommATSPartSet::iterator pos = tsi != setto.begin() ? pos = tsi-1 : pos = setto.begin();
        CommATSPartSet::iterator pos = tsi;
        tsi = setto.insert(pos, *fsi);
        modified = true;
        ++fsi;
      }
      else if(*fsi == *tsi) {
        ++fsi; ++tsi;
      }
    }

    // Copy the remaining elements of setfrom if any
    CommATSPartSet::iterator pos = setto.end();
    for( ; fsi != setfrom.end(); ++fsi) {
      pos = setto.insert(pos, *fsi);
      modified = true;
    }
    return modified;
  }

  //! Merge the elements of fromMap into toMap
  bool CommContextLattice::mergeCommATSPartMaps(CommATSPartMap& toMap, CommATSPartMap& fromMap) {
    CommATSPartMap::iterator fmi = fromMap.begin(), tmi = toMap.begin();
    bool modified = false;
    // Iterate through the maps
    while(fmi != fromMap.end() && tmi != toMap.end()) {
      // If the key is present in toMap but not in fromMap      
      if(tmi->first < fmi->first) {
        // Nothing to insert 
        // Move the iterator of toMap
        ++tmi;
      }
      // If the key is present in fromMap but not in toMap
      else if(tmi->first > fmi->first) {
        // Insert the <key,value> from fromMap
        // Find the position after which the item needs to be inserted
        // CommATSPartMap::iterator pos = tmi != toMap.begin() ? pos = tmi-1 : pos = toMap.begin();
        CommATSPartMap::iterator pos = tmi;
        tmi = toMap.insert(pos, *fmi);
        // toMap[fmi->first] = fmi->second;
        modified = true;
        ++fmi;
      }
      // If the key is present in both maps
      else if(fmi->first == tmi->first) {
        // Merge the two sets
        modified = mergeCommATSPartSets(tmi->second, fmi->second) || modified;
        ++fmi; ++tmi;
      }      
    }
    // Copy the remaining elements of fromMap
    for( ; fmi != fromMap.end(); ++fmi) {
      toMap[fmi->first] = fmi->second;
      modified = true;
    }
    return modified;
  }

  //! replaceML is used to update the lattice information after returning from a function.
  //! newL is the lattice information after returning from a function.
  //! newL is the most recent information about edges coming back from a function.
  //! Overwrite this edge information with newL.
  bool CommContextLattice::replaceML(Lattice* newL) {
    bool modified = (*this == newL);
    copy(newL);
    return modified;
  }

  bool CommContextLattice::meetUpdate(Lattice* that) {
    // scope reg("CommContextLattice::meetUpdate", scope::low, attrGE("mpiCommAnalysisDebugLevel", 3));
    CommContextLattice* thatCCL = dynamic_cast<CommContextLattice*>(that);
    ROSE_ASSERT(thatCCL);
    bool modified = mergeCommATSPartMaps(outgoing, thatCCL->outgoing);
    // Update incoming based on modified outgoing
    if(modified) createIncomingMapfromOutgoingMap();

    // dbg << "After Meet: " << str() << endl;
    return modified;
  }

  bool CommContextLattice::setMLValueToFull(MemLocObjectPtr ml_p) {
    return false;
  }

  string CommContextLattice::str(const CommATSPartSet& commATSPartSet) const {
    ostringstream oss;
    CommATSPartSet::const_iterator cs = commATSPartSet.begin();
    for( ; cs != commATSPartSet.end(); ) {
      oss << cs->str();
      ++cs;
      if(cs != commATSPartSet.end()) oss << cs->str() << ",\n";
    }
    return oss.str();
  }

  string CommContextLattice::str(string indent) const {
    ostringstream oss;
    oss << "<b> CommContextLatticeVal: </b>";
    oss << "<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\">";
    oss << "<tr> <th>" << "outgoing(" << outgoing.size() << "):" << "</th> </tr>";
    CommATSPartMap::const_iterator o = outgoing.begin();
    for( ; o != outgoing.end(); ++o) {
      oss << "<tr>";
      oss <<"<td width=\"35%\">" << (o->first.get()? o->first->str() : "NULLCommATSPart") << "</td>";
      oss << "<td>" << str(o->second) << "</td>";
      oss << "</tr>";
    }
    oss << "</table>";
    oss << "<table border=\"1\" cellpadding=\"3\" cellspacing=\"0\">";
    oss << "<tr> <th>" << "incoming(" << incoming.size() << "):" << "</th> </tr>";
    CommATSPartMap::const_iterator i = incoming.begin();
    for( ; i != incoming.end(); ++i) {
      oss << "<tr>";
      oss <<"<td width=\"35%\">" << (i->first.get()? i->first->str() : "NULLCommATSPart") << "</td>";
      oss << "<td>" << str(i->second) << "</td>";
      oss << "</tr>";
    }
    oss << "</table>";
    return oss.str();
  }

  // Generic insert method
  bool CommContextLattice::insert(CommATSPartPtr src, CommATSPartPtr tgt, CommATSPartMap& commATSMap) {    
    CommATSPartSet& capSet = commATSMap[src];
    if(mpiCommAnalysisDebugLevel() >= 3 && capSet.size() > 0) {
      scope reg("CommContextLattice::insert", scope::low, attrGE("mpiCommAnalysisDebugLevel", 3));
      dbg << "<b> CommATSPartSet(" << capSet.size() << ") </b>";
      dbg << str(capSet) << "\n";
    }
    pair<CommATSPartSet::iterator, bool> insertReturnVal = capSet.insert(tgt);
    return insertReturnVal.second;
  }

  bool CommContextLattice::outGoingInsert(CommATSPartPtr src, CommATSPartPtr tgt) {    
    return insert(src, tgt, outgoing);
  }

  bool CommContextLattice::inComingInsert(CommATSPartPtr src, CommATSPartPtr tgt) {
    return insert(src, tgt, incoming);
  }

  bool CommContextLattice::parentPartEqual(CommATSPartPtr caPartPtr, PartPtr parent) {
    if(caPartPtr->getParent() == parent) return true;
    return false;
  }

  //! Iterate through the map and apply the filter on each element of the CommATSPartSet
  //! If filter returns true add the element to the list to return
  list<CommATSPartPtr> CommContextLattice::applyMapFilter(CommATSPartMap& commATSPartMap,
                                                          boost::function<bool (CommATSPartPtr)> filter) {
    list<CommATSPartPtr> caParts;
    CommATSPartMap::iterator mi = commATSPartMap.begin();
    for( ; mi != commATSPartMap.end(); ++mi) {
      CommATSPartSet& caPartSet = mi->second;
      CommATSPartSet::iterator si = caPartSet.begin();
      for( ; si != caPartSet.end(); ++si) {
        CommATSPartPtr caPartPtr = *si;
        if(filter(caPartPtr)) caParts.push_back(caPartPtr);
      }
    }
    return caParts;
  }

  //! Iterate through outgoing map applying the parentPart
  list<CommATSPartPtr> CommContextLattice::parentPartFilterOutgoingMap(PartPtr parent) {
    boost::function<bool (CommATSPartPtr)> filter = boost::bind(&CommContextLattice::parentPartEqual, this, _1, parent);
    return applyMapFilter(outgoing, filter);
  }

  bool CommContextLattice::createIncomingMapfromOutgoingMap() {
    CommATSPartMap::iterator om = outgoing.begin(); 
    CommATSPartSet::iterator os;
    incoming.clear();
    for( ; om != outgoing.end(); ++om) {
      for(os = om->second.begin(); os != om->second.end(); ++os) {
        incoming[*os].insert(om->first);
      }
    }
    return true;
  }

  /******************
   * Helper Methods *
   ******************/
  bool isMPIFuncCall(CFGNode cfgn) {
    ROSE_ASSERT(cfgn.getNode());
    SgFunctionCallExp* fncall = isSgFunctionCallExp(cfgn.getNode());
    ROSE_ASSERT(fncall);
    Function func(fncall);
    if(func.get_name().getString().find("MPI_",0) == 0) return true;
    return false;
  }

  /*******************
   * MPICommAnalysis *
   *******************/
  MPICommAnalysis::MPICommAnalysis() {
  }

  void MPICommAnalysis::initAnalysis(set<PartPtr>& startingParts) {
    set<PartPtr>::iterator s = startingParts.begin();
    for( ; s != startingParts.end(); ++s) {
      // Build the CommATSPart for each starting part
      // All ATS parts are assumed to start in NonMPICommContext
      PartPtr baseStartPart = *s;
      CommContextPtr ccp = makePtr<NonMPICommContext>(baseStartPart->getPartContext());
      CommATSPartPtr srcCommATSPart = makePtr<CommATSPart>(baseStartPart, this, ccp);
      NodeState* state = NodeState::getNodeState(this, baseStartPart);

      CommContextLattice* ccl = dynamic_cast<CommContextLattice*>(state->getLatticeAbove(this, baseStartPart->inEdgeFromAny(), 0));
      ROSE_ASSERT(ccl);
      CommATSPartPtr dummyCommATSPart;
      ccl->outGoingInsert(dummyCommATSPart, srcCommATSPart);
      if(mpiCommAnalysisDebugLevel() >= 3) dbg << state->str() << endl;
    }
  }

  void MPICommAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    CommContextLattice* ccl_p = new CommContextLattice(pedge);
    initLattices.push_back(ccl_p);
  }

  // 1. First obtain the src CommATSPartPtr for the current part
  // 2. For each outgoing edge construct the target CommATSPartPtr
  // 3. Populate CommContextLattice's outgoing map with src and target CommATSPartPtr
  bool MPICommAnalysis::transfer(PartPtr part, CFGNode cn, NodeState& state, 
                                 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    scope reg("MPICommAnalysis::transfer", scope::low, attrGE("mpiCommAnalysisDebugLevel", 3));
    bool modified=false;

    // 1. First obtain the src CommATSPartPtr for the current part
    // CommATSPartPtr for the current part will be in the outgoing map of incoming dataflow info
    // Iterate through the map keys
    // For each key iterate through their sets
    // Collect all CommATSPart that were created at a predecessor node for the current part
    CommContextLattice* inCCL = dynamic_cast<CommContextLattice*>(dfInfo[part->inEdgeFromAny()][0]);
    assert(inCCL);
    if(mpiCommAnalysisDebugLevel() >= 3) {
      dbg << "inCCL=" << inCCL->str() << endl;
    }

    // Collect all CommATSPart whose parent part is same as the current part
    list<CommATSPartPtr> caPartList = inCCL->parentPartFilterOutgoingMap(part);
    list<CommATSPartPtr>::iterator li = caPartList.begin();

    // We should find at least one CommATSPart in the outgoing map whose getParent() == current_part
    if(caPartList.size() == 0) {
      // Error
      // Output debug information before failing
      scope reg("MPICommAnalysis::Error", scope::medium, attrGE("mpiCommAnalysisDebugLevel", 3));
      dbg << "<b>";
      dbg << "No CommATSPart in inCCL->outgoing with match condition CommATSPart::getParent() ==  current_part\n";
      dbg << "inCCL=" << inCCL->str() << endl;
      dbg<< "current_part=" << part->str() << endl;
      dbg << "</b>";
    }

    ROSE_ASSERT(caPartList.size() > 0);
    // We have least one CommATSPart in the outgoing map whose getParent() == current_part

    // If this part does not have any outEdge (example: termination part)
    // Leave the dfInfo intact and do nothing in the transfer function
    // Composer adds an edge beyond the termination part and stores the dfInfo on that edge
    list<PartEdgePtr> oedges = part->outEdges();    
    if(oedges.size() == 0) return true;

    // 2. For each outgoing edge construct the target CommATSPartPtr based on src CommATSPartPtr
    // 3. Populate CommContextLattice's outgoing map with src and target CommATSPartPtr
    dfInfo.clear();

    // For each CommATSPart for the current part
    // Iterate through all the outedges of the current part and create an equivalent target CommATSPart
    for( ; li != caPartList.end(); ++li) {
      CommATSPartPtr currCommATSPartPtr = *li;
      // if(mpiCommAnalysisDebugLevel() >= 3) {
      //   dbg << "parent=" << part->str() << endl;
      //   dbg << "commATSPart=" << currCommATSPartPtr->str() << endl;
      // }
      list<PartEdgePtr>::iterator oe = oedges.begin();
      for( ; oe != oedges.end(); ++oe) {
        PartPtr tgt = (*oe)->target();
        // Build the target part based on the current CommATSPart
        CommATSPartPtr tgtCommATSPartPtr = buildCommATSPart(tgt, currCommATSPartPtr);
        CommContextLattice* outCCL = new CommContextLattice(*oe);
        modified = outCCL->outGoingInsert(currCommATSPartPtr, tgtCommATSPartPtr) || modified;
        // Populate the incoming map based on outgoing map
        outCCL->createIncomingMapfromOutgoingMap();

        if(mpiCommAnalysisDebugLevel() >= 3) {
          dbg << "outEdge=" << (*oe)->str() << endl;
          dbg << "outCCL=" << outCCL->str() << endl;
        }
        // Add the lattice to corresponding edge
        dfInfo[*oe].push_back(outCCL);
      }
    }   
    return modified;
  }

  set<PartPtr> MPICommAnalysis::GetStartAStates_Spec() {
    set<PartPtr> sParts = composer->GetStartAStates(this);
    set<PartPtr> sCommParts;
    set<PartPtr>::iterator s=sParts.begin();
    // MPICommAnalysis::initAnalysis creates a starting CommATSPart for each parent starting part
    // For each starting part get the CommContextLattice Above
    // Iterate through all sets in the outgoing map 
    // and find all CommATSPart corresponding to each starting part  
    for( ; s != sParts.end(); ++s) {
      PartPtr spart = *s;
      CommContextLattice* ccl_p = getCommContextLatticeAbove(spart, spart->inEdgeFromAny());
      list<CommATSPartPtr> caPartList = ccl_p->parentPartFilterOutgoingMap(spart);
      sCommParts.insert(caPartList.begin(), caPartList.end());
    }
    return sCommParts;
  }

  // Iterate through all sets in the outgoing map 
  // and find all CommATSPart corresponding to each ending part  
  set<PartPtr> MPICommAnalysis::GetEndAStates_Spec() {
    set<PartPtr> eParts = composer->GetEndAStates(this);
    set<PartPtr> eCommParts;
    set<PartPtr>::iterator e=eParts.begin();
    for( ; e != eParts.end(); ++e) {
      PartPtr epart = *e;
      CommContextLattice* ccl_p = getCommContextLatticeAbove(epart, epart->inEdgeFromAny());
      list<CommATSPartPtr> caPartList = ccl_p->parentPartFilterOutgoingMap(epart);
      eCommParts.insert(caPartList.begin(), caPartList.end());
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

  CommATSPartPtr MPICommAnalysis::buildCommATSPart(PartPtr base, CommATSPartPtr parentCommATSPart) {
    // If its a MPI call create CommATSPart with MPICommContext
    set<CFGNode> cfgnodes;
    CommATSPartPtr commATSPart;
    typedef bool (*isMPICallFuncPtr)(CFGNode);
    isMPICallFuncPtr mpifnptr_p = isMPIFuncCall;
    if(base->mustOutgoingFuncCall(cfgnodes) && base->filterAll(mpifnptr_p)) {
      // Create MPI abstraction using call site
      MPIOpAbsPtr mpiop = createMPIOpAbs(base);
      CommContextPtr cc = makePtr<MPICommContext>(mpiop);
      commATSPart = makePtr<CommATSPart>(base, this, cc);
    }
    else if(parentCommATSPart->mustIncomingFuncCall(cfgnodes) && parentCommATSPart->filterAll(isMPIFuncCall)) {
      // Switch back to NonMPI context
      CommContextPtr cc = makePtr<NonMPICommContext>(base->getPartContext());
      commATSPart = makePtr<CommATSPart>(base, this, cc);
    }
    else {
      CommContextPtr cc = parentCommATSPart->getCommContext();
      commATSPart = makePtr<CommATSPart>(base, this, cc);
    }
    ROSE_ASSERT(commATSPart.get());
    return commATSPart;
  }

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
