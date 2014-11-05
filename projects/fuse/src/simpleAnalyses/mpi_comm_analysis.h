#ifndef _MPI_COMM_ANALYSIS_H
#define _MPI_COMM_ANALYSIS_H

/****************************************
 * Author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "compose.h"

namespace fuse {

  class MPICommAnalysis;

  /******************
   * MPICommATSPart *
   ******************/
  class MPICommATSPart : public Part {
    PartPtr parent;
    MPICommAnalysis* mpicommanalysis;
  public:
    MPICommATSPart(PartPtr base, MPICommAnalysis* analysis);

    std::list<PartEdgePtr> outEdges();
    std::list<PartEdgePtr> inEdges();

    std::set<CFGNode> CFGNodes() const;

    std::set<PartPtr> matchingCallParts() const;

    PartEdgePtr outEdgeToAny();
    PartEdgePtr inEdgeFromAny();

    bool equal(const PartPtr& that) const;
    bool less(const PartPtr& that) const;

    std::string str(std::string indent="") const;
  };

  typedef CompSharedPtr<MPICommATSPart> MPICommATSPartPtr;

  /**********************
   * MPICommATSPartEdge *
   **********************/
  class MPICommATSPartEdge : public PartEdge {
    PartEdgePtr parent;
    MPICommAnalysis* mpicommanalysis;
  public:
    MPICommATSPartEdge(MPICommAnalysis* analysis, PartEdgePtr parent);

    PartPtr source() const;
    PartPtr target() const;

    std::list<PartEdgePtr> getOperandPartEdge(SgNode* anchor, SgNode* operand);
    std::map<CFGNode, boost::shared_ptr<SgValueExp> > getPredicateValue();

    bool equal(const PartEdgePtr& that) const;
    bool less(const PartEdgePtr& that) const;

    std::string str(std::string indent="") const;
  };

  typedef CompSharedPtr<MPICommATSPartEdge> MPICommATSPartEdgePtr;

  /*******************
   * MPICommAnalysis *
   *******************/
  class MPICommAnalysis : public FWDataflow {

    std::map<PartPtr, MPICommATSPartPtr> mpiCommATSPartMap;

    std::map<MPICommATSPartPtr, std::set<MPICommATSPartPtr> > predMap;
    std::map<MPICommATSPartPtr, std::set<MPICommATSPartPtr> > succMap;

  public:
    MPICommAnalysis() { }

    ComposedAnalysisPtr copy() { return boost::make_shared<MPICommAnalysis>(); }

    // Initializes the state of analysis lattices at the given function, part and edge into our out of the part
    // by setting initLattices to refer to freshly-allocated Lattice objects.
    void genInitLattice(PartPtr part, PartEdgePtr pedge, 
                      std::vector<Lattice*>& initLattices);

    bool transfer(PartPtr part, CFGNode cn, NodeState& state, 
                  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);
   
    bool implementsExpr2MemRegion() { return false; }
    bool implementsExpr2MemLoc() { return false; }
    bool implementsExpr2Val() { return false; }

    //! Fuse functionality implemented by this analysis
    bool implementsATSGraph() { return true; }

    //! Return the start abstract state (Part) of the application
    std::set<PartPtr> GetStartAStates_Spec();
    //! Return the end abstract state (Part) of the application
    std::set<PartPtr> GetEndAStates_Spec();
    
    // pretty print for the object
    std::string str(std::string indent="") const { return "MPICommAnalysis"; }
  };
};

#endif
