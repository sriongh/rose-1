/*****************************************
 * author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "sage3basic.h"
#include "tight_composer.h"
#include <boost/enable_shared_from_this.hpp>
#include "sight.h"
#include "stx_analysis.h"

using namespace std;
using namespace boost;
using namespace sight;

namespace fuse {

  DEBUG_LEVEL(tightComposerDebugLevel, 2);

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


  CodeLocObjectPtr TightComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLCodeLocObject;
  }
  

  CodeLocObjectPtr TightComposer::Expr2CodeLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLCodeLocObject;
  }

  
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
    assert(0);
    return NULLValueObject;
  }
  
  ValueObjectPtr TightComposer::Expr2Val_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLValueObject;
  }
  
  // Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the 
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  ValueObjectPtr TightComposer::OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLValueObject;
  }
    
  MemRegionObjectPtr TightComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLMemRegionObject;
  }
  

  MemRegionObjectPtr TightComposer::Expr2MemRegion_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLMemRegionObject;
  }
  
  // Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemRegionObjectPtr TightComposer::OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLMemRegionObject;
  }
    
  MemLocObjectPtr TightComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLMemLocObject;
  }
  

  MemLocObjectPtr TightComposer::Expr2MemLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLMemLocObject;
  }
  

  // Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemLocObjectPtr TightComposer::OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return NULLMemLocObject;
  }
  
  // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
  bool TightComposer::mayEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  //bool mayEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
  bool TightComposer::mayEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }
  
  // Returns whether the given pai   of AbstractObjects are must-equal at the given PartEdge
  bool TightComposer::mustEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }
  //bool mustEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
  bool TightComposer::mustEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
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
    assert(0);
    return false;
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
      ComposedAnalysis::transferPropagateAState(*a, part, visited, firstVisit, initialized, curNodeIt, curPartAnchor, worklistGraph, toAnchors, fromAnchors);
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
    return ComposedAnalysis::tight;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2CodeLocTightness() {
    return ComposedAnalysis::tight;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2MemRegionTightness() {
    return ComposedAnalysis::tight;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2MemLocTightness() {
    return ComposedAnalysis::tight;
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
