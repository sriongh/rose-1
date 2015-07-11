#include "sage3basic.h"
#include "pointsToAnalysis.h"

using namespace std;
using namespace sight;

namespace fuse
{
  DEBUG_LEVEL(pointsToAnalysisDebugLevel, 0);

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
    // Incoming dfInfo is associated with inEdgeFromAny
    assert(dfInfo.size()==1);    
    assert(dfInfo[part->inEdgeFromAny()].size()==1);
    assert(*dfInfo[part->inEdgeFromAny()].begin());

    Lattice *l = *dfInfo[part->inEdgeFromAny()].begin();
    latticeMap = (dynamic_cast<AbstractObjectMap*>(l));

    assert(latticeMap);
  }

  bool PointsToAnalysisTransfer::finish()
  {
    return modified;
  }

  LatticePtr PointsToAnalysisTransfer::getLattice(MemLocObjectPtr ml) {
    LatticePtr lat = latticeMap->get(ml);
    assert(lat);
    return lat;
  }

  bool PointsToAnalysisTransfer::setLattice(MemLocObjectPtr ml, LatticePtr lat) {
    assert(lat);
    return latticeMap->insert(ml, lat);
  }

  bool PointsToAnalysisTransfer::updateLatticeMap(PointsToRelation& prel) {    
    return setLattice(prel.first, prel.second);
  }

  PointsToRelation PointsToAnalysisTransfer::make_pointsto(MemLocObjectPtr key,
                                                           boost::shared_ptr<AbstractObjectSet> latticeElem) {
    assert(key && latticeElem);
    return std::make_pair(key, latticeElem);
  }
  
  // PointsTo LatticeMap key is a MemLocObjectPtr
  // Given an pointer type expression return the MemLocObjectPtr to be used as key in LatticeMap
  MemLocObjectPtr PointsToAnalysisTransfer::getLatticeMapKey(SgExpression* anchor, SgExpression* operand, PartEdgePtr pedge) {
    //operand is of pointer type
    assert(operand->get_type()->variantT() == V_SgPointerType);
    MemLocObjectPtr ml;

    // SgVarRefExp, SgDotExp do not require any processing
    // Array operandessions may require additional processing from other analyses
    // Pointer dereferencing operandessions requires additional LatticeMap lookup.
    // example *p = &x and p -> q where MemLocObject(*p) = MemLocObject(q)
    switch(operand->variantT()) {      
    case V_SgVarRefExp:
    case V_SgDotExp:
      ml = composer->OperandExpr2MemLoc(anchor, operand, pedge, analysis);
      break;    
    case V_SgPntrArrRefExp:
    case V_SgPointerDerefExp:
    default: 
      dbg << "Unhandled operand in PointsToAnalysisTransfer::getLatticeMapKey(operand=" << SgNode2Str(operand) << ")\n";
      assert(false);
    };
    return ml;
  }

  boost::shared_ptr<AbstractObjectSet>
  PointsToAnalysisTransfer::getLatticeElem(MemLocObjectPtr key) {
    boost::shared_ptr<AbstractObjectSet> lat = boost::dynamic_pointer_cast<AbstractObjectSet>(getLattice(key));
    assert(lat);
    return lat;
  }

  boost::shared_ptr<AbstractObjectSet>
  PointsToAnalysisTransfer::getLatticeElem(SgExpression* anchor, SgExpression* operand, PartEdgePtr pedge) {
    MemLocObjectPtr keyML;
    switch(operand->variantT()) {
    case V_SgVarRefExp:
    case V_SgDotExp: {
      // operand is a pointer variable
      keyML = composer->OperandExpr2MemLoc(anchor, operand, pedge, analysis);      
      break;
    }
    case V_SgPointerDerefExp:
    case V_SgPntrArrRefExp:
    default: {
      dbg << "Unhandled operand in PointsToAnalysisTransfer::getLatticeElem(operand=" << SgNode2Str(operand) << ")\n";
      assert(false);
    }
    };
    
    assert(keyML);    
    return getLatticeElem(keyML);
  }

  // an expression of pointer type can get assigned in many ways
  // p = &expr where expr: SgVarRefExp | SgDotExp | SgPntrArrRefExp | SgFunctionRefExp
  // p = q where q: another pointer variable
  // *p = expr where expr is SgPointerType expression
  void PointsToAnalysisTransfer::visit(SgAssignOp* sgn) {
    SgExpression* lexpr = sgn->get_lhs_operand();
    SgExpression* rexpr = sgn->get_rhs_operand();

    // both expressions are of pointer types
    if(isSgPointerType(lexpr->get_type()) &&
       isSgPointerType(rexpr->get_type())) {
      MemLocObjectPtr keyML = getLatticeMapKey(sgn, lexpr, part->inEdgeFromAny());
      PointsToRelation prel;
      switch(rexpr->variantT()) {
      case V_SgAddressOfOp: {
        boost::shared_ptr<AbstractObjectSet> lat = getLatticeElem(sgn, lexpr, part->inEdgeFromAny());
        lat->setToEmpty();
        MemLocObjectPtr toML = composer->OperandExpr2MemLoc(sgn, isSgAddressOfOp(rexpr)->get_operand(),
                                                            part->inEdgeFromAny(), analysis);
        lat->insert(toML);
        prel = make_pointsto(keyML, lat);
        break;
      }
      case V_SgVarRefExp: {
        boost::shared_ptr<AbstractObjectSet> llat = getLatticeElem(keyML);
        llat->setToEmpty();
        boost::shared_ptr<AbstractObjectSet> rlat = getLatticeElem(sgn, rexpr, part->inEdgeFromAny());
        llat->copy(rlat.get());
        prel = make_pointsto(keyML, llat);
      }
      default:
        assert(false);
      };
      modified = updateLatticeMap(prel);
    }
    modified = false;
  }

  void PointsToAnalysisTransfer::visit(SgFunctionCallExp* sgn) {
    if(Part::isOutgoingFuncCall(cn)) {
      SgExprListExp* args = sgn->get_args();
      SgExpressionPtrList& argsList = args->get_expressions();
      SgExpressionPtrList::iterator a = argsList.begin();
      for(int i=0 ; a != argsList.end(); ++a, ++i) {
        SgExpression* arg = *a;
        // If its a cast expression strip the cast
        if(isSgCastExp(arg)) {
          arg = isSgCastExp(arg)->get_operand();
        }
        ROSE_ASSERT(!isSgCastExp(arg));
        if(isSgAddressOfOp(arg)) {
          SgType* argType = arg->get_type();
          dbg << "argtype=" << SgNode2Str(argType) << endl;
        }
      }
    }
  }

  void PointsToAnalysisTransfer::visit(SgPointerDerefExp* sgn) {
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

  MemLocObjectPtr PointsToAnalysis::Expr2MemLoc(SgNode* sgn, PartEdgePtr pedge)
  {
    scope reg(txt()<<"PointsToAnalysis::Expr2MemLoc(sgn=" << SgNode2Str(sgn) << ")", scope::medium, attrGE("pointsToAnalysisDebugLevel", 2));
    if(pointsToAnalysisDebugLevel()>=2) {
      dbg << "pedge=" << pedge->str() << endl;     
    }

    // ML object returned by Pointsto analysis
    boost::shared_ptr<PTMemLocObject> ptML_p = boost::make_shared<PTMemLocObject>(pedge, getComposer(), this);

    // Switch based on the type of the expression
    switch(sgn->variantT()) {
    // Handle all pointer dereference expressions by looking up analysis state
    case V_SgPointerDerefExp: {
      MemLocObjectPtr opML_p = getComposer()->OperandExpr2MemLoc(sgn, isSgPointerDerefExp(sgn)->get_operand(), pedge, this);

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

      assert(lattice);
      AbstractObjectMap* aom_p = dynamic_cast<AbstractObjectMap*>(lattice);

      if(pointsToAnalysisDebugLevel() >= 2) {
        dbg << "PointsToMap=" << aom_p->str() << endl;
      }

      boost::shared_ptr<AbstractObjectSet> aos_p = boost::dynamic_pointer_cast<AbstractObjectSet>(aom_p->get(opML_p));
      assert(!aos_p->isEmptyLat());
      if(pointsToAnalysisDebugLevel() >= 2) dbg << "MLSet=" << aos_p->str() << endl;
      ptML_p->add(aos_p, pedge);
      break;
    }

    // For all other cases forward the query back to the composer
    case V_SgVarRefExp:
    case V_SgInitializedName:
    default:
      MemLocObjectPtr ml_p = getComposer()->Expr2MemLoc(sgn, pedge, this);
      ptML_p->add(ml_p, pedge);
      break;      
    };     

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

  // void Expr2MemLocTraversal::visit(SgPointerDerefExp* sgn)
  // {
  //   scope regvis("Expr2MemLocTraversal::visit(SgPointerDerefExp* sgn)", scope::medium, attrGE("pointsToAnalysisDebugLevel", 1));
  //   SgExpression* operand = sgn->get_operand();
  //   operand->accept(*this);
  //   boost::shared_ptr<AbstractObjectSet> new_p_aos = 
  //           boost::make_shared<AbstractObjectSet>(pedge, composer, analysis, AbstractObjectSet::may);
  //   for(AbstractObjectSet::const_iterator i=p_aos->begin(); i!=p_aos->end(); i++) {
  //     boost::shared_ptr<AbstractObjectSet> ao = boost::dynamic_pointer_cast<AbstractObjectSet>(aom->get(*i));
  //     assert(ao);
  //     new_p_aos->meetUpdate(ao.get());
  //   }
  //   p_aos = new_p_aos;
  // }

  // void Expr2MemLocTraversal::visit(SgVarRefExp* sgn)
  // {
  //   scope regvis("Expr2MemLocTraversal::visit(SgVarRefExp* sgn)", scope::medium, attrGE("pointsToAnalysisDebugLevel", 1));
  //   dbg << "isSgPointerType(sgn->get_type())="<<isSgPointerType(sgn->get_type())<<endl;
  //   // return points to set only for pointer types
  //   /*if(isSgPointerType(sgn->get_type()))
  //   {
  //     MemLocObjectPtr ml = composer->Expr2MemLoc(sgn, pedge, analysis);
  //     p_aos = boost::dynamic_pointer_cast<AbstractObjectSet>(aom->get(ml));
  //   }*/
  //   p_aos = boost::make_shared<AbstractObjectSet>(pedge, composer, analysis, AbstractObjectSet::may);
  //   p_aos->insert(composer->Expr2MemLoc(sgn, pedge, analysis));
  //   if(pointsToAnalysisDebugLevel()>=1) dbg << "p_aos="<<p_aos->str()<<endl;
  // }

  // void Expr2MemLocTraversal::visit(SgAssignOp* sgn)
  // {
  //   // handle p = q where p, q are pointer types    
  // }
};
