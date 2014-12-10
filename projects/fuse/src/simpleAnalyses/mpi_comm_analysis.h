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
    enum MPIOp_t { SEND, RECV, ISEND, IRECV, BARRIER, NOOP };
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
  typedef boost::shared_ptr<MPIOpAbs> MPIOpAbsPtr;

  /****************
   * MPIOpAbsType *
   ****************/
  //! Group MPI operations by the type of MPI operation
  class MPIOpAbsType : public MPIOpAbs {
    MPIOp op;
  public:
    MPIOpAbsType(const Function& mpif);
    MPIOpAbsType(const MPIOpAbsType& that);
    bool operator<(const MPIOpAbsPtr& that) const;
    bool operator==(const MPIOpAbsPtr& that) const;
  };

  typedef boost::shared_ptr<MPIOpAbsType> MPIOpAbsTypePtr;

  /*********************
   * MPIOpAbsTargetVal *
   *********************/
  //! Group MPI operations by type and value of target expression
  class MPIOpAbsTargetVal : public MPIOpAbs {
  };
  typedef boost::shared_ptr<MPIOpAbsTargetVal> MPIOpAbsTargetValPtr;

  /********************
   * MPIOpAbsCallSite *
   ********************/
  //! Group MPI operations by type and call site
  class MPIOpAbsCallSite : public MPIOpAbs {
    MPIOp op;
    const SgFunctionCallExp* callsite;
  public:
    MPIOpAbsCallSite(const Function& mpif, const SgFunctionCallExp* sgfncall);
    MPIOpAbsCallSite(const MPIOpAbsCallSite& that);
    bool operator<(const MPIOpAbsPtr& that) const;
    bool operator==(const MPIOpAbsPtr& that) const;
  };
  typedef boost::shared_ptr<MPIOpAbsCallSite> MPIOpAbsCallSitePtr;

  //! Methods for creating MPI operation abstraction
  MPIOpAbsPtr createMPIOpAbs(const Function& mpif);
  MPIOpAbsPtr createMPIOpAbs(const Function& mpif, const SgFunctionCallExp* callsite);

  /*************************
   * MPICommATSPartContext *
   *************************/
  //! Abstract based class for context associated with parts of MPICommATS.
  //! This context separates the MPI parts of the program from non MPI parts
  //! based on the usage of MPI function calls.
  //! Based on MPI function call used the part is provided a context.
  //! Provides a total order between parts implementing < operator. 
  //! Context are of two types - MPICallContext and NonMPICallContext.
  //! NonMPICallContext parts are ordered before MPICallContext parts.
  class MPICommATSPartContext : public PartContext {
  public:
    std::list<PartContextPtr> getSubPartContexts() const=0;
    bool operator==(const PartContextPtr& that) const;
    bool operator<(const PartContextPtr& that) const;
    virtual bool less(const PartContextPtr& that) const=0;
    virtual bool equals(const PartContextPtr& that) const=0;
  };
  typedef boost::shared_ptr<MPICommATSPartContext> MPICommATSPartContextPtr;

  /******************
   * MPICallContext *
   ******************/
  //! Provides a context to MPICommATSPart based on the MPI operation used.
  //! Two MPICallContext are differentiated based on the abstraction used for MPI operations.
  //! The MPI operation abstraction selectively blows up the context of MPI call sites
  //! Communication analysis carries out the semantics of MPI operation by issuing the operation
  //! to runtime based on the context.
  //! Each context is executed at least once by the communication analysis.
  class MPICallContext : public MPICommATSPartContext {
    MPIOpAbsPtr mpiopabs_p;
  public:    
    std::list<PartContextPtr> getSubPartContexts() const;
    bool less(const PartContextPtr& that) const;
    bool equals(const PartContextPtr& that) const;
  };
  typedef boost::shared_ptr<MPICallContext> MPICallContextPtr;

  //! All non MPI parts are assigned this context.
  //! Two NonMPICallContexts are differentiated based on the parent context.
  /*********************
   * NonMPICallContext *
   *********************/
  class NonMPICallContext : public MPICommATSPartContext {
    PartContextPtr parentContext_p;
  public:    
    std::list<PartContextPtr> getSubPartContexts() const;
    bool less(const PartContextPtr& that) const;
    bool equals(const PartContextPtr& that) const;
  };
  typedef boost::shared_ptr<NonMPICallContext> NonMPICallContextPtr;

  /******************
   * MPICommATSPart *
   ******************/
  class MPICommATSPart : public Part {
    MPICommATSPartContextPtr context;
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
  typedef std::map<PartPtr, MPICommATSPartPtr> Part2MCAPartMap;
  typedef std::pair<PartPtr, MPICommATSPartPtr> Part2MCAPartMapElement;

  typedef std::set<MPICommATSPartPtr> MCAPartSet;
  typedef std::map<MPICommATSPartPtr, MCAPartSet> MCAPart2MCAPartsMap;
  typedef std::pair<MPICommATSPartPtr, MCAPartSet> MCAPart2MCAPartsMapElement;

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
