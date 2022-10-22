#include "sage3basic.h"

using namespace std;

#include "concrete_values.h"
#include "const_prop_analysis.h"
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <boost/make_shared.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>
#include "sageInterface.h"

#ifndef DISABLE_SIGHT
using namespace sight;
#endif

using namespace SageInterface;

#include <cwchar>

#define ConcreteDebugLevel 0

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
} // namespace lambda
} // namespace boost

namespace fuse {
ConcreteValueKindPtr NULLConcreteValueKind;

/**************************************
 ***** ConcreteUninitializedKind ******
 **************************************/

// Applies the given unary or binary operation to this and the given ConcreteValueKind
// Returns:
//    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated ConcreteValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent ConcreteValueObject's list, return that.
ConcreteValueKindPtr ConcreteUninitializedKind::op(SgUnaryOp* op) {
  // Uninitialized denotes the empty set, so any operation applied to it results in the empty set
  return copyAOType();
}

ConcreteValueKindPtr ConcreteUninitializedKind::op(SgBinaryOp* op, ConcreteValueKindPtr that) {
  // Uninitialized denotes the empty set, so any operation that involves it results in the empty set
  return copyAOType();
}

// Returns whether this and that ConcreteValueKinds are may/must equal to each other
bool ConcreteUninitializedKind::mayEqualAO(ConcreteValueKindPtr that)
{
  // Uninitialized denotes the empty set, which does not overlap with any other set
  return false;
}

bool ConcreteUninitializedKind::mustEqualAO(ConcreteValueKindPtr that) {
  // Uninitialized denotes the empty set, which may only be equal to another empty set
  return that->getKind() == ConcreteValueKind::uninitialized;
}

// Returns whether the two ConcreteValueKinds denote the same set of concrete values
bool ConcreteUninitializedKind::equalSetAO(ConcreteValueKindPtr that) {
  // Uninitialized denotes the empty set, which may only be equal to another empty set
  return (that->getKind() == ConcreteValueKind::uninitialized);
}

// Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given ConcreteValueKind
bool ConcreteUninitializedKind::subSetAO(ConcreteValueKindPtr that) {
  // Uninitialized denotes the empty set, which is a subset of every other set
  return true;
}

// Computes the meet of this and that and returns the resulting kind
pair<bool, ConcreteValueKindPtr> ConcreteUninitializedKind::meetUpdateAO(ConcreteValueKindPtr that)
{
  bool modified = that->getKind() != ConcreteValueKind::uninitialized;
  // Uninitialized MEET * => *
  return make_pair(modified, that->copyAOType());
}

// Computes the intersection of this and that and returns the resulting kind
std::pair<bool, ConcreteValueKindPtr> ConcreteUninitializedKind::intersectUpdateAO(ConcreteValueKindPtr that) {
  bool modified = false;
  // Uninitialized INTERSECT * => Uninitialized
  return make_pair(modified, copyAOType());
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool ConcreteUninitializedKind::isConcrete()
{ return false; }

// Returns the number of concrete values in this set
int ConcreteUninitializedKind::concreteSetSize()
{ return -1; }

// Returns the type of the concrete value (if there is one)
SgType* ConcreteUninitializedKind::getConcreteType()
{ return NULL; }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > ConcreteUninitializedKind::getConcreteValue()
{ return std::set<boost::shared_ptr<SgValueExp> >(); }

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool ConcreteUninitializedKind::isFullAO(PartEdgePtr pedge) { return false; }
// Returns whether this AbstractObject denotes the empty set.
bool ConcreteUninitializedKind::isEmptyAO(PartEdgePtr pedge) { return true; }

std::string ConcreteUninitializedKind::str(std::string indent) const
{ return "[ConcreteUninitializedKind]"; }

/*****************************
 ***** ConcreteExactKind *****
 *****************************/

// Creates a ConcreteExactKind from the given value. This function is overloaded with different argument types
// and for each type it creates a ConcreteExactKind with a different SgValueExp.
ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(bool val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(char val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildCharVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(short val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildShortVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(int val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(long val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongIntVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(long long val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(unsigned char val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedCharVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(unsigned short val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedShortVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(unsigned int val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedIntVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(unsigned long val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedLongVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(unsigned long long val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedLongLongIntVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(wchar_t val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildWcharVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(float val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildFloatVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(double val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildDoubleVal(val))); }

ConcreteValueKindPtr ConcreteExactKind::createConcreteValueKindFromVal(long double val)
{ return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongDoubleVal(val))); }


// Applies the given operation functor to the expression in this ExactKind and returns the resulting ConcreteKind
template<class DoOpType>
	ConcreteValueKindPtr ConcreteExactKind::doUnaryOp(DoOpType& doOp) {
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

	// Applies the given operation functor to the expression in this ExactKind, whoch is assumed to be an integral type
	// and returns the resulting ConcreteKind
	template<class DoOpType>
	ConcreteValueKindPtr ConcreteExactKind::doUnaryIntegralOp(DoOpType& doOp) {
	  switch(exp.get()->variantT()) {
	/*    case V_SgBoolValExp:             return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp            (doOp(isSgBoolValExp            (exp.get())->get_value()))));
	    case V_SgCharVal:                return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildCharVal               (doOp(isSgCharVal               (exp.get())->get_value()))));
	    case V_SgIntVal:                 return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal                (doOp(isSgIntVal                (exp.get())->get_value()))));
	    case V_SgLongIntVal:             return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongIntVal            (doOp(isSgLongIntVal            (exp.get())->get_value()))));
	    case V_SgLongLongIntVal:         return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal        (doOp(isSgLongLongIntVal        (exp.get())->get_value()))));
	    case V_SgUnsignedCharVal:        return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedCharVal       (doOp(isSgUnsignedCharVal       (exp.get())->get_value()))));
	    case V_SgUnsignedIntVal:         return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedIntVal        (doOp(isSgUnsignedIntVal        (exp.get())->get_value()))));
	    case V_SgUnsignedLongVal:        return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedLongVal       (doOp(isSgUnsignedLongVal       (exp.get())->get_value()))));
	    case V_SgUnsignedLongLongIntVal: return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedLongLongIntVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value()))));
	    case V_SgUnsignedShortVal:       return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildUnsignedShortVal      (doOp(isSgUnsignedShortVal      (exp.get())->get_value()))));
	    case V_SgUpcMythread:            return boost::make_shared<ConcreteUnknownKind>();
	    case V_SgUpcThreads:             return boost::make_shared<ConcreteUnknownKind>();
	    case V_SgWcharVal:               return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildWcharVal              (doOp(isSgWcharVal              (exp.get())->get_value()))));*/
	    case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgBoolValExp            (exp.get())->get_value()));
	    case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgCharVal               (exp.get())->get_value()));
	    case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgShortVal              (exp.get())->get_value()));
	    case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgIntVal                (exp.get())->get_value()));
	    case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgLongIntVal            (exp.get())->get_value()));
	    case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal        (exp.get())->get_value()));
	    case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal       (exp.get())->get_value()));
	    case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal        (exp.get())->get_value()));
	    case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal       (exp.get())->get_value()));
	    case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value()));
	    case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal      (exp.get())->get_value()));
	    case V_SgUpcMythread:            return boost::make_shared<ConcreteUnknownKind>();
	    case V_SgUpcThreads:             return boost::make_shared<ConcreteUnknownKind>();
	    case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgWcharVal              (exp.get())->get_value()));


	    case V_SgEnumVal:
	      //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnusmVal               (exp.get())->get_value()-1)));
	      // NOTE: Need to write code to get the value of the Enum Val and create a new one
	      assert(0);
	    default:
	      // We've enumerated all cases so we should never get here
	      assert(0);
	  }
	}

	// Applies the given operation functor to the expression in this ConcreteKind, which is assumed to a floating point type
	// and returns the resulting ConcreteKind
	template<class DoOpType>
	ConcreteValueKindPtr ConcreteExactKind::doUnaryFloatOp(DoOpType& doOp) {
	  switch(exp.get()->variantT()) {
	    case V_SgComplexVal:             return boost::make_shared<ConcreteUnknownKind>();
	    /*case V_SgDoubleVal:              return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildDoubleVal             (doOp(isSgDoubleVal             (exp.get())->get_value()))));
	    case V_SgFloatVal:               return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildFloatVal              (doOp(isSgFloatVal              (exp.get())->get_value()))));
	    case V_SgLongDoubleVal:          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongDoubleVal         (doOp(isSgLongDoubleVal         (exp.get())->get_value()))));*/
	    case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgDoubleVal             (exp.get())->get_value()));
	    case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgFloatVal              (exp.get())->get_value()));
	    case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal         (exp.get())->get_value()));

	    default:
	      // We've enumerated all cases so we should never get here
	      assert(0);
	  }
}

// Applies the given unary or binary operation to this and the given ConcreteValueKind
// Returns:
//    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated ConcreteValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent ConcreteValueObject's list, return that.
ConcreteValueKindPtr ConcreteExactKind::op(SgUnaryOp* op) {
  if(isSgBitComplementOp(op)) {
    if(isSgBoolValExp(exp.get()))
      switch(exp.get()->variantT()) {
      case V_SgBoolValExp: case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
      case V_SgFloatVal: case V_SgDoubleVal: case V_SgLongDoubleVal:
        return boost::make_shared<ConcreteUnknownKind>();
      default: return doUnaryIntegralOp(~ boost::lambda::_1);
    }
  } else if(isSgCastExp(op)) {
    SgType* t=isSgCastExp(op)->get_type();
    // Unwrap any typedefs that alias the base type of t
    while(SgTypedefType* tt=isSgTypedefType(t))
      t = tt->get_base_type();

    switch(t->variantT()) {
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
        cerr << "ERROR: unknown cast expression type. unwrapped type="<<SgNode2Str(t)<<" full type="<<SgNode2Str(isSgCastExp(op)->get_type())<<", op="<<SgNode2Str(op)<<endl;
        assert(0);
    }
  } else if(isSgConjugateOp(op)) {
    // TO DO
    /*if(isSgComplexVal(exp.get()))
      return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildComplexVal(
               isSgComplexVal(exp.get())->get_real_value(),
              -isSgComplexVal(exp.get())->get_imaginary_value())));
    else
      return boost::make_shared<ConcreteUnknownKind>();*/
  } else if(isSgExpressionRoot(op)) {
    // What is this?
  } else if(isSgImagPartOp(op)) {
    if(isSgComplexVal(exp.get()))
      return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(isSgComplexVal(exp.get())->get_imaginary_value()));
    else
      return boost::make_shared<ConcreteUnknownKind>();
  } else if(isSgRealPartOp(op)) {
    if(isSgComplexVal(exp.get()))
      return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(isSgComplexVal(exp.get())->get_real_value()));
    else
      return boost::make_shared<ConcreteUnknownKind>();
  } else if(isSgMinusMinusOp(op)) {
    switch(exp.get()->variantT()) {
      case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads: return boost::make_shared<ConcreteUnknownKind>();
      default: return doUnaryOp(boost::lambda::_1 - 1);
    }
  } else if(isSgPlusPlusOp(op)) {
    switch(exp.get()->variantT()) {
      case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads: return boost::make_shared<ConcreteUnknownKind>();
      default: return doUnaryOp(boost::lambda::_1 + 1);
    }
  } else if(isSgMinusOp(op)) {
    switch(exp.get()->variantT()) {
      case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads: return boost::make_shared<ConcreteUnknownKind>();
      default: return doUnaryOp(0 - boost::lambda::_1);
    }
  } else if(isSgNotOp(op)) {
    switch(exp.get()->variantT()) {
      case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads: return boost::make_shared<ConcreteUnknownKind>();
      default: return doUnaryOp(!boost::lambda::_1);
    }

  } else if(isSgThrowOp(op)) {
    return boost::make_shared<ConcreteUnknownKind>();
    // TODO: control flow effects

  } else if(isSgUnaryAddOp(op)) {
    // What is this?

  } else if(isSgUserDefinedUnaryOp(op)) {
    // What is this?

  } else if(isSgAddressOfOp(op)) {
    // TODO
    return boost::make_shared<ConcreteUnknownKind>();


  } else if(isSgPointerDerefExp(op)) {
    // This should be handled inside ConcreteMemLocObjects
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
DoOpRetType ConcreteExactKind::bindDoOpArgs1(DoOpType doOp) {
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
DoOpRetType ConcreteExactKind::bindDoOpArgs2(DoOpType doOp) {
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

//! Calculate the number of dimensions of an array type
size_t getArrayDimCount(SgArrayType* t, SgPntrArrRefExp* ref)
{
  ROSE_ASSERT(t);
  SgExpression * indexExp =  t->get_index();

//strip off THREADS for UPC array with a dimension like dim*THREADS
  if (isUpcArrayWithThreads(t))
  {
    SgMultiplyOp* multiply = isSgMultiplyOp(indexExp);
    ROSE_ASSERT(multiply);
    indexExp = multiply->get_lhs_operand();
  }

  /*cout << "t="<<SgNode2Str(t)<<endl;
  cout << "indexExp="<<(indexExp? SgNode2Str(indexExp): "NULL")<<endl;*/

  // If the dimension is not specified in the type, (e.g. array[])
  if (indexExp == NULL || isSgNullExpression(indexExp)) {
    // Look at the array's declaration to see if its size was specified via an SgAggregateInitializer
    if(SgVarRefExp* vref=isSgVarRefExp(ref->get_lhs_operand())) {
      if(SgAggregateInitializer* init = isSgAggregateInitializer(vref->get_symbol()->get_declaration()->get_initializer())) {
        return init->get_initializers()->get_expressions().size();
      } else {
        cerr << "ERROR: cannot count the number of elements in array "<<SgNode2Str(ref->get_lhs_operand())<<" since SgAggregateInitializer could not be found!"<<endl;
        assert(0);
      }
    } else {
      cerr << "ERROR: cannot count the number of elements in array reference "<<SgNode2Str(ref->get_lhs_operand())<<" since it is not a SgVarRefExp!"<<endl;
      assert(0);
    }
  } else {
    //Take advantage of the fact that the value expression is always SgUnsignedLongVal in AST
    SgUnsignedLongVal * valExp = isSgUnsignedLongVal(indexExp);
    SgIntVal * valExpInt = isSgIntVal(indexExp);
    ROSE_ASSERT(valExp || valExpInt); // TODO: return -1 is better ?
    if (valExp)
      return valExp->get_value();
    else
      return valExpInt->get_value();
  }

  assert(0);
}

//! Calculate the number of elements of an array type
size_t getArrayElementCount_GB(SgArrayType* t, SgPntrArrRefExp* ref)
{
  ROSE_ASSERT(t);

  size_t result = getArrayDimCount(t, ref);

  // consider multi dimensional case
  SgArrayType* arraybase = isSgArrayType(t->get_base_type());
  if (arraybase)
    result = result * getArrayElementCount_GB(arraybase, isSgPntrArrRefExp(ref->get_rhs_operand()));

  return result;
} // getArrayElementCount()


// Returns the offset of the given SgPntrArrRefExp relative to the starting point of its parent expression,
// which may be a SgVarRefExp, SgDotExp, SgPntrArrRefExp or other expressions
long long getPntrArrRefOffset(SgPntrArrRefExp* ref, ConcreteExactKindPtr that) {
  //scope s(txt()<<"getPntrArrRefOffset("<<SgNode2Str(ref));
  /*cout << "----------------------------------"<<endl;
  cout << "getPntrArrRefOffset("<<SgNode2Str(ref)<<endl;
  cout << "rhs="<<SgNode2Str(ref->get_rhs_operand())<<endl;
  cout << "lhs="<<SgNode2Str(ref->get_lhs_operand())<<endl;
  if(isSgVarRefExp(ref->get_lhs_operand())) {
    cout << "decl="<<SgNode2Str(isSgVarRefExp(ref->get_lhs_operand())->get_symbol()->get_declaration())<<endl;
    cout << "initializer="<<SgNode2Str(isSgVarRefExp(ref->get_lhs_operand())->get_symbol()->get_declaration()->get_initializer())<<endl;
  }
  cout << "lhs type="<<SgNode2Str(ref->get_lhs_operand()->get_type())<<endl;*/

  unsigned long long subArraySize;
  if(SgArrayType* arrType = isSgArrayType(ref->get_lhs_operand()->get_type())) {
    /*cout << "   index="<<SgNode2Str(arrType->get_index())<<endl;
    cout << "   dim="<<SgNode2Str(arrType->get_dim_info())<<"="<<arrType->get_dim_info()<<endl;
    cout << "   rank="<<arrType->get_rank()<<endl;
    cout << "   eltCount="<<getArrayElementCount_GB(arrType, ref)<<endl;
    cout << " that="<<that->str()<<endl;*/

    // Compute the number of entries in the array of the current sub-level in the SgArrayType by
    // dividing the number of total entries in the current SgArrayType by the number of sub-arrays
    // in the next dimension.
    /*assert(isSgValueExp(arrType->get_index()));
    long long sTypeIdx;
    unsigned long long usTypeIdx;
    if(IsSignedConstInt(isSgValueExp(arrType->get_index()), sTypeIdx))
      subArraySize = getArrayElementCount_GB(arrType, ref) / sTypeIdx;
    else if(IsUnsignedConstInt(isSgValueExp(arrType->get_index()), usTypeIdx))
      subArraySize = getArrayElementCount_GB(arrType, ref) / usTypeIdx;
    else
      // The index in the array's type must be an integer of some sort
      assert(0);*/
    subArraySize = getArrayElementCount_GB(arrType, ref) / getArrayDimCount(arrType, ref);
  } else if(/*SgPointerType* paType = */isSgPointerType(ref->get_lhs_operand()->get_type())) {
    subArraySize = 1;
  } else
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
// both integral types and returns the resulting ConcreteKind
template<class DoOpType>
ConcreteValueKindPtr ConcreteExactKind::doBinaryOpIntInt(DoOpType& doOp, ConcreteExactKindPtr that) {
  long long sVal1, sVal2;
  unsigned long long usVal1, usVal2;
  if(IsSignedConstInt(exp.get(), sVal1)) {
    if(IsSignedConstInt(that->getVal().get(), sVal2))
      return createConcreteValueKindFromVal(doOp(sVal1, sVal2));
    else if(IsUnsignedConstInt(that->getVal().get(), usVal2))
      return createConcreteValueKindFromVal(doOp(sVal1, (long long)usVal2));
  } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
    if(IsSignedConstInt(that->getVal().get(), sVal2))
      return createConcreteValueKindFromVal(doOp((long long)usVal1, sVal2));
    else if(IsUnsignedConstInt(that->getVal().get(), usVal2))
      return createConcreteValueKindFromVal(doOp(usVal1, usVal2));
  }
  assert(0);
  /*switch(exp.get()->variantT()) {
    case V_SgBoolValExp:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgCharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgShortVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedCharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedShortVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedLongVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedLongLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgWcharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUpcMythread: case V_SgUpcThreads:
      return boost::make_shared<ConcreteUnknownKind>();

    case V_SgEnumVal:
      //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
      // NOTE: Need to write code to get the value of the Enum Val and create a new one
      assert(0);
    default:
      // We've enumerated all cases so we should never get here
      assert(0);
  }  */
}

// Applies the given binary operation functor to the expression in this and that ConcreteKinds, assuming at least one
// is not an integral type and returns the resulting ConcreteKind
template<class DoOpType>
ConcreteValueKindPtr ConcreteExactKind::doBinaryOp(DoOpType& doOp, ConcreteExactKindPtr that) {
  long long sVal1, sVal2;
  unsigned long long usVal1, usVal2;

  if(IsSignedConstInt(exp.get(), sVal1)) {
    if(IsSignedConstInt(that->getVal().get(), sVal2))
      return createConcreteValueKindFromVal(doOp(sVal1, sVal2));
    else if(IsUnsignedConstInt(that->getVal().get(), usVal2))
      return createConcreteValueKindFromVal(doOp(sVal1, (long long)usVal2));
  } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
    if(IsSignedConstInt(that->getVal().get(), sVal2))
      return createConcreteValueKindFromVal(doOp((long long)usVal1, sVal2));
    else if(IsUnsignedConstInt(that->getVal().get(), usVal2))
      return createConcreteValueKindFromVal(doOp(usVal1, usVal2));
  }
  // If the values are not integers, lets see if they're floating point
  double fVal1, fVal2;
  if(IsConstFloat(exp.get(), fVal1) && IsConstFloat(that->getVal().get(), fVal2))
    return createConcreteValueKindFromVal(doOp(fVal1,fVal2));

  assert(0);
  /*
  switch(exp.get()->variantT()) {
    case V_SgBoolValExp:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgBoolValExp(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgCharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgCharVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgShortVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgShortVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgLongIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgLongLongIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedCharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgUnsignedCharVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedShortVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgUnsignedShortVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgUnsignedIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedLongVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgUnsignedLongVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgUnsignedLongLongIntVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgUnsignedLongLongIntVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgWcharVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgWcharVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgFloatVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgFloatVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgDoubleVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgDoubleVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgLongDoubleVal:
      switch(that->exp.get()->variantT()) {
        case V_SgBoolValExp:             return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgBoolValExp            (that->exp.get())->get_value()));
        case V_SgCharVal:                return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgCharVal               (that->exp.get())->get_value()));
        case V_SgShortVal:               return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgShortVal              (that->exp.get())->get_value()));
        case V_SgIntVal:                 return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgIntVal                (that->exp.get())->get_value()));
        case V_SgLongIntVal:             return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgLongIntVal            (that->exp.get())->get_value()));
        case V_SgLongLongIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgLongLongIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedCharVal:        return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedCharVal       (that->exp.get())->get_value()));
        case V_SgUnsignedShortVal:       return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedShortVal      (that->exp.get())->get_value()));
        case V_SgUnsignedIntVal:         return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedIntVal        (that->exp.get())->get_value()));
        case V_SgUnsignedLongVal:        return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedLongVal       (that->exp.get())->get_value()));
        case V_SgUnsignedLongLongIntVal: return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgUnsignedLongLongIntVal(that->exp.get())->get_value()));
        case V_SgWcharVal:               return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgWcharVal              (that->exp.get())->get_value()));
        case V_SgFloatVal:               return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgFloatVal              (that->exp.get())->get_value()));
        case V_SgDoubleVal:              return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgDoubleVal             (that->exp.get())->get_value()));
        case V_SgLongDoubleVal:          return createConcreteValueKindFromVal(doOp(isSgLongDoubleVal(exp.get())->get_value(), isSgLongDoubleVal         (that->exp.get())->get_value()));
        case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
          return boost::make_shared<ConcreteUnknownKind>();
        case V_SgEnumVal:
          //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
          // NOTE: Need to write code to get the value of the Enum Val and create a new one
          assert(0);
        default:
          // We've enumerated all cases so we should never get here
          assert(0);
      }

    case V_SgComplexVal: case V_SgUpcMythread: case V_SgUpcThreads:
      return boost::make_shared<ConcreteUnknownKind>();

    case V_SgEnumVal:
      //return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildEnumVal               (isSgEnumVal               (exp.get())->get_value()-1)));
      // NOTE: Need to write code to get the value of the Enum Val and create a new one
      assert(0);
    default:
      // We've enumerated all cases so we should never get here
      assert(0);
  }  */
}


// Applies the given unary or binary operation to this and the given ConcreteValueKind
// Returns:
//    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated ConcreteValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent ConcreteValueObject's list, return that.
ConcreteValueKindPtr ConcreteExactKind::op(SgBinaryOp* op, ConcreteValueKindPtr that) {
  //struct timeval tfStart, tfEnd; gettimeofday(&tfStart, NULL);

  // Special cases for short-circuiting operations since their operand may not be
  // available if the lhs resolves the outcome of the condition and some smart amalysis
  // eliminated the portion of the ATS that computes the irrelevant side of the condition.
  // False && * ==> False
  if(isSgAndOp(op) && isConstantFalse(getVal().get())) {
    //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op1\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
    return copyAOType();
  }
  // True || * ==> True
  if(isSgOrOp(op) && isConstantTrue(getVal().get())) {
    //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op2\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
    return copyAOType();
  }

  // For all other cases we must have a valid rhs operand
  assert(that);

  /*scope("ConcreteExactKind::op(SgBinaryOp*)", scope::medium, attrGE("ConcreteDebugLevel", 2));
  if(ConcreteDebugLevel()>=2) {
    dbg << "this="<<str()<<endl;
    dbg << "that="<<that->str()<<endl;
    dbg << "(that->getKind() == ConcreteValueKind::uninitialized)="<<(that->getKind() == ConcreteValueKind::uninitialized)<<endl;
    dbg << "(that->getKind() == ConcreteValueKind::unknown)="<<(that->getKind() == ConcreteValueKind::unknown)<<endl;
  }*/

  if(that->getKind() == ConcreteValueKind::uninitialized) {
    if(isSgAndOp(op)) {
      // True && uninitialized => uninitialized
      if(isConstantTrue(getVal().get())) {
        //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op3\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
        return that;
      }
      // False && uninitialized => False
      else {
        //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op4\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
        return boost::make_shared<ConcreteUninitializedKind>();
      }

    } else if(isSgOrOp(op)) {
      // True || uninitialized => True
      if(isConstantTrue(getVal().get())) {
        //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op5\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
        return copyAOType();
      }
      // False | uninitialized => uninitialized
      else {
        //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op6\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
        return boost::make_shared<ConcreteUninitializedKind>();
      }

    // * op uninitialized => *
    } else {
      //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op6A\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
      return copyAOType();
    }
  }

  // * op unknown => unknown
  if(that->getKind() == ConcreteValueKind::unknown) {
    //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op6B\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
    return boost::make_shared<ConcreteUnknownKind>();
  }

  if(that->getKind() == ConcreteValueKind::exact) {
    ConcreteExactKindPtr thatConcrete = that->asExactKind();

    // ----- Arithmetic -----
    if(isSgAddOp(op) || isSgPlusAssignOp(op)) {
      long long sVal1, sVal2;
      unsigned long long usVal1, usVal2;
      if(IsSignedConstInt(exp.get(), sVal1)) {
        if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(sVal1+sVal2)));
        else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(sVal1+usVal2)));
      } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
        if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(usVal1+sVal2)));
        else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(usVal1+usVal2)));
      }
      // If the values are not integers, lets see if they're floating point
      double fVal1, fVal2;
      if(IsConstFloat(exp.get(), fVal1) && IsConstFloat(thatConcrete->getVal().get(), fVal2))
        return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildDoubleVal(fVal1+fVal2)));
      // If not floating point, give up
      return boost::make_shared<ConcreteUnknownKind>();

    } else if(isSgSubtractOp(op) || isSgMinusAssignOp(op)) {
      //return createConcreteValueKindFromVal(thatConcrete->bindDoOpArgs2(bindDoOpArgs1(boost::lambda::_1 - boost::lambda::_2)));
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
      ConcreteValueKindPtr ret = doBinaryOp(boost::lambda::_1 && boost::lambda::_2, thatConcrete);
      //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op7\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
      return ret;
    } else if(isSgOrOp(op) || isSgIorAssignOp(op)) {
      ConcreteValueKindPtr ret = doBinaryOp(boost::lambda::_1 || boost::lambda::_2, thatConcrete);
      //gettimeofday(&tfEnd, NULL); cout << "ConcreteExactKind::op8\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<"\t"<<SgNode2Str(op)<<endl;
      return ret;
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
          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(sVal1==sVal2)));
        else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(sVal1==(long long)usVal2)));
      } else if(IsUnsignedConstInt(exp.get(), usVal1)) {
        if(IsSignedConstInt(thatConcrete->getVal().get(), sVal2))
          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp((long long)usVal1==sVal2)));
        else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal2))
          return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(usVal1==usVal2)));
      }
      // If the values are not integers, lets see if they're floating point
      double fVal1, fVal2;
      if(IsConstFloat(exp.get(), fVal1) && IsConstFloat(thatConcrete->getVal().get(), fVal2))
        return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(fVal1==fVal2)));
      // If not floating point, give up
      return boost::make_shared<ConcreteUnknownKind>();
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
      // This should be handled inside ConcreteMemLocObjects
      assert(0);
    } else if(isSgArrowExp(op)) {
      // This should be handled inside ConcreteMemLocObjects
      assert(0);
    } else if(isSgPntrArrRefExp(op)) {
#ifndef DISABLE_SIGHT
      scope s("SgPntrArrRefExp");
#endif
      assert(thatConcrete);
      // Get the offset of the SgPntrArrRefExp relative to the starting point of its parent expression
      long long nextOffset = getPntrArrRefOffset(isSgPntrArrRefExp(op), thatConcrete);
      return doBinaryOp(boost::lambda::_1 + boost::lambda::_2,
                        boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildLongLongIntVal(nextOffset))));

    } else if(isSgDotStarOp(op)) {
      // This should be handled inside ConcreteMemLocObjects
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
  } else if(that->getKind() == ConcreteValueKind::offsetList) {
    ConcreteExactKindPtr thatOffset = that->asExactKind();

    // Implement the operations that are defined for pointers by having the offset list kind deal with them
    if(isSgAddOp(op) || isSgSubtractOp(op)) return thatOffset->op(op, shared_from_this());
    // Other options are undefined.
    // Technically this means that we can return an uninitialized kind but to be safe we'll return unknown
    else
      return boost::make_shared<ConcreteUnknownKind>();
  }

  // We've enumerated all cases so we should never get here
  cerr << "ERROR: no support for "<<SgNode2Str(op)<<"!";
  assert(0);
}

// Returns whether the two given SgValueExps denote the same numeric value.
// If unknown, return unknownVal.
bool ConcreteExactKind::equalVals(SgValueExp* val1, SgValueExp* val2, bool unknownVal) {
  long long sVal1, sVal2;
  unsigned long long usVal1, usVal2;

  // First check if both concrete values are integers of some type
  if(IsSignedConstInt(val1, sVal1)) {
    if(IsSignedConstInt(val2, sVal2))
      return sVal1==sVal2;
    else if(IsUnsignedConstInt(val2, usVal2))
      return sVal1==(long long)usVal2;
  } else if(IsUnsignedConstInt(val1, usVal1)) {
    if(IsSignedConstInt(val2, sVal2))
      return (long long)usVal1==sVal2;
    else if(IsUnsignedConstInt(val2, usVal2))
      return usVal1==usVal2;
  }
  // If the values are not integers, lets see if they're floating point
  double fVal1, fVal2;
  if(IsConstFloat(val1, fVal1) && IsConstFloat(val2, fVal2))
    return fVal1==fVal2;

  // Otherwise, return unknownVal since we don't handle the other cases
  return unknownVal;
}

// Returns whether the SgValueExps denoted by val1 < the value denoted by val2.
// If unknown, return unknownVal.
bool ConcreteExactKind::lessThanVals(SgValueExp* val1, SgValueExp* val2, bool unknownVal) {
  long long sVal1, sVal2;
  unsigned long long usVal1, usVal2;

  // First check if both concrete values are integers of some type
  if(IsSignedConstInt(val1, sVal1)) {
    if(IsSignedConstInt(val2, sVal2))
      return sVal1<sVal2;
    else if(IsUnsignedConstInt(val2, usVal2))
      return sVal1<(long long)usVal2;
  } else if(IsUnsignedConstInt(val1, usVal1)) {
    if(IsSignedConstInt(val2, sVal2))
      return (long long)usVal1<sVal2;
    else if(IsUnsignedConstInt(val2, usVal2))
      return usVal1<usVal2;
  }
  // If the values are not integers, lets see if they're floating point
  double fVal1, fVal2;
  if(IsConstFloat(val1, fVal1) && IsConstFloat(val2, fVal2))
    return fVal1<fVal2;

  // Otherwise, return unknownVal since we don't handle the other cases
  return unknownVal;
}

// Returns whether this and that ConcreteValueKinds are may/must equal to each other
bool ConcreteExactKind::mayEqualAO(ConcreteValueKindPtr that)
{
  // If that is not concrete, use its implementation
  if(that->getKind() != ConcreteValueKind::exact) return that->mayEqualAO(shared_from_this());
  ConcreteExactKindPtr thatConcrete = that->asExactKind();

  return equalVals(exp.get(), thatConcrete->getVal().get(),
                   /*unknownVal, default to mayEquals*/ true);
}

bool ConcreteExactKind::mustEqualAO(ConcreteValueKindPtr that) {
  // If that is not concrete, use its implementation
  if(that->getKind() != ConcreteValueKind::exact) return that->mustEqualAO(shared_from_this());
  ConcreteExactKindPtr thatConcrete = that->asExactKind();

  return equalVals(exp.get(), thatConcrete->getVal().get(),
                   /*unknownVal, default to mustEquals*/ false);
}

// Returns whether the two ConcreteValueKinds denote the same set of concrete values
bool ConcreteExactKind::equalSetAO(ConcreteValueKindPtr that) {
  // The logic here is the same as mustEquals
  return mustEqualAO(that);
}

// Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given ConcreteValueKind
bool ConcreteExactKind::subSetAO(ConcreteValueKindPtr that) {
  // The logic here is the same as mustEquals
  return mustEqualAO(that);
}

// Computes the meet of this and that and returns the resulting kind
pair<bool, ConcreteValueKindPtr> ConcreteExactKind::meetUpdateAO(ConcreteValueKindPtr that)
{
  //scope s("ConcreteExactKind::meetUpdateV");
  // Concrete MEET Uninitialized => Concrete
  if(that->getKind() == ConcreteValueKind::uninitialized) {
    //dbg << "that=>uninitialized"<<endl;
    return make_pair(true, copyAOType());
  }

  // Concrete MEET Unknown => Unknown
  // Concrete MEET Offset  => Unknown
  if(that->getKind() == ConcreteValueKind::offsetList ||
     that->getKind() == ConcreteValueKind::unknown) {
    //dbg << "that=>offsetlist or unknown"<<endl;
    return make_pair(true, boost::make_shared<ConcreteUnknownKind>());
  }

  // That is definitely concrete

  // If this and that denote the same concrete value
  if(mustEqualAO(that)) {
    //dbg << "must equal, not modified"<<endl;
    return make_pair(false, copyAOType());
  }
  // If the concrete values differ
  else {
    //dbg << "different, modified"<<endl;
    return make_pair(true, boost::make_shared<ConcreteUnknownKind>());
  }
}

// Computes the intersection of this and that and returns the resulting kind
pair<bool, ConcreteValueKindPtr> ConcreteExactKind::intersectUpdateAO(ConcreteValueKindPtr that)
{
  /*scope s("ConcreteExactKind::intersectUpdateV");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that->str()<<endl;*/
  // Concrete Intersection Unknown => Concrete
  if(that->getKind() == ConcreteValueKind::unknown) {
    //dbg << "that=>uninitialized"<<endl;
    return make_pair(true, copyAOType());
  }

  // Concrete INTERSECT Uninitialized => Uninitialized
  // Concrete INTERSECT Offset  => Uninitialized
  if(that->getKind() == ConcreteValueKind::offsetList ||
     that->getKind() == ConcreteValueKind::uninitialized) {
    //dbg << "that=>offsetlist or unknown"<<endl;
    return make_pair(true, boost::make_shared<ConcreteUninitializedKind>());
  }

  // That is definitely concrete

  // If this and that denote the same concrete value
  if(mustEqualAO(that)) {
    //dbg << "must equal, not modified"<<endl;
    return make_pair(false, copyAOType());
  }
  // If the concrete values differ
  else {
    //dbg << "different, modified"<<endl;
    return make_pair(true, boost::make_shared<ConcreteUninitializedKind>());
  }
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool ConcreteExactKind::isConcrete()
{ return true; }

// Returns the number of concrete values in this set
int ConcreteExactKind::concreteSetSize()
{ return 1; }

// Returns the type of the concrete value (if there is one)
SgType* ConcreteExactKind::getConcreteType()
{ return exp->get_type(); }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > ConcreteExactKind::getConcreteValue() {
  std::set<boost::shared_ptr<SgValueExp> > vals;
  vals.insert(exp);
  return vals;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool ConcreteExactKind::isFullAO(PartEdgePtr pedge) { return false; }
// Returns whether this AbstractObject denotes the empty set.
bool ConcreteExactKind::isEmptyAO(PartEdgePtr pedge) { return false; }

std::string ConcreteExactKind::str(std::string indent) const
{ return txt()<<"[ConcreteExactKind: val="<<(exp? SgNode2Str(exp.get()): "NULL")<<"]"; }


/**********************************
 ***** ConcreteOffsetListKind *****
 **********************************/

SgTypeLongLong *ConcreteOffsetListKind::type = NULL;

// Applies the given unary or binary operation to this and the given ConcreteValueKind
// Returns:
//    - if this ConcreteValueKind can be updated to incorporate the result of the addition,
//       return a freshly-allocated ConcreteValueKind that holds the result.
//    - if the two objects could not be merged and therefore that must be placed after
//       this in the parent ConcreteValueObject's list, return that.
ConcreteValueKindPtr ConcreteOffsetListKind::op(SgUnaryOp* op) {
  // The only operations defiend on offset kinds are binary + and -
  return boost::make_shared<ConcreteUnknownKind>();
}

ConcreteValueKindPtr ConcreteOffsetListKind::op(SgBinaryOp* op, ConcreteValueKindPtr that) {
  // * op uninitialized => *
  if(that && that->getKind() == ConcreteValueKind::uninitialized)
    return copyAOType();

  // * op unknown => unknown
  if(that && that->getKind() == ConcreteValueKind::unknown)
    return boost::make_shared<ConcreteUnknownKind>();

  // If that is a concrete value or that was not provided because it is not needed (e.g. dot expression)
  if(!that || that->getKind() == ConcreteValueKind::exact) {
    ConcreteExactKindPtr thatConcrete; if(that) thatConcrete = that->asExactKind();
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
            return boost::make_shared<ConcreteUnknownKind>();

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
            return boost::make_shared<ConcreteUnknownKind>();
        }

        return boost::make_shared<ConcreteOffsetListKind>(newOffsetL);
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
            return boost::make_shared<ConcreteUnknownKind>();

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
            return boost::make_shared<ConcreteUnknownKind>();
        }

        return boost::make_shared<ConcreteOffsetListKind>(newOffsetL);
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
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal==offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgNotEqualOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal!=offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal!=offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgLessOrEqualOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal<=offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal<=offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgLessThanOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal<offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal<offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgGreaterOrEqualOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal>=offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal>=offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            case V_SgGreaterThanOp:
              if(IsSignedConstInt(thatConcrete->getVal().get(), sVal)) {
                if(sVal>offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), usVal)) {
                if((long long)usVal>offsetL.back().get())
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true)));
                else
                  return boost::make_shared<ConcreteExactKind>(boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false)));
              }
              break;

            default:
              break;
          }
        }
        // If we don't have enough information to compare the two values
        return boost::make_shared<ConcreteUnknownKind>();
      }

      // ----- Memory References -----
      case V_SgDotExp:
      {
        SIGHT_VERB_DECL(scope, ("V_SgDotExp", scope::medium), 1, ConcreteDebugLevel)
        // Find the index of the DotExp's RHS within the class of its LHS
        SgClassType *type;
        if((type = isSgClassType(isSgDotExp(op)->get_lhs_operand()->get_type()))==NULL) {
          if(isSgReferenceType(isSgDotExp(op)->get_lhs_operand()->get_type()))
            type = isSgClassType(isSgReferenceType(isSgDotExp(op)->get_lhs_operand()->get_type())->get_base_type());
        }
/*        cout << "op="<<SgNode2Str(op)<<endl;
        cout << "isSgDotExp(op)->get_lhs_operand()="<<SgNode2Str(isSgDotExp(op)->get_lhs_operand())<<endl;
        cout << "type="<<SgNode2Str(isSgDotExp(op)->get_lhs_operand()->get_type())<<endl;
        if(isSgReferenceType(isSgDotExp(op)->get_lhs_operand()->get_type())) {
          cout << "isSgReferenceType(type)->get_base_type()="<<SgNode2Str(isSgReferenceType(isSgDotExp(op)->get_lhs_operand()->get_type())->get_base_type())<<endl;
        }*/
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
        SgInitializedName* rhsVarDecl=NULL;
        SgMemberFunctionDeclaration* rhsFuncDecl=NULL;
        if(isSgVarRefExp(isSgDotExp(op)->get_rhs_operand()))
          rhsVarDecl = isSgVarRefExp(isSgDotExp(op)->get_rhs_operand())->get_symbol()->get_declaration();
        else if(isSgMemberFunctionRefExp(isSgDotExp(op)->get_rhs_operand()))
          rhsFuncDecl = isSgMemberFunctionDeclaration(isSgMemberFunctionRefExp(isSgDotExp(op)->get_rhs_operand())->get_symbol()->get_declaration()->get_definingDeclaration());
        //dbg << "rhsDecl="<<SgNode2Str(rhsDecl)<<endl;
        assert(rhsVarDecl || rhsFuncDecl);

        for(SgDeclarationStatementPtrList::const_iterator m=members.begin(); m!=members.end(); m++) {
          //scope s2(txt()<<memberIdx<<":    member ="<<SgNode2Str(*m));
          if(isSgVariableDeclaration(*m)) {
            const SgInitializedNamePtrList& decls = isSgVariableDeclaration(*m)->get_variables();
            for(SgInitializedNamePtrList::const_iterator d=decls.begin(); d!=decls.end(); d++) {
              //dbg << "        decl "<<memberIdx<<"="<<SgNode2Str(*d)<<" type="<<SgNode2Str((*d)->get_type())<<endl;
              if(*d == rhsVarDecl) {
                //dbg << "    memberIdx="<<memberIdx<<endl;

                // Return a new offset kind that extends the current one by appending a memberIdx rank
                list<intWrap> newOffsetL = offsetL;
                // Remove the trailing element in newOffsetL if it is a concrete constant 0 since such
                // a concrete offset has no meaning
                if(newOffsetL.back().getType()==intWrap::offsetT && newOffsetL.back().get()==0) newOffsetL.pop_back();
                newOffsetL.push_back(rank(memberIdx));
                return boost::make_shared<ConcreteOffsetListKind>(newOffsetL);
              }
              memberIdx++;
            }
          } else if(isSgMemberFunctionDeclaration(*m)) {
            if(rhsFuncDecl == isSgMemberFunctionDeclaration(*m)->get_definingDeclaration()) {
              //dbg << "    memberIdx="<<memberIdx<<endl;

              // Return a new offset kind that extends the current one by appending a memberIdx rank
              list<intWrap> newOffsetL = offsetL;
              // Remove the trailing element in newOffsetL if it is a concrete constant 0 since such
              // a concrete offset has no meaning
              if(newOffsetL.back().getType()==intWrap::offsetT && newOffsetL.back().get()==0) newOffsetL.pop_back();
              newOffsetL.push_back(rank(memberIdx));
              return boost::make_shared<ConcreteOffsetListKind>(newOffsetL);
            }
            memberIdx++;
          }

          /*else if(isSgClassDeclaration(*m)) {
            memberIdx += getNumClassMembers(isSgClassDeclaration(isSgClassDeclaration(*m)->get_definingDeclaration())->get_definition());
          }*/
        }
        assert(0); // There must be at least one match
      }

      case V_SgArrowExp:
        // Arrow expressions introduce aliasing, so we don't know their referent
        return boost::make_shared<ConcreteUnknownKind>();

      case V_SgPntrArrRefExp:
      {
        SIGHT_VERB_DECL(scope, ("V_SgPntrArrRefExp", scope::medium), 1, ConcreteDebugLevel)
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

        return boost::make_shared<ConcreteOffsetListKind>(newOffsetL);
      }

      case V_SgDotStarOp:
        // TODO
        return boost::make_shared<ConcreteUnknownKind>();

      case V_SgPointerAssignOp:
        // TODO (Fortran)
        return boost::make_shared<ConcreteUnknownKind>();

      // ----- Miscelaneous ----
      case V_SgAssignOp:
        // This should be handled inside the ConstantPropagationAnalysis transfer function
        assert(0);

      case V_SgCommaOpExp:
        // This should be handled inside the control flow functionality
        assert(0);

      default:
        return boost::make_shared<ConcreteUnknownKind>();
    }
  } else if(that->getKind() == ConcreteValueKind::offsetList) {
    // This is not yet implemented so this assert will notify us when we need to do the heavy lifting
    assert(0);
  }

  // We've enumerated all cases so we should never get here
  cerr << "ERROR: no support for "<<SgNode2Str(op)<<"!";
  assert(0);
}

// Returns whether this and that ConcreteValueKinds are may/must equal to each other
bool ConcreteOffsetListKind::mayEqualAO(ConcreteValueKindPtr that)
{
  // If that unknown or uninitialized, use its implementation
  if(that->getKind() == ConcreteValueKind::uninitialized || that->getKind() == ConcreteValueKind::unknown)
    return that->mayEqualAO(shared_from_this());

  if(that->getKind() == ConcreteValueKind::exact) {
    ConcreteExactKindPtr thatConcrete = that->asExactKind();

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

  if(that->getKind() == ConcreteValueKind::offsetList) {
    ConcreteOffsetListKindPtr thatOffset = that->asOffsetListKind();

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

bool ConcreteOffsetListKind::mustEqualAO(ConcreteValueKindPtr that) {
  // If that unknown or uninitialized, use its implementation
  if(that->getKind() == ConcreteValueKind::uninitialized || that->getKind() == ConcreteValueKind::unknown)
    return that->mayEqualAO(shared_from_this());

  if(that->getKind() == ConcreteValueKind::exact) {
    ConcreteExactKindPtr thatConcrete = that->asExactKind();

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

  if(that->getKind() == ConcreteValueKind::offsetList) {
    ConcreteOffsetListKindPtr thatOffset = that->asOffsetListKind();

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

// Returns whether the two ConcreteValueKinds denote the same set of concrete values
bool ConcreteOffsetListKind::equalSetAO(ConcreteValueKindPtr that) {
  // The logic here is the same as mustEquals
  return mustEqualAO(that);
}

// Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given ConcreteValueKind
bool ConcreteOffsetListKind::subSetAO(ConcreteValueKindPtr that) {
  // The logic here is the same as mustEquals
  return mustEqualAO(that);
}

// Computes the meet of this and that and returns the resulting kind
pair<bool, ConcreteValueKindPtr> ConcreteOffsetListKind::meetUpdateAO(ConcreteValueKindPtr that)
{
  // OffsetList MEET Uninitialized => OffsetList
  if(that->getKind() == ConcreteValueKind::uninitialized)
    return make_pair(true, copyAOType());

  // OffsetList MEET Unknown => Unknown
  if(that->getKind() == ConcreteValueKind::unknown)
    return make_pair(true, boost::make_shared<ConcreteUnknownKind>());

  // OffsetList MEET Concrete
  if(that->getKind() == ConcreteValueKind::exact) {
    ConcreteExactKindPtr thatConcrete = that->asExactKind();

    // If this offset is just a single concrete value
    if(offsetL.size()==1 && offsetL.back().getType()==ConcreteOffsetListKind::intWrap::offsetT) {
      // If both objects denote the same concrete value
      long long thatSV;
      unsigned long long thatUSV;
      if(IsSignedConstInt(thatConcrete->getVal().get(), thatSV)) {
        if(offsetL.back().get()==thatSV) return make_pair(false, copyAOType());
      } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), thatUSV)) {
        if(offsetL.back().get()==(long long)thatUSV) return make_pair(false, copyAOType());
      }
    }
    // If the two objects do not denote the same concrete value
    return make_pair(true, boost::make_shared<ConcreteUnknownKind>());
  }

  // OffsetList MEET OffsetList
  if(that->getKind() == ConcreteValueKind::offsetList) {
    ConcreteOffsetListKindPtr thatOffset = that->asOffsetListKind();
    // Compare the two offset lists directly
    // !!! (NOTE: this comparison doesn't take types into account when ranks are compared)
    if(offsetL.size() == thatOffset->offsetL.size()) {
      list<intWrap>::const_iterator thisI = offsetL.begin();
      list<intWrap>::const_iterator thatI = thatOffset->offsetL.begin();
      for(; thisI!=offsetL.end(); thisI++, thatI++) {
        // If the two offsetList objects are not identical
        if(*thisI != *thatI) return make_pair(true, boost::make_shared<ConcreteUnknownKind>());
      }
      // If we reached this point the two offset lists must be identical
      return make_pair(false, copyAOType());
    // The two objects have lists of different sizes, so they're not identical
    } else
      return make_pair(true, boost::make_shared<ConcreteUnknownKind>());
  }

  // We've covered all the cases
  assert(0);
}

// Computes the intersection of this and that and returns the resulting kind
pair<bool, ConcreteValueKindPtr> ConcreteOffsetListKind::intersectUpdateAO(ConcreteValueKindPtr that)
{
  // OffsetList INTERSECT Unknown => OffsetList
  if(that->getKind() == ConcreteValueKind::unknown)
    return make_pair(true, copyAOType());

  // OffsetList INTERSECT Uninitialized => Uninitialized
  if(that->getKind() == ConcreteValueKind::uninitialized)
    return make_pair(true, boost::make_shared<ConcreteUninitializedKind>());

  // OffsetList INTERSECT Concrete
  if(that->getKind() == ConcreteValueKind::exact) {
    ConcreteExactKindPtr thatConcrete = that->asExactKind();

    // If this offset is just a single concrete value
    if(offsetL.size()==1 && offsetL.back().getType()==ConcreteOffsetListKind::intWrap::offsetT) {
      // If both objects denote the same concrete value
      long long thatSV;
      unsigned long long thatUSV;
      if(IsSignedConstInt(thatConcrete->getVal().get(), thatSV)) {
        if(offsetL.back().get()==thatSV) return make_pair(false, copyAOType());
      } else if(IsUnsignedConstInt(thatConcrete->getVal().get(), thatUSV)) {
        if(offsetL.back().get()==(long long)thatUSV) return make_pair(false, copyAOType());
      }
    }
    // If the two objects do not denote the same concrete value, their intersection is empty
    return make_pair(true, boost::make_shared<ConcreteUninitializedKind>());
  }

  // OffsetList INTERSECT OffsetList
  if(that->getKind() == ConcreteValueKind::offsetList) {
    ConcreteOffsetListKindPtr thatOffset = that->asOffsetListKind();
    // Compare the two offset lists directly
    // !!! (NOTE: this comparison doesn't take types into account when ranks are compared)
    if(offsetL.size() == thatOffset->offsetL.size()) {
      list<intWrap>::const_iterator thisI = offsetL.begin();
      list<intWrap>::const_iterator thatI = thatOffset->offsetL.begin();
      for(; thisI!=offsetL.end(); thisI++, thatI++) {
        // If the two offsetList objects are not identical, their intersection is empty
        if(*thisI != *thatI) return make_pair(true, boost::make_shared<ConcreteUninitializedKind>());
      }
      // If we reached this point the two offset lists must be identical
      return make_pair(false, copyAOType());
    // The two objects have lists of different sizes, so they're not identical and their intersection is empty
    } else
      return make_pair(true, boost::make_shared<ConcreteUninitializedKind>());
  }

  // We've covered all the cases
  assert(0);
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool ConcreteOffsetListKind::isConcrete()
{ return offsetL.size()==1 && offsetL.back().getType()==ConcreteOffsetListKind::intWrap::offsetT; }

// Returns the number of concrete values in this set
int ConcreteOffsetListKind::concreteSetSize()
{ return (isConcrete()? 1: -1); }

// Returns the type of the concrete value (if there is one)
SgType* ConcreteOffsetListKind::getConcreteType()
// !!! NOTE: this may be a memory leak
{ return type; }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > ConcreteOffsetListKind::getConcreteValue() {
  assert(isConcrete());
  std::set<boost::shared_ptr<SgValueExp> > vals;
  vals.insert(boost::shared_ptr<SgLongLongIntVal>(SageBuilder::buildLongLongIntVal(offsetL.back().get())));
  return vals;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool ConcreteOffsetListKind::isFullAO(PartEdgePtr pedge) { return false; }
// Returns whether this AbstractObject denotes the empty set.
bool ConcreteOffsetListKind::isEmptyAO(PartEdgePtr pedge) { return false; }

std::string ConcreteOffsetListKind::str(std::string indent) const {
  ostringstream oss;

  oss <<"[ConcreteOffsetListKind: offsetL=";
  for(list<intWrap>::const_iterator o=offsetL.begin(); o!=offsetL.end(); o++) {
    if(o!=offsetL.begin()) oss << ",";
    oss << o->str();
  }
  oss << "]";

  return oss.str();
}

/*******************************
 ***** ConcreteUnknownKind *****
 *******************************/

// Applies the given unary or binary operation to this and the given ConcreteValueKind
// Returns:
//   - if this ConcreteValueKind can be updated to incorporate the result of the addition,
//      return a freshly-allocated ConcreteValueKind that holds the result.
//   - if the two objects could not be merged and therefore that must be placed after
//      this in the parent ConcreteValueObject's list, return that.
ConcreteValueKindPtr ConcreteUnknownKind::op(SgUnaryOp* op) {
  // Uninitialized denotes the full value set, so any operation applied to it results in the full set
  return copyAOType();
}

ConcreteValueKindPtr ConcreteUnknownKind::op(SgBinaryOp* op, ConcreteValueKindPtr that) {
  // Uninitialized denotes the full value set, so any operation that involves it results in the full set
  return copyAOType();
}

// Returns whether this and that ConcreteValueKinds are may/must equal to each other
bool ConcreteUnknownKind::mayEqualAO(ConcreteValueKindPtr that) {
  // Unknown denotes the full set, which overlaps with every other set
  return true;
}

bool ConcreteUnknownKind::mustEqualAO(ConcreteValueKindPtr that) {
  // Unknown denotes the full set, which has unbounded size and therefore is not must-equal to any set
  return false;
}

// Returns whether the two ConcreteValueKinds denote the same set of concrete values
bool ConcreteUnknownKind::equalSetAO(ConcreteValueKindPtr that) {
  // Unknown denotes the full set, which may only be equal to another full set
  return that->getKind() == ConcreteValueKind::unknown;
}

// Returns whether this ConcreteValueKind denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given ConcreteValueKind
bool ConcreteUnknownKind::subSetAO(ConcreteValueKindPtr that) {
  // Unknown  denotes the full set, which is a subset of another full set
  return that->getKind() == ConcreteValueKind::unknown;
}

// Computes the meet of this and that and returns the resulting kind
pair<bool, ConcreteValueKindPtr> ConcreteUnknownKind::meetUpdateAO(ConcreteValueKindPtr that)
{
  bool modified = false;
  // Unknown MEET * => Unknown
  return make_pair(modified, copyAOType());
}

// Computes the intersection of this and that and returns the resulting kind
std::pair<bool, ConcreteValueKindPtr> ConcreteUnknownKind::intersectUpdateAO(ConcreteValueKindPtr that) {
  bool modified = that->getKind() != ConcreteValueKind::unknown;
  // Unknown INTERSECT * => *
  return make_pair(modified, that->copyAOType());
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool ConcreteUnknownKind::isConcrete()
{ return false; }

// Returns the number of concrete values in this set
int ConcreteUnknownKind::concreteSetSize()
{ return -1; }

// Returns the type of the concrete value (if there is one)
SgType* ConcreteUnknownKind::getConcreteType()
{ return NULL; }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > ConcreteUnknownKind::getConcreteValue()
{ return std::set<boost::shared_ptr<SgValueExp> >(); }

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool ConcreteUnknownKind::isFullAO(PartEdgePtr pedge) { return true; }
// Returns whether this AbstractObject denotes the empty set.
bool ConcreteUnknownKind::isEmptyAO(PartEdgePtr pedge) { return false; }

std::string ConcreteUnknownKind::str(std::string indent) const
{ return "[ConcreteUnknownKind]"; }

/********************************
 ***** ConcreteValueObject ******
 ********************************/

ConcreteValueObjectPtr NULLConcreteValueObject;

ConcreteValueObject::ConcreteValueObject(ConcreteValueKindPtr kind, SgNode* base) : ValueObject(base), kind(kind)
{
}

bool ConcreteValueObject::mayEqualAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  ConcreteValueObjectPtr that = boost::dynamic_pointer_cast<ConcreteValueObject>(o);
  assert(that);
  return kind->mustEqualAO(that->kind);
}

bool ConcreteValueObject::mustEqualAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  ConcreteValueObjectPtr that = boost::dynamic_pointer_cast<ConcreteValueObject>(o);
  assert(that);
  return kind->mustEqualAO(that->kind);
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool ConcreteValueObject::equalSetAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  ConcreteValueObjectPtr that = boost::dynamic_pointer_cast<ConcreteValueObject>(o);
  assert(that);
  return kind->equalSetAO(that->kind);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool ConcreteValueObject::subSetAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  ConcreteValueObjectPtr that = boost::dynamic_pointer_cast<ConcreteValueObject>(o);
  assert(that);
  return kind->subSetAO(that->kind);
}

// Computes the meet of this and that and saves the result in this.
// Returns true if this causes this to change and false otherwise.
bool ConcreteValueObject::meetUpdateAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  ConcreteValueObjectPtr that = boost::dynamic_pointer_cast<ConcreteValueObject>(o);
  assert(that);

  pair<bool, ConcreteValueKindPtr> ret = kind->meetUpdateAO(that->kind);
  // Update kind
  kind = ret.second;

  // Return whether kind was modified
  return ret.first;
}

// Computes the intersection of this and that and saves the result in this.
// Returns true if this causes this to change and false otherwise.
bool ConcreteValueObject::intersectUpdateAO(ValueObjectPtr o, PartEdgePtr pedge)
{
  ConcreteValueObjectPtr that = boost::dynamic_pointer_cast<ConcreteValueObject>(o);
  assert(that);

  pair<bool, ConcreteValueKindPtr> ret = kind->intersectUpdateAO(that->kind);
  // Update kind
  kind = ret.second;

  // Return whether kind was modified
  return ret.first;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool ConcreteValueObject::isFullAO(PartEdgePtr pedge)
{ return kind->isFullAO(pedge); }

// Returns whether this lattice denotes the empty set.
bool ConcreteValueObject::isEmptyAO(PartEdgePtr pedge)
{ return kind->isEmptyAO(pedge); }


// Allocates a copy of this object and returns a pointer to it
ValueObjectPtr ConcreteValueObject::copyAOType() const
{
  return boost::make_shared<ConcreteValueObject>(*this);
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool ConcreteValueObject::isConcrete()
{ return kind->isConcrete(); }

// Returns the number of concrete values in this set
int ConcreteValueObject::concreteSetSize()
{ return kind->concreteSetSize(); }

// Returns the type of the concrete value (if there is one)
SgType* ConcreteValueObject::getConcreteType()
{
  assert(kind->asExactKind());
  return kind->asExactKind()->getVal()->get_type();
}

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
set<boost::shared_ptr<SgValueExp> > ConcreteValueObject::getConcreteValue()
{
  assert(kind->asExactKind());
  set<boost::shared_ptr<SgValueExp> > concreteVals;
  concreteVals.insert(kind->asExactKind()->getVal());
  return concreteVals;
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& ConcreteValueObject::getHierKey() const {
  if(!isHierKeyCached) {
    ((ConcreteValueObject*)this)->cachedHierKey = boost::make_shared<AOSHierKey>(((ConcreteValueObject*)this)->shared_from_this());

    // The all object gets an empty key since it contains all the object types
    if(kind->getKind()==ConcreteValueKind::unknown) { }
    else {
      ((ConcreteValueObject*)this)->cachedHierKey->add(boost::make_shared<ConcreteValueKind::comparableKind>(kind->getKind()));
      kind->addHierSubKey(((ConcreteValueObject*)this)->cachedHierKey);

      //dbg << "computed"<<endl;
    }
    ((ConcreteValueObject*)this)->isHierKeyCached = true;
  }
  //dbg << "((ConcreteValueObject*)this)->cachedHierKey="<<((ConcreteValueObject*)this)->cachedHierKey<<endl;
  return cachedHierKey;
}

string ConcreteValueObject::str(string indent) const
{
  return kind->str(indent);
}

string ConcreteValueObject::strp(PartEdgePtr pedge, string indent) const
{
  return kind->str(indent);
}

} // namespace fuse
