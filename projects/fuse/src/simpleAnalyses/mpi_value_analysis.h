/*****************************************
 * author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#ifndef _MPIRANKANALYSIS_H
#define _MPIRANKANALYSIS_H

#include "compose.h"
#include "abstract_object_map.h"
#include "const_prop_analysis.h"
#include "mpi.h"

namespace fuse {

  class MPIValueObject;
  typedef boost::shared_ptr<MPIValueObject> MPIValueObjectPtr;

  /******************
   * MPIValueObject *
   ******************/
  class MPIValueObject : public FiniteLattice, public ValueObject {

    CPValueKindPtr kind;

  public:

    // Do we need a default constructor?
    MPIValueObject(PartEdgePtr pedge);
  
    MPIValueObject(CPValueKindPtr kind, PartEdgePtr pedge);
  
    // Do we need th copy constructor?
    MPIValueObject(const MPIValueObject & X);
  
    // Wrapper for shared_from_this that returns an instance of this class rather than its parent
    MPIValueObjectPtr shared_from_this() { return boost::static_pointer_cast<MPIValueObject>(ValueObject::shared_from_this()); }
  
    // Access functions.
    CPValueKindPtr getKind() const;
    // Sets this object's kind to the given kind, returning true if this causes the MPIValueObject to change
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
    bool meetUpdate(MPIValueObject* that);
  
    // Set this Lattice object to represent the set of all possible execution prefixes.
    // Return true if this causes the object to change and false otherwise.
    bool setToFull();
  
    // Set this Lattice object to represent the of no execution prefixes (empty set)
    // Return true if this causes the object to change and false otherwise.
    bool setToEmpty();
  
    // Returns whether this lattice denotes the set of all possible execution prefixes.
    bool isFullLat();
    // Returns whether this lattice denotes the empty set.
    bool isEmptyLat();

    bool setMLValueToFull(MemLocObjectPtr ml);
       
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
  
    // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
    // the normal ROSE mechanisms to decode it
    set<boost::shared_ptr<SgValueExp> > getConcreteValue();

    SgType* getConcreteType();

    // pretty print for the object
    std::string str(std::string indent="") const;
    std::string strp(PartEdgePtr pedge, std::string indent="") const;
  };

  /**********************
   * MVATransferVisitor *
   **********************/

  class MPIValueAnalysis;

  class MVATransferVisitor : public VariableStateTransfer<MPIValueObject, MPIValueAnalysis> {
    PartPtr part;
    CFGNode cfgn;
    NodeState& state;
    std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo;
    bool modified;

  public:
    MVATransferVisitor(PartPtr _part,
                       CFGNode _cfgn,
                       NodeState& _state,
                       std::map<PartEdgePtr, std::vector<Lattice*> >& _dfInfo,
                       Composer* _composer,
                       MPIValueAnalysis* _analysis);

    void visit(SgFunctionCallExp* sgn);
    bool finish();
  };

  /*******************
   * MPIValueAnalysis *
   *******************/

  class MPIValueAnalysis : public FWDataflow {

  public:
    // constructors
    MPIValueAnalysis();

    ComposedAnalysisPtr copy();
    void genInitLattice(PartPtr part, 
                        PartEdgePtr pedge, 
                        std::vector<Lattice*>& initLattices);
    // transfer function
    bool transfer(PartPtr part, 
                  CFGNode cfgn, 
                  NodeState& state, 
                  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

    boost::shared_ptr<DFTransferVisitor> getTransferVisitor(PartPtr part, 
                                                            CFGNode cfgn, 
                                                            NodeState& state, 
                                                            std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

    // information exported by this analysis
    bool implementsExpr2Val();
    ValueObjectPtr Expr2Val(SgNode* e, PartEdgePtr pedge);

    // analysis name
    std::string str(std::string indent) const;

    friend class MVATransferVisitor;
  };

} // end namespace;


#endif
