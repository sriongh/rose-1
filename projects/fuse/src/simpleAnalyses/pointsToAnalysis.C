#include "sage3basic.h"
#include "pointsToAnalysis.h"

using namespace std;
using namespace sight;

namespace fuse
{
  DEBUG_LEVEL(pointsToAnalysisDebugLevel, 2);

  /****************************
   * PointsToAnalysisTransfer *
   ****************************/
  
  PointsToAnalysisTransfer::PointsToAnalysisTransfer(PartPtr part,
                                                     CFGNode cn, NodeState& state,
                                                     std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                                                     Composer* _composer, PointsToAnalysis* _analysis)                                                     
    :DFTransferVisitor(part, cn, state, dfInfo),
     composer(_composer),
     analysis(_analysis),
     modified(false)
  {
    // Set the pointer of the map for this PartEdge
    initLattice();
  }

  bool PointsToAnalysisTransfer::finish()
  {
    return modified;
  }

  void PointsToAnalysisTransfer::initLattice()
  {     
    // Incoming dfInfo is associated with inEdgeFromAny
    assert(dfInfo.size()==1);    
    assert(dfInfo[part->inEdgeFromAny()].size()==1);
    assert(*dfInfo[part->inEdgeFromAny()].begin());

    Lattice *l = *dfInfo[part->inEdgeFromAny()].begin();
    productLattice = (dynamic_cast<AbstractObjectMap*>(l));

    assert(productLattice);
  }

  PointsToAnalysisTransfer::AbstractObjectSetPtr PointsToAnalysisTransfer::getLattice(SgExpression* sgexp) 
  {
    MemLocObjectPtr ml= composer->Expr2MemLoc(sgexp, part->inEdgeFromAny(), analysis);
    return getLatticeCommon(ml);
  }

  
  PointsToAnalysisTransfer::AbstractObjectSetPtr PointsToAnalysisTransfer::getLatticeOperand(SgNode* sgn, SgExpression* operand) 
  { 
    MemLocObjectPtr oml = composer->OperandExpr2MemLoc(sgn, operand, part->inEdgeFromAny(), analysis); 
    return getLatticeCommon(oml);
  }

  PointsToAnalysisTransfer::AbstractObjectSetPtr PointsToAnalysisTransfer::getLatticeCommon(MemLocObjectPtr ml)
  {
    return boost::dynamic_pointer_cast<AbstractObjectSet>(productLattice->get(ml));
  }

  bool PointsToAnalysisTransfer::setLattice(SgExpression* sgexp, PointsToAnalysisTransfer::AbstractObjectSetPtr lat)
  {
    MemLocObjectPtr ml = composer->Expr2MemLoc(sgexp, part->inEdgeFromAny(), analysis);
    return setLatticeCommon(ml, lat);
  }

  bool PointsToAnalysisTransfer::setLatticeOperand(SgNode* sgn, SgExpression* operand, PointsToAnalysisTransfer::AbstractObjectSetPtr lat)                                                   
  {
    MemLocObjectPtr ml = composer->OperandExpr2MemLoc(sgn, operand, part->inEdgeFromAny(), analysis);
    return setLatticeCommon(ml, lat);
  }

  bool PointsToAnalysisTransfer::setLatticeCommon(MemLocObjectPtr ml, PointsToAnalysisTransfer::AbstractObjectSetPtr lat)
  {
    return productLattice->insert(ml, lat);
    if(pointsToAnalysisDebugLevel() >= 3) {
      dbg << productLattice->strp(part->inEdgeFromAny());
    }    
  }

  // NOTE: requires extension for a full blown analysis
  void PointsToAnalysisTransfer::visit(SgAssignOp* sgn)
  {
    SgExpression* rhs_operand = sgn->get_rhs_operand();
    SgExpression* lhs_operand = sgn->get_lhs_operand();

    // Handle p = &x
    // NOTE: rhs can be a complex expression but code below only handles the trivial case
    if(isSgAddressOfOp(rhs_operand)) {
      // Operand of SgAddressOfOp should be a variable
      SgVarRefExp* sgvexp = isSgVarRefExp(isSgAddressOfOp(rhs_operand)->get_operand()); 
      assert(sgvexp);    
      MemLocObjectPtr ml = composer->OperandExpr2MemLoc(sgn, sgvexp, part->inEdgeFromAny(), analysis); 
      assert(ml);

      // Get the AbstractObjectSet (lattice) for lhs_operand
      AbstractObjectSetPtr aos_p = getLatticeOperand(sgn, lhs_operand);

      // Pointer is reset as as result of this assignment
      // Discard previous elements of this set
      if(aos_p->size() > 0) aos_p->setToEmpty();

      // Insert memory object for rhs_operand into this lattice
      aos_p->insert(ml);

      // If a new element is inserted into set, AbstractObjectMap corresponding to lhs needs update.
      // AbstractObjetMap::get only returns a copy of the set.
      // AbstractObjectMap should be updated with the set* methods.     
      modified = setLatticeOperand(sgn, lhs_operand, aos_p) || modified;
    }
    //TODO: handle p = q
    else if(isSgPointerType(lhs_operand->get_type()) &&
            isSgPointerType(rhs_operand->get_type())) {
      AbstractObjectSetPtr laos_p = getLatticeOperand(sgn, lhs_operand); 
      AbstractObjectSetPtr raos_p = getLatticeOperand(sgn, rhs_operand);

      if(pointsToAnalysisDebugLevel() >= 3) {
        dbg << "laos_p=" << laos_p->str() << endl;
        dbg << "raos_p=" << raos_p->str() << endl;
      }

      // Union the information
      // NOTE: points to information can be NULL
      // Merge pointsToSet from rhs if available
      assert(laos_p && raos_p);
      if(raos_p->size() > 0) {
        modified = laos_p->meetUpdate(dynamic_cast<Lattice*>(raos_p.get()));
      }
      
      if(pointsToAnalysisDebugLevel() >= 3) {
        dbg << "modified=" << modified << ", laos_p=" << laos_p->str() << endl;
      }

      // If the set was updated then update the map as well.
      if(modified)
        modified = setLatticeOperand(sgn, lhs_operand, laos_p) || modified;
    }
  }

  void PointsToAnalysisTransfer::visit(SgPointerDerefExp* sgn) {
    scope reg(txt()<<"PointsToAnalysisTransfer::visit(sgn=" << SgNode2Str(sgn) << ")", scope::medium, attrGE("pointsToAnalysisDebugLevel", 2));
    MemLocObjectPtr ml_p = composer->Expr2MemLoc(sgn, part->inEdgeFromAny(), analysis);
    if(pointsToAnalysisDebugLevel() >= 2) {
      dbg << "ML(sgn)=" << ml_p->str() << endl;
    }
  }

  /********************
   * PointsToAnalysis *
   ********************/

  void PointsToAnalysis::genInitLattice(PartPtr part, PartEdgePtr pedge,
                                        std::vector<Lattice*>& initLattices)
  {
    AbstractObjectMap* productlattice = new AbstractObjectMap(boost::make_shared<AbstractObjectSet>(pedge, 
                                                                                                    getComposer(), 
                                                                                                    this, 
                                                                                                    AbstractObjectSet::may),
                                                              pedge,
                                                              getComposer(),
                                                              this);
    initLattices.push_back(productlattice);                                                                                                  
  }


  boost::shared_ptr<DFTransferVisitor>
  PointsToAnalysis::getTransferVisitor(PartPtr part, CFGNode cn, NodeState& state, 
                                       std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)                                     
  {
    PointsToAnalysisTransfer* ptat = new PointsToAnalysisTransfer(part, cn, state, dfInfo, getComposer(), this);
    return boost::shared_ptr<DFTransferVisitor>(ptat);
  }

  std::string PointsToAnalysis::str(std::string indent="") const
  { 
    return "PointsToAnalysis"; 
  }

  // void PointsToAnalysis::copyAbstractObjectSet(const AbstractObjectSet& aos, std::list<MemLocObjectPtr>& list)
  // {
  //   for(AbstractObjectSet::const_iterator it = aos.begin(); it != aos.end(); it++)
  //   {
  //     list.push_back(boost::dynamic_pointer_cast<MemLocObject> (*it));
  //   }
  // }

  // boost::shared_ptr<AbstractObjectSet> 
  // PointsToAnalysis::getPointsToSet(SgNode* sgn, PartEdgePtr pedge, AbstractObjectMap* aom)
  // {
  //   // use the visitor pattern to get pointsToSet for arbitrary sgn
  //   Expr2MemLocTraversal e2mlt(composer, this, pedge, aom);
  //   sgn->accept(e2mlt);
  //   boost::shared_ptr<AbstractObjectSet> aos = e2mlt.getPointsToSet();
  //   return aos;
  // }

  // PointsToMLPtr PointsToAnalysis::Expr2PointsToMLPtr(SgNode* sgn, PartEdgePtr pedge, boost::shared_ptr<AbstractObjectSet> aos)
  // {
  //   // we have points to set for the current MemLocObject
  //   // wrap it up by PointsToML
  //   std::list<MemLocObjectPtr> pointsToSet;
  //   //NOTE: It can be empty if no entry was found in AbstractObjectMap
  //   if(aos.get())
  //   {
  //     // copy the elements of aos into pointsToSet which will be wrapped by PoinsToML
  //     copyAbstractObjectSet(*aos, pointsToSet);
  //   }
  //   else
  //   {
  //     // wrap other memory objects that don't point to any memory object
  //     // push the ml for sgn itself into the pointsToSet as it does not points to any other object
  //     MemLocObjectPtr ml = composer->Expr2MemLoc(sgn, pedge, this);
  //     pointsToSet.push_back(ml);
  //   }
  //   PointsToMLPtr ptmlp = boost::make_shared<PointsToML> (pointsToSet);
  //   return ptmlp;
  // }

  MemLocObjectPtr PointsToAnalysis::Expr2MemLoc(SgNode* sgn, PartEdgePtr pedge)
  {
    scope reg(txt()<<"PointsToAnalysis::Expr2MemLoc(sgn=" << SgNode2Str(sgn) << ")", scope::medium, attrGE("pointsToAnalysisDebugLevel", 2));
    if(pointsToAnalysisDebugLevel()>=2) {
      dbg << "pedge=" << pedge->str() << endl;     
    }

    Lattice* lattice;

    // Incoming information is stored Above for Forward analysis
    if(pedge->target()) {
      NodeState* state = NodeState::getNodeState(this, pedge->target());
      lattice = state->getLatticeAbove(this, pedge, 0);
    }
    // Incoming information is stored Below for Backward analysis
    else if(pedge->source()) {
      NodeState* state = NodeState::getNodeState(this, pedge->source());
      lattice = state->getLatticeBelow(this, pedge, 0);
    }
    else ROSE_ASSERT(0);

    AbstractObjectMap* aom_p = dynamic_cast<AbstractObjectMap*>(lattice);

    if(pointsToAnalysisDebugLevel() >= 2) {
      dbg << "PointsToMap=" << aom_p->str() << endl;
    }

    boost::shared_ptr<PTMemLocObject> ptML_p = boost::make_shared<PTMemLocObject>(pedge, getComposer(), this);

    if(isSgPointerDerefExp(sgn)) {
      SgPointerDerefExp* expr = isSgPointerDerefExp(sgn);
      SgVarRefExp* operand = isSgVarRefExp(expr->get_operand());
      MemLocObjectPtr opML_p = getComposer()->OperandExpr2MemLoc(expr, operand, pedge, this);
      boost::shared_ptr<AbstractObjectSet> aos_p = boost::dynamic_pointer_cast<AbstractObjectSet>(aom_p->get(opML_p));
      assert(aos_p);
      ptML_p->add(aos_p, pedge);
    }
    else {
      MemLocObjectPtr ml_p = getComposer()->Expr2MemLoc(sgn, pedge, this);
      ptML_p->add(ml_p, pedge);
    }

    return ptML_p;
  }

  /******************
   * PTMemLocObject *
   ******************/

  PTMemLocObject::PTMemLocObject(PartEdgePtr pedge, Composer* composer, PointsToAnalysis* ptanalysis)
    : MemLocObject(NULL) { 
    aos_p = boost::make_shared<AbstractObjectSet>(pedge, composer, ptanalysis, AbstractObjectSet::may);
  }

  PTMemLocObject::PTMemLocObject(const PTMemLocObject& thatPTML)
    : MemLocObject(thatPTML) {
    aos_p = boost::make_shared<AbstractObjectSet>(*(thatPTML.aos_p));
  }

  void PTMemLocObject::add(MemLocObjectPtr ml_p, PartEdgePtr pedge) {
    if(ml_p->isFullML(pedge)) {
      // Set the set to full
      aos_p->setToFull();
      return;
    }
    // If the set is already full return
    else if(aos_p->isFullLat()) return;
    // Add the element otherwise
    else aos_p->insert(ml_p);
  }

  void PTMemLocObject::add(boost::shared_ptr<AbstractObjectSet> thataos_p, PartEdgePtr pedge) {
    // Addd elements of the set into this set
    AbstractObjectSet::const_iterator cIt = thataos_p->begin();    
    for( ; cIt != thataos_p->end(); ++cIt) {
      MemLocObjectPtr cItML_p = boost::dynamic_pointer_cast<MemLocObject>(*cIt);
      add(cItML_p, pedge);
    }
  }

  const AbstractObjectSet& PTMemLocObject::getMLSet() const {
    return *(aos_p.get());
  }

  boost::shared_ptr<AbstractObjectSet> PTMemLocObject::getMLSetPtr() const {
    return aos_p;
  }

  Lattice* PTMemLocObject::getMLSetLatticePtr() const {
    return static_cast<Lattice*>(aos_p.get());
  }

  // If the two sets of PTMemLocObject contain overlapping MemLocObjects
  // then the two PTMemLocObjects mayEquals.
  bool PTMemLocObject::mayEqualML(MemLocObjectPtr thatML_p, PartEdgePtr pedge) {
    // scope reg(txt()<<"PTMemLocObject::mayEqualML", scope::medium, attrGE("pointsToAnalysisDebugLevel", 2));
    PTMemLocObjectPtr thatPTML_p = boost::dynamic_pointer_cast<PTMemLocObject>(thatML_p);
    assert(thatPTML_p);

    if(isFullML(pedge) || thatPTML_p->isFullML(pedge)) return true;

    // if(pointsToAnalysisDebugLevel() >= 2) {
    //   dbg << "thisML=" << str() << endl;
    //   dbg << "thatML=" << thatPTML_p->str() << endl;
    // }
    
    // Check if the sets are overlapping.
    const AbstractObjectSet& thatMLSet = thatPTML_p->getMLSet();
    // Iterate over one of the sets
    // Check if any ML from that set mayEquals any element in this set
    AbstractObjectSet::const_iterator cIt = thatMLSet.begin();
    for( ; cIt != thatMLSet.end(); ++cIt) {
      if(aos_p->containsMay(boost::static_pointer_cast<AbstractObject>(*cIt))) return true;
    }
    return false;
  }

  // Two PTMemLocObjects are mustEquals if the sets are singleton and
  // the object in the sets mustEqual each other
  bool PTMemLocObject::mustEqualML(MemLocObjectPtr thatML_p, PartEdgePtr pedge) {
    PTMemLocObjectPtr thatPTML_p = boost::dynamic_pointer_cast<PTMemLocObject>(thatML_p);
    assert(thatPTML_p);

    if(isFullML(pedge) || thatPTML_p->isFullML(pedge)) return false;
    
    const AbstractObjectSet& thatMLSet = thatPTML_p->getMLSet();
    
    // If the sets are not singleton they are not mustEquals.
    if(aos_p->size() != 1 || thatMLSet.size() != 1) return false;

    assert(aos_p->size() == thatMLSet.size() == 1);

    // The object in the set should also mustEqual each other
    const AbstractObjectPtr thatAO_p = boost::static_pointer_cast<AbstractObject>(*thatMLSet.begin());

    return aos_p->containsMust(thatAO_p);
  }

  bool PTMemLocObject::equalSetML(MemLocObjectPtr thatML_p, PartEdgePtr pedge) {
    PTMemLocObjectPtr thatPTML_p = boost::dynamic_pointer_cast<PTMemLocObject>(thatML_p);
    assert(thatPTML_p);

    // If this full then thatML should also be full for the two to be equalSet
    if(isFullML(pedge)) return thatML_p->isFullML(pedge);

    const AbstractObjectSet& thatMLSet = thatPTML_p->getMLSet();

    // If the sets are not of same size then they are not equalSets
    if(aos_p->size() != thatMLSet.size()) return false;

    // Two sets are of same size
    AbstractObjectSet::const_iterator cIt = thatMLSet.begin();
    for( ; cIt != thatMLSet.end(); ++cIt) {
      // If a single element in that set not equals any element in this set
      // then the two MLs are not equalSets.
      if(!aos_p->containsEqualSet(*cIt)) return false;
    }
    
    // Two sets have identical elements.
    return true;
  }

  bool PTMemLocObject::subSetML(MemLocObjectPtr thatML_p, PartEdgePtr pedge) {
    PTMemLocObjectPtr thatPTML_p = boost::dynamic_pointer_cast<PTMemLocObject>(thatML_p);
    assert(thatPTML_p);

    // If this full then thatML should also be full for the two to be equalSet
    if(isFullML(pedge)) return thatML_p->isFullML(pedge);

    const AbstractObjectSet& thatMLSet = thatPTML_p->getMLSet();
    AbstractObjectSet::const_iterator cbegin, cend, cIt;

    // Larger set of the two.
    boost::shared_ptr<AbstractObjectSet> laos_p;

    if(aos_p->size() <= thatMLSet.size()) {
      // aos_p is the smaller set.
      cbegin = aos_p->begin();
      cend = aos_p->end();
      laos_p = thatPTML_p->getMLSetPtr();
    }
    else {
      // thatMLSet is the smaller set
      cbegin = thatMLSet.begin();
      cend = thatMLSet.end();
      laos_p = aos_p;
    }
    assert(laos_p);

    // Iterate on the smaller set.
    // Check if the element equals any element in the larger set.
    for( cIt = cbegin; cIt != cend; ++cIt) {
      // If a single element in the smaller set not equals any element in the larger set
      // then the subSet relation does not hold.
      if(!laos_p->containsEqualSet(*cIt)) return false;
    }
    return true;
  }

  bool PTMemLocObject::isLiveML(PartEdgePtr pedge) {
    if(aos_p->size() == 0) return false;
    AbstractObjectSet::const_iterator cIt = aos_p->begin();
    for( ; cIt != aos_p->end(); ++cIt) {
      MemLocObjectPtr cML_p = boost::dynamic_pointer_cast<MemLocObject>(*cIt);
      if(cML_p->isLiveML(pedge)) return true;
    }
    return false;
  }

  bool PTMemLocObject::meetUpdateML(MemLocObjectPtr thatML_p, PartEdgePtr pedge) {
    PTMemLocObjectPtr thatPTML_p = boost::dynamic_pointer_cast<PTMemLocObject>(thatML_p);
    assert(thatPTML_p);

    if(isFullML(pedge)) return false;
    
    Lattice* thatMLSetLatPtr = thatPTML_p->getMLSetLatticePtr();
    return aos_p->meetUpdate(thatMLSetLatPtr);
  }

  bool PTMemLocObject::isFullML(PartEdgePtr pedge) {
    return aos_p->isFullLat();
  }

  bool PTMemLocObject::isEmptyML(PartEdgePtr pedge) {
    return aos_p->isEmptyLat();
  }

  MemLocObjectPtr PTMemLocObject::copyML() const {
    return boost::make_shared<PTMemLocObject>(*this);
  }

  MemLocObject* PTMemLocObject::copyMLPtr() const {
    return boost::make_shared<PTMemLocObject>(*this).get();
  }

  string PTMemLocObject::str(string indent) const {
    ostringstream oss;
    oss << "[PointsToML:" << aos_p->str(indent) << "]\n";
    return oss.str();
  }


  /************************
   * Expr2MemLocTraversal *
   ************************/

  void Expr2MemLocTraversal::visit(SgPointerDerefExp* sgn)
  {
    scope regvis("Expr2MemLocTraversal::visit(SgPointerDerefExp* sgn)", scope::medium, attrGE("pointsToAnalysisDebugLevel", 1));
    SgExpression* operand = sgn->get_operand();
    operand->accept(*this);
    boost::shared_ptr<AbstractObjectSet> new_p_aos = 
            boost::make_shared<AbstractObjectSet>(pedge, composer, analysis, AbstractObjectSet::may);
    for(AbstractObjectSet::const_iterator i=p_aos->begin(); i!=p_aos->end(); i++) {
      boost::shared_ptr<AbstractObjectSet> ao = boost::dynamic_pointer_cast<AbstractObjectSet>(aom->get(*i));
      assert(ao);
      new_p_aos->meetUpdate(ao.get());
    }
    p_aos = new_p_aos;
  }

  void Expr2MemLocTraversal::visit(SgVarRefExp* sgn)
  {
    scope regvis("Expr2MemLocTraversal::visit(SgVarRefExp* sgn)", scope::medium, attrGE("pointsToAnalysisDebugLevel", 1));
    dbg << "isSgPointerType(sgn->get_type())="<<isSgPointerType(sgn->get_type())<<endl;
    // return points to set only for pointer types
    /*if(isSgPointerType(sgn->get_type()))
    {
      MemLocObjectPtr ml = composer->Expr2MemLoc(sgn, pedge, analysis);
      p_aos = boost::dynamic_pointer_cast<AbstractObjectSet>(aom->get(ml));
    }*/
    p_aos = boost::make_shared<AbstractObjectSet>(pedge, composer, analysis, AbstractObjectSet::may);
    p_aos->insert(composer->Expr2MemLoc(sgn, pedge, analysis));
    if(pointsToAnalysisDebugLevel()>=1) dbg << "p_aos="<<p_aos->str()<<endl;
  }

  void Expr2MemLocTraversal::visit(SgAssignOp* sgn)
  {
    // handle p = q where p, q are pointer types    
  }
};
