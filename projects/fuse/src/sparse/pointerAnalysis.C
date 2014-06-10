#include "pointerAnalysis.h"
#include "heapReachingDef.h"
#include "sageInterface.h"
#include "stx_analysis.h"
    
/**
 * This is an implementation of Ondrej Lothak'11 pointer analysis
 * 
 * author: jisheng zhao (jz10@rice.edu)
 */

#define foreach BOOST_FOREACH

using namespace boost;
using namespace std;
using namespace scc_private;

std::string getVarName_(SgNode* sgn) {
  std::stringstream ss;

  if (!sgn) {
    ss << "[NULL]";
    return ss.str();
  }

  if (SgVarRefExp* varRef = isSgVarRefExp(sgn))
    ss << varRef->get_symbol()->get_name();
  else if (SgAddressOfOp* addrOf = isSgAddressOfOp(sgn))
    ss << "&" << getVarName_(addrOf->get_operand());
  else if (SgPointerDerefExp* deRef = isSgPointerDerefExp(sgn))
    ss << "*" << getVarName_(deRef->get_operand());
  else if (SgIntVal* intVal = isSgIntVal(sgn))
    ss << intVal->get_value();
  else
    ss << sgn->class_name() << "[" << sgn << "]";

  return ss.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Sparse PointTo Analysis
///
/////////////////////////////////////////////////////////////////////////////////////////////

/// Sparse PointTo memory location
SPTMemLoc::SPTMemLoc(SSAMemLocPtr memLoc_, SparsePointToAnalysis* analysis_) 
: SSAMemLoc(memLoc_->getSSAInstance(), memLoc_->getVarExpr(), memLoc_->getLabelMemLoc(), memLoc_->getPart()), 
MemLocObject(memLoc_->getVarExpr()) {
  analysis = analysis_;
}


bool SPTMemLoc::mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge) {
  ROSE_ASSERT(analysis);
  
  SPTMemLocPtr memLoc = boost::dynamic_pointer_cast<SPTMemLoc>(o);
  if (!memLoc) {
    SSAMemLocPtr ssaMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(o);
    if (!ssaMemLoc)
      // Delegate to label memory location that got from predecessor
      return this->memLoc->mayEqualML(o, pedge);
    else {
      // Delegate to label memory location that got from predecessor
      // PartPtr currPart = ((PGSSA* )ssa)->getPart(expr);
      return this->memLoc->mayEqualML(ssaMemLoc->getLabelMemLoc(), pedge); // currPart->inEdgeFromAny());
    }
  } else 
    return analysis->mayAlias(expr, memLoc);
}

bool SPTMemLoc::mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge) {
  ROSE_ASSERT(analysis);
  
  SPTMemLocPtr memLoc = boost::dynamic_pointer_cast<SPTMemLoc>(o);
  if (!memLoc) {
    SSAMemLocPtr ssaMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(o);
    if (!ssaMemLoc) 
      // Delegate to label memory location that got from predecessor
      return this->memLoc->mustEqualML(o, pedge);
    else {
      // Delegate to label memory location that got from predecessor
      // PartPtr currPart = ((PGSSA* )ssa)->getPart(expr);
      return this->memLoc->mustEqualML(ssaMemLoc->getLabelMemLoc(), pedge); // currPart->inEdgeFromAny());
    }
  } else 
    return analysis->mustAlias(expr, memLoc);
}

/// Internal comparision, i.e. compare reaching def and predecessor's may equal
bool SPTMemLoc::mayEqualML(MemLocObjectPtr o) const { 
  SSAMemLocPtr ssaMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(o);
  PartPtr currPart = ((PGSSA* )ssa)->getPart(expr);
  if (!ssaMemLoc)
    // Delegate to label memory location that got from predecessor
    return this->memLoc->mayEqualML(o, currPart->inEdgeFromAny());
  else {
    // Compare the signature
    if (isSameSig(this->expr, ssaMemLoc->getVarExpr()))
      return true;

    // Delegate to label memory location that got from predecessor
    return this->memLoc->mayEqualML(ssaMemLoc->getLabelMemLoc(), currPart->inEdgeFromAny());
  }
}

/// Internal comparision, i.e. compare reachingdef andpredecessor's must equal
bool SPTMemLoc::mustEqualML(MemLocObjectPtr o) const { 
  SSAMemLocPtr ssaMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(o);
  PartPtr currPart = ((PGSSA* )ssa)->getPart(expr);
  if (!ssaMemLoc)
    // Delegate to label memory location that got from predecessor
    return this->memLoc->mustEqualML(o, currPart->inEdgeFromAny());
  else {
    // Compare the signature
    if (isSameSig(this->expr, ssaMemLoc->getVarExpr()))
      return true;

    // Delegate to label memory location that got from predecessor
    return this->memLoc->mustEqualML(ssaMemLoc->getLabelMemLoc(), currPart->inEdgeFromAny());
  }
}

/// Sparse PointTo Lattice
SPTLattice::SPTLattice(PartEdgePtr pedge) : FiniteLattice(pedge), Lattice(pedge), full(false) {
  this->pedge = pedge;
}

Lattice* SPTLattice::copy() const {
  SPTLattice* newLattice = new SPTLattice(pedge);
  newLattice->copyPTSet(ptSet);

  return newLattice;
}

bool SPTLattice::meetUpdate(Lattice* lattice) {
  SPTLattice* ptLattice = dynamic_cast<SPTLattice* >(lattice);
  ROSE_ASSERT(ptLattice);
  ptSet.insert(ptLattice->ptSet.begin(), ptLattice->ptSet.end());
}

bool SPTLattice::meetUpdate(SPTLatticePtr lattice) {
  ptSet.insert(lattice->ptSet.begin(), lattice->ptSet.end());
}

bool SPTLattice::operator==(Lattice* lattice) {
  SPTLattice* ptLattice = dynamic_cast<SPTLattice* >(lattice);
  if (ptLattice)
    return ptSet == ptLattice->ptSet;
  else
    return false;
}

/// Update current lattice's point-to set with current lattice's point-to set
bool SPTLattice::update(SPTLatticePtr another) {
  if (another->ptSet.size() == 0)
    return false;

  bool ret = false;
  for (PTSet::iterator pti = another->ptSet.begin(); pti != another->ptSet.end(); ++ pti) {
    if (ptSet.find(* pti) == ptSet.end()) {
      ptSet.insert(* pti);
      ret = true;
    }
  }

  return ret;
}

/// Add one memory location to point-to set
bool SPTLattice::add(SPTMemLocPtr memLoc) {
  if (ptSet.find(memLoc) != ptSet.end())
    return false;
  else{
    ptSet.insert(memLoc);   
    return true;
  }
}

/// Dump lattice content
void SPTLattice::dump(PGSSA* pgssa) {
  std::cout << "{";
  SSAMemLocPtr memLoc;
  foreach(memLoc, ptSet) {
    PartPtr part = pgssa->getPart(memLoc->getVarExpr());
    std::cout << part->str();
    std::cout << ","; 
  }
  std::cout << "}" << std::endl;
}

/// Sparse PointTo Transfer Visitor
SparsePointToAnalysisTransfer::SparsePointToAnalysisTransfer(// const Function& func, 
							     PartPtr part, 
							     CFGNode cn, NodeState& state,
							     std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
							     Composer* composer, 
							     SparsePointToAnalysis* analysis_)
  : PGSSAValueTransferVisitor<SPTLattice>(part, cn, state, dfInfo, analysis_->getPGSSA(), 
					  composer), analysis(analysis_), modified(false) {}

void SparsePointToAnalysisTransfer::visit(SgVarRefExp* sgn) {
  // std::cout << "before visit: " << sgn->class_name() << std::endl;
  analysis->handleExpr(sgn);
  // std::cout << "before visit 1: " << sgn->class_name() << std::endl;
  // Re-collect current point-to information
  SPTLatticePtr l = getCurrentLattice(sgn);
  // std::cout << "before visit 2: " << sgn->class_name() << std::endl;
  if (l) setLattice(sgn, l, false);
} 
   
void SparsePointToAnalysisTransfer::visit(SgDotExp* sgn) {
  // std::cout << "before visit: "<< sgn->class_name() <<std::endl;
  analysis->handleExpr(sgn);
  // Re-collect current point-to information
  SPTLatticePtr l = getCurrentLattice(sgn);
  if (l) setLattice(sgn, l, false);
}

void SparsePointToAnalysisTransfer::visit(SgAssignOp* sgn) {
  // std::cout << "before visit: "<< sgn->class_name() <<std::endl;
  analysis->handleExpr(sgn);
  // Handle different constraint cases:
  // 1. assign: p = q;
  SgVarRefExp* lv = isSgVarRefExp(sgn->get_lhs_operand());
  SgVarRefExp* rv = isSgVarRefExp(sgn->get_rhs_operand());
  if (lv && rv) {
    // copy lattice
    SPTLatticePtr lLattice = getCurrentLattice(lv);
    SPTLatticePtr rLattice = getCurrentLattice(rv);
    if (lLattice == rLattice)
      return;
    else {
      // update the new values into lhs operand
      modified = lLattice->update(rLattice);
      setLattice(lv, lLattice, modified);
    }

    return;
  }

  // 2. address taken: p = &q;
  SgExpression* le = isSgExpression(sgn->get_lhs_operand());
  SgAddressOfOp* addr = isSgAddressOfOp(sgn->get_rhs_operand());
  if (le && addr){
    // update point-to set with new point-to value
    SPTLatticePtr lLattice = getCurrentLattice(le);
    SPTMemLocPtr memLoc = analysis->getSPTMemLoc(addr);
    if (memLoc) {
      modified = lLattice->add(memLoc);
      setLattice(le, lLattice, modified);
      // lLattice->dump(pgssa);
    }

    return;
  }

  // 3. load: p = * q 
  le = isSgExpression(sgn->get_lhs_operand());
  SgPointerDerefExp* pdr = isSgPointerDerefExp(sgn->get_rhs_operand());
  if (le && pdr) {
    SPTLatticePtr lLattice = getCurrentLattice(le);
    SgExpression* re = pdr->get_operand();
    SPTLatticePtr rLattice = getCurrentLattice(re);
    
    for (PTSet::iterator pti = rLattice->getPTSet().begin(); pti != rLattice->getPTSet().end(); 
	 ++ pti) {
      SPTLatticePtr lattice = getCurrentLattice((* pti)->getVarExpr());
      if (lattice) 
	modified |= lLattice->update(lattice);
    }
    if (modified)
      setLattice(le, lLattice, modified);

    return;
  }

  // 4. store: * p = q
  pdr = isSgPointerDerefExp(sgn->get_lhs_operand());
  SgExpression* re = isSgExpression(sgn->get_rhs_operand());
  if (pdr && re) {
    // copy lattice
    SPTLatticePtr lLattice = getCurrentLattice(pdr);
    SPTLatticePtr rLattice = getCurrentLattice(re);
    ROSE_ASSERT(lLattice && rLattice);
    if (lLattice == rLattice)
      return;
    else {
      // update the new values into lhs operand
      modified = lLattice->update(rLattice);
      setLattice(pdr, lLattice, modified);
    }
   
    return;
  }
}

void SparsePointToAnalysisTransfer::visit(SgAddressOfOp* sgn) {
  // std::cout << "before visit: "<< sgn->class_name() <<std::endl;
  analysis->handleExpr(sgn);
  // Re-collect current point-to information
  SPTLatticePtr l = getCurrentLattice(sgn);
  if (l)
    setLattice(sgn, l, false);
}

void SparsePointToAnalysisTransfer::visit(SgArrowExp* sgn) {
  // std::cout << "before visit: "<< sgn->class_name() <<std::endl;
  analysis->handleExpr(sgn);
  // Re-collect current point-to information
  SPTLatticePtr l = getCurrentLattice(sgn);
  if (l)
    setLattice(sgn, l, false);
}

void SparsePointToAnalysisTransfer::visit(SgPntrArrRefExp* sgn) {
  // std::cout << "before visit: "<< sgn->class_name() <<std::endl;
  analysis->handleExpr(sgn);
  // Re-collect current point-to information
  SPTLatticePtr l = getCurrentLattice(sgn);
  if (l)
    setLattice(sgn, l, false);
}

void SparsePointToAnalysisTransfer::visit(SgPointerDerefExp* sgn) {
  // std::cout << "before visit: "<< sgn->class_name() <<std::endl;
  analysis->handleExpr(sgn);
  // Re-collect current point-to information
  SPTLatticePtr l = getCurrentLattice(sgn);
  if (l)
    setLattice(sgn, l, false);
}

SPTLatticePtr SparsePointToAnalysisTransfer::getPhiLattice(SgNode* sgn) {
  ROSE_ASSERT(false);
  SPTLatticePtr l;
  return l;
}

/// Sparse PointTo Analysis
void SparsePointToAnalysis::genInitLattice(const Function& func, PartPtr part, PartEdgePtr pedge, 
                                           std::vector<Lattice* >& initLattice) {
  PGSSAObjectMap* l = new PGSSAObjectMap(boost::make_shared<SPTLattice>(pedge), pedge, getComposer(), this);
  initLattice.push_back(l);
}

boost::shared_ptr<PGSSAIntraProcTransferVisitor>
SparsePointToAnalysis::getSSATransferVisitor(// const Function& func, 
					     PartPtr part, CFGNode cn, NodeState& state,
                                              std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
  // currentFuncDef = func.get_declaration()->get_definition();
  // if (visitorMap.find(currentFuncDef) == visitorMap.end()) {
  if (hasVisitor)
    return currVisitor;
  else {
    SparsePointToAnalysisTransfer* t = new SparsePointToAnalysisTransfer(part, cn, state, dfInfo, 
									 getComposer(), this);
    hasVisitor = true;
    currVisitor = boost::shared_ptr<SparsePointToAnalysisTransfer>(t);
  }

  return currVisitor;
} 

void SparsePointToAnalysis::handleExpr(SgExpression* expr) {
  if (exprMap.count(expr) > 0)
    return;

  // Create the memory location
  SPTMemLocPtr memLoc = getSPTMemLoc(expr);
  exprMap[expr] = memLoc;
}

SPTMemLocPtr SparsePointToAnalysis::getSPTMemLoc(SgExpression* expr) {
  SSAMemLocPtr memLoc = pgssa.getMemLocObject(expr);
  if (memLoc == PGSSA::EmptySSAMemLoc)
    ROSE_ASSERT(false && "Couldn't find SSA memory location");
  
  return getSPTMemLoc(memLoc);
}


SPTMemLocPtr SparsePointToAnalysis::getSPTMemLoc(SSAMemLocPtr memLoc) {
  if (memLocMap.count(memLoc) > 0)
    return memLocMap[memLoc];

  SPTMemLocPtr sptMemLoc = SPTMemLocPtr(new SPTMemLoc(memLoc, this));
  memLocMap[memLoc] = sptMemLoc;
  
  return sptMemLoc;
}

bool SparsePointToAnalysis::mayAlias(SgExpression* expr, SPTMemLocPtr memLoc) {
  ROSE_ASSERT(exprMap.count(expr) > 0);
  SPTMemLocPtr keyML = exprMap[expr];
  PGSSAObjectMap* ptg = currentObjMap();
  ROSE_ASSERT(ptg);
  LatticePtr lattice = ptg->getValue(keyML);
  SPTLatticePtr ptLattice = boost::dynamic_pointer_cast<SPTLattice>(lattice);
  if (ptLattice && ptLattice->getNumOfAlias() > 0) {
    lattice = ptg->getValue(memLoc);
    SPTLatticePtr another = boost::dynamic_pointer_cast<SPTLattice>(lattice);
    if (another && another->getNumOfAlias() > 0) {
      SPTMemLocPtr memLocL;
      foreach(memLocL, ptLattice->getPTSet()) {
        SPTMemLocPtr memLocR;        
        foreach(memLocR, another->getPTSet()) {
          if (memLocL->mayEqualML(memLocR))
            return true;
        }
      }
    } 
  } 

  return false;
}

bool SparsePointToAnalysis::mustAlias(SgExpression* expr, SPTMemLocPtr memLoc) {
  ROSE_ASSERT(exprMap.count(expr));
  SPTMemLocPtr keyML = exprMap[expr];
  PGSSAObjectMap* ptg = currentObjMap();
  ROSE_ASSERT(ptg); 
  LatticePtr lattice = ptg->getValue(keyML);
  SPTLatticePtr ptLattice = boost::dynamic_pointer_cast<SPTLattice>(lattice);
  if (ptLattice && ptLattice->getNumOfAlias() > 0) {
    lattice = ptg->getValue(memLoc);
    SPTLatticePtr another = boost::dynamic_pointer_cast<SPTLattice>(lattice);
    if (another && another->getNumOfAlias() > 0) {
      SPTMemLocPtr memLocL;
      foreach(memLocL, ptLattice->getPTSet()) {
        SPTMemLocPtr memLocR;
        foreach(memLocR, another->getPTSet()) {
          if (memLocL->mustEqualML(memLocR)) // TODO: change to UnionMemLocs mustEqual!!
            return true;
        }
      }
    }
  }

  return false; 
}
  
PGSSAObjectMap* SparsePointToAnalysis::getObjectMap(PartEdgePtr pedge) {
  return new PGSSAObjectMap(boost::make_shared<SPTLattice>(pedge), pedge, getComposer(), this);
}

/*boost::shared_ptr<SparsePointToAnalysisTransfer> SparsePointToAnalysis::getVisitor(const Function& func) {
  SparsePointToAnalysisTransfer* t = NULL;
  SgFunctionDefinition* funcDef = func.get_declaration()->get_definition();
  ROSE_ASSERT(visitorMap.find(funcDef) != visitorMap.end());
  return visitorMap[funcDef];
}*/

MemLocObjectPtr SparsePointToAnalysis::Expr2MemLoc(SgNode* sgn, PartEdgePtr pedge) {
  /*boost::shared_ptr<SparsePointToAnalysisTransfer> transfer = getVisitor(currentFuncDef);

  if (SgExpression* expr = isSgExpression(sgn)) {
    return (boost::dynamic_pointer_cast<MemLocObject>(transfer->getLatticeAtPart(expr, pedge->target())));
  } else if (SgInitializedName* initName = isSgInitializedName(sgn)) {
    return (boost::dynamic_pointer_cast<MemLocObject>(transfer->getLatticeAtPart(initName, pedge->target())));
  } else 
    ROSE_ASSERT(false && "Unsupported node");
  */
  if (exprMap.find(sgn) == exprMap.end()) {
    // ROSE_ASSERT(false && "Expression was not analyzed");
    MemLocObjectPtr memLoc =  pgssa.getMemLocObject(sgn);
    if (!memLoc)
      memLoc = PGSSA::createSSAMemLoc(sgn, pedge->target(), &pgssa);
    return memLoc;
  } else 
    return exprMap[sgn];
}

void SparsePointToAnalysis::dumpPTG() {
  std::cout << "---------------------------" << std::endl;
  std::cout << "dump PTG" << std::endl;
  SgFunctionDefinition* funcDef;
  PGSSAObjectMap* objMap;
  // foreach(tie(funcDef, objMap), funcObjMap) {
  // }
  std::cout << "---------------------------" << std::endl;
}
