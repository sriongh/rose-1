#pragma once

#ifndef LATTICEARITH_H_
#define LATTICEARITH_H_

#include "lattice.h"

using namespace fuse;

class LatticeArith : public virtual FiniteLattice {

protected:
    bool top;
    bool bottom;

public:
    LatticeArith(PartEdgePtr pe) : Lattice(pe), FiniteLattice(pe), top(false), bottom(false) {};
    
    LatticeArith(PartEdgePtr pe, bool top_, bool bottom_) : Lattice(pe), FiniteLattice(pe), 
      top(top_), bottom(bottom_) {};

    virtual ~LatticeArith() {
    }

    bool isTop() {
        return top;
    }

    bool isBottom() {
        return bottom;
    }

    bool hasValue() {
        return !(isTop() || isBottom());
    }

    // pure function from printable
    virtual std::string str(std::string str_) {
        std::stringstream ss;
        ss << str_ << " top=" << top <<  " bottom=" << bottom;
        return ss.str();
    }

    // pure function from Lattice
    virtual void initialize() = 0;

    // pure function from Lattice
    // returns a copy of this lattice
    virtual Lattice* copy() const = 0;
    virtual LatticeArith* copyInstance() const = 0;
     
    // pure function from Lattice
    // overwrites the state of "this" Lattice with "that" Lattice
    virtual void copy(Lattice* that_) {
        LatticeArith * that = dynamic_cast<LatticeArith*>(that_);
        ROSE_ASSERT(that != NULL);
        this->top = that->top;
        this->bottom = that->bottom;
    }

    // pure function from Lattice
    virtual bool operator==(Lattice* that_) {
        // Implementation of equality operator.
        LatticeArith * that = dynamic_cast<LatticeArith*>(that_);
        ROSE_ASSERT(that != NULL);
        return ((this->top == that->top) && (this->bottom == that->bottom));
    }

    virtual LatticeArith * operator+(LatticeArith * that_) = 0;
    virtual LatticeArith * operator-(LatticeArith * that_) = 0;
    virtual LatticeArith * operator*(LatticeArith * that_) = 0;
    virtual LatticeArith * operator/(LatticeArith * that_) = 0;
    virtual LatticeArith * operator<<(LatticeArith * that_) = 0;
    virtual LatticeArith * operator>>(LatticeArith * that_) = 0;
    virtual LatticeArith * operator>(LatticeArith * that_) = 0;
    virtual LatticeArith * operator>=(LatticeArith * that_) = 0;
    virtual LatticeArith * operator<(LatticeArith * that_) = 0;
    virtual LatticeArith * operator<=(LatticeArith * that_) = 0;
    virtual LatticeArith * operator!=(LatticeArith * that_) = 0;
    virtual LatticeArith * operator==(LatticeArith * that_) = 0;

    virtual bool is_shortLatticeArith() = 0;
    virtual bool is_intLatticeArith() = 0;
};


#endif /* LATTICEARITH_H_ */
