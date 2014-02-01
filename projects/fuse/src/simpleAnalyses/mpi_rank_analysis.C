#include "sage3basic.h"
#include "mpi_rank_analysis.h"
#include "const_prop_analysis.h"

namespace fuse {

  /**********************
   * MRATransferVisitor *
   **********************/

  MRATransferVisitor::MRATransferVisitor(PartPtr _part,
                                         CFGNode _cfgn,
                                         NodeState& _state,
                                         std::map<PartEdgePtr, std::vector<Lattice*> >& _dfInfo)
    : DFTransferVisitor(_part, _cfgn, _state, _dfInfo),
      part(_part),
      cfgn(_cfgn),
      state(_state),
      dfInfo(_dfInfo),
      modified(false) {
  }

  void  MRATransferVisitor::visit(SgFunctionCallExp* sgn) {
  }

  bool MRATransferVisitor::finish() {
    return modified;
  }

  /*******************
   * MPIRankAnalysis *
   *******************/

  MPIRankAnalysis::MPIRankAnalysis()
    : FWDataflow() {
  }
  
  ComposedAnalysisPtr MPIRankAnalysis::copy() {
    return boost::make_shared<MPIRankAnalysis>();
  }

  void MPIRankAnalysis::genInitLattice(PartPtr part, 
                                       PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    AbstractObjectMap* aMap = new AbstractObjectMap(boost::make_shared<CPValueObject>(pedge),
                                                    pedge,
                                                    getComposer(),
                                                    this);
    initLattices.push_back(aMap);
  }

  bool MPIRankAnalysis::transfer(PartPtr part, 
                                 CFGNode cfgn, 
                                 NodeState& state, 
                                 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    assert(0);
    return false;
  }

  boost::shared_ptr<DFTransferVisitor>
  MPIRankAnalysis::getTransferVisitor(PartPtr part, 
                                      CFGNode cfgn, 
                                      NodeState& state, 
                                      std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    return boost::shared_ptr<DFTransferVisitor>(new MRATransferVisitor(part, cfgn, state, dfInfo));
  }

  bool MPIRankAnalysis::implementsExpr2Val() {
    return true;
  }

  ValueObjectPtr MPIRankAnalysis::Expr2Val(SgNode* e, PartEdgePtr pedge) {
    return NULLValueObject;
  }

  std::string MPIRankAnalysis::str(std::string indent="") {
    return "MPIRankAnalysis";
  }
      
}; // end namespace
