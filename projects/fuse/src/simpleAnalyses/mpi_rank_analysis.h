#ifndef _MPIRANKANALYSIS_H
#define _MPIRANKANALYSIS_H

#include "compose.h"
#include "abstract_object_map.h"

namespace fuse {

  /**********************
   * MRATransferVisitor *
   **********************/

  class MRATransferVisitor : public DFTransferVisitor {
    PartPtr part;
    CFGNode cfgn;
    NodeState& state;
    std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo;
    bool modified;

  public:
    MRATransferVisitor(PartPtr _part,
                       CFGNode _cfgn,
                       NodeState& _state,
                       std::map<PartEdgePtr, std::vector<Lattice*> >& _dfInfo);

    void visit(SgFunctionCallExp* sgn);
    bool finish();
  };

  /*******************
   * MPIRankAnalysis *
   *******************/

  class MPIRankAnalysis : public FWDataflow {

  public:
    // constructors
    MPIRankAnalysis();

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
    std::string str(std::string indent);

    friend class MRATransferVisitor;
  };

} // end namespace;


#endif
