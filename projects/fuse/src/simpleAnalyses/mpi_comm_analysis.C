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
      FiniteLattice(pedge) {
  }

  MPICommValueObject::MPICommValueObject(const MPICommValueObject& that)
    : Lattice(that),
      FiniteLattice(that) {
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

  string MPICommValueObject::str(string indent) const  {
    assert(0);
  }

  /********************
   * MPICommOpCallExp *
   ********************/
  MPICommOpCallExp::MPICommOpCallExp(const Function& func,
                                     SgExprListExp* arglist) 
    : mpifunc(func),
      argList(arglist) {
    string name = mpifunc.get_name().getString();
    if(name.compare("MPI_Send") == 0) optype = MPICommOp::SEND;
    else if(name.compare("MPI_Recv") == 0) optype = MPICommOp::RECV;
    else optype = MPICommOp::NOOP;
  }

  MPICommOpCallExp::MPICommOpCallExp(const MPICommOpCallExp& that)
    : mpifunc(that.mpifunc),
      argList(that.argList),
      optype(that.optype) { }


  SgExpression* MPICommOpCallExp::getCommOpBufferExpr() {
    SgExpressionPtrList& exprPtrList = argList->get_expressions();
    SgExpression* expr0 = exprPtrList[0];
    SgExpression* buffExpr;
    buffExpr = expr0;
    // switch(expr0->variantT()) {
    // case V_SgCastExp:
    //   buffExpr = isSgCastExp(expr0)->get_operand();
    //   break;
    // case V_SgVarRefExp:
    //   buffExpr = expr0;
    //   break;
    // default: assert(0);
    // }
    return buffExpr;
  }

  SgExpression* MPICommOpCallExp::getCommOpDestExpr() {
    SgExpressionPtrList& exprPtrList = argList->get_expressions();
    SgExpression* expr3 = exprPtrList[3];
    SgExpression* destExpr;
    switch(expr3->variantT()) {
    case V_SgIntVal:
    case V_SgVarRefExp:
    case V_SgAddOp:
    case V_SgSubtractOp:
      destExpr = expr3;
      break;
    default: 
      dbg << "destExpr=" << SgNode2Str(expr3) << endl;
      assert(0);
    }
    return destExpr;
  }

  SgExpression* MPICommOpCallExp::getCommOpTagExpr() {
    SgExpressionPtrList& exprPtrList = argList->get_expressions();
    SgExpression* expr4 = exprPtrList[4];
    SgExpression* tagExpr;
    switch(expr4->variantT()) {
    case V_SgIntVal:
    case V_SgVarRefExp:
      tagExpr = expr4;
      break;
    default: assert(0);
    }
    return tagExpr;
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
    scope("MPICommAnalysisTransfer::visit(SgFunctionCallExp* sgn)", 
          scope::medium, attrGE("mpiCommAnalysisDebugLevel", 2));
    Function func = getFunction(sgn);
    if(isMPIFuncCall(func)) {
      MPICommOpCallExp commOpCallExp(func, sgn->get_args());
      // Check if this CFGNode is a outgoing function call cfgIndex=2
      if(Part::isOutgoingFuncCall(cn) && commOpCallExp.isMPICommOp()) {
        SgExpression* buffExpr = commOpCallExp.getCommOpBufferExpr();
        Composer* composer = analysis->getComposer();
        MemLocObjectPtr buffML = composer->OperandExpr2MemLoc(sgn, buffExpr, part->inEdgeFromAny());
        ValueObjectPtr  buffVO = composer->OperandExpr2Val(sgn, buffExpr, part->inEdgeFromAny());
        dbg << "buffML=" << buffML->str();
        dbg << "buffVO=" << buffVO->str();
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
  MPICommAnalysis::MPICommAnalysis() {
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
