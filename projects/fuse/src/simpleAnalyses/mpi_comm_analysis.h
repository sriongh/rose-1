#ifndef _MPI_COMM_ANALYSIS_H
#define _MPI_COMM_ANALYSIS_H

/****************************************
 * Author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "compose.h"

namespace fuse {

  class MPICommAnalysis;

  /*********
   * MPIOp *
   *********/
  //! Type of MPI operations
  class MPIOp {
    enum MPIOp_t { SEND, RECV, ISEND, IRECV, BARRIER, NOOP};
    MPIOp_t op;
  public:
    MPIOp(const Function& mpif);
    MPIOp(const MPIOp& that);
    bool operator<(const MPIOp& that) const;
    bool operator==(const MPIOp& that) const;
  };

  /************
   * MPIOpAbs *
   ************/
  //! Abstract base class for grouping MPI operations.
  class MPIOpAbs {
  public:    
    virtual bool operator==(const MPIOpAbsPtr& that) const = 0;
    virtual bool operator<(const MPIOpAbsPtr& that) const = 0;
  };
  typedef CompSharedPtr<MPIOpAbs> MPIOpAbsPtr;

  // /****************
  //  * MPIOpAbsType *
  //  ****************/
  // //! Group MPI operations by the type of MPI operation
  // class MPIOpAbsType : public MPIOpAbs {
  //   MPIOp op;
  // public:
  //   MPIOpAbsType(const Function& mpif);
  //   MPIOpAbsType(const MPIOpAbsType& that);
  //   bool operator<(const MPIOpAbsPtr& that) const;
  //   bool operator==(const MPIOpAbsPtr& that) const;
  // };

  typedef CompSharedPtr<MPIOpAbsType> MPIOpAbsTypePtr;

  /*********************
   * MPIOpAbsTargetVal *
   *********************/
  //! Group MPI operations by type and value of target expression
  class MPIOpAbsTargetVal : public MPIOpAbs {
  };
  typedef CompSharedPtr<MPIOpAbsTargetVal> MPIOpAbsTargetValPtr;

  /********************
   * MPIOpAbsCallSite *
   ********************/
  //! Group MPI operations by type and call site
  class MPIOpAbsCallSite : public MPIOpAbs {
    MPIOp op;
    //! Part that denotes the callsite at which this call was made
    PartPtr callsite;
  public:
    MPIOpAbsCallSite(const Function& mpif, PartPtr pCallSite);
    MPIOpAbsCallSite(const MPIOpAbsCallSite& that);
    bool operator<(const MPIOpAbsPtr& that) const;
    bool operator==(const MPIOpAbsPtr& that) const;
  };
  typedef CompSharedPtr<MPIOpAbsCallSite> MPIOpAbsCallSitePtr;

  //! Methods for creating MPI operation abstraction
  // MPIOpAbsPtr createMPIOpAbs(const Function& mpif);
  MPIOpAbsPtr createMPIOpAbs(const Function& mpif, PartPtr callsite);

  /**********************
   * CommATSPartContext *
   **********************/
  //! Contexts are additional information used to refine a Part reachable through multiple edges.
  //! Abstract based class for context associated with parts of CommATS.
  //! Subclassed by two types of context MPICallContext and NonMPICallContext.
  //! MPICallContext are used only at call sites of MPI operations
  class CommATSPartContext : public PartContext {
  public:
    std::list<PartContextPtr> getSubPartContexts() const=0;
    bool operator==(const PartContextPtr& that) const;
    bool operator<(const PartContextPtr& that) const;
    virtual bool less(const PartContextPtr& that) const=0;
    virtual bool equals(const PartContextPtr& that) const=0;
  };
  typedef CompSharedPtr<CommATSPartContext> CommATSPartContextPtr;

  /******************
   * MPICallContext *
   ******************/
  //! Provides a context to CommATSPart based on the MPI operation used.
  //! Two MPICallContext are differentiated based on the abstraction used for MPI operations.
  //! The MPI operation abstraction selectively blows up the context of MPI call sites
  //! Communication analysis carries out the semantics of MPI operation by issuing the operation
  //! to runtime based on the context.
  //! Each context is executed at least once by the communication analysis.
  class MPICallContext : public CommATSPartContext {
    MPIOpAbsPtr mpiopabs_p;
    PartContextPtr calleeContext_p;
  public:
    MPICallContext(MPIOpAbsPtr mpiopabs_p, PartContextPtr calleeContext_p);
    MPICallContext(const MPICallContext& that);
    std::list<PartContextPtr> getSubPartContexts() const;
    bool less(const PartContextPtr& that) const;
    bool equals(const PartContextPtr& that) const;
  };
  typedef CompSharedPtr<MPICallContext> MPICallContextPtr;

  //! All non MPI parts are assigned this context.
  //! Two NonMPIContexts are differentiated based on the parent context.
  /*****************
   * NonMPIContext *
   *****************/
  class NonMPIContext : public MPICommATSPartContext {
    PartContextPtr parentContext_p;
  public:
    NonMPIContext(PartContextPtr parentContext_p);
    NonMPIContext(const NonMPIContext& that);
    std::list<PartContextPtr> getSubPartContexts() const;
    bool less(const PartContextPtr& that) const;
    bool equals(const PartContextPtr& that) const;
  };
  typedef CompSharedPtr<NonMPIContext> NonMPIContextPtr;

  /******************
   * CommATSPart *
   ******************/
  class CommATSPart : public Part {
    CommATSPartContextPtr context;
    MPICommAnalysis* mpicommanalysis;
  public:
    CommATSPart(PartPtr base, MPICommAnalysis* analysis);

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
  typedef CompSharedPtr<CommATSPart> CommATSPartPtr;

  /**********************
   * CommATSPartEdge *
   **********************/
  class CommATSPartEdge : public PartEdge {
    PartEdgePtr parent;
    MPICommAnalysis* mpicommanalysis;
  public:
    CommATSPartEdge(MPICommAnalysis* analysis, PartEdgePtr parent);

    PartPtr source() const;
    PartPtr target() const;

    std::list<PartEdgePtr> getOperandPartEdge(SgNode* anchor, SgNode* operand);
    std::map<CFGNode, boost::shared_ptr<SgValueExp> > getPredicateValue();

    bool equal(const PartEdgePtr& that) const;
    bool less(const PartEdgePtr& that) const;

    std::string str(std::string indent="") const;
  };

  typedef CompSharedPtr<CommATSPartEdge> CommATSPartEdgePtr;

  /*******************
   * MPICommAnalysis *
   *******************/
  typedef std::map<PartPtr, CommATSPartPtr> Part2MCAPartMap;
  typedef std::pair<PartPtr, CommATSPartPtr> Part2MCAPartMapElement;

  typedef std::set<CommATSPartPtr> MCAPartSet;
  typedef std::map<CommATSPartPtr, MCAPartSet> MCAPart2MCAPartsMap;
  typedef std::pair<CommATSPartPtr, MCAPartSet> MCAPart2MCAPartsMapElement;

  class MPICommAnalysis : public FWDataflow {

    Part2MCAPartMap p2mcapMap;

    MCAPart2MCAPartsMap predMap;
    MCAPart2MCAPartsMap succMap;

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
    std::string str(const Part2MCAPartMap& p2mcapMap) const ;
    std::string str(const MCAPart2MCAPartsMap& mcap2mcapMap) const ;
    std::string str(const MCAPartSet& mcapSet) const ;
  };
};

#endif
