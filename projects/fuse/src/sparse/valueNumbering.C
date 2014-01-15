#include "valueNumbering.h"
#include "sageInterface.h"

/**
 * This is an implementation of Alpern, Wegman & Zadeck value numbering 
 * 
 * author: jisheng zhao (jz10@rice.edu)
 */
#define foreach BOOST_FOREACH

using namespace boost;
using namespace std;
using namespace scc_private;

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Value Numbering based on Sparse Framework
///
////////////////////////////////////////////////////////////////////////////////////////////

/// Sparse Value Numbering Lattice
SVNLattice::SVNLattice(PartEdgePtr pedge) : FiniteLattice(pedge), Lattice(pedge), full(false), 
					    valId(-1), value(NULL) {
  this->pedge = pedge;
}

SVNLattice::SVNLattice(PartEdgePtr pedge, SgExpression* value_, int valId_) : FiniteLattice(pedge), 
									      Lattice(pedge), 
									      full(false),
									      valId(valId_), 
									      value(value_) {
  this->pedge = pedge;
}
  
Lattice* SVNLattice::copy() const {
  SVNLattice* newLattice = new SVNLattice(pedge);
  newLattice->setVN(valId);

  return newLattice;
}

bool SVNLattice::meetUpdate(Lattice* lattice) {
  SVNLattice* vnLattice = dynamic_cast<SVNLattice* >(lattice);
  ROSE_ASSERT(vnLattice);
  
  
  return true;
}

bool SVNLattice::meetUpdate(SVNLatticePtr lattice) {
  ROSE_ASSERT(false && "Not support");
  return true;
}

bool SVNLattice::operator==(Lattice* lattice) {
  SVNLattice* vnLattice = dynamic_cast<SVNLattice* >(lattice);
  if (vnLattice)
    return getVN() == vnLattice->getVN();

  return false;
}

bool SVNLattice::equalsVal(SgExpression* valExp) {
  // TODO:
  // Compare the constant values
  
  return false;
}

/// Sparse Value Numbering memory location
SVNMemLoc::SVNMemLoc(SSAMemLocPtr memLoc_, SparseValueNumbering* analysis_)
  : SSAMemLoc(memLoc_->getSSAInstance(), memLoc_->getVarExpr(), memLoc_->getLabelMemLoc(), memLoc_->getPart()),
   MemLocObject(memLoc_->getVarExpr()) {
  analysis = analysis_;
}


bool SVNMemLoc::mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge) {
  ROSE_ASSERT(analysis);
  ROSE_ASSERT(false && "Not support");

  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(o);
  if (!memLoc) {
    bool res = false;
    // Compare the value number
    if (getValue() && memLoc->getValue())
      res = getValue() == memLoc->getValue();
    if (res)
      // If value numbers are same, then this is must equal case, otherwise keep checking
      return true;

    // TODO: the two memLocs should be exactly same type, change the code below!!
    SSAMemLocPtr ssaMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(o);
    if (!ssaMemLoc)
      // Delegate to label memory location that got from predecessor
      return this->memLoc->mayEqualML(o, pedge);
    else {
      // Delegate to label memory location that got from predecessor
      // PartPtr currPart = ((PGSSA* )ssa)->getPart(expr);
      return this->memLoc->mayEqualML(ssaMemLoc->getLabelMemLoc(), pedge); // currPart->inEdgeFromAny());
    }
  } 

  return true;
}

bool SVNMemLoc::mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge) {
  ROSE_ASSERT(analysis);
  ROSE_ASSERT(false && "Not support");

  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(o);
  if (!memLoc) {
    // Compare the value number
    if (getValue() && memLoc->getValue())
      return getValue() == memLoc->getValue();
    
    // TODO: the two memLocs should be exactly same type, change the code below!!
    SSAMemLocPtr ssaMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(o);
    if (!ssaMemLoc)
      // Delegate to label memory location that got from predecessor
      return this->memLoc->mustEqualML(o, pedge);
    else {
      // Delegate to label memory location that got from predecessor
      // PartPtr currPart = ((PGSSA* )ssa)->getPart(expr);
      return this->memLoc->mustEqualML(ssaMemLoc->getLabelMemLoc(), pedge); // currPart->inEdgeFromAny());
    }
  } 
  
  return false;
}

/// Internal comparision, i.e. compare reaching def and predecessor's may equal
bool SVNMemLoc::mayEqualML(MemLocObjectPtr o) const {
  SSAMemLocPtr ssaMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(o);
  PartPtr currPart = ((PGSSA* )ssa)->getPart(expr);
  if (!ssaMemLoc)
    // Delegate to label memory location that got from predecessor
    return this->memLoc->mayEqualML(o, currPart->inEdgeFromAny());
  else {
    // Compare the signature
    if (SSAMemLoc::isSameSig(this->expr, ssaMemLoc->getVarExpr()))
      return true;

    // Delegate to label memory location that got from predecessor
    return this->memLoc->mayEqualML(ssaMemLoc->getLabelMemLoc(), currPart->inEdgeFromAny());
  }
}

/// Internal comparision, i.e. compare reachingdef and predecessor's must equal
bool SVNMemLoc::mustEqualML(MemLocObjectPtr o) const {
  SSAMemLocPtr ssaMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(o);
  PartPtr currPart = ((PGSSA* )ssa)->getPart(expr);
  if (!ssaMemLoc)
    // Delegate to label memory location that got from predecessor
    return this->memLoc->mustEqualML(o, currPart->inEdgeFromAny());
  else {
    // Compare the signature
    if (SSAMemLoc::isSameSig(this->expr, ssaMemLoc->getVarExpr()))
      return true;

    // Delegate to label memory location that got from predecessor
    return this->memLoc->mustEqualML(ssaMemLoc->getLabelMemLoc(), currPart->inEdgeFromAny());
  }
}

/// Sparse Value Numbering
ComposedAnalysisPtr SparseValueNumbering::copy() {
  std::cout << "before copy" << std::endl;
  ComposedAnalysisPtr ret = boost::make_shared<SparseValueNumbering>(new SparseValueNumbering(this));
  std::cout << "After copy" << std::endl;
  return ret;
}

void SparseValueNumbering::genInitLattice(const Function& func, PartPtr part, PartEdgePtr pedge,
					  std::vector<Lattice* >& initLattice) {
  PGSSAObjectMap* l = new PGSSAObjectMap(boost::make_shared<SVNLattice>(pedge), pedge, getComposer(), this);
  initLattice.push_back(l);
}

boost::shared_ptr<PGSSAIntraProcTransferVisitor>
SparseValueNumbering::getSSATransferVisitor(// const Function& func, 
					    PartPtr part, CFGNode cn, NodeState& state,
					     std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
  // currentFuncDef = func.get_declaration()->get_definition();
  // if (visitorMap.find(currentFuncDef) == visitorMap.end()) {
  if (hasVisitor) 
    return currVisitor;
  else {
    SparseValueNumberingTransfer* t = new SparseValueNumberingTransfer(part, cn, state, dfInfo,
									getComposer(), this);
    hasVisitor = true;
    currVisitor = boost::shared_ptr<SparseValueNumberingTransfer>(t);
  }

  return currVisitor;
}

PGSSAObjectMap* SparseValueNumbering::getObjectMap(PartEdgePtr pedge) {
  return new PGSSAObjectMap(boost::make_shared<SVNLattice>(pedge), pedge, getComposer(), this);
}

MemLocObjectPtr SparseValueNumbering::Expr2MemLoc(SgNode* sgn, PartEdgePtr pedge) {
  boost::shared_ptr<SparseValueNumberingTransfer> transfer = currVisitor; // getVisitor(currentFuncDef)
  
  MemLocObjectPtr memLoc =  pgssa.getMemLocObject(sgn);
  if (!memLoc)
    memLoc = PGSSA::createSSAMemLoc(sgn, pedge->target(), &pgssa);
  
  return memLoc;
}

/// Sparse Value Numbering Transfer Vistor
SparseValueNumberingTransfer::SparseValueNumberingTransfer(// const Function& func, 
							   PartPtr part,
							   CFGNode cn, NodeState& state,
							   std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
							   Composer* composer,
							   SparseValueNumbering* analysis_)
  : PGSSAValueTransferVisitor<SVNLattice>(part, cn, state, dfInfo, analysis_->getPGSSA(),
                                          composer), analysis(analysis_), modified(false) {}

/// Initialize the global arithmatic ID manager
GlobalArithID SparseValueNumberingTransfer::gArithID;

/// Create a new lattice based on the valueexpression
SVNLatticePtr SparseValueNumberingTransfer::createSVNLattice(SgExpression* expr, 
							     PartEdgePtr pedge) {
  if (SgValueExp* valExpr = isSgValueExp(expr))
    foreach(SVNLatticePtr l, values) {
      // Check equality
      if (l->equalsVal(valExpr))
	return SVNLatticePtr(new SVNLattice(pedge, valExpr, l->getVN()));
    }

  SVNLatticePtr l = SVNLatticePtr(new SVNLattice(pedge, expr, gArithID.getValID()));
  values.insert(l);
  
  return l; 
}

/// Customized get memory location process, i.e. will replace value numbering memory location with standard SSA 
/// memory location on demand
SSAMemLocPtr SparseValueNumberingTransfer::getMemLocObject(SgNode* sgn) {
  SSAMemLocPtr oldMemLoc = pgssa->getMemLocObject(sgn);
  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(oldMemLoc);
  // The value numbering memory location was already there
  if (memLoc)
    return memLoc;
  
  // Create the value numbering memory location and reregister it in PGSSA internal tables
  memLoc = SVNMemLocPtr(new SVNMemLoc(oldMemLoc, (SparseValueNumbering* )analysis));
  pgssa->reRegisterMemLocObj(sgn, oldMemLoc, memLoc);
  
  return memLoc;
}

void SparseValueNumberingTransfer::visit(SgValueExp *sgn) {
  // Create a new lattice 
  SVNLatticePtr l = createSVNLattice(sgn, part->inEdgeFromAny());
  // Register the lattice for constant value
  setLattice(sgn, l, false);
 
  // Add value numbering lattice to memloc object
  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(getMemLocObject(sgn));
  if (memLoc)
    memLoc->setValue(l);
  else {
    if (SgIntVal* intVal = isSgIntVal(sgn))
      std::cout << "no memloc?? " << sgn << " " << sgn->class_name() 
		<< " val: " << intVal->get_value() << std::endl;
  }
}

void SparseValueNumberingTransfer::visit(SgVarRefExp* sgn) {
  // Re-collect current point-to information
  SVNLatticePtr l = getCurrentLattice(sgn, true);
  if (l) {
  } else 
    // Create new lattice
    l = createSVNLattice(sgn, part->inEdgeFromAny());
  setLattice(sgn, l, false);

  // Add value numbering lattice to memloc object
  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(getMemLocObject(sgn));
  memLoc->setValue(l);
}

void SparseValueNumberingTransfer::visit(SgDotExp* sgn) {
  // Re-collect current value numbering information
  SVNLatticePtr l = getCurrentLattice(sgn, true);
  if (l) {
  } else
    // Create new lattice
    l = createSVNLattice(sgn, part->inEdgeFromAny());
  setLattice(sgn, l, false);

  // Add value numbering lattice to memloc object
  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(getMemLocObject(sgn));
  memLoc->setValue(l);
}
  
void SparseValueNumberingTransfer::visit(SgAssignOp* sgn) {
  SVNLatticePtr l = getLattice(sgn->get_rhs_operand());
  // Set the lattice and recollect the parts that need to be reanalyzed
  setLattice(sgn, l);

  // Add value numbering lattice to memloc object
  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(getMemLocObject(sgn));
  memLoc->setValue(l);
}

void SparseValueNumberingTransfer::visit(SgArrowExp* sgn) {
  // Re-collect current value numbering information
  SVNLatticePtr l = getCurrentLattice(sgn, true);
  if (l) {
  } else
    // Create new lattice
    l = createSVNLattice(sgn, part->inEdgeFromAny());
  setLattice(sgn, l, false);

  // Add value numbering lattice to memloc object
  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(getMemLocObject(sgn));
  memLoc->setValue(l);
}

void SparseValueNumberingTransfer::visit(SgPntrArrRefExp* sgn) {
  // Re-collect current value numbering information
  SVNLatticePtr l = getCurrentLattice(sgn, true);
  if (l) {
  } else
    // Create new lattice
    l = createSVNLattice(sgn, part->inEdgeFromAny());
  setLattice(sgn, l, false);

  // Add value numbering lattice to memloc object
  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(getMemLocObject(sgn));
  memLoc->setValue(l);
}

void SparseValueNumberingTransfer::visit(SgBinaryOp* sgn) {
  // Record binary operation
  SVNLatticePtr l = transferArithOp(sgn);

  // Add value numbering lattice to memloc object
  SVNMemLocPtr memLoc = boost::dynamic_pointer_cast<SVNMemLoc>(getMemLocObject(sgn));
  memLoc->setValue(l);
}

SVNLatticePtr SparseValueNumberingTransfer::transferArithOp(SgExpression* sgn) {
  SVNLatticePtr l = createSVNLattice(sgn, part->inEdgeFromAny());
  SgBinaryOp* binaryOp = isSgBinaryOp(sgn);
  if (binaryOp) {
    SVNLatticePtr lhsLattice = getLattice(binaryOp->get_lhs_operand());
    SVNLatticePtr rhsLattice = getLattice(binaryOp->get_rhs_operand());
    
    // Acauire the Value ID from the arithmatic ID manager
    int valID = gArithID.getArithID(binaryOp, lhsLattice->getVN(), rhsLattice->getVN());
    // Set the correct value ID
    l->setVN(valID);
  };

  setLattice(sgn, l);
  
  return l;
}

SVNLatticePtr SparseValueNumberingTransfer::getPhiLattice(SgNode* sgn) {
  ROSE_ASSERT(false);
  // TODO:
  SVNLatticePtr val;
  return val;
}

/// Get the ID for binary operation, if it is an unsupported binary operation, return -1
int GlobalArithID::getArithID(SgBinaryOp* sgn, int lID, int rID) {
  if (SgAddOp* addOp = isSgAddOp(sgn)) {
    if (internal_adds.find(lID) != internal_adds.end()) {
      map<int, int>& valMap = internal_adds[lID];
      if (valMap.find(rID) != valMap.end())
	return valMap[rID];
    } 
    if (internal_adds.find(rID) != internal_adds.end()) {
      map<int, int>& valMap = internal_adds[lID];
      if (valMap.find(lID) != valMap.end())
	return valMap[lID];
    }
    // Register new value ID
    int newID = getValID();
    internal_adds[lID][rID] = newID;
    internal_adds[rID][lID] = newID;
  } else if (SgSubtractOp* subOp = isSgSubtractOp(sgn)) {
    if (internal_subs.find(lID) != internal_subs.end()){
      map<int, int>& valMap = internal_subs[lID];
      if (valMap.find(rID) != valMap.end())
	return valMap[rID];
    }
    // Registernew value ID
    int newID = getValID();
    internal_subs[lID][rID] = newID;
  } else if (SgMultiplyOp* mulOp = isSgMultiplyOp(sgn)) {
    if (internal_muls.find(lID) != internal_muls.end()){
      map<int, int>& valMap = internal_muls[lID];
      if (valMap.find(rID) != valMap.end())
	return valMap[rID];
    }
    if (internal_muls.find(rID) != internal_muls.end()){
      map<int, int>& valMap = internal_muls[rID];
      if (valMap.find(lID) != valMap.end())
	return valMap[lID];
    }
    // Register new value ID
    int newID = getValID();
    internal_muls[lID][rID] = newID;
    internal_muls[rID][lID] = newID;
  } else if (SgDivideOp* divOp = isSgDivideOp(sgn)) {
    if (internal_divs.find(lID) != internal_divs.end()){
      map<int, int>& valMap = internal_divs[lID];
      if (valMap.find(rID) != valMap.end())
	return valMap[rID];
    }
    // Register new value ID
    int newID = getValID();
    internal_divs[lID][rID] = newID;
  } else if (SgModOp* modOp = isSgModOp(sgn)) {
    if (internal_mods.find(lID) != internal_mods.end()){
      map<int, int>& valMap = internal_mods[lID];
      if (valMap.find(rID) != valMap.end())
	return valMap[rID];
    }
    // Register new value ID
    int newID = getValID();
    internal_mods[lID][rID] = newID;
  } else if (SgAndOp* andOp = isSgAndOp(sgn)) {
    if (internal_ands.find(lID) != internal_ands.end()){
      map<int, int>& valMap = internal_ands[lID];
      if (valMap.find(rID) != valMap.end())
	return valMap[rID];
    }
    if (internal_ands.find(rID) != internal_ands.end()){
      map<int, int>& valMap = internal_ands[rID];
      if (valMap.find(lID) != valMap.end())
	return valMap[lID];
    }
    // Register new value ID
    int newID = getValID();
    internal_ands[lID][rID] = newID;
    internal_ands[rID][lID] = newID;
  } else if (SgOrOp* orOp = isSgOrOp(sgn)) {
    if (internal_ors.find(lID) != internal_ors.end()){
      map<int, int>& valMap = internal_ors[lID];
      if (valMap.find(rID) != valMap.end())
	return valMap[rID];
    }
    if (internal_ors.find(rID) != internal_ors.end()){
      map<int, int>& valMap = internal_ors[rID];
      if (valMap.find(lID) != valMap.end())
	return valMap[lID];
    }
    // Register new value ID
    int newID = getValID();
    internal_ors[lID][rID] = newID;
    internal_ors[rID][lID] = newID;
  }

  return -1;
}
