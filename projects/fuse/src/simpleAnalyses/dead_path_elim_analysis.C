#include "sage3basic.h"
using namespace std;

#include "dead_path_elim_analysis.h"

#ifndef DISABLE_SIGHT
using namespace sight;
#endif

namespace fuse {

#define DPEAnalDebugLevel 0
#if DPEAnalDebugDevel==0
  //  #define DISABLE_SIGHT
#endif

std::string DPELevel2Str(enum DPELevel level) {
  return (level==bottom? "bottom": (level==dead? "dead": (level==live? "live": "???")));
}

/****************************
 ***** DeadPathElimPart *****
 ****************************/

DeadPathElimPart::DeadPathElimPart(PartPtr base, ComposedAnalysis* analysis) :
  Part(analysis, base)
{
  cacheInitialized_outEdges=false;
  cacheInitialized_inEdges=false;
  cacheInitialized_CFGNodes=false;
  cacheInitialized_matchingCallParts=false;
  cacheInitialized_matchingEntryExitParts=false;
  cacheInitialized_inEdgeFromAny=false;
  cacheInitialized_outEdgeToAny=false;
}

DeadPathElimPart::DeadPathElimPart(const DeadPathElimPart& that) :
  Part((const Part&)that)
{
  cacheInitialized_outEdges               = that.cacheInitialized_outEdges;
  cache_outEdges                          = that.cache_outEdges;
  cacheInitialized_inEdges                = that.cacheInitialized_inEdges;
  cache_inEdges                           = that.cache_inEdges;
  cacheInitialized_CFGNodes               = that.cacheInitialized_CFGNodes;
  cache_CFGNodes                          = that.cache_CFGNodes;
  cacheInitialized_matchingCallParts      = that.cacheInitialized_matchingCallParts;
  cache_matchingCallParts                 = that.cache_matchingCallParts;
  cacheInitialized_matchingEntryExitParts = that.cacheInitialized_matchingEntryExitParts;
  cache_matchingEntryExitParts            = that.cache_matchingEntryExitParts;
  cacheInitialized_inEdgeFromAny          = that.cacheInitialized_inEdgeFromAny;
  cache_inEdgeFromAny                     = that.cache_inEdgeFromAny;
  cacheInitialized_outEdgeToAny           = that.cacheInitialized_outEdgeToAny;
  cache_outEdgeToAny                      = that.cache_outEdgeToAny;
  cache_equal                             = that.cache_equal;
  cache_less                              = that.cache_less;
}

// Returns a shared pointer to this of type DeadPathElimPartPtr
DeadPathElimPartPtr DeadPathElimPart::get_shared_this()
{ return dynamicPtrCast<DeadPathElimPart>(makePtrFromThis(shared_from_this())); }

// -------------------------------------------
// Functions that need to be defined for Parts
// -------------------------------------------

list<PartEdgePtr> DeadPathElimPart::outEdges()
{
  SIGHT_VERB_DECL(scope, (txt()<<"DeadPathElimPart::outEdges("<<str()<<")"), 2, DPEAnalDebugLevel)
  //SIGHT_VERB(dbg<<"cacheInitialized_outEdges="<<cacheInitialized_outEdges<<endl, 2, DPEAnalDebugLevel)

  if(!cacheInitialized_outEdges) {
  //  scope reg(txt()<<"DeadPathElimPart::outEdges()", scope::medium, attrGE("DPEAnalDebugLevel", 2));
    // The NodeState at the current part
    SIGHT_VERB(dbg << "getNodeStateLocPart()="<<getNodeStateLocPart()->str()<<endl, 2, DPEAnalDebugLevel)
    NodeState* outState = NodeState::getNodeState(analysis, getNodeStateLocPart());
    SIGHT_VERB(dbg << "outState="<<outState->str(analysis)<<endl, 2, DPEAnalDebugLevel)

    list<PartEdgePtr> baseEdges = getInputPart()->outEdges();
    SIGHT_VERB(dbg << "#baseEdges="<<baseEdges.size()<<endl, 2, DPEAnalDebugLevel)

    // Consider all the DeadPathElimParts along all of this part's outgoing edges. Since this is a forward
    // analysis, they are maintained separately
    const map<PartEdgePtr, vector<Lattice*> >& lattices = outState->getLatticeBelowAll(analysis);
    for(map<PartEdgePtr, vector<Lattice*> >::const_iterator i=lattices.begin(); i!=lattices.end(); ++i) {
      SIGHT_VERB_DECL(scope, (txt()<<"be="<<i->first->str(), scope::low), 2, DPEAnalDebugLevel)
      assert(i->second.size()==1);
      SIGHT_VERB(dbg << "outState->getLatticeBelow(analysis, *be, 0) = "<<i->second[0]->str()<<endl, 2, DPEAnalDebugLevel)

      DeadPathElimPartEdge* outPartEdge = dynamic_cast<DeadPathElimPartEdge*>(i->second[0]);
      assert(outPartEdge);
      SIGHT_VERB(dbg << "outPartEdge("<<(outPartEdge->level==live)<<")="<<outPartEdge->str()<<endl, 2, DPEAnalDebugLevel)

      // If this outgoing edge is not a wildcard and is live
      if(outPartEdge->target() && outPartEdge->level==live)
        // Create a new DeadPathElimPartEdgePtr from an existing outPartEdge. To ensure that the
        // original is not deallocated when the shared pointer goes out of scope, we keep the shared
        // pointer in a cache data structure that persists.
        //cache_outEdges.push_back(initPtr(dynamic_cast<DeadPathElimPartEdge*>(outPartEdge)));
        //dbg << "dynamic_cast<DeadPathElimPartEdge*>(outPartEdge)=<"<<dynamic_cast<DeadPathElimPartEdge*>(outPartEdge)<<"> = "<<dynamic_cast<DeadPathElimPartEdge*>(outPartEdge)->str()<<endl;
        //dbg << "initPtr(dynamic_cast<DeadPathElimPartEdge*>(outPartEdge))=<"<<initPtr(dynamic_cast<DeadPathElimPartEdge*>(outPartEdge)).get()<<"> = "<<initPtr(dynamic_cast<DeadPathElimPartEdge*>(outPartEdge))->str()<<endl;
        cache_outEdges.push_back(DeadPathElimPartEdge::raw2shared(dynamic_cast<DeadPathElimPartEdge*>(outPartEdge)));
    }

    SIGHT_VERB_IF(2, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
    dbg << "Cached: "<<endl;
    for(list<PartEdgePtr>::iterator e=cache_outEdges.begin(); e!=cache_outEdges.end(); ++e)
      dbg << e->str()<<endl;
#endif
    SIGHT_VERB_FI()


    cacheInitialized_outEdges=true;
  } else {
  //SIGHT_VERB(dbg<<"#cache_outEdges="<<cache_outEdges.size()<<endl, 2, DPEAnalDebugLevel)
    SIGHT_VERB_IF(2, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
    dbg << "Cached: "<<endl;
    for(list<PartEdgePtr>::iterator e=cache_outEdges.begin(); e!=cache_outEdges.end(); ++e)
      dbg << e->str()<<endl;
#endif
    SIGHT_VERB_FI()
  }
  return cache_outEdges;
}

list<PartEdgePtr> DeadPathElimPart::inEdges()
{
  if(!cacheInitialized_inEdges) {
    AnalysisParts parts = getAnalysis()->NodeState2All(getNodeStateLocPart());
    AnalysisPartEdgeLists inEdges = parts.inEdgesAll(*getAnalysis());
    /*list<PartEdgePtr> baseEdges      = getNodeStateLocPart()->inEdges();
    list<PartEdgePtr> baseSuperEdges = getInput()->inEdges();
    assert(baseEdges.size() == baseSuperEdges.size());*/

  //  scope reg(txt()<<"DeadPathElimPart::inEdges() #baseEdges="<<baseEdges.size(), scope::medium, attrGE("DPEAnalDebugLevel", 2));

    // Since this is a forward analysis, information from preceding parts is aggregated under the NULL edge
    // of this part. As such, to get the parts that lead to this part we need to iterate over the incoming edges
    // and then look at the parts they arrive from.
    /*list<PartEdgePtr>::iterator be=baseEdges.begin(), beSuper=baseSuperEdges.begin();
    for(; beSuper!=baseSuperEdges.end(); ++be, ++beSuper) {*/
    for(AnalysisPartEdgeLists::iterator edge=inEdges.begin(); edge!=inEdges.end(); ++edge) {
      SIGHT_VERB(dbg << "edge->NodeState()="<<edge->NodeState()->str()<<endl, 2, DPEAnalDebugLevel)
      NodeState* inState = NodeState::getNodeState(analysis, edge->NodeState()->source());
      SIGHT_VERB_IF(2, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
      scope inscope("inState", scope::low);
      dbg << inState->str()<<endl;
#endif
      SIGHT_VERB_FI()
      DeadPathElimPartEdge* inPartEdge = dynamic_cast<DeadPathElimPartEdge*>(inState->getLatticeBelow(analysis, edge->index(), 0));
      assert(inPartEdge);
      SIGHT_VERB(dbg << "inPartEdge="<<inPartEdge->str()<<endl, 2, DPEAnalDebugLevel)

      if(inPartEdge->level==live)
        // Create a new DeadPathElimPartEdgePtr from an existing inPartedge. To ensure that the
        // original is not deallocated when the shared pointer goes out of scope, we keep the shared
        // pointer in a cache data structure that persists.
        //cache_inEdges.push_back(initPtr(dynamic_cast<DeadPathElimPartEdge*>(inPartEdge->copy())));
        cache_inEdges.push_back(DeadPathElimPartEdge::raw2shared(dynamic_cast<DeadPathElimPartEdge*>(inPartEdge)));
    }
    cacheInitialized_inEdges=true;
  }
  return cache_inEdges;
}

set<CFGNode> DeadPathElimPart::CFGNodes() const
{
  if(!cacheInitialized_CFGNodes) {
    const_cast<DeadPathElimPart*>(this)->cache_CFGNodes = getInputPart()->CFGNodes();
    const_cast<DeadPathElimPart*>(this)->cacheInitialized_CFGNodes = true;
  }
  return cache_CFGNodes;
  //return getSupersetPart()->CFGNodes();
}

// If this Part corresponds to a function call/return, returns the set of Parts that contain
// its corresponding return/call, respectively.
set<PartPtr> DeadPathElimPart::matchingCallParts() const {
  if(!cacheInitialized_matchingCallParts) {
    // Wrap the parts returned by the call to the parent Part with DeadPathElimPart
    set<PartPtr> parentMatchParts = getInputPart()->matchingCallParts();
    for(set<PartPtr>::iterator mp=parentMatchParts.begin(); mp!=parentMatchParts.end(); mp++) {
      const_cast<DeadPathElimPart*>(this)->cache_matchingCallParts.insert(DeadPathElimPart::create(*mp, analysis));
    }
    const_cast<DeadPathElimPart*>(this)->cacheInitialized_matchingCallParts=true;
  }
  return cache_matchingCallParts;
}

// If this Part corresponds to a function entry/exit, returns the set of Parts that contain
// its corresponding exit/entry, respectively.
set<PartPtr> DeadPathElimPart::matchingEntryExitParts() const {
  if(!cacheInitialized_matchingEntryExitParts) {
    // Wrap the parts returned by the call to the parent Part with DeadPathElimPart
    set<PartPtr> parentMatchParts = getInputPart()->matchingEntryExitParts();
    for(set<PartPtr>::iterator mp=parentMatchParts.begin(); mp!=parentMatchParts.end(); mp++) {
      const_cast<DeadPathElimPart*>(this)->cache_matchingEntryExitParts.insert(DeadPathElimPart::create(*mp, analysis));
    }
    const_cast<DeadPathElimPart*>(this)->cacheInitialized_matchingEntryExitParts=true;
  }
  return cache_matchingEntryExitParts;
}


/*
// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    Part that denotes a subset of A (the function is called on this part),
//    it returns a list of Parts that partition O.
list<PartPtr> DeadPathElimPart::getOperandPart(SgNode* anchor, SgNode* operand)
{
  if(level==live) {
    list<PartPtr> baseOpParts = base->getOperandPart(anchor, operand);
    list<PartPtr> dpeOpParts;
    for(list<PartPtr>::iterator p=baseOpParts.begin(); p!=baseOpParts.end(); p++) {
      NodeState* inState = NodeState::getNodeState(analysis, (*be)->source());
      DeadPathElimPart* inPart = dynamic_cast<DeadPathElimPart*>(inState->getLatticeAbove(analysis, *be, 0));
      assert(inPart);
      dpeOpParts = make_part<DeadPathElimParts>()
    }
    return baseOpParts;
  } else {
    list<PartPtr> emptyL;
    return emptyL;
  }
}
*/
// Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
PartEdgePtr DeadPathElimPart::inEdgeFromAny() {
  if(!cacheInitialized_inEdgeFromAny) {
    /*scope("DeadPathElimPart::inEdgeFromAny()");
    dbg << "    getNodeStateLocPart()="<<getNodeStateLocPart()->str()<<endl;
    dbg << "    getSupersetPart()="<<getSupersetPart()->str()<<endl;*/
    cache_inEdgeFromAny = DeadPathElimPartEdge::create(getNodeStateLocPart()->inEdgeFromAny(),
                                                       getInputPart()->inEdgeFromAny(), analysis);
    cacheInitialized_inEdgeFromAny=true;
  }
  return cache_inEdgeFromAny;
}

// Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
PartEdgePtr DeadPathElimPart::outEdgeToAny() {
  if(!cacheInitialized_outEdgeToAny) {
    cache_outEdgeToAny = DeadPathElimPartEdge::create(getNodeStateLocPart()->outEdgeToAny(),
                                                      getInputPart()->outEdgeToAny(), analysis);
    cacheInitialized_outEdgeToAny=true;
  }
  return cache_outEdgeToAny;
}

bool DeadPathElimPart::equal(const PartPtr& o) const
{
  const DeadPathElimPartPtr that = dynamicConstPtrCast<DeadPathElimPart>(o);
  assert(that.get());
  assert(analysis == that->analysis);

  if(cache_equal.find(that.get()) == cache_equal.end())
    const_cast<DeadPathElimPart*>(this)->cache_equal[that.get()] = (getInputPart() == that->getInputPart());
  return const_cast<DeadPathElimPart*>(this)->cache_equal[that.get()];
}

bool DeadPathElimPart::less(const PartPtr& o) const
{
  const DeadPathElimPartPtr that = dynamicConstPtrCast<DeadPathElimPart>(o);
  assert(that.get());
  assert(analysis == that->analysis);

  if(cache_less.find(that.get()) == cache_less.end())
    const_cast<DeadPathElimPart*>(this)->cache_less[that.get()] = (getInputPart() < that->getInputPart());
  return const_cast<DeadPathElimPart*>(this)->cache_less[that.get()];
}

// Pretty print for the object
std::string DeadPathElimPart::str(std::string indent) const
{
  ostringstream oss;
  oss << "[DPEPart: "<<getInputPart()->str()<<"]";
  return oss.str();
}

/************************************
 ***** DeadPathElimPartEdge *****
 ************************************/
/* GB 2012-10-15 - Commented out because this constructor makes it difficult to set the lattice of the created edge
DeadPathElimPartEdge::DeadPathElimPartEdge(DeadPathElimPartPtr src, DeadPathElimPartPtr tgt,
                                           PartEdgePtr baseEdge, DeadPatComposedAnalysishElimAnalysis* analysis) :
    Lattice(baseEdge), FiniteLattice(baseEdge), baseEdge(baseEdge), src(src), tgt(tgt), level(bottom), analysis(analysis)
{}*/

// Constructor to be used when constructing the edges (e.g. from genInitLattice()).
DeadPathElimPartEdge::DeadPathElimPartEdge(PartEdgePtr NodeStateLocPartEdge, PartEdgePtr inputPartEdge, ComposedAnalysis* analysis, DPELevel level) :
        Lattice(NodeStateLocPartEdge), FiniteLattice(NodeStateLocPartEdge), PartEdge(analysis, NodeStateLocPartEdge, inputPartEdge)
{
  src = inputPartEdge->source() ? DeadPathElimPart::create(inputPartEdge->source(), analysis) : dynamicPtrCast<DeadPathElimPart>(NULLPart);
  tgt = inputPartEdge->target() ? DeadPathElimPart::create(inputPartEdge->target(), analysis) : dynamicPtrCast<DeadPathElimPart>(NULLPart);
  /*dbg << "DeadPathElimPartEdge::DeadPathElimPartEdge()"<<endl;
  dbg << "inputPartEdge="<<inputPartEdge->str()<<endl;
  dbg << "src="<<(src? src->str() : "NULL")<<endl;
  dbg << "tgt="<<(tgt? tgt->str() : "NULL")<<endl;*/

  cacheInitialized_getPredicateValue=false;

  this->level = level;
}

// Constructor to be used when traversing the part graph created by the DeadPathElimAnalysis, after
// all the DeadPathElimPartEdges have been constructed and stored in NodeStates.
DeadPathElimPartEdge::DeadPathElimPartEdge(PartEdgePtr NodeStateLocPartEdge, PartEdgePtr inputPartEdge, ComposedAnalysis* analysis) :
        Lattice(NodeStateLocPartEdge), FiniteLattice(NodeStateLocPartEdge), PartEdge(analysis, NodeStateLocPartEdge, inputPartEdge)
{
  // Look up this edge in the results of the DeadPathElimAnalysis results and copy data from that edge into this object
  //DeadPathElimPartEdge* dpeEdge;
  if(getInputPartEdge()->source()) src=DeadPathElimPart::create(getInputPartEdge()->source(), analysis);
  if(getInputPartEdge()->target()) tgt=DeadPathElimPart::create(getInputPartEdge()->target(), analysis);

  // If the source is not a wildcard, look for the record in the source part, which maintains separate information
  // for all the outgoing edges
  //dbg << "getInputPartEdge()="<<getInputPartEdge()->str()<<endl;
  //dbg << "this="<<str()<<endl;
  // If the edge has a concrete source and target
  if(getInputPartEdge()->source() && getInputPartEdge()->target()) {
    /*DeadPathElimPartPtr sourceDPEPart = DeadPathElimPart::create(getInputPartEdge()->source(), analysis, bottom);
    dbg << "seEdge->source()="<<getInputPartEdge()->source()->str()<<endl;
    dbg << "seEdge->target()="<<getInputPartEdge()->target()->str()<<endl;
    dbg << "sourceDPEPart="<<sourceDPEPart->str()<<endl;*/
    NodeState* state = NodeState::getNodeState(analysis, getNodeStateLocPartEdge()->source());
    /*list<PartEdgePtr> edges = getInputPartEdge()->source()->outEdges();
    dbg << "source->outEdges="<<endl;
    for(list<PartEdgePtr>::iterator e=edges.begin(); e!=edges.end(); e++)
    { dbg << (*e)->str()<<endl; }*/

    SIGHT_VERB_IF(2, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
      dbg << "source state="<<endl<<"        "<<state->str(analysis, "        ")<<endl;
      dbg << "getInputPartEdge()="<<getInputPartEdge()->str("        ")<<endl;
#endif
    SIGHT_VERB_FI()
    // Get the DeadPathElimPartEdge that is stored along getNodeStateLocPartEdge() at the NodeState of its source part
    AnalysisPartEdges pedges = getAnalysis()->NodeState2All(getNodeStateLocPartEdge());
    DeadPathElimPartEdge* dpeEdge = dynamic_cast<DeadPathElimPartEdge*>(state->getLatticeBelow(analysis, pedges.index(), 0));
    SIGHT_VERB(dbg << "dpeEdge lattice = "<<state->getLatticeBelow(analysis, pedges.index(), 0)->str("        ")<<endl, 2, DPEAnalDebugLevel)
    level = dpeEdge->level;
  // If the target is a wildcard look at the source part and aggregate the DPEEdges along all the outgoing paths.
  // The resulting edge is live if any of the outgoing edges are live.
  } else if(getInputPartEdge()->source()) {
    NodeState* state = NodeState::getNodeState(analysis, getNodeStateLocPartEdge()->source());

    AnalysisPartEdges pedges = getAnalysis()->NodeState2All(getNodeStateLocPartEdge());
    DeadPathElimPartEdge* dpeEdge = dynamic_cast<DeadPathElimPartEdge*>(state->getLatticeBelow(analysis, pedges.index(), 0));
    assert(dpeEdge);
    SIGHT_VERB(dbg << "dpeEdge lattice = "<<state->getLatticeBelow(analysis, pedges.index(), 0)->str("        ")<<endl, 2, DPEAnalDebugLevel)
    level = dpeEdge->level;

    // SIGHT_VERB_IF(2, DPEAnalDebugLevel)
    //   dbg << "source state="<<endl<<"        "<<state->str(analysis, "        ")<<endl;
    //   dbg << "getInputPartEdge()="<<getInputPartEdge()->str("        ")<<endl;
    // SIGHT_VERB_FI()


    // // Merge the lattices along all the outgoing edges
    // map<PartEdgePtr, std::vector<Lattice*> >& e2lats = state->getLatticeBelowAllMod(analysis);
    // assert(e2lats.size()>=1);

    // level = dead;
    // // Get the DeadPathElimPartEdge that is stored along getNodeStateLocPartEdge() at the NodeState of its source part
    // AnalysisPartEdges pedges = getAnalysis()->NodeState2All(getNodeStateLocPartEdge());
    // for(map<PartEdgePtr, std::vector<Lattice*> >::iterator lats=e2lats.begin(); lats!=e2lats.end(); lats++) {
    //   PartEdge* edgePtr = lats->first.get();
    //   SIGHT_VERB(dbg << "edgePtr="<<edgePtr->str("        ")<<endl, 2, DPEAnalDebugLevel)
    //   assert(edgePtr->source() == getInputPartEdge().get()->source());

    //   DeadPathElimPartEdge* dpeEdge = dynamic_cast<DeadPathElimPartEdge*>(state->getLatticeBelow(analysis, lats->first, 0));
    //   assert(dpeEdge);

    //   if(dpeEdge->level == live) level = live;
    // }
  // If the source is a wildcard, look for the record in the target part where all the edges are aggregated
  } else if(getInputPartEdge()->target()) {
    assert(getInputPartEdge()->target());
    DeadPathElimPartPtr targetDPEPart = DeadPathElimPart::create(getInputPartEdge()->target(), analysis);
    //#SA: record is aggregated on the wildcard edge
    NodeState* state = NodeState::getNodeState(analysis, getNodeStateLocPartEdge()->target());
    AnalysisPartEdges pedges = getAnalysis()->NodeState2All(getNodeStateLocPartEdge());
    DeadPathElimPartEdge* dpeEdge = dynamic_cast<DeadPathElimPartEdge*>(state->getLatticeAbove(analysis, pedges.index(), 0));
    // If the analysis recorded info at this edge, use its level
    if(dpeEdge) level = dpeEdge->level;
    // Otherwise, if the DPE analysis was never run here, it must be dead
    else        level = dead;
  }
  //assert(dpeEdge);

  cacheInitialized_getPredicateValue=false;
}

DeadPathElimPartEdge::DeadPathElimPartEdge(const DeadPathElimPartEdge& that) :
  Lattice(that.latPEdge),
  FiniteLattice(that.latPEdge),
  PartEdge((const PartEdge&)that),
  src(that.src), tgt(that.tgt), level(that.level)
{
  cache_getOperandPartEdge           = that.cache_getOperandPartEdge;
  cache_getPredicateValue            = that.cache_getPredicateValue;
  cacheInitialized_getPredicateValue = that.cacheInitialized_getPredicateValue;
}

// Returns a shared pointer to this of type DeadPathElimPartEdgePtr
DeadPathElimPartEdgePtr DeadPathElimPartEdge::get_shared_this()
{ return dynamicPtrCast<DeadPathElimPartEdge>(makePtrFromThis(shared_from_this())); }


PartPtr DeadPathElimPartEdge::source() const
{ return src; }

PartPtr DeadPathElimPartEdge::target() const
{ return tgt; }

// Overload the setPartEdge (from Lattice), setSupersetPartEdge and setNodeStateLocPartEdge (from PartEdge) methods to ensure that they
// are always set in a consistent manner regardless of which one is called
// Sets the PartEdge that this Lattice's information corresponds to.
// Returns true if this causes the edge to change and false otherwise
bool DeadPathElimPartEdge::setPartEdge(PartEdgePtr latPEdge)
{
  bool modified = Lattice::setPartEdge(latPEdge);
  PartEdge::setNodeStateLocPartEdge(latPEdge);
  return modified;
}

// Sets this Part's parent
void DeadPathElimPartEdge::setInputPartEdge(PartEdgePtr parent)
{
  PartEdge::setInputPartEdge(parent);
}

// Sets this PartEdge's NodeStateLocPartEdge
void DeadPathElimPartEdge::setNodeStateLocPartEdge(PartEdgePtr parent)
{
  PartEdge::setNodeStateLocPartEdge(parent);
}

// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    PartEdge that denotes a subset of A (the function is called on this PartEdge),
//    it returns a list of PartEdges that partition O.
std::list<PartEdgePtr> DeadPathElimPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand)
{
  if(cache_getOperandPartEdge.find(anchor) == cache_getOperandPartEdge.end() ||
     cache_getOperandPartEdge[anchor].find(operand) == cache_getOperandPartEdge[anchor].end()) {

    // operand precedes anchor in the CFG, either immediately or at some distance. As such, the edge
    //   we're looking for is not necessarily the edge from operand to anchor but rather the first
    //   edge along the path from operand to anchor. Since operand is part of anchor's expression
    //   tree we're guaranteed that there is only one such path.
    // The implementor of the partition we're running on may have created multiple parts for
    //   operand to provide path sensitivity and indeed, may have created additional outgoing edges
    //   from each of the operand's parts. Fortunately, since in the original program the original
    //   edge led from operand to anchor and the implementor of the partition could have only hierarchically
    //   refined the original partition, all the new edges must also lead from operand to anchor.
    //   As such, the returned list contains all the outgoing edges from all the parts that correspond
    //   to operand.
    // Note: if the partitioning process is not hierarchical we may run into minor trouble since the
    //   new edges from operand may lead to parts other than anchor. However, this is just an issue
    //   of precision since we'll account for paths that are actually infeasible.

    // The target of this edge identifies the termination point of all the execution prefixes
    // denoted by this edge. We thus use it to query for the parts of the operands and only both
    // if this part is itself live.
    SIGHT_VERB_DECL(scope, ("DeadPathElimPartEdge::getOperandPartEdge()", scope::medium), 1, DPEAnalDebugLevel)
    SIGHT_VERB(dbg << "anchor="<<SgNode2Str(anchor)<<" operand="<<SgNode2Str(operand)<<endl, 1, DPEAnalDebugLevel)
    SIGHT_VERB(dbg << "this="<<str()<<endl, 1, DPEAnalDebugLevel)

    if(level==live) {
      SIGHT_VERB(dbg << "getNodeStateLocPartEdge="<<getNodeStateLocPartEdge()->str()<<endl, 1, DPEAnalDebugLevel)
      std::list<PartEdgePtr> atsLocEdges = getNodeStateLocPartEdge()->getOperandPartEdge(anchor, operand);
      SIGHT_VERB(dbg << "getInputPartEdge="<<getInputPartEdge()->str()<<endl, 1, DPEAnalDebugLevel)
      std::list<PartEdgePtr> inputEdges = getInputPartEdge()->getOperandPartEdge(anchor, operand);
      if(atsLocEdges.size() != inputEdges.size()) {
#ifndef DISABLE_SIGHT
        dbg << "ERROR: #atsLocEdge("<<atsLocEdges.size()<<") != #inputEdges("<<inputEdges.size()<<")"<<endl;
        { scope s("atsLocEdges");
        for(list<PartEdgePtr>::iterator e=atsLocEdges.begin(); e!=atsLocEdges.end(); ++e)
          dbg << (*e)->str()<<endl; }
        { scope s("inputEdges");
          for(list<PartEdgePtr>::iterator e=inputEdges.begin(); e!=inputEdges.end(); ++e)
            dbg << (*e)->str()<<endl; }
#endif

        cerr << "ERROR: #atsLocEdge("<<atsLocEdges.size()<<") != #inputEdges("<<inputEdges.size()<<")"<<endl;
        { cerr << "atsLocEdges"<<endl;
        for(list<PartEdgePtr>::iterator e=atsLocEdges.begin(); e!=atsLocEdges.end(); ++e)
          cerr << "    "<< (*e)->str()<<endl; }
        { cerr << "inputEdges"<<endl;
          for(list<PartEdgePtr>::iterator e=inputEdges.begin(); e!=inputEdges.end(); ++e)
            cerr << "    "<< (*e)->str()<<endl; }
      }
      assert(atsLocEdges.size() == inputEdges.size());
      std::list<PartEdgePtr>::iterator atsLocE=atsLocEdges.begin(),
                                       inputE=inputEdges.begin();
      for(; atsLocE!=atsLocEdges.end(); ++atsLocE, ++inputE) {
        SIGHT_VERB(dbg << "atsLocE="<<(*atsLocE)->str()<<endl, 1, DPEAnalDebugLevel)
        PartEdgePtr dpeEdge = DeadPathElimPartEdge::create(*atsLocE, *inputE, analysis);
        SIGHT_VERB_IF(1, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
        scope reg("dpeEdge", scope::low);
        dbg<<dpeEdge->str()<<endl;
#endif
        SIGHT_VERB_FI()
        cache_getOperandPartEdge[anchor][operand].push_back(dpeEdge);
      }
  /*
      for(list<PartPtr>::iterator opP=opParts.begin(); opP!=opParts.end(); opP++) {
        list<PartEdgePtr> edges = (*opP)->outEdges();
        for(list<PartEdgePtr>::iterator e=edges.begin(); e!=edges.end(); e++) {
           2*dbg << "opP = "<<(*opP)->str()<<endl;
          dbg << "e = "<<(*e)->str()<<endl;
          dbg << "e->target() = "<<(*e)->target()->str()<<endl;* /
          assert(src || tgt);
          DeadPathElimAnalysis* analysis = (src? src->analysis : tgt->analysis);
          PartEdgePtr edge = DeadPathElimPartEdge::create(DeadPathElimPart::create((*opP)->inEdgeFromAny(), analysis),
                                                            DeadPathElimPart::create((*e)->target()->inEdgeFromAny(), analysis));
          //dbg << "edge = "<<edge->str()<<endl;
          l.push_back(edge);
        }
      }
      return l;*/
    } else {
      list<PartEdgePtr> emptyL;
      cache_getOperandPartEdge[anchor][operand] = emptyL;
    }
  }
  return cache_getOperandPartEdge[anchor][operand];
}

// If the source Part corresponds to a conditional of some sort (if, switch, while test, etc.)
// it must evaluate some predicate and depending on its value continue, execution along one of the
// outgoing edges. The value associated with each outgoing edge is fixed and known statically.
// getPredicateValue() returns the value associated with this particular edge. Since a single
// Part may correspond to multiple CFGNodes getPredicateValue() returns a map from each CFG node
// within its source part that corresponds to a conditional to the value of its predicate along
// this edge.
std::map<CFGNode, boost::shared_ptr<SgValueExp> > DeadPathElimPartEdge::getPredicateValue()
{
  if(!cacheInitialized_getPredicateValue) {
    cache_getPredicateValue = getNodeStateLocPartEdge()->getPredicateValue();
    cacheInitialized_getPredicateValue = true;
  }
  return cache_getPredicateValue;
  //return latPEdge->getPredicateValue();
}

// Adds a mapping from a CFGNode to the outcome of its predicate
void DeadPathElimPartEdge::mapPred2Val(CFGNode n, boost::shared_ptr<SgValueExp> val)
{
  predVals[n] = val;
}

// Empties out the mapping of CFGNodes to the outcomes of their predicates
void DeadPathElimPartEdge::clearPred2Val()
{
  predVals.clear();
}

bool DeadPathElimPartEdge::equal(const PartEdgePtr& o) const
{
  const DeadPathElimPartEdgePtr that = dynamicConstPtrCast<DeadPathElimPartEdge>(o);
  assert(that.get());
  /*if(latPEdge==that->latPEdge) {
    assert(src==that->src);
    assert(tgt==that->tgt);
    return true;
  } else
    return false;*/
  return src==that->src && tgt==that->tgt;
}

bool DeadPathElimPartEdge::less(const PartEdgePtr& o)  const
{
  const DeadPathElimPartEdgePtr that = dynamicConstPtrCast<DeadPathElimPartEdge>(o);
  assert(that.get());

  //return latPEdge < that->latPEdge;
  return (src < that->src) ||
         (src==that->src && tgt<that->tgt);
}

// Pretty print for the object
std::string DeadPathElimPartEdge::str(std::string indent) const
{
  static bool recursive = false;
  if(recursive) return "#REC#";

  recursive = true;
  ostringstream oss;

  //cout << "latPEdge="<<latPEdge->str()<<endl;
  //cout << "getNodeStateLocPartEdge()="<<getNodeStateLocPartEdge()->str()<<endl;


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * This check should be reinstated but removing it for now to work on other stuff
   if(latPEdge != getNodeStateLocPartEdge()) {
    dbg << "DeadPathElimPartEdge"<<endl;
    dbg << "this="<<"[DPEEdge("<<(level==dead? "D": (level==live? "L": (level==bottom? "B": "<font color=\"#FF0000\"><b>??? </b></font>")))<<"): "<<
                      (src ? src->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;"): "NULL")<<" ==&gt; " <<
                      (tgt ? tgt->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;"): "NULL")<<endl;
    dbg << "latPEdge="<<latPEdge->str()<<endl;
    dbg << "getNodeStateLocPartEdge()="<<getNodeStateLocPartEdge()->str()<<endl;
    dbg << "getSupersetPartEdge()="<<getSupersetPartEdge()->str()<<endl;
  }
  assert(latPEdge == getNodeStateLocPartEdge());*/
  oss << "[DPEEdge<"<<this<<"> ("<<(level==dead? "D": (level==live? "L": (level==bottom? "B": "<font color=\"#FF0000\"><b>??? </b></font>")))<<"): "<<
                      (src ? src->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;"): "NULL")<<" ==&gt; " <<
                      (tgt ? tgt->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;"): "NULL")<<
                      ", "<<endl;
  oss << indent << "    parent=<"<<getInputPartEdge()->str()<<"]";
  oss << indent << "    location=<"<<getNodeStateLocPartEdge()->str()<<"]";
  recursive = false;

  return oss.str();
}

// ----------------------------------------------
// Functions that need to be defined for Lattices
// ----------------------------------------------

void DeadPathElimPartEdge::initialize() { }

// Returns a copy of this lattice
Lattice* DeadPathElimPartEdge::copy() const
{ return DeadPathElimPartEdge::createRaw(*this); }

// Overwrites the state of "this" Lattice with "that" Lattice
void DeadPathElimPartEdge::copy(Lattice* that_arg)
{
  Lattice::copy(that_arg);

  DeadPathElimPartEdge* that = dynamic_cast<DeadPathElimPartEdge*>(that_arg);
  assert(that);
  assert(PartEdge::compatible(*that));

  src   = that->src;
  tgt   = that->tgt;
  level = that->level;
}

bool DeadPathElimPartEdge::operator==(Lattice* that_arg) /*const*/
{
  // NOTE: because Lattices use pointers and Parts use boost::shared_ptrs we can't take advantage
  // of the base operator== from PartEdge. However, in this case this does not matter since Lattices
  // from different analyses can never be compared.
  DeadPathElimPartEdge* that = dynamic_cast<DeadPathElimPartEdge*>(that_arg);
  assert(that);
  assert(analysis == that->analysis);

  /*if(latPEdge==that->latPEdge) {
    assert(src==that->src);
    assert(tgt==that->tgt);
    return true;
  } else
    return false;*/
  return src==that->src && tgt==that->tgt;
}

// Called by analyses to transfer this lattice's contents from across function scopes from a caller function
//    to a callee's scope and vice versa. If this this lattice maintains any information on the basis of
//    individual MemLocObjects these mappings must be converted, with MemLocObjects that are keys of the ml2ml
//    replaced with their corresponding values. If a given key of ml2ml does not appear in the lattice, it must
//    be added to the lattice and assigned a default initial value. In many cases (e.g. over-approximate sets
//    of MemLocObjects) this may not require any actual insertions. If the value of a given ml2ml mapping is
//    NULL (empty boost::shared_ptr), any information for MemLocObjects that must-equal to the key should be
//    deleted.
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    the keys in ml2ml are in scope on one side of Part, while the values on the other side. Specifically, it is
//    guaranteed that the keys are in scope at the edge returned by getPartEdge() while the values are in scope
//    at newPEdge.
// remapML must return a freshly-allocated object.
Lattice* DeadPathElimPartEdge::remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr newPEdge) {
  return copy();
}

// Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously
//    maintained in this lattice about them.
// Returns true if the Lattice state is modified and false otherwise.
bool DeadPathElimPartEdge::replaceML(Lattice* newL)
{
  copy(newL);

  return false;
}

// Computes the meet of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool DeadPathElimPartEdge::meetUpdate(Lattice* that_arg)
{
  DeadPathElimPartEdge* that = dynamic_cast<DeadPathElimPartEdge*>(that_arg);
  assert(that);
  // We don't check this becase when we meet information from a caller with information from a callee its a pain
  // to convert the edges from caller scope to callee scope, although this may be a good idea in the future to
  // clean up the code.
  //assert(latPEdge==that->latPEdge);
  // We don't need to make sure that the sources are the same since they will be wildcards but will not necessarily be equal to each other
  //assert(src==that->src);
  //assert(tgt==that->tgt);
  assert(analysis==that->analysis);

  // The result of the meet is the max of the lattice points of the two arguments
  bool modified = (level<that->level);
  SIGHT_VERB(dbg << "DeadPathElimPartEdge::meetUpdate() level="<<level<<" that->level="<<that->level<<" newLevel="<<(level<that->level? that->level: level)<<endl, 1, DPEAnalDebugLevel)
  level = (level<that->level? that->level: level);

  // Copy the new level to the source and target of the edge
  /*if(src) src->level = level;
  if(tgt) tgt->level = level;*/

  // Union the predVals maps
  for(map<CFGNode, boost::shared_ptr<SgValueExp> >::iterator v=that->predVals.begin(); v!=that->predVals.end(); v++) {
    // If both edges have a mapping for the current CFGNode, they must be the same
    if(predVals.find(v->first) != predVals.end())
      assert(ValueObject::equalValueExp(predVals[v->first].get(), v->second.get()));
    // Otherwise, add the new mapping to predVals
    else {
      predVals[v->first] = v->second;
      modified = true;
    }
  }

  SIGHT_VERB(dbg << "DeadPathElimPartEdge::meetUpdate() final="<<str()<<endl, 1, DPEAnalDebugLevel)

  return modified;
}

// Computes the intersection of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool DeadPathElimPartEdge::intersectUpdate(Lattice* that_arg)
{
  // We will almost certainly not need to use intersections for this object
  assert(0);
}

// Set this Lattice object to represent the set of all possible execution prefixes.
// Return true if this causes the object to change and false otherwise.
bool DeadPathElimPartEdge::setToFull()
{
  bool modified = level!=live;
  level = live;
  return modified;
}

// Set this Lattice object to represent the of no execution prefixes (empty set)
// Return true if this causes the object to change and false otherwise.
bool DeadPathElimPartEdge::setToEmpty()
{
  bool modified = level!=bottom;
  level = bottom;
  return modified;
}

// Set all the information associated Lattice object with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
bool DeadPathElimPartEdge::setMLValueToFull(MemLocObjectPtr ml)
{
  // Do nothing since this object does not contain information about MemLocObjects
  return false;
}

// Set this Lattice object to represent a dead part
// Return true if this causes the object to change and false otherwise.
bool DeadPathElimPartEdge::setToDead()
{
  bool modified = level!=bottom;
  level = dead;
  return modified;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool DeadPathElimPartEdge::isFull()
{ return level == live; }

// Returns whether this lattice denotes the empty set.
bool DeadPathElimPartEdge::isEmpty()
{ return level == bottom; }

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool DeadPathElimPartEdge::isFull(PartEdgePtr pedge)
{ return level == live; }

// Returns whether this lattice denotes the empty set.
bool DeadPathElimPartEdge::isEmpty(PartEdgePtr pedge)
{ return level == bottom; }

/*TO DO LIST
----------
- extend ValueObjectPtr to provide the cardinality of the set, a way to enumerate it if finite
- update stx_analysis.C::isLive to use Method 3, using the above API*/

/********************************
 ***** DeadPathElimAnalysis *****
 ********************************/

boost::shared_ptr<DFTransferVisitor> DeadPathElimAnalysis::getTransferVisitor(
                                   AnalysisParts& parts,
                                   CFGNode cn, NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)
{
  return boost::shared_ptr<DeadPathElimTransfer>(new DeadPathElimTransfer(parts, cn, state, dfInfo, this));
}

/********************************
 ***** DeadPathElimTransfer *****
 ********************************/

DeadPathElimTransfer::DeadPathElimTransfer(AnalysisParts& parts, CFGNode cn, NodeState &s,
                                           std::map<PartEdgePtr, std::vector<Lattice*> > &dfInfo, DeadPathElimAnalysis* dpea)
   : DFTransferVisitor(parts, cn, s, dfInfo),
     dpea(dpea),
     modified(false)
{ }

bool DeadPathElimTransfer::finish() {
  return modified;

  // !!! WHY AREN'T WE DEALLOCATING DeadPathElimPartEdge* dfEdge = dynamic_cast<DeadPathElimPartEdge*>(*dfInfo[NULLPartEdge].begin());????
}

// General function for SgNodes with 2 outgoing edges, where the first edge must/may be taken when some value (provided)
// is definitely true and the second edge must/may taken when this value is false.
// trueBranchMayMust - set to may/must if the true branch is taken only if the value may/must be true
// falseBranchMayMust - set to may/must if the false branch is taken only if the value may/must be false
void DeadPathElimTransfer::visit2OutNode(SgNode* sgn, ValueObjectPtr val, maymust trueBranchMayMust, maymust falseBranchMayMust) {
  SIGHT_VERB_DECL(scope, ("visit2OutNode", scope::medium), 1, DPEAnalDebugLevel);
  // If the conditional has a concrete value, replace the NULL-keyed dfInfo with two copies of the lattice for each
  // successor, one of which is live and the other dead
  SIGHT_VERB(dbg << "val="<<val->str()<<", val->isConcrete()="<<val->isConcrete()<<", trueBranchMayMust="<<(trueBranchMayMust==may?"may":"must")<<", falseBranchMayMust="<<(falseBranchMayMust==may?"may":"must")<<endl, 1, DPEAnalDebugLevel)
  if(val->isConcrete()) {
    set<boost::shared_ptr<SgValueExp> > concreteVals = val->getConcreteValue();
    SIGHT_VERB(dbg << "#concreteVals="<<concreteVals.size()<<endl, 1, DPEAnalDebugLevel)

    // If there is just one option and it is interpretable as a boolean
    if(concreteVals.size()==1 && ValueObject::isValueBoolCompatible(*concreteVals.begin())) {
      boost::shared_ptr<SgValueExp> concreteVal = *concreteVals.begin();

      // Get the edge that is propagated along the incoming dataflow path
      //#SA: Incoming dfInfo is associated with inEdgeFromAny
      DeadPathElimPartEdge* dfEdge = dynamic_cast<DeadPathElimPartEdge*>(*dfInfo[parts.index()->inEdgeFromAny()].begin());
      // Adjust the base Edge so that it now starts at its original target part and terminates at NULL
      // (i.e. advance it forward by one node without specifying the target yet)
      dfEdge->src = dfEdge->tgt;
      dfEdge->tgt = NULLPart;
      dfEdge->clearPred2Val(); // Reset its predicate values

      // Record the lattice value of the incoming edge
      DPELevel dfLevel = dfEdge->level;

      // Empty out dfInfo in preparation of it being overwritten
      dfInfo.clear();

      // The concrete value inside val
      bool IfPredValue = ValueObject::SgValue2Bool(concreteVal);

      // Consider all the source part's outgoing edges (implemented by a server analysis)
      AnalysisPartEdgeLists outE = parts.outEdgesIndexInput(*dpea->getComposer());///*NodeState_valid*/ false, /*indexes_valid*/ true, /*inputs_valid*/ true);
      SIGHT_VERB(dbg << "IfPredValue="<<IfPredValue<<" outE.size()="<<outE.size()<<endl, 1, DPEAnalDebugLevel)
      assert(outE.size()==1 || outE.size()==2);
      for(AnalysisPartEdgeLists::iterator edge=outE.begin(); edge!=outE.end(); ++edge) {
        std::map<CFGNode, boost::shared_ptr<SgValueExp> > pv = edge->input()->getPredicateValue();
        SIGHT_VERB_IF(1, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
          dbg << "e="<<edge->input()->str()<<endl;
          dbg << "cn="<<CFGNode2Str(cn)<<" pv="<<endl;
          for(map<CFGNode, boost::shared_ptr<SgValueExp> >::iterator v=pv.begin(); v!=pv.end(); v++)
          { indent ind; dbg << CFGNode2Str(v->first) << "("<<(v->first==cn)<<"|"<<(v->first.getNode()==cn.getNode())<<") =&gt; "<<SgNode2Str(v->second.get())<<endl; }
#endif
        SIGHT_VERB_FI()

        assert(pv.find(cn) != pv.end());
        assert(ValueObject::isValueBoolCompatible(pv[cn]));

        // Create a DeadPathElimPartEdge to wrap this server analysis-implemented edge
        DeadPathElimPartEdge* dpeEdge;
        // If this is the first edge to synthesize, make the dfEdge into true branch DeadPathElimPartEdge
        if(dfInfo.size()==0) dpeEdge = dfEdge;
        else                 dpeEdge = dynamic_cast<DeadPathElimPartEdge*>(dfEdge->copy());
        assert(dfEdge);

        // If the current edge corresponds to the true branch
        if(ValueObject::SgValue2Bool(pv[cn])) {
          // Set the level of the true edge to live/dead if the outcome of this conditional is true/false
          // and the incoming edge was live
          if(IfPredValue==true || (IfPredValue==false && trueBranchMayMust==may))
            dpeEdge->level = (dfLevel==live? live: dfLevel);
          else
            dpeEdge->level = (dfLevel==live? dead: dfLevel);

          // Add the true predicate mapping to this edge
          dpeEdge->mapPred2Val(cn, boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));

          SIGHT_VERB(dbg << "True Edge="<<dpeEdge->str()<<endl, 1, DPEAnalDebugLevel)
        // Else, if the current edge corresponds to the false branch
        } else {
          // Set the level of the true edge to live/dead if the outcome of this conditional is true/false
          // and the incoming edge was live
          if(IfPredValue==false || (IfPredValue==true && falseBranchMayMust==may))
            dpeEdge->level = (dfLevel==live? live: dfLevel);
          else
            dpeEdge->level = (dfLevel==live? dead: dfLevel);

          // Add the false predicate mapping to this edge
          dpeEdge->mapPred2Val(cn, boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));

          SIGHT_VERB(dbg << "False Edge="<<dpeEdge->str()<<endl, 1, DPEAnalDebugLevel)
        }

        // Set this dpeEdge's target to be the same as the target of the current server edge but using the edge's level
        dpeEdge->tgt = DeadPathElimPart::create(edge->input()->target(), dpea);

        // Set this dpeEdges's baseEdge to be the current edge using both Lattice API (setPartEdge) and Part API (setParent)
//%%% keep setParent since *e corresponds to the ATS to be refined
//%%% remove setPartEdge() since that edge should be the one from the ATS on top of which we're running. That should be set by the ComposedAnalysis.
//        dpeEdge->setPartEdge(*e);
        dpeEdge->setInputPartEdge(edge->input());

        // Add the current DeadPathElimPartEdge to dfInfo
        vector<Lattice *> dfLatVec;
        dfLatVec.push_back(dpeEdge);
        dfInfo[edge->index()] = dfLatVec;
      }

      modified = true;
    } else
      visit((SgNode*)sgn);
  } else
    visit((SgNode*)sgn);
}

void DeadPathElimTransfer::visit(SgIfStmt *sgn)
{
  SIGHT_VERB(dbg << "DeadPathElimTransfer::visit(SgIfStmt), conditional="<<SgNode2Str(sgn->get_conditional())<<" isSgExprStmt="<<isSgExprStatement(sgn->get_conditional())<<endl, 1, DPEAnalDebugLevel)
  if(SgExprStatement* es=isSgExprStatement(sgn->get_conditional())) {
#ifndef DISABLE_SIGHT
    indent ind;
#endif
    // Get the value of the predicate test in the SgIfStmt's conditional
    ValueObjectPtr val = dpea->getComposer()->OperandExpr2Val(sgn, es->get_expression(), parts.NodeState()->inEdgeFromAny(), dpea);
    visit2OutNode(sgn, val, must, must);
  } else {
    visit((SgNode*)sgn);
    assert(0);
  }
}

void DeadPathElimTransfer::visit(SgAndOp *op)
{
  SIGHT_VERB(dbg << "DeadPathElimTransfer::visit(SgAndOp), cn="<<CFGNode2Str(cn)<<endl, 1, DPEAnalDebugLevel)
  // If this is the portion of the short-circuit operation after the first argument was evaluated but before
  // the second argument
  if(cn.getIndex()==1) {
    ValueObjectPtr val = dpea->getComposer()->OperandExpr2Val(op, op->get_lhs_operand(), parts.NodeState()->inEdgeFromAny(), dpea);
    visit2OutNode(op, val, may, must);
  } else {
    visit((SgNode*)op);
  }
}

void DeadPathElimTransfer::visit(SgOrOp *op)
{
  SIGHT_VERB(dbg << "DeadPathElimTransfer::visit(SgOrOp), cn="<<CFGNode2Str(cn)<<endl, 1, DPEAnalDebugLevel)
  // If this is the portion of the short-circuit operation after the first argument was evaluated but before
  // the second argument
  if(cn.getIndex()==1) {
    ValueObjectPtr val = dpea->getComposer()->OperandExpr2Val(op, op->get_lhs_operand(), parts.NodeState()->inEdgeFromAny(), dpea);
    visit2OutNode(op, val, must, may);
  } else {
    visit((SgNode*)op);
  }
}

void DeadPathElimTransfer::visit(SgNode *sgn)
{
  SIGHT_VERB_DECL(scope, (txt()<<"DeadPathElimTransfer::visit("<<SgNode2Str(sgn)<<")"), 1, DPEAnalDebugLevel)

  // Get the edge that is propagated along the incoming dataflow path
  //#SA: Incoming dfInfo is associated with inEdgeFromAny
  DeadPathElimPartEdge* dfEdge = dynamic_cast<DeadPathElimPartEdge*>(*dfInfo[parts.index()->inEdgeFromAny()].begin());
  SIGHT_VERB(dbg << "Initial dfEdge="<<dfEdge->str()<<endl, 1, DPEAnalDebugLevel)
  // Adjust the base Edge so that it now starts at its original target part and terminates at NULL
  // (i.e. advance it forward by one node without specifying the target yet)
  dfEdge->src = dfEdge->tgt;
  dfEdge->tgt = NULLPart;
  dfEdge->clearPred2Val();
  SIGHT_VERB(dbg << "Final dfEdge="<<dfEdge->str()<<endl, 1, DPEAnalDebugLevel)

  // Consider all the source part's outgoing edges (implemented by a server analysis)  
  std::list<PartEdgePtr> baseEdges = parts.input()->outEdges();
  // Consider all the source part's outgoing edges (implemented by a server analysis)
  AnalysisPartEdgeLists outE = parts.outEdgesIndexInput(*dpea->getComposer());///*NodeState_valid*/ false, /*indexes_valid*/ true, /*inputs_valid*/ true);
  // If this abstract state has no outgoing edges, return without modifying dfInfo. This is because
  // the composer may wish to do something with the resulting Lattice (e.g. copy it on a wildcard outgoing edge)
  // even if its information content has not changed.
  // if(baseEdges.size()==0) { modified=true; return; }
  if(outE.size() == 0) { modified=true; return; }

  // Empty out dfInfo in preparation for it being overwritten
  dfInfo.clear();

  for(AnalysisPartEdgeLists::iterator edge=outE.begin(); edge!=outE.end(); ++edge) {
    SIGHT_VERB_IF(1, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
      dbg << "e="<<edge->input()->str()<<endl;
#endif
    SIGHT_VERB_FI()

    // Create a DeadPathElimPartEdge to this server analysis-implemented edge
    DeadPathElimPartEdge* dpeEdge;
    // If this is the first edge to synthesize, make the dfEdge into true branch DeadPathElimPartEdge
    if(dfInfo.size()==0) dpeEdge = dfEdge;
    else                 dpeEdge = dynamic_cast<DeadPathElimPartEdge*>(dfEdge->copy());
    assert(dpeEdge);

    // // Set this dpeEdge's target to be the same as the current server edge's target but with the dfEdge's level
    // dpeEdge->tgt = DeadPathElimPart::create((*e)->target(), dpea);
    // Set this dpeEdge's target to be the same as the target of the current server edge but using the edge's level
    dpeEdge->tgt = DeadPathElimPart::create(edge->input()->target(), dpea);

    // Set this dpeEdges's baseEdge to be the current edge using both Lattice API (setPartEdge) and setSupersetPartEdge API (setParent)
    //dpeEdge->setPartEdge(*e);
    // dpeEdge->setInputPartEdge(*e);
    // Set this dpeEdges's baseEdge to be the current edge using both Lattice API (setPartEdge) and Part API (setParent)
    //%%% keep setParent since *e corresponds to the ATS to be refined
    //%%% remove setPartEdge() since that edge should be the one from the ATS on top of which we're running. 
    // That should be set by the ComposedAnalysis.
    // dpeEdge->setPartEdge(*e);
    dpeEdge->setInputPartEdge(edge->input());

    SIGHT_VERB(dbg << "dpeEdge="<<dpeEdge->str()<<endl, 1, DPEAnalDebugLevel)

    // Add the current DeadPathElimPartEdge to dfInfo
    vector<Lattice *> dfLatVec;
    dfLatVec.push_back(dpeEdge);
    dfInfo[edge->index()] = dfLatVec;
    // dfInfo[*e] = dfLatVec;
  }
  SIGHT_VERB(dbg << "dfInfo=\n" << NodeState::str(dfInfo) << endl, 1, DPEAnalDebugLevel)
}

DeadPathElimAnalysis::DeadPathElimAnalysis(bool trackBase2RefinedPartEdgeMapping):
    FWDataflow(trackBase2RefinedPartEdgeMapping, /*useSSA*/ false)
{
  cacheInitialized_GetStartAStates_Spec=false;
  cacheInitialized_GetEndAStates_Spec=false;
}

// Initializes the state of analysis lattices at the given function, part and edge into our out of the part
// by setting initLattices to refer to freshly-allocated Lattice objects.
void DeadPathElimAnalysis::genInitLattice(const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                                          std::vector<Lattice*>& initLattices)
{
  DeadPathElimPartEdge* newPartEdge = DeadPathElimPartEdge::createRaw(parts.NodeState()->inEdgeFromAny(), parts.input()->inEdgeFromAny(), this, bottom);

  SIGHT_VERB_DECL(scope, (txt() << "DeadPathElimPart::genInitLattice()"), 2, DPEAnalDebugLevel)
  SIGHT_VERB(dbg << "parts.NodeState()="<<parts.NodeState()->str()<<endl, 2, DPEAnalDebugLevel)
  SIGHT_VERB(dbg << "pedges.NodeState()="<<pedges.NodeState()->str()<<endl, 2, DPEAnalDebugLevel)

  // If this an the entry node of this function, set newPart to live
  set<PartPtr> startParts = getComposer()->GetStartAStates(this);
  if(startParts.find(parts.NodeState()) != startParts.end()) {
    newPartEdge->setToFull();
  }

  SIGHT_VERB(dbg << "newPartEdge="<<newPartEdge->str()<<endl, 2, DPEAnalDebugLevel)
  initLattices.push_back(newPartEdge);
}

bool DeadPathElimAnalysis::transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                                    std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)
{
  assert(0);
  return false;
}

// Return the anchor Parts of the application
set<PartPtr> DeadPathElimAnalysis::GetStartAStates_Spec()
{
  if(!cacheInitialized_GetStartAStates_Spec) {
    set<PartPtr> baseStartParts = getComposer()->GetStartAStates(this);
    for(set<PartPtr>::iterator baseSPart=baseStartParts.begin(); baseSPart!=baseStartParts.end(); baseSPart++) {
      //NodeState* startState = NodeState::getNodeState(this, *baseSPart);
      SIGHT_VERB_IF(2, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
        dbg << "startPart = "<<baseSPart->get()->str()<<endl;
      //  dbg << "startState = "<<startState->str(this)<<endl;
#endif
      SIGHT_VERB_FI()

      //#SA: dfInfo is aggregated on inEdgeFromAny
//      DeadPathElimPartEdge* startDPEPartEdge = dynamic_cast<DeadPathElimPartEdge*>(startState->getLatticeAbove(this, (baseSPart->get())->inEdgeFromAny(), 0));
//      assert(startDPEPartEdge);
//      cache_GetStartAStates_Spec.insert(startDPEPartEdge->target());
      cache_GetStartAStates_Spec.insert(DeadPathElimPart::create(*baseSPart, this));
    }
    cacheInitialized_GetStartAStates_Spec = true;
  }
  return cache_GetStartAStates_Spec;
}

set<PartPtr> DeadPathElimAnalysis::GetEndAStates_Spec()
{
  if(!cacheInitialized_GetEndAStates_Spec) {
    set<PartPtr> endParts = getComposer()->GetEndAStates(this);
    for(set<PartPtr>::iterator baseEPart=endParts.begin(); baseEPart!=endParts.end(); baseEPart++) {
      //NodeState* endState = NodeState::getNodeState(this, *baseEPart);
      SIGHT_VERB_IF(2, DPEAnalDebugLevel)
#ifndef DISABLE_SIGHT
        dbg << "endPart = "<<baseEPart->get()->str()<<endl;
        //dbg << "endState = "<<endState->str(this)<<endl;
#endif
      SIGHT_VERB_FI()
      //#SA: dfInfo is aggregated on inEdgeFromAny
/*      DeadPathElimPartEdge* endDPEPartEdge = dynamic_cast<DeadPathElimPartEdge*>(endState->getLatticeAbove(this, (baseEPart->get())->inEdgeFromAny(), 0));
      assert(endDPEPartEdge);

      cache_GetEndAStates_Spec.insert(endDPEPartEdge->target());*/
        cache_GetEndAStates_Spec.insert(DeadPathElimPart::create(*baseEPart, this));
    }
    cacheInitialized_GetEndAStates_Spec = true;
  }
  return cache_GetEndAStates_Spec;
}

}; // namespace fuse
