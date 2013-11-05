#ifndef ABSTRACT_OBJECT_H
#define ABSTRACT_OBJECT_H

#include "sight.h"
#include "partitions.h"
#include "CallGraphTraverse.h"
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

class CodeLocObject;
class ValueObject;
class MemLocObject;
class MemRegionObject;

class AbstractObject : public sight::printable, public boost::enable_shared_from_this<AbstractObject>
{
  SgNode* base;
  
  public:
  typedef enum {Value, CodeLoc, MemRegion, MemLoc} AOType;
  
  AbstractObject() {}
  AbstractObject(SgNode* base) : base(base) {}
  AbstractObject(const AbstractObject& that) : base(that.base) {}
  
  SgNode* getBase() const { return base; }

  // Analyses that are being composed inside a given composer provide a pointer to themselves
  // in the client argument. Code that uses the composer from the outside, does not need to provide
  // a client.
  
  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  virtual bool isValueObject();
  virtual bool isCodeLocObject();
  virtual bool isMemRegionObject();
  virtual bool isMemLocObject();
  virtual AOType getAOType()=0;
  
  // Returns whether this object may/must be equal to o within the given Part p
  virtual bool mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  virtual bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  
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
  
  // General versions of subSet() that accounts for framework details before routing the call to the 
  // derived class' subSet() check. Specifically, it routes the call through the composer to make 
  // sure the subSet() call gets the right PartEdge.
  virtual bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  
  // Returns true if this object is live at the given part and false otherwise
  virtual bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  
  /*// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isFullCL(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isEmptyCL(PartEdgePtr pedge);*/
  
  // General versions of isFull() and isEmpty that account for framework details before routing the call to the 
  // derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make 
  // sure the isFullML() and isEmptyML() call gets the right PartEdge.
  virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)=0;
  
  // Allocates a copy of this object and returns a pointer to it
  virtual AbstractObjectPtr copyAO() const=0;

  /* Don't have good idea how to represent a finite number of options 
  virtual bool isFiniteSet()=0;
  virtual set<AbstractObj> getValueSet()=0;*/
  
  //virtual std::string str(const std::string& indent)=0;
  
  // Variant of the str method that can produce information specific to the current Part.
  // Useful since AbstractObjects can change from one Part to another.
  virtual std::string strp(PartEdgePtr pedge, std::string indent="")
  { return str(indent); }
};

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
  AOType getAOType() { return AbstractObject::CodeLoc; }
  
  PartPtr getPart()    const { return part; }
  CFGNode getCFGNode() const { return cfgNode; }
  
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  virtual bool mayEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  virtual bool mustEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  
public:
  // General version of mayEqual and mustEqual that implements may/must equality with respect to ExprObj
  // and uses the derived class' may/mustEqual check for all the other cases
  // GREG: Currently nothing interesting here since we don't support ExprObjs for CodeLocObjects
  bool mayEqual(CodeLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(CodeLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  bool mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  virtual bool equalSetCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  virtual bool subSetCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  
public:
  // General version of equalSet and subSet that implements may/must equality with respect to ExprObj
  // and uses the derived class' may/mustEqual check for all the other cases
  // GREG: Currently nothing interesting here since we don't support ExprObjs for CodeLocObjects
  bool equalSet(CodeLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool subSet(CodeLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
//private:
  // Returns true if this object is live at the given part and false otherwise.
  // This method is called by composers and should not be called by analyses.
  virtual bool isLiveCL(PartEdgePtr pedge);
  
public:
  // General version of isLive that accounts for framework details before routing the call to the derived class' 
  // isLiveCL check. Specifically, it routes the call through the composer to make sure the isLiveCL call gets the 
  // right PartEdge
  bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdateCL(CodeLocObjectPtr that, PartEdgePtr pedge);
  
  // General version of meetUpdate that accounts for framework details before routing the call to the derived class' 
  // meetUpdateCL check. Specifically, it routes the call through the composer to make sure the meetUpdateCL 
  // call gets the right PartEdge
  bool meetUpdate(CodeLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isFullCL(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isEmptyCL(PartEdgePtr pedge);
    
  // General versions of isFull() and isEmpty that account for framework details before routing the call to the 
  // derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make 
  // sure the isFullML() and isEmptyML() call gets the right PartEdge.
  // These functions are just aliases for the real implementations in AbstractObject
  bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);
  bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);
  
  // Allocates a copy of this object and returns a pointer to it
  virtual CodeLocObjectPtr copyCL() const;
  
  // Allocates a copy of this object and returns a regular pointer to it
  virtual CodeLocObject* copyCLPtr() const;
  
  AbstractObjectPtr copyAO() const
  { return copyCL(); }
  
  virtual std::string str(std::string indent=""); // pretty print for the object
};

// The combination of multiple CodeLocObjects. Maintains multiple CodeLocObjects and responds to
//   API calls with the most or least accurate response that its constituent objects return, depending
//   on the value of the template parameter defaultMayEq (the default value that mayEqual would return
//   if any constituent MemLocObject returns this value).
// For practical purposes analyses should ensure that different instances of IntersectCodeLocObject 
//   are only compared if they include the same types of CodeLocObjects in the same order. Otherwise, 
//   the comparisons will be uselessly inaccurate.
template <bool defaultMayEq>
class CombinedCodeLocObject: public CodeLocObject
{
  public:
  std::list<CodeLocObjectPtr> codeLocs;
  
  CombinedCodeLocObject(CodeLocObjectPtr codeLoc) : CodeLocObject(NULL) { codeLocs.push_back(codeLoc); }
  CombinedCodeLocObject(const std::list<CodeLocObjectPtr>& codeLocs) : CodeLocObject(NULL), codeLocs(codeLocs) {}
  
  const std::list<CodeLocObjectPtr>& getCodeLocs() const { return codeLocs; }
  
  void add(CodeLocObjectPtr codeLoc);
  
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are private to prevent analyses from calling them directly.
  bool mayEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  bool mustEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetCL(CodeLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns true if this object is live at the given part and false otherwise
  bool isLiveCL(PartEdgePtr pedge);
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateCL(CodeLocObjectPtr that, PartEdgePtr pedge);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullCL(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyCL(PartEdgePtr pedge);
  
  // Allocates a copy of this object and returns a pointer to it
  CodeLocObjectPtr copyCL() const;
  
  std::string str(std::string indent="");
};
typedef CombinedCodeLocObject<false> IntersectCodeLocObject;
typedef boost::shared_ptr<IntersectCodeLocObject> IntersectCodeLocObjectPtr;
typedef CombinedCodeLocObject<true> UnionCodeLocObject;
typedef boost::shared_ptr<UnionCodeLocObject> UnionCodeLocObjectPtr;

// Sriram: gcc 4.1.2 complains of undefined references to unused to template functions
// fix: explicit template instantiation
extern template class CombinedCodeLocObject<true>;
extern template class CombinedCodeLocObject<false>;

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
  AOType getAOType() { return AbstractObject::CodeLoc; }
  
//private:
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  virtual bool mayEqualV(ValueObjectPtr o, PartEdgePtr pedge)=0;
  virtual bool mustEqualV(ValueObjectPtr o, PartEdgePtr pedge)=0;

public:
  
  // Returns whether this object may/must be equal to o within the given Part p
  // by propagating the call through the composer
  bool mayEqual(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  bool mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  virtual bool equalSetV(ValueObjectPtr o, PartEdgePtr pedge)=0;
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  virtual bool subSetV(ValueObjectPtr o, PartEdgePtr pedge)=0;
  
public:
  // General version of mayEqual and mustEqual that implements may/must equality with respect to ExprObj
  // and uses the derived class' may/mustEqual check for all the other cases
  // GREG: Currently nothing interesting here since we don't support ExprObjs for ValueObjects
  bool equalSet(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool subSet(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
//private:
  // Returns true if this object is live at the given part and false otherwise.
  // This method is called by composers and should not be called by analyses.
  // NOTE: we do not currently allow ValueObjects to implement an isLive methods because we assume that they'll always be live
  bool isLiveV(PartEdgePtr pedge) { return true; }
  
public:
  // Returns true if this object is live at the given part and false otherwise
  // NOTE: we currently assume that ValueObjects are always live
  bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) { return true; }
  
  // Computes the meet of this and that and saves the result in this.
  // Returns true if this causes this to change and false otherwise.
  virtual bool meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge)=0;
  
  // General version of meetUpdate that accounts for framework details before routing the call to the derived class' 
  // meetUpdateV check. Specifically, it routes the call through the composer to make sure the meetUpdateV
  // call gets the right PartEdge
  bool meetUpdate(ValueObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isEmptyV(PartEdgePtr pedge)=0;
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isFullV(PartEdgePtr pedge)=0;
  
  // General version of isFull/isEmpty that accounts for framework details before routing the call to the 
  // derived class' isFullV/isEmptyV check. Specifically, it routes the call through the composer to make 
  // sure the isFullV/isEmptyV call gets the right PartEdge
  bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  virtual bool isConcrete()=0;
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
  virtual ValueObjectPtr copyV() const=0;
  AbstractObjectPtr copyAO() const;
};

// The default implementation of ValueObjects that denotes the set of all ValueObjects
class UnknownValueObject : public ValueObject
{
  public:
  UnknownValueObject() {}
  
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are private to prevent analyses from calling them directly.
  bool mayEqualV(ValueObjectPtr o, PartEdgePtr pedge);
  bool mustEqualV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullV(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyV(PartEdgePtr pedge);
  
  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();
  
  // Allocates a copy of this object and returns a pointer to it
  ValueObjectPtr copyV() const;
  
  std::string str(std::string indent="");
};

// The combination of multiple ValueObjects. Maintains multiple ValueObjects and responds to
//   API calls with the most or least accurate response that its constituent objects return, depending
//   on the value of the template parameter defaultMayEq (the default value that mayEqual would return
//   if any constituent ValueObject returns this value).
// For practical purposes analyses should ensure that different instances of IntersectValueObject 
//   are only compared if they include the same types of ValueObjects in the same order. Otherwise, 
//   the comparisons will be uselessly inaccurate.
template <bool defaultMayEq>
class CombinedValueObject : public ValueObject
{
  std::list<ValueObjectPtr> vals;
  
  public:
  CombinedValueObject(ValueObjectPtr val): ValueObject(NULL) { vals.push_back(val); }
  CombinedValueObject(const std::list<ValueObjectPtr>& vals): ValueObject(NULL), vals(vals) {}
  
  const std::list<ValueObjectPtr>& getVals() const { return vals; }
  
  // Creates a new CombinedValueObject. Template instantiation is used to ensure that instances of
  // CombinedalueObjectV<true> and CombinedValueObjeVctV<false> get instantiated
  //static boost::shared_ptr<CombinedValueObject<defaultMayEq> > create(const std::list<ValueObjectPtr>& vals);
  
  void add(ValueObjectPtr val);
  
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are private to prevent analyses from calling them directly.
  bool mayEqualV(ValueObjectPtr o, PartEdgePtr pedge);
  bool mustEqualV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetV(ValueObjectPtr o, PartEdgePtr pedge);
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullV(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyV(PartEdgePtr pedge);
  
  // Returns true if this ValueObject corresponds to a concrete value that is statically-known
  bool isConcrete();
  // Returns the type of the concrete value (if there is one)
  SgType* getConcreteType();
  // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
  // the normal ROSE mechanisms to decode it
  std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();
  
  // Allocates a copy of this object and returns a pointer to it
  ValueObjectPtr copyV() const;
  
  std::string str(std::string indent="");
};
typedef CombinedValueObject<false> IntersectValueObject;
typedef boost::shared_ptr<IntersectValueObject> IntersectValueObjectPtr;
typedef CombinedValueObject<true> UnionValueObject;
typedef boost::shared_ptr<UnionValueObject> UnionValueObjectPtr;

// Sriram: gcc 4.1.2 complains of undefined references to unused to template functions
// fix: explicit template instantiation
extern template class CombinedValueObject<true>;
extern template class CombinedValueObject<false>;

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
  
  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  bool isValueObject()      { return false; }
  bool isCodeLocObject()    { return false; }
  bool isMemRegionObject()  { return true;  }
  bool isMemLocObject()     { return false; }
  AOType getAOType() { return AbstractObject::MemRegion; }
  
//private:
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  virtual bool mayEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge)=0;
  virtual bool mustEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge)=0;
  
public:
  // General version of mayEqual and mustEqual that accounts for framework details before routing the call to the 
  // derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
  // the call through the composer to make sure the may/mustEqual call gets the right PartEdge
  bool mayEqual (MemRegionObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(MemRegionObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Check whether that is a MemRegionObject and if so, call the version of may/mustEqual specific to MemRegionObjects
  bool mayEqual (AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
//private:
  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  virtual bool equalSetMR(MemRegionObjectPtr o, PartEdgePtr pedge)=0;
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  virtual bool subSetMR(MemRegionObjectPtr o, PartEdgePtr pedge)=0;
  
public:
  // General version of equalSet and subSet that implements may/must equality with respect to ExprObj
  // and uses the derived class' may/mustEqual check for all the other cases
  // GREG: Currently nothing interesting here since we don't support ExprObjs for MemoryRegionObjects
  bool equalSet(MemRegionObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool subSet  (MemRegionObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool subSet  (AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns true if this object is live at the given part and false otherwise
  virtual bool isLiveMR(PartEdgePtr pedge)=0;
public:
  //MemRegionObjectPtr getThis();
  // General version of isLive that accounts for framework details before routing the call to the derived class' 
  // isLiveMR check. Specifically, it routes the call through the composer to make sure the isLiveML call gets the 
  // right PartEdge
  bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
// private:
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge)=0;
  
  // General version of meetUpdate that accounts for framework details before routing the call to the derived class' 
  // meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML 
  // call gets the right PartEdge
  bool meetUpdate(MemRegionObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isEmptyMR(PartEdgePtr pedge)=0;
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isFullMR(PartEdgePtr pedge)=0;
  
  // General version of isFull/isEmpty that accounts for framework details before routing the call to the 
  // derived class' isFullMR/isEmptyMR check. Specifically, it routes the call through the composer to make 
  // sure the isFullMR/isEmptyMR call gets the right PartEdge
  bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns a ValueObject that denotes the size of this memory region
  virtual ValueObjectPtr getRegionSize(PartEdgePtr pedge) const=0;
    
  // Allocates a copy of this object and returns a pointer to it
  virtual MemRegionObjectPtr copyMR() const=0;
  AbstractObjectPtr copyAO() const
  { return copyMR(); }
}; // class MemRegionObject

// Special MemRegionObject used internally by the framework to associate with the return value of a function
class FuncResultMemRegionObject : public MemRegionObject
{
  Function func;
  public:
  FuncResultMemRegionObject(Function func);
  
  // Returns whether this object may/must be equal to o within the given Part p
  bool mayEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool mustEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Returns true if this object is live at the given part and false otherwise.
  // This method is called by composers and should not be called by analyses.
  bool isLiveMR(PartEdgePtr pedge) { return true; }
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullMR(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyMR(PartEdgePtr pedge);
  
  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr getRegionSize(PartEdgePtr pedge) const;
  
  std::string str(std::string indent="") const { return "FuncResultMemRegionObject"; }
  std::string str(std::string indent="")       { return "FuncResultMemRegionObject"; }
  std::string strp(PartEdgePtr pedge, std::string indent="") { return "FuncResultMemRegionObject"; }
  
  // Allocates a copy of this object and returns a pointer to it
  MemRegionObjectPtr copyMR() const;
};
typedef boost::shared_ptr<FuncResultMemRegionObject> FuncResultMemRegionObjectPtr;

// The combination of multiple MemRegionObjects. Maintains multiple MemRegionObjects and responds to
//   API calls with the most or least accurate response that its constituent objects return, depending
//   on the value of the template parameter defaultMayEq (the default value that mayEqual would return
//   if any constituent MemRegionObject returns this value).
// For practical purposes analyses should ensure that different instances of IntersectMemRegionObject 
//   are only compared if they include the same types of MemRegionObjects in the same order. Otherwise, 
//   the comparisons will be uselessly inaccurate.
template <bool defaultMayEq>
class CombinedMemRegionObject : public virtual MemRegionObject
{
  std::list<MemRegionObjectPtr> memRegions;
  
  public:
  CombinedMemRegionObject(MemRegionObjectPtr memReg) : MemRegionObject(NULL) {   memRegions.push_back(memReg); }
  CombinedMemRegionObject(const std::list<MemRegionObjectPtr>& memRegions) : MemRegionObject(NULL), memRegions(memRegions) {}
  
  virtual ~CombinedMemRegionObject() {}
  
  const std::list<MemRegionObjectPtr>& getMemRegions() const { return memRegions; }

  public:
  // Creates a new CombinedMemRegionObject. If all the sub-objects have a given type (Scalar, FunctionMemLoc, 
  // LabeledAggregate, Array or Pointer), the created CombinedMemRegionObject has the same type. Otherwise, the
  // created CombinedMemRegionObject is an instance of the generic CombinedMemRegionObject class.
  /*static boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > create(MemRegionObjectPtr memReg);
  static boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > create(const std::list<MemRegionObjectPtr>& memRegions);*/
  
  void add(MemRegionObjectPtr memLoc);
  
  // Returns whether this object may/must be equal to o within the given Part p
  bool mayEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  bool mustEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetMR(MemRegionObjectPtr o, PartEdgePtr pedge);
  
  // Allocates a copy of this object and returns a pointer to it
  MemRegionObjectPtr copyMR() const;
  
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
  
  std::string str(std::string indent="");
};
typedef CombinedMemRegionObject<false> IntersectMemRegionObject;
typedef boost::shared_ptr<IntersectMemRegionObject> IntersectMemRegionObjectPtr;
typedef CombinedMemRegionObject<true> UnionMemRegionObject;
typedef boost::shared_ptr<UnionMemRegionObject> UnionMemRegionObjectPtr;


// Sriram: gcc 4.1.2 complains of undefined references to unused to template functions
// fix: explicit template instantiation
extern template class CombinedMemRegionObject<true>;
extern template class CombinedMemRegionObject<false>; // not sure if this is needed as there were no errors

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
  MemLocObject(const MemLocObject& that) : 
    AbstractObject(that), 
    region(that.region? that.region->copyMR(): that.region), 
    index (that.index ? that.index->copyV()  : that.index) {}

  // Wrapper for shared_from_this that returns an instance of this class rather than its parent
  MemLocObjectPtr shared_from_this() { return boost::static_pointer_cast<MemLocObject>(AbstractObject::shared_from_this()); }
  
  // Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
  // classes to save the cost of a dynamic cast.
  bool isValueObject()      { return false; }
  bool isCodeLocObject()    { return false; }
  bool isMemRegionObject()  { return false;  }
  bool isMemLocObject()     { return true;  }
  AOType getAOType() { return AbstractObject::MemLoc; }
  
  MemRegionObjectPtr getRegion() const { return region; }
  ValueObjectPtr     getIndex() const  { return index; }
  
//private:
  // Returns whether this object may/must be equal to o within the given Part p
  // These methods are called by composers and should not be called by analyses.
  virtual bool mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge);
  virtual bool mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge);
  
  // General version of mayEqual and mustEqual that accounts for framework details before routing the call to the 
  // derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
  // the call through the composer to make sure the may/mustEqual call gets the right PartEdge
  bool mayEqual (MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Check whether that is a MemLocObject and if so, call the version of may/mustEqual specific to MemLocObjects
  bool mayEqual (AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
//private:
  // Returns whether the two abstract objects denote the same set of concrete objects
  // These methods are called by composers and should not be called by analyses.
  virtual bool equalSetML(MemLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  // These methods are called by composers and should not be called by analyses.
  virtual bool subSetML(MemLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSet(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSet(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

//private:
  // Returns true if this object is live at the given part and false otherwise
  virtual bool isLiveML(PartEdgePtr pedge);
public:
  
  // General version of isLive that accounts for framework details before routing the call to the derived class' 
  // isLiveML check. Specifically, it routes the call through the composer to make sure the isLiveML call gets the 
  // right PartEdge
  bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

// private:
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge);
  
  // General version of meetUpdate that accounts for framework details before routing the call to the derived class' 
  // meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML 
  // call gets the right PartEdge
  bool meetUpdate(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  virtual bool isFullML(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  virtual bool isEmptyML(PartEdgePtr pedge);
    
  // General versions of isFull() and isEmpty that account for framework details before routing the call to the 
  // derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make 
  // sure the isFullML() and isEmptyML() call gets the right PartEdge.
  // These functions are just aliases for the real implementations in AbstractObject
  bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);
  bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL);

  // Allocates a copy of this object and returns a pointer to it
  virtual MemLocObjectPtr copyML() const;
  
  // Allocates a copy of this object and returns a regular pointer to it
  virtual MemLocObject* copyMLPtr() const;
  
  AbstractObjectPtr copyAO() const
  { return copyML(); }
  
  // Returns true if the given expression denotes a memory location and false otherwise
  static bool isMemExpr(SgExpression* expr)
  { return isSgVarRefExp(expr) || isSgPntrArrRefExp(expr) || isSgPointerDerefExp(expr); }
  
  virtual std::string str(std::string indent=""); // pretty print for the object
}; // MemLocObject

// Special MemLocObject used internally by the framework to associate with the return value of a function.
// This is just a MemLoc dedicated to wrapping FuncResultMemLocRegion
class FuncResultMemLocObject : public MemLocObject
{
  public:
  FuncResultMemLocObject(Function func);
  FuncResultMemLocObject(const FuncResultMemLocObject& that);
  
  std::string str(std::string indent="") const { return "FuncResultMemLocObject"; }
  std::string str(std::string indent="")       { return "FuncResultMemLocObject"; }
  std::string strp(PartEdgePtr pedge, std::string indent="") { return "FuncResultMemLocObject"; }
  
  // Allocates a copy of this object and returns a shared pointer to it
  MemLocObjectPtr copyML() const;
};
typedef boost::shared_ptr<FuncResultMemLocObject> FuncResultMemLocObjectPtr;

// The combination of multiple MemLocObjects. Maintains multiple MemLocObjects and responds to
//   API calls with the most or least accurate response that its constituent objects return, depending
//   on the value of the template parameter defaultMayEq (the default value that mayEqual would return
//   if any constituent MemLocObject returns this value).
// For practical purposes analyses should ensure that different instances of IntersectMemLocObject 
//   are only compared if they include the same types of MemLocObjects in the same order. Otherwise, 
//   the comparisons will be uselessly inaccurate.
template <bool defaultMayEq>
class CombinedMemLocObject : public virtual MemLocObject
{
  public:
  std::list<MemLocObjectPtr> memLocs;
  
  public:
  CombinedMemLocObject(MemLocObjectPtr memLoc) : MemLocObject(NULL) {   memLocs.push_back(memLoc); }
  CombinedMemLocObject(const std::list<MemLocObjectPtr>& memLocs) : MemLocObject(NULL), memLocs(memLocs) {}
  
  const std::list<MemLocObjectPtr>& getMemLocs() const { return memLocs; }
    
  // Creates a new CombinedMemLocObject. Template instantiation is used to ensure that instances of
  // CombinedMemLocObject<true> and CombinedMemLocObject<false> get instantiated
  /*static boost::shared_ptr<CombinedMemLocObject<defaultMayEq> > create(MemLocObjectPtr memLoc);
  static boost::shared_ptr<CombinedMemLocObject<defaultMayEq> > create(const std::list<MemLocObjectPtr>& memLocs);*/
  
  void add(MemLocObjectPtr memLoc);
  
  // Returns whether this object may/must be equal to o within the given Part p
  bool mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge);
  bool mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSetML(MemLocObjectPtr o, PartEdgePtr pedge);
  
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSetML(MemLocObjectPtr o, PartEdgePtr pedge);
  
  // Allocates a copy of this object and returns a pointer to it
  MemLocObjectPtr copyML() const;
  
  // Returns true if this object is live at the given part and false otherwise
  bool isLiveML(PartEdgePtr pedge);
  
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge);
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFullML(PartEdgePtr pedge);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmptyML(PartEdgePtr pedge);
  
  std::string str(std::string indent="");
};
typedef CombinedMemLocObject<false> IntersectMemLocObject;
typedef boost::shared_ptr<IntersectMemLocObject> IntersectMemLocObjectPtr;
typedef CombinedMemLocObject<true> UnionMemLocObject;
typedef boost::shared_ptr<UnionMemLocObject> UnionMemLocObjectPtr;


// Sriram: gcc 4.1.2 complains of undefined references to unused to template functions
// fix: explicit template instantiation
extern template class CombinedMemLocObject<true>;
extern template class CombinedMemLocObject<false>; // not sure if this is needed as there were no errors

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
  virtual std::string str(std::string indent=""); // pretty print for the object

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
