#ifndef COMPOSE_H
#define COMPOSE_H

#include "analysis.h"
#include "abstract_object.h"
#include "partitions.h"
#include "graphIterator.h"
#include "composed_analysis.h"
/* GB 2012-09-02: DESIGN NOTE
   Analyses and abstract objects written for the compositional framework must support a wide range of functionality,
   both mandatory (e.g. mayEqual/mustEqual in AbstractObjects) and optional (e.g. Expr2MemLoc() in ComposedAnalysis).
   In general optional functionality should go into the composer, which will then find an implementor for this
 * functionality. Mandatory functionality should be placed as body-less virtual methods that are required for specific
 * instances of AbstractObject and ComposedAnalysis. Note that although we do currently have a way for ComposedAnalyses
 * to provide optional functionality and have the Composer find it, the same is not true for AbstractObjects because
 * thus far we have not found a use-case where we wanted AbstractObject functionality to be optional. This issue
 * may need to be revisited.
*/

// ------------------------------
// ----- Composition Driver -----
// ------------------------------

namespace fuse {
/* Class for declaring the debug level used in various code modules. It ensures that at start-up time
   (before main() runs) a dbglog attribute with a given name and value is created and it persists
   until the end of the analysis (exit of main()). */
class debugLevel {
  void* debugAttribute;
  int level;
  public:
  debugLevel(const char* name, int level) : level(level) {
/*    static bool sightInitialized=false;
    if(!sightInitialized) { */
#ifndef DISABLE_SIGHT
    debugAttribute = attr_enter(name, (long)level);
#endif
  }
  ~debugLevel() {
#ifndef DISABLE_SIGHT
    attr_exit(debugAttribute);
#endif
  }
  int operator()() const { return level; }
};

// Declaration of a debug level with a given name and a given value
#define DEBUG_LEVEL(debugLevelName, debugLevelVal) \
static debugLevel debugLevelName(#debugLevelName, debugLevelVal);

class Composer;

// Represents the state of our knowledge about some fact
typedef enum {Unknown=-1, False=0, True=1} knowledgeT;

// #####################
// ##### COMPOSERS #####
// #####################

class Composer: public virtual NodeState2AllMapper
{
  public:

  Composer();

  // The types of functions we may be interested in calling
  typedef enum {any, codeloc, val, memloc, memregion, atsGraph, ssaGraph} reqType;

  // Abstract interpretation functions that return this analysis' abstractions that
  // represent the outcome of the given SgExpression at the end of all execution prefixes
  // that terminate at PartEdge pedge
  // The objects returned by these functions are expected to be deallocated by their callers.

  // Analyses that are being composed inside a given composer provide a pointer to themselves
  // in the client argument. Code that uses the composer from the outside, does not need to provide
  // a client.

  virtual ValueObjectPtr Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;

/*  // Variant of Expr2Val that runs the query on the analysis that called the method rather than
  // some prior server analysis
  virtual ValueObjectPtr Expr2ValSelf(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* self=NULL)=0;*/

  // Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the
  // given node n, where the part edge denotes the set of execution prefixes that terminate at SgNode n.
  virtual ValueObjectPtr OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;

  virtual CodeLocObjectPtr Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;

  // Variant of Expr2CodeLoc that inquires about the code location denoted by the operand of the
  // given node n, where the part edge denotes the set of execution prefixes that terminate at SgNode n.
  virtual CodeLocObjectPtr OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;

/*  // Variant of OperandExpr2Value that runs the query on the analysis that called the method rather than
  // some prior server analysis
  virtual ValueObjectPtr OperandExpr2ValSelf(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* self)=0;

  virtual CodeLocObjectPtr Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;*/

  virtual MemRegionObjectPtr  Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;
  // Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  virtual MemRegionObjectPtr OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;

  virtual MemLocObjectPtr  Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;

/*  // Variant of Expr2MemLoc that runs the query on the analysis that called the method rather than
  // some prior server analysis
  virtual MemLocObjectPtr Expr2MemLocSelf(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* self=NULL)=0;*/

  // Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  virtual MemLocObjectPtr OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client=NULL)=0;

/*  // Variant of OperandExpr2MemLoc that runs the query on the analysis that called the method rather than
  // some prior server analysis
  virtual MemLocObjectPtr OperandExpr2MemLocSelf(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* self)=0;
*/
  // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
  // Wrapper for calling type-specific versions of mayEqual without forcing the caller to care about the type of objects.
  bool mayEqual(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge, ComposedAnalysis* client);

  // Special calls for each type of AbstractObject
  virtual bool mayEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  //virtual bool mayEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  virtual bool mayEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  //virtual bool mayEqualML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }

  // Returns whether the given pair of AbstractObjects are must-equal at the given PartEdge
  // Wrapper for calling type-specific versions of mustEqual without forcing the caller to care about the type of object
  bool mustEqual(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge, ComposedAnalysis* client);

  // Special calls for each type of AbstractObject
  virtual bool mustEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  //virtual bool mustEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  virtual bool mustEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  //virtual bool mustEqualML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }

  // Returns whether the two abstract objects denote the same set of concrete objects
  // Wrapper for calling type-specific versions of equalSet without forcing the caller to care about the type of object
  virtual bool equalSet(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge, ComposedAnalysis* client);

  // Special calls for each type of AbstractObject
  virtual bool equalSetV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  //virtual bool equalSetCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  virtual bool equalSetMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  //virtual bool equalSetML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }

  // Returns whether abstract object ao1 denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the abstract object ao2.
  // Wrapper for calling type-specific versions of subSet without forcing the caller to care about the type of object
  virtual bool subSet(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge, ComposedAnalysis* client);

  // Special calls for each type of AbstractObject
  virtual bool subSetV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  //virtual bool subSetCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  virtual bool subSetMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }
  //virtual bool subSetML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client) { throw NotImplementedException(); }

  // Returns whether the given AbstractObject is live at the given PartEdge
  // Wrapper for calling type-specific versions of isLive without forcing the caller to care about the type of object
  bool isLive(AbstractObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client);

  // Special calls for each type of AbstractObject
  virtual bool isLiveV (ValueObjectPtr val,    PartEdgePtr pedge, ComposedAnalysis* client)=0;
  //virtual bool isLiveCL(CodeLocObjectPtr cl,   PartEdgePtr pedge, ComposedAnalysis* client)=0;
  virtual bool isLiveMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client)=0;
  //virtual bool isLiveML(MemLocObjectPtr ml,    PartEdgePtr pedge, ComposedAnalysis* client)=0;

  virtual bool OperandIsLiveV (SgNode* n, SgNode* operand, ValueObjectPtr val,    PartEdgePtr pedge, ComposedAnalysis* client)=0;
  //virtual bool OperandIsLiveCL(SgNode* n, SgNode* operand, CodeLocObjectPtr cl,   PartEdgePtr pedge, ComposedAnalysis* client)=0;
  virtual bool OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client)=0;
  //virtual bool OperandIsLiveML(SgNode* n, SgNode* operand, MemLocObjectPtr ml,    PartEdgePtr pedge, ComposedAnalysis* client)=0;

  // Computes the meet of from and to and saves the result in to.
  // Returns true if this causes this to change and false otherwise.
  virtual bool meetUpdateV (ValueObjectPtr     to, ValueObjectPtr     from, PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  //virtual bool meetUpdateCL(CodeLocObjectPtr   to, CodeLocObjectPtr   from, PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  virtual bool meetUpdateMR(MemRegionObjectPtr to, MemRegionObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  //virtual bool meetUpdateML(MemLocObjectPtr    to, MemLocObjectPtr    from, PartEdgePtr pedge, ComposedAnalysis* analysis)=0;

  // Returns whether the given AbstractObject corresponds to the set of all sub-executions
  virtual bool isFullV (ValueObjectPtr     val, PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  //virtual bool isFullCL(CodeLocObjectPtr   cl,  PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  virtual bool isFullMR(MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  //virtual bool isFullML(MemLocObjectPtr    ml,  PartEdgePtr pedge, ComposedAnalysis* analysis)=0;

  // Returns whether the given AbstractObject corresponds to the empty set of executions
  virtual bool isEmptyV (ValueObjectPtr     val, PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  //virtual bool isEmptyCL(CodeLocObjectPtr   cl,  PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  virtual bool isEmptyMR(MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* analysis)=0;
  //virtual bool isEmptyML(MemLocObjectPtr    ml,  PartEdgePtr pedge, ComposedAnalysis* analysis)=0;

  // Returns a ValueObject that denotes the size of this memory region
  virtual ValueObjectPtr getRegionSizeMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* analysis)=0;

  // Return the anchor Parts of a given function
  virtual std::set<PartPtr> GetStartAStates(ComposedAnalysis* client)=0;
  // There may be multiple terminal points in the application (multiple calls to exit(), returns from main(), etc.)
  virtual std::set<PartPtr> GetEndAStates(ComposedAnalysis* client)=0;

  // Return an SSAGraph object that describes the overall structure of the transition system
  virtual SSAGraph* GetSSAGraph(ComposedAnalysis* client)=0;

  // Given a Part or PartEdge implemented by the entire composer, returns the set of refined Parts/PartEdges implemented
  // by the composer or the NULLPart/NULLPartEdge if this relationship was not tracked.
  virtual const std::set<PartPtr>&     getRefinedParts    (PartPtr     base) const=0;
  virtual const std::set<PartEdgePtr>& getRefinedPartEdges(PartEdgePtr base) const=0;

  // Returns the number of Parts/PartEdges that refine the given base
  virtual unsigned int  numRefinedParts    (PartPtr     base) const=0;
  virtual unsigned int  numRefinedPartEdges(PartEdgePtr base) const=0;

  /*
  // Returns whether dom is a dominator of part
  virtual std::set<PartPtr> isDominator(PartPtr part, PartPtr dom, ComposedAnalysis* client);

  // Map that maintains each part's dominator set. When a part is not mapped to any set, it is assumed that it
  // is mapped to the set of all parts
  std::map<PartPtr, std::set<PartPtr> > dominators;
  // The set of all parts
  set<PartPtr> allParts;
  // Flag that indicates whether the dominator-related data structures have been computed
  bool domInit;

  // Initializes the dominator-related data structures
  void initializeDominators(ComposedAnalysis* client);*/

  /* // Given a Part that this analysis implements returns the Part from the preceding analysis
  // that this Part corresponds to (we assume that each analysis creates one or more Parts and PartEdges
  // for each Part and PartEdge of its parent analysis)
  virtual PartPtr     sourcePart    (PartPtr part)=0;
  // Given a PartEdge that this analysis implements returns the PartEdge from the preceding analysis
  // that this PartEdge corresponds to (we assume that each analysis creates one or more Parts and PartEdges
  // for each Part and PartEdge of its parent analysis)
  virtual PartEdgePtr sourcePartEdge(PartEdgePtr pedge)=0;*/
};
typedef boost::shared_ptr<Composer> ComposerPtr;

// Classes FuncCaller and FuncCallerArgs wrap the functionality to call functions
// Expr2* and ComposerGetFunction*Part on analyses inside the ChainComposer. FuncCaller
// exposes the () operator that takes FuncCallerArgs as the argument. Specific implementations
// decide what function the () operator actually calls and what the arguments actually are
// but by abstracting these details away we can get a general algorithm for the ChainComposer to
// choose the analysis that implements a given function.
/*class FuncCallerArgs
{
  // Dummy virtual methods to allow dynamic casting on classes derived from FuncCallerArgs
  virtual void dummy() {}
};
*/
template<class RetObject, class ArgsObject>
class FuncCaller
{
  public:
  // Calls the implementation of some API operation inside server analysis on behalf of client analysis.
  virtual RetObject operator()(const ArgsObject& args, PartEdgePtr pedge, ComposedAnalysis* server, ComposedAnalysis* client)=0;
  // Returns a string representation of the returned object
  virtual std::string retStr(RetObject ml)=0;
  // Returns the name of the function being called, for debugging purposes
  virtual std::string funcName() const=0;
};

// Records the info required to forward queries of each type. We maintain one instance of this class
// for each analysis
class CCQueryServers
{
  public:
  // Records the last analysis in the composition chain that can answer queries of a given type
  ComposedAnalysis* lastCodeLocAnalysis;
  ComposedAnalysis* lastValAnalysis;
  ComposedAnalysis* lastMemLocAnalysis;
  ComposedAnalysis* lastMemRegionAnalysis;
  ComposedAnalysis* lastATSGraphAnalysis;
  ComposedAnalysis* lastSSAGraphAnalysis;

  // The number of ATSGraph analyses that separate the current analysis from the last analysis that
  // can answer a given query type. This is also the number of times we'll call PartEdge->getParent()
  // to convert the edges of the current analysis to those of the server of the given query type.
  // There is no counter for ATS Graph queries since it would always be 0.
  int ATSGraphsSinceLastCodeLocAnalysis;
  int ATSGraphsSinceLastValAnalysis;
  int ATSGraphsSinceLastMemLocAnalysis;
  int ATSGraphsSinceLastMemRegionAnalysis;

  CCQueryServers() {}

  // Initialize this object with the info of the initial analysis at the start of the composition chain
  CCQueryServers(ComposedAnalysis* startAnalysis);

  // Given the information from the prior analysis in the composition chain, create a record that
  // accounts for queries being serviced by nextAnalysis
  CCQueryServers(const CCQueryServers& info, ComposedAnalysis* nextAnalysis);
  std::string str(std::string indent="") const;
};

// Simple implementation of a Composer where the analyses form a linear sequence of
// dependences
class ChainComposer : public Composer, public UndirDataflow
{
  std::list<ComposedAnalysis*> allAnalyses;
  std::list<ComposedAnalysis*> doneAnalyses;
  // The analysis that is currently executing
  ComposedAnalysis* currentAnalysis;
  // The optional pass that tests the results of the other analyses
  ComposedAnalysis* testAnalysis;
  // If true, the debug output of testAnalysis is emitted.
  bool verboseTest;

  // Maps each completed analysis to the CCQueryServers object that records which analyses it
  // should query to answer all types of queries
  std::map<ComposedAnalysis*, CCQueryServers> queryInfo;

  // Maps each analysis that implements the ATS to the ATSGraph that has been generated for it, if any
  std::map<ComposedAnalysis*, SSAGraph*> SSAGraphCache;

  public:
  ChainComposer(const std::list<ComposedAnalysis*>& analyses,
                ComposedAnalysis* testAnalysis, bool verboseTest,
                ComposedAnalysis* stxAnalysis=NULL);
  ChainComposer(const ChainComposer&);

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy()
  { return boost::make_shared<ChainComposer>(*this); }

  ~ChainComposer();

  private:


  // Class implements a single operator that takes as arguments a ComposedAnalysis* and a PartEdgePtr.
  // When given a boost function that takes either a ComposedAnalysis* or a PartEdge as an argument
  // (not both), calls it with it just this argument
  template<class RetType>
  class CallWithEitherComposedAnalysisOrPartEdge {
    public:
    virtual RetType operator()(PartEdgePtr pedge, ComposedAnalysis* analysis, Composer::reqType type) const=0;
  };

  // Generic function that looks up the composition chain from the given client
  // analysis and returns the result produced by the first instance of the function
  // called by the caller object found along the way.
  template<class RetType>
  RetType callServerAnalysisFunc(
           // The name of the operation being called
           std::string opName,
           // Calls some operation on an analysis
           //function<RetType (ComposedAnalysis*)> callOp,
           const CallWithEitherComposedAnalysisOrPartEdge<RetType>& callOp,
           // Returns whether a given analysis supports the operation or not
           //boost::function<bool (ComposedAnalysis*)> isSupported,
           // The type of the request (any, memloc, codeloc, val, atsGraph)
           Composer::reqType type,
           // Returns whether a given analysis implements the operation tightly (it uses itself as
           // a server for the operation) or loosely (it implements the operation but uses other
           // analyses when it needs to call it)
           boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)> checkTightness,
           // Returns a string representation of the result of the operation
           boost::function<std::string (RetType, std::string)> ret2Str,
           // Returns an object that denotes the intersection of multiple instances of RetType
           boost::function<RetType (const std::map<ComposedAnalysis*, RetType>&)> createIntersection,
           // The PartEdge at which the operation is being called
           PartEdgePtr pedge,
           // The client analysis calling the operation
           ComposedAnalysis* client,
           // Flag that indicates whether the operation's invocation should be logged in detail
           bool verbose);

  // Invokes callOp on the PartEdge(s) that correspond to the given operand of SgNode n, with PartEdge
  // pedge guaranteed to terminate at n. Returns the union of all the returned values.
  template<class RetPtrType, class UnionRetType, class UnionRetPtrType>
  UnionRetPtrType OperandExpr2Any
                       (std::string OpName, SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client,
                        boost::function<RetPtrType (PartEdgePtr, ComposedAnalysis*)> callOp,
                        boost::function<UnionRetPtrType (std::list<RetPtrType>)> unionOp,
                        // Returns a string representation of the result of the operation
                        boost::function<std::string (RetPtrType, std::string)> ret2Str);

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

  // Variant of OperandExpr2Value that runs the query on the analysis that called the method rather than
  // some prior server analysis
  //  ValueObjectPtr OperandExpr2ValSelf(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* self);

  MemRegionObjectPtr Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);

  private:
  MemRegionObjectPtr Expr2MemRegion_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);

  public:
  // Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemRegionObjectPtr OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);

/*  // Variant of OperandExpr2MemLoc that runs the query on the analysis that called the method rather than
  // some prior server analysis
  MemLocObjectPtr OperandExpr2MemLocSelf(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* self);

  // #SA: Variant of Expr2MemRegion called by an analysis to call its own Expr2MemRegion inorder
  // to interpret complex expressions
  //MemRegionObjectPtr Expr2MemRegionSelf(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* self);
*/

  MemLocObjectPtr Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);

  // Variant of Expr2MemLoc that runs the query on the analysis that called the method rather than
  // some prior server analysis
  //MemLocObjectPtr Expr2MemLocSelf(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* self);

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

  /*// Calls the isLive() method of the given MemLocObject that denotes an operand of the given SgNode n within
  // the context of its own PartEdges and returns true if it may be live within any of them
  bool OperandIsLive(SgNode* n, SgNode* operand, MemLocObjectPtr ml, PartEdgePtr pedge, ComposedAnalysis* client);*/

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

  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr getRegionSizeMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* analysis);

  // Return the anchor Parts of an application
  std::set<PartPtr> GetStartAStates(ComposedAnalysis* client);
  std::set<PartPtr> GetEndAStates(ComposedAnalysis* client);

  // Return an SSAGraph object that describes the overall structure of the transition system
  SSAGraph* GetSSAGraph(ComposedAnalysis* client);

  // Given a Part implemented by the entire composer, returns the set of refined Parts implemented
  // by the composer or the NULLPart if this relationship was not tracked.
  const std::set<PartPtr>& getRefinedParts(PartPtr base) const;

  // Given a PartEdge implemented by the entire composer, returns the set of refined PartEdges implemented
  // by the composer or the NULLPartEdge if this relationship was not tracked.
  const std::set<PartEdgePtr>& getRefinedPartEdges(PartEdgePtr base) const;

  // Returns the number of Parts/PartEdges that refine the given base
  unsigned int  numRefinedParts    (PartPtr     base) const;
  unsigned int  numRefinedPartEdges(PartEdgePtr base) const;

  protected:
  // Maps base parts from the ATS on which this analysis runs to the parts implemented
  // by this analysis that refine themto the edges that refine them. Set inside
  // registerBase2RefinedMapping(), which is called inside the PartEdge constructor
  // when the connection between a given refined part and its base part is first established.
  // NOTE: Once we change ChainComposer to derive from ComposedAnalysis, we can modify
  //       this to implement that interface.
  //std::map<PartEdgePtr, boost::shared_ptr<std::set<PartEdgePtr> > > base2RefinedPartEdge;

  public:

  // -----------------------------------------
  // ----- Methods from ComposedAnalysis -----
  // -----------------------------------------

  // Runs the analysis, combining the intra-analysis with the inter-analysis of its choice
  // ChainComposer invokes the runAnalysis methods of all its constituent analyses in sequence
  void runAnalysis();

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

  // Returns whether the class implements Expr* loosely or tightly (if it does at all)
  ComposedAnalysis::implTightness Expr2ValTightness();
  ComposedAnalysis::implTightness Expr2CodeLocTightness();
  ComposedAnalysis::implTightness Expr2MemRegionTightness();
  ComposedAnalysis::implTightness Expr2MemLocTightness();

  // Return the anchor Parts of a given function
  std::set<PartPtr> GetStartAStates_Spec();
  std::set<PartPtr> GetEndAStates_Spec();

  // Given a Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisParts object that contains all the Parts relevant for analysis.
  AnalysisParts NodeState2All(PartPtr part, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis.
  AnalysisPartEdges NodeState2All(PartEdgePtr pedge, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a Part from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisParts object that contains the input and index the Parts relevant for analysis.
  AnalysisParts Input2Index(PartPtr part, bool indexes_valid=true, bool inputs_valid=true);

  // Given a PartEdge from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisPartEdges object that contains the input and index the Parts relevant for analysis.
  AnalysisPartEdges Input2Index(PartEdgePtr pedge, bool indexes_valid=true, bool inputs_valid=true);

  std::string str(std::string indent="") const;
};

// Composer that invokes multiple analyses in parallel (they do not interact) and runs them to completion independently.
// It also implements the ComposedAnalysis interface and can be used by another Composer as an analysis. Thus, when this
// Composer's constituent analyses ask a query on the composer, it merely forwards this query to its parent Composer.
// Further, when its parent Composer makes queries of it, this Composer forwards those queries to its constituent
// analyses and returns an Intersection object that contains their responses.
class LooseParallelComposer : public Composer, public UndirDataflow
{
  std::list<ComposedAnalysis*> allAnalyses;

  // Indicates whether at least one sub-analysis implements a partition
  knowledgeT subAnalysesImplementPartitions;

  public:
  LooseParallelComposer(const std::list<ComposedAnalysis*>& analyses, knowledgeT subAnalysesImplementPartitions=Unknown);

  // ---------------------------------
  // ----- Methods from Composer -----
  // ---------------------------------

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy() { return boost::make_shared<LooseParallelComposer>(allAnalyses, subAnalysesImplementPartitions); }

  // The Expr2* and GetFunction*Part functions are implemented by calling the same functions in the parent composer

  // Abstract interpretation functions that return this analysis' abstractions that
  // represent the outcome of the given SgExpression.
  // The objects returned by these functions are expected to be deallocated by their callers.
  ValueObjectPtr Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);

  // Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  ValueObjectPtr OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);

  CodeLocObjectPtr Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);

  // Variant of Expr2CodeLoc that inquires about the code location denoted by the operand of the
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  CodeLocObjectPtr OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);

  MemRegionObjectPtr Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);

  // Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemRegionObjectPtr OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);

  // #SA: Variant of Expr2MemRegion for an analysis to call its own Expr2MemRegion method to interpret complex expressions
  ///MemRegionObjectPtr Expr2MemRegionSelf(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* self);
  MemLocObjectPtr Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client);

  // Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemLocObjectPtr OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client);

  // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
  bool mayEqualV (ValueObjectPtr     val1, ValueObjectPtr     val2, PartEdgePtr pedge, ComposedAnalysis* client);
  //bool mayEqualCL(CodeLocObjectPtr   cl1,  CodeLocObjectPtr   cl2,  PartEdgePtr pedge, ComposedAnalysis* client);
  bool mayEqualMR(MemRegionObjectPtr mr1,  MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client);
  //bool mayEqualML(MemLocObjectPtr    ml1,  MemLocObjectPtr    ml2,  PartEdgePtr pedge, ComposedAnalysis* client);

  // Returns whether the given pair of AbstractObjects are must-equal at the given PartEdge
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
  bool isLiveV (ValueObjectPtr     val, PartEdgePtr pedge, ComposedAnalysis* client);
  //bool isLiveCL(CodeLocObjectPtr   cl,  PartEdgePtr pedge, ComposedAnalysis* client);
  bool isLiveMR(MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* client);
  //bool isLiveML(MemLocObjectPtr    ml,  PartEdgePtr pedge, ComposedAnalysis* client);

  // Calls the isLive() method of the given AbstractObject that denotes an operand of the given SgNode n within
  // the context of its own PartEdges and returns true if it may be live within any of them
  bool OperandIsLiveV (SgNode* n, SgNode* operand, ValueObjectPtr     val, PartEdgePtr pedge, ComposedAnalysis* client);
  //bool OperandIsLiveCL(SgNode* n, SgNode* operand, CodeLocObjectPtr   cl,  PartEdgePtr pedge, ComposedAnalysis* client);
  bool OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* client);
  //bool OperandIsLiveML(SgNode* n, SgNode* operand, MemLocObjectPtr    ml,  PartEdgePtr pedge, ComposedAnalysis* client);

  // Computes the meet of from and to and saves the result in to.
  // Returns true if this causes this to change and false otherwise.
  bool meetUpdateV (ValueObjectPtr     to, ValueObjectPtr     from, PartEdgePtr pedge, ComposedAnalysis* analysis);
  //bool meetUpdateCL(CodeLocObjectPtr   to, CodeLocObjectPtr   from, PartEdgePtr pedge, ComposedAnalysis* analysis);
  bool meetUpdateMR(MemRegionObjectPtr to, MemRegionObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis);
  //bool meetUpdateML(MemLocObjectPtr    to, MemLocObjectPtr    from, PartEdgePtr pedge, ComposedAnalysis* analysis);

  // Returns whether the given AbstractObject corresponds to the set of all sub-executions
  bool isFullV (ValueObjectPtr     val, PartEdgePtr pedge, ComposedAnalysis* analysis);
  //bool isFullCL(CodeLocObjectPtr   cl,  PartEdgePtr pedge, ComposedAnalysis* analysis);
  bool isFullMR(MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* analysis);
  //bool isFullML(MemLocObjectPtr    ml,  PartEdgePtr pedge, ComposedAnalysis* analysis);

  // Returns whether the given AbstractObject corresponds to the empty set of executions
  bool isEmptyV (ValueObjectPtr     val, PartEdgePtr pedge, ComposedAnalysis* analysis);
  //bool isEmptyCL(CodeLocObjectPtr   cl,  PartEdgePtr pedge, ComposedAnalysis* analysis);
  bool isEmptyMR(MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* analysis);
  //bool isEmptyML(MemLocObjectPtr    ml,  PartEdgePtr pedge, ComposedAnalysis* analysis);

  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr getRegionSizeMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* analysis);

  // Return the anchor Parts of a given function
  std::set<PartPtr> GetStartAStates(ComposedAnalysis* client);
  std::set<PartPtr> GetEndAStates(ComposedAnalysis* client);

  // Return an SSAGraph object that describes the overall structure of the transition system
  SSAGraph* GetSSAGraph(ComposedAnalysis* client);

  // Common functionality for GetStartAStates_Spec() and GetEndAStates_Spec()

  // Functors that call either GetStartStates or GetEndStates in GetStartOrEndStates_Spec
  class callStartOrEndAStates {
    public:
    virtual std::set<PartPtr> callGetStartOrEndAStates_ComposedAnalysis(ComposedAnalysis* analysis)=0;
    virtual std::set<PartPtr> callGetStartOrEndAStates_Composer(Composer* composer, ComposedAnalysis* analysis)=0;
  };

  std::set<PartPtr> GetStartOrEndAStates_Spec(callStartOrEndAStates& caller, std::string funcName);

  // Given a Part implemented by the entire composer, returns the set of refined Parts implemented
  // by the composer or the NULLPart if this relationship was not tracked.
  const std::set<PartPtr>& getRefinedParts(PartPtr base) const
  { assert(0); }

  // Given a PartEdge implemented by the entire composer, returns the set of refined PartEdges implemented
  // by the composer or the NULLPartEdge if this relationship was not tracked.
  const std::set<PartEdgePtr>& getRefinedPartEdges(PartEdgePtr base) const
  { assert(0); }

  // Returns the number of Parts/PartEdges that refine the given base
  unsigned int  numRefinedParts    (PartPtr     base) const { return 0; }
  unsigned int  numRefinedPartEdges(PartEdgePtr base) const { return 0; }
  // -----------------------------------------
  // ----- Methods from ComposedAnalysis -----
  // -----------------------------------------

  // Runs the analysis, combining the intra-analysis with the inter-analysis of its choice
  // LooseParallelComposer invokes the runAnalysis methods of all its constituent analyses in sequence
  void runAnalysis();

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
  bool implementsExpr2Val       ();
  bool implementsExpr2CodeLoc   ();
  bool implementsExpr2MemRegion ();
  bool implementsExpr2MemLoc    ();

  // Returns whether the class implements Expr* loosely or tightly (if it does at all)
  ComposedAnalysis::implTightness Expr2ValTightness();
  ComposedAnalysis::implTightness Expr2CodeLocTightness();
  ComposedAnalysis::implTightness Expr2MemRegionTightness();
  ComposedAnalysis::implTightness Expr2MemLocTightness();

  // Return the anchor Parts of a given function
  std::set<PartPtr> GetStartAStates_Spec();
  std::set<PartPtr> GetEndAStates_Spec();

  // Given a Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisParts object that contains all the Parts relevant for analysis.
  AnalysisParts NodeState2All(PartPtr part, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis.
  AnalysisPartEdges NodeState2All(PartEdgePtr pedge, bool NodeStates_valid=true, bool indexes_valid=true, bool inputs_valid=true);

  // Given a Part from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisParts object that contains the input and index the Parts relevant for analysis.
  AnalysisParts Input2Index(PartPtr part, bool indexes_valid=true, bool inputs_valid=true);

  // Given a PartEdge from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisPartEdges object that contains the input and index the Parts relevant for analysis.
  AnalysisPartEdges Input2Index(PartEdgePtr pedge, bool indexes_valid=true, bool inputs_valid=true);

  // When Expr2* is queried for a particular analysis on edge pedge, exported by this LooseParallelComposer
  // this function translates from the pedge that the LooseParallelComposer::Expr2* is given to the PartEdge
  // that this particular sub-analysis runs on. If some of the analyses that were composed in parallel with
  // this analysis (may include this analysis) implement partition graphs, we know that
  // GetStartAState/GetEndAStates wrapped them in IntersectionPartEdges. In this case this function
  // converts pedge into an IntersectionPartEdge and queries its getPartEdge method. Otherwise,
  // GetStartAState/GetEndAStates do no wrapping and thus, we can return pedge directly.
  PartEdgePtr getEdgeForAnalysis(PartEdgePtr pedge, ComposedAnalysis* analysis);

  std::string str(std::string indent="") const;
};

}; // namespace fuse

#endif
