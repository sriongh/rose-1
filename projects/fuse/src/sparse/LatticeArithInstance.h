#pragma once

#ifndef LATTICEARITHINSTANCE_H_
#define LATTICEARITHINSTANCE_H_

#include "LatticeArith.h"
/*
#define APPLY_OPERATOR(OP, TYPE, TYPE_RES) \
        if (that_->is_ ## TYPE ## LatticeArith()) { \
            LatticeArithInstance<TYPE> * that = \
            dynamic_cast<LatticeArithInstance<TYPE>*>(that_); \
            TYPE_RES resArith = getValue() OP that->getValue(); \
            LatticeArith * res = new LatticeArithInstance<TYPE_RES>(resArith); \
            return res; \
        } \
*/
#define CHECK_LATTICE_ARITH_INSTANCE_TYPE(TYPE) (dynamic_cast<LatticeArithInstance<TYPE>*>(this) != NULL);

template<typename T>
class LatticeArithInstance : virtual public LatticeArith {

protected:
    T value;

public:
    LatticeArithInstance(PartEdgePtr pe, T value_) : Lattice(pe), FiniteLattice(pe), 
      LatticeArith(pe), value(value_) {};

    LatticeArithInstance(PartEdgePtr pe, bool top_, bool bottom_) :  Lattice(pe), FiniteLattice(pe), 
      LatticeArith(pe, top_, bottom_) {};
    
    virtual ~LatticeArithInstance() {
    }

    // pure function from Lattice
    virtual void initialize() {
        // constructor does the initialization
    }

    // pure function from Lattice
    // returns a copy of this lattice
    virtual Lattice* copy() const {
        // Rely on the copy constructor
        return new LatticeArithInstance<T>(*this);
    }

    virtual LatticeArith* copyInstance() const {
      // Rely on the copy constructor                                                            
      return new LatticeArithInstance<T>(*this);
    }

    // pure function from Lattice
    // overwrites the state of "this" Lattice with "that" Lattice
    virtual void copy(Lattice* that_) {
        LatticeArithInstance<T>* that = dynamic_cast<LatticeArithInstance<T>*>(that_);
        ROSE_ASSERT((that != NULL) && "Cannot copy a LatticeArith to a LatticeArith of a different type");
        LatticeArith::copy(that);
        this->value = that->value;
    }

    // pure function from Lattice
    virtual bool operator==(Lattice* that_) {
        // Implementation of equality operator.
        LatticeArithInstance<T>* that = dynamic_cast<LatticeArithInstance<T>*>(that_);
        ROSE_ASSERT((that != NULL) && "Cannot compare two LatticeArith having different type");
        return ((this->value == that->value) && LatticeArith::operator==(that_));
    }

    
    virtual LatticeArith * operator+(LatticeArith * that_) {
        // APPLY_OPERATOR(+, int, int)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator+");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator-(LatticeArith * that_) {
        // APPLY_OPERATOR(-, int, int)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator*(LatticeArith * that_) {
        // APPLY_OPERATOR(*, int, int)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator*");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator/(LatticeArith * that_) {
        // APPLY_OPERATOR(/, int, int)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator/");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator<<(LatticeArith * that_) {
        // APPLY_OPERATOR(<<, int, int)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator<<");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator>>(LatticeArith * that_) {
        // APPLY_OPERATOR(>>, int, int)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator>>");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator>(LatticeArith * that_) {
        // APPLY_OPERATOR(>, int, bool)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator>");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator>=(LatticeArith * that_) {
        // APPLY_OPERATOR(>=, int, bool)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator>=");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator<(LatticeArith * that_) {
        // APPLY_OPERATOR(<, int, bool)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator<");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator<=(LatticeArith * that_) {
        // APPLY_OPERATOR(<=, int, bool)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator<=");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator!=(LatticeArith * that_) {
        // APPLY_OPERATOR(!=, int, bool)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator!=");
        // For the compiler not to complain
        return NULL;
    }

    virtual LatticeArith * operator==(LatticeArith * that_) {
        //  APPLY_OPERATOR(==, int, bool)
        ROSE_ASSERT(false && "Reached default in LatticeArithInstance::operator==");
        // For the compiler not to complain
        return NULL;
    }
    
    // pure function from Lattice
    virtual bool meetUpdate(Lattice* that_) {
        LatticeArithInstance<T>* that = dynamic_cast<LatticeArithInstance<T>*>(that_);
        // ROSE_ASSERT(false && "LatticeArithInstance::meetUpdate not implemented");
        if (isTop() != that->isTop()) { 
	  bottom = true;
	  return true;
	} else if (isBottom() || that->isBottom()) {
	  bottom = true;
	  return true;
	} else if (this->value == that->value) {
            return false;
        } else if (this->value != that->value) {
	  bottom = true;
	  this->value = -1;
	  return true;
	}
    }

    T getValue() {
        return value;
    }

    virtual bool setToFull() { return true; };
    virtual bool setToEmpty() { return true; };

    virtual std::string str(std::string str_) {
        std::stringstream ss;
        ss << LatticeArith::str(str_) << " value=" << value;
        return ss.str();
    }

    // expanding only the checking code for clarity
    virtual bool is_shortLatticeArith() {
        return CHECK_LATTICE_ARITH_INSTANCE_TYPE(short);
    }

    virtual bool is_intLatticeArith() {
        return CHECK_LATTICE_ARITH_INSTANCE_TYPE(int);
    }

    virtual bool setMLValueToFull(MemLocObjectPtr) {
      return false;
    }

    virtual bool isFull() {
      return bottom;
    }

    virtual bool isEmpty() {
      return top;
    }

    virtual bool isFullLat() {
      return isFull();
    }

    virtual bool isEmptyLat() {
      return isEmpty();
    }
};

typedef LatticeArithInstance<bool> BoolLatticeArith;
typedef LatticeArithInstance<char> CharLatticeArith;
typedef LatticeArithInstance<short> ShortLatticeArith;
typedef LatticeArithInstance<int> IntLatticeArith;
typedef LatticeArithInstance<long> LongLatticeArith;


#endif /* LATTICEARITHINSTANCE_H_ */
