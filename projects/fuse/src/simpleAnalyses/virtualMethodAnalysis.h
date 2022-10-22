#ifndef _VirtualMethodAnalysis_H
#define _VirtualMethodAnalysis_H

/*
 * Virtual Method Analysis
 * AbstractObjectMap maps each memory location that has a ClassType to its class
 * hierarchy information as computed by rose/src/midend/CallGraphAnalysis/ClassHierarchyGraph.*.
 * This is used to implement an ATS that wraps the input ATS but is missing edges
 * along virtual method calls that are inconsistent with the type of the object
 * on which they're called.
 *
 * Author: bronevetsky@llnl.gov
 */

#include "compose.h"
#include "abstract_object_map.h"
#include "midend/programAnalysis/CallGraphAnalysis/ClassHierarchyGraph.h"
#include "VariableStateTransfer.h"
namespace fuse {
class VirtualMethodAnalysis;

/********************************
 ***** ClassInheritanceTree *****
 ********************************/

// A tree that records the inheritance hierarchy of a given SgClassType, where the
// nodes are SgClassDefinitions and edges are inheritance relationships from
// each parent class to the derived class. This tree is focused on the inheritance
// information of a single SgClassDefinition or the minimal prefix common to
// multiple SgClassDefinitions. This is unlike the ClassHierarchyGraph, which
// contains information on all SgClassDefinitions.
class ClassInheritanceTree: public FiniteLattice {
  // Points to the object that stores the entire class hierarchy of all types
  static ClassHierarchyWrapper* classHierarchy;

  // Initializes the ClassHierarchyGraph, if needed
  static void initCHG();

  // A node in the inheritance tree. Each node encapsulates a single SgClassDefinition
  // and contains pointers to multiple parent nodes and a single child node.
  // Nodes are immutable. Once created and initialized they must not be modified.
  // This makes it possible for multiple trees to share Nodes, which is important
  // for memory use since many trees will be very similar to each other.
  // Note that the copy constructor copies just the bases and leaves lists
  // and not the Nodes inside the tree.
  class Node;
  typedef CompSharedPtr<Node> NodePtr;
  class Node {
    public:
    SgClassDefinition* def;
    std::set<NodePtr> parents;
    NodePtr child;

    bool operator==(NodePtr that) {
      dbg << "Node::== def="<<def<<"="<<SgNode2Str(def)<<", that->def="<<that->def<<"="<<SgNode2Str(that->def)<<", equal="<<(def == that->def)<<endl;
      return def == that->def; }
    bool operator!=(NodePtr that) {
      //dbg << "Node::== returning "<<(!(*this == that))<<endl;
      return !(*this == that); }
    bool operator< (NodePtr that) { return def < that->def; }
    bool operator<=(NodePtr that) { return (*this == that) || (*this < that); }
    bool operator> (NodePtr that) { return !(*this == that) && !(*this < that); }
    bool operator>=(NodePtr that) { return (*this == that) || !(*this < that); }

    Node(SgClassDefinition* def): def(def) {}

    // Copy constructor that creates a deep copy

    std::string str(std::string indent="") const {
      std::ostringstream s;
      s << "[ClassInheritanceTree::Node:(this="<<this<<", #parents="<<parents.size()<<", child="<<child.get()<<"="<<(child?child->def:NULL)<<endl;
      s << "    def="<<SgNode2Str(def)<<"]"<<endl;
      return s.str();
    }
  }; // class Node


  // The base classes from which a given set of SgClassDefinitions are
  // derived. It is empty if there is nothing common among these SgClassDefinitions.
  std::list<NodePtr> bases;

  // The leaves of the tree, which denote the most derived classes that are
  // common to all SgClassDefinitions in a given set.
  std::list<NodePtr> leaves;

  // Maps each SgClassDefinition to its nodes in the tree. A given declaration may
  // appear multiple times in the tree since a given set of SgClassDefinitions
  // may inherit from it via multiple paths.
  //std::map<SgClassDefinition*, std::set<NodePtr> > def2Nodes;

  // Records whether this object encodes one of the following hierarchy types:
  typedef enum {full, // The common hierarchy among all possible SgClassDefinitions,
                      // which is the empty tree
                tree, // A non-empty tree, which encodes some constraints on the classes
                      // from which some set of SgClassDefinitions derive
                empty // The hierarchy of no SgClassDefinitions, which has no information
               }
             stateT;
  stateT state;

  // Records whether the classes denoted by the tree include leaves as well as the classes
  // that derive from them (true) or just the derived classes (false)
  bool inclusive;

  public:

  ClassInheritanceTree(PartEdgePtr latPEdge);

  // Creates a tree that captures the inheritance of the given SgClassDefinition
  ClassInheritanceTree(SgClassDefinition* classDecl, PartEdgePtr latPEdge);

  ClassInheritanceTree(const ClassInheritanceTree& that);

  ClassInheritanceTree& operator=(const ClassInheritanceTree& that);

  // Copies the state of that tree over this tree.
  void copyFrom(const ClassInheritanceTree& that);

  // initializes this Lattice to its default state, if it is not already initialized
  void initialize() { }

  // returns a copy of this lattice
  Lattice* copy() const;

  std::string state2Str(stateT state)
  { return (state==full? "full": (state==tree? "tree": "empty")); }

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  // The part of this object is to be used for AbstractObject comparisons.
  // meetUpdate finds the common tree prefix among the two treeS
  bool meetUpdate(Lattice* that);

  // Computes the intersection of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  // The part of this object is to be used for AbstractObject comparisons.
  // intersectUpdate finds the common tree prefix among the two treeS
  bool intersectUpdate(Lattice* that);

  // Returns the set of functions the given SgFunctionCallExp may refer to given
  // the type constraints encoded in this tree
  std::set<SgFunctionDeclaration*> getCalleeDefs(SgFunctionCallExp* call);

  bool operator==(Lattice* that_arg);

  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  bool setToFull();

  // Set this Lattice object to represent the of no execution prefixes (empty set).
  // Return true if this causes the object to change and false otherwise.
  bool setToEmpty();

  // Set all the value information that this Lattice object associates with this MemLocObjectPtr to full.
  // Return true if this causes the object to change and false otherwise.
  bool setMLValueToFull(MemLocObjectPtr ml);

  // Returns whether this lattice denotes the set of all possible execution prefixes.
  bool isFull();

  // Returns whether this lattice denotes the empty set.
  bool isEmpty();

  std::string str(std::string indent="") const;
  std::string strp(PartEdgePtr pedge, std::string indent) const { return str(indent); }
}; // ClassInheritanceTree
typedef boost::shared_ptr<ClassInheritanceTree> ClassInheritanceTreePtr;

// Records the number of entry/exit points that have been discovered into/out of each function
// so far. This makes it possible to track any newly-discovered entry points and ensure that
// ATS edges are generated for the corresponding exit points.
class MethodEntryExitMap: public FiniteLattice {
  // Maps each function to the set of PartEdges that denote its entry points
  // SgFunctionCallExp with index 2 -> SgParameterList
  std::map<Function, std::set<PartEdgePtr> > entryPoints;

  // Records whether this map denotes the set of all possible entry points
  // into each function.
  bool fullEntryPoints;

  public:

  MethodEntryExitMap(PartEdgePtr latPEdge);

  MethodEntryExitMap(const MethodEntryExitMap& that);

  MethodEntryExitMap& operator=(const MethodEntryExitMap& that);

  // initializes this Lattice to its default state, if it is not already initialized
  void initialize() { }

  // returns a copy of this lattice
  Lattice* copy() const;

  // Adds a new entry point for the given function. Returns true if this causes this
  // object to change and false otherwise
  bool add(const Function& func, PartEdgePtr entry);


  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  // The part of this object is to be used for AbstractObject comparisons.
  // meetUpdate finds the common tree prefix among the two treeS
  bool meetUpdate(Lattice* that);

  // Computes the intersection of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  // The part of this object is to be used for AbstractObject comparisons.
  // intersectUpdate finds the common tree prefix among the two treeS
  bool intersectUpdate(Lattice* that);

  bool operator==(Lattice* that_arg);

  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  bool setToFull();

  // Set this Lattice object to represent the of no execution prefixes (empty set).
  // Return true if this causes the object to change and false otherwise.
  bool setToEmpty();

  // Set all the value information that this Lattice object associates with this MemLocObjectPtr to full.
  // Return true if this causes the object to change and false otherwise.
  bool setMLValueToFull(MemLocObjectPtr ml);

  // Returns whether this lattice denotes the set of all possible execution prefixes.
  bool isFull();

  // Returns whether this lattice denotes the empty set.
  bool isEmpty();

  std::string str(std::string indent="") const;
  std::string strp(PartEdgePtr pedge, std::string indent) const { return str(indent); }
}; // MethodEntryExitMap
typedef boost::shared_ptr<MethodEntryExitMap> MethodEntryExitMapPtr;


class VirtualMethodPart;
typedef CompSharedPtr<VirtualMethodPart> VirtualMethodPartPtr;
class VirtualMethodPartEdge;
typedef CompSharedPtr<VirtualMethodPartEdge> VirtualMethodPartEdgePtr;

class VirtualMethodPart : public Part
{
  private:

  friend class VirtualMethodPartEdge;
  friend class VirtualMethodAnalysisTransfer;

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
  VirtualMethodPartEdgePtr cache_inEdgeFromAny;
  bool cacheInitialized_outEdgeToAny;
  VirtualMethodPartEdgePtr cache_outEdgeToAny;
  std::map<VirtualMethodPart*, bool> cache_equal;
  std::map<VirtualMethodPart*, bool> cache_less;

  protected:
  VirtualMethodPart(PartPtr base, ComposedAnalysis* analysis);
  VirtualMethodPart(const VirtualMethodPart& that);

  public:
  // Parts must be created via static construction methods to make it possible to separately
  // initialize them. This is needed to allow Parts to register themselves with global directories,
  // a process that requires the creation of a shared pointer to themselves.
  static VirtualMethodPartPtr create(PartPtr base, ComposedAnalysis* analysis) {
    VirtualMethodPartPtr newPart(boost::shared_ptr<VirtualMethodPart>(new VirtualMethodPart(base, analysis)));
    newPart->init();
    return newPart;
  }
  static VirtualMethodPartPtr create(const VirtualMethodPart& that) {
    VirtualMethodPartPtr newPart(boost::shared_ptr<VirtualMethodPart>(new VirtualMethodPart(that)));
    newPart->init();
    return newPart;
  }

  VirtualMethodPart* copy() const { return new VirtualMethodPart(*this); }

  private:
  // Returns a shared pointer to this of type VirtualMethodPartPtr
  VirtualMethodPartPtr get_shared_this();

  // -------------------------------------------
  // Functions that need to be defined for Parts
  // -------------------------------------------

  protected:
  // Returns whether the given function call according to the available information
  // about object types
  bool isPossibleFunctionCall(const Function& calleeFunc, SgFunctionCallExp* call,
                              AbstractObjectMap* aom/*, PartEdgePtr NodeStatePartEdge*/);

  // Given baseEdges, a list of edges from the server analysis' ATS, set cache_Edges to contain the edges in
  // the VMAnalysis' ATS that wrap them
  void wrapEdges(std::list<PartEdgePtr>& cache_Edges, const AnalysisPartEdgeLists& baseEdges);

  public:
  std::list<PartEdgePtr> outEdges();
  std::list<PartEdgePtr> inEdges();
  std::set<CFGNode> CFGNodes() const;

  // If this Part corresponds to a function call/return, returns the set of Parts that contain
  // its corresponding return/call, respectively.
  std::set<PartPtr> matchingCallParts() const;

  // If this Part corresponds to a function entry/exit, returns the set of Parts that contain
  // its corresponding exit/entry, respectively.
  std::set<PartPtr> matchingEntryExitParts() const;

  // Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
  PartEdgePtr inEdgeFromAny();

  // Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
  PartEdgePtr outEdgeToAny();

  bool equal(const PartPtr& o) const;
  bool less(const PartPtr& o)  const;

  // Pretty print for the object
  std::string str(std::string indent="") const;
}; // class VirtualMethodPart

class VirtualMethodPartEdge : public PartEdge {
  // The part that this object is wrapping with live/dead status
  VirtualMethodPartPtr src;
  VirtualMethodPartPtr tgt;

  std::map<SgNode*, std::map<SgNode*, std::list<PartEdgePtr> > > cache_getOperandPartEdge;
  std::map<CFGNode, boost::shared_ptr<SgValueExp> > cache_getPredicateValue;
  bool cacheInitialized_getPredicateValue;

  // For edges from parts that contain CFGNodes that correspond to conditionals (if, switch, while test, etc.)
  // records a mapping from these CFGNodes to the value outcome that leads control along this edge.
  std::map<CFGNode, boost::shared_ptr<SgValueExp> > predVals;

  friend class VirtualMethodPart;
  friend class VirtualMethodTransfer;

  protected:
  // Constructor to be used when traversing the part graph created by the VirtualMethodAnalysis, after
  // all the VirtualMethodPartEdges have been constructed and stored in NodeStates.
  VirtualMethodPartEdge(PartEdgePtr baseEdge, ComposedAnalysis* analysis);

  VirtualMethodPartEdge(const VirtualMethodPartEdge& that);

  public:
  static VirtualMethodPartEdgePtr create(PartEdgePtr base, ComposedAnalysis* analysis) {
    VirtualMethodPartEdgePtr newPartEdge(boost::shared_ptr<VirtualMethodPartEdge>(new VirtualMethodPartEdge(base, analysis)));
    newPartEdge->init();
    return newPartEdge;
  }
  static VirtualMethodPartEdgePtr create(const VirtualMethodPartEdge& that) {
    VirtualMethodPartEdgePtr newPartEdge(boost::shared_ptr<VirtualMethodPartEdge>(new VirtualMethodPartEdge(that)));
    newPartEdge->init();
    return newPartEdge;
  }
  static VirtualMethodPartEdge* createRaw(PartEdgePtr base, ComposedAnalysis* analysis) {
    VirtualMethodPartEdge* newPartEdge = new VirtualMethodPartEdge(base, analysis);
    //newPartEdge->init();
    return newPartEdge;
  }
  static VirtualMethodPartEdge* createRaw(const VirtualMethodPartEdge& that) {
    VirtualMethodPartEdge* newPartEdge = new VirtualMethodPartEdge(that);
    //newPartEdge->init();
    return newPartEdge;
  }

  VirtualMethodPartEdge* copy() const { return new VirtualMethodPartEdge(*this); }
  private:
  // Returns a shared pointer to this of type VirtualMethodPartEdgePtr
  VirtualMethodPartEdgePtr get_shared_this();

  public:
  PartPtr source() const;
  PartPtr target() const;

  // Sets this PartEdge's parent
  void setInputPartEdge(PartEdgePtr parent);

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

  // Adds a mapping from a CFGNode to the outcome of its predicate
  void mapPred2Val(CFGNode n, boost::shared_ptr<SgValueExp> val);

  // Empties out the mapping of CFGNodes to the outcomes of their predicates
  void clearPred2Val();

  bool equal(const PartEdgePtr& o) const;
  bool less(const PartEdgePtr& o)  const;

  // Pretty print for the object
  std::string str(std::string indent="") const;
};

/*********************************
 ***** VirtualMethodAnalysis *****
 *********************************/

class VirtualMethodAnalysis : public virtual FWDataflow
{
  std::set<PartPtr> cache_GetStartAStates_Spec;
  bool cacheInitialized_GetStartAStates_Spec;
  std::set<PartPtr> cache_GetEndAStates_Spec;
  bool cacheInitialized_GetEndAStates_Spec;

  public:
  VirtualMethodAnalysis(bool trackBase2RefinedPartEdgeMapping=false);

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy() { return boost::make_shared<VirtualMethodAnalysis>(); }

  void genInitLattice(const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                      std::vector<Lattice*>& initLattices);

  bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) { assert(false); return false; }

  boost::shared_ptr<DFTransferVisitor>
  getTransferVisitor(AnalysisParts& parts, CFGNode cn, NodeState& state,
                     std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

  std::set<PartPtr> GetStartAStates_Spec();
  std::set<PartPtr> GetEndAStates_Spec();

  // Returns whether this analysis implements an Abstract Transition System graph via the methods
  // GetStartAStates_Spec() and GetEndAStates_Spec()
  bool implementsATSGraph() { return true; }

  std::string str(std::string indent) const
  { return "VirtualMethodAnalysis"; }

  friend class VirtualMethodAnalysisTransfer;
};

/*****************************************
 ***** VirtualMethodAnalysisTransfer *****
 *****************************************/

//! Transfer functions for the PointsTo analysis
class VirtualMethodAnalysisTransfer : public VariableStateTransfer<ClassInheritanceTree, VirtualMethodAnalysis>
{
  public:
  VirtualMethodAnalysisTransfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                                std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                                Composer* composer, VirtualMethodAnalysis* analysis);

  void visit(SgVarRefExp *vref);
  void visit(SgInitializedName *name);
};

} // namespace fuse
#endif
