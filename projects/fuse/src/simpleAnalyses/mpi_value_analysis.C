#include "sage3basic.h"
#include "mpi_value_analysis.h"
#include "const_prop_analysis.h"

namespace fuse {

  /******************
   * MPIValueObject *
   ******************/

  // MPIValueObject::MPIValueObject(PartEdgePtr pedge) :
  //   Lattice(pedge), FiniteLattice(pedge),
  //   ValueObject(NULL) {
  //   kind = boost::make_shared<CPUninitializedKind>();
  // }

  // MPIValueObject::MPIValueObject(CPValueKindPtr _kind, PartEdgePtr pedge)
  //   : Lattice(pedge), FiniteLattice(pedge),
  //     ValueObject(NULL),
  //     kind(_kind) {
  // }

  

  /**********************
   * MVATransferVisitor *
   **********************/

  MVATransferVisitor::MVATransferVisitor(PartPtr _part,
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

  void  MVATransferVisitor::visit(SgFunctionCallExp* sgn) {
  }

  bool MVATransferVisitor::finish() {
    return modified;
  }

  /*******************
   * MPIValueAnalysis *
   *******************/

  MPIValueAnalysis::MPIValueAnalysis()
    : FWDataflow() {
  }
  
  ComposedAnalysisPtr MPIValueAnalysis::copy() {
    return boost::make_shared<MPIValueAnalysis>();
  }

  void MPIValueAnalysis::genInitLattice(PartPtr part, 
                                       PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    AbstractObjectMap* aMap = new AbstractObjectMap(boost::make_shared<CPValueObject>(pedge),
                                                    pedge,
                                                    getComposer(),
                                                    this);
    initLattices.push_back(aMap);
  }

  bool MPIValueAnalysis::transfer(PartPtr part, 
                                 CFGNode cfgn, 
                                 NodeState& state, 
                                 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    assert(0);
    return false;
  }

  boost::shared_ptr<DFTransferVisitor>
  MPIValueAnalysis::getTransferVisitor(PartPtr part, 
                                      CFGNode cfgn, 
                                      NodeState& state, 
                                      std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    return boost::shared_ptr<DFTransferVisitor>(new MVATransferVisitor(part, cfgn, state, dfInfo));
  }

  bool MPIValueAnalysis::implementsExpr2Val() {
    return true;
  }

  ValueObjectPtr MPIValueAnalysis::Expr2Val(SgNode* e, PartEdgePtr pedge) {
    return NULLValueObject;
  }

  std::string MPIValueAnalysis::str(std::string indent="") const {
    return "MPIValueAnalysis";
  }
      
}; // end namespace
