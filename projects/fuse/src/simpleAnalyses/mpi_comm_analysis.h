#ifndef _MPI_COMM_ANALYSIS_H
#define _MPI_COMM_ANALYSIS_H

/*****************************************
 * Author: Sriram Aananthakrishnan, 2015 *
 *****************************************/

#include "compose.h"

namespace fuse {

  class MPICommValueObject : public FiniteLattice, public ValueObject {
  public:
    MPICommValueObject(PartEdgePtr pedge);
    MPICommValueObject(const MPICommValueObject& that);
    
    // Lattice interface
    void initialize();
    Lattice* copy() const;
    bool meetUpdate(Lattice* thatL);
    bool operator==(Lattice* thatL);
    bool setToFull();
    bool setToEmpty();
    bool setMLValueToFull(MemLocObjectPtr ml);
    bool isFullLat();
    bool isEmptyLat();

    // ValueObject interface
    bool mayEqualV(ValueObjectPtr thatV, PartEdgePtr pedge);
    bool mustEqualV(ValueObjectPtr thatV, PartEdgePtr pedge);
    bool equalSetV(ValueObjectPtr thatV, PartEdgePtr pedge);
    bool subSetV(ValueObjectPtr thatV, PartEdgePtr pedge);
    bool meetUpdateV(ValueObjectPtr thatV, PartEdgePtr pedge);
    bool isEmptyV(PartEdgePtr pedge);
    bool isFullV(PartEdgePtr pedge);
    bool isConcrete();
    SgType* getConcreteType();
    std::set<boost::shared_ptr<SgValueExp> > getConcreteValue();
    ValueObjectPtr copyV() const;

    // printable
    std::string str(std::string indent="") const;
  };

  class MPICommAnalysis : public FWDataflow {
  public:
    MPICommAnalysis();

    ComposedAnalysisPtr copy() { return boost::make_shared<MPICommAnalysis>(); }

    // Initializes the state of analysis lattices at the given function, part and edge into our out of the part
    // by setting initLattices to refer to freshly-allocated Lattice objects.
    void genInitLattice(PartPtr part, PartEdgePtr pedge, 
                        std::vector<Lattice*>& initLattices);

    bool transfer(PartPtr part, CFGNode cn, NodeState& state, 
                  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);
   
    bool implementsExpr2MemRegion() { return false; }
    bool implementsExpr2MemLoc() { return false; }
    bool implementsExpr2Val() { return true; }

    ValueObjectPtr Expr2Val(SgNode* sgn, PartEdgePtr pedge);

    bool implementsATSGraph() { return false; }

    // pretty print for the object
    std::string str(std::string indent="") const;
  };

}; // end namespace fuse

#endif
