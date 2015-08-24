#ifndef _MPI_COMM_ANALYSIS_H
#define _MPI_COMM_ANALYSIS_H

/*****************************************
 * Author: Sriram Aananthakrishnan, 2015 *
 *****************************************/

#include "compose.h"

namespace fuse {

  class MPICommValueObject : public FiniteLattice {

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

    // printable
    std::string str(std::string indent="") const;
  };

  // typedef MPICommValueObject<ComposedAnalysis*, false> UnionAnalMPICommValueObject;
  // typedef boost::shared_ptr<MPICommValueObject<ComposedAnalysis*, false> > UnionAnalMPICommValueObjectPtr;

  struct MPICommOp {
    enum OpType {SEND, 
                 RECV,
                 NOOP};
  };

  //! Class to process arguments of MPICommOp call expression
  class MPICommOpCallExp {
    Function mpifunc;
    SgExprListExp* argList;
    MPICommOp::OpType optype;
        
  public:
    MPICommOpCallExp(const Function& func, SgExprListExp* arglist);
    MPICommOpCallExp(const MPICommOpCallExp& that);
    SgExpression* getCommOpBufferExpr();
    SgExpression* getCommOpDestExpr();
    SgExpression* getCommOpTagExpr();
    bool isMPICommOp();
  };

  /**************************
   * MPICommAnalysisTranfer *
   **************************/

  class MPICommAnalysis;

  class MPICommAnalysisTransfer : public DFTransferVisitor {
    // Protected member variable names are part, cn, nodeState, dfInfo
    MPICommAnalysis* analysis;
  public:
    MPICommAnalysisTransfer(PartPtr part,
                            CFGNode cfgn,
                            NodeState& state,
                            std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                            MPICommAnalysis* analysis);
    void visit(SgFunctionParameterList* sgn);
    void visit(SgFunctionCallExp* sgn);
    void visit(SgNode* sgn);
    bool finish();

    //! Get the function from the parameter list
    Function getFunction(SgFunctionParameterList* sgn);
    Function getFunction(SgFunctionCallExp* sgn);
    //! Check if this function is a MPI call 
    bool isMPIFuncCall(const Function& func) const;
  };

  /*******************
   * MPICommAnalysis *
   *******************/

  class MPICommAnalysis : public FWDataflow {
  public:
    MPICommAnalysis();

    ComposedAnalysisPtr copy() { return boost::make_shared<MPICommAnalysis>(); }

    // Initializes the state of analysis lattices at the given function, part and edge into our out of the part
    // by setting initLattices to refer to freshly-allocated Lattice objects.
    void genInitLattice(PartPtr part, PartEdgePtr pedge, 
                        std::vector<Lattice*>& initLattices);

    bool transfer(PartPtr part, CFGNode cn, NodeState& state, 
                  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) { assert(0); }

    boost::shared_ptr<DFTransferVisitor> getTransferVisitor(PartPtr part, CFGNode cn, NodeState& state, 
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
