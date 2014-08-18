#include "heapDefsAndUsesTraversal.h"
#include "heapReachingDef.h"
#include <boost/foreach.hpp>
#include <iostream>
 
using namespace std;
using namespace hssa_private;

#define foreach BOOST_FOREACH

HeapUses HeapDefsAndUsesTraversal::evaluateSynthesizedAttribute(SgNode* node, 
								SynthesizedAttributesList attrs)
{
    if (StaticSingleAssignment::getDebug())
    {
        cout << "---------<" << node->class_name() << node << ">-------" << node << endl;
    }

    // if (node != NULL) std::cout << "def use traversal: " << node->class_name() << std::endl;

    // We want to propagate the def/use information up from the heap operations to the higher 
    // expressions.
    if (SgDotExp * dotExpr = isSgDotExp(node)) {
      vector<SgNode* > uses;
      uses.push_back(dotExpr);

      return HeapUses(uses, dotExpr);
    } else if (SgArrowExp* arrowExpr = isSgArrowExp(node)) {
      vector<SgNode*> uses;
      uses.push_back(arrowExpr);
      
      return HeapUses(uses, arrowExpr);
    } else if (SgPntrArrRefExp* arrRefExpr = isSgPntrArrRefExp(node)) {
      vector<SgNode*> uses;
      uses.push_back(arrRefExpr);
      
      return HeapUses(uses, arrRefExpr);
    } else if (SgPointerDerefExp* pntDeRefExpr = isSgPointerDerefExp(node)) {
      vector<SgNode*> uses;
      uses.push_back(pntDeRefExpr);
      
      return HeapUses(uses, pntDeRefExpr);
    }  
    //Catch all types of Binary Operations
    else if (SgBinaryOp * binaryOp = isSgBinaryOp(node))
    {
        ROSE_ASSERT(attrs.size() == 2 && "Error: BinaryOp without exactly 2 children.");
        HeapUses& lhs = attrs[0];
        HeapUses& rhs = attrs[1];

        //If we have an assigning operation, we want to list everything on the LHS as being defined
        //Otherwise, everything is being used.
        vector<SgNode*> uses;
        switch (binaryOp->variantT())
        {
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
	   
	    SgNode* currentOp = lhs.getCurrentOp();
	    if (currentOp != NULL) {
	      // Top heap operation from the LHS are propagated               
	      // uses.push_back(currentOp);
	      addDefAtNode(binaryOp, currentOp);
	     
	      // Register as a DPhi node here
	      addDPhiForNode(binaryOp);
	      
	      // Register the use/def for the heap variable
	      addHeapUseAndDef((SgExpression*)binaryOp);
	    } else if (rhs.getCurrentOp() != NULL)
	      // Register the use for the heap variable
	      addHeapUse((SgExpression*)binaryOp);

	    //Set all the uses as being used at this node
	    addUsesToNode(binaryOp, uses);
	    
	    return HeapUses(uses, currentOp);
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
	    
	    //Propagate the current variable up. The rhs variable is the one that could be potentially defined up the tree
	    return HeapUses(uses, rhs.getCurrentOp());
	  }
        }
    }
    //Catch all unary operations here.
    else if (isSgUnaryOp(node))
    {
        SgUnaryOp* unaryOp = isSgUnaryOp(node);

        //Now handle the uses. All unary operators use everything in their operand
        std::vector<SgNode*> uses;
	//Guard against unary ops that have no children (exception rethrow statement)
	if (attrs.size() > 0) {
	  uses.insert(uses.end(), attrs[0].getUses().begin(), attrs[0].getUses().end());
	}

        //For these definition operations, we want to insert a def for the operand.
        SgNode* currentOp = NULL;
        if (isSgMinusMinusOp(unaryOp) || isSgPlusPlusOp(unaryOp)) {
            currentOp = attrs[0].getCurrentOp();

            //The defs can be empty. For example, foo()++ where foo returns a reference
            if (currentOp != NULL)
            {
	      // Register the use/def for the heap variable
	      addHeapUseAndDef((SgExpression*)unaryOp);

	      addDefAtNode(unaryOp, currentOp);

	      //++ and -- always use their operand. Make sure it's part of the uses
	      if (find(uses.begin(), uses.end(), currentOp) == uses.end()) {
		uses.push_back(currentOp);
	      }
            }
        }
	//Some other ops also preserve the current var. We don't really distinguish between the pointer variable
	//and the value to which it points
        else if (isSgCastExp(unaryOp))
        {
            currentOp = attrs[0].getCurrentOp();
        }
        else if (treatPointersAsStructs && (isSgPointerDerefExp(unaryOp) || isSgAddressOfOp(unaryOp)))
        {
            currentOp = attrs[0].getCurrentOp();
        }

        //Set all the uses as being used here.
        addUsesToNode(unaryOp, uses);

        //Return the combined uses
        return HeapUses(uses, currentOp);
    }
    
    else if (isSgDeleteExp(node) && treatPointersAsStructs)
    {
        //Deleting a variable modifies the value that it points to
        ROSE_ASSERT(attrs.size() == 1);
        SgNode* currentOp = attrs.front().getCurrentOp();

        if (currentOp != NULL)
        {
	  addDefAtNode(node, currentOp);
	  return HeapUses(attrs.front().getUses(), currentOp);
        }
        else
        {
	  return HeapUses();
        }
    }
    
    else if (isSgStatement(node))
    {
        //Don't propagate uses and defs up to the statement level
        return HeapUses();
    }
    else
    {
      if (SgInitializedName * initName = isSgInitializedName(node)) {     
	if (initName->get_type()->containsInternalTypes()) {
	  // TODO: handle the struct and array definition
	}
      } 
      
      //For the default case, we merge the uses of every attribute and pass them upwards
      std::vector<SgNode*> uses;
      for (unsigned int i = 0; i < attrs.size(); i++)
	{
	  if (StaticSingleAssignment::getDebug())
	    {
	      cout << "Merging attr[" << i << "]" << endl;
            }
	  uses.insert(uses.end(), attrs[i].getUses().begin(), attrs[i].getUses().end());
        }
      
      //Set all the uses as being used here.
      addUsesToNode(node, uses);
      
      //In the default case, we don't propagate the variable up the tree
      return HeapUses(uses, NULL);
    }
}

/** Mark all the uses as occurring at the specified node. */
void HeapDefsAndUsesTraversal::addUsesToNode(SgNode* node, std::vector<SgNode*> uses)
{
    foreach(SgNode* useNode, uses)
    {
      hssa->hv_addHeapUse(node, useNode); 

      if (StaticSingleAssignment::getDebug())
        {
	  cout << "Found use for " << 
	    " at " << node->cfgForBeginning().toStringForDebugging() << endl;
        }
    }
}

void HeapDefsAndUsesTraversal::addDefAtNode(SgNode* defNode, SgNode* heapDef)
{
  // Add the heapDef as a definition at the current node
  hssa->hv_addHeapDef(defNode, heapDef);
 
  if (StaticSingleAssignment::getDebug())
    {
      cout << "Found def for " 
	   << " at " << defNode->cfgForBeginning().toStringForDebugging() << endl;
    }
}

void HeapDefsAndUsesTraversal::addDPhiForNode(SgNode* defNode) {
  hssa->hv_addDPhi(defNode);
}

/**
 * Insert heap variable use for given SgNode 
 */
void HeapDefsAndUsesTraversal::addHeapUse(SgExpression* sgn) {
  const StaticSingleAssignment::VarName& hvName = hssa->hv_getHeapVarName(sgn);
  hssa->getLocalUsesTable()[sgn].insert(hvName);
}

/**
 * Insert heap variable use/def for given SgNode 
 */
void HeapDefsAndUsesTraversal::addHeapUseAndDef(SgExpression* sgn) {
  const StaticSingleAssignment::VarName& hvName = hssa->hv_getHeapVarName(sgn);
  hssa->getLocalUsesTable()[sgn].insert(hvName);
  hssa->getOriginalDefTable()[sgn].insert(hvName);
}
