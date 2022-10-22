#ifndef ABSTRACT_OBJECT_H
#define ABSTRACT_OBJECT_H

//#include "sight.h"
#include "partitions.h"
#include "CallGraphTraverse.h"
#include "analysis.h"
#include <string>
#include <cstring>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

/* GB 2012-09-02: DESIGN NOTE
 * All actions on and queries of AbstractObjects are done in the context of some PartEdge. This is manifested differently
 * in different scenarios.
 * AbstractObjects from prior analyses - pointers to these are used throughout client analyses and thus, they should
 *    not maintain a reference to a host PartEdge. Since they come from completed analyses it is sufficient for callers
 *    to pass a PartEdge into the call and for the response to be computed with respect to this PartEdge, based on the results
 *    of the prior analysis. For example, the liveness of a given MemLocObject at a given PartEdge can be determined
 *    by looking at the lattice left behind at that PartEdge by the live-dead analysis.
 * AbstractObjects being kept by current analysis - these are propagated in dataflow style throughout the CFG within
 *    a given analysis. The analysis' transfer functions should ensure that every time the meaning of an AbstractObject
 *    changes as a result of being propagated across PartEdges, this is reflected in the object's internal information
 *    without maintaining an explicit dependence on the PartEdge. This, however, is not a strict requirement since there
 *    may be syntactic information relevant to the meaning of the object that requires a reference to the origin PartEdge.
 * Containers that include just objects from current analysis - like above, should maintain no reference to their
 *    source PartEdge since it is not needed. Further, the typical use-case will be to have one copy of a container for
 *    each PartEdge, meaning that they can maintain their identity without explicitly knowing the PartEdge
 * Containers that include some of both types of objects (prior and current) - there should be a separate instance of
 *    these containers for each PartEdge and each should maintain explicit reference to its host PartEdge. Thus, when it needs
 *    to provide the PartEdge to calls to functions within AbstractObjects from prior analyses, this PartEdge is always
 *    available.
 */

/* GB 2012-10-22: DESIGN NOTE
 * When it comes to the structure of MemLocObjects (e.g. are they scalars, labeled aggregates, etc. and if so, what's
 * their internal structure) we need to decide how much the reported structure can change across PartEdges. Can
 * a MemLocObject that was a Scalar in one PartEdge become an Array in another? If a Pointer dereferences to a given
 * MemLocObject in one PartEdge, must it do the same in every other? The decision for now is to make the choice of
 * MemLocObject type (Scalar, FunctionName, LabeledAggregate, Array or Pointer) static in that once a MemLocObject is
 * created, its type is fixed and does not change from one PartEdge to another. All other aspects of a MemLocObject
 * can vary freely. Thus, methods such as MemLocObject::isScalar() do not take a PartEdge as an argument, whereas
 * methods such as LabeledAggregate::fieldCount() do.
 */

// ----------------------------
// ----- Abstract Objects -----
// ----------------------------

namespace fuse {
class Composer;

// Root class of all abstract objects
class AbstractObject;
typedef boost::shared_ptr<AbstractObject> AbstractObjectPtr;
extern AbstractObjectPtr NULLAbstractObject;

class CodeLocObject;
class ValueObject;
class MemLocObject;
class MemRegionObject;

typedef enum {Union, Intersection} uiType;
// Describes for a given pair of MemLocs or MemRegions whether either is a FuncResultMemLoc/MemRegion
// and how they relate to each other
typedef enum {FuncResultSameFunc, // Both are FuncResult* and refer to the same function
              FuncResultUnequal,  // Both are FuncResult* and refer to the different functions or
                                  // one is a FuncResult* and the other is not
              NeitherFuncResult   // Neither is a FuncResult*
              } FuncResultRelationType;

class AbstractionHierarchy;
typedef boost::shared_ptr<AbstractionHierarchy> AbstractionHierarchyPtr;
class AbstractionHierarchy {
  public:
  // The key that identifies different sets in the hierarchy. The key a list that
  // encodes a path through the hierarchy from
  // the root to this specific node, which may be internal or leaf. Different
  // objects are compared by comparing these paths. Two objects are mayEqual if
  // they share a prefix and they are equalSet if their entire paths are identical.
  // Each element in the path is some object that implements the < and ==
  // comparison operations, which is ensured by having them derived from the
  // comparable class.
  // Empty key lists denote the full set.
  // The empty set can be denoted by any key that can never be a prefix of
  // another key.
  class hierKey;
  typedef boost::shared_ptr<hierKey> hierKeyPtr;
  class IntersectHierKey;
  //template<class Key> class IntersectMappedHierKey<Key>;

  class hierKey : public comparable {
    /*friend class IntersectHierKey;
    template<class Key> friend class IntersectMappedHierKey<Key>;
    protected:*/
    public:
    std::list<comparablePtr> keyList;

    // Indicates whether this object has reached the end of its location within
    // the hierarchy hierarchy. If so, no additional fields will be added to it.
    // This is useful for describing a set of memory locations where the
    // memory region is uncertain. Thus, even though index information is available,
    // it cannot be provided in the standard hierarchical format where we
    // first specify the portion of the hierarchy for the region and then the portion
    // for the index. As such, we simply ignore the index by settingendOfHierarchy
    // to true.
    // An alternative design would not use a list of keys but allow more complex
    // structures to be built to make it possible for different analyses to add
    // levels to the object hierarchy structure that interleave with levels provided
    // by other analyses
    bool endOfHierarchy;

    public:
    hierKey(bool endOfHierarchy=false);
    hierKey(const std::list<comparablePtr>& keyList, bool endOfHierarchy);
    hierKey(comparablePtr subKey, bool endOfHierarchy=false);
    hierKey(hierKeyPtr that);

    std::list<comparablePtr>::const_iterator begin();
    std::list<comparablePtr>::const_iterator end();
    void add(comparablePtr c);
    void add(std::list<comparablePtr>::const_iterator beginIt, std::list<comparablePtr>::const_iterator endIt);
    const std::list<comparablePtr>& getList();

    void reachedEndOfHierarchy();

    virtual bool equal(const comparable& that) const;
    virtual bool less(const comparable& that) const;

    // Returns whether the set denoted by key is live at the given PartEdge
    virtual bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)=0;

    friend std::ostream& operator<<(std::ostream& s, AbstractionHierarchy::hierKey* k);
  }; // class hierKey

  class IntersectComparable : public comparable {
      std::list<comparablePtr> subComp;

      public:
      IntersectComparable();
      IntersectComparable(const std::list<comparablePtr>& subComp);

      // Add a new comparablePtr to this intersection
      void add(comparablePtr newC);

      // This == That
      bool equal(const comparable& that) const;
      // This < That
      bool less(const comparable& that) const;

      // String method
      std::string str(std::string indent="") const;
    }; // IntersectComparable
    typedef CompSharedPtr<IntersectComparable> IntersectComparablePtr;

    template<typename Key>
    class IntersectMappedComparable : public comparable {
        std::map<Key, comparablePtr> subComp;

        public:
        IntersectMappedComparable();
        IntersectMappedComparable(const std::map<Key, comparablePtr>& subComp);

        // Add a new comparablePtr to this intersection
        void add(Key k, comparablePtr newC);

        // This == That
        bool equal(const comparable& that) const;
        // This < That
        bool less(const comparable& that) const;

        // String method
        std::string str(std::string indent="") const;
      }; // IntersectMappedComparable
      //typedef CompSharedPtr<IntersectMappedComparable> IntersectMappedComparablePtr;

  class IntersectHierKey: public hierKey {
      std::list<hierKeyPtr> subKeys;
      public:
      IntersectHierKey(const std::list<hierKeyPtr>& subKeys);

      // Returns whether the set denoted by key is live at the given PartEdge
      bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    };

  template<class Key>
  class IntersectMappedHierKey: public hierKey {
    std::map<Key, hierKeyPtr> subKeys;
    public:
    IntersectMappedHierKey(const std::map<Key, hierKeyPtr>& subKeys);

    // Returns whether the set denoted by key is live at the given PartEdge
    bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  };

  // An implementation of hierKey that uses an instance of some representative
  // AbstractObject (that denotes the same set as the key) to answer isLive queries
  class AOSHierKey: public hierKey {
    AbstractObjectPtr obj;
    public:
    AOSHierKey(AbstractObjectPtr obj, bool endOfHierarchy=false);
    //AOSHierKey(AbstractObjectPtr obj, const std::list<comparablePtr>& keyList, bool endOfHierarchy=false): hierKey(keyList, endOfHierarchy), obj(obj) {}
    //AOSHierKey(AbstractObjectPtr obj, comparablePtr subKey, bool endOfHierarchy=false): hierKey(subKey, endOfHierarchy), obj(obj) { }
    AOSHierKey(AbstractObjectPtr obj, hierKeyPtr subHierKey);
    bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  }; // class AOSHierKey

  protected:
  bool isHierKeyCached;
  hierKeyPtr cachedHierKey;

  public:

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy
  virtual const hierKeyPtr& getHierKey() const=0;

  AbstractionHierarchy() : isHierKeyCached(false) {}
  AbstractionHierarchy(const AbstractionHierarchy& that) :
    isHierKeyCached(that.isHierKeyCached), cachedHierKey(that.cachedHierKey) {}

  // The possible ways in which two hierarchical AbstractObjects may be related to each other
  typedef enum {equal,         // Their keys are the same
                leftContains,  // The key of the left operand is a prefix of the key of the
                               //   right operand and thus, the left set contains the right one.
                rightContains, // Inverse of above
                disjoint}      // The two keys are not equal, meaning that the corresponding
                               //   sets are disjoint.
           hierRel;

  // Generic comparison method for hierarchical objects
  hierRel hierCompare(AbstractionHierarchyPtr left, AbstractionHierarchyPtr right);

  bool operator==(AbstractionHierarchyPtr that) {
    return getHierKey() == that->getHierKey();
  }
  bool operator<(AbstractionHierarchyPtr that) {
    return getHierKey() < that->getHierKey();
  }
}; // class AbstractionHierarchy

// Abstract class that defines the API for Abstractions that form partial orders.
// When object A == object B (A.getPOKey().isOPEqual(B.getPOKey()) is true), this means that their sets are equal.
// When object A < object B (A.getPOKey().isPOLessThan(B.getPOKey()) is true), this means that set A contains set B.
// Objects that are not related via the partial order are guaranteed to be disjoint.
class AbstractionPartialOrder {
  public:
  // Returns whether sets of the given type form a partial order
  virtual bool isPartialOrder() { return false; }

  // Key that identifies this Abstraction's location within the partial order
  class PartialOrderKey;
  typedef boost::shared_ptr<PartialOrderKey> PartialOrderKeyPtr;
  class PartialOrderKey {
    // If isPartialOrder() returns true, classes must implement the methods below to establish
    // the partial order relationship among class instances.
    virtual bool isPOLessThan(const PartialOrderKey& that) { assert(0); }
    virtual bool isPOEqual(const PartialOrderKey& that) { assert(0); }

    bool isPOLessThan(PartialOrderKeyPtr that) { return isPOLessThan(*that.get()); }
    bool isPOEqual(PartialOrderKeyPtr that)    { return isPOEqual(*that.get()); }
  }; // class PartialOrderKey

  // Returns the PartialOrderKey object that defines this Abstraction's location within its partial order
  virtual PartialOrderKeyPtr getPOKey() { assert(0); }
}; // class AbstractionPartialOrder
typedef boost::shared_ptr<AbstractionPartialOrder> AbstractionPartialOrderPtr;

// Base class of abstract entities that denote sets of things.
// AbstractObjects derive from Abstraction and denote sets of state components (CodeLocs, Values,
//   MemRegions and MemLocs)
// Lattices derive from Abstraction and denote analysis-internal constraints that denote sets of
//   components, application states or even executions. They're not meant to be exposed to other
//   analyses and thus don't implement a complex API for external use.
// The methods specified in the Abstraction class focus on testing and manipulating these objects
//   as sets.

class MAOMap;
typedef boost::shared_ptr<MAOMap> MAOMapPtr;

class Abstraction;
typedef boost::shared_ptr<Abstraction> AbstractionPtr;
class Abstraction {
  public:
  // Returns a copy of this Abstraction
  virtual AbstractionPtr copyA() const=0;

  // Returns whether this object may/must be equal to o within the given Part p
  virtual bool mayEqual(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  virtual bool mustEqual(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;

  // General versions of equalSet() that accounts for framework details before routing the call to the
  // derived class' equalSet() check. Specifically, it routes the call through the composer to make
  // sure the equalSet() call gets the right PartEdge.
  virtual bool equalSet(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;

  // General versions of subSet() that accounts for framework details before routing the call to the
  // derived class' subSet() check. Specifically, it routes the call through the composer to make
  // sure the subSet() call gets the right PartEdge.
  virtual bool subSet(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdate(AbstractionPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;

  // Computes the intersection of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool intersectUpdate(AbstractionPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;

  // It is often useful to create an Abstraction object that denotes the union or intersection
  // of multiple other objects. There are multiple ways to do this:
  // - We can create a copy of one of the objects to be unioned and call its meetUpdate() and
  //   intersectUpdate() methods to union/intersect the others in. This is supported for all
  //   Abstractions.
  // - We can create an object that maps some keys to Abstractions and implements all relevant operations
  //   by forwarding them to the Abstractions within it and returning the most (intersection) or least (union)
  //   conservative answer. This only works for AbstractObjects via the MappedAbstractObject class.
  //   It is made more challenging by the fact that the keys may be any type, which hidden from the users
  //   of these objects.
  // The functions below allow either of the above methods to be used to create unions and intersections.
  // They take as argument a MAOMap, which maps some unknown keys to Abstractions. This is a good choice
  // because it allows users to iterate over the mapped abstractions without knowing anything about the
  // type of the keys, and because they can return MappedAbstractObjects that contain the Abstractions
  // that are mapped inside of them MAOMap::getMappedObj().
  virtual AbstractionPtr genUnion(MAOMapPtr maoMap)=0;
  virtual AbstractionPtr genIntersection(MAOMapPtr maoMap)=0;

  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  virtual bool setToFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  // Set this Lattice object to represent the of no execution prefixes (empty set).
  // Return true if this causes the object to change and false otherwise.
  virtual bool setToEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;

  // General versions of isFull() and isEmpty that account for framework details before routing the call to the
  // derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make
  // sure the isFullAO() and isEmptyAO() call gets the right PartEdge.
  virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;

  // Return a human-readable representation of this object
  virtual std::string str(std::string indent="") const=0;
}; // class Abstraction

// DESIGN NOTE:
// The most common use-case under which AbstractObjects are used by analyses are:
// - relations: mayEqual, mustEqual, subSet, equalsSet
// - combination: meetUpdate
// - property check: isLive, isFull, isEmpty, isConcrete, concreteSetSize, etc.
// These operations are methods within AbstractObjects, allowing any analysis to call
//   them without worrying what type of object it has. This means that the base versions
//   of the methods that apply to multiple AbstractObjects, must take a generic AbstractObjectPtr
//   as an argument rather than a CodeLocPtr, ValueObjectPtr MemLocObjectPtr or MemRegionObjectPtr.
// Further, since the results of these operations depend on the PartEdge from where the
//   question is asked, this PartEdge is provided as an argument to each of these methods.
//   A challenge to this design is that the analysis that implemented of the AbstractObject
//   may not have run on the same ATS as the client analysis that asked the question. This
//   makes it necessary to translate from the client's PartEdge to the corresponding PartEdge
//   of the server analysis. This is done by a composer, which is passed in as an argument
//   to these methods (along with the client analysis, in case the composer needs this).
// Given the above issues the classes below implement the above operations on AbstractObjects
//   as follows.
// - Each implements an abstract virtual method from the base AbstractObject class
//   (operand argument is optional)
//   type op(AbstractObjectPtr operand, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
// - For methods that take an operand AbstractObjectPtr, we cast it to the correct sub-type
//   (CodeLocObjectPTr, ValueObjectPtr, ...) and call a more specific version of the method:
//   type op(SpecializedObjectPtr operand, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
// - To convert the given PartEdge to one that is valid for the server analysis that implements the query,
//   these methods calls comp->opZZ(operand, pedge, analysis). This call converts the PartEdge appropriately
//   and ultimately calls method
//   type opZZ(SpecializedObjectPtr operand, PartEdgePtr pedge)
//   where ZZ is an abbreviation of the type of the object: CL:CodeLoc, V:Value, ML:MemLoc, MR:MemRegion
// - This method performs the logic of the query and returns the result.
// Note that analyses must call the op() methods rather than the opZZ() methods since the former will do
// appropriate PartEdge conversion while the latter will not.
//
// The logic is somewhat different for compound objects: MemLocs, Combined*, and Mapped*. These objects
// implement the above functions from AbstractObject by forwarding them to the op() functions in the
// objects inside them. If these are themselves compount, they continue the forwarding process. If
// they're not compound, they perform the above logic, converting PartEdges and calling opZZ() methods.
// Compound objects thus have no need for opZZ() methods.

class AbstractObject : public Abstraction, public boost::enable_shared_from_this<AbstractObject>,
                       public AbstractionHierarchy,
                       public AbstractionPartialOrder
{
  SgNode* base;

  public:
  typedef enum {Value, CodeLoc, MemRegion, MemLoc} AOType;

  AbstractObject() {}
  AbstractObject(SgNode* base) : base(base) {}
  AbstractObject(const AbstractObject& that) : AbstractionHierarchy(that), base(that.base) {}

  virtual ~AbstractObject();

  virtual SgNode* getBase() const { return base; }

  // Analyses that are being composed inside a given composer provide a pointer to themselves
  // in the client argument. Code that uses the composer from the outside, does not need to provide
  // a client.

  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  virtual bool isValueObject();
  virtual bool isCodeLocObject();
  virtual bool isMemRegionObject();
  virtual bool isMemLocObject();
  virtual AOType getAOType() const=0;

  // Returns true if this is a MappedAbstractObject and false otherwise
  virtual bool isMappedAO();

  // Returns whether this object may/must be equal to o within the given Part p
  virtual bool mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  virtual bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  bool mayEqual(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return mayEqual(boost::static_pointer_cast<AbstractObject>(o), pedge, comp, analysis); }
  bool mustEqual(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return mustEqual(boost::static_pointer_cast<AbstractObject>(o), pedge, comp, analysis); }

  // Simple equality test that just checks whether the two objects correspond to the same expression
  //bool mustEqualExpr(AbstractObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  //virtual bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge)=0;

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  //virtual bool subSet(AbstractObjectPtr o, PartEdgePtr pedge)=0;

  // General versions of equalSet() that accounts for framework details before routing the call to the
  // derived class' equalSet() check. Specifically, it routes the call through the composer to make
  // sure the equalSet() call gets the right PartEdge.
  virtual bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  bool equalSet(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return equalSet(boost::static_pointer_cast<AbstractObject>(o), pedge, comp, analysis); }

  // General versions of subSet() that accounts for framework details before routing the call to the
  // derived class' subSet() check. Specifically, it routes the call through the composer to make
  // sure the subSet() call gets the right PartEdge.
  virtual bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  bool subSet(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return subSet(boost::static_pointer_cast<AbstractObject>(o), pedge, comp, analysis); }

  // Returns true if this object is live at the given part and false otherwise
  virtual bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  bool meetUpdate(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return meetUpdate(boost::static_pointer_cast<AbstractObject>(o), pedge, comp, analysis); }

  // Computes the intersection of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool intersectUpdate(AbstractionPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { assert(0); }

  // !!! The default implementations provided here simply abort. This is because setToFull() and setToEmpty()
  // !!! are only useful for AbstractObjects that are also Lattices (inherit from both). Those AbstractObjects
  // !!! will implement setToFull() and setToEmpty() and those definitions will override these ones.
  // !!! AbstractObjects that are not used as Lattices will not need to implement these functions and the
  // !!! within them will be irrelevant since they're never called.
  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  virtual bool setToFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL) { assert(0); }
  // Set this Lattice object to represent the of no execution prefixes (empty set).
  // Return true if this causes the object to change and false otherwise.
  virtual bool setToEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL) { assert(0); }

/*  // General versions of isFull() and isEmpty that account for framework details before routing the call to the
  // derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make
  // sure the isFullAO() and isEmptyAO() call gets the right PartEdge.
  virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  bool isFull(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return isFull(pedge, comp, analysis); }

  virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  bool isEmpty(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return isEmpty(pedge, comp, analysis); }*/

  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();

  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Returns true if this AbstractObject corresponds to a concrete value that is statically-known
  virtual bool isConcrete()=0;
  // Returns the number of concrete values in this set
  virtual int concreteSetSize()=0;

  // It is often useful to create an Abstraction object that denotes the union or intersection
  // of multiple other objects. There are multiple ways to do this:
  // - We can create a copy of one of the objects to be unioned and call its meetUpdate() and
  //   intersectUpdate() methods to union/intersect the others in. This is supported for all
  //   Abstractions.
  // - We can create an object that maps some keys to Abstractions and implements all relevant operations
  //   by forwarding them to the Abstractions within it and returning the most (intersection) or least (union)
  //   conservative answer. This only works for AbstractObjects via the MappedAbstractObject class.
  //   It is made more challenging by the fact that the keys may be any type, which hidden from the users
  //   of these objects.
  // The functions below allow either of the above methods to be used to create unions and intersections.
  // They take as argument a MAOMap, which maps some unknown keys to Abstractions. This is a good choice
  // because it allows users to iterate over the mapped abstractions without knowing anything about the
  // type of the keys, and because they can return MappedAbstractObjects that contain the Abstractions
  // that are mapped inside of them MAOMap::getMappedObj().
  AbstractionPtr genUnion(MAOMapPtr maoMap);
  AbstractionPtr genIntersection(MAOMapPtr maoMap);

  // Allocates a copy of this object and returns a pointer to it
  virtual AbstractObjectPtr copyAO() const=0;

  // Returns a copy of this Abstraction
  AbstractionPtr copyA() const
  { return boost::static_pointer_cast<Abstraction>(copyAO()); }

  /* Don't have good idea how to represent a finite number of options
  virtual bool isFiniteSet()=0;
  virtual set<AbstractObj> getValueSet()=0;*/

  // ------------------------------------------------------------------------
  // ----- Constraints on relationships among different AbstractObjects -----
  // ------------------------------------------------------------------------

  // In the general case AbstractObjects may denote arbitrary sets. Without any constraints about how such sets may
  // relate to each other, it is not possible to create efficient data structures that store and organize them.
  // However, many implementations of AbstractObjects place constraints on possible inter-object relationships
  // that can be used to improve the efficiency of data structures.
  //
  // Support for AbstractObjects with different kinds of constraints is implemented by having each object instance
  // implement a function that returns whether it supports a given type of constraints. All objects of a given type
  // returned by the same analysis (i.e. those that may ever be compared to each other) must return the same
  // value for this method. For each kind of constraints there's a class that contains the special methods of
  // that constraint kind and objects that have this constraint must inherit from this class and implement its
  // abstract virtual methods

  // ----------------------------------------
  // Objects that form a hierarchy, where
  // - objects at each level of the hierarchy are disjoint and form a total order
  // - objects at each level are contained in some object in a prior level
  // Example: memory abstraction
  //                       Full set
  // Heap Memory,      Named Variables,       Expressions
  //                   varX, varY, ...        a=b, q++, ...

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  virtual bool isHierarchy() const { return false; }
  // AbstractObjects that form a hierarchy must inherit from the AbstractionHierarchy class

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy
  virtual const hierKeyPtr& getHierKey() const { assert(0); }

  // ----------------------------------------
  // Objects that denote disjoint sets. Because no two sets may overlap, they can be
  // represented using unique numbers, which enables efficient data structure implementations.
  virtual bool isDisjoint() const { return false; }
  // AbstractObjects that form a hierarchy must inherit from the AbstractObjectDisjoint class

  //virtual std::string str(std::string indent="") const=0;

  // Variant of the str method that can produce information specific to the current Part.
  // Useful since AbstractObjects can change from one Part to another.
  virtual std::string strp(PartEdgePtr pedge, std::string indent="")
  { return str(indent); }
};

// Stringification of hierKeys
std::ostream& operator<<(std::ostream& s, AbstractionHierarchy::hierKey* k);
std::ostream& operator<<(std::ostream& s, AbstractionHierarchy::hierKeyPtr k);

class AbstractObjectDisjoint;
typedef boost::shared_ptr<AbstractObjectDisjoint> AbstractObjectDisjointPtr;
class AbstractObjectDisjoint {
  public:
  // Return a number that uniquely identifies this object
  virtual long long getHashCode() const=0;
}; // class AbstractObjectDisjoint


/* GB: Commented to avoid having to think of how to implement isLive in AbstractObjectDisjointHierWrap.

// Wrapper class for AbstractObjectDisjoint that implements the AbstractionHierarchy
// API for use with data structures that support hierarchical objects but not disjoint ones.
class AbstractObjectDisjointHierWrap;
typedef boost::shared_ptr<AbstractObjectDisjointHierWrap> AbstractObjectDisjointHierWrapPtr;
class AbstractObjectDisjointHierWrap : public AbstractionHierarchy, public AbstractObjectDisjoint,  public boost::enable_shared_from_this<AbstractObjectDisjointHierWrap>
{
  class hashCodeWrapper : public comparable {
    long long code;
    public:
    hashCodeWrapper(long long code): code(code) {}

    // Operations that derived classes must implement
    bool equal(const comparable& that_arg) const {
      const hashCodeWrapper& that = dynamic_cast<const hashCodeWrapper&> (that_arg);
      return code == that.code;
    }

    bool less(const comparable& that_arg) const {
      const hashCodeWrapper& that = dynamic_cast<const hashCodeWrapper&> (that_arg);
      return code < that.code;
    }

    std::string str(std::string indent="") const { return sight::txt()<<code; }
  };
  CompSharedPtr<hashCodeWrapper> keyCode;
  public:

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const hierKeyPtr& getHierKey() const;

//  AbstractObjectDisjointHierWrapPtr shared_from_this() { return boost::static_pointer_cast<AbstractObjectDisjointHierWrap>(shared_from_this()); }
}; // class AbstractObjectDisjointHierWrap
*/

/* Base class for AbstractObjects that combine multiple other AOs into unions or
 * intersections by mapping from some key to the AOs.
 *
 * MappedAbstractObjects are templated by their key type, which makes it difficult for
 * code to access the objects that they contain since other code will likey get them
 * as AbstractObjects and won't know which type was used as key and thus won't be able
 * to dynamically cast them. As such, the MappedAbstractObject is not templated by
 * the key type and defines functionality for iterating over the internals of
 * MappedAbstractObjects without knowing their key type.
 */

// When external code needs to operate on MappedAbstractObjects it needs to create data
// structures that use the same key type as a MappedAbstractObject. The MAOMap below provides this
// capability. Given a single instance of a MappedAbstractObject users can get an
// AOMap that uses the same key type as the MappedAbstractObject but any value type.
// Then, when given some MappedAbstractObject that has the same type, this MAOMap
// allows users to iterate over the keys of the MappedAbstractObject and either
// get or set the values of the AOMap that correspond to those keys, all without
// exposing the actual keys and their type to the users. The MAOMap is defined in an
// un-templated base class of MappedAbstractObjects to make it possible to external code
// to access it without knowing the template parameters of a given MappedAbstractObject.

// Interface of maps from keys to objects
class MAOMap {
  public:

  // Iterates over the keys in mappedAO for the purpose of getting and then setting their values.
  class setMapFunc {
    public:
    // Applied to each Key in a given MappedAbstractObject
    // obj: the AO mapped to the key
    // valMapped: indicates whether there is a value currently mapped to the key in the AOMap
    // curVal: if valMapped is true, curVal contains the current value mapped in AOMap
    // Returns the new value to be mapped to the current key in AOMap
    virtual boost::shared_ptr<void> operator()(AbstractionPtr obj, boost::shared_ptr<void> curVal, bool valMapped)=0;
  };

  virtual void set(AbstractionPtr mappedAO, setMapFunc& f)=0;

  // Iterates over the keys in mappedAO for the purpose of getting but not setting their values.
  class getMapFunc {
    public:
    // Applied to each Key in a given MappedAbstractObject
    // obj: the AO mapped to the key
    // valMapped: indicates whether there is a value currently mapped to the key in the AOMap
    // curVal: if valMapped is true, curVal contains the current value mapped in AOMap
    virtual void operator()(AbstractionPtr obj, boost::shared_ptr<void> curVal, bool valMapped)=0;
  };

  virtual void get(AbstractionPtr mappedAO, getMapFunc& f)=0;

  class applyStrMapFunc {
    public:
    // Applied to each Key in a map
    // keyStr: string representation of the current key
    // curVal: contains the current value mapped in AOMap
    virtual void operator()(const std::string& keyStr, boost::shared_ptr<void> curVal)=0;
  };

  // Applies the given functor to all the keys in this map
  virtual void applyStr(applyStrMapFunc& f)=0;

  class applyMapFunc {
    public:
    // Applied to each Key in a map
    // curVal: contains the current value mapped in AOMap
    virtual void operator()(boost::shared_ptr<void> curVal)=0;
  };

  // Applies the given functor to all the keys in this map
  virtual void apply(applyMapFunc& f)=0;

  class setApplyMapFunc {
    public:
    // Applied to each Key in a map. The value returned for each key is assigned to that key.
    // curVal: contains the current value mapped in AOMap
    virtual boost::shared_ptr<void> operator()(boost::shared_ptr<void> curVal)=0;
  };

  // Applies the given functor to all the keys in this map and sets the value returned by each invocation
  // of the function to the key that corresponds to the call
  virtual void setApply(setApplyMapFunc& f)=0;

  class applyJoinMapFunc {
    public:
    // Applied to all the keys that appear in two map
    // curVal1: contains the current value mapped in the first AOMap
    // curVal2: contains the current value mapped in the second AOMap
    virtual void operator()(boost::shared_ptr<void> curVal1, boost::shared_ptr<void> curVal2)=0;
  };

  // Applies f to the keys shared by this map and that map
  virtual void applyJoin(MAOMapPtr that, applyJoinMapFunc& f)=0;

  class setJoinMapFunc {
    public:
    // Applied to all the keys that appear in two map. The value returned by the function
    // assigned to the current key in the first map
    // curVal1: contains the current value mapped in the first AOMap
    // curVal2: contains the current value mapped in the second AOMap
    virtual boost::shared_ptr<void> operator()(boost::shared_ptr<void> curVal1, boost::shared_ptr<void> curVal2)=0;
  };

  // Applies f to the keys shared by this map and that map. Assigns the return value of each function
  // call to the key for which the function was called
  virtual void setJoin(MAOMapPtr that, setJoinMapFunc& f)=0;

  class setMapObjVecJoinMapFunc {
    public:
    // Applied to all the keys that appear in the map and all the objects in the given vector.
    // The value returned by the function assigned to the current key in the map
    // curMapVal: contains the current value mapped in the this MAOMap
    // curObjVals: vector of the values currently mapped in each object at the current key
    virtual boost::shared_ptr<void> operator()(boost::shared_ptr<void> curMapVal,
                                               const std::vector<AbstractObjectPtr>& curObjVals)=0;
  };

  // Applies f to the keys shared by maps this map and all the objects in objs.
  // Assigns the return value of each function call to the key for which the function was called
  virtual void setObjVecJoin(const std::vector<AbstractionPtr>& objs, setMapObjVecJoinMapFunc& f)=0;

  class setMapObjMapJoinMapFunc {
    public:
    // Applied to all the keys that appear in the three maps. The value returned by the function
    // assigned to the current key in the first map
    // curVal1: contains the current value mapped in the this MAOMap
    // curValObj: contains the current AbstractObject mapped in the Mapped Abstract Object
    // curValThat: contains the current value mapped in the that MAOMap
    virtual boost::shared_ptr<void> operator()(boost::shared_ptr<void> curValThis,
                                               AbstractObjectPtr curValObj,
                                               boost::shared_ptr<void> curValThat)=0;
  };

  // Applies f to the keys shared by maps this map, obj and thatMap. Assigns the return value of each function
  // call to the key for which the function was called
  virtual void setMapObjMapJoin(AbstractionPtr obj, MAOMapPtr thatMap, setMapObjMapJoinMapFunc& f)=0;

  class set3MapJoinMapFunc {
    public:
    // Applied to all the keys that appear in the three maps. The value returned by the function
    // assigned to the current key in the first map
    // curVal1: contains the current value mapped in the this MAOMap
    // curVal2: contains the current value mapped in the MAOMap 1
    // curVal2: contains the current value mapped in the MAOMap 2
    virtual boost::shared_ptr<void> operator()(boost::shared_ptr<void> curValThis,
                                               boost::shared_ptr<void> curValMap1,
                                               boost::shared_ptr<void> curValMap2)=0;
  };

  // Applies f to the keys shared by maps this map and the other two maps. Assigns the return value of each function
  // call to the key for which the function was called
  virtual void set3MapJoin(MAOMapPtr thatMap1, MAOMapPtr thatMap2, set3MapJoinMapFunc& f)=0;

  // Returns a freshly-allocated empty instance of this map type that maps
  // the keys of this map to void pointers
  virtual MAOMapPtr create() const=0;

  // Returns a freshly-allocated copy of this map that maps the same keys to the same values as the originl
  virtual MAOMapPtr copy() const=0;

  // Returns an instance of a Mapped Abstraction that corresponds to the
  // values and keys within this map. It is assumed by this call that the values
  // in this map are a sub-type of Abstraction that is compatible with the map's
  // implementation. Concretely, maps focused on AbstractObjects will have
  // AbstractObject values and maps focused on Lattices will have Lattice values.
  virtual AbstractionPtr getMappedObj(uiType ui)=0;
}; // class MAOMap

class MappedAbstractionBase {
  public:
  uiType ui;

  // The number of contained AOs that are full
  int nFull;

  // The analysis that created this MappedAbstractObject. This is needed to allow this
  // object to call mayEqual, etc. methods on the analysis' composer inside calls
  // to mayEqualAO, etc. Note that it would not be valid to use the composer and analysis
  // from the mayEqual, etc. call that led to the mayEqualAO, etc. call because this info
  // may be different between the original call and the call inside MappedAbstractObject.
  ComposedAnalysis* analysis;

  MappedAbstractionBase(uiType ui, int nFull, ComposedAnalysis* analysis) : ui(ui), nFull(nFull), analysis(analysis) { }

  uiType getUI() const { return ui; }
  bool isUnion() const { return ui==Union; }
  bool isIntersection() const { return ui==Intersection; }

  // Returns an instance of MAOMap that is templated the same as a given instance of MappedAbstractObject
  virtual MAOMapPtr genMappedAOMap()=0;

  // Returns whether all instances of all the AbstractObjects within this MAO form a hierachy
  virtual bool membersIsHierarchy() const=0;
}; // class MappedAbstractionBase

// Key : The type of the key that differentiates the various objects inserted into this map. The less-than operator
//       must be defined for Key to make it possible to use it as a key in STL maps
// KeyIsComposedAnalysis : boolean that indicates whether Key is identical to type ComposedAnalysis or not
//       This makes it possible to write separate code for this special case.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
class MappedAbstractObject: public AOSubType, public MappedAbstractionBase {
  public:
  class ConcreteMAOMap;
  friend class ConcreteMAOMap;

  std::map<Key, AOSubTypePtr > aoMap;

  public:
  MappedAbstractObject(uiType ui, int nFull, ComposedAnalysis* analysis);
  MappedAbstractObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, AOSubTypePtr >& aoMap);
  MappedAbstractObject(const MappedAbstractObject& that);

  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > shared_from_this()
  { return boost::static_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(AbstractObject::shared_from_this()); }

  // Returns true if this is a MappedAbstractObject and false otherwise
  virtual bool isMappedAO();

  SgNode* getBase() const;

  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  AbstractObject::AOType getAOType() const;

  // Allocates a copy of this object and returns a pointer to it
  AOSubTypePtr copyAOType() const;

  void add(Key key, AOSubTypePtr ao, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  const std::map<Key, AOSubTypePtr >& getAOMap() const { return aoMap; }

private:
  //! Helper methods.
  bool mayEqualWithKey(Key key, const std::map<Key, AOSubTypePtr >& thatAOMap, PartEdgePtr pedge,
                       Composer* comp, ComposedAnalysis* analysis);
  bool mustEqualWithKey(Key key, const std::map<Key, AOSubTypePtr >& thatAOMap, PartEdgePtr pedge,
                        Composer* comp, ComposedAnalysis* analysis);
  bool equalSetWithKey(Key key,const std::map<Key, AOSubTypePtr >& thatAOMap, PartEdgePtr pedge,
                       Composer* comp, ComposedAnalysis* analysis);
  bool subSetWithKey(Key key,const std::map<Key, AOSubTypePtr >& thatAOMap, PartEdgePtr pedge,
                     Composer* comp, ComposedAnalysis* analysis);

public:
//  // Returns whether this object may/must be equal to o within the given Part p
//  // These methods are private to prevent analyses from calling them directly.
  bool mayEqualAO(AOSubTypePtr o, PartEdgePtr pedge);
  bool mustEqualAO(AOSubTypePtr o, PartEdgePtr pedge);
  // Returns whether this object may/must be equal to o within the given Part p
  virtual bool mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);
  virtual bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetAO(AOSubTypePtr o, PartEdgePtr pedge);
  // general versions of equalset() that accounts for framework details before routing the call to the
  // derived class' equalset() check. specifically, it routes the call through the composer to make
  // sure the equalset() call gets the right partedge.
  virtual bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetAO(AOSubTypePtr o, PartEdgePtr pedge);

  // General versions of subSet() that accounts for framework details before routing the call to the
  // derived class' subSet() check. Specifically, it routes the call through the composer to make
  // sure the subSet() call gets the right PartEdge.
  virtual bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Returns true if this object is live at the given part and false otherwise
  bool isLiveAO(PartEdgePtr pedge);
  bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateAO(AOSubTypePtr that, PartEdgePtr pedge);
  virtual bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  void setAOToFull();

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);
  virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  AOSubTypePtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();

  std::string str(std::string indent="") const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const;

  // Returns whether all instances of all the AbstractObjects within this MAO form a hierachy
  bool membersIsHierarchy() const;

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const;

  protected:
  // Maps the keys of this object to the hierarchical keys that describe its sub-objects
  // (in the case where they're all hierarhical)
  class MappedPartialOrderKey: public AbstractionPartialOrder::PartialOrderKey {
    public:
    std::map<Key, typename AOSubType::hierKeyPtr> key;

    // If isPartialOrder() returns true, classes must implement the methods below to establish
    // the partial order relationship among class instances.
    bool isPOLessThan(const AbstractionPartialOrder::PartialOrderKey& that);
    bool isPOEqual(const AbstractionPartialOrder::PartialOrderKey& that);
  }; // class MappedPartialOrderKey

  boost::shared_ptr<MappedPartialOrderKey> partialOrderKey;
  bool initializedPOKey;

  public:

  // Returns whether sets of the given type form a partial order
  bool isPartialOrder();

  // Computes the partial order key that describes this object and stores it in partialOrderKey
  void computePartialOrderKey();

  // Returns the PartialOrderKey object that defines this Abstraction's location within its partial order
  AbstractionPartialOrder::PartialOrderKeyPtr getPOKey();

  // Concrete implementation that uses a specific key type
  class ConcreteMAOMap : public MAOMap {
    std::map<Key, boost::shared_ptr<void> > data;
    boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > parent;
    public:

    // Creates an empty map with the same keys as the given MappedAbstractObject
    ConcreteMAOMap(boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > parent);

    // Creates a new map based on the given map:
    // If emptyMap==true: the new map will be created empty but initialized with the keys of the given map
    // Otherwise: the new map will be a shallow copy of the given map (the copy methods of the keys and values are not called)
    ConcreteMAOMap(const ConcreteMAOMap& that, bool emptyMap);

    void set(AbstractionPtr mappedAO, setMapFunc& f);
    void get(AbstractionPtr mappedAO, getMapFunc& f);

    // Applies the given functor to all the keys in this map
    void applyStr(applyStrMapFunc& f);

    // Applies the given functor to all the keys in this map
    void apply(applyMapFunc& f);

    // Applies the given functor to all the keys in this map and sets the value returned by each invocation
    // of the function to the key that corresponds to the call
    void setApply(setApplyMapFunc& f);

    // Applies f to the keys shared by this map and that map
    void applyJoin(MAOMapPtr that, applyJoinMapFunc& f);

    // Applies f to the keys shared by this map and that map. Assigns the return value of each function
    // call to the key for which the function was called
    void setJoin(MAOMapPtr that, setJoinMapFunc& f);

    // Applies f to the keys shared by maps this map and all the objects in objs.
    // Assigns the return value of each function call to the key for which the function was called
    void setObjVecJoin(const std::vector<AbstractionPtr>& objs, setMapObjVecJoinMapFunc& f);

    // Applies f to the keys shared by maps this map, obj and thatMap. Assigns the return value of each function
    // call to the key for which the function was called
    void setMapObjMapJoin(AbstractionPtr obj, MAOMapPtr thatMap, setMapObjMapJoinMapFunc& f);

    // Applies f to the keys shared by maps this map and the other two maps. Assigns the return value of each function
    // call to the key for which the function was called
    void set3MapJoin(MAOMapPtr thatMap1, MAOMapPtr thatMap2, set3MapJoinMapFunc& f);

    // Returns a freshly-allocated empty instance of this map type that maps
    // the keys of this map to void pointers
    MAOMapPtr create() const;

    // Returns a freshly-allocated copy of this map that maps the same keys to the same values as the originl
    MAOMapPtr copy() const;

    // Returns an instance of a Mapped Abstraction that corresponds to the
    // values and keys within this map. It is assumed by this call that the values
    // in this map are a sub-type of Abstraction that is compatible with the map's
    // implementation. Concretely, maps focused on AbstractObjects will have
    // AbstractObject values and maps focused on Lattices will have Lattice values.
    AbstractionPtr getMappedObj(uiType ui);
  };

  // Returns an instance of MAOMap that is templated the same as a given instance of this MappedAbstractObject
  MAOMapPtr genMappedAOMap();
}; // class MappedAbstractObject

/* #########################
   ##### CodeLocObject #####
   ######################### */

class CodeLocObject;
typedef boost::shared_ptr<CodeLocObject> CodeLocObjectPtr;
//typedef boost::shared_ptr<const CodeLocObject> ConstCodeLocObjectPtr;
extern CodeLocObjectPtr NULLCodeLocObject;

// CodeLocs denote specific locations in the application code, as a part in the server-
// provided ATS graph and a CFGNode within it
class CodeLocObject : public AbstractObject
{
  protected:
  // Denote a specific CFGNode in a specific part.
  // If part==NULLPart, it denotes all Parts in the ATS graph.
  // If cfgNode.getNode()==NULL it denotes all CFGNodes in a part;
  //         if part=NULLPart, it denotes all CFGNodes in the application.
  // NOTE: Instead of using NULL to denote sets we can also explicitly enumerate set members
  //       since there is a finite number of ATS graph nodes and CFGNodes. We'll need to
  //       evaluate whether this approach is cost-effective.
  PartPtr part;
  CFGNode cfgNode;

  public:
  //CodeLocObject(SgNode* base) : AbstractObject(base) {}
  CodeLocObject(SgNode* base) : AbstractObject(base) { assert(0); }
  CodeLocObject(PartPtr part, CFGNode cfgNode) : AbstractObject(NULL), part(part), cfgNode(cfgNode) {
    // If part denotes all parts in the ATS graph, set cfgNode to denote all CFGNodes
    if(part==NULLPart) this->cfgNode = CFGNode();
    // Otherwise, use the provided cfgNode
    else               this->cfgNode = cfgNode;
  }
  CodeLocObject(const CodeLocObject& that) : AbstractObject(that), part(that.part), cfgNode(that.cfgNode) {}

  // Wrapper for shared_from_this that returns an instance of this class rather than its parent
  CodeLocObjectPtr shared_from_this() { return boost::static_pointer_cast<CodeLocObject>(AbstractObject::shared_from_this()); }

  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  bool isValueObject()      { return false; }
  bool isCodeLocObject()    { return true; }
  bool isMemRegionObject()  { return false;  }
  bool isMemLocObject()     { return false; }
  AOType getAOType() const { return AbstractObject::CodeLoc; }

  PartPtr getPart()    const { return part; }
  CFGNode getCFGNode() const { return cfgNode; }

  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  virtual bool mayEqualAO(CodeLocObjectPtr o, PartEdgePtr pedge);
  virtual bool mustEqualAO(CodeLocObjectPtr o, PartEdgePtr pedge);

public:
  // General version of mayEqual and mustEqual that implements may/must equality with respect to ExprObj
  // and uses the derived class' may/mustEqual check for all the other cases
  // GREG: Currently nothing interesting here since we don't support ExprObjs for CodeLocObjects
  virtual bool mayEqual(CodeLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual(CodeLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  virtual bool mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  virtual bool equalSetAO(CodeLocObjectPtr o, PartEdgePtr pedge);
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  virtual bool subSetAO(CodeLocObjectPtr o, PartEdgePtr pedge);

public:
  // General version of equalSet and subSet that implements may/must equality with respect to ExprObj
  // and uses the derived class' may/mustEqual check for all the other cases
  // GREG: Currently nothing interesting here since we don't support ExprObjs for CodeLocObjects
  virtual bool equalSet(CodeLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool subSet(CodeLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  virtual bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

//private:
  // Returns true if this object is live at the given part and false otherwise.
  // This method is called by composers and should not be called by analyses.
  virtual bool isLiveAO(PartEdgePtr pedge);

public:
  // General version of isLive that accounts for framework details before routing the call to the derived class'
  // isLiveAO check. Specifically, it routes the call through the composer to make sure the isLiveAO call gets the
  // right PartEdge
  virtual bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdateAO(CodeLocObjectPtr that, PartEdgePtr pedge);

  // General version of meetUpdate that accounts for framework details before routing the call to the derived class'
  // meetUpdateAO check. Specifically, it routes the call through the composer to make sure the meetUpdateAO
  // call gets the right PartEdge
  virtual bool meetUpdate(CodeLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isEmptyAO(PartEdgePtr pedge);

  // General versions of isFull() and isEmpty that account for framework details before routing the call to the
  // derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make
  // sure the isFullAO() and isEmptyAO() call gets the right PartEdge.
  // These functions are just aliases for the real implementations in AbstractObject
  virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);
  virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Allocates a copy of this object and returns a pointer to it
  virtual CodeLocObjectPtr copyAOType() const=0;
  AbstractObjectPtr copyAO() const;

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  virtual CodeLocObjectPtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);

  virtual std::string str(std::string indent="") const; // pretty print for the object
};

/* #############################
   ##### FullCodeLocObject #####
   ############################# */

//! NOTE:Its sufficient to create only a single instance of this object globally.
class FullCodeLocObject : public CodeLocObject
{
  public:
  FullCodeLocObject() : CodeLocObject(NULL) { }

  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are private to prevent analyses from calling them directly.
  bool mayEqualAO(CodeLocObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(CodeLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetAO(CodeLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetAO(CodeLocObjectPtr o, PartEdgePtr pedge);

  // Returns true if this object is live at the given part and false otherwise
  bool isLiveAO(PartEdgePtr pedge);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateAO(CodeLocObjectPtr that, PartEdgePtr pedge);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();

  // Allocates a copy of this object and returns a pointer to it
  CodeLocObjectPtr copyAOType() const;

  std::string str(std::string indent="") const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const {
    // The full object has no way to know. May need to change return value to allow dont-know
    ROSE_ASSERT(0);
  }

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const hierKeyPtr& getHierKey() const
  // The key of a full object is empty, so return the cachedHierKey, which is guaranteed to be empty
  { return cachedHierKey; }
};

/* #############################
   #### MappedCodeLocObject ####
   ############################# */

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType>
class MappedCodeLocObject : public MappedAbstractObject<Key, KeyIsComposedAnalysis, CodeLocObject, CodeLocObjectPtr, AbstractObject::CodeLoc, MappedAOSubType>
{
public:
  MappedCodeLocObject(uiType ui, ComposedAnalysis* analysis) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, CodeLocObject, CodeLocObjectPtr, AbstractObject::CodeLoc, MappedAOSubType>(ui, /*nFull*/ 0, analysis)
  {}
  MappedCodeLocObject(uiType ui, ComposedAnalysis* analysis, const std::map<Key, CodeLocObjectPtr>& aoMap) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, CodeLocObject, CodeLocObjectPtr, AbstractObject::CodeLoc, MappedAOSubType>(ui, /*nFull*/ 0, analysis, aoMap)
  {}
  MappedCodeLocObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, CodeLocObjectPtr>& aoMap) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, CodeLocObject, CodeLocObjectPtr, AbstractObject::CodeLoc, MappedAOSubType>(ui, nFull, analysis, aoMap)
  {}
  MappedCodeLocObject(const MappedCodeLocObject& that) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, CodeLocObject, CodeLocObjectPtr, AbstractObject::CodeLoc, MappedAOSubType>(that)
  {}
}; // class MappedCodeLocObject

template<class Key, bool KeyIsComposedAnalysis>
class GenericMappedCodeLocObject : public MappedCodeLocObject<Key, KeyIsComposedAnalysis,
                                                              GenericMappedCodeLocObject<Key, KeyIsComposedAnalysis> > {
  public:
  GenericMappedCodeLocObject(uiType ui, ComposedAnalysis* analysis) : MappedCodeLocObject<Key, KeyIsComposedAnalysis, GenericMappedCodeLocObject<Key, KeyIsComposedAnalysis> >(ui, analysis) {}
  GenericMappedCodeLocObject(uiType ui, ComposedAnalysis* analysis, const std::map<Key, CodeLocObjectPtr>& aoMap) : MappedCodeLocObject<Key, KeyIsComposedAnalysis, GenericMappedCodeLocObject<Key, KeyIsComposedAnalysis> >(ui, analysis, aoMap) {}
  GenericMappedCodeLocObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, CodeLocObjectPtr>& aoMap) : MappedCodeLocObject<Key, KeyIsComposedAnalysis, GenericMappedCodeLocObject<Key, KeyIsComposedAnalysis> >(ui, nFull, analysis, aoMap) {}
  GenericMappedCodeLocObject(const GenericMappedCodeLocObject& that) : MappedCodeLocObject<Key, KeyIsComposedAnalysis, GenericMappedCodeLocObject<Key, KeyIsComposedAnalysis> >(that) {}
};

extern template class GenericMappedCodeLocObject<ComposedAnalysis*, true>;
extern template class MappedCodeLocObject<ComposedAnalysis*, true, GenericMappedCodeLocObject<ComposedAnalysis*, true> >;
extern template class MappedAbstractObject<ComposedAnalysis*, true, CodeLocObject, CodeLocObjectPtr, AbstractObject::CodeLoc, GenericMappedCodeLocObject<ComposedAnalysis*, true> >;
typedef boost::shared_ptr<GenericMappedCodeLocObject<ComposedAnalysis*, true> > AnalMapCodeLocObjectPtr;
typedef GenericMappedCodeLocObject<ComposedAnalysis*, true> AnalMapCodeLocObject;

/* ##############################
   # PartEdgeUnionCodeLocObject #
   ############################## */

//! Special CodeLocObject to union CodeLocObject from different PartEdges.
//! CodeLocObjects should be of same type i.e., from the same analysis.
class PartEdgeUnionCodeLocObject : public CodeLocObject {
  CodeLocObjectPtr unionCL_p;
public:
  PartEdgeUnionCodeLocObject();
  PartEdgeUnionCodeLocObject(const PartEdgeUnionCodeLocObject& that);
  SgNode* getBase() const;
  void add(CodeLocObjectPtr cl_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  CodeLocObjectPtr getUnionAO() { return unionCL_p; }

  bool mayEqualAO(CodeLocObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(CodeLocObjectPtr o, PartEdgePtr pedge);
  bool equalSetAO(CodeLocObjectPtr o, PartEdgePtr pedge);
  bool subSetAO(CodeLocObjectPtr o, PartEdgePtr pedge);
  bool isLiveAO(PartEdgePtr pedge);
  bool meetUpdateAO(CodeLocObjectPtr that, PartEdgePtr pedge);
  bool isFullAO(PartEdgePtr pedge);
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();

  CodeLocObjectPtr copyAOType() const;
  void setAOToFull();

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  virtual CodeLocObjectPtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);

  std::string str(std::string indent="") const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const;

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const;

  // ----------------------------------------
  // Objects that denote disjoint sets. Because no two sets may overlap, they can be
  // represented using unique numbers, which enables efficient data structure implementations.
  bool isDisjoint() const;
  // AbstractObjects that form a hierarchy must inherit from the AbstractObjectDisjoint class
};


/* #######################
   ##### ValueObject #####
   ####################### */

class ValueObject;
typedef boost::shared_ptr<ValueObject> ValueObjectPtr;
//typedef boost::shared_ptr<const ValueObject> ConstValueObjectPtr;
extern ValueObjectPtr NULLValueObject;

class ValueObject : public AbstractObject
{
  std::map<ValueObjectPtr, bool> mayEqualCache;
  std::map<ValueObjectPtr, bool> mustEqualCache;
  std::map<ValueObjectPtr, bool> equalSetCache;
  std::map<ValueObjectPtr, bool> subSetCache;

  public:
  ValueObject() {}
  ValueObject(SgNode* base) : AbstractObject(base) {}
  ValueObject(const ValueObject& that) : AbstractObject(that) {}

  // Wrapper for shared_from_this that returns an instance of this class rather than its parent
  ValueObjectPtr shared_from_this() { return boost::static_pointer_cast<ValueObject>(AbstractObject::shared_from_this()); }

  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  bool isValueObject()     { return true; }
  bool isCodeLocObject()   { return false; }
  bool isMemRegionObject() { return false;  }
  bool isMemLocObject()    { return false; }
  AOType getAOType() const { return AbstractObject::CodeLoc; }

//private:
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  virtual bool mayEqualAO(ValueObjectPtr o, PartEdgePtr pedge)=0;
  virtual bool mustEqualAO(ValueObjectPtr o, PartEdgePtr pedge)=0;

public:

  // Returns whether this object may/must be equal to o within the given Part p
  // by propagating the call through the composer
  virtual bool mayEqual(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  virtual bool mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  virtual bool equalSetAO(ValueObjectPtr o, PartEdgePtr pedge)=0;
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  virtual bool subSetAO(ValueObjectPtr o, PartEdgePtr pedge)=0;

public:
  // General version of mayEqual and mustEqual that implements may/must equality with respect to ExprObj
  // and uses the derived class' may/mustEqual check for all the other cases
  // GREG: Currently nothing interesting here since we don't support ExprObjs for ValueObjects
  virtual bool equalSet(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool subSet(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  virtual bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

//private:
  // Returns true if this object is live at the given part and false otherwise.
  // This method is called by composers and should not be called by analyses.
  // NOTE: we do not currently allow ValueObjects to implement an isLive methods because we assume that they'll always be live
  virtual bool isLiveAO(PartEdgePtr pedge) { return true; }

public:
  // Returns true if this object is live at the given part and false otherwise
  // NOTE: we currently assume that ValueObjects are always live
  virtual bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) { return true; }

  // Computes the meet of this and that and saves the result in this.
  // Returns true if this causes this to change and false otherwise.
  virtual bool meetUpdateAO(ValueObjectPtr that, PartEdgePtr pedge)=0;

  // General version of meetUpdate that accounts for framework details before routing the call to the derived class'
  // meetUpdateV check. Specifically, it routes the call through the composer to make sure the meetUpdateV
  // call gets the right PartEdge
  virtual bool meetUpdate(ValueObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isEmptyAO(PartEdgePtr pedge)=0;
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isFullAO(PartEdgePtr pedge)=0;

  // General version of isFull/isEmpty that accounts for framework details before routing the call to the
  // derived class' isFullV/isEmptyV check. Specifically, it routes the call through the composer to make
  // sure the isFullV/isEmptyV call gets the right PartEdge
  virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns the type of the concrete value (if there is one)
  virtual SgType* getConcreteType()=0;
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  virtual std::set<boost::shared_ptr<SgValueExp> > getConcreteValue()=0;

  // Returns true if this SgValueExp is convertible into a boolean
  static bool isValueBoolCompatible(boost::shared_ptr<SgValueExp> val);

  // Convert the value of the given SgValueExp, cast to a boolean
  static bool SgValue2Bool(boost::shared_ptr<SgValueExp> val);

  // Returns true if the two SgValueExps correspond to the same value when cast to the given type (if t!=NULL)
  static bool equalValueExp(SgValueExp* e1, SgValueExp* e2, SgType* t=NULL);

  // GB 2012-09-26 : Do we need to have AbstractTypeObjects to represent uncertainty about the type?
  //                 How can we support type uncertainly for MemLocObjects?

  // Allocates a copy of this object and returns a pointer to it
  virtual ValueObjectPtr copyAOType() const=0;
  AbstractObjectPtr copyAO() const;

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  virtual ValueObjectPtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);
}; // class ValueObject

// The default implementation of ValueObjects that denotes the set of all ValueObjects
//! NOTE:Its sufficient to create only a single instance of this object globally.
class FullValueObject : public ValueObject
{
  public:
  FullValueObject() {}

  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are private to prevent analyses from calling them directly.
  bool mayEqualAO(ValueObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(ValueObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetAO(ValueObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetAO(ValueObjectPtr o, PartEdgePtr pedge);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateAO(ValueObjectPtr that, PartEdgePtr pedge);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Allocates a copy of this object and returns a pointer to it
  ValueObjectPtr copyAOType() const;

  std::string str(std::string indent="") const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const {
    // The full object has no way to know. May need to change return value to allow dont-know
    ROSE_ASSERT(0);
  }

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const
  // The key of a full object is empty, so return the cachedHierKey, which is guaranteed to be empty
  { return cachedHierKey; }
};

/* ###########################
   #### MappedValueObject ####
   ########################### */

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType>
class MappedValueObject : public MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>
{
  public:
  MappedValueObject(uiType ui, ComposedAnalysis* analysis) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>(ui, /*nFull*/ 0, analysis)
  {}
  MappedValueObject(uiType ui, ComposedAnalysis* analysis, const std::map<Key, ValueObjectPtr>& aoMap) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>(ui, /*nFull*/ 0, analysis, aoMap)
  {}
  MappedValueObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, ValueObjectPtr>& aoMap) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>(ui, nFull, analysis, aoMap)
  {}
  MappedValueObject(const MappedValueObject& that) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>(that)
  {}

  // Type of concrete value
  SgType* getConcreteType();

  // set of values that are enumerable
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();
}; // class MappedValueObject

template<class Key, bool KeyIsComposedAnalysis>
class GenericMappedValueObject : public MappedValueObject<Key, KeyIsComposedAnalysis,
                                                          GenericMappedValueObject<Key, KeyIsComposedAnalysis> > {
  public:
  GenericMappedValueObject(uiType ui, ComposedAnalysis* analysis) : MappedValueObject<Key, KeyIsComposedAnalysis, GenericMappedValueObject<Key, KeyIsComposedAnalysis> >(ui, analysis) {}
  GenericMappedValueObject(uiType ui, ComposedAnalysis* analysis, const std::map<Key, ValueObjectPtr>& aoMap) : MappedValueObject<Key, KeyIsComposedAnalysis, GenericMappedValueObject<Key, KeyIsComposedAnalysis> >(ui, analysis, aoMap) {}
  GenericMappedValueObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, ValueObjectPtr>& aoMap) : MappedValueObject<Key, KeyIsComposedAnalysis, GenericMappedValueObject<Key, KeyIsComposedAnalysis> >(ui, nFull, analysis, aoMap) {}
  GenericMappedValueObject(const GenericMappedValueObject<Key, KeyIsComposedAnalysis>& that) : MappedValueObject<Key, KeyIsComposedAnalysis, GenericMappedValueObject<Key, KeyIsComposedAnalysis> >(that) {}
};

extern template class GenericMappedValueObject<ComposedAnalysis*, true>;
extern template class MappedValueObject<ComposedAnalysis*, true, GenericMappedValueObject<ComposedAnalysis*, true> >;
extern template class MappedAbstractObject<ComposedAnalysis*, true, ValueObject, ValueObjectPtr, AbstractObject::Value, GenericMappedValueObject<ComposedAnalysis*, true> >;
typedef boost::shared_ptr<GenericMappedValueObject<ComposedAnalysis*, true> > AnalMapValueObjectPtr;
typedef GenericMappedValueObject<ComposedAnalysis*, true> AnalMapValueObject;

/* ############################
   # PartEdgeUnionValueObject #
   ############################ */

//! Special ValueObject to union ValueObject from different PartEdges.
//! ValueObjects should be of same type i.e., from the same analysis.
class PartEdgeUnionValueObject : public ValueObject {
  ValueObjectPtr unionV_p;
public:
  PartEdgeUnionValueObject();
  PartEdgeUnionValueObject(const PartEdgeUnionValueObject& that);
  SgNode* getBase() const;
  void add(ValueObjectPtr v_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  ValueObjectPtr getUnionAO() { return unionV_p; }

  bool mayEqualAO(ValueObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(ValueObjectPtr o, PartEdgePtr pedge);
  bool equalSetAO(ValueObjectPtr o, PartEdgePtr pedge);
  bool subSetAO(ValueObjectPtr o, PartEdgePtr pedge);
  bool isLiveAO(PartEdgePtr pedge);
  bool meetUpdateAO(ValueObjectPtr that, PartEdgePtr pedge);
  bool isFullAO(PartEdgePtr pedge);
  bool isEmptyAO(PartEdgePtr pedge);
  ValueObjectPtr copyAOType() const;
  void setAOToFull();
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  SgType* getConcreteType();
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  virtual ValueObjectPtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);

  std::string str(std::string indent="") const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const;

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const;

  // ----------------------------------------
  // Objects that denote disjoint sets. Because no two sets may overlap, they can be
  // represented using unique numbers, which enables efficient data structure implementations.
  bool isDisjoint() const;
  // AbstractObjects that form a hierarchy must inherit from the AbstractObjectDisjoint class
}; // class PartEdgeUnionValueObject


/* ###########################
   ##### MemRegionObject #####
   ###########################

Denote sets of contiguous memory regions. In the memory model of C/C++ and most other languages
memory is divided into disjoint contiguous regions of memory with no constraints on their
adresses. For instance, two malloc-ed buffers are guaranteed to be disjoint but may be placed
anywhere in memory. The same is true for the memory regions that hold different function local
variables. Although their lifetime is defined, their absolute or relative memory locations are not.
Memory regions denote such buffers, providing a way to distinguish them from each other and
identify their sizes but not learn any additional information about their locations.
*/

// Major types of abstract objects
class MemRegionObject;
typedef boost::shared_ptr<MemRegionObject> MemRegionObjectPtr;
//typedef boost::shared_ptr<const MemRegionObject> ConstMemRegionObjectPtr;
extern MemRegionObjectPtr NULLMemRegionObject;

class MemRegionObject : public AbstractObject
{
  std::map<MemRegionObjectPtr, bool> mayEqualCache;
  std::map<MemRegionObjectPtr, bool> mustEqualCache;
  std::map<MemRegionObjectPtr, bool> equalSetCache;
  std::map<MemRegionObjectPtr, bool> subSetCache;

public:
//  MemRegionObject() {}
  //# SA
  // should the default mutable value be conservatively true ?
  MemRegionObject(SgNode* base) : AbstractObject(base) {}
  MemRegionObject(const MemRegionObject& that) : AbstractObject(that) {}

  // Wrapper for shared_from_this that returns an instance of this class rather than its parent
  MemRegionObjectPtr shared_from_this() { return boost::static_pointer_cast<MemRegionObject>(AbstractObject::shared_from_this()); }

  // Returns the relationship between the given AbstractObjects, considering whether either
  // or both are FuncResultMemRegionObjects and if they refer to the same function
  FuncResultRelationType getFuncResultRel(AbstractObject* one, AbstractObjectPtr two, PartEdgePtr pedge);

  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  bool isValueObject()      { return false; }
  bool isCodeLocObject()    { return false; }
  bool isMemRegionObject()  { return true;  }
  bool isMemLocObject()     { return false; }
  AOType getAOType() const { return AbstractObject::MemRegion; }

//private:
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  virtual bool mayEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge)=0;
  virtual bool mustEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge)=0;

public:
  // General version of mayEqual and mustEqual that accounts for framework details before routing the call to the
  // derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
  // the call through the composer to make sure the may/mustEqual call gets the right PartEdge
  virtual bool mayEqual (MemRegionObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual(MemRegionObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Check whether that is a MemRegionObject and if so, call the version of may/mustEqual specific to MemRegionObjects
  virtual bool mayEqual (AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

//private:
  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  virtual bool equalSetAO(MemRegionObjectPtr o, PartEdgePtr pedge)=0;
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  virtual bool subSetAO(MemRegionObjectPtr o, PartEdgePtr pedge)=0;

public:
  // General version of equalSet and subSet that implements may/must equality with respect to ExprObj
  // and uses the derived class' may/mustEqual check for all the other cases
  // GREG: Currently nothing interesting here since we don't support ExprObjs for MemoryRegionObjects
  virtual bool equalSet(MemRegionObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool subSet  (MemRegionObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  virtual bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool subSet  (AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns true if this object is live at the given part and false otherwise
  virtual bool isLiveAO(PartEdgePtr pedge)=0;
public:
  //MemRegionObjectPtr getThis();
  // General version of isLive that accounts for framework details before routing the call to the derived class'
  // isLiveMR check. Specifically, it routes the call through the composer to make sure the isLiveML call gets the
  // right PartEdge
  virtual bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

// private:
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdateAO(MemRegionObjectPtr that, PartEdgePtr pedge)=0;

  // General version of meetUpdate that accounts for framework details before routing the call to the derived class'
  // meetUpdateMR check. Specifically, it routes the call through the composer to make sure the meetUpdateMR
  // call gets the right PartEdge
  virtual bool meetUpdate(MemRegionObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isEmptyAO(PartEdgePtr pedge)=0;
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isFullAO(PartEdgePtr pedge)=0;

  // General version of isFull/isEmpty that accounts for framework details before routing the call to the
  // derived class' isFullMR/isEmptyMR check. Specifically, it routes the call through the composer to make
  // sure the isFullMR/isEmptyMR call gets the right PartEdge
  virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns the type of the concrete regions (if there is one)
  virtual SgType* getConcreteType()=0;
  // Returns the set of concrete memory regions as SgExpressions, which allows callers to use
  // the normal ROSE mechanisms to decode it
  virtual std::set<SgNode* > getConcrete()=0;

  // Returns a ValueObject that denotes the size of this memory region
  virtual ValueObjectPtr getRegionSizeAO(PartEdgePtr pedge)=0;

  // General version of getRegionSize that accounts for framework details before routing the call to the
  // derived class' getRegionSizeAO(). Specifically, it routes the call through the composer to make
  // sure the getRegionSize call gets the right PartEdge
  virtual ValueObjectPtr getRegionSize(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Allocates a copy of this object and returns a pointer to it
  virtual MemRegionObjectPtr copyAOType() const=0;
  AbstractObjectPtr copyAO() const
  { return copyAOType(); }

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  virtual MemRegionObjectPtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);
}; // class MemRegionObject

// Special MemRegionObject used internally by the framework to associate with the return value of a function
class FuncResultMemRegionObject : public MemRegionObject/*, public AbstractionHierarchy*/
{
  // Special type of comparable object that is only equal to other instances of its type.
  // This makes it possible to use the generic FuncResultMemLocObject together with other
  // objects implemented by individual analyses while ensuring that a FuncResultMemLocObject
  // is always considered to be disjoint from the others
  class FuncResultMemRegionObject_comparable: public comparable {
    public:
    bool equal(const comparable& that) const;
    bool less(const comparable& that) const;
    std::string str(std::string indent="") const { return "FuncResultMemRegionObject"; }
  }; // class FuncResultMemRegionObject_comparable


  Function func;
  public:
  FuncResultMemRegionObject(Function func);

  // Returns whether this object may/must be equal to o within the given Part p
  bool mayEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetAO(MemRegionObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetAO(MemRegionObjectPtr o, PartEdgePtr pedge);

  // Returns true if this object is live at the given part and false otherwise.
  // This method is called by composers and should not be called by analyses.
  bool isLiveAO(PartEdgePtr pedge) { return true; }

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateAO(MemRegionObjectPtr that, PartEdgePtr pedge);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns true if this MemRegionObject denotes a finite set of concrete regions
  bool isConcrete() { return true; }
  // Returns the number of concrete values in this set
  int concreteSetSize() { return 1; }
  // Returns the type of the concrete regions (if there is one)
  SgType* getConcreteType() { return func.get_type(); }
  // Returns the set of concrete memory regions as SgExpressions, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<SgNode*> getConcrete() { std::set<SgNode* > ret; ret.insert(func.get_definition()); return ret; }

  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr getRegionSizeAO(PartEdgePtr pedge);

  std::string str(std::string indent="") const { return "FuncResultMemRegionObject"; }
  std::string strp(PartEdgePtr pedge, std::string indent="") { return "FuncResultMemRegionObject"; }

  // Allocates a copy of this object and returns a pointer to it
  MemRegionObjectPtr copyAOType() const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const {
    // The full object has no way to know. May need to change return value to allow dont-know
    ROSE_ASSERT(0);
  }

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const;
};
typedef boost::shared_ptr<FuncResultMemRegionObject> FuncResultMemRegionObjectPtr;

//! Default implementation of MemRegionObject that denotes all MemRegionObject
//! Composers use the objects to conservatively answer queries
//! Analyses should never see these objects
//! NOTE:Its sufficient to create only a single instance of this object globally.
class FullMemRegionObject : public MemRegionObject/*, public AbstractionHierarchy*/
{
 public:
  FullMemRegionObject() : MemRegionObject(NULL) { }

  //! Returns whether this object may/must be equal to o within the given Part p
  bool mayEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetAO(MemRegionObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted                                                                                           // by the given abstract object.
  bool subSetAO(MemRegionObjectPtr o, PartEdgePtr pedge);

  // Allocates a copy of this object and returns a pointer to it
  MemRegionObjectPtr copyAOType() const;
  // Returns true if this object is live at the given part and false otherwise
  bool isLiveAO(PartEdgePtr pedge);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateAO(MemRegionObjectPtr that, PartEdgePtr pedge);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);

  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns true if this MemRegionObject denotes a finite set of concrete regions
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete regions (if there is one)
  SgType* getConcreteType() { return NULL; }
  // Returns the set of concrete memory regions as SgExpressions, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<SgNode* > getConcrete() { std::set<SgNode* > empty; return empty; }

  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr getRegionSizeAO(PartEdgePtr pedge);

  std::string str(std::string indent="") const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const {
    // The full object has no way to know. May need to change return value to allow dont-know
    ROSE_ASSERT(0);
  }

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const
  // The key of a full object is empty, so return the cachedHierKey, which is guaranteed to be empty
  { return cachedHierKey; }
};

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
class MappedMemRegionObject : public MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>
{
  public:
  MappedMemRegionObject(uiType ui, ComposedAnalysis* analysis) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>(ui, /*nFull*/ 0, analysis)
  {}
  MappedMemRegionObject(uiType ui, ComposedAnalysis* analysis, const std::map<Key, MemRegionObjectPtr>& aoMap) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>(ui, /*nFull*/ 0, analysis, aoMap)
  {}
  MappedMemRegionObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, MemRegionObjectPtr>& aoMap) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>(ui, nFull, analysis, aoMap)
  {}
  MappedMemRegionObject(const MappedMemRegionObject& that) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>(that)
  {}

  // Returns the type of the concrete regions (if there is one)
  SgType* getConcreteType();
  // Returns the set of concrete memory regions as SgExpressions, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<SgNode* > getConcrete();

  ValueObjectPtr getRegionSizeAO(PartEdgePtr pedge);

  // Check whether that is a MemLocObject and if so, call the version of mayEqual specific to MemLocObjects
  bool mayEqual(AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis);

  // Check whether that is a MemLocObject and if so, call the version of mustEqual specific to MemLocObjects
  bool mustEqual(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSet(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSet(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // General version of meetUpdate() that accounts for framework details before routing the call to the derived class'
  // meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML
  // call gets the right PartEdge
  bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis);
}; // class MappedMemRegionObject

template<class Key, bool KeyIsComposedAnalysis>
class GenericMappedMemRegionObject : public MappedMemRegionObject<Key, KeyIsComposedAnalysis,
                                                                  GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis>,
                                                                  GenericMappedValueObject<Key, KeyIsComposedAnalysis> > {
  public:
  GenericMappedMemRegionObject(uiType ui, ComposedAnalysis* analysis) : MappedMemRegionObject<Key, KeyIsComposedAnalysis, GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis>, GenericMappedValueObject<Key, KeyIsComposedAnalysis> >(ui, analysis) {}
  GenericMappedMemRegionObject(uiType ui, ComposedAnalysis* analysis, const std::map<Key, MemRegionObjectPtr>& aoMap) : MappedMemRegionObject<Key, KeyIsComposedAnalysis, GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis>, GenericMappedValueObject<Key, KeyIsComposedAnalysis> >(ui, analysis, aoMap) {}
  GenericMappedMemRegionObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, MemRegionObjectPtr>& aoMap) : MappedMemRegionObject<Key, KeyIsComposedAnalysis, GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis>, GenericMappedValueObject<Key, KeyIsComposedAnalysis> >(ui, nFull, analysis, aoMap) {}
  GenericMappedMemRegionObject(const GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis>& that) : MappedMemRegionObject<Key, KeyIsComposedAnalysis, GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis>, GenericMappedValueObject<Key, KeyIsComposedAnalysis>  >(that) {}
};

extern template class GenericMappedMemRegionObject<ComposedAnalysis*, true>;
extern template class MappedMemRegionObject<ComposedAnalysis*, true, GenericMappedMemRegionObject<ComposedAnalysis*, true>, GenericMappedValueObject<ComposedAnalysis*, true> >;
extern template class MappedAbstractObject<ComposedAnalysis*, true, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, GenericMappedMemRegionObject<ComposedAnalysis*, true> >;
typedef boost::shared_ptr<GenericMappedMemRegionObject<ComposedAnalysis*, true> > AnalMapMemRegionObjectPtr;
typedef GenericMappedMemRegionObject<ComposedAnalysis*, true> AnalMapMemRegionObject;

/* ################################
   # PartEdgeUnionMemRegionObject #
   ################################ */

//! Special MemRegionObject to union MemRegionObject from different PartEdges.
//! MemRegionObjects should be of same type i.e., from the same analysis.
class PartEdgeUnionMemRegionObject : public MemRegionObject {
  MemRegionObjectPtr unionMR_p;
public:
  PartEdgeUnionMemRegionObject();
  PartEdgeUnionMemRegionObject(const PartEdgeUnionMemRegionObject& that);

  SgNode* getBase() const;

    void add(MemRegionObjectPtr mr_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  MemRegionObjectPtr getUnionAO() { return unionMR_p; }

  bool mayEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool equalSetAO(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool subSetAO(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool isLiveAO(PartEdgePtr pedge);
  bool meetUpdateAO(MemRegionObjectPtr that, PartEdgePtr pedge);
  bool isFullAO(PartEdgePtr pedge);
  bool isEmptyAO(PartEdgePtr pedge);
  MemRegionObjectPtr copyAOType() const;
  void setAOToFull();
  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  // Returns the type of the concrete regions (if there is one)
  SgType* getConcreteType();
  // Returns the set of concrete memory regions as SgExpressions, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<SgNode* > getConcrete();
  ValueObjectPtr getRegionSizeAO(PartEdgePtr pedge);

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const;
  // AbstractObjects that form a hierarchy must inherit from the AbstractionHierarchy class

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy
  const hierKeyPtr& getHierKey() const;

  // ----------------------------------------
  // Objects that denote disjoint sets. Because no two sets may overlap, they can be
  // represented using unique numbers, which enables efficient data structure implementations.
  bool isDisjoint() const;
  // AbstractObjects that form a hierarchy must inherit from the AbstractObjectDisjoint class

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  virtual MemRegionObjectPtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);

  std::string str(std::string indent="") const;
};



/* ########################
   ##### MemLocObject #####
   ########################

A MemLocObject denotes a set of memory addresses. Locations within a single memory regions can be directly
compared to each other since memory regions are guaranteed to be contiguous and have a finite size.
Locations in different memory regions cannot be equal to each other and no additional information is
available and their relative values. As such, memory locations are represented as MemoryRegionObject/ValueObject
pairs that identify the location's region and its index within the region. Memory accesses are modeled as
reading/writing a fixed number of bytes (sizeof some type) starting from some memory location.
*/

// Major types of abstract objects
class MemLocObject;
typedef boost::shared_ptr<MemLocObject> MemLocObjectPtr;
//typedef boost::shared_ptr<const MemLocObject> ConstMemLocObjectPtr;
extern MemLocObjectPtr NULLMemLocObject;

class MemLocObject : public AbstractObject
{
  protected:
  MemRegionObjectPtr region;
  ValueObjectPtr     index;
  public:
//  MemLocObject() {}
  //# SA
  // should the default mutable value be conservatively true ?
  MemLocObject(SgNode* base) : AbstractObject(base) {}
  MemLocObject(MemRegionObjectPtr region, ValueObjectPtr index, SgNode* base) : AbstractObject(base), region(region), index(index) {}
  MemLocObject(const MemLocObject& that);

  // Wrapper for shared_from_this that returns an instance of this class rather than its parent
  MemLocObjectPtr shared_from_this() { return boost::static_pointer_cast<MemLocObject>(AbstractObject::shared_from_this()); }

  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  bool isValueObject()      { return false; }
  bool isCodeLocObject()    { return false; }
  bool isMemRegionObject()  { return false;  }
  bool isMemLocObject()     { return true;  }
  AOType getAOType() const { return AbstractObject::MemLoc; }

  virtual MemRegionObjectPtr getRegion() const;
  virtual ValueObjectPtr     getIndex() const;

  // Returns the relationship between the given AbstractObjects, considering whether either
  // or both are FuncResultMemLocObjects and if they refer to the same function
  FuncResultRelationType getFuncResultRel(AbstractObject* one, AbstractObjectPtr two, PartEdgePtr pedge);

//private:
//  // Returns whether this object may/must be equal to o within the given Part p
//  // These methods are called by composers and should not be called by analyses.
//  virtual bool mayEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);
//  virtual bool mustEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // General version of mayEqual and mustEqual that accounts for framework details before routing the call to the
  // derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
  // the call through the composer to make sure the may/mustEqual call gets the right PartEdge
  virtual bool mayEqual (MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Check whether that is a MemLocObject and if so, call the version of may/mustEqual specific to MemLocObjects
  virtual bool mayEqual (AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

//private:
//  // Returns whether the two abstract objects denote the same set of concrete objects
//  // These methods are called by composers and should not be called by analyses.
//  virtual bool equalSetAO(MemLocObjectPtr o, PartEdgePtr pedge);
//
//  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
//  // by the given abstract object.
//  // These methods are called by composers and should not be called by analyses.
//  virtual bool subSetAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  virtual bool equalSet(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  virtual bool subSet(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether the two abstract objects denote the same set of concrete objects
  virtual bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  virtual bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

//  // Returns true if this object is live at the given part and false otherwise
//  virtual bool isLiveAO(PartEdgePtr pedge);
  public:

  // General version of isLive that accounts for framework details before routing the call to the derived class'
  // isLiveML check. Specifically, it routes the call through the composer to make sure the isLiveML call gets the
  // right PartEdge
  virtual bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

// private:
//  // Computes the meet of this and that and saves the result in this
//  // returns true if this causes this to change and false otherwise
//  virtual bool meetUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge);

  // General version of meetUpdate that accounts for framework details before routing the call to the derived class'
  // meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML
  // call gets the right PartEdge
  virtual bool meetUpdate(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

//  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
//  virtual bool isFullAO(PartEdgePtr pedge);
//  // Returns whether this AbstractObject denotes the empty set.
//  virtual bool isEmptyAO(PartEdgePtr pedge);

  // General versions of isFull() and isEmpty that account for framework details before routing the call to the
  // derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make
  // sure the isFullAO() and isEmptyAO() call gets the right PartEdge.
  // These functions are just aliases for the real implementations in AbstractObject
  virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);
  virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Returns true if this AbstractObject corresponds to a concrete value that is statically-known
  virtual bool isConcrete();
  // Returns the number of concrete values in this set
  virtual int concreteSetSize();

  // Allocates a copy of this object and returns a pointer to it
  virtual MemLocObjectPtr copyAOType() const;

  // Allocates a copy of this object and returns a regular pointer to it
  virtual MemLocObject* copyMLPtr() const;

  AbstractObjectPtr copyAO() const
  { return copyAOType(); }

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  virtual MemLocObjectPtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);

  // Returns true if the given expression denotes a memory location and false otherwise
  static bool isMemExpr(SgExpression* expr)
  { return isSgVarRefExp(expr) || isSgPntrArrRefExp(expr) || isSgPointerDerefExp(expr); }

  virtual std::string str(std::string indent="") const; // pretty print for the object

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  virtual bool isHierarchy() const;
  // AbstractObjects that form a hierarchy must inherit from the AbstractionHierarchy class

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  virtual const hierKeyPtr& getHierKey() const;
}; // MemLocObject

// Special MemLocObject used internally by the framework to associate with the return value of a function.
// This is just a MemLoc dedicated to wrapping FuncResultMemLocRegion
class FuncResultMemLocObject : public MemLocObject
{
  // Special type of comparable object that is only equal to other instances of its type.
  // This makes it possible to use the generic FuncResultMemLocObject together with other
  // objects implemented by individual analyses while ensuring that a FuncResultMemLocObject
  // is always considered to be disjoint from the others
/*  class FuncResultMemLocObject_comparable: public comparable {
    public:
    bool operator==(const comparable& that);
    bool operator<(const comparable& that);
    std::string str(std::string indent="") const { return "FuncResultMemLocObject"; }
  }; // class FuncResultMemLocObject_comparable
  */
  public:
  FuncResultMemLocObject(Function func);
  FuncResultMemLocObject(const FuncResultMemLocObject& that);

  std::string str(std::string indent="") const { return "FuncResultMemLocObject"; }
  std::string strp(PartEdgePtr pedge, std::string indent="") { return "FuncResultMemLocObject"; }

  // Allocates a copy of this object and returns a shared pointer to it
  MemLocObjectPtr copyAOType() const;

  bool mustEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const {
    // The full object has no way to know. May need to change return value to allow dont-know
    ROSE_ASSERT(0);
  }

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const;
};
typedef boost::shared_ptr<FuncResultMemLocObject> FuncResultMemLocObjectPtr;

//! Default implementation of MemLocObject that denotes the set of all MemLocObjects
//! NOTE:Its sufficient to create only a single instance of this object globally.
class FullMemLocObject : public MemLocObject
{
public:
  FullMemLocObject() : MemLocObject(NULL) { }

  // Returns whether this object may/must be equal to o within the given Part p
  bool mayEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);
  bool mustEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetAO(MemLocObjectPtr o, PartEdgePtr pedge);

  // Allocates a copy of this object and returns a pointer to it
  MemLocObjectPtr copyAOType() const;

  // Returns true if this object is live at the given part and false otherwise
  bool isLiveAO(PartEdgePtr pedge);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullAO(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyAO(PartEdgePtr pedge);

  // Returns true if this AbstractObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();

  std::string str(std::string indent="") const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const {
    // The full object has no way to know. May need to change return value to allow dont-know
    ROSE_ASSERT(0);
  }

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const
  // The key of a full object is empty, so return the cachedHierKey, which is guaranteed to be empty
  { return cachedHierKey; }
};

/* #############################
   #### MappedMemLocObject ####
   ############################# */

//! Collection of MLs are used by composers to store results from multiple analysis.
//! They can also used by any analysis to denote a set of memory objects.
//! Combination of multiple MemLocObjects that are aligned using a map with a key.
//! Key is either Analysis* or PartEdgePtr.
//! Boolean template parameter determines if the response to the queries are least accurate (defaultMayEq=true)
//! or most accurate(defaultMayEq=false).
//! For mayEqual and mustEqual queries the response is computed by unioning the executions for least accurate answer.
//! Intersecting the executions yields the most accurate answer.
//! Consider the code
//! \f{verbatim} if(*) p=&a; else p=&b; *p=5; \f}
//! The set of executions for this of code at the assignment statement are \f$E=\{e_1, e_2, e_3, e_4, e_5\} \f$ where
//! \f$e_1: p \rightarrow \{ \}\f$,
//! \f$e_2: p \rightarrow  \{a \}\f$,
//! \f$e_3: p \rightarrow  \{b \}\f$,
//! \f$e_4: p \rightarrow  \{a,b \}\f$,
//! \f$e_5: p \rightarrow  \top \f$.
//! The ML object exported by Points-to at the assignment is \f$ p \rightarrow \{a, b\} \f$.
//! This ML object refines the set of executions to contain only \f$PT_E=\{e_2, e_3, e_4\} \f$.
//! Constant propagation knows nothing about the pointers.
//! Its set of executions contain everything \f$CP_E=\{e_1, e_2, e_3, e_4, e_5\} \f$.
//! \f$ PT_E \cup CP_E=\{e_1, e_2, e_3, e_4, e_5\} \f$.
//! \f$ PT_E \cap CP_E=\{e_2,e_3,e_4\} \f$.
//! Consider the following may equality queries \f$ *p =_{may} a, *p =_{may} b, *p =_{may} c \f$.
//! The least accurate answer is provided by the set of executions \f$ PT_E \cup CT_E \f$.
//! The most accurate answer is provided by the set of executions \f$ PT_E \cap CT_E \f$.
/*! \f{tabular} {| l | l | l | l |}
   \hline
   &  $*p=_{may}a$ & $*p=_{may}b$ & $*p=_{may}c$ \\ \hline
   PT & T & T & F \\
   CP & T & T & T \\
   $PT \cup CP$  & T & T & T \\
   $PT \cap CP$  & T & T & F \\
   \hline
  \f} */
//! Now consider the code
//! \f{verbatim} q=&c; if(*) p=&a; else p=&b; *p=5; \f}
//! The executions \f$e_2, e_3, e_4 \f$ also have the information \f$ q \rightarrow \{c\} \f$.
//! Consider the must equality queries \f$ *p =_{must} a, *p =_{must} b, *p =_{must} c \f$.
//! As before the must equality queries are answered based on union (least accurate) or intersection (most accurate) of executions.
/*! \f{tabular} {| l | l | l | l |}
   \hline
   &  $*p=_{must}a$ & $*p=_{must}b$ & $*p=_{must}c$ \\ \hline
   PT & F & F & T \\
   CP & F & F & F \\
   $PT \cup CP$  & F & F & F \\
   $PT \cap CP$  & F & F & T \\
   \hline
  \f}*/
//! MemLocObjects that are full can be returned by either analysis or tight composer (FullMemLocObject)
//! Query dispatching to analysis compares two MLs implemented by it.
//! If an FullMemLocObject is passed to analysis it breaks the analysis implementation of the set operations for MLs.
//! Storing analysis MLs that are full is also not useful.
//! Consequently full MLs are never stored in object collection.
//! Comparison of two MappedML is performed by dispatching the query based on the key.
//! If the key is not common in the two MappedML then it is assumed that the MappedML missing the key
//! has FullAO(full set of objects) mapped to the corresponding key.
//! Since full MLs are never stored in the map, an empty map does not imply that the MappedML is full.
//! To distinguish the full state from empty state the MappedMemLocObject uses the variable n_FullML
//! that counts the number of full mls subjected to be added to the map using add  or meetUpdateML method.
//! n_FullML != 0 along with empty map determines that the MappedML denotes full set of ML.
//! n_FullML == 0 and en empty map indicates that the MappedML is empty.
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType, class MappedAOMemRegionType>
class MappedMemLocObject : public MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>
{
public:
  MappedMemLocObject(uiType ui, ComposedAnalysis* analysis) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>(ui, /*nFull*/ 0, analysis)
  {}
  MappedMemLocObject(uiType ui, ComposedAnalysis* analysis, const std::map<Key, MemLocObjectPtr>& aoMap) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>(ui, /*nFull*/ 0, analysis, aoMap)
  {}
  MappedMemLocObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, MemLocObjectPtr>& aoMap) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>(ui, nFull, analysis, aoMap)
  {}
  MappedMemLocObject(const MappedMemLocObject& that) :
    MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>(that)
  {}

  // Allocates a copy of this object and returns a regular pointer to it
  MemLocObject* copyMLPtr() const { return new MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>(*this); }

  //SgNode* getBase() const;
  MemRegionObjectPtr getRegion() const;
  ValueObjectPtr     getIndex() const;


  // Check whether that is a MemLocObject and if so, call the version of mayEqual specific to MemLocObjects
  bool mayEqual(AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis);

  // Check whether that is a MemLocObject and if so, call the version of mustEqual specific to MemLocObjects
  bool mustEqual(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSet(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSet(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // General version of meetUpdate() that accounts for framework details before routing the call to the derived class'
  // meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML
  // call gets the right PartEdge
  bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis);
}; // class MappedMemLocObject

template<class Key, bool KeyIsComposedAnalysis>
class GenericMappedMemLocObject : public MappedMemLocObject<Key, KeyIsComposedAnalysis,
                                                            GenericMappedMemLocObject   <Key, KeyIsComposedAnalysis>,
                                                            GenericMappedValueObject    <Key, KeyIsComposedAnalysis>,
                                                            GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis> > {
  public:
  GenericMappedMemLocObject(uiType ui, ComposedAnalysis* analysis) : MappedMemLocObject<Key, KeyIsComposedAnalysis, GenericMappedMemLocObject<Key, KeyIsComposedAnalysis>, GenericMappedValueObject<Key, KeyIsComposedAnalysis>, GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis> >(ui, analysis) {}
  GenericMappedMemLocObject(uiType ui, ComposedAnalysis* analysis, const std::map<Key, MemLocObjectPtr>& aoMap) : MappedMemLocObject<Key, KeyIsComposedAnalysis, GenericMappedMemLocObject<Key, KeyIsComposedAnalysis>, GenericMappedValueObject<Key, KeyIsComposedAnalysis>, GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis> >(ui, analysis, aoMap) {}
  GenericMappedMemLocObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, MemLocObjectPtr>& aoMap) : MappedMemLocObject<Key, KeyIsComposedAnalysis, GenericMappedMemLocObject<Key, KeyIsComposedAnalysis>, GenericMappedValueObject<Key, KeyIsComposedAnalysis>, GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis> >(ui, nFull, analysis, aoMap) {}
  GenericMappedMemLocObject(const GenericMappedMemLocObject& that) : MappedMemLocObject<Key, KeyIsComposedAnalysis, GenericMappedMemLocObject<Key, KeyIsComposedAnalysis>, GenericMappedValueObject<Key, KeyIsComposedAnalysis>, GenericMappedMemRegionObject<Key, KeyIsComposedAnalysis> >(that) {}
};

extern template class GenericMappedMemLocObject<ComposedAnalysis*, true>;
extern template class MappedMemLocObject<ComposedAnalysis*, true, GenericMappedMemLocObject<ComposedAnalysis*, true>, GenericMappedValueObject<ComposedAnalysis*, true>, GenericMappedMemRegionObject<ComposedAnalysis*, true> >;
extern template class MappedAbstractObject<ComposedAnalysis*, true, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, GenericMappedMemLocObject<ComposedAnalysis*, true> >;
typedef boost::shared_ptr<GenericMappedMemLocObject<ComposedAnalysis*, true> > AnalMapMemLocObjectPtr;
typedef GenericMappedMemLocObject<ComposedAnalysis*, true> AnalMapMemLocObject;

/* #############################
   # PartEdgeUnionMemLocObject #
   ############################# */

//! Special MemLocObject to union MemLocObject from different PartEdges.
//! MemLocObjects should be of same type i.e., from the same analysis.
class PartEdgeUnionMemLocObject : public MemLocObject {
  MemLocObjectPtr unionML_p;
public:
  PartEdgeUnionMemLocObject();
  PartEdgeUnionMemLocObject(const PartEdgeUnionMemLocObject& that);

  SgNode* getBase() const;

  MemRegionObjectPtr getRegion() const;
  ValueObjectPtr     getIndex() const;

  void add(MemLocObjectPtr ml_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  MemLocObjectPtr getUnionAO() { return unionML_p; }

//  bool mayEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);
//  bool mustEqualAO(MemLocObjectPtr o, PartEdgePtr pedge);
//  bool equalSetAO(MemLocObjectPtr o, PartEdgePtr pedge);
//  bool subSetAO(MemLocObjectPtr o, PartEdgePtr pedge);
//  bool isLiveAO(PartEdgePtr pedge);
//  bool meetUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge);
//  bool isFullAO(PartEdgePtr pedge);
//  bool isEmptyAO(PartEdgePtr pedge);

  bool mayEqual(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool equalSet(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool subSet(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool meetUpdate(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the number of concrete values in this set
  int concreteSetSize();
  MemLocObjectPtr copyAOType() const;
  void setAOToFull();

  // Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
  // returns the object itself. For compound objects it searches through the sub-objects inside of it for the
  // individual objects that came from a given analysis and returns their combination. For example, a Union object
  // will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
  // the resulting object.
  virtual MemLocObjectPtr project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis=NULL);

  std::string str(std::string indent="") const;

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const;

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const;

  // ----------------------------------------
  // Objects that denote disjoint sets. Because no two sets may overlap, they can be
  // represented using unique numbers, which enables efficient data structure implementations.
  bool isDisjoint() const;
  // AbstractObjects that form a hierarchy must inherit from the AbstractObjectDisjoint class
};


class UInt {
  public:
  unsigned int val;
  UInt(unsigned int val): val(val) {}
  // Enables type conversion from ComposedAnalysis just so that the corresponding code type-checks.
  // This conversion is still an error that will be caught at runtime
  UInt(ComposedAnalysis* analysis) { assert(0); }

  bool operator==(const UInt& that) const { return val == that.val; }
  bool operator<(const UInt& that) const { return val <  that.val; }
  const UInt* operator->() const { return this; }
  //const UInt* get() const { return this; }

  std::string str(std::string indent="") const {
    std::ostringstream s;
    s << val;
    return s.str();
  }
};

class CombinedCodeLocObject : public MappedCodeLocObject<UInt, false, CombinedCodeLocObject>
{
  public:
  std::list<CodeLocObjectPtr > subObjs;
  CombinedCodeLocObject(uiType ui, ComposedAnalysis* analysis, CodeLocObjectPtr subObj) :
        MappedCodeLocObject<UInt, false, CombinedCodeLocObject>(ui, analysis)
  {
    MappedCodeLocObject<UInt, false, CombinedCodeLocObject>::aoMap[UInt((unsigned int)0)] = subObj;
    subObjs.push_back(subObj);
  }
  CombinedCodeLocObject(uiType ui, ComposedAnalysis* analysis, const std::list<CodeLocObjectPtr >& subObjs) :
        MappedCodeLocObject<UInt, false, CombinedCodeLocObject>(ui, analysis),
        subObjs(subObjs)
  {
    unsigned int i=0;
    for(std::list<CodeLocObjectPtr >::const_iterator sub=subObjs.begin(); sub!=subObjs.end(); ++sub, ++i)
      MappedCodeLocObject<UInt, false, CombinedCodeLocObject>::aoMap[UInt(i)] = *sub;
  }
  CombinedCodeLocObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<UInt, CodeLocObjectPtr >& aoMap) :
        MappedCodeLocObject<UInt, false, CombinedCodeLocObject>(ui, nFull, analysis, aoMap)
  {}

  CombinedCodeLocObject(const CombinedCodeLocObject& that) :
        MappedCodeLocObject<UInt, false, CombinedCodeLocObject>(that),
        subObjs(that.subObjs)
  {}

  const std::list<CodeLocObjectPtr>& getCodeLocs() const { return subObjs; }
}; // class CombinedCodeLocObject

typedef boost::shared_ptr<CombinedCodeLocObject> CombinedCodeLocObjectPtr;

class CombinedValueObject : public MappedValueObject<UInt, false, CombinedValueObject>
{
  public:
  std::list<ValueObjectPtr > subObjs;
  CombinedValueObject(uiType ui, ComposedAnalysis* analysis, ValueObjectPtr subObj) :
        MappedValueObject<UInt, false, CombinedValueObject>(ui, analysis)
  {
    MappedValueObject<UInt, false, CombinedValueObject>::aoMap[UInt((unsigned int)0)] = subObj;
    subObjs.push_back(subObj);
  }
  CombinedValueObject(uiType ui, ComposedAnalysis* analysis, const std::list<ValueObjectPtr >& subObjs) :
        MappedValueObject<UInt, false, CombinedValueObject>(ui, analysis),
        subObjs(subObjs)
  {
    unsigned int i=0;
    for(std::list<ValueObjectPtr >::const_iterator sub=subObjs.begin(); sub!=subObjs.end(); ++sub, ++i)
      MappedValueObject<UInt, false, CombinedValueObject>::aoMap[UInt(i)] = *sub;
  }
  CombinedValueObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<UInt, ValueObjectPtr >& aoMap) :
        MappedValueObject<UInt, false, CombinedValueObject>(ui, nFull, analysis, aoMap)
  {}

  CombinedValueObject(const CombinedValueObject& that) :
        MappedValueObject<UInt, false, CombinedValueObject>(that),
        subObjs(that.subObjs)
  {}

  const std::list<ValueObjectPtr>& getValues() const { return subObjs; }
}; // class CombinedValueObject

typedef boost::shared_ptr<CombinedValueObject> CombinedValueObjectPtr;

class CombinedMemRegionObject : public MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>
{
  public:
  std::list<MemRegionObjectPtr > subObjs;
  CombinedMemRegionObject(uiType ui, ComposedAnalysis* analysis, MemRegionObjectPtr subObj) :
        MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>(ui, analysis)
  {
    MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>::aoMap[UInt((unsigned int)0)] = subObj;
    subObjs.push_back(subObj);
  }
  CombinedMemRegionObject(uiType ui, ComposedAnalysis* analysis, const std::list<MemRegionObjectPtr >& subObjs) :
        MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>(ui, analysis),
        subObjs(subObjs)
  {
    unsigned int i=0;
    for(std::list<MemRegionObjectPtr >::const_iterator sub=subObjs.begin(); sub!=subObjs.end(); ++sub, ++i)
      MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>::aoMap[UInt(i)] = *sub;
  }
  CombinedMemRegionObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<UInt, MemRegionObjectPtr >& aoMap) :
        MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>(ui, nFull, analysis, aoMap)
  {}

  CombinedMemRegionObject(const CombinedMemRegionObject& that) :
        MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>(that),
        subObjs(that.subObjs)
  {}

  const std::list<MemRegionObjectPtr>& getMemRegions() const { return subObjs; }
}; // class CombinedMemRegionObject

typedef boost::shared_ptr<CombinedMemRegionObject> CombinedMemRegionObjectPtr;

class CombinedMemLocObject : public MappedMemLocObject<UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>
{
  public:
  std::list<MemLocObjectPtr > subObjs;
  CombinedMemLocObject(uiType ui, ComposedAnalysis* analysis, MemLocObjectPtr subObj) :
        MappedMemLocObject<UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>(ui, analysis)
  {
    MappedMemLocObject<UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>::aoMap[UInt((unsigned int)0)] = subObj;
    subObjs.push_back(subObj);
  }
  CombinedMemLocObject(uiType ui, ComposedAnalysis* analysis, const std::list<MemLocObjectPtr >& subObjs) :
        MappedMemLocObject<UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>(ui, analysis),
        subObjs(subObjs)
  {
    unsigned int i=0;
    for(std::list<MemLocObjectPtr >::const_iterator sub=subObjs.begin(); sub!=subObjs.end(); ++sub, ++i)
      MappedMemLocObject<UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>::aoMap[UInt(i)] = *sub;
  }
  CombinedMemLocObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<UInt, MemLocObjectPtr >& aoMap) :
        MappedMemLocObject<UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>(ui, nFull, analysis, aoMap)
  {}

  CombinedMemLocObject(const CombinedMemLocObject& that) :
        MappedMemLocObject<UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>(that),
        subObjs(that.subObjs)
  {}

  const std::list<MemLocObjectPtr>& getMemLocs() const { return subObjs; }
}; // class CombinedMemLocObject

typedef boost::shared_ptr<CombinedMemLocObject> CombinedMemLocObjectPtr;

extern template class MappedCodeLocObject  <UInt, false, CombinedCodeLocObject>;
extern template class MappedValueObject    <UInt, false, CombinedValueObject>;
extern template class MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>;
extern template class MappedMemLocObject   <UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>;
extern template class MappedAbstractObject<UInt, false, CodeLocObject,   CodeLocObjectPtr,   AbstractObject::CodeLoc,   CombinedCodeLocObject>;
extern template class MappedAbstractObject<UInt, false, ValueObject,     ValueObjectPtr,     AbstractObject::Value,     CombinedValueObject>;
extern template class MappedAbstractObject<UInt, false, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, CombinedMemRegionObject>;
extern template class MappedAbstractObject<UInt, false, MemLocObject,    MemLocObjectPtr,    AbstractObject::MemLoc,    CombinedMemLocObject>;

/* ###########################################
   ##### Specific Types of MemLocObjects #####
   ########################################### */

// represents d-dimensional integral vectors. It encapsulates a variety of abstract representations for such vectors
// such as polyhedral constraints and strided indexes.
// TODO: we support a single multi-dimensional index for now
class IndexVector;
typedef boost::shared_ptr<IndexVector> IndexVectorPtr;
class IndexVector
{
 public:
  // the index vector's length
  size_t getSize(PartEdgePtr pedge);
  //virtual std::string str(const std::string& indent);
  virtual std::string str(std::string indent="") const; // pretty print for the object

  // Allocates a copy of this object and returns a pointer to it
  virtual IndexVectorPtr copyIV() const=0;

  // equal operator
  virtual bool mayEqual  (IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool mustEqual (IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool meetUpdate(IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether the two abstract index vectors denote the same set of concrete vectors.
  virtual bool equalSet(IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this abstract index vector denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract index vector.
  virtual bool subSet(IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  virtual bool isFull    (PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  virtual bool isEmpty   (PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
};

}; // namespace fuse

#endif
