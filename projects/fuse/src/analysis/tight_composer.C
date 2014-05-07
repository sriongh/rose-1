/*****************************************
 * author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "sage3basic.h"
#include "tight_composer.h"
#include <boost/enable_shared_from_this.hpp>
#include "sight.h"
#include "stx_analysis.h"
#include <algorithm>

using namespace std;
using namespace boost;
using namespace sight;

namespace fuse {

  DEBUG_LEVEL(tightComposerDebugLevel, 2);

  /***************
   * Expr2AnyKey *
   ***************/

  bool Expr2AnyKey::operator<(const Expr2AnyKey& that) const {
    return (reqtype < that.reqtype &&
            pedge < that.pedge &&
            sgn < that.sgn);
  }

  std::string Expr2AnyKey::str(std::string indent) const {
    std::ostringstream oss;
    oss << "[sgn=" << SgNode2Str(sgn) <<",";
    oss << "pedge=" << pedge->str() <<",";        
    oss << " reqtype="; 
    switch(reqtype) {
    case 0: oss << "any"; break;
    case 1: oss << "codeloc"; break;
    case 2: oss << "val"; break;
    case 3: oss << "memloc"; break;
    case 4: oss << "memregion"; break;
    case 5: oss << "atsGraph"; break;
    default: assert(0);
    }
    oss << "]";
    return oss.str();
  }

  /*************************
   * TightCompositionCache *
   *************************/

  void TightCompositionCache::initializeQuery(Expr2AnyKey key) {
    // there is no prior entry in the map
    assert(tccache.find(key) == tccache.end());
    QueryInfo elemInfo = make_pair(Expr2AnyState(), AbstractObjectPtr());
    pair<Expr2AnyKey, QueryInfo> elem = make_pair(key, elemInfo);
    tccache.insert(elem);
  }

  bool TightCompositionCache::isQueryCached(Expr2AnyKey key) {
    if(tccache.find(key) == tccache.end())
      return false;
    QueryInfo& qinfo = tccache.find(key)->second;
    Expr2AnyState& qstate = qinfo.first;
    if(qstate.state == Expr2AnyState::finished)
      return true;
    
    return false;
  }

  AbstractObjectPtr TightCompositionCache::getCachedAO(Expr2AnyKey key) {
    assert(tccache.find(key) != tccache.end());
    QueryInfo& qinfo = tccache.find(key)->second;
    Expr2AnyState& qstate = qinfo.first;
    assert(qstate.state == Expr2AnyState::finished);
    assert(qinfo.second);
    return qinfo.second;
  }

  bool TightCompositionCache::isRecurringQuery(Expr2AnyKey key, ComposedAnalysis* thatanalysis) {
    if(tccache.find(key) == tccache.end())
      return false;

    QueryInfo& qinfo = tccache.find(key)->second;
    Expr2AnyState& qstate = qinfo.first;

    if(qstate.state != Expr2AnyState::analysis)
      return false;

    assert(qstate.currAnalysis == thatanalysis);
    return true;
  }

  void TightCompositionCache::updateQueryState(Expr2AnyKey key, ComposedAnalysis* thatanalysis) {
    // key is already in the map
    assert(tccache.find(key) != tccache.end());
    QueryInfo& qinfo = tccache.find(key)->second;
    Expr2AnyState& qstate = qinfo.first;
    assert(qstate.state == Expr2AnyState::init || qstate.state == Expr2AnyState::analysis);
    qstate.state = Expr2AnyState::analysis;
    qstate.currAnalysis = thatanalysis;    
  }

  void TightCompositionCache::retireQueryUpdateCache(Expr2AnyKey key, AbstractObjectPtr ao) {
    // key is already in the map
    assert(tccache.find(key) != tccache.end());
    QueryInfo& qinfo = tccache.find(key)->second;
    Expr2AnyState& qstate = qinfo.first;
    assert(qstate.state == Expr2AnyState::analysis);
    qstate.state = Expr2AnyState::finished;
    assert(ao);
    qinfo.second = ao;
  }

  /******************************
   * TightComposer Methods *
   ******************************/

  TightComposer::TightComposer(const std::list<ComposedAnalysis*>& analyses) : allAnalyses(analyses) {    
    list<ComposedAnalysis*>::iterator a=allAnalyses.begin();
    // get the first analysis' direction
    // all other analyses should be in the same direction
    dir = (*a)->getDirection();
    for( ; a!=allAnalyses.end(); ++a) {        
      // Inform each analysis of the composer's identity
      (*a)->setComposer(this);
      assert(dir == (*a)->getDirection());
    }    
  }
      
  TightComposer::TightComposer(const TightComposer& that) : allAnalyses(that.allAnalyses), dir(that.dir) {
  }

  template<class AOType, class FullAOType, class IntersectAOType>
  boost::shared_ptr<AOType> TightComposer::Expr2Any(string opName,
                                                    SgNode* n,
                                                    PartEdgePtr pedge,
                                                    ComposedAnalysis* client,
                                                    Composer::reqType reqtype,
                                                    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp,
                                                    function<boost::shared_ptr<AOType> (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp,
                                                    function<boost::shared_ptr<AOType> (SgNode*, PartEdgePtr)> parentComposerExpr2AnyOp) {
    scope reg(txt()<<"TightComposer::Expr2Any",
              scope::medium, attrGE("tightComposerDebugLevel", 2));

    Expr2AnyKey key(n, pedge, reqtype);
    if(tightComposerDebugLevel() >=2) dbg << "[key=" << key.str() << "]\n";

    if(tccache.isQueryCached(key)) {
      AbstractObjectPtr ao = tccache.getCachedAO(key);
      assert(boost::dynamic_pointer_cast<IntersectAOType>(ao));
      return boost::dynamic_pointer_cast<IntersectAOType>(ao);
    }

    if(tccache.isRecurringQuery(key, client))
      return boost::make_shared<FullAOType>(n);

    tccache.initializeQuery(key);

    // list of abstract objects to return
    list<boost::shared_ptr<AOType> > aolist;
    list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    // query all the client for Expr2Any and add them the list
    for( ; a != allAnalyses.end(); ++a) {
      if(implementsExpr2AnyOp(*a)) {
        tccache.updateQueryState(key, *a);
        boost::shared_ptr<AOType> ao_p = Expr2AnyOp(*a, n, pedge);
        assert(ao_p);
        aolist.push_back(ao_p);
      }
    }

    aolist.push_front(parentComposerExpr2AnyOp(n, pedge));

    boost::shared_ptr<IntersectAOType> iao_p = boost::make_shared<IntersectAOType>(aolist);
    tccache.retireQueryUpdateCache(key, iao_p);

    if(tightComposerDebugLevel() >= 2)
      dbg << iao_p->str() << endl;
      
    return iao_p;
  }
  
  // Same as Expr2Any
  // Query for Expr2Any(operand,..) on all the PartEdge returned by getOperandPartEdge(n,operand) of the given pedge
  // Return the list of objects
  template<class AOType, class FullAOType, class IntersectAOType>
  boost::shared_ptr<AOType> TightComposer::OperandExpr2Any(string opName,
                                                           SgNode* n,
                                                           SgNode* operand,
                                                           PartEdgePtr pedge,
                                                           ComposedAnalysis* client,
                                                           Composer::reqType reqtype,
                                                           function<bool (ComposedAnalysis*)> implementsExpr2AnyOp,
                                                           function<boost::shared_ptr<AOType> (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp) {
    scope reg(txt()<<"TightComposer::Operand"<<opName, 
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    if(tightComposerDebugLevel()>=2) 
      dbg << "n="<<SgNode2Str(n)<<endl << "operand("<<operand<<")="<<SgNode2Str(operand)<<endl << "pedge="<<pedge->str()<<endl;
  
    // Get the parts of the execution prefixes that terminate at the operand before continuing directly 
    // to SgNode n in the given part
    list<PartEdgePtr> opPartEdges = pedge->getOperandPartEdge(n, operand);
    if(tightComposerDebugLevel()>=2) {
      dbg << "opPartEdges(#"<<opPartEdges.size()<<")="<<endl;
      for(list<PartEdgePtr>::iterator opE = opPartEdges.begin(); opE != opPartEdges.end(); ++opE) {
        indent ind;
        dbg << (*opE)->str()<<endl;
      }
    }

    // create empty intersect object
    // populated in the loop below
    list<boost::shared_ptr<AOType> > aolist;
    boost::shared_ptr<IntersectAOType> iao_p = boost::make_shared<IntersectAOType>(aolist);

    for(list<PartEdgePtr>::iterator opE=opPartEdges.begin(); opE != opPartEdges.end(); ++opE) {
      // list<RetTypePtr> aoListForPedge = Expr2Any<RetTypePtr> (opName, operand, *opE, client, reqtype, implementsExpr2AnyOp, Expr2AnyOp);
      // aoList.insert(aoList.end(), aoListForPedge.begin(), aoListForPedge.end());
    }

    return iao_p;
  }


  CodeLocObjectPtr TightComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLCodeLocObject;
  }
  

  // CodeLocObjectPtr TightComposer::Expr2CodeLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
  //   assert(0);
  //   return NULLCodeLocObject;
  // }

  
  // Variant of Expr2CodeLoc that inquires about the code location denoted by the operand of the 
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  CodeLocObjectPtr TightComposer::OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLCodeLocObject;
  }
    
  // Abstract interpretation functions that return this analysis' abstractions that 
  // represent the outcome of the given SgExpression. 
  // The objects returned by these functions are expected to be deallocated by their callers.
  ValueObjectPtr TightComposer::Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt() << "TightComposer::Expr2Val",
          scope::medium, attrGE("tightComposerDebugLevel", 2));

    // Call the generic Expr2Any method to get the list of ValueObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2Val, _1));
    function<ValueObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2Val, _1, _2, _3));
    list<ValueObjectPtr> voplist;
    IntersectValueObjectPtr ivo_p = boost::make_shared<IntersectValueObject>(voplist);
    return ivo_p;
  }
  
  // ValueObjectPtr TightComposer::Expr2Val_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
  //   assert(0);
  //   return NULLValueObject;
  // }
  
  // Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the 
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  ValueObjectPtr TightComposer::OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call the generic OperandExpr2Any method to get the list of ValueObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2Val, _1));
    function<ValueObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2Val, _1, _2, _3));
    list<ValueObjectPtr> voplist;
    IntersectValueObjectPtr ivo_p = boost::make_shared<IntersectValueObject>(voplist);
    return ivo_p;  
  }
    
  MemRegionObjectPtr TightComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt() << "TightComposer::Expr2MemRegion",
          scope::medium, attrGE("tightComposerDebugLevel", 2));

    // Call the generic Expr2Any method to get the list of MemRegionObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2MemRegion, _1));
    function<MemRegionObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2MemRegion, _1, _2, _3));
    assert(getComposer() != this);
    function<MemRegionObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(bind(&Composer::Expr2MemRegion, getComposer(), _1, _2, this));

    MemRegionObjectPtr iml_p = Expr2Any<MemRegionObject, FullMemRegionObject, IntersectMemRegionObject>("Expr2MemRegion", 
                                                                                                        n, 
                                                                                                        pedge, 
                                                                                                        client, Composer::memregion, 
                                                                                                        implementsExpr2AnyOp, Expr2AnyOp,
                                                                                                        ComposerExpr2AnyOp);
    return iml_p;
  }
  

  // MemRegionObjectPtr TightComposer::Expr2MemRegion_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
  //   assert(0);
  //   return NULLMemRegionObject;
  // }
  
  // Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemRegionObjectPtr TightComposer::OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call the generic OperandExpr2Any method to get the list of MemRegionObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2MemRegion, _1));
    function<MemRegionObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2MemRegion, _1, _2, _3));
    list<MemRegionObjectPtr> mrplist;
    IntersectMemRegionObjectPtr imr_p = boost::make_shared<IntersectMemRegionObject>(mrplist);
    return imr_p;
  }

  MemLocObjectPtr TightComposer::Expr2MemLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt()<<"TightComposer::Expr2MemLoc_ex",
              scope::medium, attrGE("tightComposerDebugLevel", 2));

    // Call the generic Expr2Any method to get the list of MemLocObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2MemLoc, _1));
    function<MemLocObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2MemLoc, _1, _2, _3));

    assert(getComposer() != this);
    function<MemLocObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(bind(&Composer::Expr2MemLoc, getComposer(), _1, _2, this));

    MemLocObjectPtr iml_p = Expr2Any<MemLocObject, FullMemLocObject, IntersectMemLocObject>("Expr2MemLoc", 
                                                                                                     n, 
                                                                                                     pedge, 
                                                                                                     client, Composer::memloc, 
                                                                                                     implementsExpr2AnyOp, Expr2AnyOp,
                                                                                                     ComposerExpr2AnyOp);
    return iml_p;
  }
  
  //! Any client of the TightComposer invokes this interface function
  //! TightComposer queries all the client analyses implementing Expr2MemLoc
  //! Returns IntersectMemLocObjectPtr
  MemLocObjectPtr TightComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    return Expr2MemLoc_ex(n, pedge, client);
  }
  
  // Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemLocObjectPtr TightComposer::OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
  }
  
  // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
  bool TightComposer::mayEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  //bool mayEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
  bool TightComposer::mayEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->mayEqualMR(mr2, pedge);
  }
  
  // Returns whether the given pai   of AbstractObjects are must-equal at the given PartEdge
  bool TightComposer::mustEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }
  //bool mustEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
  bool TightComposer::mustEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->mustEqualMR(mr2, pedge);
  }
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool TightComposer::equalSetV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  //bool equalSetCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
  bool TightComposer::equalSetMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return false;
  }
  
  // Returns whether abstract object ao1 denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the abstract object ao2.
  bool TightComposer::subSetV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  bool TightComposer::subSetMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }
    
  // Returns whether the given AbstractObject is live at the given PartEdge
  bool TightComposer::isLiveV (ValueObjectPtr val,    PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  bool TightComposer::isLiveMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }
  
  // Calls the isLive() method of the given AbstractObject that denotes an operand of the given SgNode n within
  // the context of its own PartEdges and returns true if it may be live within any of them
  bool TightComposer::OperandIsLiveV (SgNode* n, SgNode* operand, ValueObjectPtr val,    PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  bool TightComposer::OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  
  // Computes the meet of from and to and saves the result in to.
  // Returns true if this causes this to change and false otherwise.
  bool TightComposer::meetUpdateV (ValueObjectPtr     to, ValueObjectPtr     from, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    assert(0);
    return false;
  }

  bool TightComposer::meetUpdateMR(MemRegionObjectPtr to, MemRegionObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    assert(0);
    return false;
  }
  
  // Returns whether the given AbstractObject corresponds to the set of all sub-executions or the empty set
  bool TightComposer::isFullV (ValueObjectPtr     ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    assert(0);
    return false;
  }

  bool TightComposer::isFullMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return false;
  }
  
  // Returns whether the given AbstractObject corresponds to the empty set
  bool TightComposer::isEmptyV (ValueObjectPtr     ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    assert(0);
    return false;
  }

  bool TightComposer::isEmptyMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return ao->isEmptyMR(pedge);
  }
  
  // query all analyses in the composition list with GetStartAStates
  // construct IntersectionPart and return the set of IntersectionParts
  // this would mean that the analyses are composed tightly and they are also
  // modifying the ATS simultaneously
  std::set<PartPtr> TightComposer::GetStartAStates(ComposedAnalysis* client) {    
    // 4/8/2014: for simplicity we are not allowing analyses to be composed tightly
    // and modify the ATS at the same time
    // TightComposer is composed as a sub-analysis of LooseSequential (ChainComposer)
    // direct the query GetStartAStates to the parent composer
    return GetStartAStates_Spec();
  }

  // implementation is similar in principle to GetStartAStates
  std::set<PartPtr> TightComposer::GetEndAStates(ComposedAnalysis* client) {
    return GetEndAStates_Spec();
  }

  // -----------------------------------------
  // ----- Methods from ComposedAnalysis -----
  // -----------------------------------------

  // go through the list of all analyses and call their initNodeState
  void TightComposer::initNodeState(PartPtr part) {
    list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
      (*a)->initNodeState(part);
    }
  }

  // call the generic version of this function on each analysis
  void TightComposer::transferPropagateAState(PartPtr part, set<PartPtr>& visited, bool firstVisit, set<PartPtr>& initialized,
                                              dataflowPartEdgeIterator* curNodeIt, anchor curPartAnchor, graph& worklistGraph,
                                              map<PartPtr, set<anchor> >& toAnchors,
                                              map<PartPtr, set<pair<anchor, PartPtr> > >& fromAnchors) {
    list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
      scope reg(txt() << "TightComposer::transferPropagateAState",
                scope::medium, attrGE("tightComposerDebugLevel", 2));
      dbg << "ComposedAnalysis=" << (*a)->str() << endl;
      ComposedAnalysis::transferPropagateAState(*a, part, visited, firstVisit, initialized, curNodeIt, curPartAnchor, 
                                                worklistGraph, toAnchors, fromAnchors);
    }
  }

  bool TightComposer::transfer(PartPtr part, CFGNode cn, NodeState& state, 
                               std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    assert(0);
    return false;
  }

  set<PartPtr> TightComposer::getInitialWorklist() {
    if(getDirection() == fw) return GetStartAStates(this);
    else if (getDirection() == bw) return GetEndAStates(this);
    else assert(0);
  }

  map<PartEdgePtr, vector<Lattice*> >& TightComposer::getLatticeAnte(NodeState *state) {
    assert(0);
    return state->getLatticeAboveAllMod(this);
  }

  map<PartEdgePtr, vector<Lattice*> >& TightComposer::getLatticePost(NodeState *state) {
    map<PartEdgePtr, vector<Lattice*> > belowLat;
    assert(0);
    return state->getLatticeBelowAllMod(this);
  }

  void TightComposer::setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite) {
    assert(0);
  }

  void TightComposer::setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite) {
    assert(0);
  }

  list<PartPtr> TightComposer::getDescendants(PartPtr p) {
  list<PartPtr> descendants;
  list<PartEdgePtr> outEdges = getEdgesToDescendants(p);

  for(list<PartEdgePtr>::iterator ei=outEdges.begin(); ei!=outEdges.end(); ei++)
    descendants.push_back((*ei)->target());
  return descendants;
}


  list<PartEdgePtr> TightComposer::getEdgesToDescendants(PartPtr part) {
    direction dir = getDirection();
    if(dir == fw)
      return part->outEdges();
    else if(dir == bw)
      return part->inEdges();
    else assert(0);
  }

  // call self GetStartAStates/GetEndAStates
  set<PartPtr> TightComposer::getUltimate() {
    direction dir = getDirection();
    if(dir == fw)
      return GetEndAStates(this);
    else if(dir == bw)
      return GetStartAStates(this);
    else assert(0);
  }

  // return the iterator based on our direction
  dataflowPartEdgeIterator* TightComposer::getIterator() {
    direction dir = getDirection();
    if(dir == fw)
      return new fw_dataflowPartEdgeIterator();
    else if(dir == bw)
      return new bw_dataflowPartEdgeIterator();
    else assert(0);
  }
  
  // Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
  // with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
  void TightComposer::remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat) {
    assert(0);
  }
  
  ComposedAnalysis::direction TightComposer::getDirection() {
    return dir;
  }

  // The Expr2* and GetFunction*Part functions are implemented by calling the same functions in each of the
  // constituent analyses and returning an Intersection object that includes their responses

  // Abstract interpretation functions that return this analysis' abstractions that
  // represent the outcome of the given SgExpression. The default implementations of
  // these throw NotImplementedException so that if a derived class does not implement
  // any of these functions, the Composer is informed.
  //
  // The objects returned by these functions are expected to be deallocated by their callers.
  ValueObjectPtr TightComposer::Expr2Val(SgNode* n, PartEdgePtr pedge) {
    return Expr2Val(n, pedge, this);
  }

  CodeLocObjectPtr TightComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge) {
    return Expr2CodeLoc(n, pedge, this);
  }

  MemRegionObjectPtr TightComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge) {
    return Expr2MemRegion(n, pedge, this);
  }

  MemLocObjectPtr TightComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge) {
    return Expr2MemLoc(n, pedge, this);
  }
  
  // Return true if the class implements Expr2* and false otherwise
  bool TightComposer::implementsExpr2Val() {
    return true;
  }

  bool TightComposer::implementsExpr2CodeLoc() {
    return true;
  }

  bool TightComposer::implementsExpr2MemRegion() {
    return true;
  }

  bool TightComposer::implementsExpr2MemLoc() {
    return true;
  }

  bool TightComposer::implementsATSGraph() {
    return false;
  }
  
  // Returns whether the class implements Expr* loosely or tightly (if it does at all)
  ComposedAnalysis::implTightness TightComposer::Expr2ValTightness() {
    return ComposedAnalysis::loose;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2CodeLocTightness() {
    return ComposedAnalysis::loose;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2MemRegionTightness() {
    return ComposedAnalysis::loose;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2MemLocTightness() {
    return ComposedAnalysis::loose;
  }

  // Return the anchor Parts of a given function
  // root the query to the parent composer
  // TightComposer is always run on some ATS implemented by the parent composer
  std::set<PartPtr> TightComposer::GetStartAStates_Spec() {
    return getComposer()->GetStartAStates(this);
  }

  std::set<PartPtr> TightComposer::GetEndAStates_Spec() {
    return getComposer()->GetEndAStates(this);
  }

  std::string TightComposer::str(std::string indent) const {
    return txt() << "TightComposer";
  }
};
