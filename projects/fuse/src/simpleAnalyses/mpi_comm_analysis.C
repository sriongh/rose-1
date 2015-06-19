/*****************************************
 * Author: Sriram Aananthakrishnan, 2015 *
 *****************************************/

#include "sage3basic.h"
#include "mpi_comm_analysis.h"
#include "abstract_object_map.h"

using namespace std;
using namespace boost;

namespace fuse {

  DEBUG_LEVEL(mpiCommAnalysisDebugLevel, 2);

  /**********************
   * MPICommValueObject *
   **********************/
  MPICommValueObject::MPICommValueObject(PartEdgePtr pedge)
    : Lattice(pedge),
      FiniteLattice(pedge),
      ValueObject() {
  }

  MPICommValueObject::MPICommValueObject(const MPICommValueObject& that)
    : Lattice(that),
      FiniteLattice(that),
      ValueObject(that) {
  }

  void MPICommValueObject::initialize() {
    assert(0);
  }

  Lattice* MPICommValueObject::copy() const {
    assert(0);
  }
   
  bool MPICommValueObject::meetUpdate(Lattice* thatL) {
    assert(0);
  }

  bool MPICommValueObject::operator==(Lattice* thatL) {
    assert(0);
  }

  bool MPICommValueObject::setToFull() {
    assert(0);
  }

  bool MPICommValueObject::setToEmpty()  {
    assert(0);
  }

  bool MPICommValueObject::setMLValueToFull(MemLocObjectPtr ml) {
    assert(0);
  }

  bool MPICommValueObject::isFullLat() {
    assert(0);
  }

  bool MPICommValueObject::isEmptyLat() {
    assert(0);
  }

  bool MPICommValueObject::mayEqualV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::mustEqualV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::equalSetV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::subSetV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::meetUpdateV(ValueObjectPtr thatV, PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::isEmptyV(PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::isFullV(PartEdgePtr pedge) {
    assert(0);
  }

  bool MPICommValueObject::isConcrete() {
    assert(0);
  }

  SgType* MPICommValueObject::getConcreteType() {
    assert(0);
  }

  set<boost::shared_ptr<SgValueExp> > MPICommValueObject::getConcreteValue() {
    assert(0);
  }

  ValueObjectPtr MPICommValueObject::copyV() const {
    assert(0);
  }

  string MPICommValueObject::str(string indent) const  {
    assert(0);
  }

  /********************
   * MPICommOpCallExp *
   ********************/
  MPICommOpCallExp::MPICommOpCallExp(const Function& _func,
                                     SgFunctionCallExp* _callexp,
                                     MPICommAnalysis* _analysis) 
    : mpifunc(_func),
      callexp(_callexp),
      analysis(_analysis) {
    string name = mpifunc.get_name().getString();
    if(name.compare("MPI_Send") == 0) optype = MPICommOp::SEND;
    else if(name.compare("MPI_Recv") == 0) optype = MPICommOp::RECV;
    else optype = MPICommOp::NOOP;
  }

  MPICommOpCallExp::MPICommOpCallExp(const MPICommOpCallExp& that)
    : mpifunc(that.mpifunc),
      callexp(that.callexp),
      analysis(that.analysis),
      optype(that.optype) { }

  void MPICommOpCallExp::Expr2ValVisitor::visit(SgCastExp* sgn) {
    sgn->get_operand()->accept(*this);
  }

  void MPICommOpCallExp::Expr2ValVisitor::visit(SgAddressOfOp* sgn) {
    sgn->get_operand()->accept(*this);
  }

  void MPICommOpCallExp::Expr2ValVisitor::visit(SgVarRefExp* sgn) {
    dbg << SgNode2Str(sgn) << endl;
    assert(0);
  }

  void MPICommOpCallExp::Expr2ValVisitor::visit(SgNode* sgn) {
    dbg << "sgn=" << SgNode2Str(sgn) << endl;
    assert(0);
  }

  ValueObjectPtr MPICommOpCallExp::getCommOpBufferValueObject() {
    SgExpressionPtrList exprList = callexp->get_args()->get_expressions();
    SgExpression* buffer = exprList[0];
    Expr2ValVisitor expr2Val;
    buffer->accept(expr2Val);    
  }

  ValueObjectPtr MPICommOpCallExp::getCommOpDestValueObject() {
  }

  ValueObjectPtr MPICommOpCallExp::getCommOpTagValueObject() {
  }

  bool MPICommOpCallExp::isMPICommOp() {
    return optype != MPICommOp::NOOP;
  }



  /**************************
   * MPICommAnalysisTranfer *
   **************************/
  MPICommAnalysisTransfer::MPICommAnalysisTransfer(PartPtr part,
                                                   CFGNode cfgn,
                                                   NodeState& state,
                                                   std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                                                   MPICommAnalysis* _analysis)
    : DFTransferVisitor(part, cfgn, state, dfInfo), analysis(_analysis) { 
  }

  Function MPICommAnalysisTransfer::getFunction(SgFunctionParameterList* sgn) {
    SgFunctionDeclaration* decl = isSgFunctionDeclaration(sgn->get_parent());
    ROSE_ASSERT(decl);
    Function func(decl);
    return func;
  }

  Function MPICommAnalysisTransfer::getFunction(SgFunctionCallExp* sgn) {
    SgFunctionDeclaration* decl = isSgFunctionDeclaration(sgn->getAssociatedFunctionDeclaration());
    ROSE_ASSERT(decl);
    Function func(decl);
    return func;
  }

  bool MPICommAnalysisTransfer::isMPIFuncCall(const Function& func) const {
    if(func.get_name().getString().find("MPI_") != string::npos) return true;
    return false;
  }

  void MPICommAnalysisTransfer::visit(SgFunctionCallExp* sgn) {
    Function func = getFunction(sgn);
    if(isMPIFuncCall(func)) {
      MPICommOpCallExp commOpCallExp(func, sgn, analysis);
      // Check if this CFGNode is a outgoing function call cfgIndex=2
      if(Part::isOutgoingFuncCall(cn) && commOpCallExp.isMPICommOp()) {
        // Determine the type of MPI operation
        ValueObjectPtr bufferVO = commOpCallExp.getCommOpBufferValueObject();
      }
    }
  }
  
  void MPICommAnalysisTransfer::visit(SgFunctionParameterList* sgn) {
    Function func = getFunction(sgn);
    if(isMPIFuncCall(func)) {
    }
  }

  void MPICommAnalysisTransfer::visit(SgNode* sgn) {
    // identity transfer
  }

  bool MPICommAnalysisTransfer::finish() {
  }


  /*******************
   * MPICommAnalysis *
   *******************/
  MPICommAnalysis::MPICommAnalysis(ComposedAnalysis* _analysis) : analysis(_analysis) {
  }

  // void MPICommAnalysis::initAnalysis(set<PartPtr>& startingParts) {
  //   assert(0);
  // }

  void MPICommAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge, 
                                       std::vector<Lattice*>& initLattices) {
    AbstractObjectMap* aomap = new AbstractObjectMap(boost::make_shared<MPICommValueObject>(pedge), 
                                                     pedge, 
                                                     getComposer(), 
                                                     this);
    initLattices.push_back(aomap);
  }

  boost::shared_ptr<DFTransferVisitor> MPICommAnalysis::getTransferVisitor(PartPtr part, 
                                                                           CFGNode cn, 
                                                                           NodeState& state, 
                                                                           map<PartEdgePtr, vector<Lattice*> >& dfInfo) {
    return boost::shared_ptr<MPICommAnalysisTransfer>(new MPICommAnalysisTransfer(part, cn, state, dfInfo, this));
  }

  ValueObjectPtr MPICommAnalysis::Expr2Val(SgNode* sgn, PartEdgePtr pedge) {
    return boost::make_shared<FullValueObject>();
  }

  string MPICommAnalysis::str(std::string indent) const {
    return "MPICommAnalysis";
  }
}
