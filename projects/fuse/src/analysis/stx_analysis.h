#ifndef STX_ANALYSIS_H
#define STX_ANALYSIS_H

#include "partitions.h"
#include "abstract_object.h"
#include "compose.h"
#include "CallGraphTraverse.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>

namespace fuse {
  
//extern int stxAnalysisDebugLevel;
/*****************************
 ***** SyntacticAnalysis *****
 *****************************/

// This is a non-dataflow analysis that considers just the syntactic information 
// available about the memory locations, values and control locations at at CFG node. 
// It can precisely interpret static variables and lexical values but provides no 
// information that cannot be directly derived from the text of the code such as 
// the referents of pointers.

/********************
 ***** ANALYSIS *****
 ********************/

class SyntacticAnalysis : virtual public UndirDataflow
{
  private:
  static boost::shared_ptr<SyntacticAnalysis> _instance;

  public:
  SyntacticAnalysis() {
    initGlobalDeclarations();
  }
  static SyntacticAnalysis* instance();
  
  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy();

  void runAnalysis(const Function&  func, NodeState* state, bool, std::set<Function>) { }
  
  // The genInitLattice, genInitFact and transfer functions are not implemented since this 
  // is not a dataflow analysis.
  
  // Maps the given SgNode to an implementation of the ValueObject abstraction.
  ValueObjectPtr   Expr2Val(SgNode* e, PartEdgePtr pedge);
  static ValueObjectPtr   Expr2ValStatic(SgNode* e, PartEdgePtr pedge);
  bool implementsExpr2Val() { return true; }
  
  // Maps the given SgNode to an implementation of the Expr2CodeLoc abstraction.
  CodeLocObjectPtr Expr2CodeLoc(SgNode* e, PartEdgePtr pedge);
  static CodeLocObjectPtr Expr2CodeLocStatic(SgNode* e, PartEdgePtr pedge);
  bool implementsExpr2CodeLoc() { return true; }
  
  // Maps the given SgNode to an implementation of the MemLocObject abstraction.
  MemRegionObjectPtr Expr2MemRegion(SgNode* e, PartEdgePtr pedge);
  static MemRegionObjectPtr Expr2MemRegionStatic(SgNode* e, PartEdgePtr pedge);
  bool implementsExpr2MemRegion() { return true; }
  
  // Maps the given SgNode to an implementation of the MemLocObject abstraction.
  MemLocObjectPtr Expr2MemLoc(SgNode* e, PartEdgePtr pedge);
  static MemLocObjectPtr Expr2MemLocStatic(SgNode* e, PartEdgePtr pedge);
  bool implementsExpr2MemLoc() { return true; }
  
  // Detects declarations of global variables, stores them in globalDeclarations
  static std::set<SgVariableDeclaration*> globalDeclarations;
  void initGlobalDeclarations();
  
  // Return the starting Parts of the application
  std::set<PartPtr> GetStartAStates_Spec();
  
  // Adds the entry points into all the non-static functions (can be called from the outside) to 
  // the given set
  template <class ArgPartPtr>
  static void addFunctionEntries(std::set<ArgPartPtr>& states, SyntacticAnalysis* analysis);
  
  // Return the ending Parts of the application
  std::set<PartPtr> GetEndAStates_Spec();
  
  // Returns whether this analysis implements an Abstract Transition System graph via the methods
  // GetStartAStates_Spec() and GetEndAStates_Spec()
  bool implementsATSGraph() { return true; }
  
  // pretty print for the object
  std::string str(std::string indent="")
  { return "SyntacticAnalysis"; }
}; // SyntacticAnalysis

/**********************
 ***** PARTITIONS *****
 **********************/

class StxPart;
class StxPartEdge;

// A NULL CFGNode that is used as a wild-card for termination points of edges to/from anywhere
extern CFGNode NULLCFGNode;

//typedef boost::shared_ptr<StxPart> StxPartPtr;
//typedef boost::shared_ptr<StxPartEdge> StxPartEdgePtr;
typedef CompSharedPtr<StxPart> StxPartPtr;
typedef CompSharedPtr<StxPartEdge> StxPartEdgePtr;

// Returns the set of all the function calls that may call the given function
const std::set<SgFunctionCallExp*>& func2Calls(Function func);

class StxFuncContext: public PartContext
{
  Function func;
  CFGNode n;
  public:
  //StxFuncContext(Function func);
  StxFuncContext(CFGNode n);
    
  // Returns a list of PartContextPtr objects that denote more detailed context information about
  // this PartContext's internal contexts. If there aren't any, the function may just return a list containing
  // this PartContext itself.
  std::list<PartContextPtr> getSubPartContexts() const;
  
  bool operator==(const PartContextPtr& that) const;
  bool operator< (const PartContextPtr& that) const;
  
  std::string str(std::string indent="");
};
typedef CompSharedPtr<StxFuncContext> StxFuncContextPtr;

class StxPart : public Part
{
  CFGNode n;
  bool (*filter) (CFGNode cfgn); // a filter function to decide which raw CFG node to show (if return true) or hide (otherwise)
  
  friend class StxPartEdge;
  
  public:
  StxPart(CFGNode n, ComposedAnalysis* analysis, bool (*f) (CFGNode) = defaultFilter): 
    Part(analysis, NULLPart, makePtr<StxFuncContext>(n)), n(n), filter(f) {}
  StxPart(const StxPart& part):    Part((const Part&)part), n(part.n), filter(part.filter) {} 
  StxPart(const StxPartPtr& part): Part((const Part&)part), n(part->n), filter(part->filter) {} 
  StxPart(const StxPart& part,    bool (*f) (CFGNode) = defaultFilter): Part((const Part&)part), n(part.n), filter (f) {}
  StxPart(const StxPartPtr& part, bool (*f) (CFGNode) = defaultFilter): Part((const Part&)part), n(part->n), filter (f) {}
  
  private:
  // Returns a shared pointer to this of type StxPartPtr
  StxPartPtr get_shared_this();
  
  std::map<StxPartEdgePtr, bool> getOutEdges();
  
  public:
  std::list<PartEdgePtr>    outEdges();
  std::list<StxPartEdgePtr> outStxEdges();
  
  private:
  std::map<StxPartEdgePtr, bool> getInEdges();
  
  public:
  std::list<PartEdgePtr>    inEdges();
  std::list<StxPartEdgePtr> inStxEdges();
  std::set<CFGNode>  CFGNodes() const;
  
  // If this Part corresponds to a function call/return, returns the set of Parts that contain
  // its corresponding return/call, respectively.
  std::set<PartPtr> matchingCallParts() const;
  
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
  
  bool equal(const PartPtr& o) const;
  bool less (const PartPtr& o) const;
  
  std::string str(std::string indent="");
};
extern StxPartPtr NULLStxPart;

class StxPartEdge : public PartEdge
{
  CFGPath p;
  bool (*filter) (CFGNode cfgn);

  public:
  StxPartEdge(CFGNode src, CFGNode tgt, ComposedAnalysis* analysis, bool (*f) (CFGNode) = defaultFilter):
      PartEdge(analysis, NULLPartEdge), p(CFGEdge(src, tgt)), filter(f) {}
  StxPartEdge(CFGPath p, ComposedAnalysis* analysis, bool (*f) (CFGNode) = defaultFilter):
      PartEdge(analysis, NULLPartEdge), p(p), filter(f) {}
  StxPartEdge(const StxPartEdge& dfe): PartEdge((const PartEdge&)dfe), p(dfe.p), filter(dfe.filter) {}
  
  PartPtr source() const;
  StxPartPtr stxSource() const;
  PartPtr target() const;
  StxPartPtr stxTarget() const;
  
  CFGPath getPath() const { return p; }
  
  // Let A={ set of execution prefixes that terminate at the given anchor SgNode }
  // Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
  // Since to reach a given SgNode an execution must first execute all of its operands it must
  //    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
  // This function is the inverse of m: given the anchor node and operand as well as the
  //    PartEdge that denotes a subset of A (the function is called on this PartEdge), 
  //    it returns a list of PartEdges that partition O.
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
  bool less (const PartEdgePtr& o) const;
  
  std::string str(std::string indent="");
};
extern StxPartEdgePtr NULLStxPartEdge;

/****************************
 ***** ABSTRACT OBJECTS *****
 ****************************/

class StxValueObject : public ValueObject
{
  public:
  SgValueExp* val;
  
  StxValueObject(SgNode* n);
  StxValueObject(const StxValueObject& that);
  
  bool mayEqualV(ValueObjectPtr o, PartEdgePtr pedge);
  bool mustEqualV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Returns true if this object is live at the given part and false otherwise
  /*bool isLive(PartEdgePtr pedge) const
  { return true; }*/
  
  // Computes the meet of this and that and saves the result in this.
  // Returns true if this causes this to change and false otherwise.
  bool meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge);

  bool isFullV(PartEdgePtr pedge);
  bool isEmptyV(PartEdgePtr pedge);
  
  // Returns true if the given pair of SgValueExps represent the same value and false otherwise
  static bool equalValExp(SgValueExp* a, SgValueExp* b);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();
  
  std::string str(std::string indent); // pretty print for the object
    
  // Allocates a copy of this object and returns a pointer to it
  ValueObjectPtr copyV() const;
};
typedef boost::shared_ptr<StxValueObject> StxValueObjectPtr;

/*class StxCodeLocObject : public CodeLocObject
{
  public:
  PartEdgePtr pedge;
  SgExpression* code;
  
  StxCodeLocObject(SgNode* n, PartEdgePtr pedge);
  StxCodeLocObject(const StxCodeLocObject& that);
  
  bool mayEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  bool mustEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns true if this object is live at the given part and false otherwise
  // We don't currently support scope analysis for CodeLocObjects, so we default to them all being live.
  bool isLiveCL(PartEdgePtr pedge)
  { return true; }
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateCL(CodeLocObjectPtr that, PartEdgePtr pedge);

  bool isFull(PartEdgePtr pedge);
  bool isEmpty(PartEdgePtr pedge);
  
  / * Don't have good idea how to represent a finite number of options 
  bool isFiniteSet();
  std::set<AbstractObj> getValueSet();* /
   
  std::string str(std::string indent); // pretty print for the object
  
  // Allocates a copy of this object and returns a pointer to it
  CodeLocObjectPtr copyCL() const;
};
typedef boost::shared_ptr<StxCodeLocObject> StxCodeLocObjectPtr;*/

/**********************************
 ***** ABSTRACT MEMORY REGION *****
 **********************************/

// One simple implementation for abstract memory object
// This implementation is tied to the ROSE AST (another set of base classes such as expression,
// named, and aliased objects)
//

// Root class for the three variants of StxMemRegionObjects
class StxMemRegionType: public sight::printable
{
  public:
  
  // The three types of regions
  typedef enum {expr,    // Temporary storage of an expression's result
                named,   // Memory of a named variable
                storage, // Heap, stack or global memory that is not identified with a known named 
                         // variable. The set of all storage memory regions contains every named 
                         // region but no expression regions.
                all      // All memory, including heap, stack, global and expressions. Only used as a 
                         // result of merging an expression regions and storage/named regions.
               } regType;
  
  // Returns the type of this object: expr, named, storage or all
  virtual regType getType() const=0;
  
  // Returns the string representation of the given type
  static std::string MRType2Str(regType type);
  
  // Returns a unique ID that differentiates this object from others within its type.
  // Storage and All objects must have a single ID, while others depend on the expression and symbol they
  // are associated with
  virtual void* getUID() const=0;
  
  // Returns true if this object is live at the given part and false otherwise
  virtual bool isLiveMR(PartEdgePtr pedge)=0;
  
  StxMemRegionType() {};
};

typedef boost::shared_ptr<StxMemRegionType> StxMemRegionTypePtr;

class StxMemRegionObject : public MemRegionObject
{
  StxMemRegionTypePtr type;
  
  public:
  StxMemRegionObject(SgNode* n);
  StxMemRegionObject(const StxMemRegionObject& that);
  
  // Returns the type of this object: expr, named, storage or all
  StxMemRegionType::regType getType() { return type->getType(); }
  
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  bool mayEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool mustEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Returns true if this object is live at the given part and false otherwise
  bool isLiveMR(PartEdgePtr pedge);
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullMR(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyMR(PartEdgePtr pedge);
  
  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr getRegionSize(PartEdgePtr pedge) const;
  
  // Allocates a copy of this object and returns a pointer to it
  MemRegionObjectPtr copyMR() const;
  
  std::string str(std::string indent=""); // pretty print for the object
};
typedef boost::shared_ptr<StxMemRegionObject> StxMemRegionObjectPtr;
        
// The memory region of the temporary that holds the intermediate result of an expression
class StxExprMemRegionType;
typedef boost::shared_ptr<StxExprMemRegionType> StxExprMemRegionTypePtr;
extern StxExprMemRegionTypePtr NULLStxExprMemRegionType;

class StxExprMemRegionType : public StxMemRegionType
{
  // Expression, the temporary storage of which is denoted by this region
  SgExpression *expr;
  
  public:
  StxExprMemRegionType(SgExpression *expr) : expr(expr) {}
  
  // If the given SgNode corresponds to a named memory region, returns a freshly-allocated
  // StxExprMemRegionType that represents it. Otherwise, returns NULL.
  static StxExprMemRegionTypePtr getInstance(SgNode* n);
  
  // Returns the type of this object: expr, named, storage or all
  StxMemRegionType::regType getType() const { return StxMemRegionType::expr; }
  
  // Returns a unique ID that differentiates this object from others within its type.
  // Storage and All objects must have a single ID, while others depend on the expression and symbol they
  // are associated with
  void* getUID() const { return expr; }
  
  // Returns true if this object is live at the given part and false otherwise
  bool isLiveMR(PartEdgePtr pedge);
  
  std::string str(std::string indent); // pretty print for the object
};

// The memory regions of named global or stack variables
class StxNamedMemRegionType;
typedef boost::shared_ptr<StxNamedMemRegionType> StxNamedMemRegionTypePtr;
extern StxNamedMemRegionTypePtr NULLStxNamedMemRegionType;

class StxNamedMemRegionType : public StxMemRegionType
{
  // The InitializedName that syntactically denotes the memory region. 
  // Could be a symbol for variable, function, class/structure, etc.
  SgInitializedName* iname; // Must not be NULL
  SgSymbol* symbol; // May be NULL
  
  public:
  StxNamedMemRegionType(SgInitializedName* iname, SgSymbol* symbol) : iname(iname), symbol(symbol) {}
  
  // If the given SgNode corresponds to a named memory region, returns a freshly-allocated
  // StxNamedMemRegionType that represents it. Otherwise, returns NULL.
  static StxNamedMemRegionTypePtr getInstance(SgNode* n);
  
  // Returns the type of this object: expr, named, storage or all
  StxMemRegionType::regType getType() const { return StxMemRegionType::named; }
  
  // Returns a unique ID that differentiates this object from others within its type.
  // Storage and All objects must have a single ID, while others depend on the expression and symbol they
  // are associated with
  void* getUID() const { return iname; }
  
  // Returns true if this object is live at the given part and false otherwise
  bool isLiveMR(PartEdgePtr pedge);
  
  std::string str(std::string indent); // pretty print for the object
};

// The single memory region that contains all regions in stack+globals+heap
class StxStorageMemRegionType;
typedef boost::shared_ptr<StxStorageMemRegionType> StxStorageMemRegionTypePtr;
extern StxStorageMemRegionTypePtr NULLStxStorageMemRegionType;

class StxStorageMemRegionType : public StxMemRegionType
{
  public:
  StxStorageMemRegionType() {}
  
  // If the given SgNode corresponds to an storage memory region, returns a freshly-allocated
  // StxStorageMemRegionType that represents it. Otherwise, returns NULL.
  // It is assumed that n is not a named or expression type. As such, this method always
  // returns a valid object since Unknown types cover all cases not covered by expr or named
  static StxStorageMemRegionTypePtr getInstance(SgNode* n); 
  
  // Returns the type of this object: expr, named, storage or all
  StxMemRegionType::regType getType() const { return StxMemRegionType::storage; }
  
  // Returns a unique ID that differentiates this object from others within its type.
  // Storage and All objects must have a single ID, while others depend on the expression and symbol they
  // are associated with
  void* getUID() const { return NULL; }
  
  // Returns true if this object is live at the given part and false otherwise
  bool isLiveMR(PartEdgePtr pedge) { return true; }
  
  std::string str(std::string indent); // pretty print for the object
};

// The single memory region that contains all regions in stack+globals+heap as well as all expression regions
class StxAllMemRegionType;
typedef boost::shared_ptr<StxAllMemRegionType> StxAllMemRegionTypePtr;
extern StxAllMemRegionTypePtr NULLStxAllMemRegionType;

class StxAllMemRegionType : public StxMemRegionType
{
  public:
  StxAllMemRegionType() {}
  
  // Returns a freshly-allocated All memory region.
  static StxAllMemRegionTypePtr getInstance(SgNode* n); 
  
  // Returns the type of this object: expr, named, storage or all
  StxMemRegionType::regType getType() const { return StxMemRegionType::all; }
  
  // Returns a unique ID that differentiates this object from others within its type.
  // Storage and All objects must have a single ID, while others depend on the expression and symbol they
  // are associated with
  void* getUID() const { return NULL; }
  
  // Returns true if this object is live at the given part and false otherwise
  bool isLiveMR(PartEdgePtr pedge) { return true; }
  
  std::string str(std::string indent); // pretty print for the object
};

}; //namespace fuse

#endif
