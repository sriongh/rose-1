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
  string VariableIdSetPrettyPrint::str(VariableIdSet& vset, VariableIdMapping& vidm)
  {
    std::ostringstream ostr;
    ostr << "[";
    VariableIdSet::iterator it = vset.begin();
    for( ; it != vset.end(); ) {
        ostr << "(" << (*it).toString() << ", " << vidm.variableName(*it)  << ")";
        ++it;
        if(it != vset.end())
          ostr << ", ";
    }
    ostr << "]";
    return ostr.str();
  }

  string VariableIdSetPrettyPrint::str(VariableIdSet& vset)
  {
    std::ostringstream ostr;
    ostr << "[";
    VariableIdSet::iterator it = vset.begin();
    for( ; it != vset.end(); )
      {
        ostr << (*it).toString();
        it++;
        if(it != vset.end())
          ostr << ", ";
      }
    ostr << "]";
    return ostr.str();
  }

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
    vidm_p = new VariableIdMapping();
    vidm_p->computeVariableSymbolMapping(root);
  }

  FlowInSensAddrTakenAnalysis::FlowInSensAddrTakenAnalysis(const FlowInSensAddrTakenAnalysis& that)
    : root(that.root), vidm_p(that.vidm_p), 
      addressTakenSet(that.addressTakenSet),
      pointerTypeSet(that.pointerTypeSet),
      arrayTypeSet(that.arrayTypeSet),
      referenceTypeSet(that.referenceTypeSet) { }

  FlowInSensAddrTakenAnalysis::~FlowInSensAddrTakenAnalysis() {
    delete vidm_p;
  }

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

    if(addressTakenAnalysisDebugLevel() >= 2) {
      dbg << "addressTakenSet:" << VariableIdSetPrettyPrint::str(addressTakenSet, *vidm_p) << "\n";
      dbg << "pointerTypeSet:" << VariableIdSetPrettyPrint::str(pointerTypeSet, *vidm_p) << "\n";
      dbg << "arrayTypeSet:" << VariableIdSetPrettyPrint::str(arrayTypeSet, *vidm_p) << "\n";
      dbg << "referenceTypeSet:" << VariableIdSetPrettyPrint::str(referenceTypeSet, *vidm_p) << "\n";
    }
  }

  string FlowInSensAddrTakenAnalysis::str(string indent) const {
    return "FlowInSensAddrTakenAnalysis";
  }

  /****************
   * ATAnalMRType *
   ****************/
  ATAnalMRType::ATAnalMRType() { }
  ATAnalMRType::ATAnalMRType(const ATAnalMRType& that) { }

  /*********************
   * ATAnalNamedMRType *
   *********************/
  ATAnalNamedMRType::ATAnalNamedMRType(VariableId id, MemRegionPtr parent)
    : id(id) { }

  ATAnalNamedMRType::ATAnalNamedMRType(const ATAnalNamedMRType& that)
    : id(that.id) { }

  ATAnalNamedMRTypePtr isATAnalNamedMRType(ATAnalMRTypePtr type) {
    return boost::dynamic_pointer_cast<ATAnalNamedMRType>(type);
  }

  ATAnalMRTypePtr ATAnalNamedMRType::copyATAnalMRType() {
    return boost::make_shared<ATAnalNamedMRType>(*this);
  }

  VariableId ATAnalNamedMRType::getId() const {
    return id;
  }

  bool ATAnalNamedMRType::mayEqualMRType(ATAnalMRTypePtr that){
    // if that is a named type
    if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return id == ntype->getId();
    }
    // if that is a pointer type
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      if(atype->contains(id)) return true;
    }
    // if that is unknown type
    else if(isATAnalUnknownMRType(that)) return true;
    // if that is a expr type or
    // pointer type where pointer type does not contain this id
    else return false;
  }
  
  bool ATAnalNamedMRType::mustEqualMRType(ATAnalMRTypePtr that){
    // if that is a named type
    if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return id == ntype->getId();
    }
    // if that is a pointer type
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      if(atype->contains(id) && atype->singleton()) return true;
    }
    // if that is a unknown type or expr type
    else return false;
  }

  bool ATAnalNamedMRType::equalSetMRType(ATAnalMRTypePtr that){
    if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return id == ntype->getId();
    }
    // if that is a pointer type
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      if(atype->contains(id) && atype->singleton()) return true;
    }
    // if that is a unknown type or expr type
    else return false;
  }

  bool ATAnalNamedMRType::subSetMRType(ATAnalMRTypePtr that){
    if(ATAnalNamedMRTypePtr ntype = isATAnalNamedMRType(that)) {
      return id == ntype->getId();
    }
    // if that is a pointer type
    else if(ATAnalAliasingMRTypePtr atype = isATAnalAliasingMRType(that)) {
      if(atype->contains(id)) return true;
    }
    // if that is unknown type
    else if(isATAnalUnknownMRType(that)) return true;
    // if that is expr type
    else return false;
  }

  string ATAnalNamedMRType::str(std::string indent="") const{ }

  MemRegionPtr FlowInsensitivePointerAnalysis::Expr2MemRegion(SgNode* node, PartEdgePtr pedge) {
    
  }
}// end namespace

