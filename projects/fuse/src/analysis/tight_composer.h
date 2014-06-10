/*****************************************
 * author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#ifndef _TIGHT_COMPOSER_H
#define _TIGHT_COMPOSER_H

#include "compose.h"

namespace fuse {

  /***************
   * Expr2AnyKey *
   ***************/

  //! Key value used to identify any query.
  class Expr2AnyKey : public sight::printable {
    SgNode* sgn;
    PartEdgePtr pedge;
    Composer::reqType reqtype;
  public:
    Expr2AnyKey(SgNode* _sgn, PartEdgePtr _pedge, Composer::reqType _reqtype)
      :  sgn(_sgn), pedge(_pedge), reqtype(_reqtype) { }

    bool operator<(const Expr2AnyKey& that) const;

    std::string str(std::string indent="") const;
  };

  /*****************
   * Expr2AnyState *
   *****************/

  //! State of any Expr2Any query.
  //! Maintaining state for query is useful cycles and return Full abstract objects.
  //! The init state of the query denotes that query has not been forwarded to any analysis.
  //! Query remains in analysis state when it is being forwarded to analysis by the TightComposer.
  //! Variable analysis tracks the last analysis that the composer forwarded the query.
  //! Receiving the same query from already queried analysis indicates cycle 
  //! at which point TightComposer returns Full (univeral) abstract objects.
  //! Query is retired to finished state when there are no more analyses for forwarding.
  class Expr2AnyState {
  public:
    typedef enum {init=0, analysis=1, finished=2} StateT;
    StateT state;
    ComposedAnalysis* currAnalysis;
    Expr2AnyState() : state(init), currAnalysis(NULL) { }
  };

  /*************************
   * TightCompositionCache *
   *************************/
 
  class TightCompositionCache {
    typedef std::pair<Expr2AnyState, AbstractObjectPtr> QueryInfo;
    std::map<Expr2AnyKey, QueryInfo> tccache;
  public:
    TightCompositionCache() { }
    void initializeQuery(Expr2AnyKey key);
    bool isQueryCached(Expr2AnyKey key);
    AbstractObjectPtr getCachedAO(Expr2AnyKey key);
    bool isRecurringQuery(Expr2AnyKey key, ComposedAnalysis* analysis);
    void updateQueryState(Expr2AnyKey key, ComposedAnalysis* analysis);
    void retireQueryUpdateCache(Expr2AnyKey key, AbstractObjectPtr ao);
  };

  /*****************
   * TightComposer *
   *****************/

  class TightComposer : public Composer, public ComposedAnalysis
  {
    std::list<ComposedAnalysis*> allAnalyses;
    direction dir;

    TightCompositionCache tccache;
    
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

    //! Generic method for answering Expr2* queries.
    //! Consider two analysis A, B composed by TightComposer.
    //! Analysis A or B when implementing transfer functions invoke composer->Expr2* query on the incoming edge.
    //! The query from the analysis is forwarded to the TightComposer which is responsible for 
    //! computing the tightest possible abstraction for the given query.
    //! For any given Expr2* query, TightComposer forwards the query to all the analyses in the composition list and its predecessor.
    //! To compute tightest abstraction for the query it computes the intersection of the abstract objects returned by the analyses.
    //! It checks if each analysis can answer the query and calls the corresponding query method implemented by the analysis.
    //! The intersection is facilitated by creating abstract objects 
    //! such as IntersectMemLocObject, IntersectMemRegionObject, IntersectValueObject or IntersectCodeLocObject.
    //! Forwarding query to analyses creates cycles.
    //! \dot
    //! digraph query {
    //!  rankdir=TB
    //!  node [shape=record];
    //!  {
    //!   rank=same; A; B;
    //!   A [label="{Analysis A| {<f1> transfer|<f2> Expr2Any}}"]
    //!   B [label="{Analysis B| {<f1> transfer|<f2> Expr2Any}}"]
    //!  }
    //!  TC[label="{<f1> Expr2Any| TightComposer}"]
    //!  A:f1 -> TC:f1 [label="composer->expr2any" color="darkgreen"];
    //!  TC:f1 -> A:f2 [label="analysis->expr2any" color="blue"];
    //!  TC:f1 -> B:f2 [label="analysis->expr2any" color="blue"];
    //!  B:f2 -> TC:f1 [label="composer->expr2any" color="red"];
    //! }
    //! \enddot
    //! Analysis A queries the composer using composers' Expr2Any method.
    //! Composer forwards the queries to each analysis invoking analysis' Expr2Any method.
    //! Analysis answers those queries based on what is already known to it.
    //! However an analysis can forward the query back to the composer as it may not know anything about the expression.
    //! For example, points to analysis understands pointers but does not anything about non-pointer variables or arrays.
    //! As it does not know anything about the expression an analysis consults the composer thereby creating cycles.
    //! TightComposer should detect such cycles and return universal objects (that are true) to break the cycles.
    //! Analysis::Expr2Any method can be invoked only by the composer.
    //! Before invoking the Analysis::Expr2Any method TightComposer records the fact that it is consulting an analysis for the query at a given edge.
    //! If the analysis forwards the query back to the composer, 
    //! it would know that it already consulted the analysis for the same expression at a given edge.
    //! The query that was forwarded back to the composer is answered by returning full abstract objects.
    //! Note that the composer does not consult other analyses for the recurring query.
    //! Full abstract objects are returned only for the cases of recurring queries.
    //! The full abstract objects are returned to the analysis which may wrap its 
    //! own abstract object around it and return the wrapped abstract object back to the composer.
    //! On constructing intersect abstract objects, full abstract objects are dropped from it as \f$ True \cap dfinfo = dfinfo \f$.

    //! \tparam AOType AbstractObject type that can be MemLocObject, ValueObject, MemRegionObject or CodeLocObject.
    //! \tparam FullAOType Full or universal version of AOType which can be FullMemLocObject, FullValueObject, FullMemRegionObject, FullCodeLocObject.
    //! \tparam IntersectAOType The result of TightComposer is represented as an intersection of AbstractObject.
    //! \result Returns a boost::shared_ptr of AOType 
    //!
    template<class AOType, class FullAOType, class IntersectAOType>
    boost::shared_ptr<AOType> Expr2Any(std::string opName,
                                       SgNode* n,
                                       PartEdgePtr pedge,
                                       ComposedAnalysis* client,
                                       Composer::reqType reqtype,
                                       boost::function<bool (ComposedAnalysis*)> implementsExpr2AnyOp,
                                       boost::function<boost::shared_ptr<AOType> (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp,
                                       boost::function<boost::shared_ptr<AOType> (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp);

    template<class AOType, class FullAOType, class IntersectAOType>
    boost::shared_ptr<AOType> OperandExpr2Any(std::string opName,
                                          SgNode* n,
                                          SgNode* operand,
                                          PartEdgePtr pedge,
                                          ComposedAnalysis* client,
                                          Composer::reqType reqtype,
                                          boost::function<bool (ComposedAnalysis*)> implementsExpr2AnyOp,
                                          boost::function<boost::shared_ptr<AOType> (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp);

  public:
    CodeLocObjectPtr Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
    // private:
    //   CodeLocObjectPtr Expr2CodeLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
    // public:
    // Variant of Expr2CodeLoc that inquires about the code location denoted by the operand of the 
    // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
    CodeLocObjectPtr OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);
    
    // Abstract interpretation functions that return this analysis' abstractions that 
    // represent the outcome of the given SgExpression. 
    // The objects returned by these functions are expected to be deallocated by their callers.
    ValueObjectPtr Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
    // private:
    //   ValueObjectPtr Expr2Val_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
    // public:
    // Variant of Expr2Value that runs the query on the analysis that called the method rather than 
    // some prior server analysis
    //  ValueObjectPtr Expr2ValSelf(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* self);
  
    // Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the 
    // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
    ValueObjectPtr OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);
    
    MemRegionObjectPtr Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
    // private:
    //   MemRegionObjectPtr Expr2MemRegion_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
    // public:
    // Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
    // the part denotes the set of prefixes that terminate at SgNode n.
    MemRegionObjectPtr OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);
    
    MemLocObjectPtr Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
    private:
    MemLocObjectPtr Expr2MemLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);
  
    // public:
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
