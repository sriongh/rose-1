#include "sage3basic.h"
using namespace std;

#include "const_prop_analysis.h"
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <boost/make_shared.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>
#include "sageInterface.h"
#include "AnalysisAstAttribute.h"

#ifndef DISABLE_SIGHT
using namespace sight;
#endif

#define CPDebugLevel 0
using namespace SageInterface;

#include <cwchar>



namespace fuse {

/***************************
 ***** CPValueLattice ******
 ***************************/

CPValueLatticePtr NULLCPValueLattice;

CPValueLattice::CPValueLattice(PartEdgePtr pedge) :
  Lattice(pedge), FiniteLattice(pedge)
{
  kind = boost::make_shared<ConcreteUninitializedKind>();
}

CPValueLattice::CPValueLattice(ConcreteValueKindPtr kind, PartEdgePtr pedge) :
  Lattice(pedge), FiniteLattice(pedge), kind(kind)
{
}

CPValueLattice::CPValueLattice(const CPValueLattice & that) :
  Lattice(that.latPEdge), FiniteLattice(that.latPEdge)
{
  this->kind = that.kind->copyAOType();
}

ConcreteValueKindPtr CPValueLattice::getKind() const {
  return kind;
}

bool CPValueLattice::setKind(ConcreteValueKindPtr kind) {
  bool modified = (this->kind->getKind() == kind->getKind() &&
                   this->kind->equalSetAO(kind));
  this->kind = kind;
  return modified;
}

void
CPValueLattice::initialize()
{
  // Use the default constructor (implemented above).
  // So nothing to do here.
}

// returns a copy of this lattice
Lattice*
CPValueLattice::copy() const
{
  return new CPValueLattice(*this);
}

// Returns a shared pointer to a newly-allocated copy of this CPValueLatice
CPValueLatticePtr CPValueLattice::copyCPLat() const
{ return boost::make_shared<CPValueLattice>(*this); }


// Overwrites the state of "this" Lattice with "that" Lattice
void
CPValueLattice::copy(Lattice* X)
{
  Lattice::copy(X);
  CPValueLattice* that = dynamic_cast<CPValueLattice*>(X);
  assert(that);

  this->kind = that->kind;
}


bool
CPValueLattice::operator==(Lattice* X) /*const*/
{
  // Implementation of equality operator.
  CPValueLattice* that = dynamic_cast<CPValueLattice*>(X);
  assert(that);
  return (this->kind->getKind() == that->kind->getKind() &&
          this->kind->equalSetAO(that->kind));
}

// Computes the meet of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool
CPValueLattice::meetUpdate(Lattice* X)
{
  CPValueLattice* that = dynamic_cast<CPValueLattice*>(X);
  assert(that);
  return meetUpdate(that);
}

bool
CPValueLattice::meetUpdate(CPValueLattice* that)
{
  pair<bool, ConcreteValueKindPtr> ret = kind->meetUpdateAO(that->kind);
  // Update kind
  kind = ret.second;

  // Return whether kind was modified
  return ret.first;

/*  if(this->kind->getKind()==ConcreteValueKind::uninitialized && )
  if (this->level == emptySet) {
    if(that->level == emptySet) {
      // leave it and return false
      return false;
    } else if (that->level == constantValue) {
      this->level = constantValue;
      this->value = that->value;
      return true;
    } else if (that->level == fullSet) {
      this->level = fullSet;
      return true;
    }
    assert(0); // We should never get here
  } else if (this->level == constantValue) {
    if(that->level == emptySet) {
      return false;
    } else if (that->level == constantValue) {
      if (this->value == that->value) {
        return false;
      } else {
        this->level = fullSet;
        return true;
      }
    } else if (that->level == fullSet) {
      this->level = fullSet;
      return true;
    }
    assert(0); // We should never get here
  } else if (this->level == fullSet) {
    // Already at the fullSet. Cannot go up further.
    return false;
  }

  // Make up a return value for now.
  return false;*/
}

// Computes the intersection of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool
CPValueLattice::intersectUpdate(Lattice* X)
{
  CPValueLattice* that = dynamic_cast<CPValueLattice*>(X);
  assert(that);
  return intersectUpdate(that);
}

bool
CPValueLattice::intersectUpdate(CPValueLattice* that)
{
  pair<bool, ConcreteValueKindPtr> ret = kind->intersectUpdateAO(that->kind);
  // Update kind
  kind = ret.second;

  // Return whether kind was modified
  return ret.first;
}

// Set this Lattice object to represent the set of all possible execution prefixes.
// Return true if this causes the object to change and false otherwise.
bool
CPValueLattice::setToFull()
{
  if(kind->getKind()!=ConcreteValueKind::unknown) {
    kind = boost::make_shared<ConcreteUnknownKind>();
    return true;
  } else
    return false;
}

// Set this Lattice object to represent the of no execution prefixes (empty set)
// Return true if this causes the object to change and false otherwise.
bool
CPValueLattice::setToEmpty()
{
  if(kind->getKind()!=ConcreteValueKind::uninitialized) {
    kind = boost::make_shared<ConcreteUninitializedKind>();
    return true;
  } else
    return false;
}

// Set all the information associated Lattice object with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
bool
CPValueLattice::setMLValueToFull(MemLocObjectPtr ml)
{
  // Do nothing since this object does not contain information about MemLocObjects
  return false;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool
CPValueLattice::isFull()
{ return kind->isFullAO(getPartEdge()); }

// Returns whether this lattice denotes the empty set.
bool
CPValueLattice::isEmpty()
{ return kind->isEmptyAO(getPartEdge()); }

string
CPValueLattice::str(string indent) const
{
  return kind->str(indent);
}

// Applies the given unary or binary operation to this and the given ConcreteValueKind
// Returns:
//    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated ConcreteValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent CPValueLattice's list, return that.
CPValueLatticePtr CPValueLattice::op(SgUnaryOp* op) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (txt()<<"CPValueLattice::op(SgUnaryOp "<<SgNode2Str(op)<<")", scope::medium), 1, CPDebugLevel)
  SIGHT_VERB(dbg << "this="<<str()<<endl, 1, CPDebugLevel)
#endif
  return boost::make_shared<CPValueLattice>(kind->op(op), getPartEdge());
}

CPValueLatticePtr CPValueLattice::op(SgBinaryOp* op, CPValueLatticePtr that) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (txt()<<"CPValueLattice::op(SgBinaryOp "<<SgNode2Str(op)<<")", scope::medium), 1, CPDebugLevel)
  SIGHT_VERB_IF(1, CPDebugLevel)
    dbg << "this="<<str()<<endl;
    dbg << "that="<<(that? that->str(): "NULL")<<endl;
  SIGHT_VERB_FI()
#endif
  if(that) return boost::make_shared<CPValueLattice>(kind->op(op, that->kind), getPartEdge());
  else     return boost::make_shared<CPValueLattice>(kind->op(op, NULLConcreteValueKind), getPartEdge());
}

// Returns a freshly-allocated CPValueObject that communicates the information from this
// Lattice to other analyses
CPValueObjectPtr CPValueLattice::createValueObject()
{ return boost::make_shared<CPValueObject>(shared_from_this()); }

/**************************
 ***** CPValueObject ******
 **************************/

CPValueObjectPtr NULLCPValueObject;

CPValueObject::CPValueObject(CPValueLatticePtr ground) : ValueObject(NULL), ground(ground)
{
}

bool CPValueObject::mayEqualAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return ground->getKind()->mustEqualAO(that->ground->getKind());
}

bool CPValueObject::mustEqualAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return ground->getKind()->mustEqualAO(that->ground->getKind());
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool CPValueObject::equalSetAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return ground->getKind()->equalSetAO(that->ground->getKind());
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool CPValueObject::subSetAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return ground->getKind()->subSetAO(that->ground->getKind());
}

// Computes the meet of this and that and saves the result in this.
// Returns true if this causes this to change and false otherwise.
bool CPValueObject::meetUpdateAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  // Note: this does not change the original lattice since the base of a CPValueObject
  //       always a copy of the original lattice
  return ground->meetUpdate(that->ground.get());
}

// Computes the intersection of this and that and saves the result in this.
// Returns true if this causes this to change and false otherwise.
bool CPValueObject::intersectUpdateAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  // Note: this does not change the original lattice since the base of a CPValueObject
  //       always a copy of the original lattice
  return ground->intersectUpdate(that->ground.get());
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool CPValueObject::isFullAO(PartEdgePtr pedge)
{ return ground->getKind()->isFullAO(pedge); }

// Returns whether this lattice denotes the empty set.
bool CPValueObject::isEmptyAO(PartEdgePtr pedge)
{ return ground->getKind()->isEmptyAO(pedge); }


// Allocates a copy of this object and returns a pointer to it
ValueObjectPtr CPValueObject::copyAOType() const
{
  return boost::make_shared<CPValueObject>(*this);
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool CPValueObject::isConcrete()
{ return ground->getKind()->isConcrete(); }

// Returns the number of concrete values in this set
int CPValueObject::concreteSetSize()
{ return ground->getKind()->concreteSetSize(); }

// Returns the type of the concrete value (if there is one)
SgType* CPValueObject::getConcreteType()
{
  assert(ground->getKind()->asExactKind());
  return ground->getKind()->asExactKind()->getVal()->get_type();
}

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
set<boost::shared_ptr<SgValueExp> > CPValueObject::getConcreteValue()
{
  assert(ground->getKind()->asExactKind());
  set<boost::shared_ptr<SgValueExp> > concreteVals;
  concreteVals.insert(ground->getKind()->asExactKind()->getVal());
  return concreteVals;
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& CPValueObject::getHierKey() const {
  if(!isHierKeyCached) {
    ((CPValueObject*)this)->cachedHierKey = boost::make_shared<AOSHierKey>(((CPValueObject*)this)->shared_from_this());

    // The all object gets an empty key since it contains all the object types
    if(ground->getKind()->getKind()==ConcreteValueKind::unknown) { }
    else {
      ((CPValueObject*)this)->cachedHierKey->add(boost::make_shared<ConcreteValueKind::comparableKind>(ground->getKind()->getKind()));
      ground->getKind()->addHierSubKey(((CPValueObject*)this)->cachedHierKey);

      //dbg << "computed"<<endl;
    }
    ((CPValueObject*)this)->isHierKeyCached = true;
  }
  //dbg << "((CPValueObject*)this)->cachedHierKey="<<((CPValueObject*)this)->cachedHierKey<<endl;
  return cachedHierKey;
}

string CPValueObject::str(string indent) const
{
  return ground->str(indent);
}

string CPValueObject::strp(PartEdgePtr pedge, string indent) const
{
  return ground->str(indent);
}

// *************************
// **** CPMemLocObject *****
// *************************

// returns a copy of this lattice
/*Lattice* CPMemLocObject::copy() const {
  return new CPMemLocObject(*this);
}*/

/*bool
CPMemLocObject::operator==(Lattice* X)
{
  // Implementation of equality operator.
  CPMemLocObject* that = dynamic_cast<CPMemLocObject*>(X);
  assert(that);
  return (this->getRegion() == that->getRegion() &&
          this->getIndex()  == that->getIndex());
}*/

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
/*Lattice* CPMemLocObject::remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge) {
  if(isCPFull || isCPEmpty) return dynamic_cast<CPMemLocObject*>(copyAOTypePtr());

  CPMemLocObject* ret=NULL;
  for(std::set<MLMapping>::const_iterator m=ml2ml.begin(); m!=ml2ml.end(); m++) {
    if(m->from->mayEqual(MemLocObject::shared_from_this(), fromPEdge, analysis->getComposer(), analysis)) {
      if(ret==NULL) {
        ret = dynamic_cast<CPMemLocObject*>(m->to->copyMLPtr());
        assert(ret);
      } else
        ret->MemLocObject::meetUpdate(m->to, fromPEdge, analysis->getComposer(), analysis);
    }
  }
  assert(ret);
  return ret;
}*/

// Returns whether this object may/must be equal to o within the given Part p
// These methods are called by composers and should not be called by analyses.
bool CPMemLocObject::mayEqualAO(MemLocObjectPtr o, PartEdgePtr pedge) {
  CPMemLocObjectPtr that = boost::dynamic_pointer_cast<CPMemLocObject>(o);
  assert(that);

  /*scope s("CPMemLocObject::mayEqualML");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that->str()<<endl;*/

  // If either object denotes the set of all memlocs, they're may-equal
  if(isCPFull || that->isCPFull)
    return true;

  // If either object denotes the empty set, they're not may-equal
  if(isCPEmpty || that->isCPEmpty)
    return false;

  // Before we call into the parent class, make sure that it can understand the state of
  // this MemLocObject (a full or empty CPMemLocObject has NULL region and index)
  assert(!isCPFull && !that->isCPFull && !isCPEmpty && !that->isCPEmpty);

  // Compare the region and index directly. We call mayEqualMR here because CP does not implement
  // MemRegions, meaning that we need to thread the logic through the composer to the real implementer.
  // However, values are implemented in CP, meaning that we can call mayEqualV directly
  // dbg << "region->mayEqual()=" << region->mayEqual(that->getRegion(), pedge, analysis->getComposer(), analysis) << endl;
  // dbg << "index->mayEqual()=" << ((!index && !that->index) || index->mayEqualAO(that->getIndex(), pedge)) << endl;
  return region && region->mayEqual(that->getRegion(), pedge, analysis->getComposer(), analysis) &&
         ((!index && !that->index) || index->mayEqualAO(that->getIndex(), pedge));
}

bool CPMemLocObject::mustEqualAO(MemLocObjectPtr o, PartEdgePtr pedge) {
  CPMemLocObjectPtr that = boost::dynamic_pointer_cast<CPMemLocObject>(o);
  assert(that);

  /*scope s("CPMemLocObject::mustEqualML");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that->str()<<endl;*/

  // If either object denotes the set of all memlocs, they're not must-equal since they denote unbounded sets
  if(isCPFull || that->isCPFull)
    return false;

  // If either object denotes the empty set, they're not must-equal, since must equality occurs only for sets of size 1
  if(isCPEmpty || that->isCPEmpty)
    return false;

  // Before we call into the parent class, make sure that it can understand the state of
  // this MemLocObject (a full or empty CPMemLocObject has NULL region and index)
  assert(!isCPFull && !that->isCPFull && !isCPEmpty && !that->isCPEmpty);

  // Compare the region and index directly. We call mustEqualMR here because CP does not implement
  // MemRegions, meaning that we need to thread the logic through the composer to the real implementer.
  // However, values are implemented in CP, meaning that we can call mustEqualV directly
  return region && region->mustEqual(that->getRegion(), pedge, analysis->getComposer(), analysis) &&
         ((!index && !that->index) || index->mustEqualAO(that->getIndex(), pedge));
}

// Returns whether the two abstract objects denote the same set of concrete objects
// These methods are called by composers and should not be called by analyses.
bool CPMemLocObject::equalSetAO(MemLocObjectPtr o, PartEdgePtr pedge) {
  CPMemLocObjectPtr that = boost::dynamic_pointer_cast<CPMemLocObject>(o);
  assert(that);

  // If both objects denote the set of all memlocs, they're the same set
  if(isCPFull && that->isCPFull)
    return true;

  // If only one of the objects denotes the set of all memlocs, they're not the same set
  if(isCPFull || that->isCPFull)
    return false;

  // If both objects denotes the empty set, they're the same set
  if(isCPEmpty && that->isCPEmpty)
    return false;

  // If only one of the objects denotes the empty set, they're not the same set
  if(isCPEmpty && that->isCPEmpty)
    return false;

  // Before we call into the parent class, make sure that it can understand the state of
  // this MemLocObject (a full or empty CPMemLocObject has NULL region and index)
  assert(!isCPFull && !that->isCPFull && !isCPEmpty && !that->isCPEmpty);

  // Compare the region and index directly. We call equalSetMR here because CP does not implement
  // MemRegions, meaning that we need to thread the logic through the composer to the real implementer.
  // However, values are implemented in CP, meaning that we can call equalSetV directly
  return region && region->equalSet(that->getRegion(), pedge, analysis->getComposer(), analysis) &&
         ((!index && !that->index) || index->equalSetAO(that->getIndex(), pedge));
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
// These methods are called by composers and should not be called by analyses.
bool CPMemLocObject::subSetAO(MemLocObjectPtr o, PartEdgePtr pedge) {
  CPMemLocObjectPtr that = boost::dynamic_pointer_cast<CPMemLocObject>(o);
  assert(that);

  // If that object denotes the set of all memlocs, this is a subset of that
  if(that->isCPFull)
    return true;

  // If this denotes the set of all memlocs but that does not, this is not a subset of that
  if(isCPFull)
    return false;

  // If this denotes the empty set, it is a subset of that
  if(isCPEmpty)
    return true;

  // If that denotes the empty set but this does not, this is not a subset of that
  if(that->isCPEmpty)
    return false;

  // Before we call into the parent class, make sure that it can understand the state of
  // this MemLocObject (a full or empty CPMemLocObject has NULL region and index)
  assert(!isCPFull && !that->isCPFull && !isCPEmpty && !that->isCPEmpty);

  // Compare the region and index directly. We call subSetMR here because CP does not implement
  // MemRegions, meaning that we need to thread the logic through the composer to the real implementer.
  // However, values are implemented in CP, meaning that we can call subSetV directly
  return region && region->subSet(that->getRegion(), pedge, analysis->getComposer(), analysis) &&
         ((!index && !that->index) || index->subSetAO(that->getIndex(), pedge));
}

// Returns true if this object is live at the given part and false otherwise
bool CPMemLocObject::isLiveAO(PartEdgePtr pedge) {
  if(isCPFull || isCPEmpty) return true;

/*  // For now all CPMemLocs are live but in the future we may restrict this only to the CFGNode
  // of the Base expression.
  return true;*/

  // Forward the query to the parent analysis
  //return MemLocObject::isLiveAO(pedge);
  return MemLocObject::isLive(pedge, analysis->getComposer(), analysis);
}

// Set this Lattice object to represent the set of all possible execution prefixes.
// Return true if this causes the object to change and false otherwise.
bool CPMemLocObject::setToFull() {
  // NOTE: AbstractObjects do not support this method but we may want to make it a universal requirement.
  if(isCPFull) return false;

  isCPFull  = true;
  isCPEmpty = false;
  region    = NULLMemRegionObject;
  index     = NULLValueObject;
  return true;
}

// Set this Lattice object to represent the of no execution prefixes (empty set).
// Return true if this causes the object to change and false otherwise.
bool CPMemLocObject::setToEmpty() {
  // NOTE: AbstractObjects do not support this method but we may want to make it a universal requirement.
  if(isCPEmpty) return false;

  isCPFull  = false;
  isCPEmpty = true;
  region    = NULLMemRegionObject;
  index     = NULLValueObject;
  return true;
}

/*
// Returns whether this lattice denotes the set of all possible execution prefixes.
bool CPMemLocObject::isFull() {
  return isFullAO(getPartEdge());
}

// Returns whether this lattice denotes the empty set.
bool CPMemLocObject::isEmpty() {
  return isEmptyAO(getPartEdge());
}*/

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool CPMemLocObject::isFullAO(PartEdgePtr pedge) {
  if(isCPFull) return true;

  // Compare the region and index directly. We call isFullMR here because CP does not implement
  // MemRegions, meaning that we need to thread the logic through the composer to the real implementer.
  // However, values are implemented in CP, meaning that we can call isFullV directly
  return region->isFull(pedge, analysis->getComposer(), analysis) &&
         (!index || index->isFullAO(pedge));

}

// Returns whether this AbstractObject denotes the empty set.
bool CPMemLocObject::isEmptyAO(PartEdgePtr pedge) {
  if(isCPEmpty) return true;

  // Compare the region and index directly. We call isEmptyMR here because CP does not implement
  // MemRegions, meaning that we need to thread the logic through the composer to the real implementer.
  // However, values are implemented in CP, meaning that we can call isEmptyV directly
  return region->isEmpty(pedge, analysis->getComposer(), analysis) &&
         (!index || index->isEmptyAO(pedge));
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
// The part of this object is to be used for AbstractObject comparisons.
/*bool CPMemLocObject::meetUpdate(Lattice* that_arg)
{
  CPMemLocObject* that = dynamic_cast<CPMemLocObject*>(that_arg);
  assert(that);
  return meetUpdate(that, getPartEdge(), analysis->getComposer(), analysis);
}*/

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool CPMemLocObject::meetUpdateAO(MemLocObjectPtr that_arg, PartEdgePtr pedge) {
  CPMemLocObjectPtr that = boost::dynamic_pointer_cast<CPMemLocObject>(that_arg);
  assert(that);
/*  return meetUpdate(that.get(), pedge, analysis->getComposer(), analysis);
}


// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool CPMemLocObject::meetUpdate(CPMemLocObject* that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{*/
  /*  scope S("CPMemLocObject::meetUpdate");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that->str()<<endl;*/
  // The regions must be identical
  //assert(getRegion()->equalSet(that->getRegion(), pedge, comp, analysis));

  // If this object is full, don't bother
  if(isCPFull)
    return false;
  // If it is empty, jut copy that over this
  else if(isCPEmpty) {
    if(that->isCPEmpty) return false;

    isCPFull  = that->isCPFull;
    isCPEmpty = that->isCPEmpty;
    region    = that->region;
    index     = that->index;
    return true;
  } else {
    // Meet the regions
    bool modified = getRegion()->meetUpdate(that->getRegion(), pedge, analysis->getComposer(), analysis);

    // Meet the indexes, using a direct call to CPValueObject::meetUpdate since we know that both indexes are
    // CPValueObjects.
    modified = boost::dynamic_pointer_cast<CPValueObject>(getIndex())->meetUpdateAO(
                               boost::dynamic_pointer_cast<CPValueObject>(that->getIndex()), pedge) || modified;

    return modified;
  }
}

/*
// Computes the intersection of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
// The part of this object is to be used for AbstractObject comparisons.
bool CPMemLocObject::intersectUpdate(Lattice* that_arg)
{
  CPMemLocObject* that = dynamic_cast<CPMemLocObject*>(that_arg);
  assert(that);
  return meetUpdate(that, getPartEdge(), analysis->getComposer(), analysis);
}
*/

// Computes the intersection of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool CPMemLocObject::intersectUpdateAO(MemLocObjectPtr that_arg, PartEdgePtr pedge) {
  CPMemLocObjectPtr that = boost::dynamic_pointer_cast<CPMemLocObject>(that_arg);
  assert(that);
/*  return intersectUpdate(that.get(), pedge, analysis->getComposer(), analysis);
}

// Computes the intersection of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool CPMemLocObject::intersectUpdate(CPMemLocObject* that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{*/
  /*  scope S("CPMemLocObject::meetUpdate");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that->str()<<endl;*/
  // The regions must be identical
  //assert(getRegion()->equalSet(that->getRegion(), pedge, comp, analysis));

  // If this object is empty, don't bother
  if(isCPEmpty)
    return false;
  // If it is full, just copy that over this
  else if(isCPFull) {
    if(that->isCPFull) return false;

    isCPFull  = that->isCPFull;
    isCPEmpty = that->isCPEmpty;
    region    = that->region;
    index     = that->index;
    return true;
  } else {
    // Intersect the regions
    bool modified = getRegion()->intersectUpdate(that->getRegion(), pedge, analysis->getComposer(), analysis);

    // Intersect the indexes, using a direct call to CPValueObject::meetUpdate since we know that both indexes are
    // CPValueObjects.
    modified = boost::dynamic_pointer_cast<CPValueObject>(getIndex())->intersectUpdateAO(
                               boost::dynamic_pointer_cast<CPValueObject>(that->getIndex()), pedge) || modified;

    return modified;
  }
}

// Allocates a copy of this object and returns a pointer to it
MemLocObjectPtr CPMemLocObject::copyAOType() const {
  return boost::make_shared<CPMemLocObject>(*this);
}

  // Allocates a copy of this object and returns a regular pointer to it
/*MemLocObject* CPMemLocObject::copyAOTypePtr() const {
  return new CPMemLocObject(*this);
}*/

std::string CPMemLocObject::str(std::string indent) const { // pretty print for the object
  if(isCPFull)       return "[CPMemLocObject: Full]";
  else if(isCPEmpty) return "[CPMemLocObject: Empty]";

  ostringstream oss;
  //oss << "[CPMemLocObject: region="<<(region?region->str(indent+"    "):"NULL");
  oss << "<table><tr><td colspan=\"2\">CPMemLocObject</td></tr>";
  oss << "<tr><td>region:</td><td>"<<(region?region->str():"NULL")<<"</td><tr>";
  if(index) {
    //oss <<", "<<endl<< indent << "             index="<<index->str(indent+"    ");
    oss << "<tr><td>index:</td><td>"<<index->str()<<"</td></tr>";
  }
  //oss <<"]";
  oss << "</table>"<<endl;

  return oss.str();
  return "";
}

// **********************************************************************
//            ConstantPropagationAnalysisTransfer
// **********************************************************************

// Transfer function for logical short-circuit operations: && and ||
/*void ConstantPropagationAnalysisTransfer::transferShortCircuitLogical(SgBinaryOp *sgn)
{
  CPValueObjectPtr arg1Lat, arg2Lat, resLat;
  arg1Lat = getLatticeOperand(sgn, sgn->get_lhs_operand());
  resLat  = getLattice(sgn);

  // If the first operand of the short-circuit operation has a known value and it is sufficient to determine
  // the operation's outcome
  if(arg1Lat->getLevel() == CPValueObject::constantValue &&
     ((isSgAndOp(sgn) && !arg1Lat->getValue()) ||
      (isSgOrOp(sgn)  &&  arg1Lat->getValue()))) {
    resLat->setValue(arg1Lat->getValue());
  // Otherwise, if the second value needs to be read to determine the operation's outcome
  } else {
    // Case 1: arg1's value was known but not sufficient
    if(arg1Lat->getLevel() == CPValueObject::constantValue) {
      arg2Lat = getLatticeOperand(sgn, sgn->get_rhs_operand());
      if(arg2Lat->getLevel() == CPValueObject::constantValue) {
        resLat->setValue(arg2Lat->getValue());
      // If arg2's value is uninitialized, then any value is valid for the result. Use uninitialized
      } else if(arg2Lat->getLevel() == CPValueObject::emptySet) {
        resLat->setToEmpty();
      // If arg2 may have multiple values, then the result may have multiple values
      } else if(arg2Lat->getLevel() == CPValueObject::fullSet) {
        resLat->setToFull();
      } else assert(0);
    // Else if, arg1's value is uninitialized, then anly value is valid for the result. Use arg2's v
    } else if(arg1Lat->getLevel() == CPValueObject::emptySet) {
      resLat->setToEmpty();
    // If arg1 may have multiple values, then the result may have multiple values
    } else if(arg1Lat->getLevel() == CPValueObject::fullSet) {
      resLat->setToFull();
    } else assert(0);
  }
}*/

void ConstantPropagationAnalysisTransfer::visit(SgVarRefExp *vref) {
}

void ConstantPropagationAnalysisTransfer::visit(SgDotExp *dot) {
  SIGHT_VERB_DECL(scope, ("ConstantPropagationAnalysisTransfer::visit(SgDotExp *dot)", scope::medium), 1, CPDebugLevel)

  /*assert(dfInfo[NULLPartEdge].size()==2);

  CodeLocObjectPtr cl = boost::make_shared<CodeLocObject>(getPart(), getCFGNode());
  dbg << "cl="<<cl->str();

  AbstractObjectMap* cl2ml = dynamic_cast<AbstractObjectMap*>(dfInfo[NULLPartEdge][1]);
  assert(cl2ml);*/

  /*MemLocObjectPtr ml = boost::dynamic_pointer_cast<MemLocObject>(cl2ml->get(cl));//composer->Expr2MemLocSelf(dot->get_lhs_operand(), part->inEdgeFromAny(), analysis);
  assert(ml);
  CPMemLocObjectPtr core = boost::dynamic_pointer_cast<CPMemLocObject>(ml);
  assert(core);*/
  /*
  scope s2("visit");
  dbg << "ml="<<(ml?ml->str():"NULL")<<endl;
  UnionMemLocObjectPtr coreUnion = boost::dynamic_pointer_cast<UnionMemLocObject>(ml);
  dbg << "coreUnion="<<(coreUnion?coreUnion->str():"NULL")<<endl;

  const list<MemLocObjectPtr>& coreMLs = coreUnion->getMemLocs();
  CPMemLocObjectPtr core;
  if(coreMLs.size()==1)
    core = boost::dynamic_pointer_cast<CPMemLocObject>(*(coreMLs.begin()));
  else {
    for(list<MemLocObjectPtr>::const_iterator ml=coreMLs.begin(); ml!=coreMLs.end(); ml++) {
      CPMemLocObjectPtr cpML = boost::dynamic_pointer_cast<CPMemLocObject>(*ml);
      if(ml==coreMLs.begin())
        core = boost::dynamic_pointer_cast<CPMemLocObject>(cpML->copyAOType());
      else
        core->meetUpdate(cpML.get(), part->inEdgeFromAny(), analysis->getComposer(), analysis);
    }
  }*/
  /*list<PartEdgePtr> opPartEdges = part->inEdgeFromAny()->getOperandPartEdge(dot, dot->get_lhs_operand());
  CPMemLocObjectPtr core;
  if(opPartEdges.size()==1) {
    PartPtr operandPart = (*opPartEdges.begin())->source();
    dbg << "operandPart="<<operandPart->str()<<endl;

    if(operandPart->CFGNodes().size()==1) {
      CFGNode cn = *operandPart->CFGNodes().begin();
      dbg << "cn="<<CFGNode2Str(cn)<<endl;

      CodeLocObjectPtr lhsCL = boost::make_shared<CodeLocObject>(operandPart, cn);
      dbg << "lhsCL="<<lhsCL->str()<<endl;

      core = boost::dynamic_pointer_cast<CPMemLocObject>(cl2ml->get(lhsCL));
      dbg << "core="<<(core? core->str():"NULL")<<endl;
      assert(core);
    } else assert(0);
  } else assert(0);*/

  //MemLocObjectPtr ml = composer->OperandExpr2MemLoc(dot, dot->get_lhs_operand(), part->inEdgeFromAny(), analysis);
  MemLocObjectPtr ml = analysis->OperandExpr2MemLocUse(dot, dot->get_lhs_operand(), parts.NodeState()->inEdgeFromAny());
#ifndef DISABLE_SIGHT
  dbg << "ml="<<ml->str()<<endl;
#endif
  /*CombinedMemLocObjectPtr mlUnion = boost::dynamic_pointer_cast<CombinedMemLocObject>(ml);
  assert(mlUnion);
  const std::list<MemLocObjectPtr>& mlVals = mlUnion->getMemLocs();
  assert(mlVals.size()==1);
  CPMemLocObjectPtr core = boost::dynamic_pointer_cast<CPMemLocObject>((*mlVals.begin())->project(analysis, parts.NodeState()->inEdgeFromAny(), composer, analysis));
  dbg << "*mlVals.begin()="<<(*mlVals.begin())->str()<<endl;*/
  {
#ifndef DISABLE_SIGHT
       scope s("Project");
#endif

  MemLocObjectPtr projection = ml->project(analysis, parts.NodeState()->inEdgeFromAny(), composer, analysis);
#ifndef DISABLE_SIGHT
  dbg << "projection="<<projection->str()<<endl;
#endif
  }
  CPMemLocObjectPtr core = boost::dynamic_pointer_cast<CPMemLocObject>(ml->project(analysis, parts.NodeState()->inEdgeFromAny(), composer, analysis));
#ifndef DISABLE_SIGHT
  dbg << "core="<<core<<endl;
#endif
  assert(core);

  // Compute the offset into the region of the core MemLoc that results from the dot expression
  CPValueLatticePtr offset = core->getCPIndex()->ground->op(dot, NULLCPValueLattice);
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "offset="<<(offset? offset->str(): "NULL")<<endl, 1, CPDebugLevel)
#endif

  // Update this node's MemLoc to use the same region as core but with the new offset

  /*if(dfInfo[NULLPartEdge][1]) delete dfInfo[NULLPartEdge][1];
  dfInfo[NULLPartEdge][1] = new CPMemLocObject(core->getRegion(), offset, dot, part->inEdgeFromAny(), analysis);
  dbg << "dfInfo[1]="<<dfInfo[NULLPartEdge][1]->str()<<endl;*/
  CPMemLocObjectPtr dotML = boost::make_shared<CPMemLocObject>(core->getRegion(), offset->createValueObject(), dot, parts.NodeState()->inEdgeFromAny(), analysis);
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "dotML="<<(dotML? dotML->str(): "NULL")<<endl, 1, CPDebugLevel)
#endif
  //cl2ml->insert(cl, dotML);
  state.addFact(analysis, 0, new CPMemLocObjectNodeFact(dotML));
}

void ConstantPropagationAnalysisTransfer::visit(SgPntrArrRefExp *paRef) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ConstantPropagationAnalysisTransfer::visit(SgPntrArrRefExp *paRef)", scope::medium), 1, CPDebugLevel)
#endif

  /*assert(dfInfo[NULLPartEdge].size()==2);

  CodeLocObjectPtr cl = boost::make_shared<CodeLocObject>(getPart(), getCFGNode());
  dbg << "cl="<<cl->str();

  AbstractObjectMap* cl2ml = dynamic_cast<AbstractObjectMap*>(dfInfo[NULLPartEdge][1]);
  assert(cl2ml);*/

  // In expression array[i], the memory location denoted by "array"
  /*list<PartEdgePtr> opPartEdges = part->inEdgeFromAny()->getOperandPartEdge(paRef, paRef->get_lhs_operand());
  CPMemLocObjectPtr core;
  if(opPartEdges.size()==1) {
    PartPtr operandPart = (*opPartEdges.begin())->source();
    dbg << "operandPart="<<operandPart->str()<<endl;

    if(operandPart->CFGNodes().size()==1) {
      CFGNode cn = *operandPart->CFGNodes().begin();
      dbg << "cn="<<CFGNode2Str(cn)<<endl;

      CodeLocObjectPtr lhsCL = boost::make_shared<CodeLocObject>(operandPart, cn);
      dbg << "lhsCL="<<lhsCL->str()<<endl;

      core = boost::dynamic_pointer_cast<CPMemLocObject>(cl2ml->get(lhsCL));
      dbg << "core="<<(core? core->str():"NULL")<<endl;
      assert(core);
    } else assert(0);
  } else assert(0);*/
  //MemLocObjectPtr ml = composer->OperandExpr2MemLoc(paRef, paRef->get_lhs_operand(), part->inEdgeFromAny(), analysis);
  // MemLocObjectPtr ml = analysis->OperandExpr2MemLocUse(paRef, paRef->get_lhs_operand(), parts.NodeState()->inEdgeFromAny());
  // SIGHT_VERB(dbg << "ml="<<(ml? ml->str(): "NULL")<<endl, 1, CPDebugLevel)
  // CombinedMemLocObjectPtr mlUnion = boost::dynamic_pointer_cast<CombinedMemLocObject>(ml);
  // assert(mlUnion);
  // const std::list<MemLocObjectPtr>& mlVals = mlUnion->getMemLocs();
  // assert(mlVals.size()==1);
  // CPMemLocObjectPtr core = boost::dynamic_pointer_cast<CPMemLocObject>(*mlVals.begin());
  // assert(core);
  // SIGHT_VERB(dbg << "core="<<(core? core->str(): "NULL")<<endl, 1, CPDebugLevel)

  // // In expression array[i], the value location denoted by "i"
  // ValueObjectPtr val = composer->OperandExpr2Val(paRef, paRef->get_rhs_operand(), parts.NodeState()->inEdgeFromAny(), analysis);
  // SIGHT_VERB(dbg << "val="<<val->str()<<endl, 1, CPDebugLevel)
  // CombinedValueObjectPtr indexUnion = boost::dynamic_pointer_cast<CombinedValueObject>(val);
  // assert(indexUnion);
  // const std::list<ValueObjectPtr>& indexVals = indexUnion->getValues();
  // assert(indexVals.size()==1);
  // CPValueObjectPtr index = boost::dynamic_pointer_cast<CPValueObject>(*indexVals.begin());
  // assert(index);
  // SIGHT_VERB(dbg << "index="<<index->str()<<endl, 1, CPDebugLevel)

  // // Compute the offset into the region of the core MemLoc that results from the arraypntr reference expression
  // CPValueLatticePtr offset = core->getCPIndex()->ground->op(paRef, index->ground);
  // SIGHT_VERB(dbg << "offset="<<offset->str()<<endl, 1, CPDebugLevel)

  // CPMemLocObjectPtr paRefML = boost::make_shared<CPMemLocObject>(core->getRegion(), offset->createValueObject(), paRef, parts.NodeState()->inEdgeFromAny(), analysis);
  // SIGHT_VERB(dbg << "paRefML="<<(paRefML? paRefML->str(): "NULL")<<endl, 1, CPDebugLevel)
  // //cl2ml->insert(cl, paRefML);
  // state.addFact(analysis, 0, new CPMemLocObjectNodeFact(paRefML));
}

void ConstantPropagationAnalysisTransfer::visit(SgBinaryOp *sgn) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ConstantPropagationAnalysisTransfer::visit(SgBinaryOp)", scope::medium), 1, CPDebugLevel)
#endif
  //struct timeval vStart, vEnd; gettimeofday(&vStart, NULL);

  // Only bother to consider operators with short-circuiting a the end of the operator so that
  // all of its operands precede the operator
  if((isSgAndOp(sgn) || isSgOrOp(sgn)) && cn.getIndex()!=2) return;

  CPValueLatticePtr arg1Lat, arg2Lat;//, resLat_tmp;
  //struct timeval gStart, gEnd; gettimeofday(&gStart, NULL);
  getLattices(sgn, arg1Lat, arg2Lat);//, resLat_tmp);
  //gettimeofday(&gEnd, NULL); cout << "ConstantPropagationAnalysisTransfer::visit(SgBinaryOp) getLattice\t"<<(((gEnd.tv_sec*1000000 + gEnd.tv_usec) - (gStart.tv_sec*1000000 + gStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(sgn)<<endl;

  //struct timeval oStart, oEnd; gettimeofday(&oStart, NULL);
  CPValueLatticePtr resLat = arg1Lat->op(sgn, arg2Lat);
  //gettimeofday(&oEnd, NULL); cout << "ConstantPropagationAnalysisTransfer::visit(SgBinaryOp) op\t"<<(((oEnd.tv_sec*1000000 + oEnd.tv_usec) - (oStart.tv_sec*1000000 + oStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(sgn)<<endl;

  //struct timeval sStart, sEnd; gettimeofday(&sStart, NULL);
  setLattice(sgn, resLat);
  //gettimeofday(&sEnd, NULL); cout << "ConstantPropagationAnalysisTransfer::visit(SgBinaryOp) setLattice\t"<<(((sEnd.tv_sec*1000000 + sEnd.tv_usec) - (sStart.tv_sec*1000000 + sStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(sgn)<<endl;

  if(isSgCompoundAssignOp(sgn)) {
    setLatticeOperand(sgn, sgn->get_lhs_operand(), resLat->copyCPLat());
    addFuseAnnot(isSgCompoundAssignOp(sgn), resLat, analysis);
  }
  //dbg << "after setLattice modified="<<modified<<endl;


  //gettimeofday(&vEnd, NULL); cout << "ConstantPropagationAnalysisTransfer::visit(SgBinaryOp)\t"<<(((vEnd.tv_sec*1000000 + vEnd.tv_usec) - (vStart.tv_sec*1000000 + vStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(sgn)<<endl;
}

// Unary ops that update the operand
void ConstantPropagationAnalysisTransfer::visit(SgMinusMinusOp *sgn) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ConstantPropagationAnalysisTransfer::visit(SgMinusMinusOp)", scope::medium), 1, CPDebugLevel)
#endif

  CPValueLatticePtr arg1Lat;//, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
  CPValueLatticePtr resLat = arg1Lat->op(sgn);
//prodLat->setToEmpty();
  setLattice(sgn, resLat);
  setLatticeOperand(sgn, sgn->get_operand(), resLat);
}

void ConstantPropagationAnalysisTransfer::visit(SgPlusPlusOp *sgn) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ConstantPropagationAnalysisTransfer::visit(SgPlusPlusOp)", scope::medium), 1, CPDebugLevel)
#endif

  CPValueLatticePtr arg1Lat;//, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
  CPValueLatticePtr resLat = arg1Lat->op(sgn);
//prodLat->setToEmpty();
  setLattice(sgn, resLat);
  setLatticeOperand(sgn, sgn->get_operand(), resLat);
}

// Unary ops that do not update the operand
// void ConstantPropagationAnalysisTransfer::visit(SgUnaryOp *sgn) {
//   scope s("ConstantPropagationAnalysisTransfer::visit(SgUnaryOp)", scope::medium, attrGE("CPDebugLevel", 1));

//   CPValueLatticePtr arg1Lat;//, arg2Lat;//, resLat_tmp;
//   getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
//   CPValueLatticePtr resLat = arg1Lat->op(sgn);
// //prodLat->setToEmpty();
//   setLattice(sgn, resLat);
// }

// Unary ops that do not update the operand
void ConstantPropagationAnalysisTransfer::visit(SgCastExp *sgn) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ConstantPropagationAnalysisTransfer::visit(SgCastExp)", scope::medium), 1, CPDebugLevel)
#endif

  CPValueLatticePtr arg1Lat;//, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
  CPValueLatticePtr resLat = arg1Lat->op(sgn);
//prodLat->setToEmpty();
  setLattice(sgn, resLat);
}

// Unary ops that do not update the operand
void ConstantPropagationAnalysisTransfer::visit(SgMinusOp *sgn) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ConstantPropagationAnalysisTransfer::visit(SgMinusOp)", scope::medium), 1, CPDebugLevel)
#endif

  CPValueLatticePtr arg1Lat;//, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
  CPValueLatticePtr resLat = arg1Lat->op(sgn);
//prodLat->setToEmpty();
  setLattice(sgn, resLat);
}

// Unary ops that do not update the operand
void ConstantPropagationAnalysisTransfer::visit(SgNotOp *sgn) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ConstantPropagationAnalysisTransfer::visit(SgNotOp)", scope::medium), 1, CPDebugLevel);
#endif

  CPValueLatticePtr arg1Lat;//, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
  CPValueLatticePtr resLat = arg1Lat->op(sgn);
//prodLat->setToEmpty();
  setLattice(sgn, resLat);
}

void ConstantPropagationAnalysisTransfer::visit(SgValueExp *val) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB(scope reg("ConstantPropagationAnalysisTransfer::visit(SgValExp)", scope::low), 1, CPDebugLevel)
#endif
  assert(val);

//prodLat->setToEmpty();
  SgTreeCopy copyHelp;
  boost::shared_ptr<SgValueExp> valCopy((SgValueExp*)(val->copy(copyHelp)));
  setLattice(val, boost::make_shared<CPValueLattice>(boost::make_shared<ConcreteExactKind>(valCopy), parts.NodeState()->inEdgeFromAny()));
}

set<SgBoolValExp*> transform(boost::function<SgBoolValExp* (SgNode*)> f,
                             set<boost::shared_ptr<SgValueExp> >& svalues) {
  set<SgBoolValExp*> bvalues;
  set<boost::shared_ptr<SgValueExp> >::iterator s = svalues.begin();
  for( ; s != svalues.end(); ++s) {
    SgValueExp* sgval = (*s).get();
    SgBoolValExp* sgboolval = f(sgval);
    bvalues.insert(sgboolval);
  }
  return bvalues;
}

bool reduce(set<SgBoolValExp*> bvalues) {
  set<SgBoolValExp*>::iterator b = bvalues.begin();
  bool outcome = (*b)->get_value(); ++b;
  for( ; b != bvalues.end(); ++b) {
    outcome = outcome && (*b)->get_value();
  }
  return outcome;
}

void ConstantPropagationAnalysisTransfer::visit(SgConditionalExp* sgn) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB(scope reg("ConstantPropagationAnalysisTransfer::visit(SgConditionalExp)", scope::low), 1, CPDebugLevel)
#endif
  SgExpression* cexp = sgn->get_conditional_exp();
  SgExpression* texp = sgn->get_true_exp();
  SgExpression* fexp = sgn->get_false_exp();

  CPValueLatticePtr clat, tlat, flat;
  clat = getLatticeOperand(sgn, cexp);
  tlat = getLatticeOperand(sgn, texp);
  flat = getLatticeOperand(sgn, fexp);
  
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "clat=" << clat->str() << endl, 1, CPDebugLevel)
  SIGHT_VERB(dbg << "tlat=" << tlat->str() << endl, 1, CPDebugLevel)
  SIGHT_VERB(dbg << "flat=" << flat->str() << endl, 1, CPDebugLevel)
#endif

    ConcreteValueKindPtr ckind = clat->getKind();
  // if we know the value of the conditional expression
  if(ckind->isConcrete()) {
    ConcreteExactKindPtr outcomeKind = ckind->asExactKind();
    boost::function<SgBoolValExp* (SgNode*)> f = (SgBoolValExp*(*) (SgNode*))isSgBoolValExp;
    set<boost::shared_ptr<SgValueExp> > svalues = outcomeKind->getConcreteValue();
    set<SgBoolValExp*> bvalues = transform(f, svalues); 
    bool outcome = reduce(bvalues);
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "outcome=" << outcome << endl, 1, CPDebugLevel)
#endif
    if(outcome) setLattice(sgn, tlat);
    else setLattice(sgn, flat);
  }
  else {
    CPValueLatticePtr rlat = tlat->copyCPLat();
    rlat->meetUpdate(flat.get());
    setLattice(sgn, rlat);
  }
  modified = true;
}


ConstantPropagationAnalysisTransfer::ConstantPropagationAnalysisTransfer(
          AnalysisParts& parts, CFGNode cn, NodeState& state,
          map<PartEdgePtr, vector<Lattice*> >& dfInfo,
          Composer* composer, ConstantPropagationAnalysis* analysis)
   : VariableStateTransfer<CPValueLattice, ConstantPropagationAnalysis>
                       (state, dfInfo, boost::make_shared<CPValueLattice>(parts.NodeState()->inEdgeFromAny()),
                        composer, analysis, parts, cn,
                        CPDebugLevel, "CPDebugLevel")
{
}




// **********************************************************************
//             ConstantPropagationAnalysis
// **********************************************************************

// GB: Is this needed for boost shared-pointers?
ConstantPropagationAnalysis::ConstantPropagationAnalysis(bool useSSA) : FWDataflow(/*trackBase2RefinedPartEdgeMapping*/ false, useSSA)
{
}

// Creates a basic CPMemLocObject for the given SgNode. This object does not take into
// account any constant propagation and will be used as a seed from which to propagate
// more precise information.
CPMemLocObjectPtr ConstantPropagationAnalysis::createBasicCPML(SgNode* n, PartEdgePtr pedge) {
//  struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);
  //scope s("ConstantPropagationAnalysis::createBasicCPML");
  //dbg << "pedge="<<pedges.NodeState()->str()<<endl;
  MemRegionObjectPtr curMR = composer->Expr2MemRegion(n, pedge, this);
  //dbg << "curMR="<<curMR->str()<<endl;
//  gettimeofday(&gopeEnd, NULL); cout << "              ConstantPropagationAnalysis::createBasicCPML Expr2MemRegion\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;

/*  // If this expression denotes the starting point of a memory region, create a MemLocObject
  // that is explicitly at the start of mr
  CPMemLocObjectPtr ml;
  if(isSgVarRefExp(n) || isSgInitializedName(n))
    return boost::make_shared<CPMemLocObject>(
                                curMR, boost::make_shared<CPValueLattice>(
                                           boost::make_shared<ConcreteOffsetListKind>(ConcreteOffsetListKind::offset(0)),
                                           pedge)->createValueObject(),
                                n, pedge, this);
  // Otherwise, create one that refers to an unknown offset within mr
  else
    return boost::make_shared<CPMemLocObject>(
                                curMR, boost::make_shared<CPValueLattice>(
            // !!! Should create ServerImplKind here!!!
                                           boost::make_shared<ConcreteOffsetListKind>(ConcreteOffsetListKind::offset(0)),
                                           pedge)->createValueObject(),
                                       // boost::make_shared<CPValueObject>(boost::make_shared<ConcreteUninitializedKind>(), pedge),
                                n, pedge, this);
*/
  // Create a MemLocObject that refers to the first offset within the MemRegion. Cache the CPMemLocObject
  // associated with MemRegionObject.
 // struct timeval cacheStart, cacheEnd; gettimeofday(&cacheStart, NULL);

  CPMemLocObjectPtr res =
        boost::make_shared<CPMemLocObject>(
                              curMR, boost::make_shared<CPValueLattice>(
                                         boost::make_shared<ConcreteOffsetListKind>(ConcreteOffsetListKind::offset(0)),
                                         pedge)->createValueObject(),
                              n, pedge, this);
//  gettimeofday(&cacheEnd, NULL); cout << "              ConstantPropagationAnalysis::createBasicCPML Not In Cache\t"<<(((cacheEnd.tv_sec*1000000 + cacheEnd.tv_usec) - (cacheStart.tv_sec*1000000 + cacheStart.tv_usec)) / 1000000.0)<<endl;

  return res;
}

// Initializes the state of analysis lattices at the given function, part and edge into our out of the part
// by setting initLattices to refer to freshly-allocated Lattice objects.
void ConstantPropagationAnalysis::genInitLattice(const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                                                 vector<Lattice*>& initLattices)
{
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (txt()<<"ConstantPropagationAnalysis::genInitLattice(parts.NodeState()="<<parts.NodeState()->str()<<")", scope::medium), 1, CPDebugLevel)
#endif
  AbstractObjectMap* ml2val = new AbstractObjectMap(boost::make_shared<CPValueLattice>(pedges.NodeState()),
                                                    pedges.NodeState(),
                                                    getComposer(), this);
  /*dbg << "ConstantPropagationAnalysis::initializeState, analysis="<<returning l="<<l<<" n=<"<<escape(p.getNode()->unparseToString())<<" | "<<p.getNode()->class_name()<<" | "<<p.getIndex()<<">\n";
  dbg << "    l="<<l->str("    ")<<endl;*/
  initLattices.push_back(ml2val);

  //initLattices.push_back(new CPMemLocObject(NULLMemRegionObject, NULLCPValueObject, NULL, pedge, this));

  /*
  // The second lattice represents the memory location denoted by the current CFGNode
  assert(part->CFGNodes().size()==1);
  CFGNode n = *(part->CFGNodes().begin());

  MemRegionObjectPtr mr = composer->Expr2MemRegion(n.getNode(), part->inEdgeFromAny(), this);
  CPMemLocObject* ml;
  scope s("ConstantPropagationAnalysis::genInitLattice()", scope::medium, attrGE("CPDebugLevel", 1));

  // If this expression denotes the starting point of a memory region, create a MemLocObject
  // that is explicitly at the start of mr
  if(isSgVarRefExp(n.getNode()))
    ml = new CPMemLocObject(mr, boost::make_shared<CPValueObject>(
                                         boost::make_shared<ConcreteExactKind>(
                                                  boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(0))), pedge),
                            n.getNode(), pedge, this);
  // Otherwise, create one that refers to an unknown offset within mr
  else
    ml = new CPMemLocObject(mr, boost::make_shared<CPValueObject>(boost::make_shared<ConcreteUninitializedKind>(), pedge),
                            n.getNode(), pedge, this);

  dbg << "new MemLoc post:"<<ml->str()<<endl;
  initLattices.push_back(ml);*/

  /*AbstractObjectMap* cl2ml = new AbstractObjectMap(boost::make_shared<CPMemLocObject>(false, true, (SgNode*)NULL, pedge, this),
                                                   pedge,
                                                   getComposer(), this);
  const set<CFGNode>& nodes = part->CFGNodes();
  for(set<CFGNode>::const_iterator n=nodes.begin(); n!=nodes.end(); n++) {
    CodeLocObjectPtr curCL = boost::make_shared<CodeLocObject>(part, *n);
    cl2ml->insert(curCL, ConstantPropagationAnalysis::createBasicCPML(n->getNode(), part->inEdgeFromAny()));
  }

  dbg << "cl2ml:"<<cl2ml->str()<<endl;
  initLattices.push_back(cl2ml);*/
}

bool
ConstantPropagationAnalysis::transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                                      map<PartEdgePtr, vector<Lattice*> >& dfInfo)
{
  assert(0);
  return false;
}

boost::shared_ptr<DFTransferVisitor>
ConstantPropagationAnalysis::getTransferVisitor(AnalysisParts& parts, CFGNode cn, NodeState& state,
                                                map<PartEdgePtr, vector<Lattice*> >& dfInfo)
{
  // Why is the boost shared pointer used here?
  ConstantPropagationAnalysisTransfer* t = new ConstantPropagationAnalysisTransfer(parts, cn, state, dfInfo, getComposer(), this);
  return boost::shared_ptr<DFTransferVisitor>(t);
}

ValueObjectPtr ConstantPropagationAnalysis::Expr2Val(SgNode* n, PartEdgePtr pedge)
{
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (txt()<<"ConstantPropagationAnalysis::Expr2Val(n="<<SgNode2Str(n)<<", pedge="<<pedge->str()<<")", scope::medium), 1, CPDebugLevel)
#endif

  //MemLocObjectPtr ml = getComposer()->Expr2MemLoc(n, pedge, this);
  MemLocObjectPtr ml = Expr2MemLocUse(n, pedge);
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "ml="<<(ml? ml->str(): "NULL")<<endl, 1, CPDebugLevel)
#endif

  AnalysisPartEdges pedges = NodeState2All(pedge);

  // If pedge doesn't have wildcards
  //dbg << "source="<<pedges.NodeState()->source()->str()<<endl;
  //dbg << "target="<<pedges.NodeState()->target()->str()<<endl;
  if(pedges.NodeState()->source() && pedges.NodeState()->target()) {
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, (useSSA? NULLPart: pedges.NodeState()->source()));
    // cout << "state="<<state->str(this)<<endl;
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "state="<<state->str(this)<<endl, 1, CPDebugLevel)
#endif

    // Get the value map at the current edge
    AbstractObjectMap* cpMap =
              useSSA? dynamic_cast<AbstractObjectMap*>(state->getLatticeAbove(this, NULLPartEdge, 0)) :
                      dynamic_cast<AbstractObjectMap*>(state->getLatticeBelow(this, pedges.index(), 0));
    if(cpMap == NULL) {
      Lattice* l = useSSA? state->getLatticeBelow(this, NULLPartEdge, 0) :
                           state->getLatticeBelow(this, pedges.index(),0);
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "pedges.index()="<<pedges.index()->str()<<endl, 1, CPDebugLevel)
      SIGHT_VERB(dbg << "l="<<(l?l->str():"NULL")<<endl, 1, CPDebugLevel)
#endif
    }
    assert(cpMap);

    // We currently can only handle requests for the SgNode that corresponds to the current Part
    set<CFGNode> nodes = pedges.NodeState()->source()->CFGNodes();
    assert(nodes.size()==1);
//    assert(nodes.begin()->getNode() == n);

#ifndef DISABLE_SIGHT
    // Get the MemLoc at the source part
    SIGHT_VERB_IF(1, CPDebugLevel)
      indent ind;
      dbg << "cpMap="<<cpMap<<"="<<cpMap->str()<<endl;
    SIGHT_VERB_FI()
#endif

    // Return the lattice associated with n's expression
    CPValueLatticePtr val = boost::dynamic_pointer_cast<CPValueLattice>(cpMap->get(ml));
    assert(val);
    // cout << val->str() << endl;
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "val="<<val->str()<<endl, 1, CPDebugLevel)
#endif

    return val->copyCPLat()->createValueObject();
  // If the target of this edge is a wildcard
  } else if(pedges.NodeState()->source()) {
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->source());
    //dbg << "state="<<state->str(this)<<endl;
    // cout << "state="<<state->str(this)<<endl;

    map<PartEdgePtr, vector<Lattice*> >& e2lats = state->getLatticeBelowAllMod(this);
    assert(e2lats.size()>=1);
    CPValueLatticePtr mergedLat;
    for(map<PartEdgePtr, vector<Lattice*> >::iterator lats=e2lats.begin(); lats!=e2lats.end(); lats++) {
      PartEdgePtr indexEdge = lats->first;
      assert(indexEdge->source() == pedges.index()->source());
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt()<<"edge "<<lats->first.get()->str(), scope::medium), 1, CPDebugLevel)
#endif

      // Get the value map at the current edge
      AbstractObjectMap* cpMap = dynamic_cast<AbstractObjectMap*>(state->getLatticeBelow(this, lats->first, 0));
      assert(cpMap);

      //MemLocObjectPtr p = composer->Expr2MemLoc(n, pedge, this);
      // We currently can only handle requests for the SgNode that corresponds to the current Part
      set<CFGNode> nodes = pedges.NodeState()->source()->CFGNodes();
      assert(nodes.size()==1);
      assert(nodes.begin()->getNode() == n);

#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(2, CPDebugLevel)
        indent ind;
        dbg << "cpMap="<<cpMap<<"="<<cpMap->str()<<endl;
      SIGHT_VERB_FI()
#endif

      CPValueLatticePtr val = boost::dynamic_pointer_cast<CPValueLattice> (boost::dynamic_pointer_cast<ValueObject>(cpMap->get(ml)));
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "val="<<val->str()<<endl, 1, CPDebugLevel)
	// cout << val->str() << endl;
#endif

        if(lats==e2lats.begin())
        mergedLat = val->copyCPLat();
      else
        mergedLat->meetUpdate(val.get());
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "mergedLat="<<mergedLat->str()<<endl, 1, CPDebugLevel)
#endif
    }
    return mergedLat->createValueObject();

  // If the source of this edge is a wildcard
  } else if(pedges.NodeState()->target()) {
    // Get the NodeState at the target of this edge
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->target());
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "state="<<state->str()<<endl, 2, CPDebugLevel)
#endif
    // Get the value map at the NULL edge, which denotes the meet over all incoming edges
    AbstractObjectMap* cpMap = dynamic_cast<AbstractObjectMap*>(state->getLatticeAbove(this, pedges.index(), 0));
    assert(cpMap);
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(2, CPDebugLevel)
      indent ind;
      dbg << "cpMap="<<cpMap<<"="<<cpMap->str()<<endl;
    SIGHT_VERB_FI()
#endif
    // Return the lattice associated with n's expression since that is likely to be more precise
    CPValueLatticePtr val = boost::dynamic_pointer_cast<CPValueLattice>(cpMap->get(ml));
    // cout << val->str() << endl;
    assert(val);
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "val="<<val->str()<<endl, 1, CPDebugLevel)
#endif

    return val->copyCPLat()->createValueObject();
  }
  assert(0);
}

MemLocObjectPtr ConstantPropagationAnalysis::Expr2MemLoc(SgNode* n, PartEdgePtr pedge) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (txt()<<"ConstantPropagationAnalysis::Expr2MemLoc(n="<<SgNode2Str(n)<<", pedge="<<pedge->str()<<")", scope::medium), 1, CPDebugLevel)
#endif

  //struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);

  AnalysisPartEdges pedges = NodeState2All(pedge);
  assert(false);
  
  // SgInitializedNames denote entities that are lexically known and thus do not require
  // any special handling by ConstantPropagation Analysis
  //if(isSgInitializedName(n) || isSgVarRefExp(n)) {
  if(!isSgDotExp(n) && !isSgPntrArrRefExp(n)) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "Creating basic CPML"<<endl, 1, CPDebugLevel)
#endif
    MemLocObjectPtr ret = createBasicCPML(n, pedge);
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "ret = "<<ret->str()<<endl, 1, CPDebugLevel)
#endif
    //gettimeofday(&gopeEnd, NULL); cout << "            ConstantPropagationAnalysis::Expr2MemLoc\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
    return ret;
  }

  // NOTE: this is a temporary hack where we assume the appropriate index for the CFGNode
  //       that represents SgNode n. In the future we should change Expr2* to accept CFGNodes
  CFGNode cn;
       if(isSgBinaryOp(n) ||
          isSgUnaryOp(n))     cn = CFGNode(n, 2);
  else if(isSgValueExp(n))    cn = CFGNode(n, 1);
  else                        cn = CFGNode(n, 0);

  // Confirm that n corresponds to the source part
  if(pedges.NodeState()->source()) {
    assert(pedges.NodeState()->source()->CFGNodes().size()==1);
    //assert(pedges.NodeState()->source()->CFGNodes().begin()->getNode() == n);
  } /*else if(pedges.NodeState()->target()) {
    assert(pedges.NodeState()->target()->CFGNodes().size()==1);
    assert(pedges.NodeState()->target()->CFGNodes().begin()->getNode() == n);
  }*/

  if(pedges.NodeState()->source()) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"Source: "<<pedges.NodeState()->source()->str(), scope::medium), 2, CPDebugLevel)
#endif
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->source());
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "state="<<state->str()<<endl, 2, CPDebugLevel)
#endif
  }

  if(pedges.NodeState()->target()) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"Target: "<<pedges.NodeState()->target()->str(), scope::medium), 2, CPDebugLevel)
#endif
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->target());
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "state="<<state->str()<<endl, 2, CPDebugLevel)
#endif
  }

  // If pedge doesn't have wildcards
  if(pedges.NodeState()->source() && pedges.NodeState()->target()) {
    /*CodeLocObjectPtr cl = boost::make_shared<CodeLocObject>(pedges.NodeState()->source(), cn);
    SIGHT_VERB(dbg << "cl="<<cl->str()<<endl, 1, CPDebugLevel)*/

    //struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->source());
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "state="<<state->str()<<endl, 3, CPDebugLevel)
#endif

    /* // Get the memory location at the current edge
    AbstractObjectMap* cl2ml = dynamic_cast<AbstractObjectMap*>(state->getLatticeBelow(this, pedges.NodeState()->getSupersetPartEdge(), 1));
    assert(cl2ml);

    // Get the memory location at the current edge
    CPMemLocObjectPtr ml = boost::dynamic_pointer_cast<CPMemLocObject>(cl2ml->get(cl));*/
    // NodeFact* mlFact = state->getFact(this, 0);
    // assert(mlFact);
    // dbg << "mlFact="<<mlFact->str()<<endl;
    // CPMemLocObjectNodeFact* cpmlFact = dynamic_cast<CPMemLocObjectNodeFact*>(mlFact);
    // assert(cpmlFact);
    // CPMemLocObjectPtr ml = cpmlFact->ml;
    // assert(ml);
    // SIGHT_VERB(dbg << "ml="<<ml->str()<<endl, 1, CPDebugLevel)

    //gettimeofday(&gopeEnd, NULL); cout << "            ConstantPropagationAnalysis::Expr2MemLoc getFact\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;

    /*struct timeval copyStart, copyEnd; gettimeofday(&copyStart, NULL);
    MemLocObjectPtr mlCp = ml->copyAOType();
    gettimeofday(&copyEnd, NULL); cout << "ConstantPropagationAnalysis::Expr2MemLoc copyAOType\t"<<(((copyEnd.tv_sec*1000000 + copyEnd.tv_usec) - (copyStart.tv_sec*1000000 + copyStart.tv_usec)) / 1000000.0)<<endl;

    return mlCp;*/
    // return ml->copyAOType();
  // If the target of this edge is a wildcard
  } else if(pedges.NodeState()->source()) {
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->source());
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "state="<<state->str()<<endl, 2, CPDebugLevel)
#endif

    map<PartEdgePtr, vector<Lattice*> >& e2lats = state->getLatticeBelowAllMod(this);
    assert(e2lats.size()>=1);
    CPMemLocObjectPtr mergedML;
    for(map<PartEdgePtr, vector<Lattice*> >::iterator lats=e2lats.begin(); lats!=e2lats.end(); lats++) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt()<<"edge "<<lats->first.get()->str(), scope::medium), 1, CPDebugLevel)
#endif
      PartEdgePtr indexEdge = lats->first;
      assert(indexEdge.get()->source() == pedges.index()->source());

      // NOTE: for now we're assuming that the CFGNode index is 0 but this will need to be corrected
      /*CodeLocObjectPtr cl = boost::make_shared<CodeLocObject>(pedges.NodeState()->source(), cn);
      SIGHT_VERB(dbg << "cl="<<cl->str(), 1, CPDebugLevel)*/

      // Get the memory location at the current edge
      /*AbstractObjectMap* cl2ml = dynamic_cast<AbstractObjectMap*>(state->getLatticeBelow(this, indexEdge, 1));
      assert(cl2ml);

      // Get the memory location at the current edge
      CPMemLocObjectPtr ml = boost::dynamic_pointer_cast<CPMemLocObject>(cl2ml->get(cl));*/
      // NodeFact* mlFact = state->getFact(this, 0);
      // CPMemLocObjectNodeFact* cpmlFact = dynamic_cast<CPMemLocObjectNodeFact*>(mlFact);
      // assert(cpmlFact);
      // CPMemLocObjectPtr ml = cpmlFact->ml;
      // assert(ml);
      // SIGHT_VERB(dbg << "ml="<<ml->str()<<endl, 1, CPDebugLevel)

      // if(lats==e2lats.begin())
      //   mergedML = boost::dynamic_pointer_cast<CPMemLocObject>(ml->copyAOType());
      // else
      //   mergedML->meetUpdate((MemLocObjectPtr)ml, indexEdge, getComposer(), this);

      // SIGHT_VERB(dbg << "mergedML="<<mergedML->str()<<endl, 1, CPDebugLevel)
    }
    return mergedML;
  // If the source of this edge is a wildcard
  } else if(pedges.NodeState()->target()) {
    // Run the query on all the incoming edges and return the union of the results
    MemLocObjectPtr unionML;
    list<PartEdgePtr> inE = pedges.NodeState()->target()->inEdges();
    for(list<PartEdgePtr>::iterator edge=inE.begin(); edge!=inE.end(); ++edge) {
      if(edge==inE.begin()) unionML = Expr2MemLoc(n, *edge)->copyAOType();
      else                  unionML->meetUpdate(Expr2MemLoc(n, *edge), *edge, getComposer(), this);
    }
    assert(unionML);
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "unionML="<<unionML->str()<<endl, 1, CPDebugLevel)
#endif

    return unionML;

/*    // Get the NodeState at the target of this edge
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->target());
    SIGHT_VERB(dbg << "state="<<state->str()<<endl, 3, CPDebugLevel)

    // NOTE: for now we're assuming that the CFGNode index is 0 but this will need to be corrected
    / *CodeLocObjectPtr cl = boost::make_shared<CodeLocObject>(pedges.NodeState()->target(), cn);
    SIGHT_VERB(dbg << "cl="<<cl->str(), 1, CPDebugLevel)* /

    // Get the memory location at the current edge
    / *AbstractObjectMap* cl2ml = dynamic_cast<AbstractObjectMap*>(state->getLatticeAbove(this, NULLPartEdge, 1));
    assert(cl2ml);

    // Get the memory location at the current edge
    CPMemLocObjectPtr ml = boost::dynamic_pointer_cast<CPMemLocObject>(cl2ml->get(cl));* /

    NodeFact* mlFact = state->getFact(this, 0);
    CPMemLocObjectNodeFact* cpmlFact = dynamic_cast<CPMemLocObjectNodeFact*>(mlFact);
    assert(cpmlFact);
    CPMemLocObjectPtr ml = cpmlFact->ml;
    assert(ml);
    SIGHT_VERB(dbg << "ml="<<ml->str()<<endl, 1, CPDebugLevel)

    return ml->copyAOType();*/
  }

  //gettimeofday(&gopeEnd, NULL); cout << "            ConstantPropagationAnalysis::Expr2MemLoc\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;

  // If pedge doesn't have wildcards
  /*if(pedges.NodeState()->source() && pedges.NodeState()->target()) {
    // Confirm that n corresponds to the source part
    assert(pedges.NodeState()->source()->CFGNodes().size()==1);
    assert(pedges.NodeState()->source()->CFGNodes().begin()->getNode() == n);

    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->source());

    // Get the memory location at the current edge
    CPMemLocObject* ml = dynamic_cast<CPMemLocObject*>(state->getLatticeBelow(this, pedges.NodeState()->getSupersetPartEdge(), 1));
    if(ml==NULL) { Lattice* l = state->getLatticeBelow(this, pedges.NodeState()->getSupersetPartEdge(), 1);dbg << "ml="<<(l? l->str(): "NULL")<<endl; }
    assert(ml);

    if(CPDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

    return ml->copyAOType();
  // If the target of this edge is a wildcard
  } else if(pedges.NodeState()->source()) {
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->source());

    map<PartEdgePtr, vector<Lattice*> >& e2lats = state->getLatticeBelowAllMod(this);
    assert(e2lats.size()>=1);
    CPMemLocObjectPtr mergedML;
    for(map<PartEdgePtr, vector<Lattice*> >::iterator lats=e2lats.begin(); lats!=e2lats.end(); lats++) {
      scope sEdge(txt()<<"edge "<<lats->first.get()->str(), scope::medium, attrGE("CPDebugLevel", 1));
      PartEdge* edgePtr = lats->first.get();
      assert(edgePtr->source() == pedge.get()->source());

      // Confirm that n corresponds to the source part
      assert(pedges.NodeState()->source()->CFGNodes().size()==1);
      assert(pedges.NodeState()->source()->CFGNodes().begin()->getNode() == n);

      // Get the value map at the current edge
      CPMemLocObject* ml = dynamic_cast<CPMemLocObject*>(state->getLatticeBelow(this, lats->first, 1));
      if(ml==NULL) { Lattice* l = state->getLatticeBelow(this, pedge, 1);dbg << "ml="<<(l? l->str(): "NULL")<<endl; }
      assert(ml);

      if(CPDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

      if(lats==e2lats.begin())
        mergedML = boost::dynamic_pointer_cast<CPMemLocObject>(ml->copyAOType());
      else
        mergedML->meetUpdate(ml, lats->first, getComposer(), this);

      if(CPDebugLevel()>=1) dbg << "mergedML="<<mergedML->str()<<endl;
    }
    return mergedML;

  // If the source of this edge is a wildcard
  } else if(pedges.NodeState()->target()) {
    // Get the NodeState at the target of this edge
    NodeState* state = NodeState::getNodeState(this, pedges.NodeState()->target());
    if(CPDebugLevel()>=2) dbg << "state="<<state->str()<<endl;

    // Get the value map at the current edge
    CPMemLocObject* ml = dynamic_cast<CPMemLocObject*>(state->getLatticeAbove(this, NULLPartEdge, 1));
    if(ml==NULL) { Lattice* l = state->getLatticeAbove(this, NULLPartEdge, 1);dbg << "ml="<<(l? l->str(): "NULL")<<endl; }
    assert(ml);

    if(CPDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

    return ml->copyAOType();
  }*/
  assert(0);
}

}; // namespace fuse;
