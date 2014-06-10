#pragma once
#include "rose.h"
#include "heapSSA.h"
#include "heapReachingDef.h"

namespace hssa_private
{

    /** Attribute that describes the heap operation used by a given expression. */
    class HeapUses
    {
    private:
        /** An assignment to the current expression in the AST would define this variable */
        SgNode* currentOp;

        /** Stores all the varRefs that are used in the current subTree. */
        std::vector<SgNode*> uses;

    public:

        /** Create the attribute with no refs.   */
        HeapUses() : currentOp(NULL)
        {
        }

        HeapUses(SgNode* useNode, SgNode* sgn)
        {
            uses.push_back(useNode);
            currentOp = sgn;
        }

        /** Create the attribute with the def and list of uses.
         *
         * @param useTree The vector of uses to add, or an empty vector.
         */
        HeapUses(const std::vector<SgNode*>& useTree, SgNode* sgn = NULL)
        {
            if (useTree.size() > 0)
                uses.assign(useTree.begin(), useTree.end());
            currentOp = sgn;
        }

        /** Get the uses for this node and below.
         *
         * @return A constant reference to the use list.
         */
        std::vector<SgNode*>& getUses()
        {
            return uses;
        }

        /** Set the uses for this node and below.
         *
         * @param newUses A constant reference to the uses to copy to this node.
         */
        void setUses(const std::vector<SgNode*>& newUses)
        {
            uses.assign(newUses.begin(), newUses.end());
        }

        SgNode* getCurrentOp() const
        {
            return currentOp;
        }
    };

    /** This class collects all the defs and uses associated with each node in the traversed CFG.
     * Note that this does not compute reachability information; it just records each instance of
     * a heap operation used or defined. */
    class HeapDefsAndUsesTraversal : public AstBottomUpProcessing<HeapUses>
    {
    public:
      HeapSSA* hssa;

      //! If true, modifications to a value pointed to by a pointer will count as defs for the pointer itself.
      //! For example, (delete p) would be considered to modify p.
      const bool treatPointersAsStructs;
      
    public:
      
      //! @param treatPointersAsStructs If true, modifications to a value pointed to by a pointer will 
      //!     count as defs for the pointer itself. For example, (delete p) would be considered to modify p.
    HeapDefsAndUsesTraversal(HeapSSA* hssa, bool treatPointersAsStructs = true) : hssa(hssa),
	treatPointersAsStructs(treatPointersAsStructs)
        {
        }

      /** Called to evaluate the synthesized attribute on every node.
       *
       * This function will handle passing all variables that are defined and used by a given operation.
       *
       * @param node The node being evaluated.
       * @param attr The attributes from the child nodes.
       * @return The attribute at this node.
       */
      virtual HeapUses evaluateSynthesizedAttribute(SgNode* node, 
						    SynthesizedAttributesList attrs);
      
    protected:

        /** Mark all the uses as occurring at the specified node. */
        void addUsesToNode(SgNode* node, std::vector<SgNode*> uses);

        /** Mark the given variable as being defined at the node. */
        void addDefAtNode(SgNode* defNode, SgNode* heapDef);
    
	void addDPhiForNode(SgNode* defNode);

	void addHeapUse(SgExpression* sgn);

	void addHeapUseAndDef(SgExpression* sgn);
    };

} //namespace hssa_private
