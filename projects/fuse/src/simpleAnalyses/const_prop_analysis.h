#pragma once

#include "compose.h"
namespace fuse
{
  class CPValueLattice;
  class CPValueObject;
};
#include "VariableStateTransfer.h"
#include "abstract_object_map.h"
#include <boost/enable_shared_from_this.hpp>
#include "concrete_values.h"

namespace fuse
{
/***************************************
 ***** ConstantPropagationAnalysis *****
 ***************************************/

class ConstantPropagationAnalysis;

// This is a forward dataflow analysis that implements a simple abstraction of values
// that consists of the universal set, a single constant value and an empty set. It
// maintains a map of memory locations to these value abstractions.

class CPValueLattice;
typedef boost::shared_ptr<CPValueLattice> CPValueLatticePtr;
extern CPValueLatticePtr NULLCPValueLattice;

class CPValueObject;
typedef boost::shared_ptr<CPValueObject> CPValueObjectPtr;
extern CPValueObjectPtr NULLCPValueObject;

// CPValueLattice are used to identify both raw values and the locations of class fields within
// a class and the result of various arithmetic operatons among them. There may be arbitrary
// amounts of padding between fields within an object, meaning that we can only determine the order
// of their offsets within the object but not the actual concrete values of these offsets.
// Our representation of constants of either type is based on the Concrete ValueKind class from concrete_values.h
class CPValueLattice : public FiniteLattice, public boost::enable_shared_from_this<CPValueLattice> {
  ConcreteValueKindPtr kind;

  friend class CPValueObject;

  public:

  // Do we need a default constructor?
  CPValueLattice(PartEdgePtr pedge);
  CPValueLattice(ConcreteValueKindPtr kind, PartEdgePtr pedge);
  CPValueLattice(const CPValueLattice & X);

  // Access functions.
  ConcreteValueKindPtr getKind() const;
  // Sets this object's kind to the given kind, returning true if this causes the CPValueObject to change
  bool setKind(ConcreteValueKindPtr kind);

  void initialize();

  // Returns a copy of this lattice
  Lattice* copy() const;

  // Returns a shared pointer to a newly-allocated copy of this CPValueLatice
  CPValueLatticePtr copyCPLat() const;

  // overwrites the state of "this" Lattice with "that" Lattice
  void copy(Lattice* that);

  bool operator==(Lattice* that) /*const*/;

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdate(Lattice* that);
  bool meetUpdate(CPValueLattice* that);

  // Computes the intersection of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool intersectUpdate(Lattice* that);
  bool intersectUpdate(CPValueLattice* that);

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

  // pretty print for the object
  std::string str(std::string indent="") const;

  // Applies the given unary or binary operation to this and the given ConcreteValueKind
  // Returns:
  //    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated ConcreteValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent CPValueObject's list, return that.
  CPValueLatticePtr op(SgUnaryOp* op);
  CPValueLatticePtr op(SgBinaryOp* op, CPValueLatticePtr that);

  // Returns a freshly-allocated CPValueObject that communicates the information from this
  // Lattice to other analyses
  CPValueObjectPtr createValueObject();
}; // class CPValueLattice


// CPValueObjects are ValueObjects that are defined by CPValueLattices
class CPValueObject : public ValueObject {
  public:
  // The lattice that grounds the definition of this ValueObject.
  CPValueLatticePtr ground;

  CPValueObject(CPValueLatticePtr ground);

  // Wrapper for shared_from_this that returns an instance of this class rather than its parent
  CPValueObjectPtr shared_from_this() { return boost::static_pointer_cast<CPValueObject>(ValueObject::shared_from_this()); }

  bool mayEqualAO(ValueObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(ValueObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetAO(ValueObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetAO(ValueObjectPtr o, PartEdgePtr pedge);

  // Computes the meet of this and that and returns the resulting kind
  bool meetUpdateAO(ValueObjectPtr that, PartEdgePtr pedge);

  // Computes the intersect of this and that and returns the resulting kind
  bool intersectUpdateAO(ValueObjectPtr that, PartEdgePtr pedge);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Allocates a copy of this object and returns a pointer to it
  ValueObjectPtr copyAOType() const;

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const { return true; }

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const hierKeyPtr& getHierKey() const;

  // pretty print for the object
  std::string str(std::string indent="") const;
  std::string strp(PartEdgePtr pedge, std::string indent="") const;
}; // class CPValueObject

class CPMemLocObject;
typedef boost::shared_ptr<CPMemLocObject> CPMemLocObjectPtr;

class CPMemLocObject: public virtual MemLocObject/*, public FiniteLattice*/
{
  ConstantPropagationAnalysis* analysis;
  // Records whether this object is full or empty
  bool isCPFull;
  bool isCPEmpty;

  public:
  CPMemLocObject(bool isCPFull, bool isCPEmpty, SgNode* base, PartEdgePtr pedge, ConstantPropagationAnalysis* analysis) :
    MemLocObject(base),
    analysis(analysis),
    isCPFull(isCPFull), isCPEmpty(isCPEmpty)
  { }

  CPMemLocObject(MemRegionObjectPtr region, CPValueObjectPtr index, SgNode* base, PartEdgePtr pedge, ConstantPropagationAnalysis* analysis) :
    MemLocObject(region, index, base),
    analysis(analysis),
    isCPFull(false), isCPEmpty(false)
  { }

  CPMemLocObject(const CPMemLocObject& that) :
    MemLocObject(that),
    analysis(that.analysis),
    isCPFull(false), isCPEmpty(false)
  { }

  CPValueObjectPtr getCPIndex() const {
    return boost::dynamic_pointer_cast<CPValueObject>(getIndex());
  }

  // returns a copy of this lattice
  //Lattice* copy() const;

  // Initializes this Lattice to its default state, if it is not already initialized
  //void initialize() {}


  //bool operator==(Lattice*);

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
  //    guaranteed that the keys are in scope at fromPEdge while the values are in scope at the edge returned
  //    by getPartEdge().
  //Lattice* remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge);

  // Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously
  //    maintained in this lattice about them.
  // Returns true if the Lattice state is modified and false otherwise.
  //bool replaceML(Lattice* newL) { return false; }

  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  bool mayEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  bool equalSetAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  bool subSetAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns true if this object is live at the given part and false otherwise
  bool isLiveAO(PartEdgePtr pedge);

  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  bool setToFull();
  // Set this Lattice object to represent the of no execution prefixes (empty set).
  // Return true if this causes the object to change and false otherwise.
  bool setToEmpty();

  // Set all the value information that this Lattice object associates with this MemLocObjectPtr to full.
  // Return true if this causes the object to change and false otherwise.
  //bool setMLValueToFull(MemLocObjectPtr ml) { return false; }
  /*
  // Returns whether this lattice denotes the set of all possible execution prefixes.
  bool isFull();
  // Returns whether this lattice denotes the empty set.
  bool isEmpty();*/

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge);

/*  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  // The part of this object is to be used for AbstractObject comparisons.
  bool meetUpdate(Lattice* that);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdate(CPMemLocObject* that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);*/

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool intersectUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge);
/*
  // Computes intersection meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  // The part of this object is to be used for AbstractObject comparisons.
  bool intersectUpdate(Lattice* that);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool intersectUpdate(CPMemLocObject* that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);*/

  // Allocates a copy of this object and returns a pointer to it
  MemLocObjectPtr copyAOType() const;

  // Allocates a copy of this object and returns a regular pointer to it
  //MemLocObject* copyAOTypePtr() const;

  std::string str(std::string indent="") const; // pretty print for the object
}; // CPMemLocObject

class CPMemLocObjectNodeFact: public NodeFact
{
  public:
  CPMemLocObjectPtr ml;
  CPMemLocObjectNodeFact(CPMemLocObjectPtr ml): ml(ml) {}

  // returns a copy of this node fact
  NodeFact* copy() const { return new CPMemLocObjectNodeFact(ml); }
  std::string str(std::string indent="") const { return ml->str(); }
};

class ConstantPropagationAnalysis : virtual public FWDataflow
{
  protected:
  //static std::map<varID, Lattice*> constVars;
  //AbstractObjectMap constVars;

  public:
  ConstantPropagationAnalysis(bool useSSA);

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy() { return boost::make_shared<ConstantPropagationAnalysis>(useSSA); }

  // Creates a basic CPMemLocObject for the given SgNode. This object does not take into
  // account any constant propagation and will be used as a seed from which to propagate
  // more precise information.
  CPMemLocObjectPtr createBasicCPML(SgNode* sgn, PartEdgePtr pedge);

  // Initializes the state of analysis lattices at the given function, part and edge into our out of the part
  // by setting initLattices to refer to freshly-allocated Lattice objects.
  void genInitLattice(const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                      std::vector<Lattice*>& initLattices);

  bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

  boost::shared_ptr<DFTransferVisitor> getTransferVisitor(AnalysisParts& parts, CFGNode cn,
                                              NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

  boost::shared_ptr<ValueObject> Expr2Val(SgNode* n, PartEdgePtr pedge);
  bool implementsExpr2Val() { return true; }
  implTightness Expr2ValTightness() { return ComposedAnalysis::tight; }

  boost::shared_ptr<MemLocObject> Expr2MemLoc(SgNode* n, PartEdgePtr pedge);
  bool implementsExpr2MemLoc() { return false; }
  implTightness Expr2MemLocTightness() { return ComposedAnalysis::loose; }

  // pretty print for the object
  std::string str(std::string indent="") const
  { return "ConstPropAnalysis"; }

  friend class ConstantPropagationAnalysisTransfer;
}; // class ConstantPropagationAnalysis

class ConstantPropagationAnalysisTransfer : public VariableStateTransfer<CPValueLattice, ConstantPropagationAnalysis>
{
  private:

  // Transfer function for logical short-circuit operations: && and ||
  //void transferShortCircuitLogical(SgBinaryOp *sgn);

  public:
  //  void visit(SgNode *);
  // Values
  void visit(SgVarRefExp *vref);
  void visit(SgDotExp *dot);
  void visit(SgPntrArrRefExp *paRef);
  void visit(SgBinaryOp *sgn);

  // Unary ops that update the operand
  void visit(SgMinusMinusOp *sgn);
  void visit(SgPlusPlusOp *sgn);
  // Unary ops that do not update the operand
  // void visit(SgUnaryOp *sgn);
  void visit(SgCastExp* sgn);
  void visit(SgMinusOp* sgn);
  void visit(SgNotOp* sgn);

  void visit(SgValueExp *val);
  void visit(SgConditionalExp* sgn);

  ConstantPropagationAnalysisTransfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                                      std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                                      Composer* composer, ConstantPropagationAnalysis* analysis);
};

}; //namespace fuse

