#ifndef DATAFLOW_H
#define DATAFLOW_H

#include "nodeState.h"
#include "analysis.h"
#include "lattice.h"
#include "abstract_object.h"
#include "ssa.h"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <set>
#include <map>
#include <string>
#include "sight.h"

namespace fuse {

class NotImplementedException
{};

class ComposedAnalysis;
typedef boost::shared_ptr<ComposedAnalysis> ComposedAnalysisPtr;


class ComposedAnalysis : public virtual Dataflow, public virtual NodeState2AllMapper
{
  public:
  // Indicates whether we should do the analysis using an SSA graph of the ATS this analysis runs on (true)
  // or whether we should run on the raw dense ATS with no support info (false).
  bool useSSA;
  // The object that describes the ATS, from which we'll compute the SSA
  SSAGraph* ssa;
  // The global lattices that describe the state of the entire application. These lattices same lattices
  // will be attached to the NodeState of every part.
  std::vector<Lattice*> ssaLats;
  bool ssaLatsInitialized;

  public:
  Composer* composer;

  // trackBase2RefinedPartEdgeMapping - records whether the mapping from base PartEdges to their
  //     corresponding refined parts should be tracked
  // useSSA - records whether the analysis should use the SSA graph for its execution
  ComposedAnalysis(bool trackBase2RefinedPartEdgeMapping, bool useSSA);

  // Informs this analysis about the identity of the Composer object that composes
  // this analysis with others
  void setComposer(Composer* composer)
  {
    this->composer = composer;
  }

  Composer* getComposer()
  {
    return composer;
  }

  public:

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  virtual ComposedAnalysisPtr copy()=0;

  // Initializes the analysis before running it
  virtual void initAnalysis();

  // Abstract interpretation functions that return this analysis' abstractions that
  // represent the outcome of the given SgExpression. The default implementations of
  // these throw NotImplementedException so that if a derived class does not implement
  // any of these functions, the Composer is informed.
  //
  // The objects returned by these functions are expected to be deallocated by their callers.
  virtual ValueObjectPtr      Expr2Val         (SgNode* n, PartEdgePtr pedge) { throw NotImplementedException(); }
  virtual CodeLocObjectPtr    Expr2CodeLoc     (SgNode* n, PartEdgePtr pedge) { throw NotImplementedException(); }
  virtual MemRegionObjectPtr  Expr2MemRegion(SgNode* n, PartEdgePtr pedge) { throw NotImplementedException(); }
  virtual MemLocObjectPtr     Expr2MemLoc      (SgNode* n, PartEdgePtr pedge) { throw NotImplementedException(); }

  // Methods used by client analyses to get a MemLoc from the composer while also documenting
  // whether the MemLoc corresponds to a definition or a use of the set of memory locations
  // denoted by n.
  MemLocObjectPtr Expr2MemLocDef(SgNode* n, PartEdgePtr pedge);
  MemLocObjectPtr Expr2MemLocUse(SgNode* n, PartEdgePtr pedge);
  MemLocObjectPtr OperandExpr2MemLocDef(SgNode* n, SgNode* operand, PartEdgePtr pedge);
  MemLocObjectPtr OperandExpr2MemLocUse(SgNode* n, SgNode* operand, PartEdgePtr pedge);

  // Return true if the class implements Expr2* and false otherwise
  virtual bool implementsExpr2Val      () { return false; }
  virtual bool implementsExpr2CodeLoc  () { return false; }
  virtual bool implementsExpr2MemRegion() { return false; }
  virtual bool implementsExpr2MemLoc   () { return false; }

  // Returns whether the class implements Expr* loosely or tightly (if it does at all)
  // A loose implementation computes the objects in question for use by its clients. However, Expr* queries
  //    made by this analysis for objects that it implements are forwarder to other analyses that implement them.
  // A tight implementation computes objects for both itself and its clients and thus, Expt* queries
  //    it makes for objects it implements should be forwarded to it, rather than to other analyses
  typedef enum {loose, tight} implTightness;
  virtual implTightness Expr2ValTightness()       { return loose; }
  virtual implTightness Expr2CodeLocTightness()   { return loose; }
  virtual implTightness Expr2MemRegionTightness() { return loose; }
  virtual implTightness Expr2MemLocTightness()    { return loose; }

  /*
  // <<<<<<<<<<
  // The following set of calls are just wrappers that call the corresponding
  // functions on their operand AbstractObjects. Implementations of Composed analyses may want to
  // provide their own implementations of these functions if they implement a partition graph
  // and need to convert the pedge from case they provide this support
  // inside analysis-specific functions rather than inside AbstractObject.

  // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
  // Wrapper for calling type-specific versions of mayEqual without forcing the caller to care about the type of object
  bool mayEqual(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge);

  // Special calls for each type of AbstractObject
  virtual bool mayEqualV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge);
  virtual bool mayEqualML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge);
  virtual bool mayEqualCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge);

  // Returns whether the given pair of AbstractObjects are must-equal at the given PartEdge
  // Wrapper for calling type-specific versions of mustEqual without forcing the caller to care about the type of object
  bool mustEqual(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge);

  // Special calls for each type of AbstractObject
  virtual bool mustEqualV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge);
  virtual bool mustEqualML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge);
  virtual bool mustEqualCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge);

  // Returns whether the given AbstractObject is live at the given PartEdge
  // Wrapper for calling type-specific versions of isLive without forcing the caller to care about the type of object
  bool isLive       (AbstractObjectPtr ao, PartEdgePtr pedge);
  // Special calls for each type of AbstractObject
  virtual bool isLiveVal    (ValueObjectPtr val,   PartEdgePtr pedge);
  virtual bool isLiveMemLoc (MemLocObjectPtr ml,   PartEdgePtr pedge);
  virtual bool isLiveCodeLoc(CodeLocObjectPtr cl,  PartEdgePtr pedge);
  */

  // Returns whether the given pair of AbstractObjects are equal at the given PartEdge

  protected:
  // Keep track of whether GetStartAStates and GetEndAStates have already been called and their
  // results cached.
  bool startStatesInitialized;
  bool endStatesInitialized;

  // Cached copies of the results of GetStartAState and GetEndAState
  std::set<PartPtr> StartAStates;
  std::set<PartPtr> EndAStates;

  // Maps refined Parts and PartEdges to the base Parts and PartEdges they refine. Set inside
  // convertPart() and convertPEdge() by caching the output of getSupersetPart()/getSupersetPartEdge()
  std::map<PartPtr,     PartPtr>     refined2BasePart;
  std::map<PartEdgePtr, PartEdgePtr> refined2BasePedge;

  // Maps base Parts and PartEdges from the ATS on which this analysis runs to the parts implemented
  // by this analysis that refine them. Set inside registerBase2RefinedMapping(), which is called
  // inside the Part and PartEdge constructors when the connection between a given refined
  // its base Parts and PartEdges is first established.
  std::map<PartPtr,     std::set<PartPtr> >     base2RefinedPart;
  std::map<PartEdgePtr, std::set<PartEdgePtr> > base2RefinedPartEdge;

  protected:
  // Records whether the base2RefinedPart and base2RefinedPartEdge mappings should be tracked.
  // They are not needed in all cases, so it should be used only when explicitly needed (e.g.
  // when users need to ask for analysis results at AST nodes and we need to track down all
  // analysis ATS Parts and PartEdges that correspond to each ATS SgNode*.
  bool trackBase2RefinedPartEdgeMapping;

  public:

  // Return the anchor Parts of a given function, caching the results if possible
  std::set<PartPtr> GetStartAStates();
  std::set<PartPtr> GetEndAStates();

  public:
  // Specific Composers implement these two functions
  virtual std::set<PartPtr> GetStartAStates_Spec() { throw NotImplementedException(); }
  virtual std::set<PartPtr> GetEndAStates_Spec()   { throw NotImplementedException(); }

  // Returns whether this analysis implements an Abstract Transition System graph via the methods
  // GetStartAStates_Spec() and GetEndAStates_Spec()
  virtual bool implementsATSGraph() { return false; }

  // Given a PartPtr part implemented by this ComposedAnalysis, returns the Part from its predecessor
  // from which part was derived. This function caches the results.
  PartPtr convertPart(PartPtr part);

  // Given a PartEdgePtr pedge implemented by this ComposedAnalysis, returns the PartEdge from its predecessor
  // from which pedge was derived. This function caches the results.
  PartEdgePtr convertPEdge(PartEdgePtr pedge);

  // Given a Part/PartEdge base from the ATS on which this ComposedAnalysis runs and a Part/PartEdge implemented
  // by this composed analysis that refines base, records the mapping from the base Part/PartEdge
  // to the refined Part/PartEdge.
  void registerBase2RefinedMapping(PartPtr     base, PartPtr     refined);
  void registerBase2RefinedMapping(PartEdgePtr base, PartEdgePtr refined);

  // Given a PartEdge implemented by this analysis, returns the set of refined PartEdges implemented
  // by this analysis or the NULLPart if this relationship was not tracked.
  const std::set<PartPtr>&     getRefinedParts    (PartPtr     base) const;
  const std::set<PartEdgePtr>& getRefinedPartEdges(PartEdgePtr base) const;

  public:
  typedef enum {fw=0, bw=1, none=2} direction;

  // Runs the intra-procedural analysis on the given function and returns true if
  /* // the function's NodeState gets modified as a result and false otherwise
  // state - the function's NodeState
  // analyzeFromDirectionStart - If true the function should be analyzed from its starting point from the analysis'
  //    perspective (fw: entry point, bw: exit point)*/
  void runAnalysis();
  void runAnalysisDense();
  void runAnalysisSSA();

  // Execute the analysis transfer function on a particular CFGNode within a Part, updating its dataflow info.
  // The final state of dfInfo will map a Lattice* object to each outgoing or incoming PartEdge.
  // Returns true if the Lattices in dfInfo are modified and false otherwise.
  bool transferCFGNodeDense(ComposedAnalysis* analysis,
                            /*PartPtr part, PartPtr supersetPart,*/ AnalysisParts& parts, CFGNode cn, SgNode* sgn,
                            NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                            AnalysisPartSets& ultimateParts,//const set<PartPtr>& ultimateSupersetParts,
                            // List of edges in the superset ATS in the direction of the analysis
                            const std::list<PartEdgePtr>& descIndexEdges,
                            // Wildcard edge in the superset ATS that comes into supersetPar in the direction of the analysis
                            PartEdgePtr wildCardIndexPartEdge);

  bool transferCFGNodeSSA(ComposedAnalysis* analysis, PartPtr part, CFGNode cn, SgNode* sgn, NodeState& state, std::map<PartEdgePtr,
                       std::vector<Lattice*> >& dfInfo, const std::set<PartPtr>& ultimateParts);

  //! Generic propagateDF2DescDense that can be used by leaf analysis.
  //! Transfer functions and state propagation is determined by the ComposedAnalysis* passed to it.

  // Propagates the Lattice* mapped to different PartEdges in dfInfo along these PartEdges
  void propagateDF2DescDense(ComposedAnalysis* analysis,
                        //PartPtr part, PartPtr supersetPart,
                        AnalysisParts& parts,
                        // Set of all the Parts that have already been visited by the analysis
                        std::set<PartPtr>& visited,
                        // Set of all the Parts that have been initialized
                        std::set<PartPtr>& initialized,
                        // The dataflow iterator that identifies the state of the iteration
                        dataflowPartEdgeIterator* curNodeIt                        
#ifndef DISABLE_SIGHT
			,
			// anchor that denotes the current abstract state in the debug output
                        anchor curPartAnchor,
                        // graph widget that visualizes the flow of the worklist algorithm
                        graph& worklistGraph,
                        // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                        std::map<PartPtr, std::set<anchor> >& toAnchors,
                        // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                        std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			);

  //! TightComposer implents the following method by calling generic version of this function on each analysis.
  //! FWDataflow, BWDataflow which are dataflow analyses implements this method by passing itself to the generic version.
  virtual void propagateDF2DescDense(//PartPtr part, PartPtr supersetPart,
                          AnalysisParts& parts,
                          // Set of all the Parts that have already been visited by the analysis
                          std::set<PartPtr>& visited,
                          // Set of all the Parts that have been initialized
                          std::set<PartPtr>& initialized,
                          // The dataflow iterator that identifies the state of the iteration
                          dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
			  ,
                          // anchor that denotes the current abstract state in the debug output
                          anchor curPartAnchor,
                          // graph widget that visualizes the flow of the worklist algorithm
                          graph& worklistGraph,
                          // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                          std::map<PartPtr, std::set<anchor> >& toAnchors,
                          // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                          std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			  )=0;

  void propagateDF2DescSSA(ComposedAnalysis* analysis,
                        PartPtr part,
                        bool modified,
                        // Set of all the Parts that have already been visited by the analysis
                        std::set<PartPtr>& visited,
                        // Set of all the Parts that have been initialized
                        std::set<PartPtr>& initialized,
                        // The dataflow iterator that identifies the state of the iteration
                        dataflowPartEdgeIterator* curNodeIt,
                        std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo
#ifndef DISABLE_SIGHT
			,
                        // anchor that denotes the current abstract state in the debug output
                        anchor curPartAnchor,
                        // graph widget that visualizes the flow of the worklist algorithm
                        graph& worklistGraph,
                        // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                        std::map<PartPtr, std::set<anchor> >& toAnchors,
                        // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                        std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			);

  // Invokes the analysis-specific method to set the ATS location PartEdges of all the newly-computed
  // Lattices at part

  // Generic version that can be called by leaf analyses
  void setDescendantLatticeLocationsDense(
                          ComposedAnalysis* analysis,
                          /*PartPtr part, PartPtr supersetPart*/
                          AnalysisParts& parts);

  //! TightComposer implents the following method by calling generic version of this function on each analysis.
  //! FWDataflow, BWDataflow which are dataflow analyses implements this method by passing itself to the generic version.
  virtual void setDescendantLatticeLocationsDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts)=0;

  //! Register each dataflow analysis for the given part with NodeState map
  //! Each leaf dataflow analysis can initialize the NodeState calling initializeState
  //! part - the Part of the ATS on top of which this analysis is running.
  //! supersetPart - analyses that implement the ATS will compute a Part of their own to refine the Part .
  //!                currently being analyzed. supersetPart denotes the Part that this new Part should be refining.
  //!                If this the analysis is being composed loosely, part==supersetPart. However, if it is being
  //!                composed tightly, the ATS that part belongs to is not fully constructed and thus, it cannot be
  //!                used as a supersetPart until after the analysis has finished processing and its ATS is completed.
  virtual void initNodeState(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts)=0;

  // Generates the initial lattice state for the given dataflow node. Implementations
  // fill in the lattices above and below this part, as well as the facts, as needed. Since in many cases
  // the lattices above and below each node are the same, implementors can alternately implement the
  // genInitLattice and genInitFact functions, which are called by the default implementation of initializeState.
  virtual void initializeStateDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts, NodeState& state);
  virtual void initializeStateSSA(PartPtr part, NodeState& state);

  // Initializes the state of analysis lattices at the given function, part and edge into our out of the part
  // by setting initLattices to refer to freshly-allocated Lattice objects.
  virtual void genInitLattice(//PartPtr part, PartEdgePtr pedge, PartPtr supersetPart,
                             const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                             std::vector<Lattice*>& initLattices) {}

  // Initializes the state of analysis facts at the given function and part by setting initFacts to
  // freshly-allocated Fact objects.
  virtual void genInitFact(/*PartPtr part, */ const AnalysisParts& parts, std::vector<NodeFact*>& initFacts) {}

  //! Generic transferAState that can be used by leaf analysis.
  //! Transfer functions and state propagation is determined by the ComposedAnalysis* passed to it.
  void transferAStateDense(ComposedAnalysis* analysis,
                           /*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                           std::set<PartPtr>& visited, bool firstVisit,
                           std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt,
                           std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfoPost,
                           //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                           AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
                           ,
			   anchor curPartAnchor,
                           graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                           std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			   );
  
  void transferPropagateAStateSSA(ComposedAnalysis* analysis, PartPtr part, std::set<PartPtr>& visited, bool firstVisit,
                               std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
			       ,
			       anchor curPartAnchor,
                               graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                               std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			       );

  //! TightComposer implents the following method by calling generic version of this function on each analysis.
  //! FWDataflow, BWDataflow which are dataflow anlayses implements this method by passing itself to the generic version.
  virtual void transferAStateDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                                   std::set<PartPtr>& visited, bool firstVisit,
                                   std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt,
                                   std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfoPost,
                                   //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                                   AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
				   ,
                                   anchor curPartAnchor,
                                   graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                                   std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
				   )=0;
  
  virtual void transferPropagateAStateSSA(PartPtr part, std::set<PartPtr>& visited, bool firstVisit,
                                         std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
					 ,
					 anchor curPartAnchor,
                                         graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                                         std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
					 )=0;

  // propagates the dataflow info from the current node's NodeState (curNodeState) to the next node's
  // NodeState (nextNodeState)
  bool propagateStateToNextNode(
              std::map<PartEdgePtr, std::vector<Lattice*> >& curNodeState,  AnalysisParts& parts, //PartPtr curPart,  PartPtr curSupersetPart,
              std::map<PartEdgePtr, std::vector<Lattice*> >& nextNodeState, AnalysisParts& nextParts /*PartPtr nextPart, PartPtr nextSupersetPart*/);

  //virtual NodeState*initializeFunctionNodeState(const Function &func, NodeState *fState) = 0;
  virtual std::set<PartPtr> getInitialWorklist() = 0;
  virtual std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state) = 0;
  virtual std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state) = 0;
  virtual std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state, ComposedAnalysis* analysis) = 0;
  virtual std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state, ComposedAnalysis* analysis) = 0;
  virtual void setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite) = 0;
  virtual void setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite) = 0;

  // If we're currently at a function call, use the associated inter-procedural
  // analysis to determine the effect of this function call on the dataflow state.
  //virtual void transferFunctionCall(const Function &caller, PartPtr callPart, CFGNode callCFG, NodeState *state) = 0;

  virtual std::list<PartPtr> getDescendants(PartPtr p) = 0;

  virtual std::list<PartEdgePtr> getEdgesToDescendants(PartPtr part) = 0;

  // Returns the set of Parts that denote the end of the ATS.
  // getUltimateParts() returns the Parts from the ATS over which the analysis is being run.
  // If the analysis is being composed loosely, this ATS is already complete when the analysis starts.
  // If it is a tight composition, the ATS is created as the analysis runs.
  virtual AnalysisPartSets getUltimateParts() = 0;

  /* // getUltimateSupersetParts() returns the Parts from the completed ATS that the current analysis
  // may be refining. getUltimateParts() == getUltimateSupersetParts() for loose composition but not
  // for tight composition.
  virtual std::set<PartPtr> getUltimateSupersetParts() = 0;*/
  virtual dataflowPartEdgeIterator* getIterator() = 0;

  // Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
  // with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
  virtual void remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat)=0;

  virtual direction getDirection() = 0;

  virtual std::string str(std::string indent="") const { return "ComposedAnalysis"; }

  // --- NodeState2AllMapper ---
  // ComposedAnalyses must implement methods from NodeState2AllMapper. By default, these calls are routed to
  // the composer of this analysis but in the case of TightComposer, which is both a ComposedAnalysis and a
  // Composer, the calls are implemented by the TightComposer itself with no additional forwarding.

  // Given a Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisParts object that contains all the Parts relevant for analysis.
  virtual AnalysisParts NodeState2All(PartPtr part, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis.
  virtual AnalysisPartEdges NodeState2All(PartEdgePtr pedge, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a Part from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisParts object that contains the input and index the Parts relevant for analysis.
  virtual AnalysisParts Input2Index(PartPtr part, bool indexes_valid=true, bool inputs_valid=true);

  // Given a PartEdge from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisPartEdges object that contains the input and index the Parts relevant for analysis.
  virtual AnalysisPartEdges Input2Index(PartEdgePtr pedge, bool indexes_valid=true, bool inputs_valid=true);
}; // class ComposedAnalysis

/* Forward Dataflow Analysis */
class FWDataflow  : public ComposedAnalysis
{
  public:

  FWDataflow(bool trackBase2RefinedPartEdgeMapping, bool useSSA): ComposedAnalysis(trackBase2RefinedPartEdgeMapping, useSSA)
  {}

  //! Register each dataflow analysis for the given part with NodeState map
  //! Each leaf dataflow analysis can initialize the NodeState calling initializeState
  //! part - the Part of the ATS on top of which this analysis is running.
  //! supersetPart - analyses that implement the ATS will compute a Part of their own to refine the Part .
  //!                currently being analyzed. supersetPart denotes the Part that this new Part should be refining.
  //!                If this the analysis is being composed loosely, part==supersetPart. However, if it is being
  //!                composed tightly, the ATS that part belongs to is not fully constructed and thus, it cannot be
  //!                used as a supersetPart until after the analysis has finished processing and its ATS is completed.
  void initNodeState(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts);

  void propagateDF2DescDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                             // Set of all the Parts that have already been visited by the analysis
                             std::set<PartPtr>& visited,
                             // Set of all the Parts that have been initialized
                             std::set<PartPtr>& initialized,
                             // The dataflow iterator that identifies the state of the iteration
                             dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
			     ,
                             // anchor that denotes the current abstract state in the debug output
                             anchor curPartAnchor,
                             // graph widget that visualizes the flow of the worklist algorithm
                             graph& worklistGraph,
                             // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                             std::map<PartPtr, std::set<anchor> >& toAnchors,
                             // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                             std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			     );

  void transferAStateDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                           std::set<PartPtr>& visited, bool firstVisit,
                           std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt,
                           std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfoPost,
                           //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                           AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
			   ,
                           anchor curPartAnchor,
                           graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                           std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			   );
  
  void transferPropagateAStateSSA(PartPtr part, std::set<PartPtr>& visited, bool firstVisit,
                               std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
			       ,
			       anchor curPartAnchor,
                               graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                               std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			       );

  // Invokes the analysis-specific method to set the ATS location PartEdges of all the newly-computed
  // Lattices at part
  void setDescendantLatticeLocationsDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts);

  //NodeState* initializeFunctionNodeState(const Function &func, NodeState *fState);
  std::set<PartPtr> getInitialWorklist();
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state);
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state);
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state, ComposedAnalysis* analysis);
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state, ComposedAnalysis* analysis);
  void setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite);
  void setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite);

  //void transferFunctionCall(const Function &func, PartPtr callPart, CFGNode callCFG, NodeState *state);
  std::list<PartPtr> getDescendants(PartPtr p);

  std::list<PartEdgePtr> getEdgesToDescendants(PartPtr part);

  // Returns the set of Parts that denote the end of the ATS.
  // getUltimateParts() returns the Parts from the ATS over which the analysis is being run.
  // If the analysis is being composed loosely, this ATS is already complete when the analysis starts.
  // If it is a tight composition, the ATS is created as the analysis runs.
  AnalysisPartSets getUltimateParts();

  /* // getUltimateSupersetParts() returns the Parts from the completed ATS that the current analysis
  // may be refining. getUltimateParts() == getUltimateSupersetParts() for loose composition but not
  // for tight composition.
  std::set<PartPtr> getUltimateSupersetParts();*/

  dataflowPartEdgeIterator* getIterator();

  // Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
  // with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
  void remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat);

  direction getDirection() { return fw; }
};

/* Backward Dataflow Analysis */
class BWDataflow  : public ComposedAnalysis
{
  public:

  BWDataflow(bool trackBase2RefinedPartEdgeMapping, bool useSSA): ComposedAnalysis(trackBase2RefinedPartEdgeMapping, useSSA)
  {}

  //! Register each dataflow analysis for the given part with NodeState map
  //! Each leaf dataflow analysis can initialize the NodeState calling initializeState
  //! part - the Part of the ATS on top of which this analysis is running.
  //! supersetPart - analyses that implement the ATS will compute a Part of their own to refine the Part .
  //!                currently being analyzed. supersetPart denotes the Part that this new Part should be refining.
  //!                If this the analysis is being composed loosely, part==supersetPart. However, if it is being
  //!                composed tightly, the ATS that part belongs to is not fully constructed and thus, it cannot be
  //!                used as a supersetPart until after the analysis has finished processing and its ATS is completed.
  void initNodeState(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts);

  void propagateDF2DescDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                             // Set of all the Parts that have already been visited by the analysis
                             std::set<PartPtr>& visited,
                             // Set of all the Parts that have been initialized
                             std::set<PartPtr>& initialized,
                             // The dataflow iterator that identifies the state of the iteration
                             dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
			     ,
                             // anchor that denotes the current abstract state in the debug output
                             anchor curPartAnchor,
                             // graph widget that visualizes the flow of the worklist algorithm
                             graph& worklistGraph,
                             // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                             std::map<PartPtr, std::set<anchor> >& toAnchors,
                             // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                             std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			     );

  void transferAStateDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                           std::set<PartPtr>& visited, bool firstVisit,
                           std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt,
                           std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfoPost,
                           //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                           AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
			   ,
                           anchor curPartAnchor,
                           graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                           std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			   );
  
  void transferPropagateAStateSSA(PartPtr part, std::set<PartPtr>& visited, bool firstVisit,
                               std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
			       ,
			       anchor curPartAnchor,
                               graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                               std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			       );

  // Invokes the analysis-specific method to set the ATS location PartEdges of all the newly-computed
  // Lattices at part
  void setDescendantLatticeLocationsDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts);

  //NodeState* initializeFunctionNodeState(const Function &func, NodeState *fState);
  std::set<PartPtr> getInitialWorklist();
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state);
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state);
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state, ComposedAnalysis* analysis);
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state, ComposedAnalysis* analysis);
  void setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite);
  void setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite);

  //void transferFunctionCall(const Function &func, PartPtr callPart, CFGNode callCFG, NodeState *state);
  std::list<PartPtr> getDescendants(PartPtr p);

  std::list<PartEdgePtr> getEdgesToDescendants(PartPtr part);

  // Returns the set of Parts that denote the end of the ATS.
  // getUltimateParts() returns the Parts from the ATS over which the analysis is being run.
  // If the analysis is being composed loosely, this ATS is already complete when the analysis starts.
  // If it is a tight composition, the ATS is created as the analysis runs.
  AnalysisPartSets getUltimateParts();

  /* // getUltimateSupersetParts() returns the Parts from the completed ATS that the current analysis
  // may be refining. getUltimateParts() == getUltimateSupersetParts() for loose composition but not
  // for tight composition.
  std::set<PartPtr> getUltimateSupersetParts();*/

  dataflowPartEdgeIterator* getIterator();

  // Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
  // with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
  void remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat);

  direction getDirection() { return bw; }
};


/* Dataflow Analysis that doesn't have a direction but is still a compositional
   analysis (e.g. Syntactic analysis or orthogonal array analysis)*/
class UndirDataflow  : public ComposedAnalysis
{
  public:

  UndirDataflow() : ComposedAnalysis(/*trackBase2RefinedPartEdgeMapping*/ false, /*useSSA*/ false)
  {}

  UndirDataflow(bool trackBase2RefinedPartEdgeMapping, bool useSSA) :
    ComposedAnalysis(trackBase2RefinedPartEdgeMapping, useSSA)
  {}

  // relevant only for directional dataflow analysis
  void initNodeState(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts) { assert(0); }

  void propagateDF2DescDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                             // Set of all the Parts that have already been visited by the analysis
                             std::set<PartPtr>& visited,
                             // Set of all the Parts that have been initialized
                             std::set<PartPtr>& initialized,
                             // The dataflow iterator that identifies the state of the iteration
                             dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
			     ,
                             // anchor that denotes the current abstract state in the debug output
                             anchor curPartAnchor,
                             // graph widget that visualizes the flow of the worklist algorithm
                             graph& worklistGraph,
                             // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                             std::map<PartPtr, std::set<anchor> >& toAnchors,
                             // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                             std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			     ) { assert(0); }

  void transferAStateDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                           std::set<PartPtr>& visited, bool firstVisit,
                           std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt,
                           std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfoPost,
                           //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                           AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
			   ,			   
                           anchor curPartAnchor,
                           graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                           std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
			   ) { assert(0); }
  
  void transferPropagateAStateSSA(PartPtr part, std::set<PartPtr>& visited, bool firstVisit,
                                 std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
				 ,
				 anchor curPartAnchor,
                                 graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                                 std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
				 )  { assert(0); }

  // Invokes the analysis-specific method to set the ATS location PartEdges of all the newly-computed
  // Lattices at part
  void setDescendantLatticeLocationsDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts)
  { assert(0); }

  //NodeState* initializeFunctionNodeState(const Function &func, NodeState *fState) { return NULL; }
  std::set<PartPtr> getInitialWorklist() { return std::set<PartPtr>(); }
  static std::map<PartEdgePtr, std::vector<Lattice*> > emptyMap;
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state) { return emptyMap; }
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state) { return emptyMap; }
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state, ComposedAnalysis* analysis) { return emptyMap; }
  std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state, ComposedAnalysis* analysis) { return emptyMap; }
  void setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite) { }
  void setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite) { }

  //void transferFunctionCall(const Function &func, PartPtr callPart, CFGNode callCFG, NodeState *state) {};

  std::list<PartPtr> getDescendants(PartPtr p) { std::list<PartPtr> empty; return empty; }

  std::list<PartEdgePtr> getEdgesToDescendants(PartPtr part) { std::list<PartEdgePtr> empty; return empty; }

  // Returns the set of Parts that denote the end of the ATS.
  // getUltimateParts() returns the Parts from the ATS over which the analysis is being run.
  // If the analysis is being composed loosely, this ATS is already complete when the analysis starts.
  // If it is a tight composition, the ATS is created as the analysis runs.
  AnalysisPartSets getUltimateParts() { return AnalysisPartSets(); }

  /* // getUltimateSupersetParts() returns the Parts from the completed ATS that the current analysis
  // may be refining. getUltimateParts() == getUltimateSupersetParts() for loose composition but not
  // for tight composition.
  std::set<PartPtr> getUltimateSupersetParts() { return std::set<PartPtr>(); }*/

  dataflowPartEdgeIterator* getIterator() { return NULL; }

  // Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
  // with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
  void remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat) { }

  direction getDirection() { return none; }

  // Dummy transfer function since undirected analyses does not propagate flow information
  bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    return true;
  }
};


// #######################################
// ##### UTILITY PASSES and ANALYSES #####
// #######################################

/******************************************************
 ***            printDataflowInfoPass               ***
 *** Prints out the dataflow information associated ***
 *** with a given analysis for every CFG node a     ***
 *** function.                                      ***
 ******************************************************/
class printDataflowInfoPass : public FWDataflow
{
  Analysis* analysis;

  public:
  printDataflowInfoPass(Analysis *analysis) : FWDataflow(/*trackBase2RefinedPartEdgeMapping*/ false, /*useSSA*/ false)
  {
          this->analysis = analysis;
  }

  // Initializes the state of analysis lattices, for analyses that produce the same lattices above and below each node
  void genInitLattice(/*PartPtr part, PartEdgePtr pedge, PartPtr supersetPart, */
                      const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                      std::vector<Lattice*>& initLattices);

  bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

  // pretty print for the object
  std::string str(std::string indent="") const
  { return "printDataflowInfoPass"; }
};

/***************************************************
 ***            checkDataflowInfoPass            ***
 *** Checks the results of the composed analysis ***
 *** chain at special assert calls.              ***
 ***************************************************/
class checkDataflowInfoPass : public FWDataflow
{
  private:
  int numErrors;

  public:
  checkDataflowInfoPass() : FWDataflow(/*trackBase2RefinedPartEdgeMapping*/ false, /*useSSA*/ false), numErrors(0) { }
  checkDataflowInfoPass(int numErrors): FWDataflow(/*trackBase2RefinedPartEdgeMapping*/ false, /*useSSA*/ false), numErrors(numErrors) { }

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy() { return boost::make_shared<checkDataflowInfoPass>(numErrors); }

  int getNumErrors() const { return numErrors; }

  // Initializes the state of analysis lattices, for analyses that produce the same lattices above and below each node
  void genInitLattice(/*PartPtr part, PartEdgePtr pedge, PartPtr supersetPart, */
                      const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                      std::vector<Lattice*>& initLattices);

  bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

  // pretty print for the object
  std::string str(std::string indent="") const
  { return "checkDataflowInfoPass"; }
};


/*class InitDataflowState : public UnstructuredPassAnalysis
{
  ComposedAnalysis::direction dir;

  public:
  InitDataflowState(ComposedAnalysis* analysis, ComposedAnalysis::direction dir) : UnstructuredPassAnalysis(analysis), dir(dir)
  { }

  void visit(PartPtr p, NodeState& state);
};*/

} // namespace fuse;
#endif
