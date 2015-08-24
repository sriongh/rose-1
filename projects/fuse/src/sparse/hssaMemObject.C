#include "heapSSA.h"
// #include "analysisCommon.h"
#include <boost/make_shared.hpp>
#include "nodeState.h"
// #include "functionState.h"
#include "LatticeArithInstance.h"
#include "analysis.h"
#include "uniqueNameTraversal.h"
#include "defsAndUsesTraversal.h"
#include "iteratedDominanceFrontier.h"
#include "controlDependence.h"
#include "heapDefsAndUsesTraversal.h"
#include "PGSSA.h"
#include "partitionGraph.h"
#include "uniqueNameTraversal.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
    
using namespace boost;
using namespace hssa_private;
 
#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH
  
bool SSAMemLoc::mayEqualML(MemLocObjectPtr o, PartPtr p) {
  return mustEqualML(o, p);
}

bool SSAMemLoc::mustEqualML(MemLocObjectPtr o, PartPtr p) {
  return mustEqualML(o);
}
  
SgNode* SSAMemLoc::getDefNode(SgExpression* expr_) const {
  const StaticSingleAssignment::VarName& varName = StaticSingleAssignment::getVarName(expr_);
  const StaticSingleAssignment::NodeReachingDefTable& reachingDefs
    = ssa->getReachingDefsAtNode_(expr_);
  SgNode* varNode = NULL;
  // Get SSA look-aside info      
  if (reachingDefs.find(varName) != reachingDefs.end()) {
    StaticSingleAssignment::ReachingDefPtr reachingDef = (* reachingDefs.find(varName)).second;
    // Get reaching def node             
    varNode = reachingDef->getDefinitionNode();
  }

  return varNode;
}

bool SSAMemLoc::mayEqualML(MemLocObjectPtr o) const {
  return mustEqualML(o);
}

bool SSAMemLoc::mustEqualML(MemLocObjectPtr o) const {
  SSAMemLoc* ssao = dynamic_cast<SSAMemLoc* >(o.get());
  if (!ssao)
    return false;
  SgExpression* expr_ = ssao->getVarExpr();

  SgNode* defNode1 = getDefNode(expr);
  SgNode* defNode2 = getDefNode(expr_);

  return expr == expr_ || defNode1 == expr_ || defNode2 == expr || defNode1 == defNode2;
}
 
/// Internal comparision, i.e. if the two given expressions are same or not
bool SSAMemLoc::isSameSig(SgExpression* exprL, SgExpression* exprR) {
  if (exprL == exprR)
    return true;

  SgVarRefExp* varL = isSgVarRefExp(exprL);
  SgVarRefExp* varR = isSgVarRefExp(exprR);
  if (varL && varR)
    return varL->get_symbol() == varR->get_symbol();

  SgDotExp* dotL = isSgDotExp(exprL);
  SgDotExp* dotR = isSgDotExp(exprR);
  if (dotL && dotR)
    return isSameSig(dotL->get_lhs_operand(), dotR->get_lhs_operand())
      && isSameSig(dotL->get_rhs_operand(), dotR->get_rhs_operand());

  SgArrowExp* arrowL = isSgArrowExp(exprL);
  SgArrowExp* arrowR = isSgArrowExp(exprR);
  if (arrowL && arrowR)
    return isSameSig(arrowL->get_lhs_operand(), arrowR->get_lhs_operand())
      && isSameSig(arrowL->get_rhs_operand(), arrowR->get_rhs_operand());

  SgPntrArrRefExp* arrL = isSgPntrArrRefExp(exprL);
  SgPntrArrRefExp* arrR = isSgPntrArrRefExp(exprR);
  if (arrL && arrR) {
    SgIntVal* intValL = isSgIntVal(arrL->get_rhs_operand());
    SgIntVal* intValR = isSgIntVal(arrR->get_rhs_operand());
    if (intValL && intValR)
      // Same array with same integer index
      return isSameSig(arrL->get_lhs_operand(), arrR->get_lhs_operand())
        && intValL->get_value() == intValR->get_value();
  }

  return false;
}

/*
bool SSAScalar::mayEqual(MemLocObjectPtr o) const {
  if (SSAScalar* ssao = dynamic_cast<SSAScalar* >(o.get()))
    // Both memory locations are SSA scalar
    return mustEqualML(o);
  else if (SSAMemLoc* ssao = dynamic_cast<SSAMemLoc* >(o.get())) {
    return false; // memLoc->mayEqual(ssao->getLabelMemLoc());
  } else
    return true;
}

bool SSAScalar::mustEqual(MemLocObjectPtr o) const {
  if (SSAScalar* ssao = dynamic_cast<SSAScalar* >(o.get())) {
    // Both memory locations are SSA scalar
    SgNode* defNode1 = getDefNode(getVarExpr());
    SgNode* defNode2 = getDefNode(ssao->getVarExpr());
    
    return defNode1 == defNode2;
  } else if (SSAMemLoc* ssao = dynamic_cast<SSAMemLoc* >(o.get())) {
    return false; // memLoc->mustEqual(ssao->getLabelMemLoc());
  } else
    return false;
}

bool SSAArray::mayEqual(MemLocObjectPtr o) const {
  if (SSAMemLoc* ssao = dynamic_cast<SSAMemLoc* >(o.get())) {
    return false; // memLoc->mayEqual(ssao->getLabelMemLoc());
  } else
    return true;
}

bool SSAArray::mustEqual(MemLocObjectPtr o) const {
  if (SSAMemLoc* ssao = dynamic_cast<SSAMemLoc* >(o.get())) {
    return false; // memLoc->mustEqual(ssao->getLabelMemLoc());
  } else
    return false;
}

bool SSAPointer::mayEqual(MemLocObjectPtr o) const {
  if (SSAMemLoc* ssao = dynamic_cast<SSAMemLoc* >(o.get())) {
    return false; // memLoc->mayEqual(ssao->getLabelMemLoc());
  } else
    return true;
}

bool SSAPointer::mustEqual(MemLocObjectPtr o) const {
  if (SSAMemLoc* ssao = dynamic_cast<SSAMemLoc* >(o.get())) {
    return false; // memLoc->mustEqual(ssao->getLabelMemLoc());
  } else 
    return false;
}
*/

SSAMemLocPtr SSAMemLocFactory::createSSAMemLoc(SgNode* expr, HeapSSA* ssa) {
  /* if (SgVarRefExp* var = isSgVarRefExp(expr)) {
    // Check variable type, then decide the different memory object type       
    SgType* varType = var->get_type();
    if (isSgPointerType(varType))
      // pointer type          
      return boost::make_shared<SSAPointer>(ssa, var);
    else if (isSgArrayType(varType))
      // array type           
      return boost::make_shared<SSAArray>(ssa, var);
    else if (isSgClassType(varType))
      // class/struct type         
      return boost::make_shared<SSALabeledAggregate>(ssa, var);
    // normal scalar                 
    return boost::make_shared<SSAScalar>(ssa, var);
  } else if (SgPntrArrRefExp* arr = isSgPntrArrRefExp(expr)) {
    // array reference: A[i]                 
    // TODO: process subscripts       
    SgExpression* arrNameExp = NULL;
    std::vector<SgExpression*>* subscripts = new std::vector<SgExpression*>;
    SageInterface::isArrayReference(arr, &arrNameExp, &subscripts);
    if (arrNameExp != NULL)
      return boost::make_shared<SSAArray>(ssa, arrNameExp);
  } else if (SgPointerDerefExp* pdRef = isSgPointerDerefExp(expr)) {
    // pointer dereference: * p     
    if (SgVarRefExp* var = isSgVarRefExp(pdRef->get_operand()))
      return boost::make_shared<SSAPointer>(ssa, var);
  } else { */
    return boost::make_shared<SSADefault>(ssa, isSgExpression(expr));
    // }
  // Do nothing here   
  // std::cout << "do nothing mem loc " << expr << std::endl;
}

SSAMemLocPtr PGSSA::createSSAMemLoc(SgNode* expr, PartPtr part, PGSSA* ssa) {
  PGSSAAnalysis* analysis = ssa->getAnalysis();
  ROSE_ASSERT(analysis && "The PGSSA Analysis should not be NULL");
  /*
  if (SgVarRefExp* var = isSgVarRefExp(expr)) {
    // Variable case:
    MemLocObjectPtr memLoc = analysis->getComposer()->OperandExpr2MemLoc(expr, expr, part->inEdgeFromAny(), analysis);
    // Check variable type, then decide the different memory object type
    SgType* varType = var->get_type();
    if (isSgPointerType(varType))
      // pointer type
      return boost::make_shared<SSAPointer>(ssa, var, memLoc, part);
    else if (isSgArrayType(varType))
      // array type
      return boost::make_shared<SSAArray>(ssa, var, memLoc, part);
    else if (isSgClassType(varType))
      // class/struct type
      return boost::make_shared<SSALabeledAggregate>(ssa, var, memLoc, part);
    // normal scalar
    return boost::make_shared<SSAScalar>(ssa, var, memLoc, part);
  } else if (SgPntrArrRefExp* arr = isSgPntrArrRefExp(expr)) {
    // array reference: A[i]
    // TODO: process subscripts
    SgExpression* arrNameExp = NULL;
    std::vector<SgExpression*>* subscripts = new std::vector<SgExpression*>;
    SageInterface::isArrayReference(arr, &arrNameExp, &subscripts);
    if (arrNameExp != NULL) {
      // This is array
      MemLocObjectPtr memLoc = analysis->getComposer()->Expr2MemLoc(expr, part->inEdgeFromAny(), analysis);
      // std::cout << "array mem loc: " << memLoc->str() << std::endl;
      return boost::make_shared<SSAArray>(ssa, arrNameExp, memLoc, part);
    }
  } else if (SgPointerDerefExp* pdRef = isSgPointerDerefExp(expr)) {
    // pointer dereference: * p
    if (SgVarRefExp* var = isSgVarRefExp(pdRef->get_operand())) {
      // Use the memory location from previous analysis
      MemLocObjectPtr memLoc = analysis->getComposer()->OperandExpr2MemLoc(var, var, part->inEdgeFromAny(), analysis);
      return boost::make_shared<SSAPointer>(ssa, pdRef, memLoc, part);
    }
  } // else 
  */
  {
    // This is for the case that has no pointer/array
    MemLocObjectPtr memLoc = analysis->getComposer()->OperandExpr2MemLoc(expr, expr, part->inEdgeFromAny(), analysis);
    return boost::make_shared<SSADefault>(ssa, expr, memLoc, part);
  }
  // Do nothing here
} 

////////////////////////////////////////////////////////////////////////////////////////////
///
/// Defs/Uses travseral for Partition Graph
///
////////////////////////////////////////////////////////////////////////////////////////////
PGReachingDefPtr PGSSA::EmptyReachingDef;
PartPtr PGSSA::EmptyPart;

void PGDefsAndUsesTraversal::collectDefsAndUses(PGSSA* pgssa, PartPtr part, 
						const std::set<PartPtr>& endNs, 
						PartSet& visitedParts) {
  if (visitedParts.find(part) != visitedParts.end()
      ) //  || endNs.find(part) != endNs.end())
    return;

  visitedParts.insert(part);

  // std::cout << "collect defs " << part->str() << std::endl;
  // Traverse defs and uses
  set<CFGNode> cfgNodes = part->CFGNodes();
  for (set<CFGNode>::iterator cfgNode = cfgNodes.begin(); cfgNode != cfgNodes.end(); cfgNode ++) {
    SgNode* sgn = cfgNode->getNode();
    
    PGDefsAndUsesTraversal pgDUTrav(pgssa, part, * cfgNode, false);
    pgDUTrav.traverse(sgn);
  }

  list<PartEdgePtr> outEdges = part->outEdges();
  for (list<PartEdgePtr>::iterator ei = outEdges.begin(); ei != outEdges.end(); ei ++) {
    collectDefsAndUses(pgssa, (* ei)->target(), endNs, visitedParts);
  } 
};

PGChildUses PGDefsAndUsesTraversal::evaluateSynthesizedAttribute(SgNode* node, SynthesizedAttributesList attrs) {
  // std::cout << "evaluating: " << node->class_name() << " and " << node << std::endl;

  bool regSucc = false;
  if (cfgNode.getNode() == node) {
    // Register the SgNode with corresponding partition graph node
    pgssa->registerNode(currPart, node);
    // Register & allocate the SSA memory locatioin object
    pgssa->registerMemLocObj(node, currPart, cfgNode);
    
    // std::cout << "evaluating: " << node->class_name() << " reg: " << node << std::endl;

    regSucc = true;
  }

  PartPtr nodePart = pgssa->getPart(node);
  // std::cout << "begin to register: " << pgssa->getPart(node)->str() << std::endl;

  // We want to propagate the def/use information up from the varRefs to the higher expressions.
  if (SgInitializedName * initName = isSgInitializedName(node)) {
    VarUniqueName * uName = StaticSingleAssignment::getUniqueName(node);
    ROSE_ASSERT(uName && "Unfound unique name?");
    
    //Add this as a def, unless this initialized name is a preinitialization of a parent class. For example,
    //in the base of B : A(3) { ... } where A is a superclass of B, an initialized name for A appears.
    // Clearly, the initialized name for the parent class is not a real variable
    if (initName->get_preinitialization() != SgInitializedName::e_virtual_base_class
	&& initName->get_preinitialization() != SgInitializedName::e_nonvirtual_base_class) {
      // pgssa->getPartOriginalDefTable()[currPart][cfgNode.getNode()].insert(uName->getKey());  
      if (nodePart)
	pgssa->getPartOriginalDefTable()[nodePart][node].insert(uName->getKey());
    }

    return PGChildUses();
  }
  
  //Variable references are where all uses originate
  else if (isSgVarRefExp(node)) {
    //Get the unique name of the def.
    VarUniqueName * uName = StaticSingleAssignment::getUniqueName(node);

    //In some cases, a varRef isn't actually part of a variable name. For example,
    //foo().x where foo returns a structure. x is an SgVarRefExp, but is not part of a variable name.
    if (uName == NULL) {
      return PGChildUses();
    }
    
    if (nodePart) {
      // Add this as a use. If it's not a use (e.g. target of an assignment), we'll fix it up later.
      pgssa->getPartLocalUsesTable()[nodePart][node].insert(uName->getKey());
    
      // Add as an use
      addHeapDefAndUse((SgExpression* )node, false);
    }

    // This varref is both the only use in the subtree and the current variable
    return PGChildUses(node, isSgVarRefExp(node));
  }
  
  // We want to propagate the def/use information up from the heap operations to the higher
  // expressions.
  else if (SgDotExp * dotExpr = isSgDotExp(node)) {
    vector<SgNode* > uses;
    uses.push_back(dotExpr);

    if (nodePart) {
      // Add as an use
      addHeapDefAndUse((SgExpression* )node, false);

      pgssa->setArrayAcc(true);
    }

    return PGChildUses(uses, dotExpr, true);
  } else if (SgArrowExp* arrowExpr = isSgArrowExp(node)) {
    vector<SgNode*> uses;
    uses.push_back(arrowExpr);

    if (nodePart) {
      // Add as an use
      addHeapDefAndUse((SgExpression* )node, false);
      
      pgssa->setArrayAcc(true);
    }
    return PGChildUses(uses, arrowExpr, true);
  } else if (SgPntrArrRefExp* arrRefExpr = isSgPntrArrRefExp(node)) {
    vector<SgNode*> uses;
    uses.push_back(arrRefExpr);

    if (nodePart) {
      // Add as an use
      addHeapDefAndUse((SgExpression* )node, false);
      
      pgssa->setArrayAcc(true);
    }
    return PGChildUses(uses, arrRefExpr, true);
  } else if (SgPointerDerefExp* pntDeRefExpr = isSgPointerDerefExp(node)) {
    vector<SgNode*> uses;
    uses.push_back(pntDeRefExpr);

    if (nodePart) {
      // Add as an use
      addHeapDefAndUse((SgExpression* )node, false);
      
      pgssa->setPointerDeref(true);
    }
    return PGChildUses(uses, pntDeRefExpr, true);
  }
  // Catch the function call case
  else if (SgFunctionCallExp* funcCallExp = isSgFunctionCallExp(node)) {
    // Here we persume that all function call has side-effect, and was treated as dphi node
   
    if (nodePart == currPart) {
      // Register as a DPhi node here
      addDPhiForNode(funcCallExp);

      // Register the use/def for the heap variable
      addHeapDefAndUse(funcCallExp, false);
    }

    return PGChildUses();
  }
  // Catch all types of Binary Operations
  else if (SgBinaryOp * binaryOp = isSgBinaryOp(node)) {
    ROSE_ASSERT(attrs.size() == 2 && "Error: BinaryOp without exactly 2 children.");
    PGChildUses& lhs = attrs[0];
    PGChildUses& rhs = attrs[1];
    
    if (nodePart != currPart)
      return PGChildUses();

    //If we have an assigning operation, we want to list everything on the LHS as being defined
    //Otherwise, everything is being used.
    vector<SgNode*> uses;
    switch (binaryOp->variantT()) {
      //All the binary ops that define the LHS
    case V_SgAndAssignOp:
    case V_SgDivAssignOp:
    case V_SgIorAssignOp:
    case V_SgLshiftAssignOp:
    case V_SgMinusAssignOp:
    case V_SgModAssignOp:
    case V_SgMultAssignOp:
    case V_SgPlusAssignOp:
    case V_SgPointerAssignOp:
    case V_SgRshiftAssignOp:
    case V_SgXorAssignOp:
    case V_SgAssignOp:
      {
	//All the uses from the RHS are propagated
	uses.insert(uses.end(), rhs.getUses().begin(), rhs.getUses().end());
	
	SgVarRefExp* currentVar = lhs.getCurrentVar();
	SgNode* currentOp = lhs.getCurrentOp();
	
	if (currentVar != NULL) { 
	  // Variable case
	  // All the uses from the LHS are propagated, unless we're an assign op
	  uses.insert(uses.end(), lhs.getUses().begin(), lhs.getUses().end());

	  vector<SgNode*>::iterator currVarUse = find(uses.begin(), uses.end(), currentVar);
	  
	  //An assign op doesn't use the var it's defining. So, remove that var from the uses
	  if (isSgAssignOp(binaryOp)) {
	    if (currVarUse != uses.end()) {
	      uses.erase(currVarUse);
	    }
	    
	    //Also remove the use from the varRef node, because it's not really a use.
	    pgssa->getPartLocalUsesTable()[currPart][currentVar].clear();
	  }
	  // All the other ops always use the var they're defining (+=, -=, /=, etc)
	  else {
	    if (currVarUse == uses.end()) {
	      uses.push_back(currentVar);
	    }
	  }
	} //xxx else 
	if (currentOp != NULL) {
	  // LHS heap access case
	  // Top heap operation from the LHS are propagated
	  // Not sure if this is needed???
	  // addDefAtNode(binaryOp, currentOp);

	  // Register as a DPhi node here
	  addDPhiForNode(node); // xxx currentOp);

	  // Register the use/def for the heap variable
	  addHeapDefAndUse((SgExpression*)binaryOp, true); //xxx currentOp, true);

	  // Remove previous heap use
	  removeHeapUse((SgExpression*)currentOp);
	} 
 
	//Set all the uses as being used at this node
	addUsesToNode(binaryOp, uses);

	if (currentVar != NULL) {
    	  //Set the current var as being defined here
	  //It's possible that the LHS has no variable references. For example,
	  //foo() = 3, where foo() returns a reference
	  addDefForVarAtNode(currentVar, binaryOp);

	  return PGChildUses(uses, currentVar);
	} else if (currentOp != NULL) {
	  return PGChildUses(uses, currentOp, true);
	} 

	return PGChildUses(uses, currentVar);
      }
    case V_SgGreaterThanOp:
    case V_SgGreaterOrEqualOp: 
    case V_SgLessThanOp:
    case V_SgLessOrEqualOp:
    case V_SgEqualityOp:
    case V_SgNotEqualOp:
      {
	// Handle the comparison condition operation
	//We want to set all the varRefs as being used here
	std::vector<SgNode*> uses;
        uses.insert(uses.end(), lhs.getUses().begin(), lhs.getUses().end());
        uses.insert(uses.end(), rhs.getUses().begin(), rhs.getUses().end());

        //Set all the uses as being used here.
        addUsesToNode(binaryOp, uses);

	if (rhs.getCurrentOp()) {
          // Propagate the current variable up. The rhs variable is the one that could be
          // potentially defined up the tree
          return PGChildUses(uses, rhs.getCurrentOp(), true);
        } else {
          // Here we handle the PI function creation
	  SgVarRefExp* lhsVar = lhs.getCurrentVar();
	  SgNode* lhsOp = lhs.getCurrentOp();
	  SgVarRefExp* rhsVar = rhs.getCurrentVar();
	  SgNode* rhsOp = rhs.getCurrentOp();
	  if (lhsVar && !lhsOp) {
	    // Add PI def for lhs var
            addDefForPIAtNode(lhsVar, binaryOp);
	  }
	  if (rhsVar && !rhsOp) {
	    // Add PI def for rhs var
	    addDefForPIAtNode(rhsVar, binaryOp);
	  }

	  // Propagate the current variable up. The rhs variable is the one that could be
          // potentially defined up the tree
          return PGChildUses(uses, rhs.getCurrentVar());
        }
      }
      //Otherwise cover all the non-defining Ops
    default:
      {
	//We want to set all the varRefs as being used here
	std::vector<SgNode*> uses;
	uses.insert(uses.end(), lhs.getUses().begin(), lhs.getUses().end());
	uses.insert(uses.end(), rhs.getUses().begin(), rhs.getUses().end());
	
	//Set all the uses as being used here.
	addUsesToNode(binaryOp, uses);

	if (rhs.getCurrentOp()) {
          // Propagate the current variable up. The rhs variable is the one that could be   
	  // potentially defined up the tree                                                
          return PGChildUses(uses, rhs.getCurrentOp(), true);
	} else {
	  // Propagate the current variable up. The rhs variable is the one that could be 
	  // potentially defined up the tree
	  return PGChildUses(uses, rhs.getCurrentVar());
	}
      }
    }
  }
  // Catch all unary operations here.
  else if (isSgUnaryOp(node)) {
    SgUnaryOp* unaryOp = isSgUnaryOp(node);
  
    if (!nodePart)
      return PGChildUses();

    //Now handle the uses. All unary operators use everything in their operand
    std::vector<SgNode*> uses;
    if (isSgAddressOfOp(unaryOp) && isSgPointerMemberType(unaryOp->get_type())) {
      //SgAddressOfOp is special; it's not always a use of its operand. When creating a reference to a member variable,
      //we create reference without providing a variable instance. For example,
      //          struct foo { int bar; };
      //
      //          void test()
      //          {
      //                  int foo::*v = &foo::bar;  <---- There is no use of foo.bar on this line
      //                  foo b;
      //                  b.*v = 3;
      //          }
      //In this case, there are no uses in the operand. We also want to delete any uses for the children
      vector<SgNode*> successors = SageInterface::querySubTree<SgNode > (unaryOp);
      
      foreach(SgNode* successor, successors) {
	pgssa->getLocalUsesTable()[successor].clear();
      }
    }
    else {
      //Guard against unary ops that have no children (exception rethrow statement)
      if (attrs.size() > 0) {
	uses.insert(uses.end(), attrs[0].getUses().begin(), attrs[0].getUses().end());
      }
    }
    
    //For these definition operations, we want to insert a def for the operand.
    SgVarRefExp* currentVar = NULL;
    if (isSgMinusMinusOp(unaryOp) || isSgPlusPlusOp(unaryOp)) {
      currentVar = attrs[0].getCurrentVar();
      
      //The defs can be empty. For example, foo()++ where foo returns a reference
      if (currentVar != NULL) {
	addDefForVarAtNode(currentVar, unaryOp);
	
	//++ and -- always use their operand. Make sure it's part of the uses
	if (find(uses.begin(), uses.end(), currentVar) == uses.end()) {
	  uses.push_back(currentVar);
	}
      }
    }
    //Some other ops also preserve the current var. We don't really distinguish between the pointer variable
    //and the value to which it points
    else if (isSgCastExp(unaryOp)) {
      currentVar = attrs[0].getCurrentVar();
    }
    else if (treatPointersAsStructs && (isSgPointerDerefExp(unaryOp) 
					|| isSgAddressOfOp(unaryOp))) {
      currentVar = attrs[0].getCurrentVar();
    }
    
    //Set all the uses as being used here.
    addUsesToNode(unaryOp, uses);
    
    //Return the combined uses
    return PGChildUses(uses, currentVar);
  }  
  else if (isSgDeleteExp(node) && treatPointersAsStructs) {
    //Deleting a variable modifies the value that it points to
    ROSE_ASSERT(attrs.size() == 1);
    SgVarRefExp* currentVar = attrs.front().getCurrentVar();
    
    if (currentVar != NULL) {
      addDefForVarAtNode(currentVar, node);
      return PGChildUses(attrs.front().getUses());
    } else {
      return PGChildUses();
    }
  }
  
  else if (isSgStatement(node)) {
    // Don't propagate uses and defs up to the statement level
    return PGChildUses();
  }
  else {
    //For the default case, we merge the uses of every attribute and pass them upwards
    std::vector<SgNode*> uses;
    for (unsigned int i = 0; i < attrs.size(); i++) {
      // cout << "Merging attr[" << i << "]" << endl;
      uses.insert(uses.end(), attrs[i].getUses().begin(), attrs[i].getUses().end());
    }

    if (nodePart == currPart) 
      // Set allthe uses as being used here.
      addUsesToNode(node, uses);

    //In the default case, we don't propagate the variable up the tree
    return PGChildUses(uses, NULL);
  }
}

/// Mark all the uses as occurring at the specified node. 
void PGDefsAndUsesTraversal::addUsesToNode(SgNode* node, std::vector<SgNode*> uses) {
  foreach(SgNode* useNode, uses) {
    PartPtr usePart = pgssa->getPart(useNode);
    // Get the unique name of the def.
    VarUniqueName * uName = StaticSingleAssignment::getUniqueName(useNode);
    if (uName != NULL) {
      //Add the varRef as a def at the current node of the ref's uniqueName
      //We will correct the reference later.
      pgssa->getPartLocalUsesTable()[usePart][node].insert(uName->getKey());
      //xxx currPart][node].insert(uName->getKey());
    } else {
      // If there's no Var name, then treat the use as a heap use
      pgssa->getPartHeapUsesTable()[usePart][node].insert(useNode); 
      //xxx currPart][node].insert(useNode);
    }
  }
}

void PGDefsAndUsesTraversal::addHeapDefAndUse(SgExpression* sgn, bool isDef) {
  const StaticSingleAssignment::VarName& hvName = pgssa->getHeapVarName(sgn);
  PartPtr nodePart = pgssa->getPart(sgn);
  pgssa->getPartLocalUsesTable()[nodePart][sgn].insert(hvName);
  // std::cout << "\n add heap use: " << nodePart->str() << " and " << isDef << std::endl;
  if (isDef)
    pgssa->getPartOriginalDefTable()[nodePart][sgn].insert(hvName);
  //xxx cfgNode.getNode()].insert(hvName);
}

void PGDefsAndUsesTraversal::removeHeapUse(SgExpression* sgn) {
  const StaticSingleAssignment::VarName& hvName = pgssa->getHeapVarName(sgn);
  PartPtr nodePart = pgssa->getPart(sgn);
  std::cout << "remove heap use: " << sgn << " on " << nodePart.get() << " " 
	    << nodePart->str() << std::endl;
  pgssa->getPartLocalUsesTable()[nodePart][sgn].erase(hvName);
}

void PGDefsAndUsesTraversal::addDPhiForNode(SgNode* sgn) {
  PartPtr nodePart = pgssa->getPart(sgn);

  ROSE_ASSERT(nodePart);

  if (nodePart == currPart)
    pgssa->addDPhi(currPart, cfgNode, sgn);
  else
    pgssa->addDPhi(nodePart, cfgNode, sgn);
}

void PGDefsAndUsesTraversal::addDefForVarAtNode(SgVarRefExp* currentVar, SgNode* defNode) {
  const StaticSingleAssignment::VarName& varName = StaticSingleAssignment::getVarName(currentVar);
  ROSE_ASSERT(varName != StaticSingleAssignment::emptyName);

  // Add the varRef as a definition at the current node of the ref's uniqueName
  pgssa->getPartOriginalDefTable()[currPart][cfgNode.getNode()].insert(varName);
  
  // We still have to use the node --> varName map, since the nodes in different parts are disjoined
  pgssa->getOriginalDefTable()[defNode].insert(varName);
}

void PGDefsAndUsesTraversal::addDefForPIAtNode(SgVarRefExp* currentVar, SgNode* defNode) {
  const StaticSingleAssignment::VarName& varName = StaticSingleAssignment::getVarName(currentVar);
  ROSE_ASSERT(varName != StaticSingleAssignment::emptyName);

  // std::cout << "add pi def: " << pgssa->varnameToString(varName) << std::endl;
  // Add the varRef as a PI definition at the current node of the ref's uniqueName
  pgssa->getPIDefTable()[defNode].insert(varName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Partition Graph bases SSA Implementation
///
///////////////////////////////////////////////////////////////////////////////////////////////////////
SSAMemLocPtr PGSSA::EmptySSAMemLoc;
ValueObjectPtr PGSSA::EmptyValueObject;
LatticePtr PGSSA::EmptyLattice;
 
bool PGSSA::build(const std::set<PartPtr>& startNodes, 
		  const std::set<PartPtr>& endNodes, bool needPiNode_) {
  needPiNode = needPiNode_;
  build(startNodes, endNodes);
}

bool PGSSA::build(SgProject* proj, bool interprocedural, bool treatPointersAsStructures) {
  // Perform the unique name traversal
  const std::vector<SgInitializedName*>& allNames
    = SageInterface::querySubTree<SgInitializedName > (project, V_SgInitializedName);
  UniqueNameTraversal uniqueTrav(allNames, treatPointersAsStructures);
  uniqueTrav.traverse(project);

  return true;
}

bool PGSSA::generateUniqueName(SgProject* project, bool treatPointersAsStructures) {
  // ROSE_ASSERT(false && "Build PGSSA unique name?");

  // Perform the unique name traversal
  const std::vector<SgInitializedName*>& allNames
    = SageInterface::querySubTree<SgInitializedName > (project, V_SgInitializedName);
  UniqueNameTraversal uniqueTrav(allNames, treatPointersAsStructures);
  uniqueTrav.traverse(project);
  
  return true;
}

bool PGSSA::build(const std::set<PartPtr>& startNodes, 
		  const std::set<PartPtr>& endNodes) {
  // Generate the unique names that are used for PGSSA
  if (!hasUniqueNames)
    hasUniqueNames = generateUniqueName(SageInterface::getProject(), false);

  // Clear internal tables
  clearTables();

  // Set current function
  // while program SSA
  // setFunction(func);

  // Traverse partition graph and collect all defs and uses
  PartSet visitedParts;
  PartPtr startN;
  foreach (startN, startNodes)
    PGDefsAndUsesTraversal::collectDefsAndUses(this, startN, endNodes, visitedParts);
 
  // Insert definitions at the SgFunctionDefinition for external variables whose values flow 
  // inside the function 
  // while program SSA
  // insertDefsForExternalVariables(func.get_declaration());

  // Insert the dummy def for the heap variable  
  getDummyDefForHeapVar(partOriginalDefTable, startNodes);
 
  //Create all ReachingDef objects:                                                             
  //Create ReachingDef objects for all original definitions                                     
  populateLocalDefsTable(); 

  // Insert phi functions at join points   
  insertPhiFunctions(startNodes, endNodes);

  // Renumber all instantiated ReachingDef objects                
  renumberAllDefinitions(startNodes, endNodes);

  // Run def/use dataflow to set all reaching def connection propertly
  runDefUseDataFlow(startNodes, endNodes);

  // We have all the propagated defs, now update the use table                           
  buildSSAUseTable(endNodes);

  return true;
}

void PGSSA::clearTables() {
  originalDefTable.clear();
  expandedDefTable.clear();
  reachingDefsTable.clear();
  localUsesTable.clear();
  useTable.clear();
  ssaLocalDefTable.clear();
  
  partLocalUsesTable.clear();
  partOriginalDefTable.clear();
  partHeapLocalUses.clear();
  pgssaLocalDefTable.clear();
  partDPhiTable.clear();
  partInReachingDefTable.clear();
  partOutReachingDefTable.clear();
  heapPartUseTable.clear();
  node2Part.clear();
  node2PI.clear();
  piDefTable.clear();
  node2MemLoc.clear();
  memLoc2Node.clear();
  memLoc2Part.clear();
  memLoc2CFGNode.clear();
}

void PGSSA::createDominator(const Function& func, const std::set<PartPtr>& startNs, 
			    const std::set<PartPtr>& endNs) {
  SgFunctionDefinition* function = func.get_declaration()->get_definition();
  map<PartPtr, PartPtr> iPostDominatorMap;
  map<PartPtr, set<PartPtr> > domFrontiers =
    calculateDomFrontiers<PartPtr, PartEdgePtr > (// function, 
						  startNs, endNs, NULL, 
						  &iPostDominatorMap);
  std::cout << "pd size 1: " << iPostDominatorMap.size() << std::endl;
  
  map<FilteredCfgNode, FilteredCfgNode> iPostDominatorMap2;
  map<FilteredCfgNode, set<FilteredCfgNode> > domFrontiers2 =
    calculateDomFrontiers_<FilteredCfgNode, FilteredCfgEdge > (function, NULL, &iPostDominatorMap2);
  
  std::cout << "pd size 2: " << iPostDominatorMap2.size() << std::endl;
  
  /*
    vector<PartPtr> definitionPoints;
  set<PartPtr> phiNodes = calculateIteratedDominanceFrontier(domFrontiers, definitionPoints);

  // map<PartPtr, PartPtr> iPostDominatorMap;
  multimap<PartPtr, pair<PartPtr, PartEdgePtr> > controlDependencies =
    calculateCD<PartPtr, PartEdgePtr > (function, iPostDominatorMap, startNs);

  typedef adjacency_list<vecS, vecS, bidirectionalS, PartPtr> TreeType;
  TreeType domTree;
  typedef graph_traits<TreeType>::vertex_descriptor TreeVertex;
  
  set<PartPtr> addedNodes;
  map<PartPtr, TreeVertex> pgNodeToVertex;
  */
}

void PGSSA::insertPhiFunctions(// SgFunctionDefinition* func, 
			       const std::set<PartPtr>& startNodes, 
			       const std::set<PartPtr>& endNodes) {
  // ROSE_ASSERT(func != NULL);

  // First, find all the places where each name is defined
  map<VarName, vector<PartPtr> > nameToDefPartsMap;

  // Traverse the partition nodes in backward order
  // back_partIterator
  bw_graphEdgeIterator<PartEdgePtr, PartPtr> backIter(endNodes);
  for (; backIter != bw_graphEdgeIterator<PartEdgePtr, PartPtr>::end(); backIter ++) {
    PartPtr part = backIter.getPart();

    foreach(const FilteredCfgNode& cfgNode, part->CFGNodes()) {
      SgNode* node = cfgNode.getNode();
    
      // Don't visit the sgFunctionDefinition node twice
      // if (isSgFunctionDefinition(node) && cfgNode != FilteredCfgNode(node->cfgForBeginning()))
      // continue;
    
      // Check the definitions at this node and add them to the map
      LocalDefUseTable::const_iterator defEntry = partOriginalDefTable[part].find(node);
      if (defEntry != partOriginalDefTable[part].end()) {
	foreach(const VarName& definedVar, defEntry->second) {
	  nameToDefPartsMap[definedVar].push_back(part);
	}
      }
    
      /* TODO:
      // Handle expanded def
      defEntry = expandedDefTable.find(node);
      if (defEntry != expandedDefTable.end()) {
      
      foreach(const VarName& definedVar, defEntry->second) {
      nameToDefNodesMap[definedVar].push_back(cfgNode);
      }
      }*/
    }
  }

  // Build an iterated dominance frontier for this function
  map<PartPtr, PartPtr> iPostDominatorMap;
  map<PartPtr, set<PartPtr> > domFrontiers =
    calculateDomFrontiers<PartPtr, PartEdgePtr > (// func, 
						  startNodes, endNodes, NULL, 
						  &iPostDominatorMap);
 
  // Calculate control dependencies (for annotating the phi functions)
  // multimap< FilteredCfgNode, pair<FilteredCfgNode, FilteredCfgEdge> > controlDependencies =
  //   calculateControlDependence<FilteredCfgNode, FilteredCfgEdge > (function, iPostDominatorMap);

  // Find the phi function locations for each variable
  VarName var;
  vector<PartPtr> definitionPoints;
  
  foreach(tie(var, definitionPoints), nameToDefPartsMap) {
    ROSE_ASSERT(!definitionPoints.empty() && "We have a variable that is not defined anywhere!");
     
    // Calculate the iterated dominance frontier 
    set<PartPtr> phiNodes = calculateIteratedDomFrontier(domFrontiers, definitionPoints);

    foreach(PartPtr phiNode, phiNodes) {
      // Check each CFG node within the partition graph node
      foreach(const FilteredCfgNode& cfgNode, phiNode->CFGNodes()) {
	SgNode* node = cfgNode.getNode();
	ROSE_ASSERT(partInReachingDefTable[phiNode][node].count(var) == 0);

	// We don't want to insert phi defs for functions that have gone out of scope
	if (// !isHeapVar(var) &&
	    !isVarInScope(var, node))
	  continue;
      
	partInReachingDefTable[phiNode][node][var] 
	  = PGReachingDefPtr(new PGReachingDef(phiNode, node, PGReachingDef::PHI));
      } 
    }
  }
}

void PGSSA::insertDefsForExternalVariables(SgFunctionDeclaration* function) {
  ROSE_ASSERT(function->get_definition() != NULL);
  
  set<VarName> usedNames = getVarsUsedInSubtree(function);

  set<VarName>& originalVarsAtFunctionEntry = originalDefTable[function->get_definition()];
  set<VarName>& expandedVarsAtFunctionEntry = expandedDefTable[function->get_definition()];

  //Iterate over each used variable and check it it is declared outside of the function scope
  foreach(const VarName& usedVar, usedNames) {
    VarName rootName;
    rootName.assign(1, usedVar[0]);
     
    SgScopeStatement* varScope = SageInterface::getScope(rootName[0]);
    SgScopeStatement* functionScope = function->get_definition();
    
    //If it is a local variable, there should be a def somewhere inside the function
    if (varScope == functionScope || SageInterface::isAncestor(functionScope, varScope)) {
      //We still need to insert defs for compiler-generated variables (e.g. __func__), 
      // since they don't have defs in the AST
      if (!isBuiltinVar(rootName))
	continue;
    }
    else if (isSgGlobal(varScope)) {
      //Handle the case of declaring "extern int x" inside the function
      //Then, x has global scope but it actually has a definition inside the function 
      // so we don't need to insert one
      if (SageInterface::isAncestor(function->get_definition(), rootName[0])) {
	//When else could a var be declared inside a function and be global?
	  SgVariableDeclaration* varDecl = isSgVariableDeclaration(rootName[0]->get_parent());
	  ROSE_ASSERT(varDecl != NULL);
	  ROSE_ASSERT(varDecl->get_declarationModifier().get_storageModifier().isExtern());
	  continue;
      }
    }
    
    //Are there any other types of external vars?
    ROSE_ASSERT(isBuiltinVar(rootName) || isSgClassDefinition(varScope) || isSgNamespaceDefinitionStatement(varScope)
		|| isSgGlobal(varScope));
    
    //The variable is not in local scope; we need to insert a def for it at the function definition  
      for (size_t i = 0; i < usedVar.size(); i++) {
	//Create a new varName vector that goes from beginning to end - i
	VarName newName;
	newName.assign(usedVar.begin(), usedVar.end() - i);
	originalVarsAtFunctionEntry.insert(newName);
	
	ROSE_ASSERT(expandedVarsAtFunctionEntry.count(newName) == 0);
      }
  }
}

void PGSSA::getDummyDefForHeapVar(PartLocalDefUseTable& partOrigDefTable,
				  const std::set<PartPtr>& startNodes) { 
  // , SgFunctionDeclaration* function) {
  PartPtr startNode;
  foreach (startNode, startNodes) {
    foreach(const FilteredCfgNode& cfgNode, startNode->CFGNodes()) {
      SgNode* astNode = cfgNode.getNode();
      // Create the dummy def for the heap variable at the start node of partition graph
      const StaticSingleAssignment::VarName& heapVar = getHeapVarName(astNode);
      partOriginalDefTable[startNode][astNode].insert(heapVar);
    }
  }
}

void PGSSA::populateLocalDefsTable() {
  vector<SgFunctionDefinition*> funcs = SageInterface::querySubTree<SgFunctionDefinition >(SageInterface::getProject(), V_SgFunctionDefinition);

  foreach(SgFunctionDefinition* f, funcs) {
    // TODO: this may not be correct, how to handle Function node?
    populateLocalDefsTable(f->get_declaration());
  }
}

void PGSSA::populateLocalDefsTable(SgFunctionDeclaration* function) {
  ROSE_ASSERT(function->get_definition() != NULL);

  struct InsertDefs : public AstSimpleProcessing {
    PGSSA* pgssa;
    
    void visit(SgNode * node) {
      // Short circuit to prevent creating empty entries in the local def table 
      // when we don't need them         
      // if ((pgssa->originalDefTable.count(node) == 0 || pgssa->originalDefTable[node].empty()) &&
      //  (pgssa->expandedDefTable.count(node) == 0 || pgssa->expandedDefTable[node].empty())) {
      // return;
      // }
      if (!pgssa->hasDef(node))
	return;
      
      // This is the table of local definitions at the current node 
      PartPtr part = pgssa->getPart(node);
      if (part == PGSSA::EmptyPart)
	return;

      NodePGReachingDefTable& localDefs = pgssa->pgssaLocalDefTable[part]; // [node];
      
      foreach(const VarName& definedVar, pgssa->partOriginalDefTable[part][node]) {
	if (localDefs.find(definedVar) == localDefs.end())
	  // This can be pre-defined with PI function
	  localDefs[definedVar] 
	    = PGReachingDefPtr(new PGReachingDef(part, node, PGReachingDef::ORIGINAL_DEF));
      }

      // Handle the PI function defs
      if (pgssa->needPiNodeSupport())
	if (SgStatement* stmt = isSgStatement(node))
	  pgssa->handleCondStmt(stmt);
       
      /* TODO:
	 // Handle expanded_def?
	 if (pgssa->expandedDefTable.count(node) > 0) {
	foreach(const VarName& definedVar, pgssa->expandedDefTable[node]) {
	  localDefs[definedVar] 
	    = PGReachingDefPtr(new PGReachingDef(node, ReachingDef::EXPANDED_DEF));
	}
      }*/
    }
  };

  InsertDefs trav;
  trav.pgssa = this;
  trav.traverse(function, preorder);
}

/// Checkthe conditionalstatements, e.g. SgIfStatement,SgSwitchStatement, to insert the PI function
void PGSSA::handleCondStmt(SgStatement* stmt) {
  if (SgIfStmt* ifStmt = isSgIfStmt(stmt)) {
    // Create and register the new PI ReachingDef
    SgExprStatement* condStmt = isSgExprStatement(ifStmt->get_conditional());
    if (!condStmt)
      return;
    SgExpression* condExpr = condStmt->get_expression();
    if (piDefTable.find(condExpr) == piDefTable.end())
      return;

    SgStatement* trueStmt = ifStmt->get_true_body();
    SgStatement* falseStmt = ifStmt->get_false_body();

    addPiReachingDefs(ifStmt, trueStmt, falseStmt, condExpr, 1, 0);
  } else if (SgForStatement* forStmt = isSgForStatement(stmt)) {
    // Create and register the new PI ReachingDef
    SgStatement* condStmt = forStmt->get_test();
   
    vector< SgNode * > travSuccs = condStmt->get_traversalSuccessorContainer();
    foreach(SgNode* succNode, travSuccs) {
      if (piDefTable.find(succNode) == piDefTable.end())
	continue;

      PartPtr succPart = getPart(succNode);
      std::cout << "For node out: " << succPart->str() << " " << succNode->class_name() << std::endl;
      
      SgExpression* condExpr = isSgExpression(succNode);
      ROSE_ASSERT(condExpr);

      SgStatement* trueStmt = forStmt->get_loop_body();
      SgStatement* falseStmt = forStmt->get_else_body();

      addPiReachingDefs(forStmt, trueStmt, falseStmt, condExpr, 1, 0);
    }
  } else if (SgWhileStmt* whileStmt = isSgWhileStmt(stmt)) {
    SgStatement* condStmt = whileStmt->get_condition();
    
    vector< SgNode * > travSuccs = condStmt->get_traversalSuccessorContainer();
    foreach(SgNode* succNode, travSuccs) {
      if (piDefTable.find(succNode) == piDefTable.end())
        continue;

      SgExpression* condExpr = isSgExpression(succNode);
      ROSE_ASSERT(condExpr);

      SgStatement* trueStmt = whileStmt->get_body();
      SgStatement* falseStmt = whileStmt->get_else_body();

      addPiReachingDefs(whileStmt, trueStmt, falseStmt, condExpr, 1, 0);
    }
  } else if (SgDoWhileStmt* dwStmt = isSgDoWhileStmt(stmt)) {
    SgStatement* condStmt = whileStmt->get_condition();

    vector< SgNode * > travSuccs = condStmt->get_traversalSuccessorContainer();
    foreach(SgNode* succNode, travSuccs) {
      if (piDefTable.find(succNode) == piDefTable.end())
        continue;

      SgExpression* condExpr = isSgExpression(succNode);
      ROSE_ASSERT(condExpr);

      SgStatement* trueStmt = whileStmt->get_body();
      SgStatement* falseStmt = NULL;  // whileStmt->get_else_body();

      addPiReachingDefs(whileStmt, trueStmt, falseStmt, condExpr, 1, 0);
    }
  } else if (SgSwitchStatement* swStmt = isSgSwitchStatement(stmt)) {
    // TODO:
    SgExprStatement* condStmt = isSgExprStatement(swStmt->get_item_selector());
    if (!condStmt)
      return;
    SgVarRefExp* condExpr = isSgVarRefExp(condStmt->get_expression());
    if (!condExpr)
      // Only handle the variable case so far
      return;
    
    const StaticSingleAssignment::VarName& varName = StaticSingleAssignment::getVarName(condExpr);
    PartPtr part = getPart(swStmt);
    int condId = 0;
    foreach(PartEdgePtr edgePtr, part->outEdges()) {
      PartPtr branchPart = edgePtr->target();
      ROSE_ASSERT(branchPart->CFGNodes().size() == 1);
      const FilteredCfgNode cfgNode = * branchPart->CFGNodes().begin();
      // std::cout << "switch out: " << branchPart->str() << std::endl;

      ROSE_ASSERT(isSgCaseOptionStmt(cfgNode.getNode()) || isSgDefaultOptionStmt(cfgNode.getNode()));

      NodePGReachingDefTable& localDefs = pgssaLocalDefTable[branchPart];
      ROSE_ASSERT(localDefs.find(varName) == localDefs.end());

      // Construct the PI function for each variable
      localDefs[varName] = PGReachingDefPtr(new PGReachingDef(branchPart, condExpr, condId ++));

      // Patch the original def table, i.e. adding the varRef as a definition at the current node of
      // the ref's uniqueName
      getPartOriginalDefTable()[branchPart][cfgNode.getNode()].insert(varName);
    }
  }
}
 
void PGSSA::addPiReachingDefs(SgStatement* stmt, SgStatement* trueStmt, SgStatement* falseStmt, 
			      SgExpression* condExpr, int trueId, int falseId) {
  PartPtr part = getPart(stmt);
  foreach(PartEdgePtr edgePtr, part->outEdges()) {
    PartPtr branchPart = edgePtr->target();
    ROSE_ASSERT(branchPart->CFGNodes().size() == 1);

    const FilteredCfgNode cfgNode = * branchPart->CFGNodes().begin();
    int condId;
    if (isNodeInNode(cfgNode.getNode(), trueStmt))
      // In true body, ID = true ID
      condId = trueId;
    else if (falseStmt && isNodeInNode(cfgNode.getNode(), falseStmt))
      // In false body, Id = false ID
      condId = falseId;
    else 
      continue;

    NodePGReachingDefTable& localDefs = pgssaLocalDefTable[branchPart];

    std::set<StaticSingleAssignment::VarName >& defVars = piDefTable[condExpr];
    foreach(const StaticSingleAssignment::VarName& varName, defVars) {
      if (localDefs.find(varName) == localDefs.end()) {
	// Construct the PI function for each variable
          localDefs[varName]
            = PGReachingDefPtr(new PGReachingDef(branchPart, condExpr, condId));

          // std::cout << "add local def: " <<  localDefs[varName]->isPiFunction() << " "
          //        << varnameToString(varName) << " at " << branchPart->str()
          //        << " and  " << cfgNode.getNode() << " for " << localDefs[varName].get()
          //        << " cond id: " << condId << std::endl;

          // Patch the original def table, i.e. adding the varRef as a definition at the current node of
          // the ref's uniqueName
          getPartOriginalDefTable()[branchPart][cfgNode.getNode()].insert(varName);
      }
    }
  }
}

/// Checkif the given expressionis in the givenSgNode
bool PGSSA::isNodeInNode(SgNode* expr, SgNode* sgn) {
  struct NodeSeeker : public AstSimpleProcessing {
    SgNode* targetNode;
    bool result;

    void visit(SgNode * node) {
      if (node == targetNode)
	result = true;
    }
  };

  NodeSeeker trav;
  trav.targetNode = expr;
  trav.result = false;
  trav.traverse(sgn, preorder);

  return trav.result;
}

void PGSSA::renumberAllDefinitions(// SgFunctionDefinition* func, 
				   const std::set<PartPtr>& startNode, 
				   const std::set<PartPtr>& endNodes) {
  //Map from each name to the next index. Not in map means 0            
  map<VarName, int> nameToNextIndexMap;

  // The SgFunctionDefinition node is special. reachingDefs INTO the function definition node 
  // are actually The definitions that reach the *end* of the function reachingDefs OUT of the 
  // function definition node are the ones that come externally into the function  

  // We process partition graph nodes in reverse postorder; this provides a natural numbering for definitions      
  // back_partIterator
  bw_graphEdgeIterator<PartEdgePtr, PartPtr> backIter(endNodes);
  for (; backIter != bw_graphEdgeIterator<PartEdgePtr, PartPtr>::end(); backIter ++) {
    PartPtr part = backIter.getPart();
  
    foreach(const FilteredCfgNode& cfgNode, part->CFGNodes()) {
      SgNode* astNode = cfgNode.getNode();

      // Iterate over all the phi functions inserted at this node. We skip the SgFunctionDefinition
      // entry node, since those phi functions actually belong to the bottom of the CFG  
      // if (cfgNode != functionStartNode) {
      
      foreach(NodePGReachingDefTable::value_type& varDefPair, partInReachingDefTable[part][astNode]) {
	const VarName& definedVar = varDefPair.first;
        PGReachingDefPtr reachingDef = varDefPair.second;

	if (!reachingDef->isPhiFunction())
	  continue;
	
	// Give an index to the variable      
	int index = 0;
	if (nameToNextIndexMap.count(definedVar) > 0) {
	  index = nameToNextIndexMap[definedVar];
	}
	nameToNextIndexMap[definedVar] = index + 1;
	
	reachingDef->setRenamingNumber(index);
      }
    }

    if (pgssaLocalDefTable.find(part) == pgssaLocalDefTable.end())
      continue;
   
    // Local defs at the function end actually occur at the very beginning of the function 
    // Iterate over all the local definitions at the node 
    foreach(NodePGReachingDefTable::value_type& varDefPair, pgssaLocalDefTable[part]) {
      const VarName& definedVar = varDefPair.first;
      PGReachingDefPtr reachingDef = varDefPair.second;
 
      //Give an index to the variable                             
      int index = 0;
      if (nameToNextIndexMap.count(definedVar) > 0) {
	index = nameToNextIndexMap[definedVar];
      }
      nameToNextIndexMap[definedVar] = index + 1;

      reachingDef->setRenamingNumber(index);
      
      // Hack the dphi reaching def and renumbering them                                        
      if (isHeapVar(definedVar) && hasDPhi(part)) {
	// Renumbering the dphi function here 
	// Give an index to the variable
        int dphiInd = 0;
        if (nameToNextIndexMap.count(definedVar) > 0) {
          dphiInd = nameToNextIndexMap[definedVar];
        }
        nameToNextIndexMap[definedVar] = dphiInd + 1;

	// Go through the CFG nodes within current part and renanming the related dphi reaching defs
	foreach(const FilteredCfgNode& cfgNode, part->CFGNodes()) {
	  SgNode* sgn = cfgNode.getNode();
	  
	  if (hasDPhi(part, sgn)) {
	    PGReachingDefPtr dphiReachingDef = getDPhi(part, sgn);
	    dphiReachingDef->setRenamingNumber(dphiInd);
	  }
	}
      }
    }
  }    
}

void PGSSA::runDefUseDataFlow(// SgFunctionDefinition* func, 
			      const std::set<PartPtr>& startNodes, 
			      const std::set<PartPtr>& endNodes) {
  //Keep track of visited nodes                                                                  
  unordered_set<PartPtr> visited;
  
  set<PartPtr> worklist;

  PartPtr currPart; //  = startNode;
  worklist.insert(startNodes.begin(), startNodes.end());
  
  while (!worklist.empty()) {
    //Get the node to work on                                                                  
    currPart = *worklist.begin();
    worklist.erase(worklist.begin());
  
    // std::cout << "prop: " << currPart->str() << std::endl;

    // Propagate defs to the current part                                                       
    bool changed = propagateDefs(currPart);
    
    // std::cout << "finish prop: " << currPart->str() << std::endl;

    //For every edge, add it to the worklist if it is not seen or something has changed 
    reverse_foreach(PartEdgePtr edge, currPart->outEdges()) {
      PartPtr nextPart = edge->target();
      
      // Insert the child in the worklist if the parent is changed or it hasn't been 
      // visited yet 
      if (changed || visited.count(nextPart) == 0) {
	// Add the part to the worklist                                                   
	bool insertedNew = worklist.insert(nextPart).second;
	if (insertedNew && getDebugExtra()) {
	  if (changed)
	    cout << "Defs Changed: Added " << currPart->str() << " to the worklist." << endl;
	  else
	    cout << "Next unvisited: Added " << nextPart->str() << " to the worklist." << endl;
	}
      }
    }
    
    //Mark the current node as seen                                                            
    visited.insert(currPart);
  }
}

/// Performs the data-flow update for one individual node, populating the reachingDefsTable for that node.
/// @returns true if the OUT defs from the node changed, false if they stayed the same.
bool PGSSA::propagateDefs(PartPtr part) {
  // SgNode* node = cfgNode.getNode();

  //This updates the IN table with the reaching defs from previous nodes                         
  updateIncomingPropagatedDefs(part);
  
  // Special Case: the OUT table at the function definition node actually denotes definitions 
  // at the function entry So, if we're propagating to the *end* of the function, we shouldn't 
  // update the OUT table     
  /*if (isSgFunctionDefinition(node) && cfgNode == FilteredCfgNode(node->cfgForEnd())) {
    return false;
  }*/
  
  bool changed = false;
  set<CFGNode> cfgNodes = part->CFGNodes();
  for(set<CFGNode>::iterator cfgNode = cfgNodes.begin(); cfgNode != cfgNodes.end(); cfgNode ++) {
    // Create a staging OUT table. At the end, we will check if this table               
    // Was the same as the currently available one, to decide if any changes have occurred  
    // We initialize the OUT table to the IN table  
    NodePGReachingDefTable outDefsTable = partInReachingDefTable[part][cfgNode->getNode()];
  
    //Special case: the IN table of the function definition node actually denotes     
    //definitions reaching the *end* of the function. So, start with an empty table to prevent 
    // definitions from the bottom of the function from propagating to the top.
    /*if (isSgFunctionDefinition(node) && cfgNode == FilteredCfgNode(node->cfgForBeginning())) {
      outDefsTable.clear();
    }*/
  
    bool changedOuts = false;
    // Now overwrite any local definitions:  
    if (pgssaLocalDefTable.count(part) > 0) {
      foreach(NodePGReachingDefTable::value_type& varDefPair, pgssaLocalDefTable[part]) {
	const VarName& definedVar = varDefPair.first;
	PGReachingDefPtr localDef = varDefPair.second;

	// std::cout << "check out def: " << varnameToString(definedVar) << " and " << isHeapVar(definedVar) << std::endl;
	// std::cout << "check out def: " << localDef->isPiFunction() << "  " 
	//	  << varnameToString(definedVar) << " at " << part->str()
	//	  << " and " << cfgNode->getNode() << " and " 
	//	  << localDef.get() <<  std::endl;

	if (getPartOriginalDefTable()[part][cfgNode->getNode()].count(definedVar) > 0) { 
	  // This var name is defined in this CFG node, here we still use originalDefTable to 
	  // maintain SgNode --> {varNames }
	  outDefsTable[definedVar] = localDef;
	  
	  // std::cout << "add out def: " << varnameToString(definedVar) << " at " << part->str() 
	  // 	    << " and " << localDef->isPiFunction() << std::endl;
	  changedOuts = true;
	}
      }
    }

    if (changedOuts) {
      changed = true;
    }
    // std::cout << "prop def 2.1 " << std::endl;
    partOutReachingDefTable[part][cfgNode->getNode()] = outDefsTable;
  }
  
  // std::cout << "prop def 3 " << std::endl;

  /* 
  // Compare old to new OUT tables    
  bool changed = (reachingDefsTable[node].second != outDefsTable);
  if (changed) {
    reachingDefsTable[node].second = outDefsTable;
  }*/
  
  return changed;
}

/// Take all the outgoing defs from previous nodes and merge them as the incoming defs
/// of the current node.
void PGSSA::updateIncomingPropagatedDefs(PartPtr part) {
  // Get the previous edges in the partition graph for this part 
  // vector<FilteredCfgEdge> inEdges = cfgNode.inEdges();
  list<PartEdgePtr> inEdges = part->inEdges();

  foreach(const FilteredCfgNode& currCfgNode, part->CFGNodes()) {
    // Check each CFG nodes in current partition graph node
    SgNode* currNode = currCfgNode.getNode();

    // NodeReachingDefTable& incomingDefTable = reachingDefsTable[currNode].first;
    NodePGReachingDefTable& incomingDefTable = partInReachingDefTable[part][currNode];
   
    // std::cout << "uip curr part: " << part->str() << " and " << inEdges.size() << std::endl;
    //Iterate all of the incoming edges                        
    // for (unsigned int i = 0; i < inEdges.size(); i++) {
    foreach(PartEdgePtr prevEdge, inEdges) {
      // SgNode* prev = inEdges[i].source().getNode();
      PartPtr prevPart = prevEdge->source();
     
      // std::cout << "uip prev part: " << prevPart->str() << std::endl;
      foreach(const FilteredCfgNode& prevCfgNode, prevPart->CFGNodes()) {
	// Check each CFG nodes in previous partition graph node   
	SgNode* prevNode = prevCfgNode.getNode();

	// const NodeReachingDefTable& previousDefs = reachingDefsTable[prev].second;
	NodePGReachingDefTable& previousDefs = partOutReachingDefTable[prevPart][prevNode];
	// std::cout<< "uip prev node: " << prevNode << std::endl;

	// Merge all the previous defs into the IN table of the current node
	VarName var;
	PGReachingDefPtr previousDef;
	foreach(tie(var, previousDef), previousDefs) {
	  // Here we don't propagate defs for variables that went out of scope 
	  // (built-in vars are body-scoped but we inserted the def at the SgFunctionDefinition 
	  // node, so we make an exception)

	  // std::cout << " prev checking " << varnameToString(var) << " and " 
	  // 	    << previousDef.get() << std::endl;
	  if (!isHeapVar(var) && 
	      !isVarInScope(var, currNode) && !isBuiltinVar(var))
	    continue;
	  
	  // If this is the first time this def has propagated to this node, just copy it over  
	  if (incomingDefTable.count(var) == 0) {
	    incomingDefTable[var] = previousDef;
	   
	    // std::cout << " single-def case " << isHeapVar(var) <<  " " << varnameToString(var) 
	    // 	      << std::endl;

	    // Process the heap variable and dphi function    
	    if (isHeapVar(var)) {
	      // Get the prev node's dphi               
	      PGReachingDefPtr prevDPhi = previousDef;
	      if (!prevDPhi->isPhiFunction()
		  && hasDPhi(prevPart, previousDef->getDefNode())) {
		prevDPhi = getDPhi(prevPart, previousDef->getDefNode());
		
		// PartPtr prevPart_ = getPart(previousDef->getDefNode());
		// if (prevPart_)
		//  std::cout << "check dphi: " << previousDef->getDefNode() << " on " 
		// 	    << prevPart.get() << " " << prevPart->str() << " and " 
		// 	    << prevPart_.get() << " " << prevPart_->str() << " " 
		// 	    << prevDPhi->isDPhiFunction() << std::endl;
	      } 
	      
	      PartPtr defPart = getPart(previousDef->getDefNode());  
	      if (defPart && hasDPhi(part, currNode)) {
		// && hasDPhi(defPart, previousDef->getDefNode())) {
		PGReachingDefPtr currDPhi = getDPhi(part, currNode);
		currDPhi->addJoinedDPhi(currDPhi, prevDPhi);
		// getDPhi(defPart, previousDef->getDefNode()));
		std::cout << "update dphi: " << defPart->str() << " --> " << part->str()
			  << std::endl;
	      }
	      // std::cout << " heap var: " << part->str() << " "  
	      // 	<< hasDPhi(defPart, previousDef->getDefNode()) << " "
	      // 	<< hasDPhi(prevPart, previousDef->getDefNode()) << " "
	      // 	<< prevDPhi->isDPhiFunction() << " "
	      // 	<< (prevDPhi->isPhiFunction() || prevDPhi->isDPhiFunction()) << std::endl;
	      if (!(prevDPhi->isPhiFunction() || prevDPhi->isDPhiFunction()))
		continue;

	      // PartPtr part_ = getPart(currNode);
	      // if (part_)
	      //   std::cout << "check heap use: " << currNode << " on " << part.get() << " " 
	      // 	  << part->str() << " and "  << part_.get() << " " << part_->str() << " "
	      // 	  << hasHeapUse(part, currNode) << " "  << hasHeapUse(part_, currNode) 
	      // 	  << " " << hasDPhi(part, currNode) << std::endl;
	      if (hasHeapUse(part, currNode)) {
		// Set prev node's dphi as all current node's heap uses' reaching def
		updateHeapUseReachingDef(part, currNode, prevDPhi);
		std::cout << "update heap use " << currNode << " on " << prevPart.get() 
			  << " " << prevPart->str() << std::endl;
	      }
	    }
	  } else {
	    PGReachingDefPtr existingDef = incomingDefTable[var];
	    // std::cout << "    multi-def case " << existingDef->isPhiFunction() << " and " 
	    // 	      << (existingDef->getDefNode() == currNode) 
	    //        << " and " << previousDef->isPiFunction() 
	    // 	      << " " << existingDef->getDefPart()->str() << " for " 
	    // 	      << varnameToString(var) << std::endl;
	    
	    if (existingDef->isPhiFunction() && existingDef->getDefNode() == currNode) {
	      // && existingDef->getDefPart() == part) {
	      //There is a phi node here. We update the phi function to point to the previous 
	      // reaching definition     
	      existingDef->addJoinedDef(previousDef, prevEdge);
	    } else {
	      // If there is no phi node, and we get a new definition, it better be the same as 
	      // the one previously propagated.                   
	      if (!(*previousDef == *existingDef)) {
		printf("ERROR: At node %s@%d, two different definitions reach for variable %s\n", 
		       currNode->class_name().c_str(), currNode->get_file_info()->get_line(), 
		       varnameToString(var).c_str());
		ROSE_ASSERT(false && "Two different definitions reach for variable");
	      }
	    }
	  
	    // Process the heap variable and dphi function  
	    if (isHeapVar(var)) {
	      SgNode* heapNode = currNode;
	      if (SgExprStatement* exprStmt = isSgExprStatement(currNode)) {
		SgExpression * expr = exprStmt->get_expression();
		heapNode = expr;
		std::cout << "strange: " << part->str() << " and " << prevPart->str() << std::endl;
		//xxx ROSE_ASSERT(false);
	      }
	      // Get the prev node's dphi 
	      PGReachingDefPtr prevDPhi = existingDef;
	      if (!prevDPhi->isPhiFunction()
		  && hasDPhi(prevPart, previousDef->getDefNode())) {
		prevDPhi = getDPhi(prevPart, previousDef->getDefNode());
		//xxx ROSE_ASSERT(false && "Multi-def can not have single DPhi");
	      }
	      std::cout << "multi heap var: " << varnameToString(var) << " "
			<< prevDPhi->isPhiFunction() << " " << prevDPhi->isDPhiFunction() << " "
			<< part->str() << " --> " << prevPart->str() << " "
			<< hasDPhi(prevPart, previousDef->getDefNode()) << " " 
			<< hasDPhi(part, currNode) << " " << hasHeapUse(part, currNode) 
			<< std::endl;

	      PartPtr defPart = getPart(previousDef->getDefNode());
	      if (// defPart && 
		  hasDPhi(part, currNode)) {
                PGReachingDefPtr currDPhi = getDPhi(part, currNode);
                currDPhi->addJoinedDPhi(currDPhi, prevDPhi);
		std::cout << "joined dphi" << std::endl;
	      }
              // std::cout << " heap var: " << part->str() << " "
              //        << hasDPhi(defPart, previousDef->getDefNode()) << " "
              //        << hasDPhi(prevPart, previousDef->getDefNode()) << " "
              //        << prevDPhi->isDPhiFunction() << " "
              //        << (prevDPhi->isPhiFunction() || prevDPhi->isDPhiFunction()) << std::endl;
              if (!(prevDPhi->isPhiFunction() || prevDPhi->isDPhiFunction()))
                continue;

              // PartPtr part_ = getPart(currNode);
              // if (part_)
              //   std::cout << "check heap use: " << currNode << " on " << part.get() << " "
              //          << part->str() << " and "  << part_.get() << " " << part_->str() << " "
              //          << hasHeapUse(part, currNode) << " "  << hasHeapUse(part_, currNode)
              //          << " " << hasDPhi(part, currNode) << std::endl;
              if (hasHeapUse(part, currNode)) {
                // Set prev node's dphi as all current node's heap uses' reaching def
                updateHeapUseReachingDef(part, currNode, prevDPhi);
		std::cout << "update multi heap use " << currNode << " " << part->str() 
			  << " on " << prevPart.get() << " " << prevPart->str() << " "
			  << std::endl;
	      }
	    }
	  }
	}
      }
    }
  }
}

// void PGSSA::buildSSAUseTable(PartPtr endNode) {
void PGSSA::buildSSAUseTable(const std::set<PartPtr>& endNodes) {
  // Traverse the partition nodes in backward order
  // back_partIterator 
  bw_graphEdgeIterator<PartEdgePtr, PartPtr> backIter(endNodes);
  for (; backIter != bw_graphEdgeIterator<PartEdgePtr, PartPtr>::end(); backIter ++) {
    PartPtr part = backIter.getPart();
    foreach(const FilteredCfgNode& cfgNode, part->CFGNodes()) {
      SgNode* node = cfgNode.getNode();

      NodePGReachingDefTable& reachingDefTable = partInReachingDefTable[part][node];
      set<StaticSingleAssignment::VarName>& vars = getPartLocalUsesTable()[part][node]; 
      // partLocalUsesTable[part][node];

      foreach(const VarName& var, vars) {
	// ROSE_ASSERT(reachingDefTable.find(var) != reachingDefTable.end());
	if (reachingDefTable.find(var) == reachingDefTable.end()) //  && isHeapVar(var))
	  continue;
	
	PGReachingDefPtr reachingDef = reachingDefTable[var];

	if (reachingDef->isPiFunction())
	  reachingDef = getOrigDefForPi(reachingDef, var);
	
	if (isHeapVar(var)) 
	  // The heap variable includes both dphi function and phi function cases
	  heapPartUseTable[getPart(reachingDef->getDefNode())].insert(part);
	else
	  // For normal scalar case
	  partUseTable[getPart(reachingDef->getDefNode())].insert(part);
      }
    }
  }
}

/// Get the real definition for the given variable and Pi function
PGReachingDefPtr PGSSA::getOrigDefForPi(PGReachingDefPtr piDef, const StaticSingleAssignment::VarName& var) {
  if (piDef->isPiFunction()) {
    SgNode* defNode = piDef->getDefNode();
    PartPtr part = getPart(defNode);
    
    NodePGReachingDefTable& reachingDefTable = partInReachingDefTable[part][defNode];
    ROSE_ASSERT(reachingDefTable.find(var) != reachingDefTable.end());
    PGReachingDefPtr retRD = reachingDefTable[var];
    if (retRD->isPiFunction())
      return getOrigDefForPi(retRD, var);

    return retRD;
  } else
    ROSE_ASSERT(false);
}

void PGSSA::registerMemLocObj(SgNode* sgn, PartPtr part, CFGNode cfgNode) {
  SgExpression* expr = isSgExpression(sgn);
  if (!isSgExpression(sgn) && !isSgInitializedName(sgn)) {
    // std::cout << "Can not create " << part->str() << std::endl;
    return;
  }
  
  SSAMemLocPtr memLoc = PGSSA::createSSAMemLoc(sgn, part, this);
  if (memLoc == EmptySSAMemLoc) {
    return;
  }
  
  // std::cout << "registered sgn: " << part->str() << std::endl;
  node2MemLoc[sgn] = memLoc;
  memLoc2Node[memLoc] = sgn;
  memLoc2Part[memLoc] = part;
  memLoc2CFGNode[memLoc] = cfgNode;
}

/// Reregister the memory location
bool PGSSA::reRegisterMemLocObj(SgNode* sgn, SSAMemLocPtr oldMemLoc, SSAMemLocPtr memLoc) {
  if (node2MemLoc.find(sgn) == node2MemLoc.end() || memLoc2Node.find(oldMemLoc) == memLoc2Node.end())
    return false;

  node2MemLoc[sgn] = memLoc;
  memLoc2Node.erase(oldMemLoc);
  memLoc2Node[memLoc] = sgn;
  PartPtr part = memLoc2Part[oldMemLoc];
  memLoc2Part.erase(oldMemLoc);
  memLoc2Part[memLoc] = part;
  CFGNode cfgNode = memLoc2CFGNode[oldMemLoc];
  memLoc2CFGNode.erase(oldMemLoc);
  memLoc2CFGNode[memLoc] = cfgNode;
}

const StaticSingleAssignment::VarName& PGSSA::getHeapVarName(SgNode* sgn) {
  // We only create single heap variable here, so this is not related to the input SgNode
  if (heapVarName == NULL) {
    std::string hv_ = "_hv_";
    SgName name(hv_.c_str());
    SgType* type = NULL;
    if (SgExpression* expr = isSgExpression(sgn)) 
      type = expr->get_type();
    else if (SgFunctionDeclaration* funcDecl = isSgFunctionDeclaration(sgn))
      type = funcDecl->get_orig_return_type();
    else
      ROSE_ASSERT(type != NULL);
    SgInitializedName* initializedName = SageBuilder::buildInitializedName(name, type);
    initializedName->set_scope(currFunc);

    // Create a new unique var name object which take the psudo SgInitializedName object
    VarUniqueName* varName = new VarUniqueName(initializedName);

    heapVarName = varName;
  }

  return heapVarName->getKey();
}

void PGSSA::addDPhi(PartPtr part, CFGNode cfgNode, SgNode* sgn) {
  // ROSE_ASSERT(partDPhiTable.find(sgn) == partDPhiTable.end());
  partDPhiTable[part][sgn] = PGReachingDefPtr(new PGReachingDef(part, sgn, PGReachingDef::DPHI, 
								PGSSA::EmptyReachingDef));
  std::cout << "\n add dphi: " << part.get() << " " << part->str() << " and " << sgn << std::endl;
}

bool PGSSA::hasDef(SgNode* node) {
  if (node2Part.find(node) == node2Part.end())
    return false;
  
  PartPtr part = node2Part[node];
  if (partOriginalDefTable.find(part) != partOriginalDefTable.end())
    return partOriginalDefTable[part].find(node) != partOriginalDefTable[part].end();
  else
    return false;
}

bool PGSSA::hasDPhi(PartPtr part) {
  // std::cout << "has dphi: " << partDPhiTable.count(part) << std::endl;
  return partDPhiTable.count(part) > 0;
}
 
bool PGSSA::hasDPhi(PartPtr part, SgNode* sgn) {
  return partDPhiTable[part].count(sgn) > 0;
}

PGReachingDefPtr PGSSA::getDPhi(PartPtr part, SgNode* sgn) {
  return partDPhiTable[part][sgn];
}

bool PGSSA::hasHeapUse(PartPtr part, SgNode* sgn) {
  if (partLocalUsesTable.count(part) > 0) 
    if (partLocalUsesTable[part].count(sgn) > 0)
      // Verify that there is a heap variable was used in normal local use table
      return partLocalUsesTable[part][sgn].count(heapVarName->getKey()) > 0;
  
  return false;
}

/// Set given node's all use SgNode with the input reaching def which is a dphi    
bool PGSSA::updateHeapUseReachingDef(PartPtr part, SgNode* sgn, PGReachingDefPtr reachingDef) {
  // TODO: the part definition may be wrong
  ROSE_ASSERT(partLocalUsesTable.find(part) != partLocalUsesTable.end() 
	      && partLocalUsesTable[part].count(sgn) > 0);
  foreach (SgNode* useNode, getPartHeapUsesTable()[part][sgn]) {
    partDPhiTable[part][useNode] = reachingDef;
  }

  return true;
}

 
/// Query functions
PGReachingDefPtr PGSSA::getReachingDef(PartPtr part, SgNode* sgn) {
  if (partInReachingDefTable.count(part) > 0 && partInReachingDefTable[part].count(sgn) > 0) {
    // Scalar case
    NodePGReachingDefTable& defTable = partInReachingDefTable[part][sgn];
    const StaticSingleAssignment::VarName& varName = StaticSingleAssignment::getVarName(sgn);
    
    return defTable[varName];
  } else if (partDPhiTable.count(part) > 0 && partDPhiTable[part].count(sgn) > 0) {
    // Heap case
    std::cout << "dphi reaching def " << part->str() << std::endl;
    return partDPhiTable[part][sgn];
  }

  std::cout << "  empty reaching def " << part->str() << std::endl;
  return EmptyReachingDef;
}

/// Collect all of the non-phi function reaching defs
void PGSSA::getReachingDefs(PartPtr part, SgNode* sgn, set<PGReachingDefPtr>& reachingDefs) {
  PGReachingDefPtr reachingDef = getReachingDef(part, sgn);
  if (reachingDef == EmptyReachingDef) return;
  if (!reachingDef->isPhiFunction()) {
    reachingDefs.insert(reachingDef);
  } else {
    PGReachingDefPtr def;
    PartEdgePtr edge;
    foreach(tie(def, edge), reachingDef->getParentDefs()) {
      if (def->isPhiFunction()) {
	PartPtr prevPart = edge->source();
	getReachingDefs(prevPart, def->getNode(), reachingDefs);
      } else
	reachingDefs.insert(def);
    }
  }
}

void PGSSA::getReachingDefs(SgNode* sgn, set<PGReachingDefPtr>& reachingDefs) {
  PartPtr part = getPart(sgn);
  getReachingDefs(part, sgn, reachingDefs);
}

void PGSSA::getReachingDefsAtPart(PartPtr part, SgExpression* expr, 
				  set<PGReachingDefPtr>& reachingDefs) {
  set<PGReachingDefPtr> tmpRDs;
  if (isSgVarRefExp(expr)) {
    // Scalar case
    const StaticSingleAssignment::VarName& varName = StaticSingleAssignment::getVarName(expr);

    // Collect the reaching defs for varName in each of CFG node in current part
    foreach(const FilteredCfgNode& currCfgNode, part->CFGNodes()) {
      SgNode* sgn = currCfgNode.getNode();
      NodePGReachingDefTable& defTable = partInReachingDefTable[part][sgn];
      
      if (defTable.count(varName) > 0)
	tmpRDs.insert(defTable[varName]);
    }
  } else if (partDPhiTable.count(part) > 0) {
    // Heap case
    // Collect the heap reaching defs for varName ineach of CFG node in current part
    foreach(const FilteredCfgNode& currCfgNode, part->CFGNodes()) {
      SgNode* sgn = currCfgNode.getNode();
      if (partDPhiTable[part].count(sgn) > 0)
	tmpRDs.insert(partDPhiTable[part][sgn]);
    }
  }

  foreach (PGReachingDefPtr reachingDef, tmpRDs) {
    if (!reachingDef->isPhiFunction()) {
      reachingDefs.insert(reachingDef);
    } else {
      PGReachingDefPtr def;
      PartEdgePtr edge;
      foreach(tie(def, edge), reachingDef->getParentDefs()) {
	if (def->isPhiFunction()) {
	  PartPtr prevPart = edge->source();
	  getReachingDefs(prevPart, def->getNode(), reachingDefs);
	} else
	  reachingDefs.insert(def);
      }
    }
  }
}

PGReachingDefPtr PGSSA::getReachingDef(PartPtr part, SSAMemLocPtr memLoc) {
  if (memLoc2Node.count(memLoc) > 0) {
    SgNode* sgn = memLoc2Node[memLoc];
    return getReachingDef(part, sgn);
  }

  std::cout << "empty reaching def 1 " << part->str() << std::endl;
  return EmptyReachingDef;
}

/// Collect the Scalar SSA uses based on part
void PGSSA::collectScalarUseParts(PartPtr part, set<PartPtr>& partSet) {
  if (partUseTable.count(part)) {
    set<PartPtr>& partUses = partUseTable[part];
    partSet.insert(partUses.begin(), partUses.end());
  }
}

/// Collect the Heap SSA uses based on part
void PGSSA::collectHeapUseParts(PartPtr part, set<PartPtr>& partSet) {
  if (heapPartUseTable.count(part)) {
    set<PartPtr>& partUses = heapPartUseTable[part];
    partSet.insert(partUses.begin(), partUses.end());
  }
}

/// Collect the part uses from scalar/heap def/use chain built by PGSSA::buildSSAUseTable
void PGSSA::collectUseParts(PartPtr part, SgNode* sgn, set<PartPtr>& partSet) {
  // TODO: handle <part, sgn>
  if (isSgVarRefExp(sgn) && !hasPointerDeref() && !hasAddressTaken())
    // For normal scalar SSA uses
    collectScalarUseParts(part, partSet);
  else {
    // For heap SSA uses
    collectHeapUseParts(part, partSet);
  }
}

SSAMemLocPtr PGSSA::getMemLocObject(SgNode* sgn) {
  if (node2MemLoc.count(sgn) > 0)
    return node2MemLoc[sgn];
  else { 
    std::cout << "no mem loc " << sgn->class_name() << " and " << sgn << std::endl;
    return PGSSA::EmptySSAMemLoc;
  }
}

void PGSSA::updateMemLocObject(SgNode* sgn, SSAMemLocPtr memLoc) {
  node2MemLoc[sgn] = memLoc;
}

/// Collect the SSA memory location for the given expression at given part
void PGSSA::getDefMemLocs(SgExpression* expr, PartPtr part, SSAMemLocSet& memLocs, bool mustDef) {
  // Check the given expression related reaching defs, i.e. only scalar reaching defs
  if (!hasPointerDeref() && !hasAddressTaken()) {
      if (SgVarRefExp* varExp = isSgVarRefExp(expr)) {
      // The definiton part is not in the given part, then get reaching def
      PGReachingDefPtr reachingDef = getReachingDef(part, varExp);
      if (reachingDef != EmptyReachingDef) {
	SSAMemLocPtr memLoc = getMemLocObject(reachingDef->getDefNode());
	if (memLoc != EmptySSAMemLoc)
	  memLocs.insert(memLoc);
	return;
      }
    }
  }
  
  // Collect heap operations
  ReachingDefSet visited;
  PGReachingDefPtr heapRD = getAnyDPhi(part);
  if (heapRD != EmptyReachingDef) {
    // std::cout << "got dphi" << std::endl;
    collectHeapRDs(heapRD, getMemLocObject(expr), memLocs, visited, mustDef);
  }
}

void PGSSA::collectHeapRDs(PGReachingDefPtr heapRD, SSAMemLocPtr memLoc, SSAMemLocSet& memLocs, 
			   ReachingDefSet& visited, bool mustDef) {
  if (visited.count(heapRD) > 0)
    return;
 
  // Register as visited
  visited.insert(heapRD);

  if (heapRD->isPhiFunction()) {
    // Go through the in edges of phi function
    PGReachingDefPtr def;
    PartEdgePtr edge;
    foreach(tie(def, edge), heapRD->getParentDefs()) {
      if (def->isPhiFunction() || def->isDPhiFunction())
	collectHeapRDs(def, memLoc, memLocs, visited);
    }
  } else {
    ROSE_ASSERT(heapRD->isDPhiFunction());
    PartPtr defPart = heapRD->getDefPart();
    SSAMemLocPtr defMemLoc = getMemLocObject(heapRD->getDefNode());

    // TODO: can also consider that check Expr2Val from Composer!!
    if (!mustDef) {
      // Collect each may/must equal memory location, here's presumption is if mayEqual == true then mustEqual == true; 
      if (defMemLoc->mayEqual(memLoc))
	memLocs.insert(defMemLoc);
      // For must equal, no need to do more back-trace, stop search
      if (defMemLoc->mustEqual(memLoc))
	return;
    } else {
      // Must def case:
      if (defMemLoc->mustEqual(memLoc)) {
	memLocs.insert(defMemLoc);
	return;
      } else if (defMemLoc->mayEqual(memLoc)) {
	// For may equal, give up back-trace
	return;
      } else {
	// Not may/must equal (i.e. must not equal), then keep doing back-trace
      }
    }

    PGReachingDefPtr prevRD = heapRD->getPrevDef();
    if (prevRD == PGSSA::EmptyReachingDef)
      // If the heap def/use chain has nopredecessor, then stop search
      return;
    
    // Keep tracing the previous heap definition operation
    collectHeapRDs(prevRD, memLoc, memLocs, visited);
  }
}

/// Get one of dphi node from the given part, here we just pick up the 1st one, since using the single heap variable
PGReachingDefPtr PGSSA::getAnyDPhi(PartPtr part) {
  ROSE_ASSERT(heapVarName);
  const StaticSingleAssignment::VarName& hvName = heapVarName->getKey();
  
  // Pick up the 1st CFG node in part, and it has heap def
  foreach(const FilteredCfgNode& currCfgNode, part->CFGNodes()) {
    // Check each CFG nodes in current partition graph node
    SgNode* currNode = currCfgNode.getNode();
    /*
      NodePGReachingDefTable& incomingDefTable = partInReachingDefTable[part][currNode];
    
    if (incomingDefTable.count(hvName) == 0)
      continue;

    // Get the heap reaching def (it may be a phi function)
    PGReachingDefPtr heapRD = incomingDefTable[hvName];
    */
    
    // Check if current node has DPhi
    PGReachingDefPtr heapRD = getDPhi(part, currNode);
    if (!heapRD) {
      map<SgNode*, PGReachingDefPtr >::iterator it = partDPhiTable[part].begin();
      SgNode* sgn = it->first;
      
      heapRD = it->second;
    }

    if (heapRD && (heapRD->isDPhiFunction() || heapRD->isPhiFunction()))
      return heapRD;

    NodePGReachingDefTable& incomingDefTable = partInReachingDefTable[part][currNode];
    if (incomingDefTable.count(hvName) == 0)
      continue;

    // Get the heap reaching def (it may be a phi function)
    heapRD = incomingDefTable[hvName];

    if (heapRD && (heapRD->isDPhiFunction() || heapRD->isPhiFunction()))
      return heapRD;
  }

  if (partDPhiTable.count(part) > 0) {
    // Just pick up the 1st CFG node
    map<SgNode*, PGReachingDefPtr >::iterator it = partDPhiTable[part].begin();
    if (it != partDPhiTable[part].end()) {
      PGReachingDefPtr heapRD  = it->second;
      CFGNode cfgNode = it->first;
      // std::cout << "\ngot part: " << cfgNode.getNode() << " on " << part.get() 
      // 	<< " " << part->str() << std::endl;
      
      // ROSE_ASSERT(heapRD);
      // ROSE_ASSERT(heapRD->isDPhiFunction() || heapRD->isPhiFunction());
      if (heapRD && (heapRD->isDPhiFunction() || heapRD->isPhiFunction()))
	return heapRD;
    }
  }

  return EmptyReachingDef;
}

/// Partition graph reaching def functions
void PGReachingDef::addJoinedDef(PGReachingDefPtr newDef, PartEdgePtr edge) {
  ROSE_ASSERT(isPhiFunction());
  
  parentDefs[newDef] = edge;
}

bool PGReachingDef::operator==(const PGReachingDef& other) const {
  return (defType == other.defType) && (defNode == other.defNode) &&
    (parentDefs == other.parentDefs) && (renamingNumber == other.renamingNumber);
}

/// Print the type of reaching definition
std::string PGReachingDef::str() {
  if (defType == ORIGINAL_DEF)
    return "PGR ORG";
  else if (defType == PHI)
    return "PGR PHI";
  else if (defType == DPHI)
    return "PGR DPHI";
  else if (defType == PI)
    return "PGR PI";
  else
    return "unknown PGR";
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Partition Graph based Abstract Object Map
///
//////////////////////////////////////////////////////////////////////////////////////////////////
PGSSAObjectMap::PGSSAObjectMap(const AbstractObjectMap& that) : AbstractObjectMap(that) {
  ROSE_ASSERT(false);
}

PGSSAObjectMap::PGSSAObjectMap(LatticePtr defaultLat_, PartEdgePtr pedge, Composer* comp,
			       ComposedAnalysis * analysis) :
  AbstractObjectMap(defaultLat_, pedge, comp, analysis) { 
  mapIsFull = true;
  ROSE_ASSERT(dynamic_cast<PGSSAAnalysis* >(analysis));
  pgssa = (dynamic_cast<PGSSAAnalysis* >(analysis))->getPGSSA();
}

/// Remap the memory location and lattice between caller and callee
Lattice* PGSSAObjectMap::remapML(const std::set<pair<MemLocObjectPtr, MemLocObjectPtr> >& ml2ml, 
				 PartEdgePtr newPEdge) {
  /*PGSSAObjectMap* newmapML = new PGSSAObjectMap(* this);
  pair<MemLocObjectPtr, MemLocObjectPtr> p;
  foreach(p, ml2ml) {
    SSAMemLocPtr oldMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(p.first);
    SSAMemLocPtr newMemLoc = boost::dynamic_pointer_cast<SSAMemLoc>(p.second);
    
    // Map the lattice value from old memory location to new memory location
    if (oldMemLoc && newMemLoc && internalTable.find(oldMemLoc) != internalTable.end()) {
      LatticePtr l = internalTable[oldMemLoc];
      newmapML->insertValue(newMemLoc, l);
    }
  }

  return newmapML;*/
  
  pair<MemLocObjectPtr, MemLocObjectPtr> p;
  foreach(p, ml2ml) {
    SSAMemLocPtr oldML = boost::dynamic_pointer_cast<SSAMemLoc>(p.first);
    SSAMemLocPtr newML = boost::dynamic_pointer_cast<SSAMemLoc>(p.second);
  
    // Map the lattice value from old memory location to new memory location, if the new 
    // memory location has lattice, then do meetUpdate
    if (oldML && newML) {
      if (internalTable.find(oldML) != internalTable.end()) {
	LatticePtr oldLat = internalTable[oldML];
	if (internalTable.find(newML) != internalTable.end()) {
	  LatticePtr newLat = internalTable[newML];
	  newLat->meetUpdate(oldLat.get());
	} else
	  internalTable[newML] = oldLat;
      }
    }
  }
}

bool PGSSAObjectMap::insertValue(SSAMemLocPtr memLoc, LatticePtr valObj) {
  if (internalTable.count(memLoc) > 0)
    return false;

  internalTable[memLoc] = valObj;

  return true;
}

bool PGSSAObjectMap::removeValue(SSAMemLocPtr memLoc, LatticePtr valObj) {
  if (internalTable.count(memLoc) == 0)
    return false;
  
  internalTable.erase(memLoc);

  return true;
}
 
LatticePtr PGSSAObjectMap::getValue(SSAMemLocPtr memLoc) {
  // Check current value
  if (internalTable.count(memLoc) > 0) 
    return internalTable[memLoc];

  return PGSSA::EmptyLattice;
} 

/// Get the lattice for a give phi function
LatticePtr PGSSAObjectMap::getPhiValue(PGReachingDefPtr phiRD) {
  // Copy the 1st lattice in phi function and meetUpdate all others
  LatticePtr retLattice = PGSSA::EmptyLattice;
  
  PGReachingDefPtr def;
  PartEdgePtr edge;
  
  bool first = true;
  foreach(tie(def, edge), phiRD->getParentDefs()) {
    LatticePtr defLattice;
    if (def->isPiFunction())
      // Ignore PI function
      continue;
    else if (def->isPhiFunction())
      // Phi function case 
      defLattice = getPhiValue(def);
    else {
      // Normal reaching definition case
      SSAMemLocPtr memLoc = pgssa->getMemLocObject(def->getDefNode());
      defLattice = getValue(memLoc);
    }

    if (defLattice != PGSSA::EmptyLattice) {
      if (!first) {
	// Copy the 1st lattice from parent defs
	retLattice = LatticePtr(defLattice->copy());
	first = false;
      } else 
	// Meet update parent defs
	retLattice->meetUpdate(defLattice.get());
    } 
  }

  return retLattice;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Partition Graph based SSA Analysis
///
//////////////////////////////////////////////////////////////////////////////////////////////////

/*NodeState* PGSSAAnalysis::initializeFunctionNodeState(const Function &func, NodeState *fState) {
  // Initialize the function's entry NodeState 
  NodeState* entryState = NodeState::getNodeState(this, getComposer()->GetFunctionStartPart(func, this));

  NodeState::copyLattices_aEQa((Analysis*)this, *entryState, *fState);

  return entryState;
}*/

/// Get theinitialwork list (i.e. FlowWorkList) element for Sparse analysis, here we choose the entry
/// of partition graph
/*std::list<PartPtr> PGSSAAnalysis::getInitialWorklist(const Function &func, bool analyzeDueToCallers,
						     const set<Function> &calleesUpdated, NodeState *fState) {
  list<PartPtr> dfIt;
  // Just create the worklist with the start node of partition graph
  PartPtr funcPGStart = getComposer()->GetFunctionStartPart(func, this);
  dfIt.push_back(funcPGStart);

  return dfIt; 
}*/
 
std::set<PartPtr> PGSSAAnalysis::getInitialWorklist() {
  set<PartPtr> initWorklist;
  ROSE_ASSERT(false && "pgssa init worklist");
  set<PartPtr> tempStates = getComposer()->GetStartAStates(this);
  initWorklist.insert(tempStates.begin(), tempStates.end());
  return initWorklist;
}

std::map<PartEdgePtr, std::vector<Lattice*> >& PGSSAAnalysis::getLatticeAnte(NodeState *state) {
  return state->getLatticeAboveAllMod(this);
}

std::map<PartEdgePtr, std::vector<Lattice*> >& PGSSAAnalysis::getLatticePost(NodeState *state) {
  return state->getLatticeBelowAllMod(this); 
}

void PGSSAAnalysis::setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, 
				   bool overwrite) {
  if (overwrite) 
    state->copyLatticesOW(state->getLatticeAboveAllMod(this), dfInfo);
  else   
    state->copyLattices(state->getLatticeAboveAllMod(this), dfInfo);
}

void PGSSAAnalysis::setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, 
				   bool overwrite) {
  if (overwrite) 
    state->copyLatticesOW(state->getLatticeBelowAllMod(this), dfInfo);
  else          
    state->copyLattices(state->getLatticeBelowAllMod(this), dfInfo);
}

list<PartPtr> PGSSAAnalysis::getDescendants(PartPtr part) {
  list<PartPtr> descendants;
  list<PartEdgePtr> outEdges = part->outEdges();
  for(list<PartEdgePtr>::iterator ei=outEdges.begin(); ei!=outEdges.end(); ei++)
    descendants.push_back((*ei)->target());
  return descendants;
}
  
list<PartEdgePtr> PGSSAAnalysis::getEdgesToDescendants(PartPtr part) {
  return part->outEdges();
}
  
/*std::set<PartPtr> PGSSAAnalysis::getUltimate(const Function &func) {
  return getComposer()->GetEndAStates(this);
}*/

set<PartPtr> PGSSAAnalysis::getUltimate() { 
  return getComposer()->GetEndAStates(this); 
}

dataflowPartEdgeIterator* PGSSAAnalysis::getIterator(const Function &func) {
  set<PartPtr> terminalStates = getComposer()->GetEndAStates(this);
  return new fw_dataflowPartEdgeIterator();
}
 
/// Remaps the given Lattice across the scope transition (if any) of the given edge
void PGSSAAnalysis::remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat) {
  // Here we do same thing as FWDataflow did
  scope reg("PGSSAAnalysis::remapML", scope::medium, attrGE("composedAnalysisDebugLevel", 1));
  for (unsigned int i = 0; i < lat.size(); i ++) {
    Lattice* newL = lat[i]->getPartEdge()->forwardRemapML(lat[i], fromPEdge, this);
    assert(newL == lat[i] && "Sparse analysis still uses same lattice");
  }
}

/// This is the main framework for running SSA based analysis
void PGSSAAnalysis::runAnalysis() {
  std::cout << "run analysis 1" << std::endl;

  // Build partiton graph based SSA form 
  std::set<PartPtr> startParts, endParts;
  std::set<PartPtr> tempStates = getComposer()->GetStartAStates(this);

  std::cout << "run analysis 2 " << std::endl;

  startParts.insert(tempStates.begin(), tempStates.end());
  tempStates = getComposer()->GetEndAStates(this);
  endParts.insert(tempStates.begin(), tempStates.end());

  // getComposer()->GetFunctionStartPart(func, this);
  pgssa.setAnalysis(this);

  pgssa.build(startParts, endParts, false);

  std::cout << "run analysis 2.1 " << std::endl;

  // Make sure that we've been paired with a valid inter-procedural dataflow analysis
  // ROSE_ASSERT(dynamic_cast<InterProceduralDataflow*>(interAnalysis));
  
  // Set of all the Parts that have already been visited by the analysis
  set<PartPtr> visited;
  
  // Re-analyze it from scratch
  //xxx startParts = getInitialWorklist(); 
  // dataflowPartEdgeIterator* FlowWorkListIter = getIterator(func);
  list<PartPtr> FlowWorkList;
  list<PartPtr> SSAWorkList;

  std::cout << "run analysis 3:  " << startParts.size() << std::endl;

  set<PartPtr> ultimateParts = getUltimate(); // func);
  visited.insert(ultimateParts.begin(), ultimateParts.end());

  // Initialize FlowWorkList
  for (std::set<PartPtr>::iterator partIter = startParts.begin(); partIter != startParts.end();
       ++ partIter)
    FlowWorkList.push_back(* partIter);
  
  // while (* FlowWorkListIter != dataflowPartIterator::end()) {
  while (!FlowWorkList.empty()) {
    PartPtr currPart = FlowWorkList.front();
    FlowWorkList.pop_front();
 
    // Iterate over the nodes in this function that are downstream from the nodes added above
    // dataflowPartIterator* curNodeIt = getIterator(func);
    // curNodeIt->add(*start);
    // while(*curNodeIt!=dataflowPartIterator::end()) {
   
    bool modified = false;
    bool firstVisit;
    if ((firstVisit = (visited.find(currPart) == visited.end())))
      visited.insert(currPart);
    else {
      // If current part has been visited, go next
      // (* FlowWorkListIter) ++;
      continue;
    }

    visitPart(currPart, SSAWorkList);
   
    while (!SSAWorkList.empty()) {
      currPart = SSAWorkList.front();
      SSAWorkList.pop_front();

      if (visited.find(currPart) == visited.end())
	continue;

      visitPart(currPart, SSAWorkList);
    } // while (!SSAWorkList.empty())
    
    // Add the descendants of current part to FlowWorkList
    list<PartPtr> descendants = getDescendants(currPart);
    for (list<PartPtr>::iterator partIter = descendants.begin(); partIter != descendants.end(); 
	 ++ partIter) 
      FlowWorkList.push_back(* partIter); // descendants.begin(), descendants.end());
    
    // (* FlowWorkListIter) ++;
  } // while (* FlowWorkListIter != dataflowPartIterator::end()) 

  std::cout << "finish analysis" << std::endl;
}

bool PGSSAAnalysis::visitPart(PartPtr part, list<PartPtr>& SSAWorkList) {
  // The NodeState associated with this part
  NodeState* state = NodeState::getNodeState(this, part);

  map<PartEdgePtr, vector<Lattice*> >& dfInfoAnte = getLatticeAnte(state);
  // Create a local map for the post dataflow information. It will be deallocated 
  // at the end of the transfer function.
  map<PartEdgePtr, vector<Lattice*> > dfInfo;
  collectDefsLattice(part, dfInfo[NULLPartEdge]);

  bool modified = false;

  // Iterate over all the CFGNodes associated with this part and merge the result of applying 
  // to transfer function to all of them
  set<CFGNode> cfgNodes = part->CFGNodes();
  for(set<CFGNode>::iterator cfgNode = cfgNodes.begin(); cfgNode != cfgNodes.end(); 
      cfgNode ++) {
    SgNode* sgn = cfgNode->getNode();
    
    //  TRANSFER FUNCTION for inter-procedural case<
    // Handle the function call case

    // Handle the normal SgNodes
    boost::shared_ptr<PGSSAIntraProcTransferVisitor> transferVisitor 
      = getSSATransferVisitor(part, *cfgNode, *state, dfInfo);
   
    // Set current lattice map, i.e. PGSSA based abstract object map
    transferVisitor->setLatticeMap(currentObjMap(NULLPartEdge));
    
    // Visit the node 
    sgn->accept(*transferVisitor);
    modified = transferVisitor->finish() || modified;

    // Add the extra node to SSAWorkList
    set<PartPtr> reanalysisParts = transferVisitor->getParts();
    SSAWorkList.insert(SSAWorkList.end(), reanalysisParts.begin(), reanalysisParts.end());
  } // for(set<CFGNode>::iterator cfgNode = cfgNodes.begin(); cfgNode != cfgNodes.end(); cfgNode ++)
 
  return modified;
}

void PGSSAAnalysis::collectDefsLattice(PartPtr part, vector<Lattice*>& dfInfo) {
  // TODO:
  // ROSE_ASSERT(false && "Unsupported!");
}

PGSSAObjectMap* PGSSAAnalysis::currentObjMap(const Function& func, PartEdgePtr pedge) {
  return currentObjMap(pedge);
}

PGSSAObjectMap* PGSSAAnalysis::currentObjMap(PartEdgePtr pedge) {
  if (!currentObjMap_)
    return getObjectMap(pedge);
  else
    return currentObjMap_;
}

PGSSAObjectMap* PGSSAAnalysis::currentObjMap() {
  ROSE_ASSERT(false && "Unsupported!");
  return currentObjMap_;
}
 
PGSSAIntraProcTransferVisitorPtr PGSSAAnalysis::getSSATransferVisitor(// const Function& func, 
								      PartPtr part, CFGNode cn,
								      NodeState& state,  
								      map<PartEdgePtr, vector<Lattice*> >& dfInfo) {
  // Return the default transfer visitor that does nothing
  return boost::shared_ptr<PGSSAIntraProcTransferVisitor>(new DefaultPGSSATransferVisitor(// func, 
											  part, cn, state, 
											  dfInfo, &pgssa, getComposer())); 
}
  
// This one should be replaced with the SSA based transfer function
bool PGSSAAnalysis::transfer(const Function& func, PartPtr part, CFGNode cn, NodeState& state, 
			     std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {

  return false;
}
 
std::string PGSSAAnalysis::str(std::string) {
  return "Partition Graph Analysis";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// The PGSSA based intra-procedural transfer visitor
///
////////////////////////////////////////////////////////////////////////////////////////////////////
set<PartPtr> DefaultPGSSATransferVisitor::getParts() {
  return reanalysisParts;
}
 
SSAMemLocPtr PGSSAIntraProcTransferVisitor::getMemLocObject(SgNode* sgn) {
  SSAMemLocPtr ret = pgssa->getMemLocObject(sgn);
  return ret;
}
 
void PGSSAIntraProcTransferVisitor::collectUseParts(PartPtr part) {
  pgssa->collectScalarUseParts(part, reanalysisParts);
}

void PGSSAIntraProcTransferVisitor::collectUseParts(PartPtr part, SgNode* sgn) {
  pgssa->collectUseParts(part, sgn, reanalysisParts);
}

PGReachingDefPtr PGSSAIntraProcTransferVisitor::getReachingDef(SgNode* sgn) {
  PartPtr part = pgssa->getPart(sgn);
  if (part != PGSSA::EmptyPart)
    return pgssa->getReachingDef(part, sgn);
  else {
    return PGSSA::EmptyReachingDef;
  }
}

PGReachingDefPtr PGSSAIntraProcTransferVisitor::getReachingDef(PartPtr part, SgNode* sgn) {
  return pgssa->getReachingDef(part, sgn);
}
