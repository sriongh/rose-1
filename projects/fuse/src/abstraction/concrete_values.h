#pragma once

#include "compose.h"
#include <boost/enable_shared_from_this.hpp>

namespace fuse
{
class ConcreteValueKind;
typedef boost::shared_ptr<ConcreteValueKind> ConcreteValueKindPtr;
extern ConcreteValueKindPtr NULLConcreteValueKind;

class ConcreteUninitializedKind;
typedef boost::shared_ptr<ConcreteUninitializedKind> ConcreteUninitializedKindPtr;

class ConcreteExactKind;
typedef boost::shared_ptr<ConcreteExactKind> ConcreteExactKindPtr;

class ConcreteOffsetListKind;
typedef boost::shared_ptr<ConcreteOffsetListKind> ConcreteOffsetListKindPtr;

class ConcreteUnknownKind;
typedef boost::shared_ptr<ConcreteUnknownKind> ConcreteUnknownKindPtr;

class ConcreteValueObject;
typedef boost::shared_ptr<ConcreteValueObject> ConcreteValueObjectPtr;

// Base class for the different kinds of values we can track. Used to identify both raw values,
// the locations of class fields within an object and the result of various arithmetic operatons
// among them. There may be arbitrary amounts of padding between fields within an object, meaning
// that we can only determine the order of their offsets within the object but not the actual
// concrete values of these offsets. Our representation of constants of either type is based on
// four different instances of the base class ConcreteValueKind:
//   - ConcreteUninitializedKind denotes the empty set of values
//   - ConcreteConcreteKind: denotes a concrete constant value that is representable as a SgValueExp
//   - ConcreteOffsetListKind: denotes a unique location within a MemRegion; they are represented as lists
//       of integral constant offsets (in terms of bytes) or where padding makes this unknown, rank values
//       that denote the position of a given location relative to others.
//   - ConcreteUnknownKind: denotes the set of all values
class ConcreteValueKind: public printable, public boost::enable_shared_from_this<ConcreteValueKind> {
  public:

  // The different kinds of ConcreteValueObjects
  typedef enum {uninitialized, // The value has not been initialized (analyses may set this value to anything they want)
                exact,   // The exact value is known and can be described with an SgValueExp
                offsetList, // The value denotes the offset of some class/struct member from a pointer the member's
                            // host object. Since the compiler can add arbitrary padding, such values are lists
                            // of integral offset values and members' ranks within their host objects.

                // !!! TODO
                serverImpl, // Thin wrapper for a server-implemented ValueObject
                unknown     // The exact value is not known
     } valueKind;
  valueKind kind;

  static std::string valueKind2Str(valueKind kind)
  { return (kind==uninitialized?"uninitialized":(kind==exact?"exact":(kind==offsetList?"offsetList":(kind==serverImpl?"serverImpl":(kind==unknown?"unknown":"???"))))); }

  // A comparable sub-class that makes it possible to compare instances of valueKind in a standard
  // way and use them as elements in hierarchical keys used for creating hierarchical datastructures
  // of AbstractObjects.
  class comparableKind: public comparable {
    public:
    valueKind kind;
    comparableKind(valueKind kind): kind(kind) {}

    // Equality is implemented via the == operator on valueKinds
    bool equal(const comparable& that_arg) const {
      //try {
        const comparableKind& that = dynamic_cast<const comparableKind&>(that_arg);
        return kind == that.kind;
      //} catch (std::bad_cast bc) {
      //  ROSE_ASSERT(0);
      //}
    }

    // Less-Than comparison is implemented via the < operator on valueKinds
    bool less(const comparable& that_arg) const {
      //try{
        const comparableKind& that = dynamic_cast<const comparableKind&>(that_arg);
        return kind < that.kind;
      //} catch (std::bad_cast bc) {
      //  ROSE_ASSERT(0);
      //}
    }

    std::string str(std::string indent="") const { return valueKind2Str(kind); }
  }; // class comparableKind
  typedef boost::shared_ptr<comparableKind> comparableKindPtr;

  ConcreteValueKind(valueKind kind): kind(kind)
  {}

  valueKind getKind() const { return kind; }

  // Methods that type-cast this kind into one of its sub-kinds. If the caller knows the correct
  // sub-kind by calling getKind(), they can be sure that the returned value is non-NULL.
  ConcreteUninitializedKindPtr asUninitializedKind()
  { return boost::static_pointer_cast<ConcreteUninitializedKind>(shared_from_this()); }
  ConcreteExactKindPtr asExactKind()
  { return boost::static_pointer_cast<ConcreteExactKind>(shared_from_this()); }
  ConcreteOffsetListKindPtr asOffsetListKind()
  { return boost::static_pointer_cast<ConcreteOffsetListKind>(shared_from_this()); }
  ConcreteUnknownKindPtr asUnknownKind()
  { return boost::static_pointer_cast<ConcreteUnknownKind>(shared_from_this()); }

  // Applies the given unary or binary operation to this and the given ConcreteValueKind
  // Returns:
  //    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated ConcreteValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ConcreteValueObject's list, return that.
  virtual ConcreteValueKindPtr op(SgUnaryOp* op)=0;
  virtual ConcreteValueKindPtr op(SgBinaryOp* op, ConcreteValueKindPtr that)=0;

  // Returns whether this and that ConcreteValueKinds are may/must equal to each other
  virtual bool mayEqualAO(ConcreteValueKindPtr that)=0;
  virtual bool mustEqualAO(ConcreteValueKindPtr that)=0;

  // Returns whether the two ConcreteValueKinds denote the same set of concrete values
  virtual bool equalSetAO(ConcreteValueKindPtr that)=0;

  // Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given ConcreteValueKind
  virtual bool subSetAO(ConcreteValueKindPtr that)=0;

  // Computes the meet of this and that and returns the resulting kind
  virtual std::pair<bool, ConcreteValueKindPtr> meetUpdateAO(ConcreteValueKindPtr that)=0;

  // Computes the intersection of this and that and returns the resulting kind
  virtual std::pair<bool, ConcreteValueKindPtr> intersectUpdateAO(ConcreteValueKindPtr that)=0;

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
  virtual bool isFullAO(PartEdgePtr pedge)=0;
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isEmptyAO(PartEdgePtr pedge)=0;

  // Returns a copy of this ConcreteExactKind
  virtual ConcreteValueKindPtr copyAOType() const=0;

  // Appends to the given hierarchical key the additional information that uniquely
  // identifies this type's set
  virtual void addHierSubKey(const AbstractionHierarchy::hierKeyPtr& key)=0;
}; // ConcreteValueKind

// ConcreteValueObjects form a set hierarchy and thus implement the AbstractionHierarchy
// interface. The hierarchy is:
// empty key: unknown
// non-empty:
//   uninitalized: special value to denote the empty set
//   concrete:
//     SgValueExp
//   offsetList:
//     rankT
//       long
//     offsetT
//       long

class ConcreteUninitializedKind : public ConcreteValueKind {
  public:
  ConcreteUninitializedKind() : ConcreteValueKind(uninitialized)
  {}

  // Applies the given unary or binary operation to this and the given ConcreteValueKind
  // Returns:
  //    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated ConcreteValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ConcreteValueObject's list, return that.
  ConcreteValueKindPtr op(SgUnaryOp* op);
  ConcreteValueKindPtr op(SgBinaryOp* op, ConcreteValueKindPtr that);

  // Returns whether this and that ConcreteValueKinds are may/must equal to each other
  bool mayEqualAO(ConcreteValueKindPtr that);
  bool mustEqualAO(ConcreteValueKindPtr that);

  // Returns whether the two ConcreteValueKinds denote the same set of concrete values
  bool equalSetAO(ConcreteValueKindPtr that);

  // Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given ConcreteValueKind
  bool subSetAO(ConcreteValueKindPtr that);

  // Computes the meet of this and that and returns the resulting kind
  std::pair<bool, ConcreteValueKindPtr> meetUpdateAO(ConcreteValueKindPtr that);

  // Computes the intersection of this and that and returns the resulting kind
  std::pair<bool, ConcreteValueKindPtr> intersectUpdateAO(ConcreteValueKindPtr that);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns a copy of this ConcreteUninitializedKind
  ConcreteValueKindPtr copyAOType() const { return boost::make_shared<ConcreteUninitializedKind>(); }

  std::string str(std::string indent="") const;

  // Appends to the given hierarchical key the additional information that uniquely
  // identifies this type's set
  void addHierSubKey(const AbstractionHierarchy::hierKeyPtr& key)
  // Don't add anything else since the hier-level key already separates the empty
  // set from all the other sets
  { }
}; // ConcreteUninitializedKind

class ConcreteExactKind : public ConcreteValueKind {
  // The expression that denotes the known value of this object
  boost::shared_ptr<SgValueExp> exp;

  public:
  ConcreteExactKind(boost::shared_ptr<SgValueExp> exp) : ConcreteValueKind(exact), exp(exp)
  {}

  boost::shared_ptr<SgValueExp> getVal() const { return exp; }

  // Applies the given operation functor to the expression in this ExactKind and returns the resulting ConcreteKind
  template<class DoOpType>
  ConcreteValueKindPtr doUnaryOp(DoOpType& doOp);

  // Applies the given operation functor to the expression in this ExactKind, whoch is assumed to be an integral type
  // and returns the resulting ConcreteKind
  template<class DoOpType>
  ConcreteValueKindPtr doUnaryIntegralOp(DoOpType& doOp);

  // Applies the given operation functor to the expression in this ExactKind, which is assumed to a floating point type
  // and returns the resulting ConcreteKind
  template<class DoOpType>
  ConcreteValueKindPtr doUnaryFloatOp(DoOpType& doOp);

  template<class DoOpType, class DoOpRetType>
  DoOpRetType bindDoOpArgs1(DoOpType doOp);

  template<class DoOpType, class DoOpRetType>
  DoOpRetType bindDoOpArgs2(DoOpType doOp);

  // Creates a ConcreteExactKind from the given value. This function is overloaded with different argument types
  // and for each type it creates a ConcreteExactKind with a different SgValueExp.
  ConcreteValueKindPtr createConcreteValueKindFromVal(bool val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(char val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(short val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(int val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(long val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(long long val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(unsigned char val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(unsigned short val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(unsigned int val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(unsigned long val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(unsigned long long val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(wchar_t val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(float val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(double val);
  ConcreteValueKindPtr createConcreteValueKindFromVal(long double val);

  // Applies the given binary operation functor to the expression in this and that ExactKinds, assuming they're
  // both integral types and returns the resulting ConcreteKind
  template<class DoOpType>
  ConcreteValueKindPtr doBinaryOpIntInt(DoOpType& doOp, ConcreteExactKindPtr that);

  // Applies the given binary operation functor to the expression in this and that ExactKinds, assuming at least one
  // is not an integral type and returns the resulting ConcreteKind
  template<class DoOpType>
  ConcreteValueKindPtr doBinaryOp(DoOpType& doOp, ConcreteExactKindPtr that);

  // Applies the given unary or binary operation to this and the given ConcreteValueKind
  // Returns:
  //    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated ConcreteValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ConcreteValueObject's list, return that.
  ConcreteValueKindPtr op(SgUnaryOp* op);
  ConcreteValueKindPtr op(SgBinaryOp* op, ConcreteValueKindPtr that);

  // Returns whether the two given SgValueExps denote the same numeric value.
  // If unknown, return unknownVal.
  static bool equalVals(SgValueExp* val1, SgValueExp* val2, bool unknownVal);

  // Returns whether the SgValueExps denoted by val1 < the value denoted by val2.
  // If unknown, return unknownVal.
  static bool lessThanVals(SgValueExp* val1, SgValueExp* val2, bool unknownVal);

  // Returns whether this and that ConcreteValueKinds are may/must equal to each other
  bool mayEqualAO(ConcreteValueKindPtr that);
  bool mustEqualAO(ConcreteValueKindPtr that);

  // Returns whether the two ConcreteValueKinds denote the same set of concrete values
  bool equalSetAO(ConcreteValueKindPtr that);

  // Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given ConcreteValueKind
  bool subSetAO(ConcreteValueKindPtr that);

  // Computes the meet of this and that and returns the resulting kind
  std::pair<bool, ConcreteValueKindPtr> meetUpdateAO(ConcreteValueKindPtr that);

  // Computes the intersection of this and that and returns the resulting kind
  std::pair<bool, ConcreteValueKindPtr> intersectUpdateAO(ConcreteValueKindPtr that);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns a copy of this ConcreteExactKind
  ConcreteValueKindPtr copyAOType() const { return boost::make_shared<ConcreteExactKind>(getVal()); }

  std::string str(std::string indent="") const;

  // Generic wrapper for comparing SgNode*'s that implements the comparable interface
  class comparableSgValueExp : public comparable {
    protected:
    SgValueExp *val;
    public:
    comparableSgValueExp(SgValueExp* val): val(val) {}
    bool equal(const comparable& that_arg) const {
      //try{
        const comparableSgValueExp& that = dynamic_cast<const comparableSgValueExp&>(that_arg);
        return equalVals(val, that.val, val==that.val);
      //} catch (std::bad_cast bc) {
      //  ROSE_ASSERT(0);
      //}
    }
    bool less(const comparable& that_arg) const {
      //try{
        const comparableSgValueExp& that = dynamic_cast<const comparableSgValueExp&>(that_arg);
        return equalVals(val, that.val, val<that.val);
      //} catch (std::bad_cast bc) {
      //  ROSE_ASSERT(0);
      //}
    }
    std::string str(std::string indent="") const { return SgNode2Str(val); }
  };
  typedef boost::shared_ptr<comparableSgNode> comparableSgNodePtr;

  // Appends to the given hierarchical key the additional information that uniquely
  // identifies this type's set
  void addHierSubKey(const AbstractionHierarchy::hierKeyPtr& key)
  { key->add(boost::make_shared<comparableSgValueExp>(exp.get())); }

}; // ConcreteExactKind


// Models unique locations with MemRegions, as lists of either
//   - integral constant offsets (in terms of bytes) or
//   - where padding makes this unknown, rank values that denote the position of a given location relative to others.
//
// Below are some examples of how locations within MemRegions are modeled by ConcreteOffsetListKind:
//   a.b.c: region(a), rank(b in a) + rank(c in b)
//   (&a.b)+1: region(a), rank(b in a) + concrete(1)
//   array[1].a[2].b[3]: region(array), concrete(1*sizeof(elt of array)) + rank(a in array elt) +
//                                      concrete(2*sizeof elt of array[1]) + rank(b in array[1] elt) +
//                                      concrete(3*sizeof elt of array[1].a[2])
//
// This information is maintained by ConcreteOffsetListKind as a linked list. However, some combinations
// can ve represented more concisely:
//   concrete + concrete: concrete object that denotes the sum of the arguments
//   concrete + rank, rank + concrete: maintained as a list
//   rank + rank: unknown
//   * + unknown, unknown + *: a single unknown value object
class ConcreteOffsetListKind : public ConcreteValueKind {
  class intWrap: public comparable {
    public:
    typedef enum {rankT, offsetT} type;

    private:
    type t;

    long long v;
    public:
    intWrap(long long v, type t): t(t), v(v){}
    intWrap(const intWrap& that): t(that.t), v(that.v){}

    bool operator==(const intWrap& that) const { return t==that.t && v==that.v; }
    bool operator!=(const intWrap& that) const { return !(*this == that); }

    long long operator=(long long v) { return (this->v = v);}

    long long get() const { return v; }
    void set(unsigned long long v) { this->v = v; }

    type getType() const { return t; }

    bool equal(const comparable& that_arg) const {
      //try {
        const intWrap& that = dynamic_cast<const intWrap&>(that_arg);
        return t == that.t && v == that.v;
      //} catch (std::bad_cast bc) {
      //  ROSE_ASSERT(0);
      //}
    }
    bool less(const comparable& that_arg) const {
      //try{
        const intWrap& that = dynamic_cast<const intWrap&>(that_arg);
        return (t  < that.t) ||
               (t == that.t && v < that.v);
      //} catch (std::bad_cast bc) {
      //  ROSE_ASSERT(0);
      //}
    }
    std::string str(std::string indent="") const { return txt()<<"[type="<<(t==rankT?"rankT":(t==offsetT?"offsetT":"NULL"))<<", v="<<v<<"]"; }
  }; // class intWrap
  typedef boost::shared_ptr<intWrap> intWrapPtr;

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

  // List that identifies the unique location of a given MemLoc within a MemRegion. It is a list
  // of offsets and ranks, the former specifying a concrete number of offset bytes and the latter
  // specifying an order among indexes within MemRegions but without specifying a concrete number
  // of bytes.
  std::list<intWrap> offsetL;

  public:
  ConcreteOffsetListKind(const rank& r) : ConcreteValueKind(offsetList) {
    if(type==NULL) type = SageBuilder::buildLongLongType();
    offsetL.push_back(r);
  }

  ConcreteOffsetListKind(const offset& o) : ConcreteValueKind(offsetList) {
    if(type==NULL) type = SageBuilder::buildLongLongType();
    offsetL.push_back(o);
  }

  ConcreteOffsetListKind(const std::list<intWrap>& offsetL) : ConcreteValueKind(offsetList), offsetL(offsetL) {
    if(type==NULL) type = SageBuilder::buildLongLongType();
  }

  // Applies the given unary or binary operation to this and the given ConcreteValueKind
  // Returns:
  //    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated ConcreteValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ConcreteValueObject's list, return that.
  ConcreteValueKindPtr op(SgUnaryOp* op);
  ConcreteValueKindPtr op(SgBinaryOp* op, ConcreteValueKindPtr that);

  // Returns whether this and that ConcreteValueKinds are may/must equal to each other
  bool mayEqualAO(ConcreteValueKindPtr that);
  bool mustEqualAO(ConcreteValueKindPtr that);

  // Returns whether the two ConcreteValueKinds denote the same set of concrete values
  bool equalSetAO(ConcreteValueKindPtr that);

  // Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given ConcreteValueKind
  bool subSetAO(ConcreteValueKindPtr that);

  // Computes the meet of this and that and returns the resulting kind
  std::pair<bool, ConcreteValueKindPtr> meetUpdateAO(ConcreteValueKindPtr that);

  // Computes the intersection of this and that and returns the resulting kind
  std::pair<bool, ConcreteValueKindPtr> intersectUpdateAO(ConcreteValueKindPtr that);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns a copy of this ConcreteOffsetListKind
  ConcreteValueKindPtr copyAOType() const { return boost::make_shared<ConcreteOffsetListKind>(offsetL); }

  std::string str(std::string indent="") const;

  // Appends to the given hierarchical key the additional information that uniquely
  // identifies this type's set
  void addHierSubKey(const AbstractionHierarchy::hierKeyPtr& key) {
    // Add all the intWrap objects on the offset list to the key
    for(std::list<intWrap>::iterator o=offsetL.begin(); o!=offsetL.end(); o++)
      key->add(boost::make_shared<intWrap>(*o));
  }
}; // ConcreteOffsetListKind

class ConcreteUnknownKind : public ConcreteValueKind {
  public:
  ConcreteUnknownKind() : ConcreteValueKind(unknown)
  {}

  // Applies the given unary or binary operation to this and the given ConcreteValueKind
  // Returns:
  //    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
  //       return a freshly-allocated ConcreteValueKind that holds the result.
  //    - if the two objects could not be merged and therefore that must be placed after
  //       this in the parent ConcreteValueObject's list, return that.
  ConcreteValueKindPtr op(SgUnaryOp* op);
  ConcreteValueKindPtr op(SgBinaryOp* op, ConcreteValueKindPtr that);

  // Returns whether this and that ConcreteValueKinds are may/must equal to each other
  bool mayEqualAO(ConcreteValueKindPtr that);
  bool mustEqualAO(ConcreteValueKindPtr that);

  // Returns whether the two ConcreteValueKinds denote the same set of concrete values
  bool equalSetAO(ConcreteValueKindPtr that);

  // Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given ConcreteValueKind
  bool subSetAO(ConcreteValueKindPtr that);

  // Computes the meet of this and that and returns the resulting kind
  std::pair<bool, ConcreteValueKindPtr> meetUpdateAO(ConcreteValueKindPtr that);

  // Computes the intersection of this and that and returns the resulting kind
  std::pair<bool, ConcreteValueKindPtr> intersectUpdateAO(ConcreteValueKindPtr that);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns a copy of this ConcreteUnknownKind
  ConcreteValueKindPtr copyAOType() const { return boost::make_shared<ConcreteUnknownKind>(); }

  std::string str(std::string indent="") const;

  // Appends to the given hierarchical key the additional information that uniquely
  // identifies this type's set
  void addHierSubKey(const AbstractionHierarchy::hierKeyPtr& key)
  // Don't add anything else since the full set is denoted by the empty key
  { }
}; // ConcreteUnknownKind

// Generic implementation of ValueObjects based on ConcreteValueKinds
class ConcreteValueObject : public ValueObject {
  public:
  // The lattice that grounds the definition of this ValueObject.
  ConcreteValueKindPtr kind;

  ConcreteValueObject(ConcreteValueKindPtr kind, SgNode* base=NULL);

  // Wrapper for shared_from_this that returns an instance of this class rather than its parent
  ConcreteValueObjectPtr shared_from_this() { return boost::static_pointer_cast<ConcreteValueObject>(ValueObject::shared_from_this()); }

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
}; // class ConcreteValueObject

}; // namespace fuse
