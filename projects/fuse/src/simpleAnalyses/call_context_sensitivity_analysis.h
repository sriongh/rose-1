#ifndef CALL_CONTEXT_SENSITIVITY_ANALYSIS_H
#define CALL_CONTEXT_SENSITIVITY_ANALYSIS_H

#include "compose.h"
#include "boost/enable_shared_from_this.hpp"

namespace fuse
{
/* #######################################
   ##### Calling Context Sensitivity #####
   ####################################### */

// This is an analysis that adds the sensitivity to calling contexts to the Abstract Transition System it runs on,

//extern int callContextSensitivityDebugLevel;

/* ###########################
   ##### CallCtxSensPart #####
   ########################### */

class CallPartContext;
typedef CompSharedPtr<CallPartContext> CallPartContextPtr;
class CallCtxSensPart;
typedef CompSharedPtr<CallCtxSensPart> CallCtxSensPartPtr;
class CallCtxSensPartEdge;
typedef CompSharedPtr<CallCtxSensPartEdge> CallCtxSensPartEdgePtr;
class CallCtxSensMR;
typedef boost::shared_ptr<CallCtxSensMR> CallCtxSensMRPtr;
class CallCtxSensML;
typedef boost::shared_ptr<CallCtxSensML> CallCtxSensMLPtr;

class CallCtxSensLattice;
class CallContextSensitivityAnalysis;

class CallPartContext : public PartContext
{
  std::list<PartPtr> stack;
  public:
  CallPartContext() {}
  CallPartContext(const std::list<PartPtr>& stack): stack(stack) {}
  CallPartContext(const CallPartContext& context): stack(context.stack) {}
  CallPartContext(CallPartContextPtr context): stack(context->stack) {}

  // Returns the current depth of the context stack
  unsigned int getCtxtStackDepth() const;

  // Push the given Part onto the context stack
  void push(PartPtr part);

  // Pop the given Part off the context stack
  void pop();

  // Returns the last (most recent) element on the stack
  PartPtr last() const;

  // Returns the number of elements on the stack
  int size() const;

  // Returns a list of PartContextPtr objects that denote more detailed context information about
  // this PartContext's internal contexts.
  std::list<PartContextPtr> getSubPartContexts() const;

  bool operator==(const PartContextPtr& that_arg) const;
  bool operator<(const PartContextPtr& that_arg) const;

  bool operator==(const PartContext& that_arg) const;
  bool operator<(const PartContext& that_arg) const;

  // Returns true if the set of sub-executions denoted by this context is a non-strict subset of the set of
  // sub-executions denoted by that context.
  //bool isSubsetEq(CallPartContextPtr that) const;
  bool isSubsetEq(const CallPartContext& that) const;

  // Returns true if the sets of sub-executions denoted by this and that contexts overlap.
  //bool setOverlap(CallPartContextPtr that) const;
  bool setOverlap(const CallPartContext& that) const;

  // Computes the meet of this and that context and saves the result in this.
  // Returns true if this causes this to change and false otherwise.
  //bool meetUpdate(CallPartContextPtr that, PartEdgePtr pedge);
  bool meetUpdate(const CallPartContext& that, PartEdgePtr pedge);

  // Returns whether this context denotes the set of all possible sub-execution prefixes.
  bool isFull(PartEdgePtr pedge);
  // Returns whether this context denotes the empty set of sub-execution prefixes
  bool isEmpty(PartEdgePtr pedge);

  // Set this object to represent the set of all possible sub-executions.
  // Return true if this causes the object to change and false otherwise.
  bool setToFull();

  std::string str(std::string indent="") const;
}; // CallPartContext

// This object's current level in the lattice: (bottom, dead, live)
class CallCtxSensPart : public Part
{
  friend class CallCtxSensPartEdge;
  friend class CallCtxSensLattice;
  friend class CallContextSensitivityAnalysis;
  friend class CallCtxSensMR;
  friend class CallCtxSensML;

  // Data structures for caching the results of various functions
  bool cacheInitialized_outEdges;
  std::list<PartEdgePtr> cache_outEdges;
  bool cacheInitialized_inEdges;
  std::list<PartEdgePtr> cache_inEdges;
  bool cacheInitialized_CFGNodes;
  std::set<CFGNode> cache_CFGNodes;
  bool cacheInitialized_matchingCallParts;
  std::set<PartPtr> cache_matchingCallParts;
  bool cacheInitialized_matchingEntryExitParts;
  std::set<PartPtr> cache_matchingEntryExitParts;
  bool cacheInitialized_inEdgeFromAny;
  CallCtxSensPartEdgePtr cache_inEdgeFromAny;
  bool cacheInitialized_outEdgeToAny;
  CallCtxSensPartEdgePtr cache_outEdgeToAny;
  std::map<CallCtxSensPart*, bool> cache_equal;
  std::map<CallCtxSensPart*, bool> cache_less;

  // The Part's calling context, which is a stack of function calls
  CallPartContext context;

  // If we're at the limit for calling context depth, records whether we have recursively called the function
  // in which the last call in context resides.
  bool recursive;

  // Identifies the function in which the last call in context resides.
  Function lastCtxtFunc;

  // The CallContextSensitivityAnalysis this Part is associated with
  CallContextSensitivityAnalysis* ccsa;

  protected:
  // Creates a CallCtxSensPart from the given base, using no calling context
  CallCtxSensPart(PartPtr base, ComposedAnalysis* analysis);

  // Creates a CallCtxSensPart from the given base, using the given context information
  CallCtxSensPart(PartPtr base, const CallPartContext& context, const Function& lastCtxtFunc, bool recursive, ComposedAnalysis* analysis);

  // Creates a CallCtxSensPart from the given base, using the context information from contextPart
  CallCtxSensPart(PartPtr base, CallCtxSensPartPtr contextPart, ComposedAnalysis* analysis);
  CallCtxSensPart(PartPtr base, const CallCtxSensPart& contextPart, ComposedAnalysis* analysis);

  CallCtxSensPart(const CallCtxSensPart& that);

  // Initializes the state of the cache
  void initCache();

  public:
  // Parts must be created via static construction methods to make it possible to separately
  // initialize them. This is needed to allow Parts to register themselves with global directories,
  // a process that requires the creation of a shared pointer to themselves.
  static CallCtxSensPartPtr create(PartPtr base, ComposedAnalysis* analysis) {
    CallCtxSensPartPtr newPart(boost::shared_ptr<CallCtxSensPart>(new CallCtxSensPart(base, analysis)));
    newPart->init();
    return newPart;
  }
  static CallCtxSensPartPtr create(PartPtr base, const CallPartContext& context, const Function& lastCtxtFunc, bool recursive, ComposedAnalysis* analysis) {
    CallCtxSensPartPtr newPart(boost::shared_ptr<CallCtxSensPart>(new CallCtxSensPart(base, context, lastCtxtFunc, recursive, analysis)));
    newPart->init();
    return newPart;
  }
  static CallCtxSensPartPtr create(PartPtr base, CallCtxSensPartPtr contextPart, ComposedAnalysis* analysis) {
    CallCtxSensPartPtr newPart(boost::shared_ptr<CallCtxSensPart>(new CallCtxSensPart(base, contextPart, analysis)));
    newPart->init();
    return newPart;
  }
  static CallCtxSensPartPtr create(PartPtr base, const CallCtxSensPart& contextPart, ComposedAnalysis* analysis) {
    CallCtxSensPartPtr newPart(boost::shared_ptr<CallCtxSensPart>(new CallCtxSensPart(base, contextPart, analysis)));
    newPart->init();
    return newPart;
  }
  static CallCtxSensPartPtr create(const CallCtxSensPart& that) {
    CallCtxSensPartPtr newPart(boost::shared_ptr<CallCtxSensPart>(new CallCtxSensPart(that)));
    newPart->init();
    return newPart;
  }

  private:
  // Returns a shared pointer to this of type CallCtxSensPartPtr;
  CallCtxSensPartPtr get_shared_this();

  public:
  // -------------------------------------------
  // Functions that need to be defined for Parts
  // -------------------------------------------
  std::list<PartEdgePtr> outEdges();
  std::list<PartEdgePtr> inEdges();
  std::set<CFGNode> CFGNodes() const;

  // If this Part corresponds to a function call/return, returns the set of Parts that contain
  // its corresponding return/call, respectively.
  std::set<PartPtr> matchingCallParts() const;

  // If this Part corresponds to a function entry/exit, returns the set of Parts that contain
  // its corresponding exit/entry, respectively.
  std::set<PartPtr> matchingEntryExitParts() const;

  /*
  // Let A={ set of execution prefixes that terminate at the given anchor SgNode }
  // Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
  // Since to reach a given SgNode an execution must first execute all of its operands it must
  //    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
  // This function is the inverse of m: given the anchor node and operand as well as the
  //    Part that denotes a subset of A (the function is called on this part),
  //    it returns a list of Parts that partition O.
  std::list<PartPtr> getOperandPart(SgNode* anchor, SgNode* operand);*/

  // Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
  PartEdgePtr inEdgeFromAny();

  // Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
  PartEdgePtr outEdgeToAny();

  // Returns the specific context of this Part. Can return the NULLPartContextPtr if this
  // Part doesn't implement a non-trivial context.
  PartContextPtr getPartContext() const;

  // Returns the specific context of this Part. Can return the NULLPartContextPtr if this
  // Part doesn't implement a non-trivial context.
  CallPartContextPtr getCCSPartContext() const;

  bool equal(const PartPtr& o) const;
  bool less(const PartPtr& o)  const;

  // Pretty print for the object
  std::string str(std::string indent="") const;
}; // class CallCtxSensPart

/* ###############################
   ##### CallCtxSensPartEdge #####
   ############################### */

class CallCtxSensPartEdge : public PartEdge {
  private:
  // The part that this object is wrapping
  CallCtxSensPartPtr src;
  CallCtxSensPartPtr tgt;

  friend class CallCtxSensPart;
  friend class CallContextSensitivityAnalysis;
  friend class CallCtxSensMR;
  friend class CallCtxSensML;
  protected:
  // Constructor to be used when constructing the edges (e.g. from genInitLattice()).
  CallCtxSensPartEdge(PartEdgePtr baseEdge, CallCtxSensPartPtr src, CallCtxSensPartPtr tgt, ComposedAnalysis* analysis);

  CallCtxSensPartEdge(const CallCtxSensPartEdge& that);

  public:
  // PartEdges must be created via static construction methods to make it possible to separately
  // initialize them. This is needed to allow PartEdges to register themselves with global directories,
  // a process that requires the creation of a shared pointer to themselves.
  static CallCtxSensPartEdgePtr create(PartEdgePtr baseEdge, CallCtxSensPartPtr src, CallCtxSensPartPtr tgt, ComposedAnalysis* analysis) {
    CallCtxSensPartEdgePtr newPartEdge(boost::shared_ptr<CallCtxSensPartEdge>(new CallCtxSensPartEdge(baseEdge, src, tgt, analysis)));
    newPartEdge->init();
    return newPartEdge;
  }
  static CallCtxSensPartEdgePtr create(const CallCtxSensPartEdge& that) {
    CallCtxSensPartEdgePtr newPartEdge(boost::shared_ptr<CallCtxSensPartEdge>(new CallCtxSensPartEdge(that)));
    newPartEdge->init();
    return newPartEdge;
  }

  private:
  // Returns a shared pointer to this of type CallCtxSensPartEdgePtr
  CallCtxSensPartEdgePtr get_shared_this();

  public:
  PartPtr source() const;
  PartPtr target() const;

  // Let A={ set of execution prefixes that terminate at the given anchor SgNode }
  // Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
  // Since to reach a given SgNode an execution must first execute all of its operands it must
  //    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
  // This function is the inverse of m: given the anchor node and operand as well as the
  //    PartEdge that denotes a subset of A (the function is called on this PartEdge),
  //    it returns a list of PartEdges that partition O.
  // A default implementation that walks the server analysis-provided graph backwards to find
  //    matching PartEdges is provided.
  std::list<PartEdgePtr> getOperandPartEdge(SgNode* anchor, SgNode* operand);

  // If the source Part corresponds to a conditional of some sort (if, switch, while test, etc.)
  // it must evaluate some predicate and depending on its value continue, execution along one of the
  // outgoing edges. The value associated with each outgoing edge is fixed and known statically.
  // getPredicateValue() returns the value associated with this particular edge. Since a single
  // Part may correspond to multiple CFGNodes getPredicateValue() returns a map from each CFG node
  // within its source part that corresponds to a conditional to the value of its predicate along
  // this edge.
  std::map<CFGNode, boost::shared_ptr<SgValueExp> > getPredicateValue();

  bool equal(const PartEdgePtr& o) const;
  bool less(const PartEdgePtr& o)  const;

  // Pretty print for the object
  std::string str(std::string indent="") const;
}; // CallCtxSensPartEdge

/* #########################
   ##### CallCtxSensMR #####
   ######################### */

// Memory region object that wraps server-provided MemRegionObjects but adds to them the context
// from which they were derived.
class CallCtxSensMR : public MemRegionObject
{
  protected:
    // The server MemRegionObject that this object wraps
    MemRegionObjectPtr baseMR;

    // The context of this object
    CallPartContext context;

    // The analysis that produced this MemLoc
    CallContextSensitivityAnalysis* ccsa;

  public:
    CallCtxSensMR(SgNode* sgn, MemRegionObjectPtr baseMR, const CallPartContext& context, CallContextSensitivityAnalysis* ccsa);
    CallCtxSensMR(const CallCtxSensMR& that);

    // pretty print
    //std::string str(std::string indent="") const;
    std::string str(std::string indent="") const;// { return ((const CallCtxSensMR*)this)->str(indent); }

    // copy this object and return a pointer to it
    MemRegionObjectPtr copyAOType() const;

    bool mayEqualAO(MemRegionObjectPtr that, PartEdgePtr pedge);
    bool mustEqualAO(MemRegionObjectPtr that, PartEdgePtr pedge);

    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSetAO(MemRegionObjectPtr o, PartEdgePtr pedge);

    // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the given abstract object.
    bool subSetAO(MemRegionObjectPtr o, PartEdgePtr pedge);

    bool isLiveAO(PartEdgePtr pedge);

    // Computes the meet of this and that and saves the result in this
    // returns true if this causes this to change and false otherwise
    bool meetUpdateAO(MemRegionObjectPtr that, PartEdgePtr pedge);

    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    bool isFullAO(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the empty set.
    bool isEmptyAO(PartEdgePtr pedge);

    // Returns a ValueObject that denotes the size of this memory region
    ValueObjectPtr getRegionSizeAO(PartEdgePtr pedge);

    // Set this object to represent the set of all possible MemLocs
    // Return true if this causes the object to change and false otherwise.
    bool setToFull();
    // Set this LattCallCtxSensMLice object to represent the empty set of MemLocs.
    // Return true if this causes the object to change and false otherwise.
    bool setToEmpty();

    // Returns true if this MemRegionObject denotes a finite set of concrete regions
    bool isConcrete() { return baseMR->isConcrete(); }
    // Returns the number of concrete values in this set
    int concreteSetSize() { return baseMR->concreteSetSize(); }
    // Returns the type of the concrete regions (if there is one)
    SgType* getConcreteType() { return baseMR->getConcreteType(); }
    // Returns the set of concrete memory regions as SgExpressions, which allows callers to use
    // the normal ROSE mechanisms to decode it
    std::set<SgNode* > getConcrete() { return baseMR->getConcrete(); }

    // Returns whether all instances of this class form a hierarchy. Every instance of the same
    // class created by the same analysis must return the same value from this method!
    bool isHierarchy() const;
    // AbstractObjects that form a hierarchy must inherit from the AbstractionHierarchy class

    // Returns a key that uniquely identifies this particular AbstractObject in the
    // set hierarchy.
    const hierKeyPtr& getHierKey() const;
}; // CallCtxSensMR

/* #########################
   ##### CallCtxSensML #####
   ######################### */

// Memory location object that wraps server-provided MemLocObjects but adds to them the context
// from which they were derived.
class CallCtxSensML : public MemLocObject
{
  protected:
    // The server MemLocObject that this object wraps
    MemLocObjectPtr baseML;

    // The context of this object
    CallPartContext context;

    // The analysis that produced this MemLoc
    CallContextSensitivityAnalysis* ccsa;

  public:
    CallCtxSensML(SgNode* sgn, MemLocObjectPtr baseML, const CallPartContext& context, CallContextSensitivityAnalysis* ccsa);
    CallCtxSensML(const CallCtxSensML& that);

    virtual MemRegionObjectPtr getRegion() const;
    virtual ValueObjectPtr     getIndex() const;
    virtual SgNode* getBase() const;

    // pretty print
    //std::string str(std::string indent="") const;
    std::string str(std::string indent="") const;// { return ((const CallCtxSensML*)this)->str(indent); }

    // Allocates a copy of this object and returns a regular pointer to it
    MemLocObject* copyMLPtr() const;

    // Allocates a copy of this object and returns a pointer to it
    MemLocObjectPtr copyAOType() const;

    bool mayEqualAO(MemLocObjectPtr that, PartEdgePtr pedge);
    bool mustEqualAO(MemLocObjectPtr that, PartEdgePtr pedge);

    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSetAO(MemLocObjectPtr o, PartEdgePtr pedge);

    // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the given abstract object.
    bool subSetAO(MemLocObjectPtr o, PartEdgePtr pedge);

    bool isLiveAO(PartEdgePtr pedge);

    // Computes the meet of this and that and saves the result in this
    // returns true if this causes this to change and false otherwise
    bool meetUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge);

    // Computes the intersection of this and that and saves the result in this
    // returns true if this causes this to change and false otherwise
    bool intersectUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge);

    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    bool isFullAO(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the empty set.
    bool isEmptyAO(PartEdgePtr pedge);

    // Set this object to represent the set of all possible MemLocs
    // Return true if this causes the object to change and false otherwise.
    bool setToFull();
    // Set this Lattice object to represent the empty set of MemLocs.
    // Return true if this causes the object to change and false otherwise.
    bool setToEmpty();

    // Returns whether all instances of this class form a hierarchy. Every instance of the same
    // class created by the same analysis must return the same value from this method!
    bool isHierarchy() const;
    // AbstractObjects that form a hierarchy must inherit from the AbstractionHierarchy class

    // Returns a key that uniquely identifies this particular AbstractObject in the
    // set hierarchy.
    const hierKeyPtr& getHierKey() const;
}; // CallCtxSensML

/* ##############################
   ##### CallCtxSensLattice #####
   ############################## */

// Lattice object associated with each PartEdge of the server analysis. Maintains multiple CallCtxSensPartEdges
// that may reach a given Part.
class CallCtxSensLattice: public FiniteLattice
{
  private:
  // Each lattice is maintained on some PartEdge of the server's Abstract Transition System.
  // Each such edge will be associated with one or more CallCtxSensPartEdges. Since our client
  // analyses will access these edges by calling CallCtxSensPart::outEdges and
  // CallCtxSensPart::inEdges, this information is maintained as a mapping from Parts to
  // their incoming and outgoing edges.
  std::map<CallCtxSensPartPtr, std::set<CallCtxSensPartPtr> > outgoing;
  std::map<CallCtxSensPartPtr, std::set<CallCtxSensPartPtr> > incoming;

  friend class CallCtxSensPart;
  friend class CallContextSensitivityAnalysis;

  /* Each Part and PartEdge in the given Abstract Transition System is replicated one or more times based on
   *    calling context information of its function. The context is a stack of PartPtrs of function calls or bodies.
   *    When a function is entered, the PartPtr that identifies where it is called from (the call itself
   *    or the function from which the call was made) is added onto the stack of the callee's CallCtxSensPart. This
   *    Part's successors carry the same call context information and when we encounter a function exit, we simply
   *    pop the latest call from the stack.
   * The size of the stack must be statically bounded to maintain efficiency and to keep the analysis time finite
   *    even with recursive programs. Suppose that the analysis reaches a point where it sees function A calling
   *    function B at call site c. If it has not yet reached the limit on the amount of context that may be explicitly
   *    represented, it can add c to the context and set this as the context as the entry node of B reachable from
   *    call site c. However, if the context size limit is reached we cannot maintain the additional precision and
   *    instead treat all code execucted deeper in the calling stack as having the same context. If the program is
   *    non-recursive if we ever observe a return from A, then we'll know to pop the last element from the context
   *    off the stack since it must be A's call site.
   * If the program is recursive then it is possible for it to call a function recursively an unbounded number of
   *    times while in the same context. If there are no calls to A within this context then nothing special needs
   *    to be done since we are still sure that any returns from A observed in this context are soundly modeled by
   *    popping the last element from the context state. If there are calls to A (they are recursive by definition)
   *    then it becomes impossible to tell whether a given return from A corresponds to an exit from the original call
   *    to A that is the last element in the context stack or the return from some deeper nested call. As such,
   *    such calls are modeled by considering both possibilities: one outgoing edge for the case where the top element
   *    in the context stack is popped and one where the stack is unchanged.
   */
  public:
  CallCtxSensLattice(PartEdgePtr baseEdge, ComposedAnalysis* analysis/*, const std::list<PartPtr>& context, bool recursive*/);

  CallCtxSensLattice(const CallCtxSensLattice& that);

  void initialize();

  // Returns a copy of this lattice
  Lattice* copy() const;

  // Overwrites the state of "this" Lattice with "that" Lattice
  void copy(Lattice* that);

  bool operator==(Lattice* that) /*const*/;

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
  Lattice* remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr newPEdge);

  // Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously
  //    maintained in this lattice about them.
  // Returns true if the Lattice state is modified and false otherwise.
  bool replaceML(Lattice* newL);

  // Updates the internal state of the key in thisMap associated with iterator iThis to incorporate
  //    recursion information stored at iterator iThat. Update iThis to point to the key's location in
  //    thisMap after the update.
  // Returns true if this causes thisMap to change and false otherwise.
  bool updateMapKey(
             std::map<CallCtxSensPartPtr, std::set<CallCtxSensPartPtr> >& thisMap,
             std::map<CallCtxSensPartPtr, std::set<CallCtxSensPartPtr> >::iterator& iThis,
             std::map<CallCtxSensPartPtr, std::set<CallCtxSensPartPtr> >::iterator  iThat);

  // Updates the internal state of the key in thisSet associated with iterator iThis to incorporate
  //    recursion information stored at iterator iThat. Update iThis to point to the key's location in
  //    thisSet after the update.
  // Returns true if this causes thisMap to change and false otherwise.
  bool updateSetElement(
               std::set<CallCtxSensPartPtr>& thisSet,
               std::set<CallCtxSensPartPtr>::iterator& jThis,
               std::set<CallCtxSensPartPtr>::iterator  jThat);

  // Computes the meet of this and that and saves the result in this
  // Returns true if this causes this to change and false otherwise
  bool meetUpdate(Lattice* that);

  // Computes the intersection of this and that and saves the result in this
  // Returns true if this causes this to change and false otherwise
  bool intersectUpdate(Lattice* that);

  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  bool setToFull();

  // Set this Lattice object to represent the of no execution prefixes (empty set)
  // Return true if this causes the object to change and false otherwise.
  bool setToEmpty();

  // Set all the information associated Lattice object with this MemLocObjectPtr to full.
  // Return true if this causes the object to change and false otherwise.
  bool setMLValueToFull(MemLocObjectPtr ml);

  // Returns whether this lattice denotes the set of all possible execution prefixes.
  bool isFull();
  // Returns whether this lattice denotes the empty set.
  bool isEmpty();

  std::string str(std::string indent="") const;
}; // CallCtxSensLattice

/* ##########################################
   ##### CallContextSensitivityAnalysis #####
   ########################################## */

class CallContextSensitivityAnalysis : public FWDataflow
{
  protected:
  // The number of calls that this analysis is sensitive to
  unsigned int sensDepth;

  public:
  // The type of sensitivity:
  // callSite - calls to the same function from different calls (even if they're in the same function) are treated
  //            separately
  // callFunc - all calls to a function from the same function are treated together but calls from different
  //            functions are treated separately
  typedef enum {callSite, callFunc} sensType;

  // Records whether the given call (PartPtr) to the given function is ambiguous
  std::map<PartPtr, std::map<Function, bool> > callAmbiguity;

  protected:
  sensType type;

  public:
  CallContextSensitivityAnalysis(int sensDepth, sensType type, bool trackBase2RefinedPartEdgeMapping) :
    FWDataflow(trackBase2RefinedPartEdgeMapping, /*useSSA*/ false), sensDepth(sensDepth), type(type) {}

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy() { return boost::make_shared<CallContextSensitivityAnalysis>(sensDepth, type, trackBase2RefinedPartEdgeMapping); }

  unsigned int getSensDepth() const { return sensDepth; }
  sensType getSensType() const { return type; }

  // Initializes the state of analysis lattices at the given function, part and edge into our out of the part
  // by setting initLattices to refer to freshly-allocated Lattice objects.
  void genInitLattice(const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                      std::vector<Lattice*>& initLattices);

  bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);


  // Returns true if the given part denotes an outgoing function call to a function that is targeted
  // by other calls.
  bool isOutgoingCallAmbiguous(PartEdgePtr edge);
  bool isOutgoingCallAmbiguous(PartEdgePtr edge, Function callee);

  // Returns true if the given part denotes a function exit may return control to multiple function call sites.
  bool isFuncExitAmbiguous(PartEdgePtr edge, std::set<CFGNode>& matchNodes);

  // Returns true if the given part denotes an incoming function call to a function that is targeted
  // by other calls.
  //bool isIncomingCallAmbiguous(PartPtr part, std::set<CFGNode>& matchNodes);

  // Portion sof the transfer function that creates edges along the outgoing edges of function calls
  std::set<CallCtxSensPartPtr> createCallOutEdge(PartEdgePtr baseEdge, CallCtxSensPartPtr src);

  // Portion sof the transfer function that creates edges along the exit edges of functions
  std::set<CallCtxSensPartPtr> createFuncExitEdge(PartEdgePtr baseEdge, CallCtxSensPartPtr src);

  MemRegionObjectPtr Expr2MemRegion(SgNode* n, PartEdgePtr pedge);
  bool implementsExpr2MemRegion () { return true; }

  MemLocObjectPtr Expr2MemLoc(SgNode* n, PartEdgePtr pedge);
  bool implementsExpr2MemLoc () { return true; }

  // Return the anchor Parts of the application
  std::set<PartPtr> GetStartAStates_Spec();
  std::set<PartPtr> GetEndAStates_Spec();

  // Returns whether this analysis implements an Abstract Transition System graph via the methods
  // GetStartAStates_Spec() and GetEndAStates_Spec()
  bool implementsATSGraph() { return true; }

  // Given a PartEdge pedge implemented by this ComposedAnalysis, returns the part from its predecessor
  // from which pedge was derived. This function caches the results if possible.
  //PartEdgePtr convertPEdge_Spec(PartEdgePtr pedge);

  // pretty print for the object
  std::string str(std::string indent="") const
  { return "CallContextSensitivityAnalysis"; }
}; // CallContextSensitivityAnalysis

}; //namespace fuse


#endif  /* CALL_CONTEXT_SENSITIVITY_ANALYSIS_H */

