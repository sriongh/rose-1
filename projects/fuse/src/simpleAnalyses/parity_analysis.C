#include "sage3basic.h"
#include "const_prop_analysis.h"

#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <boost/make_shared.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>
using namespace std;


#include <cwchar>

// Define type conversions for lambda operators that are not supported by Boost::Lambda
namespace boost {
namespace lambda {

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, char> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, char, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, short> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, short, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, int, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, long> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, unsigned char> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned char, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, unsigned short> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned short, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, unsigned int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned int, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, unsigned long> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, float> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, float, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, double> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, double, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, long double> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long double, long long int> {
  typedef long long type;
};

//////////////////////////////////////


template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, char> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, char, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, short> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, short, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, int, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, long> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, unsigned long long int> {
  typedef long long type;
};


template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, unsigned char> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned char, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, unsigned short> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned short, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, unsigned int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned int, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, unsigned long> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, float> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, float, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, double> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, double, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, long double> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long double, unsigned long long int> {
  typedef long long type;
};

//////////////////////////////////////



template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, char> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, char, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, short> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, short, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, int, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, long> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, unsigned char> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned char, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, unsigned short> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned short, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, unsigned int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned int, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, unsigned long> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long long int, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, unsigned long long int> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, unsigned long long int, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, float> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, float, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, double> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, double, wchar_t> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, wchar_t, long double> {
  typedef long long type;
};

template<class Act>
struct plain_return_type_2<arithmetic_action<Act>, long double, wchar_t> {
  typedef long long type;
};
}
}

namespace fuse {
DEBUG_LEVEL(constantPropagationAnalysisDebugLevel, 0);

// ************************
// **** CPValueObject *****
// ************************

CPValueObjectPtr NULLCPValueObject;
CPValueKindPtr NULLCPValueKind;

CPValueObject::CPValueObject(PartEdgePtr pedge) :
  Lattice(pedge), FiniteLattice(pedge), ValueObject(NULL)
{
  kind = boost::make_shared<CPUninitializedKind>();
}

/*CPValueObject::CPValueObject(SgValueExp* val, PartEdgePtr pedge) :
  Lattice(pedge), FiniteLattice(pedge), ValueObject(NULL)
{
  kind = boost::make_shared<CPExactKind>();
}*/

CPValueObject::CPValueObject(CPValueKindPtr kind, PartEdgePtr pedge) :
  Lattice(pedge), FiniteLattice(pedge), ValueObject(NULL), kind(kind)
{
}

// This is the same as the implicit definition, so it might not be required to be defined explicitly.
// I am searching for the minimal example of the use of the data flow classes.
CPValueObject::CPValueObject(const CPValueObject & that) : Lattice(that.latPEdge), FiniteLattice(that.latPEdge), ValueObject(that)
{
  this->kind = that.kind;
}

CPValueKindPtr CPValueObject::getKind() const {
  return kind;
}

bool CPValueObject::setKind(CPValueKindPtr kind) {
  bool modified = (this->kind->getKind() == kind->getKind() &&
                   this->kind->equalSetV(kind));
  this->kind = kind;
  return modified;
}

void
CPValueObject::initialize()
{
  // Use the default constructor (implemented above).
  // So nothing to do here.
}


// returns a copy of this lattice
Lattice*
CPValueObject::copy() const
{
  return new CPValueObject(*this);
}


// Overwrites the state of "this" Lattice with "that" Lattice
void
CPValueObject::copy(Lattice* X)
{
  Lattice::copy(X);
  CPValueObject* that = dynamic_cast<CPValueObject*>(X);
  assert(that);

  this->kind = that->kind;
}


bool
CPValueObject::operator==(Lattice* X) /*const*/
{
  // Implementation of equality operator.
  CPValueObject* that = dynamic_cast<CPValueObject*>(X);
  assert(that);
  return (this->kind->getKind() == that->kind->getKind() &&
          this->kind->equalSetV(that->kind));
}

// computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool
CPValueObject::meetUpdate(Lattice* X)
{
  CPValueObject* that = dynamic_cast<CPValueObject*>(X);
  assert(that);
  return meetUpdate(that);
}

bool
CPValueObject::meetUpdate(CPValueObject* that)
{
  pair<bool, CPValueKindPtr> ret = kind->meetUpdateV(that->kind);
  // Update kind
  kind = ret.second;

  // Return whether kind was modified
  return ret.first;

/*  if(this->kind->getKind()==CPValueKind::uninitialized && )
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

// Set this Lattice object to represent the set of all possible execution prefixes.
// Return true if this causes the object to change and false otherwise.
bool
CPValueObject::setToFull()
{
  if(kind->getKind()!=CPValueKind::unknown) {
    kind = boost::make_shared<CPUnknownKind>();
    return true;
  } else
    return false;
}

// Set this Lattice object to represent the of no execution prefixes (empty set)
// Return true if this causes the object to change and false otherwise.
bool
CPValueObject::setToEmpty()
{
  if(kind->getKind()!=CPValueKind::uninitialized) {
    kind = boost::make_shared<CPUninitializedKind>();
    return true;
  } else
    return false;
}

// Set all the information associated Lattice object with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
bool
CPValueObject::setMLValueToFull(MemLocObjectPtr ml)
{
  // Do nothing since this object does not contain information about MemLocObjects
  return false;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool
CPValueObject::isFullLat()
{ return isFullV(getPartEdge()); }

// Returns whether this lattice denotes the empty set.
bool
CPValueObject::isEmptyLat()
{ return isEmptyV(getPartEdge()); }

string
CPValueObject::str(string indent)
{
  return strp(latPEdge, indent);
}

string
CPValueObject::strp(PartEdgePtr pedge, string indent)
{
  return kind->str(indent);
}

// Applies the given unary or binary operation to this and the given CPValueKind
// Returns:
//    - if this CPValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated CPValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent CPValueObject's list, return that.
CPValueObjectPtr CPValueObject::op(SgUnaryOp* op) {
  scope s(txt()<<"CPValueObject::op(SgUnaryOp "<<SgNode2Str(op)<<")", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "this="<<str()<<endl;
  return boost::make_shared<CPValueObject>(kind->op(op), getPartEdge());
}

CPValueObjectPtr CPValueObject::op(SgBinaryOp* op, CPValueObjectPtr that) {
  scope s(txt()<<"CPValueObject::op(SgBinaryOp "<<SgNode2Str(op)<<")", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));
  if(constantPropagationAnalysisDebugLevel()>=1) {
    dbg << "this="<<str()<<endl;
    dbg << "that="<<(that? that->str(): "NULL")<<endl;
  }
  if(that) return boost::make_shared<CPValueObject>(kind->op(op, that->kind), getPartEdge());
  else     return boost::make_shared<CPValueObject>(kind->op(op, NULLCPValueKind), getPartEdge());
}

bool CPValueObject::mayEqualV(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return this->kind->mustEqualV(that->kind);
}

bool CPValueObject::mustEqualV(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return this->kind->mustEqualV(that->kind);
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool CPValueObject::equalSetV(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return this->kind->equalSetV(that->kind);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool CPValueObject::subSetV(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return this->kind->subSetV(that->kind);
}

// Computes the meet of this and that and saves the result in this.
// Returns true if this causes this to change and false otherwise.
bool CPValueObject::meetUpdateV(ValueObjectPtr o, PartEdgePtr pedge)
{
  CPValueObjectPtr that = boost::dynamic_pointer_cast<CPValueObject>(o);
  assert(that);
  return meetUpdate(that.get());
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool
CPValueObject::isFullV(PartEdgePtr pedge)
{ return kind->isFullV(pedge); }

// Returns whether this lattice denotes the empty set.
bool
CPValueObject::isEmptyV(PartEdgePtr pedge)
{ return kind->isEmptyV(pedge); }


// Allocates a copy of this object and returns a pointer to it
ValueObjectPtr CPValueObject::copyV() const
{
  return boost::make_shared<CPValueObject>(*this);
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool CPValueObject::isConcrete()
{ return kind->getKind() == CPValueKind::concrete; }

// Returns the type of the concrete value (if there is one)
SgType* CPValueObject::getConcreteType()
{
  assert(kind->asConcreteKind());
  return kind->asConcreteKind()->getVal()->get_type();
}

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
set<boost::shared_ptr<SgValueExp> > CPValueObject::getConcreteValue()
{
  assert(kind->asConcreteKind());
  set<boost::shared_ptr<SgValueExp> > concreteVals;
  concreteVals.insert(kind->asConcreteKind()->getVal());
  return concreteVals;
}

// ***********************************
// **** CPUninitializedKind *****
// ***********************************

// Applies the given unary or binary operation to this and the given CPValueKind
// Returns:
//    - if this CPValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated CPValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent CPValueObject's list, return that.
CPValueKindPtr CPUninitializedKind::op(SgUnaryOp* op) {
  // Uninitialized denotes the empty set, so any operation applied to it results in the empty set
  return copyV();
}

CPValueKindPtr CPUninitializedKind::op(SgBinaryOp* op, CPValueKindPtr that) {
  // Uninitialized denotes the empty set, so any operation that involves it results in the empty set
  return copyV();
}

// Returns whether this and that CPValueKinds are may/must equal to each other
bool CPUninitializedKind::mayEqualV(CPValueKindPtr that)
{
  // Uninitialized denotes the empty set, which does not overlap with any other set
  return false;
}

bool CPUninitializedKind::mustEqualV(CPValueKindPtr that) {
  // Uninitialized denotes the empty set, which may only be equal to another empty set
  return that->getKind() == CPValueKind::uninitialized;
}

// Returns whether the two CPValueKinds denote the same set of concrete values
bool CPUninitializedKind::equalSetV(CPValueKindPtr that) {
  // Uninitialized denotes the empty set, which may only be equal to another empty set
  return (that->getKind() == CPValueKind::uninitialized);
}

// Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given CPValueKind
bool CPUninitializedKind::subSetV(CPValueKindPtr that) {
  // Uninitialized denotes the empty set, which is a subset of every other set
  return true;
}

// Computes the meet of this and that and returns the resulting kind
pair<bool, CPValueKindPtr> CPUninitializedKind::meetUpdateV(CPValueKindPtr that)
{
  bool modified = that->getKind() != CPValueKind::uninitialized;
  // Uninitialized MEET * => *
  return make_pair(modified, that->copyV());
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool CPUninitializedKind::isConcrete()
{ return false; }

// Returns the type of the concrete value (if there is one)
SgType* CPUninitializedKind::getConcreteType()
{ return NULL; }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > CPUninitializedKind::getConcreteValue()
{ return std::set<boost::shared_ptr<SgValueExp> >(); }

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool CPUninitializedKind::isFullV(PartEdgePtr pedge) { return false; }
// Returns whether this AbstractObject denotes the empty set.
bool CPUninitializedKind::isEmptyV(PartEdgePtr pedge) { return true; }

std::string CPUninitializedKind::str(std::string indent)
{ return "[CPUninitializedKind]"; }

// ******************************
// **** CPConcreteKind *****
// ******************************

// Creates a CPConcreteKind from the given value. This function is overloaded with different argument types
// and for each type it creates a CPConcreteKind with a different SgValueExp.
CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(bool val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(char val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildCharVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(short val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildShortVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(int val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(long val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongIntVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(long long val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(unsigned char val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedCharVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(unsigned short val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedShortVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(unsigned int val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedIntVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(unsigned long val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedLongVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(unsigned long long val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedLongLongIntVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(wchar_t val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildWcharVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(float val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildFloatVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(double val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildDoubleVal(val))); }

CPValueKindPtr CPConcreteKind::createCPValueKindFromVal(long double val)
{ return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongDoubleVal(val))); }


// Applies the given operation functor to the expression in this ConcreteKind and returns the resulting CPKind
template<class DoOpType>
	CPValueKindPtr CPConcreteKind::doUnaryOp(DoOpType& doOp) {
	  switch(exp.get()->variantT()) {
	    case V_SgBoolValExp:
	    case V_SgCharVal:
	    case V_SgShortVal:
	    case V_SgIntVal:
	    case V_SgLongIntVal:
	    case V_SgLongLongIntVal:
	    case V_SgUnsignedCharVal:
	    case V_SgUnsignedIntVal:
	    case V_SgUnsignedLongVal:
	    case V_SgUnsignedLongLongIntVal:
	    case V_SgUnsignedShortVal:
	    case V_SgUpcMythread:
	    case V_SgUpcThreads:
	    case V_SgWcharVal:
	    case V_SgEnumVal:
	      return doUnaryIntegralOp(doOp);

	    case V_SgDoubleVal: case V_SgFloatVal: case V_SgLongDoubleVal: case V_SgComplexVal:
	      return doUnaryFloatOp(doOp);

	    default:
	      // We've enumerated all cases so we should never get here
	      assert(0);
	  }
	}

	// Applies the given operation functor to the expression in this ConcreteKind, whoch is assumed to be an integral type
	// and returns the resulting CPKind
	template<class DoOpType>
	CPValueKindPtr CPConcreteKind::doUnaryIntegralOp(DoOpType& doOp) {
	  switch(exp.get()->variantT()) {
	/*    case V_SgBoolValExp:             return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp            (doOp(isSgBoolValExp            (exp.get())->get_value()))));
	    case V_SgCharVal:                return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildCharVal               (doOp(isSgCharVal               (exp.get())->get_value()))));
	    case V_SgIntVal:                 return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal                (doOp(isSgIntVal                (exp.get())->get_value()))));
	    case V_SgLongIntVal:             return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongIntVal            (doOp(isSgLongIntVal            (exp.get())->get_value()))));
	    case V_SgLongLongIntVal:         return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal        (doOp(isSgLongLongIntVal        (exp.get())->get_value()))));
	    case V_SgUnsignedCharVal:        return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedCharVal       (doOp(isSgUnsignedCharVal       (exp.get())->get_value()))));
	    case V_SgUnsignedIntVal:         return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedIntVal        (doOp(isSgUnsignedIntVal        (exp.get())->get_value()))));
	    case V_SgUnsignedLongVal:        return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedLongVal       (doOp(isSgUnsignedLongVal       (exp.get())->get_value()))));
	    case V_SgUnsignedLongLongIntVal: return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedLongLongIntVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value()))));
	    case V_SgUnsignedShortVal:       return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedShortVal      (doOp(isSgUnsignedShortVal      (exp.get())->get_value()))));
	    case V_SgUpcMythread:            return boost::make_shared<CPUnknownKind>();
	    case V_SgUpcThreads:             return boost::make_shared<CPUnknownKind>();
	    case V_SgWcharVal:               return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildWcharVal              (doOp(isSgWcharVal              (exp.get())->get_value()))));*/
	    case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgBoolValExp            (exp.get())->get_value()));
	    case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgCharVal               (exp.get())->get_value()));
	    case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgShortVal               (exp.get())->get_value()));
	    case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgIntVal                (exp.get())->get_value()));
	    case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgLongIntVal            (exp.get())->get_value()));
	    case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgLongLongIntVal        (exp.get())->get_value()));
	    case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedCharVal       (exp.get())->get_value()));
	    case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedIntVal        (exp.get())->get_value()));
	    case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongVal       (exp.get())->get_value()));
	    case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value()));
	    case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedShortVal      (exp.get())->get_value()));
	    case V_SgUpcMythread:            return boost::make_shared<CPUnknownKind>();
	    case V_SgUpcThreads:             return boost::make_shared<CPUnknownKind>();
	    case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgWcharVal              (exp.get())->get_value()));


	    case V_SgEnumVal:
	      //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnusmVal               (exp.get())->get_value()-1)));
	      // NOTE: Need to write code to get the value of the Enum Val and create a new one
	      assert(0);
	    default:
	      // We've enumerated all cases so we should never get here
	      assert(0);
	  }
	}

	// Applies the given operation functor to the expression in this ConcreteKind, which is assumed to a floating point type
	// and returns the resulting CPKind
	template<class DoOpType>
	CPValueKindPtr CPConcreteKind::doUnaryFloatOp(DoOpType& doOp) {
	  switch(exp.get()->variantT()) {
	    case V_SgComplexVal:             return boost::make_shared<CPUnknownKind>();
	    /*case V_SgDoubleVal:              return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildDoubleVal             (doOp(isSgDoubleVal             (exp.get())->get_value()))));
	    case V_SgFloatVal:               return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildFloatVal              (doOp(isSgFloatVal              (exp.get())->get_value()))));
	    case V_SgLongDoubleVal:          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongDoubleVal         (doOp(isSgLongDoubleVal         (exp.get())->get_value()))));*/
	    case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgDoubleVal             (exp.get())->get_value()));
	    case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgFloatVal              (exp.get())->get_value()));
	    case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgLongDoubleVal         (exp.get())->get_value()));

	    default:
	      // We've enumerated all cases so we should never get here
	      assert(0);
	  }
}

// Applies the given unary or binary operation to this and the given CPValueKind
// Returns:
//    - if this CPValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated CPValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent CPValueObject's list, return that.
CPValueKindPtr CPConcreteKind::op(SgUnaryOp* op) {
  if(isSgBitComplementOp(op)) {
    if(isSgBoolValExp(exp.get()))
      switch(exp.get()->variantT()) {
      case V_SgBoolValExp: case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
      case V_SgFloatVal: case V_SgDoubleVal: case V_SgLongDoubleVal:
        return boost::make_shared<CPUnknownKind>();
      default: return doUnaryIntegralOp(~ boost::lambda::_1);
    }
  } else if(isSgCastExp(op)) {
    switch(isSgCastExp(op)->get_type()->variantT()) {
      case V_SgTypeBool:               return doUnaryOp(boost::lambda::ll_static_cast<bool>              (boost::lambda::_1));
      case V_SgTypeComplex:            assert(0);
      case V_SgTypeFloat:              return doUnaryOp(boost::lambda::ll_static_cast<float>             (boost::lambda::_1));
      case V_SgTypeDouble:             return doUnaryOp(boost::lambda::ll_static_cast<double>            (boost::lambda::_1));
      case V_SgTypeLongDouble:         return doUnaryOp(boost::lambda::ll_static_cast<long double>       (boost::lambda::_1));
      case V_SgTypeChar:               return doUnaryOp(boost::lambda::ll_static_cast<char>              (boost::lambda::_1));
      case V_SgTypeInt:                return doUnaryOp(boost::lambda::ll_static_cast<int>               (boost::lambda::_1));
      case V_SgTypeLong:               return doUnaryOp(boost::lambda::ll_static_cast<long>              (boost::lambda::_1));
      case V_SgTypeLongLong:           return doUnaryOp(boost::lambda::ll_static_cast<long long>         (boost::lambda::_1));
      case V_SgTypeShort:              return doUnaryOp(boost::lambda::ll_static_cast<short>             (boost::lambda::_1));
      case V_SgTypeUnsignedChar:       return doUnaryOp(boost::lambda::ll_static_cast<unsigned char>     (boost::lambda::_1));
      case V_SgTypeUnsignedInt:        return doUnaryOp(boost::lambda::ll_static_cast<unsigned int>      (boost::lambda::_1));
      case V_SgTypeUnsignedLong:       return doUnaryOp(boost::lambda::ll_static_cast<unsigned long>     (boost::lambda::_1));
      case V_SgTypeUnsignedLongLong:   return doUnaryOp(boost::lambda::ll_static_cast<unsigned long long>(boost::lambda::_1));
      case V_SgTypeUnsignedShort:      return doUnaryOp(boost::lambda::ll_static_cast<unsigned short>    (boost::lambda::_1));
      case V_SgTypeSignedChar:         return doUnaryOp(boost::lambda::ll_static_cast<char>              (boost::lambda::_1));
      case V_SgTypeSignedInt:          return doUnaryOp(boost::lambda::ll_static_cast<int>               (boost::lambda::_1));
      case V_SgTypeSignedLong:         return doUnaryOp(boost::lambda::ll_static_cast<long>              (boost::lambda::_1));
      case V_SgTypeSignedLongLong:     return doUnaryOp(boost::lambda::ll_static_cast<long long>         (boost::lambda::_1));
      case V_SgTypeSignedShort:        return doUnaryOp(boost::lambda::ll_static_cast<short>             (boost::lambda::_1));
      case V_SgTypeWchar:              return doUnaryOp(boost::lambda::ll_static_cast<wchar_t>           (boost::lambda::_1));

      case V_SgArrayType: case V_SgFunctionType: case V_SgJavaWildcardType: case V_SgModifierType:
      case V_SgNamedType: case V_SgPointerType: case V_SgQualifiedNameType: case V_SgReferenceType:
      case V_SgTemplateType: case V_SgTypeCAFTeam: case V_SgTypeCrayPointer: case V_SgTypeDefault:
      case V_SgTypeEllipse: case V_SgTypeGlobalVoid: case V_SgTypeImaginary: case V_SgTypeLabel:
      case V_SgTypeVoid:
      default:
        assert(0);
    }
  } else if(isSgConjugateOp(op)) {
    // TO DO
    /*if(isSgComplexVal(exp.get()))
      return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildComplexVal(
               isSgComplexVal(exp.get())->get_real_value(),
              -isSgComplexVal(exp.get())->get_imaginary_value())));
    else
      return boost::make_shared<CPUnknownKind>();*/
  } else if(isSgExpressionRoot(op)) {
    // What is this?
  } else if(isSgImagPartOp(op)) {
    if(isSgComplexVal(exp.get()))
      return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(isSgComplexVal(exp.get())->get_imaginary_value()));
    else
      return boost::make_shared<CPUnknownKind>();
  } else if(isSgRealPartOp(op)) {
    if(isSgComplexVal(exp.get()))
      return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(isSgComplexVal(exp.get())->get_real_value()));
    else
      return boost::make_shared<CPUnknownKind>();
  } else if(isSgMinusMinusOp(op)) {
    switch(exp.get()->variantT()) {
      case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads: return boost::make_shared<CPUnknownKind>();
      default: return doUnaryOp(boost::lambda::_1 - 1);
    }
  } else if(isSgPlusPlusOp(op)) {
    switch(exp.get()->variantT()) {
      case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads: return boost::make_shared<CPUnknownKind>();
      default: return doUnaryOp(boost::lambda::_1 + 1);
    }
  } else if(isSgMinusOp(op)) {
    switch(exp.get()->variantT()) {
      case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads: return boost::make_shared<CPUnknownKind>();
      default: return doUnaryOp(0 - boost::lambda::_1);
    }
  } else if(isSgNotOp(op)) {
    switch(exp.get()->variantT()) {
      case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads: return boost::make_shared<CPUnknownKind>();
      default: return doUnaryOp(!boost::lambda::_1);
    }

  } else if(isSgThrowOp(op)) {
    return boost::make_shared<CPUnknownKind>();
    // TODO: control flow effects

  } else if(isSgUnaryAddOp(op)) {
    // What is this?

  } else if(isSgUserDefinedUnaryOp(op)) {
    // What is this?

  } else if(isSgAddressOfOp(op)) {
    // This should be handled inside CPMemLocObjects
    assert(0);

  } else if(isSgPointerDerefExp(op)) {
    // This should be handled inside CPMemLocObjects
    assert(0);
  }

  // We've enumerated all cases so we should never get here
  cerr << "ERROR: no support for "<<SgNode2Str(op)<<"!";
  assert(0);
}

// Returns whether a given AST node that represents a constant is interpretable as a signed integer and
// sets val to be the numeric value of that integer
bool IsSignedConstInt(SgExpression* exp, long long &val)
{
  switch(exp->variantT())
  {
    case V_SgBoolValExp:             val = isSgBoolValExp(exp)->get_value();             return true;
    case V_SgCharVal:                val = isSgCharVal(exp)->get_value();                return true;
    case V_SgWcharVal:               val = isSgWcharVal(exp)->get_valueUL();             return true;
    case V_SgIntVal:                 val = isSgIntVal(exp)->get_value();                 return true;
    case V_SgLongIntVal:             val = isSgLongIntVal(exp)->get_value();             return true;
    case V_SgLongLongIntVal:         val = isSgLongLongIntVal(exp)->get_value();         return true;
    case V_SgShortVal:               val = isSgShortVal(exp)->get_value();               return true;
    default: return false;
  }
}

// Returns whether a given AST node that represents a constant is interpretable as an unsigned integer and
// sets val to be the numeric value of that integer
bool IsUnsignedConstInt(SgExpression* exp, unsigned long long &val)
{
  switch(exp->variantT())
  {
    case V_SgBoolValExp:             val = isSgBoolValExp(exp)->get_value();             return true;
    case V_SgUnsignedCharVal:        val = isSgUnsignedCharVal(exp)->get_value();        return true;
    case V_SgUnsignedIntVal:         val = isSgUnsignedIntVal(exp)->get_value();         return true;
    case V_SgUnsignedLongVal:        val = isSgUnsignedLongVal(exp)->get_value();        return true;
    case V_SgUnsignedLongLongIntVal: val = isSgUnsignedLongLongIntVal(exp)->get_value(); return true;
    case V_SgUnsignedShortVal:       val = isSgUnsignedShortVal(exp)->get_value();       return true;
    default: return false;
  }
}

// Returns whether a given AST node that represents a constant is interpretable as a float and
// sets val to be the numeric value of that float
bool IsConstFloat(SgExpression* exp, double &val)
{
  switch(exp->variantT())
  {
    case V_SgBoolValExp:             val = isSgBoolValExp(exp)->get_value();             return true;
    case V_SgCharVal:                val = isSgCharVal(exp)->get_value();                return true;
    case V_SgWcharVal:               val = isSgWcharVal(exp)->get_valueUL();             return true;
    case V_SgIntVal:                 val = isSgIntVal(exp)->get_value();                 return true;
    case V_SgLongIntVal:             val = isSgLongIntVal(exp)->get_value();             return true;
    case V_SgLongLongIntVal:         val = isSgLongLongIntVal(exp)->get_value();         return true;
    case V_SgShortVal:               val = isSgShortVal(exp)->get_value();               return true;
    case V_SgUnsignedIntVal:         val = isSgUnsignedIntVal(exp)->get_value();         return true;
    case V_SgUnsignedLongVal:        val = isSgUnsignedLongVal(exp)->get_value();        return true;
    case V_SgUnsignedLongLongIntVal: val = isSgUnsignedLongLongIntVal(exp)->get_value(); return true;
    case V_SgUnsignedShortVal:       val = isSgUnsignedShortVal(exp)->get_value();       return true;
    case V_SgDoubleVal:              val = isSgDoubleVal(exp)->get_value();              return true;
    case V_SgFloatVal:               val = isSgFloatVal(exp)->get_value();               return true;
    default: return false;
  }
}

/*
template<class DoOpType, class DoOpRetType>
DoOpRetType CPConcreteKind::bindDoOpArgs1(DoOpType doOp) {
  switch(exp.get()->variantT()) {
    case V_SgBoolValExp:             return boost::bind(doOp, isSgBoolValExp            (exp.get())->get_value(), _1);
    case V_SgCharVal:                return boost::bind(doOp, isSgCharVal               (exp.get())->get_value(), _1);
    case V_SgIntVal:                 return boost::bind(doOp, isSgIntVal                (exp.get())->get_value(), _1);
    case V_SgLongIntVal:             return boost::bind(doOp, isSgLongIntVal            (exp.get())->get_value(), _1);
    case V_SgLongLongIntVal:         return boost::bind(doOp, isSgLongLongIntVal        (exp.get())->get_value(), _1);
    case V_SgUnsignedCharVal:        return boost::bind(doOp, isSgUnsignedCharVal       (exp.get())->get_value(), _1);
    case V_SgUnsignedIntVal:         return boost::bind(doOp, isSgUnsignedIntVal        (exp.get())->get_value(), _1);
    case V_SgUnsignedLongVal:        return boost::bind(doOp, isSgUnsignedLongVal       (exp.get())->get_value(), _1);
    case V_SgUnsignedLongLongIntVal: return boost::bind(doOp, isSgUnsignedLongLongIntVal(exp.get())->get_value(), _1);
    case V_SgUnsignedShortVal:       return boost::bind(doOp, isSgUnsignedShortVal      (exp.get())->get_value(), _1);
    case V_SgUpcMythread:  case V_SgUpcThreads:
      assert(0);

    case V_SgWcharVal:               return boost::bind(doOp, isSgWcharVal              (exp.get())->get_value(), _1);
    case V_SgEnumVal:
      assert(0);
    case V_SgDoubleVal:              return boost::bind(doOp, isSgDoubleVal             (exp.get())->get_value(), _1);
    case V_SgFloatVal:               return boost::bind(doOp, isSgFloatVal              (exp.get())->get_value(), _1);
    case V_SgLongDoubleVal:          return boost::bind(doOp, isSgLongDoubleVal         (exp.get())->get_value(), _1);
    case V_SgComplexVal:
      assert(0);

    default:
      // We've enumerated all cases so we should never get here
      assert(0);
  }
}

template<class DoOpType, class DoOpRetType>
DoOpRetType CPConcreteKind::bindDoOpArgs2(DoOpType doOp) {
  switch(exp.get()->variantT()) {
    case V_SgBoolValExp:             return boost::bind(doOp, isSgBoolValExp            (exp.get())->get_value());
    case V_SgCharVal:                return boost::bind(doOp, isSgCharVal               (exp.get())->get_value());
    case V_SgIntVal:                 return boost::bind(doOp, isSgIntVal                (exp.get())->get_value());
    case V_SgLongIntVal:             return boost::bind(doOp, isSgLongIntVal            (exp.get())->get_value());
    case V_SgLongLongIntVal:         return boost::bind(doOp, isSgLongLongIntVal        (exp.get())->get_value());
    case V_SgUnsignedCharVal:        return boost::bind(doOp, isSgUnsignedCharVal       (exp.get())->get_value());
    case V_SgUnsignedIntVal:         return boost::bind(doOp, isSgUnsignedIntVal        (exp.get())->get_value());
    case V_SgUnsignedLongVal:        return boost::bind(doOp, isSgUnsignedLongVal       (exp.get())->get_value());
    case V_SgUnsignedLongLongIntVal: return boost::bind(doOp, isSgUnsignedLongLongIntVal(exp.get())->get_value());
    case V_SgUnsignedShortVal:       return boost::bind(doOp, isSgUnsignedShortVal      (exp.get())->get_value());
    case V_SgUpcMythread:  case V_SgUpcThreads:
      assert(0);

    case V_SgWcharVal:               return boost::bind(doOp, isSgWcharVal              (exp.get())->get_value());
    case V_SgEnumVal:
      assert(0);
    case V_SgDoubleVal:              return boost::bind(doOp, isSgDoubleVal             (exp.get())->get_value());
    case V_SgFloatVal:               return boost::bind(doOp, isSgFloatVal              (exp.get())->get_value());
    case V_SgLongDoubleVal:          return boost::bind(doOp, isSgLongDoubleVal         (exp.get())->get_value());
    case V_SgComplexVal:
      assert(0);

    default:
      // We've enumerated all cases so we should never get here
      assert(0);
  }
}*/

// Returns whether v denotes an integral (as opposed to floating point) value
bool isIntegralVal(SgValueExp* v) {
  switch(v->variantT()) {
    case V_SgBoolValExp:
    case V_SgCharVal:
    case V_SgShortVal:
    case V_SgIntVal:
    case V_SgLongIntVal:
    case V_SgLongLongIntVal:
    case V_SgUnsignedCharVal:
    case V_SgUnsignedShortVal:
    case V_SgUnsignedIntVal:
    case V_SgUnsignedLongVal:
    case V_SgUnsignedLongLongIntVal:
    case V_SgWcharVal:
    case V_SgUpcMythread: case V_SgUpcThreads:
    case V_SgEnumVal:
      return true;

    case V_SgFloatVal:
    case V_SgDoubleVal:
    case V_SgLongDoubleVal:
    case V_SgComplexVal:
      return false;

    default:
      // We've enumerated all cases so we should never get here
      assert(0);
  }
}

// Returns whether v denotes an floating point (as opposed to integral) value
bool isFloatVal(SgValueExp* v)
{ return !isIntegralVal(v); }

// Returns the offset of the given SgPntrArrRefExp relative to the starting point of its parent expression,
// which may be a SgVarRefExp, SgDotExp, SgPntrArrRefExp or other expressions
long long getPntrArrRefOffset(SgPntrArrRefExp* ref, CPConcreteKindPtr that) {
  /*scope s(txt()<<"getPntrArrRefOffset("<<SgNode2Str(ref));
  dbg << "rhs="<<SgNode2Str(ref->get_rhs_operand())<<endl;
  dbg << "lhs="<<SgNode2Str(ref->get_lhs_operand())<<endl;*/

  SgArrayType* arrType = isSgArrayType(ref->get_lhs_operand()->get_type());
  assert(arrType);

  /*dbg << "   index="<<SgNode2Str(arrType->get_index())<<endl;
  dbg << "   dim="<<SgNode2Str(arrType->get_dim_info())<<"="<<arrType->get_dim_info()<<endl;
  dbg << "   rank="<<arrType->get_rank()<<endl;
  dbg << "   eltCount="<<SageInterface::getArrayElementCount(arrType)<<endl;*/

  // Compute the number of entries in the array of the current sub-level in the SgArrayType by
  // dividing the number of total entries in the current SgArrayType by the number of sub-arrays
  // in the next dimension.
  assert(isSgValueExp(arrType->get_index()));
  unsigned long long subArraySize;
  long long sTypeIdx;
  unsigned long long usTypeIdx;
  if(IsSignedConstInt(isSgValueExp(arrType->get_index()), sTypeIdx))
    subArraySize = SageInterface::getArrayElementCount(arrType) / sTypeIdx;
  else if(IsUnsignedConstInt(isSgValueExp(arrType->get_index()), usTypeIdx))
    subArraySize = SageInterface::getArrayElementCount(arrType) / usTypeIdx;
  else
    // The index in the array's type must be an integer of some sort
    assert(0);

  // Given the number of entries in the next level's sub-array, compute the offset of
  // the next array index (that value), which is a multiple of the next level's sub-array
  long long sRefIdx;
  unsigned long long usRefIdx;
  if(IsSignedConstInt(that->getVal().get(), sRefIdx))
    return subArraySize * sRefIdx;
  else if(IsUnsignedConstInt(that->getVal().get(), usRefIdx))
    return subArraySize * usRefIdx;
  else
    // The index in the array's reference expression must be an integer of some sort
    assert(0);
}

// Applies the given binary operation functor to the expression in this and that ConcreteKinds, assuming they're
// both integral types and returns the resulting CPKind
template<class DoOpType>
CPValueKindPtr CPConcreteKind::doBinaryOpIntInt(DoOpType& doOp, CPConcreteKindPtr that) {
  long long sVal1, sVal2;
  unsigned long long usVal1, usVal2;
  if(IsSignedConstInt(exp.get(), sVal1)) {
    if(IsSignedConstInt(that->getVal().get(), sVal2))
      return createCPValueKindFromVal(doOp(sVal1, sVal2));
    else if(IsUnsignedConstInt(that->getVal().get(), usVal2))
      return createCPValueKindFromVal(doOp(sVal1, (long long)usVal2));
  } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
    if(IsSignedConstInt(that->getVal().get(), sVal2))
      return createCPValueKindFromVal(doOp((long long)usVal1, sVal2));
    else if(IsUnsignedConstInt(that->getVal().get(), usVal2))
      return createCPValueKindFromVal(doOp(usVal1, usVal2));
  }
  assert(0);
  /*switch(exp.get()->variantT()) {
    case V_SgBoolValExp:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgCharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgShortVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedCharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedShortVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedLongVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedLongLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgWcharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUpcMythread: case V_SgUpcThreads:
      return boost::make_shared<CPUnknownKind>();

    case V_SgEnumVal:
      //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
      // NOTE: Need to write code to get the value of the Enum Val and create a new one
      assert(0);
    default:
      // We've enumerated all cases so we should never get here
      assert(0);
  }  */
}

// Applies the given binary operation functor to the expression in this and that ConcreteKinds, assuming at least one
// is not an integral type and returns the resulting CPKind
template<class DoOpType>
CPValueKindPtr CPConcreteKind::doBinaryOp(DoOpType& doOp, CPConcreteKindPtr that) {
  long long sVal1, sVal2;
  unsigned long long usVal1, usVal2;

  if(IsSignedConstInt(exp.get(), sVal1)) {
    if(IsSignedConstInt(that->getVal().get(), sVal2))
      return createCPValueKindFromVal(doOp(sVal1, sVal2));
    else if(IsUnsignedConstInt(that->getVal().get(), usVal2))
      return createCPValueKindFromVal(doOp(sVal1, (long long)usVal2));
  } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
    if(IsSignedConstInt(that->getVal().get(), sVal2))
      return createCPValueKindFromVal(doOp((long long)usVal1, sVal2));
    else if(IsUnsignedConstInt(that->getVal().get(), usVal2))
      return createCPValueKindFromVal(doOp(usVal1, usVal2));
  }
  // If the values are not integers, lets see if they're floating point
  double fVal1, fVal2;
  if(IsConstFloat(exp.get(), fVal1) && IsConstFloat(that->getVal().get(), fVal2))
    return createCPValueKindFromVal(doOp(fVal1,fVal2));

  assert(0);
  /*
  switch(exp.get()->variantT()) {
    case V_SgBoolValExp:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgCharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgShortVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedCharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedShortVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedLongVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedLongLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgWcharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgFloatVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgDoubleVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongDoubleVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createCPValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<CPUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
      return boost::make_shared<CPUnknownKind>();

    case V_SgEnumVal:
      //return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
      // NOTE: Need to write code to get the value of the Enum Val and create a new one
      assert(0);
    default:
      // We've enumerated all cases so we should never get here
      assert(0);
  }  */
}


// Applies the given unary or binary operation to this and the given CPValueKind
// Returns:
//    - if this CPValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated CPValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent CPValueObject's list, return that.
CPValueKindPtr CPConcreteKind::op(SgBinaryOp* op, CPValueKindPtr that) {
  assert(that);

  /*scope("CPConcreteKind::op(SgBinaryOp*)", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 2));
  if(constantPropagationAnalysisDebugLevel()>=2) {
    dbg << "this="<<str()<<endl;
    dbg << "that="<<that->str()<<endl;
    dbg << "(that->getKind() == CPValueKind::uninitialized)="<<(that->getKind() == CPValueKind::uninitialized)<<endl;
    dbg << "(that->getKind() == CPValueKind::unknown)="<<(that->getKind() == CPValueKind::unknown)<<endl;
  }*/

  // * op uninitialized => *
  if(that->getKind() == CPValueKind::uninitialized)
    return copyV();

  // * op unknown => unknown
  if(that->getKind() == CPValueKind::unknown)
    return boost::make_shared<CPUnknownKind>();

  if(that->getKind() == CPValueKind::concrete) {
    CPConcreteKindPtr thatConcrete = that->asConcreteKind();


    // ----- Arithmetic -----
    if(isSgAddOp(op) || isSgPlusAssignOp(op)) {
      long long sVal1, sVal2;
      unsigned long long usVal1, usVal2;
      if(IsSignedConstInt(exp.get(), sVal1)) {
        if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(sVal1+sVal2)));
        else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(sVal1+usVal2)));
      } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
        if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(usVal1+sVal2)));
        else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(usVal1+usVal2)));
      }
      // If the values are not integers, lets see if they're floating point
      double fVal1, fVal2;
      if(IsConstFloat(exp.get(), fVal1) && IsConstFloat(thatConcrete->getVal().get(), fVal2))
        return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildDoubleVal(fVal1+fVal2)));
      // If not floating point, give up
      return boost::make_shared<CPUnknownKind>();

    } else if(isSgSubtractOp(op) || isSgMinusAssignOp(op)) {
      //return createCPValueKindFromVal(thatConcrete->bindDoOpArgs2(bindDoOpArgs1(boost::lambda::_1 - boost::lambda::_2)));
      return doBinaryOp(boost::lambda::_1 - boost::lambda::_2, thatConcrete);
    } else if(isSgMultiplyOp(op) || isSgMultAssignOp(op)) {
      return doBinaryOp(boost::lambda::_1 * boost::lambda::_2, thatConcrete);
    } else if(isSgDivideOp(op) || isSgDivAssignOp(op)) {
      return doBinaryOp(boost::lambda::_1 / boost::lambda::_2, thatConcrete);
    } else if(isSgIntegerDivideOp(op) || isSgIntegerDivideAssignOp(op)) {
      // TODO

    } else if(isSgModOp(op) || isSgModAssignOp(op)) {
      assert(isIntegralVal(exp.get()) && isIntegralVal(thatConcrete->exp.get()));
      return doBinaryOpIntInt(boost::lambda::_1 % boost::lambda::_2, thatConcrete);
    } else if(isSgExponentiationOp(op) || isSgExponentiationAssignOp(op)) {
      // TO DO
    // ----- Logical -----
    } else if(isSgAndOp(op) || isSgAndAssignOp(op)) {
      return doBinaryOp(boost::lambda::_1 && boost::lambda::_2, thatConcrete);
    } else if(isSgOrOp(op) || isSgIorAssignOp(op)) {
      return doBinaryOp(boost::lambda::_1 || boost::lambda::_2, thatConcrete);
    // ----- Bitwise -----
    } else if(isSgBitAndOp(op)) {
      assert(isIntegralVal(exp.get()) && isIntegralVal(thatConcrete->exp.get()));
      return doBinaryOpIntInt(boost::lambda::_1 & boost::lambda::_2, thatConcrete);
    } else if(isSgBitOrOp(op)) {
      assert(isIntegralVal(exp.get()) && isIntegralVal(thatConcrete->exp.get()));
      return doBinaryOpIntInt(boost::lambda::_1 | boost::lambda::_2, thatConcrete);
    } else if(isSgBitXorOp(op) || isSgXorAssignOp(op)) {
      assert(isIntegralVal(exp.get()) && isIntegralVal(thatConcrete->exp.get()));
      return doBinaryOpIntInt(boost::lambda::_1 ^ boost::lambda::_2, thatConcrete);
    } else if(isSgLshiftOp(op) || isSgLshiftAssignOp(op)) {
      assert(isIntegralVal(exp.get()) && isIntegralVal(thatConcrete->exp.get()));
      return doBinaryOpIntInt(boost::lambda::_1 << boost::lambda::_2, thatConcrete);
    } else if(isSgRshiftOp(op) || isSgRshiftAssignOp(op)) {
      assert(isIntegralVal(exp.get()) && isIntegralVal(thatConcrete->exp.get()));
      return doBinaryOpIntInt(boost::lambda::_1 >> boost::lambda::_2, thatConcrete);
    } else if(isSgJavaUnsignedRshiftOp(op) || isSgJavaUnsignedRshiftAssignOp(op)) {
      // TODO (Java))

    // ----- Comparison -----
    } else if(isSgEqualityOp(op)) {
      long long sVal1, sVal2;
      unsigned long long usVal1, usVal2;
      if(IsSignedConstInt(exp.get(), sVal1)) {
        if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(sVal1==sVal2)));
        else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(sVal1==(long long)usVal2)));
      } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
        if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp((long long)usVal1==sVal2)));
        else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
          return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(usVal1==usVal2)));
      }
      // If the values are not integers, lets see if they're floating point
      double fVal1, fVal2;
      if(IsConstFloat(exp.get(), fVal1) && IsConstFloat(thatConcrete->getVal().get(), fVal2))
        return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(fVal1==fVal2)));
      // If not floating point, give up
      return boost::make_shared<CPUnknownKind>();
    } else if(isSgNotEqualOp(op)) {
      return doBinaryOp(boost::lambda::_1 != boost::lambda::_2, thatConcrete);
    } else if(isSgLessOrEqualOp(op)) {
      return doBinaryOp(boost::lambda::_1 <= boost::lambda::_2, thatConcrete);
    } else if(isSgLessThanOp(op)) {
      return doBinaryOp(boost::lambda::_1 <  boost::lambda::_2, thatConcrete);
    } else if(isSgGreaterOrEqualOp(op)) {
      return doBinaryOp(boost::lambda::_1 >= boost::lambda::_2, thatConcrete);
    } else if(isSgGreaterThanOp(op)) {
      return doBinaryOp(boost::lambda::_1 >  boost::lambda::_2, thatConcrete);
    } else if(isSgIsOp(op)) {
      // TODO

    } else if(isSgIsNotOp(op)) {
      // TODO

    // ----- Memory References -----
    } else if(isSgDotExp(op)) {
      // This should be handled inside CPMemLocObjects
      assert(0);
    } else if(isSgArrowExp(op)) {
      // This should be handled inside CPMemLocObjects
      assert(0);
    } else if(isSgPntrArrRefExp(op)) {
      scope s("SgPntrArrRefExp");
      assert(thatConcrete);
      // Get the offset of the SgPntrArrRefExp relative to the starting point of its parent expression
      long long nextOffset = getPntrArrRefOffset(isSgPntrArrRefExp(op), thatConcrete);
      return doBinaryOp(boost::lambda::_1 + boost::lambda::_2,
                        boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(nextOffset))));

    } else if(isSgDotStarOp(op)) {
      // This should be handled inside CPMemLocObjects
      assert(0);
    } else if(isSgPointerAssignOp(op)) {
      // TODO (Fortran)

    // ----- Miscelaneous ----
    } else if(isSgAssignOp(op)) {
      // This should be handled inside the ConstantPropagationAnalysis transfer function
      assert(0);
    } else if(isSgCommaOpExp(op)) {
      // This should be handled inside the control flow functionality
      assert(0);

    } else if(isSgConcatenationOp(op)) {
      // TODO
    } else if(isSgMembershipOp(op)) {
      // TODO
    } else if(isSgNonMembershipOp(op)) {
      // TODO
    } else if(isSgUserDefinedBinaryOp(op)) {
      // TODO
    }
  } else if(that->getKind() == CPValueKind::offsetList) {
    CPConcreteKindPtr thatOffset = that->asConcreteKind();

    // Implement the operations that are defined for pointers by having the offset list kind deal with them
    if(isSgAddOp(op) || isSgSubtractOp(op)) return thatOffset->op(op, shared_from_this());
    // Other options are undefined.
    // Technically this means that we can return an uninitialized kind but to be safe we'll return unknown
    else
      return boost::make_shared<CPUnknownKind>();
  }

  // We've enumerated all cases so we should never get here
  cerr << "ERROR: no support for "<<SgNode2Str(op)<<"!";
  assert(0);
}

// Returns whether this and that CPValueKinds are may/must equal to each other
bool CPConcreteKind::mayEqualV(CPValueKindPtr that)
{
  // If that is not concrete, use its implementation
  if(that->getKind() != CPValueKind::concrete) return that->mayEqualV(shared_from_this());
  CPConcreteKindPtr thatConcrete = that->asConcreteKind();

  long long sVal1, sVal2;
  unsigned long long usVal1, usVal2;
  // First check if both concrete values are integers of some type
  if(IsSignedConstInt(exp.get(), sVal1)) {
    if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
      return sVal1==sVal2;
    else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
      return sVal1==(long long)usVal2;
  } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
    if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
      return (long long)usVal1==sVal2;
    else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
      return usVal1==usVal2;
  }
  // If the values are not integers, lets see if they're floating point
  double fVal1, fVal2;
  if(IsConstFloat(exp.get(), fVal1) && IsConstFloat(thatConcrete->getVal().get(), fVal2))
    return fVal1==fVal2;

  // Otherwise, default to mayEquals
  return true;
}

bool CPConcreteKind::mustEqualV(CPValueKindPtr that) {
  // If that is not concrete, use its implementation
  if(that->getKind() != CPValueKind::concrete) return that->mustEqualV(shared_from_this());
  CPConcreteKindPtr thatConcrete = that->asConcreteKind();

  long long sVal1, sVal2;
  unsigned long long usVal1, usVal2;

  // First check if both concrete values are integers of some type
  if(IsSignedConstInt(exp.get(), sVal1)) {
    if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
      return sVal1==sVal2;
    else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
      return sVal1==(long long)usVal2;
  } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
    if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
      return (long long)usVal1==sVal2;
    else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
      return usVal1==usVal2;
  }
  // If the values are not integers, lets see if they're floating point
  double fVal1, fVal2;
  if(IsConstFloat(exp.get(), fVal1) && IsConstFloat(thatConcrete->getVal().get(), fVal2))
    return fVal1==fVal2;

  // Otherwise, default to not mustEquals
  return false;
}

// Returns whether the two CPValueKinds denote the same set of concrete values
bool CPConcreteKind::equalSetV(CPValueKindPtr that) {
  // The logic here is the same as mustEquals
  return mustEqualV(that);
}

// Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given CPValueKind
bool CPConcreteKind::subSetV(CPValueKindPtr that) {
  // The logic here is the same as mustEquals
  return mustEqualV(that);
}

// Computes the meet of this and that and returns the resulting kind
pair<bool, CPValueKindPtr> CPConcreteKind::meetUpdateV(CPValueKindPtr that)
{
  // Concrete MEET Uninitialized => Concrete
  if(that->getKind() == CPValueKind::uninitialized)
    return make_pair(true, copyV());

  // Concrete MEET Unknown => Unknown
  // Concrete MEET Offset  => Unknown
  if(that->getKind() == CPValueKind::offsetList ||
     that->getKind() == CPValueKind::unknown)
    return make_pair(true, boost::make_shared<CPUnknownKind>());

  // That is definitely concrete

  // If this and that denote the same concrete value
  if(mustEqualV(that)) return make_pair(false, copyV());
  // If the concrete values differ
  else                 return make_pair(true, boost::make_shared<CPUnknownKind>());
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool CPConcreteKind::isConcrete()
{ return true; }

// Returns the type of the concrete value (if there is one)
SgType* CPConcreteKind::getConcreteType()
{ return exp->get_type(); }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > CPConcreteKind::getConcreteValue() {
  std::set<boost::shared_ptr<SgValueExp> > vals;
  vals.insert(exp);
  return vals;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool CPConcreteKind::isFullV(PartEdgePtr pedge) { return false; }
// Returns whether this AbstractObject denotes the empty set.
bool CPConcreteKind::isEmptyV(PartEdgePtr pedge) { return false; }

std::string CPConcreteKind::str(std::string indent)
{ return txt()<<"[CPConcreteKind: val="<<(exp? SgNode2Str(exp.get()): "NULL")<<"]"; }


// ********************************
// **** CPOffsetListKind *****
// ********************************

SgTypeLongLong *CPOffsetListKind::type = NULL;

// Applies the given unary or binary operation to this and the given CPValueKind
// Returns:
//    - if this CPValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated CPValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent CPValueObject's list, return that.
CPValueKindPtr CPOffsetListKind::op(SgUnaryOp* op) {
  // The only operations defiend on offset kinds are binary + and -
  return boost::make_shared<CPUnknownKind>();
}

CPValueKindPtr CPOffsetListKind::op(SgBinaryOp* op, CPValueKindPtr that) {
  // * op uninitialized => *
  if(that && that->getKind() == CPValueKind::uninitialized)
    return copyV();

  // * op unknown => unknown
  if(that && that->getKind() == CPValueKind::unknown)
    return boost::make_shared<CPUnknownKind>();

  // If that is a concrete value or that was not provided because it is not needed (e.g. dot expression)
  if(!that || that->getKind() == CPValueKind::concrete) {
    CPConcreteKindPtr thatConcrete; if(that) thatConcrete = that->asConcreteKind();
    assert(offsetL.size()>0);

    // The arithmetic only operations defined on offset kinds are binary + and -
    switch(op->variantT()) {
      case V_SgAddOp:
      {
        assert(thatConcrete);
        // The offset list of the newly-generated offset kind
        list<intWrap> newOffsetL = offsetL;

        // If the last value in the offset list is a concrete offset
        if(offsetL.back().getType()==intWrap::offsetT) {
          long long sVal;
          unsigned long long usVal;
          long long res;
          if(IsSignedConstInt(thatConcrete->getVal().get(), sVal))
            res = offsetL.back().get() + sVal;
          else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal))
            res = offsetL.back().get() + usVal;
          else
            return boost::make_shared<CPUnknownKind>();

          newOffsetL.back().set(res);
        // If the last value in the offset is a rank, append the concrete offset onto the list
        } else if(offsetL.back().getType()==intWrap::rankT) {
          long long sVal;
          unsigned long long usVal;
          if(IsSignedConstInt(thatConcrete->getVal().get(), sVal))
            offsetL.push_back(intWrap(sVal, intWrap::offsetT));
          else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal))
            offsetL.push_back(intWrap((long long)usVal, intWrap::offsetT));
          else
            return boost::make_shared<CPUnknownKind>();
        }

        return boost::make_shared<CPOffsetListKind>(newOffsetL);
      }

      case V_SgSubtractOp :
      {
        assert(thatConcrete);
        // The offset list of the newly-generated offset kind
        list<intWrap> newOffsetL = offsetL;

        // If the last value in the offset list is a concrete offset
        if(offsetL.back().getType()==intWrap::offsetT) {
          long long sVal;
          unsigned long long usVal;
          long long res;
          if(IsSignedConstInt(thatConcrete->getVal().get(), sVal))
            res = offsetL.back().get() - sVal;
          else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal))
            res = offsetL.back().get() - usVal;
          else
            return boost::make_shared<CPUnknownKind>();

          newOffsetL.back().set(res);
        // If the last value in the offset is a rank, append the concrete offset onto the list
        } else if(offsetL.back().getType()==intWrap::rankT) {
          long long sVal;
          unsigned long long usVal;
          if(IsSignedConstInt(thatConcrete->getVal().get(), sVal))
            offsetL.push_back(intWrap(sVal, intWrap::offsetT));
          else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal))
            offsetL.push_back(intWrap((long long)usVal, intWrap::offsetT));
          else
            return boost::make_shared<CPUnknownKind>();
        }

        return boost::make_shared<CPOffsetListKind>(newOffsetL);
      }

      // ----- Comparison -----
      case V_SgEqualityOp:
      case V_SgNotEqualOp:
      case V_SgLessOrEqualOp:
      case V_SgLessThanOp:
      case V_SgGreaterOrEqualOp:
      case V_SgGreaterThanOp:
      {
        assert(thatConcrete);
        // If this offset is just a single concrete value
        if(offsetL.size()==1 && offsetL.back().getType()==intWrap::offsetT) {
          long long sVal;
          unsigned long long usVal;
          switch(op->variantT()) {
            case V_SgEqualityOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal==offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal==offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgNotEqualOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal!=offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal!=offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgLessOrEqualOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal<=offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal<=offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgLessThanOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal<offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal<offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgGreaterOrEqualOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal>=offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal>=offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgGreaterThanOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal>offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal>offsetL.back().get())
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<CPConcreteKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            default:
              break;
          }
        }
        // If we don't have enough information to compare the two values
        return boost::make_shared<CPUnknownKind>();
      }

      // ----- Memory References -----
      case V_SgDotExp:
      {
        scope s("V_SgDotExp", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));
        // Find the index of the DotExp's RHS within the class of its LHS
        SgClassType *type = isSgClassType(isSgDotExp(op)->get_lhs_operand()->get_type());
        assert(type);
        SgClassDeclaration* decl = isSgClassDeclaration(type->get_declaration()->get_definingDeclaration());
        assert(decl);

        /*dbg << "  type="<<SgNode2Str(type)<<endl;
        dbg << "  decl="<<SgNode2Str(type->get_declaration())<<", classdecl="<<isSgClassDeclaration(type->get_declaration()->get_definingDeclaration())<<endl;
        dbg << "  def="<<SgNode2Str(isSgClassDeclaration(type->get_declaration()->get_definingDeclaration())->get_definition())<<endl;*/
        SgClassDefinition* def = decl->get_definition();
        assert(def);

        const SgDeclarationStatementPtrList& members = def->get_members();
        long long memberIdx=0;
        //dbg << "    RHS="<<SgNode2Str(isSgVarRefExp(isSgDotExp(op)->get_rhs_operand()))<<", symbol="<<SgNode2Str(isSgVarRefExp(isSgDotExp(op)->get_rhs_operand())->get_symbol())<<", decl="<<SgNode2Str(isSgVarRefExp(isSgDotExp(op)->get_rhs_operand())->get_symbol()->get_declaration())<<endl;
        SgInitializedName* rhsDecl = isSgVarRefExp(isSgDotExp(op)->get_rhs_operand())->get_symbol()->get_declaration();
        //dbg << "rhsDecl="<<SgNode2Str(rhsDecl)<<endl;
        assert(rhsDecl);

        for(SgDeclarationStatementPtrList::const_iterator m=members.begin(); m!=members.end(); m++) {
          //scope s2(txt()<<memberIdx<<":    member ="<<SgNode2Str(*m));
          if(isSgVariableDeclaration(*m)) {
            const SgInitializedNamePtrList& decls = isSgVariableDeclaration(*m)->get_variables();
            for(SgInitializedNamePtrList::const_iterator d=decls.begin(); d!=decls.end(); d++) {
              //dbg << "        decl "<<memberIdx<<"="<<SgNode2Str(*d)<<" type="<<SgNode2Str((*d)->get_type())<<endl;
              if(*d == rhsDecl) {
                //dbg << "    memberIdx="<<memberIdx<<endl;

                // Return a new offset kind that extends the current one by appending a memberIdx rank
                list<intWrap> newOffsetL = offsetL;
                // Remove the trailing element in newOffsetL if it is a concrete constant 0 since such
                // a concrete offset has no meaning
                if(newOffsetL.back().getType()==intWrap::offsetT && newOffsetL.back().get()==0) newOffsetL.pop_back();
                newOffsetL.push_back(rank(memberIdx));
                return boost::make_shared<CPOffsetListKind>(newOffsetL);
              }
              /*if(SgClassType* varClassType = isSgClassType((*d)->get_type())) {
                SgClassDeclaration* varClassDecl = isSgClassDeclaration(varClassType->get_declaration()->get_definingDeclaration());
                memberIdx += getNumClassMembers(varClassDecl->get_definition());
              } else {*/
               memberIdx++;
              //}
            }
          } /*else if(isSgClassDeclaration(*m)) {
            memberIdx += getNumClassMembers(isSgClassDeclaration(isSgClassDeclaration(*m)->get_definingDeclaration())->get_definition());
          }*/
        }
        assert(0); // There must be at least one match
      }

      case V_SgArrowExp:
        // Arrow expressions introduce aliasing, so we don't know their referent
        return boost::make_shared<CPUnknownKind>();

      case V_SgPntrArrRefExp:
      {
        scope s("V_SgPntrArrRefExp", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));
        assert(thatConcrete);

        SgPntrArrRefExp* ref = isSgPntrArrRefExp(op);

        // Get the offset of the SgPntrArrRefExp relative to the starting point of its parent expression
        long long nextOffset = getPntrArrRefOffset(ref, thatConcrete);
        list<intWrap> newOffsetL = offsetL;

        // If the last element in the current offset list is a concrete value, add nextOffset to it
        if(offsetL.back().getType()==intWrap::offsetT)
          newOffsetL.back().set(newOffsetL.back().get() + nextOffset);
        // If the last element is a rank, append nextOffset
        else
          newOffsetL.push_back(offset(nextOffset));

        return boost::make_shared<CPOffsetListKind>(newOffsetL);
      }

      case V_SgDotStarOp:
        // TODO
        return boost::make_shared<CPUnknownKind>();

      case V_SgPointerAssignOp:
        // TODO (Fortran)
        return boost::make_shared<CPUnknownKind>();

      // ----- Miscelaneous ----
      case V_SgAssignOp:
        // This should be handled inside the ConstantPropagationAnalysis transfer function
        assert(0);

      case V_SgCommaOpExp:
        // This should be handled inside the control flow functionality
        assert(0);

      default:
        return boost::make_shared<CPUnknownKind>();
    }
  } else if(that->getKind() == CPValueKind::offsetList) {
    // This is not yet implemented so this assert will notify us when we need to do the heavy lifting
    assert(0);
  }

  // We've enumerated all cases so we should never get here
  cerr << "ERROR: no support for "<<SgNode2Str(op)<<"!";
  assert(0);
}

// Returns whether this and that CPValueKinds are may/must equal to each other
bool CPOffsetListKind::mayEqualV(CPValueKindPtr that)
{
  // If that unknown or uninitialized, use its implementation
  if(that->getKind() == CPValueKind::uninitialized || that->getKind() == CPValueKind::unknown)
    return that->mayEqualV(shared_from_this());

  if(that->getKind() == CPValueKind::concrete) {
    CPConcreteKindPtr thatConcrete = that->asConcreteKind();

    // If this object denotes a concrete value
    if(offsetL.size()==1 && offsetL.begin()->getType()==intWrap::offsetT) {
      long long thatSV;
      unsigned long long thatUSV;
      // If that object is a concrete integer (as opposed to some other concrete value), use == comparison
      if(IsSignedConstInt(thatConcrete->getVal().get(), thatSV)) {
        return offsetL.begin()->get() == thatSV;
      } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), thatUSV)) {
        return offsetL.begin()->get() == (long long)thatUSV;
      // If that object is not an integer, they may not be equal
      } else
        return false;
    // If this object denotes some combination of ranks and concrete values
    } else
      // Conservatively answer that they're may-equal
      return true;
  }

  if(that->getKind() == CPValueKind::offsetList) {
    CPOffsetListKindPtr thatOffset = that->asOffsetListKind();

    list<intWrap>::iterator itThis = offsetL.begin();
    list<intWrap>::iterator itThat = thatOffset->offsetL.begin();
    for(; itThis!=offsetL.end() && itThat!=thatOffset->offsetL.end(); itThis++, itThat++) {
      // Check if the current entry in both lists is identical. If not, they're not may-equal
      if(itThis->getType() != itThat->getType()) return false;
      if(itThis->get()     != itThat->get())     return false;
    }
    // We've reached the end of one or both of the offset lists

    // If we've reached the end of both offset lists, then the objects denote the same offset
    if(itThis==offsetL.end() && itThat==thatOffset->offsetL.end()) return true;

    // We've reached the end of only one offset list, meaning that the other object refers to some
    // deeper offset within the parent memory region. The objects may denote the same offset only if
    // the un-finished object's remaining offsetL elements all denote a 0 offset or 0 rank
    // !!!GB: I'm not sure about the 0 rank since there may be additional padding!!!
    if(itThis==offsetL.end())
      for(; itThat!=thatOffset->offsetL.end(); itThat++) { if(itThat->get()!=0) return false; }
    if(itThat==thatOffset->offsetL.end())
      for(; itThis!=offsetL.end(); itThis++) { if(itThis->get()!=0) return false; }

    // All the offsets are 0, so the two objects are may-equal
    return true;
  }

  // We've covered all the cases
  assert(0);
}

bool CPOffsetListKind::mustEqualV(CPValueKindPtr that) {
  // If that unknown or uninitialized, use its implementation
  if(that->getKind() == CPValueKind::uninitialized || that->getKind() == CPValueKind::unknown)
    return that->mayEqualV(shared_from_this());

  if(that->getKind() == CPValueKind::concrete) {
    CPConcreteKindPtr thatConcrete = that->asConcreteKind();

    // If this object denotes a concrete value
    if(offsetL.size()==1 && offsetL.begin()->getType()==intWrap::offsetT) {
      long long thatSV;
      unsigned long long thatUSV;
      // If that object is a concrete integer (as opposed to some other concrete value), use == comparison
      if(IsSignedConstInt(thatConcrete->getVal().get(), thatSV)) {
        return offsetL.begin()->get() == thatSV;
      } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), thatUSV)) {
        return offsetL.begin()->get() == (long long)thatUSV;
      // If that object is not an integer, they may not be equal
      } else
        return false;
    // If this object denotes some combination of ranks and concrete values
    } else
      // Conservatively answer that they're not must-equal
      return false;
  }

  if(that->getKind() == CPValueKind::offsetList) {
    CPOffsetListKindPtr thatOffset = that->asOffsetListKind();

    list<intWrap>::iterator itThis = offsetL.begin();
    list<intWrap>::iterator itThat = thatOffset->offsetL.begin();
    for(; itThis!=offsetL.end() && itThat!=thatOffset->offsetL.end(); itThis++, itThat++) {
      // Check if the current entry in both lists is identical. If not, they're not may-equal
      if(itThis->getType() != itThat->getType()) return false;
      if(itThis->get()     != itThat->get())     return false;
    }
    // We've reached the end of one or both of the offset lists

    // If we've reached the end of both offset lists, then the objects denote the same offset
    if(itThis==offsetL.end() && itThat==thatOffset->offsetL.end()) return true;

    // We've reached the end of only one offset list, meaning that the other object refers to some
    // deeper offset within the parent memory region. The objects may denote the same offset only if
    // the un-finished object's remaining offsetL element is a single concrete value 0.
    if(itThis==offsetL.end() && itThat->getType()!=intWrap::offsetT && itThat->get()==0) return true;
    if(itThat==thatOffset->offsetL.end() && itThis->getType()!=intWrap::offsetT && itThis->get()==0) return true;

    // The trailing offset is not a concrete 0, so the objects are not must-equal
    return false;
  }

  // We've covered all the cases
  assert(0);
}

// Returns whether the two CPValueKinds denote the same set of concrete values
bool CPOffsetListKind::equalSetV(CPValueKindPtr that) {
  // The logic here is the same as mustEquals
  return mustEqualV(that);
}

// Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given CPValueKind
bool CPOffsetListKind::subSetV(CPValueKindPtr that) {
  // The logic here is the same as mustEquals
  return mustEqualV(that);
}

// Computes the meet of this and that and returns the resulting kind
pair<bool, CPValueKindPtr> CPOffsetListKind::meetUpdateV(CPValueKindPtr that)
{
  // OffsetList MEET Uninitialized => OffsetList
  if(that->getKind() == CPValueKind::uninitialized)
    return make_pair(true, copyV());

  // OffsetList MEET Unknown => Unknown
  if(that->getKind() == CPValueKind::unknown)
    return make_pair(true, boost::make_shared<CPUnknownKind>());

  // OffsetList MEET Concrete
  if(that->getKind() == CPValueKind::concrete) {
    CPConcreteKindPtr thatConcrete = that->asConcreteKind();

    // If this offset is just a single concrete value
    if(offsetL.size()==1 && offsetL.back().getType()==CPOffsetListKind::intWrap::offsetT) {
      // If both objects denote the same concrete value
      long long thatSV;
      unsigned long long thatUSV;
      if(IsSignedConstInt(thatConcrete->getVal().get(), thatSV)) {
        if(offsetL.back().get()==thatSV) return make_pair(false, copyV());
      } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), thatUSV)) {
        if(offsetL.back().get()==(long long)thatUSV) return make_pair(false, copyV());
      }
    }
    // If the two objects do not denote the same concrete value
    return make_pair(true, boost::make_shared<CPUnknownKind>());
  }

  // OffsetList MEET OffsetList
  if(that->getKind() == CPValueKind::offsetList) {
    CPOffsetListKindPtr thatOffset = that->asOffsetListKind();
    // Compare the two offset lists directly
    // !!! (NOTE: this comparison doesn't take types into account when ranks are compared)
    if(offsetL.size() == thatOffset->offsetL.size()) {
      list<intWrap>::const_iterator thisI = offsetL.begin();
      list<intWrap>::const_iterator thatI = thatOffset->offsetL.begin();
      for(; thisI!=offsetL.end(); thisI++, thatI++) {
        // If the two offsetList objects are not identical
        if(*thisI != *thatI) return make_pair(true, boost::make_shared<CPUnknownKind>());
      }
      // If we reached this point the two offset lists must be identical
      return make_pair(false, copyV());
    // The two objects have lists of different sizes, so they're not identical
    } else
      return make_pair(true, boost::make_shared<CPUnknownKind>());
  }

  // We've covered all the cases
  assert(0);
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool CPOffsetListKind::isConcrete()
{ return offsetL.size()==1 && offsetL.back().getType()==CPOffsetListKind::intWrap::offsetT; }

// Returns the type of the concrete value (if there is one)
SgType* CPOffsetListKind::getConcreteType()
// !!! NOTE: this may be a memory leak
{ return type; }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > CPOffsetListKind::getConcreteValue() {
  assert(isConcrete());
  std::set<boost::shared_ptr<SgValueExp> > vals;
  vals.insert(boost::shared_ptr<SgLongLongIntVal>(SageBuilder::buildLongLongIntVal(offsetL.back().get())));
  return vals;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool CPOffsetListKind::isFullV(PartEdgePtr pedge) { return false; }
// Returns whether this AbstractObject denotes the empty set.
bool CPOffsetListKind::isEmptyV(PartEdgePtr pedge) { return false; }

std::string CPOffsetListKind::str(std::string indent) {
  ostringstream oss;

  oss <<"[CPOffsetListKind: offsetL=";
  for(list<intWrap>::iterator o=offsetL.begin(); o!=offsetL.end(); o++) {
    if(o!=offsetL.begin()) oss << ",";
    oss << o->get();
  }
  oss << "]";

  return oss.str();
}

// *****************************
// **** CPUnknownKind *****
// *****************************

// Applies the given unary or binary operation to this and the given CPValueKind
// Returns:
//    - if this CPValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated CPValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent CPValueObject's list, return that.
CPValueKindPtr CPUnknownKind::op(SgUnaryOp* op) {
  // Uninitialized denotes the full value set, so any operation applied to it results in the full set
  return copyV();
}

CPValueKindPtr CPUnknownKind::op(SgBinaryOp* op, CPValueKindPtr that) {
  // Uninitialized denotes the full value set, so any operation that involves it results in the full set
  return copyV();
}

// Returns whether this and that CPValueKinds are may/must equal to each other
bool CPUnknownKind::mayEqualV(CPValueKindPtr that) {
  // Unknown denotes the full set, which overlaps with every other set
  return true;
}

bool CPUnknownKind::mustEqualV(CPValueKindPtr that) {
  // Unknown denotes the full set, which has unbounded size and therefore is not must-equal to any set
  return false;
}

// Returns whether the two CPValueKinds denote the same set of concrete values
bool CPUnknownKind::equalSetV(CPValueKindPtr that) {
  // Unknown denotes the full set, which may only be equal to another full set
  return that->getKind() == CPValueKind::unknown;
}

// Returns whether this CPValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given CPValueKind
bool CPUnknownKind::subSetV(CPValueKindPtr that) {
  // Unknown  denotes the full set, which is a subset of another full set
  return that->getKind() == CPValueKind::unknown;
}

// Computes the meet of this and that and returns the resulting kind
pair<bool, CPValueKindPtr> CPUnknownKind::meetUpdateV(CPValueKindPtr that)
{
  bool modified = that->getKind() != CPValueKind::unknown;
  // Unknown MEET * => Unknown
  return make_pair(modified, copyV());
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool CPUnknownKind::isConcrete()
{ return false; }

// Returns the type of the concrete value (if there is one)
SgType* CPUnknownKind::getConcreteType()
{ return NULL; }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > CPUnknownKind::getConcreteValue()
{ return std::set<boost::shared_ptr<SgValueExp> >(); }

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool CPUnknownKind::isFullV(PartEdgePtr pedge) { return true; }
// Returns whether this AbstractObject denotes the empty set.
bool CPUnknownKind::isEmptyV(PartEdgePtr pedge) { return false; }

std::string CPUnknownKind::str(std::string indent)
{ return "[CPUnknownKind]"; }


// *************************
// **** CPMemLocObject *****
// *************************

// returns a copy of this lattice
Lattice* CPMemLocObject::copy() const {
  return new CPMemLocObject(*this);
}

bool
CPMemLocObject::operator==(Lattice* X) /*const*/
{
  // Implementation of equality operator.
  CPMemLocObject* that = dynamic_cast<CPMemLocObject*>(X);
  assert(that);
  return (this->getRegion() == that->getRegion() &&
          this->getIndex()  == that->getIndex());
}

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
Lattice* CPMemLocObject::remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge) {
  if(isCPFull || isCPEmpty) return dynamic_cast<CPMemLocObject*>(copyMLPtr());

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
}

// Returns whether this object may/must be equal to o within the given Part p
// These methods are called by composers and should not be called by analyses.
bool CPMemLocObject::mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge) {
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
  return region && region->mayEqual(that->getRegion(), pedge, analysis->getComposer(), analysis) &&
         ((!index && !that->index) || index->mayEqualV(that->getIndex(), pedge));
}

bool CPMemLocObject::mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge) {
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
         ((!index && !that->index) || index->mustEqualV(that->getIndex(), pedge));
}

// Returns whether the two abstract objects denote the same set of concrete objects
// These methods are called by composers and should not be called by analyses.
bool CPMemLocObject::equalSetML(MemLocObjectPtr o, PartEdgePtr pedge) {
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
         ((!index && !that->index) || index->equalSetV(that->getIndex(), pedge));
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
// These methods are called by composers and should not be called by analyses.
bool CPMemLocObject::subSetML(MemLocObjectPtr o, PartEdgePtr pedge) {
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
         ((!index && !that->index) || index->subSetV(that->getIndex(), pedge));
}

// Returns true if this object is live at the given part and false otherwise
bool CPMemLocObject::isLiveML(PartEdgePtr pedge) {
  if(isCPFull || isCPEmpty) return true;

  // For now all CPMemLocs are live but in the future we may restrict this only to the CFGNode
  // of the Base expression.
  return true;
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

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool CPMemLocObject::isFullLat() {
  return isFullML(getPartEdge());
}

// Returns whether this lattice denotes the empty set.
bool CPMemLocObject::isEmptyLat() {
  return isEmptyML(getPartEdge());
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool CPMemLocObject::isFullML(PartEdgePtr pedge) {
  if(isCPFull) return true;

  // Compare the region and index directly. We call isFullMR here because CP does not implement
  // MemRegions, meaning that we need to thread the logic through the composer to the real implementer.
  // However, values are implemented in CP, meaning that we can call isFullV directly
  return region->isFull(pedge, analysis->getComposer(), analysis) &&
         (!index || index->isFullV(pedge));

}

// Returns whether this AbstractObject denotes the empty set.
bool CPMemLocObject::isEmptyML(PartEdgePtr pedge) {
  if(isCPEmpty) return true;

  // Compare the region and index directly. We call isEmptyMR here because CP does not implement
  // MemRegions, meaning that we need to thread the logic through the composer to the real implementer.
  // However, values are implemented in CP, meaning that we can call isEmptyV directly
  return region->isEmpty(pedge, analysis->getComposer(), analysis) &&
         (!index || index->isEmptyV(pedge));
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool CPMemLocObject::meetUpdateML(MemLocObjectPtr that_arg, PartEdgePtr pedge) {
  CPMemLocObjectPtr that = boost::dynamic_pointer_cast<CPMemLocObject>(that_arg);
  assert(that);
  return meetUpdate(that.get(), pedge, analysis->getComposer(), analysis);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
// The part of this object is to be used for AbstractObject comparisons.
bool CPMemLocObject::meetUpdate(Lattice* that_arg)
{
  CPMemLocObject* that = dynamic_cast<CPMemLocObject*>(that_arg);
  assert(that);
  return meetUpdate(that, getPartEdge(), analysis->getComposer(), analysis);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
// The part of this object is to be used for AbstractObject comparisons.
bool CPMemLocObject::meetUpdate(CPMemLocObject* that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  scope S("CPMemLocObject::meetUpdate");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that->str()<<endl;
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
    // Meet the indexes
    bool modified = getRegion()->meetUpdate(that->getRegion(), pedge, comp, analysis);

    // Meet the indexes, using a direct call to CPValueObject::meetUpdate since we know that both indexes are
    // CPValueObjects.
    modified = boost::dynamic_pointer_cast<CPValueObject>(getIndex())->meetUpdate(
                               boost::dynamic_pointer_cast<CPValueObject>(that->getIndex()).get()) || modified;

    return modified;
  }
}

// Allocates a copy of this object and returns a pointer to it
MemLocObjectPtr CPMemLocObject::copyML() const {
  return boost::make_shared<CPMemLocObject>(*this);
}

  // Allocates a copy of this object and returns a regular pointer to it
MemLocObject* CPMemLocObject::copyMLPtr() const {
  return new CPMemLocObject(*this);
}

std::string CPMemLocObject::str(std::string indent) { // pretty print for the object
  if(isCPFull)       return "[CPMemLocObject: Full]";
  else if(isCPEmpty) return "[CPMemLocObject: Empty]";

  ostringstream oss;
  oss << "[CPMemLocObject this="<<this<<", region="<<(region?region->str(indent+"    "):"NULL");
  if(index) {
    oss <<", "<<endl<<
         indent << "             index="<<index->str(indent+"    ");
  }
  oss <<"]";

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
  scope s("ConstantPropagationAnalysisTransfer::visit(SgDotExp *dot)", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

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
        core = boost::dynamic_pointer_cast<CPMemLocObject>(cpML->copyML());
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

  MemLocObjectPtr ml = composer->OperandExpr2MemLoc(dot, dot->get_rhs_operand(), part->inEdgeFromAny(), analysis);
  UnionMemLocObjectPtr mlUnion = boost::dynamic_pointer_cast<UnionMemLocObject>(ml);
  assert(mlUnion);
  const std::list<MemLocObjectPtr>& mlVals = mlUnion->getMemLocs();
  assert(mlVals.size()==1);
  CPMemLocObjectPtr core = boost::dynamic_pointer_cast<CPMemLocObject>(*mlVals.begin());
  assert(core);

  // Compute the offset into the region of the core MemLoc that results from the dot expression
  CPValueObjectPtr offset = core->getCPIndex()->op(dot, NULLCPValueObject);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "offset="<<(offset? offset->str(): "NULL")<<endl;

  // Update this node's MemLoc to use the same region as core but with the new offset

  /*if(dfInfo[NULLPartEdge][1]) delete dfInfo[NULLPartEdge][1];
  dfInfo[NULLPartEdge][1] = new CPMemLocObject(core->getRegion(), offset, dot, part->inEdgeFromAny(), analysis);
  dbg << "dfInfo[1]="<<dfInfo[NULLPartEdge][1]->str()<<endl;*/
  CPMemLocObjectPtr dotML = boost::make_shared<CPMemLocObject>(core->getRegion(), offset, dot, part->inEdgeFromAny(), analysis);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "dotML="<<(dotML? dotML->str(): "NULL")<<endl;
  //cl2ml->insert(cl, dotML);
  nodeState.addFact(analysis, 0, new CPMemLocObjectNodeFact(dotML));
}

void ConstantPropagationAnalysisTransfer::visit(SgPntrArrRefExp *paRef) {
  scope s("ConstantPropagationAnalysisTransfer::visit(SgPntrArrRefExp *paRef)", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

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
  MemLocObjectPtr ml = composer->OperandExpr2MemLoc(paRef, paRef->get_lhs_operand(), part->inEdgeFromAny(), analysis);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "ml="<<(ml? ml->str(): "NULL")<<endl;
  UnionMemLocObjectPtr mlUnion = boost::dynamic_pointer_cast<UnionMemLocObject>(ml);
  assert(mlUnion);
  const std::list<MemLocObjectPtr>& mlVals = mlUnion->getMemLocs();
  assert(mlVals.size()==1);
  CPMemLocObjectPtr core = boost::dynamic_pointer_cast<CPMemLocObject>(*mlVals.begin());
  assert(core);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "core="<<(core? core->str(): "NULL")<<endl;

  // In expression array[i], the value location denoted by "i"
  ValueObjectPtr val = composer->OperandExpr2Val(paRef, paRef->get_rhs_operand(), part->inEdgeFromAny(), analysis);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "val="<<val->str()<<endl;
  UnionValueObjectPtr indexUnion = boost::dynamic_pointer_cast<UnionValueObject>(val);
  assert(indexUnion);
  const std::list<ValueObjectPtr>& indexVals = indexUnion->getVals();
  assert(indexVals.size()==1);
  CPValueObjectPtr index = boost::dynamic_pointer_cast<CPValueObject>(*indexVals.begin());
  assert(index);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "index="<<index->str()<<endl;

  // Compute the offset into the region of the core MemLoc that results from the arraypntr reference expression
  CPValueObjectPtr offset = core->getCPIndex()->op(paRef, index);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "offset="<<offset->str()<<endl;

  CPMemLocObjectPtr paRefML = boost::make_shared<CPMemLocObject>(core->getRegion(), offset, paRef, part->inEdgeFromAny(), analysis);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "paRefML="<<(paRefML? paRefML->str(): "NULL")<<endl;
  //cl2ml->insert(cl, paRefML);
  nodeState.addFact(analysis, 0, new CPMemLocObjectNodeFact(paRefML));
}

void ConstantPropagationAnalysisTransfer::visit(SgBinaryOp *sgn) {
  scope s("ConstantPropagationAnalysisTransfer::visit(SgBinaryOp)", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

  // Only bother to consider operators with short-circuiting a the end of the operator so that
  // all of its operands precede the operator
  if((isSgAndOp(sgn) || isSgOrOp(sgn)) && cn.getIndex()!=2) return;

  CPValueObjectPtr arg1Lat, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat, arg2Lat);//, resLat_tmp);
  CPValueObjectPtr resLat = arg1Lat->op(sgn, arg2Lat);

//prodLat->setToEmpty();

  setLattice(sgn, resLat);
  if(isSgCompoundAssignOp(sgn))
    setLatticeOperand(sgn, sgn->get_lhs_operand(), boost::dynamic_pointer_cast<CPValueObject>(resLat->copyV()));
}

// Unary ops that update the operand
void ConstantPropagationAnalysisTransfer::visit(SgMinusMinusOp *sgn) {
  scope s("ConstantPropagationAnalysisTransfer::visit(SgMinusMinusOp)", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

  CPValueObjectPtr arg1Lat;//, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
  CPValueObjectPtr resLat = arg1Lat->op(sgn);
//prodLat->setToEmpty();
  setLattice(sgn, resLat);
  setLatticeOperand(sgn, sgn->get_operand(), resLat);
}

void ConstantPropagationAnalysisTransfer::visit(SgPlusPlusOp *sgn) {
  scope s("ConstantPropagationAnalysisTransfer::visit(SgPlusPlusOp)", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

  CPValueObjectPtr arg1Lat;//, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
  CPValueObjectPtr resLat = arg1Lat->op(sgn);
//prodLat->setToEmpty();
  setLattice(sgn, resLat);
  setLatticeOperand(sgn, sgn->get_operand(), resLat);
}

// Unary ops that do not update the operand
void ConstantPropagationAnalysisTransfer::visit(SgUnaryOp *sgn) {
  scope s("ConstantPropagationAnalysisTransfer::visit(SgUnaryOp)", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

  CPValueObjectPtr arg1Lat;//, arg2Lat;//, resLat_tmp;
  getLattices(sgn, arg1Lat);//, arg2Lat);//, resLat_tmp);
  CPValueObjectPtr resLat = arg1Lat->op(sgn);
//prodLat->setToEmpty();
  setLattice(sgn, resLat);
}

void ConstantPropagationAnalysisTransfer::visit(SgValueExp *val) {
  scope reg("ConstantPropagationAnalysisTransfer::visit(SgValExp)", scope::low, attrGE("constantPropagationAnalysisDebugLevel", 1));
  assert(val);

//prodLat->setToEmpty();
  SgTreeCopy copyHelp;
  boost::shared_ptr<SgValueExp> valCopy((SgValueExp*)(val->copy(copyHelp)));
  setLattice(val, boost::make_shared<CPValueObject>(boost::make_shared<CPConcreteKind>(valCopy), part->inEdgeFromAny()));
}


bool
ConstantPropagationAnalysisTransfer::finish()
{
  return modified;
}

ConstantPropagationAnalysisTransfer::ConstantPropagationAnalysisTransfer(
          PartPtr part, PartPtr supersetPart, CFGNode cn, NodeState& state,
          map<PartEdgePtr, vector<Lattice*> >& dfInfo,
          Composer* composer, ConstantPropagationAnalysis* analysis)
   : VariableStateTransfer<CPValueObject, ConstantPropagationAnalysis>
                       (state, dfInfo, boost::make_shared<CPValueObject>(part->inEdgeFromAny()),
                        composer, analysis, part, supersetPart, cn,
                        constantPropagationAnalysisDebugLevel, "constantPropagationAnalysisDebugLevel")
{
}




// **********************************************************************
//             ConstantPropagationAnalysis
// **********************************************************************

// GB: Is this needed for boost shared-pointers?
ConstantPropagationAnalysis::ConstantPropagationAnalysis()
{
}

// Creates a basic CPMemLocObject for the given SgNode. This object does not take into
// account any constant propagation and will be used as a seed from which to propagate
// more precise information.
CPMemLocObjectPtr ConstantPropagationAnalysis::createBasicCPML(SgNode* n, PartEdgePtr pedge) {
  MemRegionObjectPtr curMR = composer->Expr2MemRegion(n, pedge, this);

  // If this expression denotes the starting point of a memory region, create a MemLocObject
  // that is explicitly at the start of mr
  CPMemLocObjectPtr ml;
  if(isSgVarRefExp(n) || isSgInitializedName(n))
    return boost::make_shared<CPMemLocObject>(
                                curMR, boost::make_shared<CPValueObject>(
                                           boost::make_shared<CPOffsetListKind>(CPOffsetListKind::offset(0)),
                                           pedge),
                                n, pedge, this);
  // Otherwise, create one that refers to an unknown offset within mr
  else
    return boost::make_shared<CPMemLocObject>(
                                curMR, boost::make_shared<CPValueObject>(
            // !!! Should create ServerImplKind here!!!
                                           boost::make_shared<CPOffsetListKind>(CPOffsetListKind::offset(0)),
                                           pedge),
                                       // boost::make_shared<CPValueObject>(boost::make_shared<CPUninitializedKind>(), pedge),
                                n, pedge, this);

}

// Initializes the state of analysis lattices at the given function, part and edge into our out of the part
// by setting initLattices to refer to freshly-allocated Lattice objects.
void ConstantPropagationAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge, PartPtr supersetPart,
                                                 vector<Lattice*>& initLattices)
{
  scope sEdge("genInitLattice", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 2));

  AbstractObjectMap* ml2val = new AbstractObjectMap(boost::make_shared<CPValueObject>(pedge),
                                                    pedge,
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
  scope s("ConstantPropagationAnalysis::genInitLattice()", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

  // If this expression denotes the starting point of a memory region, create a MemLocObject
  // that is explicitly at the start of mr
  if(isSgVarRefExp(n.getNode()))
    ml = new CPMemLocObject(mr, boost::make_shared<CPValueObject>(
                                         boost::make_shared<CPConcreteKind>(
                                                  boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(0))), pedge),
                            n.getNode(), pedge, this);
  // Otherwise, create one that refers to an unknown offset within mr
  else
    ml = new CPMemLocObject(mr, boost::make_shared<CPValueObject>(boost::make_shared<CPUninitializedKind>(), pedge),
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
ConstantPropagationAnalysis::transfer(PartPtr part, PartPtr supersetPart, CFGNode cn, NodeState& state,
                                      map<PartEdgePtr, vector<Lattice*> >& dfInfo)
{
  assert(0);
  return false;
}

boost::shared_ptr<DFTransferVisitor>
ConstantPropagationAnalysis::getTransferVisitor(PartPtr part, PartPtr supersetPart, CFGNode cn, NodeState& state,
                                                map<PartEdgePtr, vector<Lattice*> >& dfInfo)
{
  // Why is the boost shared pointer used here?
  ConstantPropagationAnalysisTransfer* t = new ConstantPropagationAnalysisTransfer(part, supersetPart, cn, state, dfInfo, getComposer(), this);
  return boost::shared_ptr<DFTransferVisitor>(t);
}

ValueObjectPtr ConstantPropagationAnalysis::Expr2Val(SgNode* n, PartEdgePtr pedge)
{
  scope s(txt()<<"ConstantPropagationAnalysis::Expr2Val(n="<<SgNode2Str(n)<<", pedge="<<pedge->str()<<")", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

  MemLocObjectPtr ml = getComposer()->Expr2MemLoc(n, pedge, this);
  if(constantPropagationAnalysisDebugLevel()>=1) dbg << "ml="<<(ml? ml->str(): "NULL")<<endl;

  // If pedge doesn't have wildcards
  if(pedge->source() && pedge->target()) {
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->source());
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "state="<<state->str(this)<<endl;

    // Get the value map at the current edge
    AbstractObjectMap* cpMap = dynamic_cast<AbstractObjectMap*>(state->getLatticeBelow(this, pedge, 0));
    if(cpMap == NULL) {
      Lattice* l = state->getLatticeBelow(this, pedge, 0);
      dbg << "l="<<l->str()<<endl;
    }
    assert(cpMap);

    // We currently can only handle requests for the SgNode that corresponds to the current Part
    set<CFGNode> nodes = pedge->source()->CFGNodes();
    assert(nodes.size()==1);
    assert(nodes.begin()->getNode() == n);

    // Get the MemLoc at the source part
    if(constantPropagationAnalysisDebugLevel()>=2) {
      indent ind;
      dbg << "cpMap Below="<<cpMap<<"="<<cpMap->str()<<endl;
      dbg << "nodeState = "<<state->str()<<endl;
    }

    // Return the lattice associated with n's expression
    CPValueObjectPtr val = boost::dynamic_pointer_cast<CPValueObject>(cpMap->get(ml));
    assert(val);
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "val="<<val->str()<<endl;

    return val->copyV();
  // If the target of this edge is a wildcard
  } else if(pedge->source()) {
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->source());
    //dbg << "state="<<state->str(this)<<endl;

    map<PartEdgePtr, vector<Lattice*> >& e2lats = state->getLatticeBelowAllMod(this);
    assert(e2lats.size()>=1);
    CPValueObjectPtr mergedVal;
    for(map<PartEdgePtr, vector<Lattice*> >::iterator lats=e2lats.begin(); lats!=e2lats.end(); lats++) {
      PartEdge* edgePtr = lats->first.get();
      assert(edgePtr->source() == pedge.get()->source());
      scope sEdge(txt()<<"edge "<<lats->first.get()->str(), scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

      // Get the value map at the current edge
      AbstractObjectMap* cpMap = dynamic_cast<AbstractObjectMap*>(state->getLatticeBelow(this, lats->first, 0));
      assert(cpMap);

      //MemLocObjectPtr p = composer->Expr2MemLoc(n, pedge, this);
      // We currently can only handle requests for the SgNode that corresponds to the current Part
      set<CFGNode> nodes = pedge->source()->CFGNodes();
      assert(nodes.size()==1);
      assert(nodes.begin()->getNode() == n);

      if(constantPropagationAnalysisDebugLevel()>=2) {
        indent ind;
        dbg << "cpMap="<<cpMap<<"="<<cpMap->str()<<endl;
      }

      CPValueObjectPtr val = boost::dynamic_pointer_cast<CPValueObject> (boost::dynamic_pointer_cast<ValueObject>(cpMap->get(ml)));
      if(constantPropagationAnalysisDebugLevel()>=1) dbg << "val="<<val->str()<<endl;

      if(lats==e2lats.begin())
        mergedVal = boost::dynamic_pointer_cast<CPValueObject>(val->copyV());
      else
        mergedVal->meetUpdate(val.get());

      if(constantPropagationAnalysisDebugLevel()>=1) dbg << "mergedVal="<<mergedVal->str()<<endl;
    }
    return mergedVal;

  // If the source of this edge is a wildcard
  } else if(pedge->target()) {
    // Get the NodeState at the target of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->target());
    if(constantPropagationAnalysisDebugLevel()>=2) dbg << "state="<<state->str()<<endl;

    // Get the value map at the NULL edge, which denotes the meet over all incoming edges
    AbstractObjectMap* cpMap = dynamic_cast<AbstractObjectMap*>(state->getLatticeAbove(this, NULLPartEdge, 0));
    assert(cpMap);

    if(constantPropagationAnalysisDebugLevel()>=2) {
      indent ind;
      dbg << "cpMap="<<cpMap<<"="<<cpMap->str()<<endl;
    }

    // Return the lattice associated with n's expression since that is likely to be more precise
    CPValueObjectPtr val = boost::dynamic_pointer_cast<CPValueObject>(cpMap->get(ml));
    assert(val);
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "val="<<val->str()<<endl;

    return val->copyV();
  }
  assert(0);
}

MemLocObjectPtr ConstantPropagationAnalysis::Expr2MemLoc(SgNode* n, PartEdgePtr pedge) {
  scope s(txt()<<"ConstantPropagationAnalysis::Expr2MemLoc(n="<<SgNode2Str(n)<<", pedge="<<pedge->str()<<")", scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));

  // SgInitializedNames denote entities that are lexically known and thus do not require
  // any special handling by ConstantPropagation Analysis
  //if(isSgInitializedName(n) || isSgVarRefExp(n)) {
  if(!isSgDotExp(n) && !isSgPntrArrRefExp(n)) {
    return createBasicCPML(n, pedge);
  }

  // NOTE: this is a temporary hack where we assume the appropriate index for the CFGNode
  //       that represents SgNode n. In the future we should change Expr2* to accept CFGNodes
  CFGNode cn;
       if(isSgBinaryOp(n) ||
          isSgUnaryOp(n))     cn = CFGNode(n, 2);
  else if(isSgValueExp(n))    cn = CFGNode(n, 1);
  else                        cn = CFGNode(n, 0);

  // Confirm that n corresponds to the source part
  if(pedge->source()) {
    assert(pedge->source()->CFGNodes().size()==1);
    //assert(pedge->source()->CFGNodes().begin()->getNode() == n);
  } else if(pedge->target()) {
    assert(pedge->target()->CFGNodes().size()==1);
    assert(pedge->target()->CFGNodes().begin()->getNode() == n);
  }

  if(pedge->source()) {
    scope s(txt()<<"Source: "<<pedge->source()->str(), scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));
    NodeState* state = NodeState::getNodeState(this, pedge->source());
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "state="<<state->str()<<endl;
  }

  if(pedge->target()) {
    scope s(txt()<<"target: "<<pedge->target()->str(), scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));
    NodeState* state = NodeState::getNodeState(this, pedge->target());
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "state="<<state->str()<<endl;
  }

  // If pedge doesn't have wildcards
  if(pedge->source() && pedge->target()) {
    CodeLocObjectPtr cl = boost::make_shared<CodeLocObject>(pedge->source(), cn);
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "cl="<<cl->str()<<endl;

    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->source());
    if(constantPropagationAnalysisDebugLevel()>=3) dbg << "state="<<state->str()<<endl;

    /* // Get the memory locaiton at the current edge
    AbstractObjectMap* cl2ml = dynamic_cast<AbstractObjectMap*>(state->getLatticeBelow(this, pedge, 1));
    assert(cl2ml);

    // Get the memory locaiton at the current edge
    CPMemLocObjectPtr ml = boost::dynamic_pointer_cast<CPMemLocObject>(cl2ml->get(cl));*/
    NodeFact* mlFact = state->getFact(this, 0);
    CPMemLocObjectNodeFact* cpmlFact = dynamic_cast<CPMemLocObjectNodeFact*>(mlFact);
    assert(cpmlFact);
    CPMemLocObjectPtr ml = cpmlFact->ml;
    assert(ml);
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

    return ml->copyML();
  // If the target of this edge is a wildcard
  } else if(pedge->source()) {
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->source());
    if(constantPropagationAnalysisDebugLevel()>=2) dbg << "state="<<state->str()<<endl;

    map<PartEdgePtr, vector<Lattice*> >& e2lats = state->getLatticeBelowAllMod(this);
    assert(e2lats.size()>=1);
    CPMemLocObjectPtr mergedML;
    for(map<PartEdgePtr, vector<Lattice*> >::iterator lats=e2lats.begin(); lats!=e2lats.end(); lats++) {
      scope sEdge(txt()<<"edge "<<lats->first.get()->str(), scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));
      PartEdgePtr edge = lats->first;
      assert(edge.get()->source() == pedge.get()->source());

      // NOTE: for now we're assuming that the CFGNode index is 0 but this will need to be corrected
      CodeLocObjectPtr cl = boost::make_shared<CodeLocObject>(pedge->source(), cn);
      if(constantPropagationAnalysisDebugLevel()>=1) dbg << "cl="<<cl->str();

      // Get the memory locaiton at the current edge
      /*AbstractObjectMap* cl2ml = dynamic_cast<AbstractObjectMap*>(state->getLatticeBelow(this, edge, 1));
      assert(cl2ml);

      // Get the memory locaiton at the current edge
      CPMemLocObjectPtr ml = boost::dynamic_pointer_cast<CPMemLocObject>(cl2ml->get(cl));*/
      NodeFact* mlFact = state->getFact(this, 0);
      CPMemLocObjectNodeFact* cpmlFact = dynamic_cast<CPMemLocObjectNodeFact*>(mlFact);
      assert(cpmlFact);
      CPMemLocObjectPtr ml = cpmlFact->ml;
      assert(ml);
      if(constantPropagationAnalysisDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

      if(lats==e2lats.begin())
        mergedML = boost::dynamic_pointer_cast<CPMemLocObject>(ml->copyML());
      else
        mergedML->meetUpdate(ml.get(), edge, getComposer(), this);

      if(constantPropagationAnalysisDebugLevel()>=1) dbg << "mergedML="<<mergedML->str()<<endl;
    }
    return mergedML;
  // If the source of this edge is a wildcard
  } else if(pedge->target()) {
    // Get the NodeState at the target of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->target());
    if(constantPropagationAnalysisDebugLevel()>=3) dbg << "state="<<state->str()<<endl;

    // NOTE: for now we're assuming that the CFGNode index is 0 but this will need to be corrected
    CodeLocObjectPtr cl = boost::make_shared<CodeLocObject>(pedge->target(), cn);
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "cl="<<cl->str();

    // Get the memory locaiton at the current edge
    /*AbstractObjectMap* cl2ml = dynamic_cast<AbstractObjectMap*>(state->getLatticeAbove(this, NULLPartEdge, 1));
    assert(cl2ml);

    // Get the memory locaiton at the current edge
    CPMemLocObjectPtr ml = boost::dynamic_pointer_cast<CPMemLocObject>(cl2ml->get(cl));*/
    NodeFact* mlFact = state->getFact(this, 0);
    CPMemLocObjectNodeFact* cpmlFact = dynamic_cast<CPMemLocObjectNodeFact*>(mlFact);
    assert(cpmlFact);
    CPMemLocObjectPtr ml = cpmlFact->ml;
    assert(ml);
    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

    return ml->copyML();
  }

  // If pedge doesn't have wildcards
  /*if(pedge->source() && pedge->target()) {
    // Confirm that n corresponds to the source part
    assert(pedge->source()->CFGNodes().size()==1);
    assert(pedge->source()->CFGNodes().begin()->getNode() == n);

    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->source());

    // Get the memory locaiton at the current edge
    CPMemLocObject* ml = dynamic_cast<CPMemLocObject*>(state->getLatticeBelow(this, pedge, 1));
    if(ml==NULL) { Lattice* l = state->getLatticeBelow(this, pedge, 1);dbg << "ml="<<(l? l->str(): "NULL")<<endl; }
    assert(ml);

    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

    return ml->copyML();
  // If the target of this edge is a wildcard
  } else if(pedge->source()) {
    // Get the NodeState at the source of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->source());

    map<PartEdgePtr, vector<Lattice*> >& e2lats = state->getLatticeBelowAllMod(this);
    assert(e2lats.size()>=1);
    CPMemLocObjectPtr mergedML;
    for(map<PartEdgePtr, vector<Lattice*> >::iterator lats=e2lats.begin(); lats!=e2lats.end(); lats++) {
      scope sEdge(txt()<<"edge "<<lats->first.get()->str(), scope::medium, attrGE("constantPropagationAnalysisDebugLevel", 1));
      PartEdge* edgePtr = lats->first.get();
      assert(edgePtr->source() == pedge.get()->source());

      // Confirm that n corresponds to the source part
      assert(pedge->source()->CFGNodes().size()==1);
      assert(pedge->source()->CFGNodes().begin()->getNode() == n);

      // Get the value map at the current edge
      CPMemLocObject* ml = dynamic_cast<CPMemLocObject*>(state->getLatticeBelow(this, lats->first, 1));
      if(ml==NULL) { Lattice* l = state->getLatticeBelow(this, pedge, 1);dbg << "ml="<<(l? l->str(): "NULL")<<endl; }
      assert(ml);

      if(constantPropagationAnalysisDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

      if(lats==e2lats.begin())
        mergedML = boost::dynamic_pointer_cast<CPMemLocObject>(ml->copyML());
      else
        mergedML->meetUpdate(ml, lats->first, getComposer(), this);

      if(constantPropagationAnalysisDebugLevel()>=1) dbg << "mergedML="<<mergedML->str()<<endl;
    }
    return mergedML;

  // If the source of this edge is a wildcard
  } else if(pedge->target()) {
    // Get the NodeState at the target of this edge
    NodeState* state = NodeState::getNodeState(this, pedge->target());
    if(constantPropagationAnalysisDebugLevel()>=2) dbg << "state="<<state->str()<<endl;

    // Get the value map at the current edge
    CPMemLocObject* ml = dynamic_cast<CPMemLocObject*>(state->getLatticeAbove(this, NULLPartEdge, 1));
    if(ml==NULL) { Lattice* l = state->getLatticeAbove(this, NULLPartEdge, 1);dbg << "ml="<<(l? l->str(): "NULL")<<endl; }
    assert(ml);

    if(constantPropagationAnalysisDebugLevel()>=1) dbg << "ml="<<ml->str()<<endl;

    return ml->copyML();
  }*/
  assert(0);
}

}; // namespace fuse;
