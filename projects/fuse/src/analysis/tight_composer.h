/*****************************************
 * author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#ifndef _TIGHT_COMPOSER_H
#define _TIGHT_COMPOSER_H

#include "compose.h"

namespace fuse {
  
  class TightComposer : public Composer, public ComposedAnalysis
  {
    std::list<ComposedAnalysis*> allAnalyses;
    direction dir;

  public:
    TightComposer(const std::list<ComposedAnalysis*>& analyses);
    TightComposer(const TightComposer&);
  
    // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
    ComposedAnalysisPtr copy() {
      return boost::make_shared<TightComposer>(*this); 
    }

    // -------------------------
    // - Methods from Composer -
    // -------------------------

  public:
    CodeLocObjectPtr Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
  private:
    CodeLocObjectPtr Expr2CodeLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
  public:
    // Variant of Expr2CodeLoc that inquires about the code location denoted by the operand of the 
    // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
    CodeLocObjectPtr OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);
    
    // Abstract interpretation functions that return this analysis' abstractions that 
    // represent the outcome of the given SgExpression. 
    // The objects returned by these functions are expected to be deallocated by their callers.
    ValueObjectPtr Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
  private:
    ValueObjectPtr Expr2Val_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
  public:
    // Variant of Expr2Value that runs the query on the analysis that called the method rather than 
    // some prior server analysis
    //  ValueObjectPtr Expr2ValSelf(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* self);
  
    // Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the 
    // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
    ValueObjectPtr OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);
    
    MemRegionObjectPtr Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
  private:
    MemRegionObjectPtr Expr2MemRegion_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
  public:
    // Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
    // the part denotes the set of prefixes that terminate at SgNode n.
    MemRegionObjectPtr OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);
    
    MemLocObjectPtr Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
  private:
    MemLocObjectPtr Expr2MemLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
  public:
    // Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
    // the part denotes the set of prefixes that terminate at SgNode n.
    MemLocObjectPtr OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);
  
    // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
    bool mayEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client);
    //bool mayEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
    bool mayEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client);
    //bool mayEqualML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client);
  
    // Returns whether the given pai   of AbstractObjects are must-equal at the given PartEdge
    bool mustEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client);
    //bool mustEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
    bool mustEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client);
    //bool mustEqualML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client);
  
    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSetV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client);
    //bool equalSetCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
    bool equalSetMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client);
    //bool equalSetML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client);
  
    // Returns whether abstract object ao1 denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the abstract object ao2.
    bool subSetV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client);
    //bool subSetCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
    bool subSetMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client);
    //bool subSetML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client);
    
    // Returns whether the given AbstractObject is live at the given PartEdge
    bool isLiveV (ValueObjectPtr val,    PartEdgePtr pedge, ComposedAnalysis* client);
    //bool isLiveCL(CodeLocObjectPtr cl,   PartEdgePtr pedge, ComposedAnalysis* client);
    bool isLiveMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client);
    //bool isLiveML(MemLocObjectPtr ml,    PartEdgePtr pedge, ComposedAnalysis* client);
  
    // Calls the isLive() method of the given AbstractObject that denotes an operand of the given SgNode n within
    // the context of its own PartEdges and returns true if it may be live within any of them
    bool OperandIsLiveV (SgNode* n, SgNode* operand, ValueObjectPtr val,    PartEdgePtr pedge, ComposedAnalysis* client);
    //bool OperandIsLiveCL(SgNode* n, SgNode* operand, CodeLocObjectPtr cl,   PartEdgePtr pedge, ComposedAnalysis* client);
    bool OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client);
    //bool OperandIsLiveML(SgNode* n, SgNode* operand, MemLocObjectPtr ml,    PartEdgePtr pedge, ComposedAnalysis* client);
  
    // Computes the meet of from and to and saves the result in to.
    // Returns true if this causes this to change and false otherwise.
    bool meetUpdateV (ValueObjectPtr     to, ValueObjectPtr     from, PartEdgePtr pedge, ComposedAnalysis* analysis);
    //bool meetUpdateCL(CodeLocObjectPtr   to, CodeLocObjectPtr   from, PartEdgePtr pedge, ComposedAnalysis* analysis);
    bool meetUpdateMR(MemRegionObjectPtr to, MemRegionObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis);
    //bool meetUpdateML(MemLocObjectPtr    to, MemLocObjectPtr    from, PartEdgePtr pedge, ComposedAnalysis* analysis);
  
    // Returns whether the given AbstractObject corresponds to the set of all sub-executions or the empty set
    bool isFullV (ValueObjectPtr     ao, PartEdgePtr pedge, ComposedAnalysis* analysis);
    //bool isFullCL(CodeLocObjectPtr   ao, PartEdgePtr pedge, ComposedAnalysis* analysis);
    bool isFullMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis);
    //bool isFullML(MemLocObjectPtr    ao, PartEdgePtr pedge, ComposedAnalysis* analysis);
  
    // Returns whether the given AbstractObject corresponds to the empty set
    bool isEmptyV (ValueObjectPtr     ao, PartEdgePtr pedge, ComposedAnalysis* analysis);
    //bool isEmptyCL(CodeLocObjectPtr   ao, PartEdgePtr pedge, ComposedAnalysis* analysis);
    bool isEmptyMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis);
    //bool isEmptyML(MemLocObjectPtr    ao, PartEdgePtr pedge, ComposedAnalysis* analysis);
  
    // Return the anchor Parts of an application
    std::set<PartPtr> GetStartAStates(ComposedAnalysis* client);
    std::set<PartPtr> GetEndAStates(ComposedAnalysis* client);

    // -----------------------------------------
    // ----- Methods from ComposedAnalysis -----
    // -----------------------------------------
    //! initialize the NodeState for the given part for each analysis in the list
    void initNodeState(PartPtr part);
    bool transfer(PartPtr part, CFGNode cn, NodeState& state, 
                  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);
    void transferPropagateAState(PartPtr part, std::set<PartPtr>& visited, bool firstVisit, 
                                 std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt, anchor curPartAnchor, 
                                 graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                                 std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors);
  std::set<PartPtr> getInitialWorklist();
    std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state);
    std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state);
    void setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite);
    void setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite);

    std::list<PartPtr> getDescendants(PartPtr p);
    std::list<PartEdgePtr> getEdgesToDescendants(PartPtr part);
    std::set<PartPtr> getUltimate();
    dataflowPartEdgeIterator* getIterator();
  
    // Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
    // with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
    void remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat);
  
    direction getDirection();

    // The Expr2* and GetFunction*Part functions are implemented by calling the same functions in each of the
    // constituent analyses and returning an Intersection object that includes their responses

    // Abstract interpretation functions that return this analysis' abstractions that
    // represent the outcome of the given SgExpression. The default implementations of
    // these throw NotImplementedException so that if a derived class does not implement
    // any of these functions, the Composer is informed.
    //
    // The objects returned by these functions are expected to be deallocated by their callers.
    ValueObjectPtr     Expr2Val      (SgNode* n, PartEdgePtr pedge);
    CodeLocObjectPtr   Expr2CodeLoc  (SgNode* n, PartEdgePtr pedge);
    MemRegionObjectPtr Expr2MemRegion(SgNode* n, PartEdgePtr pedge);
    MemLocObjectPtr    Expr2MemLoc   (SgNode* n, PartEdgePtr pedge);
  
    // Return true if the class implements Expr2* and false otherwise
    bool implementsExpr2Val      ();
    bool implementsExpr2CodeLoc  ();
    bool implementsExpr2MemRegion();
    bool implementsExpr2MemLoc   ();
    bool implementsATSGraph      ();
  
    // Returns whether the class implements Expr* loosely or tightly (if it does at all)
    ComposedAnalysis::implTightness Expr2ValTightness();
    ComposedAnalysis::implTightness Expr2CodeLocTightness();
    ComposedAnalysis::implTightness Expr2MemRegionTightness();
    ComposedAnalysis::implTightness Expr2MemLocTightness();

    // Return the anchor Parts of a given function
    std::set<PartPtr> GetStartAStates_Spec();
    std::set<PartPtr> GetEndAStates_Spec();

    std::string str(std::string indent="") const;

  }; // end TightComposer
}; // end namespace fuse

#endif
