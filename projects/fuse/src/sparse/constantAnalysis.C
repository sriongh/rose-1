#include "constantAnalysis.h"
#include "rose.h"
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/operators.hpp>
#include <new>
#include "sageInterface.h"
#include <iostream>
      
/**
 * This is an implementation of Wegman & Zadeck sparse conditional constant progagation (SCC)
 *
 * author: jisheng zhao (jz10@rice.edu)
 */
 
#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

using namespace std;
using namespace boost;
using namespace scc_private;

///////////////////////////////////////////////////////////////////////////////////////
///
/// Sparse Constant Analysis Value Object
///
///////////////////////////////////////////////////////////////////////////////////////
CAValueObject::CAValueObject(PartEdgePtr pedge) : FiniteLattice(pedge), Lattice(pedge), 
			    ValueObject(NULL) {
  top = true; bottom = false;
}

CAValueObject::CAValueObject(int v, PartEdgePtr pedge) : Lattice(pedge), FiniteLattice(pedge), 
  ValueObject(NULL) {
  value = v; top = false; bottom = false;
}

CAValueObject::CAValueObject(int v, bool t, bool b, PartEdgePtr pedge) : Lattice(pedge), 
									 FiniteLattice(pedge),
									 ValueObject(NULL) {
  value = v; top = t;  bottom = b;
}
 
bool CAValueObject::operator==(Lattice* that) {
  CAValueObject* valObj = dynamic_cast<CAValueObject* >(that);
  if (valObj == NULL)
    return false;

  if (top != valObj->top)
    return false;
  else if (bottom != valObj->bottom)
    return false;
  else
    return value == valObj->value;
}

bool CAValueObject::meetUpdate(Lattice* that) {
  CAValueObject* valObj = dynamic_cast<CAValueObject* >(that);
  if (valObj == NULL)
    return false;

  if (top != valObj->top) {
    if (!bottom && !valObj->bottom) {
      value = valObj->value;
    } else {
      setBottom();
    }
  } else if (value != valObj->value)
    setBottom();

  return true;
}

std::string CAValueObject::str(string indent) const {
  return "Constant Analysis Lattice"; // strp(latPEdge, indent);
}

std::string CAValueObject::strp(PartEdgePtr pedge, string indent) const {
  ostringstream outs;
  if(bottom)
    outs << "[bottom]";
  else if (bottom && top)
    outs << "[unknown: "<<value<<"]";
  else if (value)
    outs << "[const: "<<value<<"]";
  else if(top)
    outs << "[top]";

  return outs.str();
}

bool CAValueObject::setToFull() {
  return false;
}

bool CAValueObject::setToEmpty() {
  return false;
}

bool CAValueObject::isFullLat() {
  return false;
}

bool CAValueObject::isEmptyLat() {
  return false;
}

bool CAValueObject::mayEqualV(ValueObjectPtr o, PartEdgePtr pedge) {
  ROSE_ASSERT(false);
  return false;
}

bool CAValueObject::mustEqualV(ValueObjectPtr o, PartEdgePtr pedge) {
  ROSE_ASSERT(false);
  return false;
}

bool CAValueObject::equalSet(AbstractObjectPtr o, PartEdgePtr pedge) {
  return false;
}

bool CAValueObject::meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge) {
  ROSE_ASSERT(false);
  return false;
}

bool CAValueObject::isFull(PartEdgePtr pedge) {
  return false;
}

bool CAValueObject::isEmpty(PartEdgePtr pedge) {
  return false;
}

ValueObjectPtr CAValueObject::copyV() const {
  ROSE_ASSERT(false);
}

bool CAValueObject::isConcrete() {
  return !bottom && !top && value;
}

SgType* CAValueObject::getConcreteType() {
  ROSE_ASSERT(false);
  return NULL;
}

set<boost::shared_ptr<SgValueExp> > CAValueObject::getConcreteValue() {
  // ROSE_ASSERT(false);
  assert(isConcrete());
  set<boost::shared_ptr<SgValueExp> > concreteVals;
  concreteVals.insert(boost::shared_ptr<SgValueExp>(SageBuilder::buildIntVal(value)));
  return concreteVals;
}

bool CAValueObject::setMLValueToFull(MemLocObjectPtr memLoc) {
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////
///
/// Sparse Constant Analysis
///
///////////////////////////////////////////////////////////////////////////////////////
void SparseConstantAnalysis::genInitLattice(const Function& func, PartPtr part, PartEdgePtr pedge, 
					    std::vector<Lattice*>& initLattices) {
  // CAObjectMap* l = new CAObjectMap(boost::make_shared<CAValueObject>(pedge),
  // pedge, getComposer(), this);
  // CAValueObject* val = new CAValueObject(pedge);
  PGSSAObjectMap* l = new PGSSAObjectMap(boost::make_shared<CAValueObject>(pedge), pedge, 
					 getComposer(), this); 
  initLattices.push_back(l);
}

boost::shared_ptr<PGSSAIntraProcTransferVisitor> 
SparseConstantAnalysis::getSSATransferVisitor(// const Function& func, 
					      PartPtr part, CFGNode cn,
					      NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {                    
  // currentFuncDef = func.get_declaration()->get_definition();
  // if (visitorMap.find(currentFuncDef) == visitorMap.end()) {
  if (hasVisitor)
    return currVisitor;
  else {
    SparseConstantAnalysisTransfer* t = new SparseConstantAnalysisTransfer(part, cn, state, dfInfo,
									   getComposer(), this);
    hasVisitor = true;
    currVisitor = boost::shared_ptr<SparseConstantAnalysisTransfer>(t);
  }

  return currVisitor;
}

ValueObjectPtr SparseConstantAnalysis::Expr2Val(SgNode* sgn, PartEdgePtr pedge) {
  boost::shared_ptr<SparseConstantAnalysisTransfer> transfer = currVisitor; // getVisitor(currentFuncDef);
  
  if (SgExpression* expr = isSgExpression(sgn))
    return (boost::dynamic_pointer_cast<ValueObject>(transfer->getLatticeAtPart(expr, pedge->target())));
  else
    return PGSSA::EmptyValueObject;
} 
 
PGSSAObjectMap* SparseConstantAnalysis::getObjectMap(PartEdgePtr pedge) {
  return new PGSSAObjectMap(boost::make_shared<CAValueObject>(pedge), pedge, getComposer(), this);
}


///////////////////////////////////////////////////////////////////////////////////////
///
/// Sparse Constant Analysis Transfer Visitor
///
///////////////////////////////////////////////////////////////////////////////////////
void SparseConstantAnalysisTransfer::visit(SgIntVal *sgn) {
  CAValueObjectPtr resLat(new CAValueObject(sgn->get_value(), part->inEdgeFromAny()));
   
  // Set lattice without adding new parts for reanalysis
  setLattice(sgn, resLat, false);
}   

void SparseConstantAnalysisTransfer::visit(SgPntrArrRefExp* sgn) {
  // ROSE_ASSERT(false && "Not supported");
  PartPtr currPart = pgssa->getPart(sgn);
  SSAMemLocPtr memLoc = pgssa->getMemLocObject(sgn);
  // std::cout << "try to get next" << std::endl;
  MemLocObjectPtr ml = analysis->getComposer()->OperandExpr2MemLoc(sgn, sgn->get_lhs_operand(), part->inEdgeFromAny(), analysis);
  // analysis->getComposer()->Expr2Val(sgn, currPart->inEdgeFromAny(), analysis);
}

void SparseConstantAnalysisTransfer::visit(SgBinaryOp* sgn) {
  transferArithOp(sgn);
}

void SparseConstantAnalysisTransfer::transferArithOp(SgExpression* sgn) {
  CAValueObject * lattice = new CAValueObject(part->inEdgeFromAny());
  SgBinaryOp* binaryOp = isSgBinaryOp(sgn);
  if (binaryOp) {
    CAValueObjectPtr lhsLattice = getLattice(binaryOp->get_lhs_operand());
    CAValueObjectPtr rhsLattice = getLattice(binaryOp->get_rhs_operand());
     
    if (lhsLattice->isTop() || rhsLattice->isTop() || lhsLattice->isBottom() || rhsLattice->isBottom())
      lattice->setBottom();
    else {
      if (isSgAddOp(binaryOp))
	lattice->setValue(lhsLattice->getValue() + rhsLattice->getValue());
      else if (isSgSubtractOp(binaryOp))
	lattice->setValue(lhsLattice->getValue() - rhsLattice->getValue());
      else if (isSgMultiplyOp(binaryOp))
	lattice->setValue(lhsLattice->getValue() * rhsLattice->getValue());
      else
	lattice->setBottom();
    }
  };

  setLattice(sgn, CAValueObjectPtr(lattice)); 
}

void SparseConstantAnalysisTransfer::visit(SgAssignOp* sgn) {
  CAValueObjectPtr rhsLattice = getLattice(sgn->get_rhs_operand());

  // Set the lattice and recollect the parts that need to be reanalyzed
  setLattice(sgn, rhsLattice);
}
 
SparseConstantAnalysisTransfer::SparseConstantAnalysisTransfer(PartPtr part, CFGNode cn,
							       NodeState& state,
							       std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
							       Composer* composer, SparseConstantAnalysis* analysis_) 
  : PGSSAValueTransferVisitor<CAValueObject>(part, cn, state, dfInfo, analysis_->getPGSSA(), composer), analysis(analysis_) {}
 
CAValueObjectPtr SparseConstantAnalysisTransfer::getPhiLattice(SgNode* sgn) {
  ROSE_ASSERT(false);
  // TODO:
  CAValueObjectPtr val;
  return val;
}

