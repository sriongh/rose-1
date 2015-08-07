/*****************************************
 * Author: Sriram Aananthakrishnan, 2013 *
 *****************************************/

#include "sage3basic.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "address_taken_analysis.h"

namespace fuse {

  DEBUG_LEVEL(addressTakenAnalysisDebugLevel, 2);

  /*******************
   * VariableIdUtils *
   *******************/
  void set_union(const VariableIdSet& set1, const VariableIdSet& set2, VariableIdSet& rset)
  {
    VariableIdSet::const_iterator it1 = set1.begin();
    VariableIdSet::const_iterator it2 = set2.begin();
    VariableIdSet::iterator rit = rset.begin();
  
    // // re-implementation of set-union
    while(true)
      {
        if(it1 == set1.end())
          {
            rset.insert(it2, set2.end());
            break;
          }
        if(it2 == set2.end())
          {
            rset.insert(it1, set1.end());
            break;
          }
    
        if(*it1 < *it2)
          {
            rset.insert(rit, *it1); ++it1; ++rit;
          }
        else if(*it2 < *it1)
          {
            rset.insert(rit, *it2); ++it2; ++rit;
          }
        else
          {
            rset.insert(rit, *it1); ++it1; ++it2; ++rit;
          }
      }
  }

  /*************************************************
   ***************** ProcessQuery  *****************
   *************************************************/

  MatchResult& ProcessQuery::getMatchResult()
  {
    return match_result;
  }

  void ProcessQuery::printMatchResult()
  {
    // MatchResult is list of maps
    // each map corresponds to one particular match instance
    for(MatchResult::iterator it = match_result.begin(); it != match_result.end(); it++)
      {
        for(SingleMatchVarBindings::iterator smbIt = (*it).begin(); smbIt != (*it).end(); smbIt++)
          {
            std::cout << "MATCH=";
            SgNode* matchedTerm = (*smbIt).second;
            ROSE_ASSERT(matchedTerm != NULL);
            // std::cout << "  VAR: " << (*smbIt).first << "=" << 
            // astTermWithNullValuesToString(matchedTerm) << " @" << matchedTerm << std::endl;
          }
      }
  }

  void ProcessQuery::clearMatchResult()
  {
    match_result.clear();
  }

  MatchResult& ProcessQuery::operator()(std::string query, SgNode* root)
  {
    AstMatching m;
    match_result = m.performMatching(query, root);
    return match_result;
  }

  /*************************************************
   ************* ComputeAddressTakenInfo  **********
   *************************************************/


  void ComputeAddressTakenInfo::OperandToVariableId::debugPrint(SgNode* sgn)
  {
    dbg << sgn->class_name() << ": " << SgNode2Str(sgn) << ", " \
    << sgn->unparseToString() << ", " \
    << sgn->get_file_info()->get_filenameString() << ", " \
    << sgn->get_file_info()->get_line() << ", " \
    << endl;
  }

  // base case for the recursion
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgVarRefExp *sgn)
  { 
    if(debuglevel > 0) debugPrint(sgn);
    VariableId id = cati.vidm.variableId(sgn);
    ROSE_ASSERT(id.isValid());
    // insert the id into VariableIdSet
    cati.addressTakenSet.insert(id);
  }

  // only the rhs_op of SgDotExp is modified
  // recurse on rhs_op
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgDotExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    SgNode* rhs_op = sgn->get_rhs_operand();
    rhs_op->accept(*this);
  }

  // only the rhs_op of SgDotExp is modified
  // recurse on rhs_op
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgArrowExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    SgNode* rhs_op = sgn->get_rhs_operand();
    rhs_op->accept(*this);
  }

  // For example q = &(*p) where both q and p are pointer types
  // In the example, q can potentially modify all variables pointed to by p
  // same as writing q = p.
  // Since we dont know anything about p, q can potentially modify all 
  // the elements in addressTakenSet as result of the above expression 
  // As a result, the variables whose addresses can be taken is the entire 
  // set as we dont have any idea about p
  // We dont need to add any new variable to addressTakenSet 
  // as a consequence of the expressions similar to above.
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgPointerDerefExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    // we raise a flag
    cati.sound = false;
  }

  // For example &(A[B[C[..]]]) or &(A[x][x][x])
  // any pointer can that takes this address can modify
  // contents of only A. The inner index expressions are r-values
  // it is sufficient to add A to addressTakenSet
  // keep recursing on the lhs until we find A
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgPntrArrRefExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    // collect the info only if its array type
    // otherwise some pointer arithmetic is going on to compute the address of the array
    SgNode* arr_op = sgn->get_lhs_operand();
    if(isSgArrayType(arr_op)) {    
      arr_op->accept(*this);
    }
    else {
      // raise the flag as we dont know whose address is taken
      cati.sound = false;
    }
  }

  // &(a = expr)
  // a's address is taken here
  // process the lhs recursively
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgAssignOp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    SgNode* lhs_op = sgn->get_lhs_operand();
    lhs_op->accept(*this);
  }

  // &(++i)
  // prefix increments first and the result can be used as lvalue (in C++)
  // postfix uses the operand as lvalue and increments later and therefore
  // postfix increment cannot be lvalue
  // both prefix/postfix are illegal in C
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgPlusPlusOp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    SgNode* operand = sgn->get_operand();
    operand->accept(*this);
  }

  // same as prefix increment
  // &(--i)
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgMinusMinusOp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    SgNode* operand = sgn->get_operand();
    operand->accept(*this);
  }

  // &(a+1, b)
  // b's address is taken
  // keep recursing on the rhs_op
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgCommaOpExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    SgNode* rhs_op = sgn->get_rhs_operand();
    rhs_op->accept(*this);
  }

  // if we see SgConditionalExp as operand of &
  // both true and false branch are lvalues
  // recurse on both of them to pick up the lvalues
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgConditionalExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    SgNode* true_exp = sgn->get_true_exp();
    SgNode* false_exp = sgn->get_false_exp();
    true_exp->accept(*this);
    false_exp->accept(*this);
  }

  // cast can be lvalue
  // example :  &((struct _Rep *)((this) -> _M_data()))[-1] expr from stl_list
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgCastExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    SgNode* operand = sgn->get_operand();
    operand->accept(*this);
  }

  // void f() { }
  // void (*f_ptr)() = &f;
  // & on SgFunctionRefExp is redundant as the functions
  // are implicity converted to function pointer
  //
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgFunctionRefExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    // raise the flag
    cati.sound = false;
  }

  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgMemberFunctionRefExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    // raise the flag
    // functions can potentially modify anything
    cati.sound = false;
  }

  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgTemplateFunctionRefExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    // raise the flag
    // functions can potentially modify anything
    cati.sound = false;
  }
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgTemplateMemberFunctionRefExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    // raise the flag
    // functions can potentially modify anything
    cati.sound = false;
  }

  // A& foo() { return A(); }
  // &(foo())
  // if foo() returns a reference then foo() returns a lvalue
  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgFunctionCallExp* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    // we can look at its defintion and process the return expression ?
    // function calls can modify anything
    // raise the flag more analysis required
    cati.sound = false;
  }

  void ComputeAddressTakenInfo::OperandToVariableId::visit(SgNode* sgn)
  {
    if(debuglevel > 0) debugPrint(sgn);
    std::cerr << "unhandled operand " << sgn->class_name() << " of SgAddressOfOp in AddressTakenAnalysis\n";
    std::cerr << sgn->unparseToString() << std::endl;
    ROSE_ASSERT(0);
  }

  void ComputeAddressTakenInfo::computeAddressTakenInfo(SgNode* root)
  {
    // query to match all SgAddressOfOp subtrees
    // process query
    ProcessQuery collectSgAddressOfOp;
    // TODO: not sufficient to pick up address taken by function pointers
    std::string matchquery;

    // "#SgTemplateArgument|"
    // "#SgTemplateArgumentList|"
    // "#SgTemplateParameter|"
    // "#SgTemplateParameterVal|"
    // "#SgTemplateParamterList|"
  
    // skipping all template declaration specific nodes as they dont have any symbols
    // we still traverse SgTemplateInstatiation*
    matchquery = \
      "#SgTemplateClassDeclaration|"\
      "#SgTemplateFunctionDeclaration|"\
      "#SgTemplateMemberFunctionDeclaration|"\
      "#SgTemplateVariableDeclaration|" \
      "#SgTemplateClassDefinition|"\
      "#SgTemplateFunctionDefinition|"\
      "$HEAD=SgAddressOfOp($OP)";
    
    MatchResult& matches = collectSgAddressOfOp(matchquery, root);
    for(MatchResult::iterator it = matches.begin(); it != matches.end(); ++it) {
      SgNode* matchedOperand = (*it)["$OP"];
      // SgNode* head = (*it)["$HEAD"];
      // debugPrint(head); debugPrint(matchedOperand);
      OperandToVariableId optovid(*this);
      matchedOperand->accept(optovid);
    }              
  }

  VariableIdSet ComputeAddressTakenInfo::getAddressTakenSet() {
    return addressTakenSet;
  }

  bool ComputeAddressTakenInfo::isSound() {
    return sound;
  }

  /*************************************************
   **************** TypeAnalysis *******************
   *************************************************/

  VariableIdSet CollectTypeInfo::getPointerTypeSet()
  {
    return pointerTypeSet;
  }

  VariableIdSet CollectTypeInfo::getArrayTypeSet()
  {
    return arrayTypeSet;
  }

  VariableIdSet CollectTypeInfo::getReferenceTypeSet()
  {
    return referenceTypeSet;
  }

  void CollectTypeInfo::collectTypes()
  {
    if(varsUsed.size() == 0) {
      // get the entire set from VariableIdMapping
      varsUsed = vidm.getVariableIdSet();
    }

    for(VariableIdSet::iterator it = varsUsed.begin(); it != varsUsed.end(); ++it)
      {
        SgSymbol* v_symbol = vidm.getSymbol(*it);
        SgType* v_type = v_symbol->get_type();
        // Note on function pointer types
        // function pointer can modify any variable
        // not just the variables in the addressTakenSet
        // answering function pointer derefence requires side
        // effect analysis to determine the list of variables
        // can modify. Currenty we ignore function pointers as
        // the goal of this analysis is supposed to be simple.
        if(isSgPointerType(v_type))
          {
            SgType* baseType = v_type->findBaseType();
            // perhaps its worthwile to keep them in
            // a separte set and not support any dereferencing
            // queries rather than not adding them
            if(!isSgFunctionType(baseType))
              {
                pointerTypeSet.insert(*it);
              }
          }
        else if(isSgArrayType(v_type))
          {
            arrayTypeSet.insert(*it);
          }
        else if(isSgReferenceType(v_type))
          {
            referenceTypeSet.insert(*it);
          }
      }
  }

  /**********************************
   * FlowInSensAddrTakenAnalysis *
   **********************************/
  FlowInSensAddrTakenAnalysis::FlowInSensAddrTakenAnalysis(SgProject* root)
    : root(root), sound(true) {
    vidm_p = boost::make_shared<VariableIdMapping>();
    vidm_p->computeVariableSymbolMapping(root);
  }

  FlowInSensAddrTakenAnalysis::FlowInSensAddrTakenAnalysis(const FlowInSensAddrTakenAnalysis& that)
    : root(that.root), vidm_p(that.vidm_p), 
      addressTakenSet(that.addressTakenSet),
      pointerTypeSet(that.pointerTypeSet),
      arrayTypeSet(that.arrayTypeSet),
      referenceTypeSet(that.referenceTypeSet) { }

  ComposedAnalysisPtr FlowInSensAddrTakenAnalysis::copy() {
    return boost::make_shared<FlowInSensAddrTakenAnalysis>(*this);
  }

  void FlowInSensAddrTakenAnalysis::runAnalysis() {
    scope reg("FlowInSensAddrTakenAnalysis::runAnalysis()", scope::medium, attrGE("addressTakenAnalysisDebugLevel", 2));

    ComputeAddressTakenInfo cati(*vidm_p);
    cati.computeAddressTakenInfo(isSgNode(root));
    addressTakenSet = cati.getAddressTakenSet();
    sound = cati.isSound();

    CollectTypeInfo collectTypeInfo(*vidm_p);
    collectTypeInfo.collectTypes();
  
    pointerTypeSet = collectTypeInfo.getPointerTypeSet();
    arrayTypeSet = collectTypeInfo.getArrayTypeSet();
    referenceTypeSet = collectTypeInfo.getReferenceTypeSet();

    set_union(addressTakenSet, arrayTypeSet, aliasingSet);
  }

  VariableIdMappingPtr FlowInSensAddrTakenAnalysis::getVariableIdMapping() const {
    return vidm_p;
  }

  FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::Expr2MemRegionCreate(FlowInSensAddrTakenAnalysis& addrTakenAnalysis,
                                                                          Composer* composer,
                                                                          PartEdgePtr pedge)
    : addrTakenAnalysis(addrTakenAnalysis), composer(composer), pedge(pedge) { }

  bool FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::contains(const VariableIdSet& vidSet, VariableId id) const {
    VariableIdSet::const_iterator cIt;
    cIt = vidSet.find(id);
    return (cIt != vidSet.end());
  }

  ATAnalMRTypePtr FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::getATAnalMRType() const {
    return type;
  }

  void FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::createATAnalNamedMRType(VariableId id) {
    assert(id.isValid());
    const VariableIdSet& aliasingSet = addrTakenAnalysis.aliasingSet;
    
    if(contains(aliasingSet, id)) {
      type = boost::make_shared<ATAnalNamedMRType>(ATAnalMRType::named, id, true, addrTakenAnalysis.vidm_p);
    }
    else {
      type = boost::make_shared<ATAnalNamedMRType>(ATAnalMRType::named, id, false, addrTakenAnalysis.vidm_p);
    }
    
    assert(type);
  }

  void FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::visit(SgVarRefExp* sgn) {
    VariableId id = addrTakenAnalysis.vidm_p->variableId(sgn);
    createATAnalNamedMRType(id);
  }

  void FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::visit(SgInitializedName* sgn) {
    VariableId id = addrTakenAnalysis.vidm_p->variableId(sgn);
    createATAnalNamedMRType(id);
  }

  void FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::visit(SgDotExp* sgn) {
    assert(false);
  }

  void FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::visit(SgArrowExp* sgn) {
    assert(false);
  }

  void FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::visit(SgPointerDerefExp* sgn) {
    type = boost::make_shared<ATAnalAliasingMRType>(ATAnalMRType::aliasing, addrTakenAnalysis.aliasingSet, addrTakenAnalysis.vidm_p);
    assert(type);
  }

  void FlowInSensAddrTakenAnalysis::Expr2MemRegionCreate::visit(SgExpression* sgn) {
    MemRegionObjectPtr parent = composer->Expr2MemRegion(sgn, pedge, &addrTakenAnalysis);
    type = boost::make_shared<ATAnalExprMRType>(ATAnalMRType::expr, parent, sgn);
  }

  MemRegionObjectPtr FlowInSensAddrTakenAnalysis::Expr2MemRegion(SgNode* sgn, PartEdgePtr pedge) {
    Expr2MemRegionCreate createMemRegion(*this, getComposer(), pedge);
    sgn->accept(createMemRegion);
    ATAnalMRTypePtr type = createMemRegion.getATAnalMRType();
    MemRegionObjectPtr parent = getComposer()->Expr2MemRegion(sgn, pedge, this);
    return boost::make_shared<FlowInSensATAnalMR>(sgn, type, parent);
  }

  MemLocObjectPtr FlowInSensAddrTakenAnalysis::Expr2MemLoc(SgNode* node, PartEdgePtr pedge) {
    return boost::make_shared<FullMemLocObject>();
  }

  string FlowInSensAddrTakenAnalysis::str(string indent) const {
    return "FlowInSensAddrTakenAnalysis";
  }


  /****************
   * ATAnalMRType *
   ****************/
  ATAnalMRType::ATAnalMRType(MRType type) : type(type) { }
  
  ATAnalMRType::ATAnalMRType(const ATAnalMRType& that) : type(that.type)  { }
  
  ATAnalMRType::MRType ATAnalMRType::getType() const {
    return type;
  }

  /*********************
   * ATAnalNamedMRType *
   *********************/
  ATAnalNamedMRType::ATAnalNamedMRType(MRType type, VariableId id, bool addrtaken, VariableIdMappingPtr vidm_p)
    : ATAnalMRType(type), id(id), addrtaken(addrtaken), vidm_p(vidm_p) { }

  ATAnalNamedMRType::ATAnalNamedMRType(const ATAnalNamedMRType& that)
    : ATAnalMRType(that), id(that.id), addrtaken(that.addrtaken), vidm_p(that.vidm_p) { }

  ATAnalMRTypePtr ATAnalNamedMRType::copyATAnalMRType() const {
    return boost::make_shared<ATAnalNamedMRType>(*this);
  }

  VariableId ATAnalNamedMRType::getId() const {
    return id;
  }

  bool ATAnalNamedMRType::mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    if(that->getType() == ATAnalMRType::unknown) return true;
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      return atype->contains(id);
    }
    // if that is a named type
    else if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return id == ntype->getId();
    }
    // that is expr type or
    return false;
  }
  
  bool ATAnalNamedMRType::mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      return (atype->contains(id) && atype->singleton());
    }
    else if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return id == ntype->getId();
    }
    return false;
  }

  bool ATAnalNamedMRType::equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return id == ntype->getId();
    }
    // if that is a pointer type
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      return (atype->contains(id) && atype->singleton());
    }
    // if that is a unknown type or expr type
    else return false;
  }

  bool ATAnalNamedMRType::subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) { 
    if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return id == ntype->getId();
    }
    // if that is a pointer type
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      return atype->contains(id);
    }
    // if that is unknown type
    else if(that->getType() == ATAnalMRType::unknown) return true;
    // if that is expr type
    else return false;
  }

  bool ATAnalNamedMRType::isFullMRType(PartEdgePtr pedge) {
    return false;
  }

  string ATAnalNamedMRType::str(string indent) const{
    ostringstream oss;
    oss << "named: " << vidm_p->variableName(id);
    return oss.str();
  }

  /************************
   * ATAnalAliasingMRType *
   ************************/
  ATAnalAliasingMRType::ATAnalAliasingMRType(MRType type, VariableIdSet aliasingSet, VariableIdMappingPtr vidm_p)
    : ATAnalMRType(type), aliasingSet(aliasingSet), vidm_p(vidm_p) { }

  ATAnalAliasingMRType::ATAnalAliasingMRType(const ATAnalAliasingMRType& that)
    : ATAnalMRType(that), aliasingSet(that.aliasingSet), vidm_p(vidm_p) { }

  ATAnalMRTypePtr ATAnalAliasingMRType::copyATAnalMRType() const {
    return boost::make_shared<ATAnalAliasingMRType>(*this);
  }

  const VariableIdSet& ATAnalAliasingMRType::getAliasingSet() const {
    return aliasingSet;
  }

  bool ATAnalAliasingMRType::contains(VariableId id) const {
    if(aliasingSet.find(id) != aliasingSet.end()) return true;
    return false;
  }

  bool ATAnalAliasingMRType::singleton() const {
    return (aliasingSet.size() == 1);
  }

  bool ATAnalAliasingMRType::set_intersect(ATAnalAliasingMRTypePtr that) const {
    VariableIdSet::const_iterator thisI = aliasingSet.begin();
    // if we find even one intersecting element return true
    for( ; thisI != aliasingSet.end(); ++thisI) {
      if(that->contains(*thisI)) return true;
    }
    return false;
  }

  bool ATAnalAliasingMRType::set_equal(ATAnalAliasingMRTypePtr that) const {
    const VariableIdSet& thatAliasingSet = that->getAliasingSet();

    if(aliasingSet.size() != thatAliasingSet.size()) return false;
    VariableIdSet::const_iterator thisI, thatI;
    for(thisI=aliasingSet.begin(), thatI=thatAliasingSet.begin();
        thisI != aliasingSet.end() && thatI != thatAliasingSet.end();
        ++thisI, ++thatI) {
      if(*thisI != *thatI) return false;
    }
    return true;
  }

  bool ATAnalAliasingMRType::set_subset(ATAnalAliasingMRTypePtr that) const {
    const VariableIdSet& thatAliasingSet = that->getAliasingSet();

    if(aliasingSet.size() > thatAliasingSet.size()) return false;
    VariableIdSet::const_iterator thisI = aliasingSet.begin();
    for( ; thisI != aliasingSet.end(); ++thisI) {
      if(!that->contains(*thisI)) return false;
    }
    return true;    
  }

  bool ATAnalAliasingMRType::mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    // AliasingType are not equal to temporary expr 
    if(that->getType() == ATAnalMRType::expr) return false;
    // If that is unknown type
    else if(that->getType() == ATAnalMRType::unknown) return true;
    // If named type check if VariableId is present in this set
    else if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return contains(ntype->getId());
    }
    // If both are aliasing type
    // Check for set intersection
    //! NOTE: Instead of having identical set can we just keep one copy of the set?
    else {
      ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that);
      assert(atype);
      return set_intersect(atype); 
    }
  }

  bool ATAnalAliasingMRType::mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    // Does not must equals expr or unknown type
    if(that->getType() == ATAnalMRType::expr ||
       that->getType() == ATAnalMRType::unknown) return false;
    // both are aliasing type
    // check they are equal and singleton
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      return singleton() && set_equal(atype);
    }
    // same as above
    else {
      ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that);
      assert(ntype);
      return singleton() && contains(ntype->getId());
    }
  }

  bool ATAnalAliasingMRType::equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    // not equal sets with expr or unknown type
    if(that->getType() == ATAnalMRType::expr ||
       that->getType() == ATAnalMRType::unknown) return false;
    // check for set equality
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that))
      return set_equal(atype);
    // check if singleton and contains for named type
    else {
      ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that);
      assert(ntype);
      return singleton() && contains(ntype->getId());
    }
  }

  bool ATAnalAliasingMRType::subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    // not a subset of expr type
    if(that->getType() == ATAnalMRType::expr) return false;
    // all types are subset of unknown
    else if(that->getType() == ATAnalMRType::unknown) return true;
    // check for set_subset
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      return set_subset(atype);
    }
    // only case that this is a subset of named type is when they are equal
    else {
      ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that);
      assert(ntype);
      return singleton() && contains(ntype->getId());
    }
  }

  bool ATAnalAliasingMRType::isFullMRType(PartEdgePtr pedge) {
    return false;
  }

  string ATAnalAliasingMRType::str(string indent) const {
    ostringstream oss;
    oss << "aliasing: {";
    VariableIdSet::const_iterator cit = aliasingSet.begin();
    for( ; cit != aliasingSet.end(); ) {
      oss << vidm_p->variableName(*cit);
      ++cit;
      if(cit != aliasingSet.end()) oss << ", ";
    }
    oss << "}";
    return oss.str();
  }

  /********************
   * ATAnalExprMRType *
   ********************/
  ATAnalExprMRType::ATAnalExprMRType(MRType type, MemRegionObjectPtr parent, SgExpression* expr)
    : ATAnalMRType(type), parent(parent), expr(expr) { }

  ATAnalExprMRType::ATAnalExprMRType(const ATAnalExprMRType& that)
    : ATAnalMRType(that), parent(that.parent), expr(that.expr) { }

  ATAnalMRTypePtr ATAnalExprMRType::copyATAnalMRType() const {
    return boost::make_shared<ATAnalExprMRType>(*this);
  }

  MemRegionObjectPtr ATAnalExprMRType::getParent() const {
    return parent;
  }

  bool ATAnalExprMRType::mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    if(that->getType()==ATAnalMRType::named ||
       that->getType()==ATAnalMRType::aliasing) return false;
    else if(that->getType()==ATAnalMRType::unknown) return true;
    else {
      ATAnalExprMRTypePtr etype = isATAnalExprMRType(that);
      assert(etype);
      return parent->mayEqualMR(etype->getParent(), pedge);
    }
  }

  bool ATAnalExprMRType::mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    if(that->getType() == ATAnalMRType::named ||
       that->getType() == ATAnalMRType::aliasing ||
       that->getType() == ATAnalMRType::unknown) return false;
    else {
      ATAnalExprMRTypePtr etype = isATAnalExprMRType(that);
      assert(etype);
      return parent->mustEqualMR(etype->getParent(), pedge);
    }
  }

  bool ATAnalExprMRType::equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    if(that->getType() == ATAnalMRType::named ||
       that->getType() == ATAnalMRType::aliasing ||
       that->getType() == ATAnalMRType::unknown) return false;
    else {
      ATAnalExprMRTypePtr etype = isATAnalExprMRType(that);
      assert(etype);
      return parent->equalSetMR(etype->getParent(), pedge);
    }
  }

  bool ATAnalExprMRType::subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    if(that->getType() == ATAnalMRType::named ||
       that->getType() == ATAnalMRType::aliasing) return false;
    else if(that->getType() == ATAnalMRType::unknown) return true;
    else {
      ATAnalExprMRTypePtr etype = isATAnalExprMRType(that);
      assert(etype);
      return parent->subSetMR(etype->getParent(), pedge);
    }
  }

  bool ATAnalExprMRType::isFullMRType(PartEdgePtr pedge) {
    return false;
  }

  string ATAnalExprMRType::str(string indent) const {
    ostringstream oss;
    oss << "expr: " << SgNode2Str(expr);
    return oss.str();
  }

  /***********************
   * ATAnalUnknownMRType *
   ***********************/
  ATAnalUnknownMRType::ATAnalUnknownMRType(MRType type) : ATAnalMRType(type) { }
  ATAnalUnknownMRType::ATAnalUnknownMRType(const ATAnalUnknownMRType& that) : ATAnalMRType(that) { }

  ATAnalMRTypePtr ATAnalUnknownMRType::copyATAnalMRType() const {
    return boost::make_shared<ATAnalUnknownMRType>(*this);
  }
  
  bool ATAnalUnknownMRType::mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    return true;
  }
  
  bool ATAnalUnknownMRType::mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    return false;
  }
  
  bool ATAnalUnknownMRType::equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    return that->getType() == ATAnalMRType::unknown;
  }
  
  bool ATAnalUnknownMRType::subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge) {
    return that->getType() == ATAnalMRType::unknown;
  }

  bool ATAnalUnknownMRType::isFullMRType(PartEdgePtr pedge) {
    return true;
  }
  
  string ATAnalUnknownMRType::str(string indent) const {
    return "unknown";
  }


  /*********************
   * Utility functions *
   *********************/ 
  ATAnalNamedMRTypePtr isATAnalNamedMRType(ATAnalMRTypePtr type) {
    return boost::dynamic_pointer_cast<ATAnalNamedMRType>(type);
  }

  ATAnalAliasingMRTypePtr isATAnalAliasingMRType(ATAnalMRTypePtr type) {
    return boost::dynamic_pointer_cast<ATAnalAliasingMRType>(type);
  }

  ATAnalExprMRTypePtr isATAnalExprMRType(ATAnalMRTypePtr type) {
    return boost::dynamic_pointer_cast<ATAnalExprMRType> (type);
  }

  ATAnalUnknownMRTypePtr isATAnalUnknownMRType(ATAnalMRTypePtr type) {
    return boost::dynamic_pointer_cast<ATAnalUnknownMRType>(type);
  }

  /**********************
   * FlowInSensATAnalMR *
   **********************/
  FlowInSensATAnalMR::FlowInSensATAnalMR(SgNode* sgn,
                                         ATAnalMRTypePtr type,
                                         MemRegionObjectPtr parent)
    : MemRegionObject(sgn), type(type), parent(parent) { }
  
  FlowInSensATAnalMR::FlowInSensATAnalMR(const FlowInSensATAnalMR& that) :
    MemRegionObject(that),
    sgn(that.sgn),
    type(that.type),
    parent(parent) { }

  MemRegionObjectPtr FlowInSensATAnalMR::copyMR() const {
    return boost::make_shared<FlowInSensATAnalMR>(sgn, type->copyATAnalMRType(), parent);
  }
  
  ATAnalMRTypePtr FlowInSensATAnalMR::getATAnalMRTypePtr() const {
    return type;
  }
    
  bool FlowInSensATAnalMR::mayEqualMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
    if(FlowInSensATAnalMRPtr thatMR = isFlowInSensATAnalMR(that))
      return type->mayEqualMRType(thatMR->getATAnalMRTypePtr(), pedge);
    // if thatMR is not FlowInSensATAnalMR
    else assert(false);
  }

  bool FlowInSensATAnalMR::mustEqualMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
    if(FlowInSensATAnalMRPtr thatMR = isFlowInSensATAnalMR(that))
      return type->mustEqualMRType(thatMR->getATAnalMRTypePtr(), pedge);    
    // if thatMR is not FlowInSensATAnalMR
    else assert(false);
  }
  
  bool FlowInSensATAnalMR::equalSetMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
    if(FlowInSensATAnalMRPtr thatMR = isFlowInSensATAnalMR(that))
      return type->equalSetMRType(thatMR->getATAnalMRTypePtr(), pedge);
    // if thatMR is not FlowInSensATAnalMR
    else assert(false);
  }
  
  bool FlowInSensATAnalMR::subSetMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
    if(FlowInSensATAnalMRPtr thatMR = isFlowInSensATAnalMR(that))
      return type->subSetMRType(thatMR->getATAnalMRTypePtr(), pedge);
    // if thatMR is not FlowInSensATAnalMR
    else assert(false);
  }
  
  bool FlowInSensATAnalMR::isLiveMR(PartEdgePtr pedge) {
    assert(false);
  }
  
  bool FlowInSensATAnalMR::meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
    assert(false);
  }
  
  bool FlowInSensATAnalMR::isEmptyMR(PartEdgePtr pedge) {
    assert(false);
  }
  
  bool FlowInSensATAnalMR::isFullMR(PartEdgePtr pedge) {
    return type->isFullMRType(pedge);
  }
  
  ValueObjectPtr FlowInSensATAnalMR::getRegionSize(PartEdgePtr pedge) const {
    assert(false);
  }
  
  string FlowInSensATAnalMR::str(string indent) const {
    ostringstream oss;
    oss << "[FlowInSensATAnalMR: " << type->str() << "]";
    return oss.str();
  }

  FlowInSensATAnalMRPtr isFlowInSensATAnalMR(MemRegionObjectPtr that) {
    return boost::dynamic_pointer_cast<FlowInSensATAnalMR>(that);
  }
 
}// end namespace

