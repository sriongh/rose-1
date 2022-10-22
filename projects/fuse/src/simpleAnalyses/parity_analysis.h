#ifndef CONSTANT_PROPAGATION_ANALYSIS_H
#define CONSTANT_PROPAGATION_ANALYSIS_H

#include "compose.h"
namespace fuse
{
    class ParityValueObject;
};
#include "VariableStateTransfer.h"
#include "abstract_object_map.h"
#include <boost/enable_shared_from_this.hpp>

namespace fuse
{
/**************************
 ***** ParityAnalysis *****
 **************************/

class ParityAnalysis;

// This is a forward dataflow analysis that implements a simple abstraction of values
// that consists of the universal set, a single constant value and an empty set. It
// maintains a map of memory locations to these value abstractions.

class ParityValueObject;
typedef boost::shared_ptr<ParityValueObject> ParityValueObjectPtr;
extern ParityValueObjectPtr NULLParityValueObject;

class CPValueKind;
typedef boost::shared_ptr<CPValueKind> CPValueKindPtr;
extern CPValueKindPtr NULLCPValueKind;

class CPUninitializedKind;
typedef boost::shared_ptr<CPUninitializedKind> CPUninitializedKindPtr;

class CPExactKind;
typedef boost::shared_ptr<CPExactKind> CPExactKindPtr;

class CPOffsetListKind;
typedef boost::shared_ptr<CPOffsetListKind> CPOffsetListKindPtr;

class CPUnknownKind;
typedef boost::shared_ptr<CPUnknownKind> CPUnknownKindPtr;

class CPValueKind: public printable, public boost::enable_shared_from_this<CPValueKind> {
  public:

  // The different kinds of ParityValueObjects
  typedef enum {uninitialized, // The value has not been initialized (analyses may set this value to anything they want)
                concrete,   // The exact value is known and can be described with an SgValueExp
                //rank,     // The exact value is not known but we know their relative order
                offsetList, // The value denotes the offset of some class/struct member from a pointer the member's
                            // host object. Since the compiler can add arbitrary padding, such values are lists
                            // of concrete values and members' ranks within their host objects.

          // !!! TODO
          serverImpl, // Thin wrapper for a server-implemented ValueObject
                unknown     // The exact value is not known
     } valueKind;
  valueKind kind;

  CPValueKind(valueKind kind): kind(kind)
  {}

  valueKind getKind() const { return kind; }

  CPUninitializedKindPtr asUninitializedKind()
  { return boost::static_pointer_cast<CPUninitializedKind>(shared_from_this()); }
  CPExactKindPtr asConcreteKind()
  { return boost::static_pointer_cast<CPExactKind>(shared_from_this()); }
  CPOffsetListKindPtr asOffsetListKind()
  { return boost::static_pointer_cast<CPOffsetListKind>(shared_from_this()); }
  CPUnknownKindPtr asUnknownKind()
  { return boost::static_pointer_cast<CPUnknownKind>(shared_from_this()); }

  // Applies the given unary or binary operation to this and the given CPValueKind
  // Returns:
  //    - if this CPValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated CPValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ParityValueObject's list, return that.
  virtual CPValueKindPtr op(SgUnaryOp* op)=0;
  virtual CPValueKindPtr op(SgBinaryOp* op, CPValueKindPtr that)=0;

  // Returns whether this and that CPValueKinds are may/must equal to each other
  virtual bool mayEqualV(CPValueKindPtr that)=0;
  virtual bool mustEqualV(CPValueKindPtr that)=0;

  // Returns whether the two CPValueKinds denote the same set of concrete values
  virtual bool equalSetV(CPValueKindPtr that)=0;

  // Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given CPValueKind
  virtual bool subSetV(CPValueKindPtr that)=0;

  // Computes the meet of this and that and returns the resulting kind
  virtual std::pair<bool, CPValueKindPtr> meetUpdateV(CPValueKindPtr that)=0;

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  virtual bool isConcrete()=0;
  // Returns the number of concrete values in this set
  virtual int concreteSetSize()=0;
  // Returns the type of the concrete value (if there is one)
  virtual SgType* getConcreteType()=0;
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  virtual std::set<boost::shared_ptr<SgValueExp> > getConcreteValue()=0;

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isFullV(PartEdgePtr pedge)=0;
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isEmptyV(PartEdgePtr pedge)=0;

  // Returns a copy of this CPExactKind
  virtual CPValueKindPtr copyV() const=0;
}; // CPValueKind

class ParityValueObject : public FiniteLattice, public ValueObject {
  //SgNode* n;

  // StxValueObjects are used to identify both raw values and the locations of class fields within
  // a class. Since there may be arbitrary amounts of padding between such fields, we can only determine
  // the order of their offsets within the class object, not the actual concrete values of these offsets.
  // Since the application may use these offsets to find additional memory regions (e.g. a.b.c, (&a.b)+1 or
  // array[1].a[2].b[3]), we represent such derived offsets as sequences of StxValueObjects, the sum of
  // which corresponds to the actual offset.
  // Thus,
  // a.b.c: region(a), rank(b in a) + rank(c in b)
  // (&a.b)+1: region(a), rank(b in a) + concrete(1)
  // array[1].a[2].b[3]: region(array), concrete(1*sizeof(elt of array)) + rank(a in array elt) +
  //                                    concrete(2*sizeof elt of array[1]) + rank(b in array[1] elt) +
  //                                    concrete(3*sizeof elt of array[1].a[2])
  // We maintain such sums of StxValueObjects as a linked list, with each object in the chain pointing
  // to the next one. However, not all combinations need to be represented explicitly:
  // concrete + concrete: concrete object that denotes the sum of the arguments
  // concrete + rank, rank + concrete: maintained as a list
  // rank + rank: unknown
  // * + unknown, unknown + *: a single unknown value object
/*  // pointer(memregion, *1) + *2, *2 + pointer(memregion, *1):
  //          resulting object is pointer(memregion, *1+*2), with the + following the above rules */

  CPValueKindPtr kind;

  public:

  // Do we need a default constructor?
  ParityValueObject(PartEdgePtr pedge);

  // This constructor builds a constant value lattice.
  //ParityValueObject(SgValueExp* val, PartEdgePtr pedge);

  ParityValueObject(CPValueKindPtr kind, PartEdgePtr pedge);

  // Do we need th copy constructor?
  ParityValueObject(const ParityValueObject & X);

  // Wrapper for shared_from_this that returns an instance of this class rather than its parent
  ParityValueObjectPtr shared_from_this() { return boost::static_pointer_cast<ParityValueObject>(ValueObject::shared_from_this()); }

  // Access functions.
  CPValueKindPtr getKind() const;
  // Sets this object's kind to the given kind, returning true if this causes the ParityValueObject to change
  bool setKind(CPValueKindPtr kind);

  void initialize();

  // returns a copy of this lattice
  Lattice* copy() const;

  // overwrites the state of "this" Lattice with "that" Lattice
  void copy(Lattice* that);

  bool operator==(Lattice* that) /*const*/;

  // computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdate(Lattice* that);
  bool meetUpdate(ParityValueObject* that);

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
  bool isFullLat();
  // Returns whether this lattice denotes the empty set.
  bool isEmptyLat();

  // pretty print for the object
  std::string str(std::string indent="");
  std::string strp(PartEdgePtr pedge, std::string indent="");

  // Applies the given unary or binary operation to this and the given CPValueKind
  // Returns:
  //    - if this CPValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated CPValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ParityValueObject's list, return that.
  ParityValueObjectPtr op(SgUnaryOp* op);
  ParityValueObjectPtr op(SgBinaryOp* op, ParityValueObjectPtr that);

  bool mayEqualV(ValueObjectPtr o, PartEdgePtr pedge);
  bool mustEqualV(ValueObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetV(ValueObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetV(ValueObjectPtr o, PartEdgePtr pedge);

  // Computes the meet of this and that and returns the resulting kind
  bool meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullV(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyV(PartEdgePtr pedge);

  // Allocates a copy of this object and returns a pointer to it
  ValueObjectPtr copyV() const;

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  set<boost::shared_ptr<SgValueExp> > getConcreteValue();
}; // class ParityValueObject

class CPUninitializedKind : public CPValueKind {
  public:
  CPUninitializedKind() : CPValueKind(uninitialized)
  {}

  // Applies the given unary or binary operation to this and the given CPValueKind
  // Returns:
  //    - if this CPValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated CPValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ParityValueObject's list, return that.
  CPValueKindPtr op(SgUnaryOp* op);
  CPValueKindPtr op(SgBinaryOp* op, CPValueKindPtr that);

  // Returns whether this and that CPValueKinds are may/must equal to each other
  bool mayEqualV(CPValueKindPtr that);
  bool mustEqualV(CPValueKindPtr that);

  // Returns whether the two CPValueKinds denote the same set of concrete values
  bool equalSetV(CPValueKindPtr that);

  // Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given CPValueKind
  bool subSetV(CPValueKindPtr that);

  // Computes the meet of this and that and returns the resulting kind
  std::pair<bool, CPValueKindPtr> meetUpdateV(CPValueKindPtr that);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullV(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyV(PartEdgePtr pedge);

  // Returns a copy of this CPUninitializedKind
  CPValueKindPtr copyV() const { return boost::make_shared<CPUninitializedKind>(); }

  std::string str(std::string indent="");
}; // CPUninitializedKind

class CPExactKind : public CPValueKind {
  // The expression that denotes the known value of this object
  boost::shared_ptr<SgValueExp> exp;

  public:
  CPExactKind(boost::shared_ptr<SgValueExp> exp) : CPValueKind(concrete), exp(exp)
  {}

  boost::shared_ptr<SgValueExp> getVal() const { return exp; }

  // Applies the given operation functor to the expression in this ConcreteKind and returns the resulting CPKind
  template<class DoOpType>
  CPValueKindPtr doUnaryOp(DoOpType& doOp);

  // Applies the given operation functor to the expression in this ConcreteKind, whoch is assumed to be an integral type
  // and returns the resulting CPKind
  template<class DoOpType>
  CPValueKindPtr doUnaryIntegralOp(DoOpType& doOp);

  // Applies the given operation functor to the expression in this ConcreteKind, which is assumed to a floating point type
  // and returns the resulting CPKind
  template<class DoOpType>
  CPValueKindPtr doUnaryFloatOp(DoOpType& doOp);

  template<class DoOpType, class DoOpRetType>
  DoOpRetType bindDoOpArgs1(DoOpType doOp);

  template<class DoOpType, class DoOpRetType>
  DoOpRetType bindDoOpArgs2(DoOpType doOp);

  // Creates a CPExactKind from the given value. This function is overloaded with different argument types
  // and for each type it creates a CPExactKind with a different SgValueExp.
  CPValueKindPtr createCPValueKindFromVal(bool val);
  CPValueKindPtr createCPValueKindFromVal(char val);
  CPValueKindPtr createCPValueKindFromVal(short val);
  CPValueKindPtr createCPValueKindFromVal(int val);
  CPValueKindPtr createCPValueKindFromVal(long val);
  CPValueKindPtr createCPValueKindFromVal(long long val);
  CPValueKindPtr createCPValueKindFromVal(unsigned char val);
  CPValueKindPtr createCPValueKindFromVal(unsigned short val);
  CPValueKindPtr createCPValueKindFromVal(unsigned int val);
  CPValueKindPtr createCPValueKindFromVal(unsigned long val);
  CPValueKindPtr createCPValueKindFromVal(unsigned long long val);
  CPValueKindPtr createCPValueKindFromVal(wchar_t val);
  CPValueKindPtr createCPValueKindFromVal(float val);
  CPValueKindPtr createCPValueKindFromVal(double val);
  CPValueKindPtr createCPValueKindFromVal(long double val);

  // Applies the given binary operation functor to the expression in this and that ConcreteKinds, assuming they're
  // both integral types and returns the resulting CPKind
  template<class DoOpType>
  CPValueKindPtr doBinaryOpIntInt(DoOpType& doOp, CPExactKindPtr that);

  // Applies the given binary operation functor to the expression in this and that ConcreteKinds, assuming at least one
  // is not an integral type and returns the resulting CPKind
  template<class DoOpType>
  CPValueKindPtr doBinaryOp(DoOpType& doOp, CPExactKindPtr that);

  // Applies the given unary or binary operation to this and the given CPValueKind
  // Returns:
  //    - if this CPValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated CPValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ParityValueObject's list, return that.
  CPValueKindPtr op(SgUnaryOp* op);
  CPValueKindPtr op(SgBinaryOp* op, CPValueKindPtr that);

  // Returns whether this and that CPValueKinds are may/must equal to each other
  bool mayEqualV(CPValueKindPtr that);
  bool mustEqualV(CPValueKindPtr that);

  // Returns whether the two CPValueKinds denote the same set of concrete values
  bool equalSetV(CPValueKindPtr that);

  // Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given CPValueKind
  bool subSetV(CPValueKindPtr that);

  // Computes the meet of this and that and returns the resulting kind
  std::pair<bool, CPValueKindPtr> meetUpdateV(CPValueKindPtr that);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullV(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyV(PartEdgePtr pedge);

  // Returns a copy of this CPExactKind
  CPValueKindPtr copyV() const { return boost::make_shared<CPExactKind>(getVal()); }

  std::string str(std::string indent="");
}; // CPExactKind

class CPOffsetListKind : public CPValueKind {
  class intWrap {
    public:
    typedef enum {rankT, offsetT} type;

    private:
    type t;

    long long v;
    public:
    intWrap(long long v, type t): t(t), v(v){}

    bool operator==(const intWrap& that) const { return t==that.t && v==that.v; }
    bool operator!=(const intWrap& that) const { return !(*this == that); }

    long long operator=(long long v) { return (this->v = v);}

    long long get() const { return v; }
    void set(unsigned long long v) { this->v = v; }

    type getType() const { return t; }
  };

  static SgTypeLongLong* type;

  public:

  // Represents the rank of a field within a class/struct (full offset is not known)
  class rank : public intWrap {
    public:
    rank(long long v): intWrap(v, intWrap::rankT) {}
  };

  // Represents a known offset from the address of some memory region or an internal class field
  class offset : public intWrap {
    public:
    offset(long long v): intWrap(v, intWrap::offsetT) {}
  };

  std::list<intWrap> offsetL;

  public:
  CPOffsetListKind(const rank& r) : CPValueKind(offsetList) {
    if(type==NULL) type = SageBuilder::buildLongLongType();
    offsetL.push_back(r);
  }

  CPOffsetListKind(const offset& o) : CPValueKind(offsetList) {
    if(type==NULL) type = SageBuilder::buildLongLongType();
    offsetL.push_back(o);
  }

  CPOffsetListKind(const std::list<intWrap>& offsetL) : CPValueKind(offsetList), offsetL(offsetL) {
    if(type==NULL) type = SageBuilder::buildLongLongType();
  }

  // Applies the given unary or binary operation to this and the given CPValueKind
  // Returns:
  //    - if this CPValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated CPValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ParityValueObject's list, return that.
  CPValueKindPtr op(SgUnaryOp* op);
  CPValueKindPtr op(SgBinaryOp* op, CPValueKindPtr that);

  // Returns whether this and that CPValueKinds are may/must equal to each other
  bool mayEqualV(CPValueKindPtr that);
  bool mustEqualV(CPValueKindPtr that);

  // Returns whether the two CPValueKinds denote the same set of concrete values
  bool equalSetV(CPValueKindPtr that);

  // Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given CPValueKind
  bool subSetV(CPValueKindPtr that);

  // Computes the meet of this and that and returns the resulting kind
  std::pair<bool, CPValueKindPtr> meetUpdateV(CPValueKindPtr that);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullV(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyV(PartEdgePtr pedge);

  // Returns a copy of this CPOffsetListKind
  CPValueKindPtr copyV() const { return boost::make_shared<CPOffsetListKind>(offsetL); }

  std::string str(std::string indent="");
}; // CPOffsetListKind

class CPUnknownKind : public CPValueKind {
  public:
  CPUnknownKind() : CPValueKind(unknown)
  {}

  // Applies the given unary or binary operation to this and the given CPValueKind
  // Returns:
  //    - if this CPValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated CPValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ParityValueObject's list, return that.
  CPValueKindPtr op(SgUnaryOp* op);
  CPValueKindPtr op(SgBinaryOp* op, CPValueKindPtr that);

  // Returns whether this and that CPValueKinds are may/must equal to each other
  bool mayEqualV(CPValueKindPtr that);
  bool mustEqualV(CPValueKindPtr that);

  // Returns whether the two CPValueKinds denote the same set of concrete values
  bool equalSetV(CPValueKindPtr that);

  // Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given CPValueKind
  bool subSetV(CPValueKindPtr that);

  // Computes the meet of this and that and returns the resulting kind
  std::pair<bool, CPValueKindPtr> meetUpdateV(CPValueKindPtr that);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullV(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyV(PartEdgePtr pedge);

  // Returns a copy of this CPUnknownKind
  CPValueKindPtr copyV() const { return boost::make_shared<CPUnknownKind>(); }

  std::string str(std::string indent="");
}; // CPUnknownKind

class CPMemLocObject;
typedef boost::shared_ptr<CPMemLocObject> CPMemLocObjectPtr;

class CPMemLocObject: public MemLocObject, public FiniteLattice
{
  ParityAnalysis* analysis;
  // Records whether this object is full or empty
  bool isCPFull;
  bool isCPEmpty;

  public:
  CPMemLocObject(bool isCPFull, bool isCPEmpty, SgNode* base, PartEdgePtr pedge, ParityAnalysis* analysis) :
    Lattice(pedge),
    MemLocObject(base),
    FiniteLattice(pedge),
    analysis(analysis),
    isCPFull(isCPFull), isCPEmpty(isCPEmpty)
  { }

  CPMemLocObject(MemRegionObjectPtr region, ParityValueObjectPtr index, SgNode* base, PartEdgePtr pedge, ParityAnalysis* analysis) :
    Lattice(pedge),
    MemLocObject(region, index, base),
    FiniteLattice(pedge),
    analysis(analysis),
    isCPFull(false), isCPEmpty(false)
  { }

  CPMemLocObject(const CPMemLocObject& that) :
    Lattice(that.getPartEdge()),
    MemLocObject(that),
    FiniteLattice(that.getPartEdge()),
    analysis(that.analysis),
    isCPFull(false), isCPEmpty(false)
  { }

  ParityValueObjectPtr getCPIndex() const {
    return boost::dynamic_pointer_cast<ParityValueObject>(getIndex());
  }

  // returns a copy of this lattice
  Lattice* copy() const;

  // Initializes this Lattice to its default state, if it is not already initialized
  void initialize() {}


  bool operator==(Lattice*);

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
  Lattice* remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge);

  // Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously
  //    maintained in this lattice about them.
  // Returns true if the Lattice state is modified and false otherwise.
  bool replaceML(Lattice* newL) { return false; }

  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  bool mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge);
  bool mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  bool equalSetML(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  bool subSetML(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns true if this object is live at the given part and false otherwise
  bool isLiveML(PartEdgePtr pedge);

  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  bool setToFull();
  // Set this Lattice object to represent the of no execution prefixes (empty set).
  // Return true if this causes the object to change and false otherwise.
  bool setToEmpty();

  // Set all the value information that this Lattice object associates with this MemLocObjectPtr to full.
  // Return true if this causes the object to change and false otherwise.
  bool setMLValueToFull(MemLocObjectPtr ml) { return false; }

  // Returns whether this lattice denotes the set of all possible execution prefixes.
  bool isFullLat();
  // Returns whether this lattice denotes the empty set.
  bool isEmptyLat();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullML(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyML(PartEdgePtr pedge);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  // The part of this object is to be used for AbstractObject comparisons.
  bool meetUpdate(Lattice* that);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  // The part of this object is to be used for AbstractObject comparisons.
  bool meetUpdate(CPMemLocObject* that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Allocates a copy of this object and returns a pointer to it
  MemLocObjectPtr copyML() const;

  // Allocates a copy of this object and returns a regular pointer to it
  MemLocObject* copyMLPtr() const;

  std::string str(std::string indent=""); // pretty print for the object
}; // CPMemLocObject

class CPMemLocObjectNodeFact: public NodeFact
{
  public:
  CPMemLocObjectPtr ml;
  CPMemLocObjectNodeFact(CPMemLocObjectPtr ml): ml(ml) {}

  // returns a copy of this node fact
  NodeFact* copy() const { return new CPMemLocObjectNodeFact(ml); }
  std::string str(std::string indent="") { return ml->str(); }
};

class ParityAnalysis : virtual public FWDataflow
{
  protected:
  //static std::map<varID, Lattice*> constVars;
  //AbstractObjectMap constVars;

  public:
  ParityAnalysis();

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy() { return boost::make_shared<ParityAnalysis>(); }

  // Creates a basic CPMemLocObject for the given SgNode. This object does not take into
  // account any constant propagation and will be used as a seed from which to propagate
  // more precise information.
  CPMemLocObjectPtr createBasicCPML(SgNode* sgn, PartEdgePtr pedge);

  // Initializes the state of analysis lattices at the given function, part and edge into our out of the part
  // by setting initLattices to refer to freshly-allocated Lattice objects.
  void genInitLattice(PartPtr part, PartEdgePtr pedge, PartPtr supersetPart,
                      std::vector<Lattice*>& initLattices);

  bool transfer(PartPtr part, PartPtr supersetPart, CFGNode cn, NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

  boost::shared_ptr<DFTransferVisitor> getTransferVisitor(PartPtr part, PartPtr supersetPart, CFGNode cn,
                                              NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

  boost::shared_ptr<ValueObject> Expr2Val(SgNode* n, PartEdgePtr pedge);
  bool implementsExpr2Val() { return true; }
  implTightness Expr2ValTightness() { return ComposedAnalysis::tight; }

  boost::shared_ptr<MemLocObject> Expr2MemLoc(SgNode* n, PartEdgePtr pedge);
  bool implementsExpr2MemLoc() { return true; }
  implTightness Expr2MemLocTightness() { return ComposedAnalysis::tight; }

  // pretty print for the object
  std::string str(std::string indent="")
  { return "ParityAnalysis"; }

  friend class ParityAnalysisTransfer;
}; // class ParityAnalysis

class ParityAnalysisTransfer : public VariableStateTransfer<ParityValueObject, ParityAnalysis>
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
  void visit(SgUnaryOp *sgn);

  void visit(SgValueExp *val);

  bool finish();

  ParityAnalysisTransfer(PartPtr part, PartPtr supersetPart, CFGNode cn, NodeState& state,
                                      std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                                      Composer* composer, ParityAnalysis* analysis);
};

}; //namespace fuse

#endif
