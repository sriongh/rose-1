#include "sage3basic.h"
using namespace std;

#include "live_dead_analysis.h"
#include "compose.h"
#include <boost/make_shared.hpp>

#ifndef DISABLE_SIGHT
using namespace sight;
#endif

namespace fuse {
#define liveDeadAnalysisDebugLevel 0


// ###############################
// ##### LiveDeadMemAnalysis #####
// ###############################

LiveDeadMemAnalysis::LiveDeadMemAnalysis(funcSideEffectUses* fseu):
    BWDataflow(/*trackBase2RefinedPartEdgeMapping*/ false, /*useSSA*/ false), fseu(fseu)
{
}

// Initializes the state of analysis lattices at the given function, part and edge into our out of the part
// by setting initLattices to refer to freshly-allocated Lattice objects.
void LiveDeadMemAnalysis::genInitLattice(const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                                         std::vector<Lattice*>& initLattices)
{
  AbstractObjectSet* s = new AbstractObjectSet(pedges.NodeState(), getComposer(), this, AbstractObjectSet::may);

  // If this part is the return statement of main(), make sure that its return value is live
  if(SgReturnStmt* returnStmt = parts.NodeState()->maySgNodeAny<SgReturnStmt>()) {
    SgFunctionDefinition* funcD = SageInterface::getEnclosingFunctionDefinition(returnStmt);
    if(SageInterface::findMain(SageInterface::getGlobalScope(returnStmt)) &&
       (funcD == SageInterface::findMain(SageInterface::getGlobalScope(returnStmt))->get_definition())) {
      // Get the memory location of the return statement's operand. Note that although this is a backward
      // analysis, the edge passed to OperandExpr2MemLoc() is the edge that comes into part. This is because
      // this edge denotes the set of executions that terminates at the return statement part and executions
      // always run forwards.
      // MemLocObjectPtrPair p(composer->OperandExpr2MemLoc(returnStmt, returnStmt->get_expression(), part->inEdgeFromAny(), this));
      // s->insert(p.expr? p.expr : p.mem);
      MemLocObjectPtr p(composer->OperandExpr2MemLoc(returnStmt, returnStmt->get_expression(), parts.NodeState()->inEdgeFromAny(), this));
      s->insert(p);
    }
  }

  initLattices.push_back(s);
}

/// Visits live expressions - helper to LiveDeadVarsTransfer
class LDMAExpressionTransfer : public ROSE_VisitorPatternDefaultBase
{
    LiveDeadMemTransfer &ldmt;

public:
    // Should only be called on expressions
    void visit(SgNode *) { assert(0); }
    // Catch up any other expressions that are not yet handled
    void visit(SgExpression * expr)
    {
        // Function Reference
        // !!! CURRENTLY WE HAVE NO NOTION OF VARIABLES THAT IDENTIFY FUNCTIONS, SO THIS CASE IS EXCLUDED FOR NOW
        /*} else if(isSgFunctionRefExp(sgn)) {*/
        /*} else if(isSgMemberFunctionRefExp(sgn)) {*/

        // !!! DON'T KNOW HOW TO HANDLE THESE
        /*} else if(isSgStatementExpression(sgn)) {(*/

        // Typeid
        // !!! DON'T KNOW WHAT TO DO HERE SINCE THE RETURN VALUE IS A TYPE AND THE ARGUMENT'S VALUE IS NOT USED
        /*} else if(isSgTypeIdOp(sgn)) {*/
        // Var Args
        // !!! DON'T HANDLE THESE RIGHT NOW. WILL HAVE TO IN THE FUTURE
        /*  SgVarArgOp
            SgExpression *  get_operand_expr () const
            SgVarArgCopyOp
            SgExpression *  get_lhs_operand () const
            SgExpression *  get_rhs_operand () const
            SgVarArgEndOp
            SgExpression *  get_operand_expr
            SgVarArgStartOneOperandOp
            SgExpression *  get_operand_expr () const
            SgVarArgStartOp
            SgExpression *  get_lhs_operand () const
            SgExpression *  get_rhs_operand () const */
        // !!! WHAT IS THIS?
        /*  SgVariantExpression*/


        // TODO: Make this assert(0), because unhandled expression types are likely to give wrong results
      assert(0);
    }
    // Plain assignment: lhs = rhs
    void visit(SgAssignOp *sgn) {
      ldmt.assign(sgn, sgn->get_lhs_operand());

      // If this expression or the value it writes are live, make the rhs live
      if(ldmt.isMemLocLive(sgn) || ldmt.isMemLocLive(sgn, sgn->get_lhs_operand())) {
        // If the lhs of the assignment is a complex expression (i.e. it refers to a variable that may be live) OR
        // if is a known expression that is known to may-be-live
        // THIS CODE ONLY APPLIES TO RHSs THAT ARE SIDE-EFFECT-FREE AND WE DON'T HAVE AN ANALYSIS FOR THAT YET
        /*if(!isVarExpr(sgn->get_lhs_operand()) ||
          (isVarExpr(sgn->get_lhs_operand()) &&
          liveLat->isLiveVar(SgExpr2Var(sgn->get_lhs_operand()))))
          { */
        ldmt.use(sgn, sgn->get_rhs_operand());
      }
    }
    // Initializer for a variable
    void visit(SgAssignInitializer *sgn) {
      // Only make the operand(s) live if the expression is live
      if(!ldmt.isMemLocLive(sgn)) return;

      SIGHT_VERB_IF(1, liveDeadAnalysisDebugLevel)
#ifndef DISABLE_SIGHT
        dbg << "LDMAExpressionTransfer::visit(SgAssignInitializer *sgn)"<<endl;
        dbg << "&nbsp;&nbsp;&nbsp;&nbsp;sgn="<<SgNode2Str(sgn)<<endl;
        dbg << "&nbsp;&nbsp;&nbsp;&nbsp;sgn->get_operand()="<<SgNode2Str(sgn->get_operand())<<endl;
#endif
      SIGHT_VERB_FI()
      ldmt.use(sgn, sgn->get_operand());
    }
    // Initializer for a function arguments
    void visit(SgConstructorInitializer *sgn) {
      // Only make the operand(s) live if the expression is live
      if(!ldmt.isMemLocLive(sgn)) return;

      SgExprListExp* exprList = sgn->get_args();
      for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
        ldmt.use(sgn, *expr);
    }
    // Initializer that captures internal stucture of structs or arrays ("int x[2] = {1,2};", it is the "1,2")
    // NOTE: Should this use abstractMemory interface ?
    void visit(SgAggregateInitializer *sgn) {
      // Only make the operand(s) live if the expression is live
      if(!ldmt.isMemLocLive(sgn)) return;

      SgExprListExp* exprList = sgn->get_initializers();
      for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
        ldmt.use(sgn, *expr);
    }
    // Designated Initializer
    void visit(SgDesignatedInitializer *sgn) {
      // Only make the operand(s) live if the expression is live
      if(!ldmt.isMemLocLive(sgn)) return;

      SgExprListExp* exprList = sgn->get_designatorList();
      for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
        ldmt.use(sgn, *expr);
    }
    // Variable access
    void visit(SgVarRefExp *sgn) {
//  Liao, 4/5/2012. We cannot decide if a SgVarRefExp is read or written
//    without its context information: for example, in  a = b; both a and b are represented as
//    SgVarRefExp. But a is written and b is read.
//    We should let the ancestor node (like SgAssignOp) decide on the READ/Written of SgVarRefExp.
//  GB 2012-09-04: We don't need to care about read/write access for this because the only way that a
//    variable reference can be live is if it is used, not assigned by its parent expression.

      //ldmt.useMem(sgn);
    }
    void visit(SgFunctionRefExp* sgn) {
      // TODO: This should be handled when we properly support CodeLocObjects
    }
    // Array access
    void visit(SgPntrArrRefExp *sgn) {
      //if(ldmt.isMemLocLive(sgn)) {
        SIGHT_VERB(dbg << "visit(SgPntrArrRefExp *sgn)"<<endl, 1, liveDeadAnalysisDebugLevel)
        // The only way for this SgPntrArrRefExp to appear s if it is used by its parent expression
        ldmt.useMem(sgn);
        // Both the lhs and rhs are used to identify the memory location being accessed
        SIGHT_VERB(dbg << "LHS"<<endl, 1, liveDeadAnalysisDebugLevel)
        ldmt.use(sgn, sgn->get_lhs_operand());
        SIGHT_VERB(dbg << "RHS"<<endl, 1, liveDeadAnalysisDebugLevel)
        ldmt.use(sgn, sgn->get_rhs_operand());
      //}
    }
    // Binary Operations
    void visit(SgBinaryOp *sgn) {
      /* GB 2012-09-05 : The liveness of the operand is not changed as a result of an auto-update
      // Self-update expressions, where the lhs is assigned
      if(isSgCompoundAssignOp(sgn))
        ldmt.assign(sgn, sgn->get_lhs_operand());*/

      SIGHT_VERB(dbg << "LiveDead: visit("<<SgNode2Str(sgn)<<") ldmt.isMemLocLive(sgn)="<<ldmt.isMemLocLive(sgn)<<endl, 1, liveDeadAnalysisDebugLevel)

      // If this expression is live or writes writes to a live memory location, make the operands live
      if(ldmt.isMemLocLive(sgn) || (isSgCompoundAssignOp(sgn) && ldmt.isMemLocLive(sgn, sgn->get_lhs_operand()))) {
        // Both the lhs and rhs are used
        ldmt.use(sgn, sgn->get_lhs_operand());
        ldmt.use(sgn, sgn->get_rhs_operand());
      }
    }
    // Unary Operations
    void visit(SgUnaryOp *sgn) {
      /* GB 2012-09-05 : The liveness of the operand is not changed as a result of an auto-update
       * // If this is an auto-update operation
      if(isSgMinusMinusOp(sgn) || isSgPlusPlusOp(sgn)) {
        // The argument is defined
        ldmt.assign(sgn, sgn->get_operand());
      }*/

        // If this expression is live or writes writes to a live memory location, make the operands live
      if(ldmt.isMemLocLive(sgn) ||
         ((isSgMinusMinusOp(sgn) || isSgPlusPlusOp(sgn)) && ldmt.isMemLocLive(sgn, sgn->get_operand())))
        // The argument is used
        ldmt.use(sgn, sgn->get_operand());
    }

    // Conditionals (condE ? trueE : falseE)
    void visit(SgConditionalExp *sgn) {
      // Only make the operand(s) live if the expression is live
      if(!ldmt.isMemLocLive(sgn)) return;

      // The arguments are used
      ldmt.use(sgn, sgn->get_conditional_exp());
      ldmt.use(sgn, sgn->get_true_exp());
      ldmt.use(sgn, sgn->get_false_exp());
    }
    // Delete
    void visit(SgDeleteExp *sgn) {
      // GB: 2012-09-05 - I don't think we need to process these in live-dead because a deleted memory location
      //      definitely cannot be used afterwards and must be dead at deletion time.

      // Delete expressions return nothing
      // The arguments are used
      //ldmt.use(sgn, sgn->get_variable());
    }
    // New
    void visit(SgNewExp *sgn) {
      // Only make the operand(s) live if the expression is live
      if(!ldmt.isMemLocLive(sgn)) return;

      // The placement arguments are used
      SgExprListExp* exprList = sgn->get_placement_args();
      // NOTE: placement args are optional
      // exprList could be NULL
      // check for NULL before adding to used set
      if(exprList) {
        for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
          ldmt.use(sgn, *expr);
      }

      // The placement arguments are used
      // check for NULL before adding to used set
      // not sure if this check is required for get_constructor_args()
      exprList = sgn->get_constructor_args()->get_args();
      if(exprList) {
        for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
          ldmt.use(sgn, *expr);
      }

      // The built-in arguments are used (DON'T KNOW WHAT THESE ARE!)
      // check for NULL before adding to used set
      // not sure if this check is required for get_builtin_args()
      if(sgn->get_builtin_args()) {
        ldmt.use(sgn, sgn->get_builtin_args());
      }
    }
    // Function Calls
    void visit(SgFunctionCallExp *sgn) {
      // The side-effects of calls to functions for which source is available are processed by inter-procedural
      // analyses. Here we need to consider the variable that identifies the function being called and the
      // side-effects of functions for which source is not available.

      // !!! CURRENTLY WE HAVE NO NOTION OF VARIABLES THAT IDENTIFY FUNCTIONS, SO THIS CASE IS EXCLUDED FOR NOW
      /*// The expression that identifies the called function is used
        ldmt.use(sgn, sgn->get_function());*/

      /*
      // If the function's source code is not available
      Function func(sgn);
      if(liveDeadAnalysisDebugLevel()>=1) dbg << "LiveDeadAnalysis::visit(SgFunctionCallExp *sgn) call to function "<<func.get_name().getString()<<"()"<<endl;
      if(!func.get_definition()) {
        if(liveDeadAnalysisDebugLevel()>=1) dbg << "LiveDeadAnalysis::visit(SgFunctionCallExp *sgn) Function "<<func.get_name().getString()<<"() with no definition."<<endl;

        // The function call's arguments are used
        SgExprListExp* exprList = sgn->get_args();
        for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
            expr!=exprList->get_expressions().end(); expr++)
            ldmt.use(sgn, *expr);

        / * // If this function has no definition and the user provided a class to provide
        // the variables that are used by such functions
        if(sgn->getAssociatedFunctionDeclaration() &&
           sgn->getAssociatedFunctionDeclaration()->get_definition()==NULL &&
           ldmt.fseu) {
            set<MemLocObjectPtr> funcUsedVars = ldmt.fseu->usedVarsInFunc(Function(sgn->getAssociatedFunctionDeclaration()), ldmt.part, ldmt.nodeState);
            //ldmt.use(sgn, funcUsedVars.begin(), funcUsedVars.end());
            for(set<MemLocObjectPtr>::iterator used=funcUsedVars.begin(); used!=funcUsedVars.end(); used++)
            ldmt.use(sgn, *used);
        }* /
      }*/
    }

    // Sizeof
    void visit(SgSizeOfOp *sgn) {
      // Only make the operand(s) live if the expression is live
      if(!ldmt.isMemLocLive(sgn)) return;

      // XXX: The argument is NOT used, but its type is
      // NOTE: get_operand_expr() returns NULL when sizeof(type)
      // FIX: use get_operand_expr() only when sizeof() involves expr
      // ldmt.use(sgn, sgn->get_operand_expr());
      if(sgn->get_operand_expr()) {
        ldmt.use(sgn, sgn->get_operand_expr());
      }
    }
    // This
    void visit(SgThisExp *sgn) {}
    // Literal values
    void visit(SgValueExp* sgn) {}

    LDMAExpressionTransfer(LiveDeadMemTransfer &ldmt)
    : ldmt(ldmt)
    { }
};

LiveDeadMemTransfer::LiveDeadMemTransfer(AnalysisParts& parts, CFGNode cn, NodeState &s,
                    std::map<PartEdgePtr, std::vector<Lattice*> > &dfInfo,
                    LiveDeadMemAnalysis* ldma,
                    Composer* composer, funcSideEffectUses *fseu)
    : DFTransferVisitor(parts, cn, s, dfInfo),
    ldma(ldma),
    composer(composer),
    modified(false),
    assigned(parts.NodeState()->inEdgeFromAny(), composer, (ComposedAnalysis*)ldma, AbstractObjectSet::may),
    used(parts.NodeState()->inEdgeFromAny(), composer, (ComposedAnalysis*)ldma, AbstractObjectSet::may),
    part(part),
    fseu(fseu)
{
  //#SA: Incoming dfInfo is associated with outEdgeToAny
  assert(dfInfo.find(part->outEdgeToAny()) != dfInfo.end());
  liveLat = dynamic_cast<AbstractObjectSet*>(*(dfInfo[parts.index()->outEdgeToAny()].begin()));

  SIGHT_VERB_IF(1, liveDeadAnalysisDebugLevel)
#ifndef DISABLE_SIGHT
    dbg << "LiveDeadMemTransfer: liveLat=";
    indent ind;
    dbg << liveLat->str("")<<endl;
#endif
  SIGHT_VERB_FI()
  // Make sure that all the lattice is initialized
  liveLat->initialize();
}

// Note that the variable corresponding to this expression is assigned
void LiveDeadMemTransfer::assign(SgNode *sgn, SgExpression* operand)
{
  //dbg << "LiveDeadMemTransfer::assign(sgn="<<SgNode2Str(sgn)<<" operand="<<SgNode2Str(operand)<<endl;
  // MemLocObjectPtrPair p(composer->OperandExpr2MemLoc(sgn, operand, part->outEdgeToAny(), ldma)/*ceml->Expr2Obj(sgn)*/);
  MemLocObjectPtr p(composer->OperandExpr2MemLoc(sgn, operand, parts.NodeState()->outEdgeToAny(), ldma)/*ceml->Expr2Obj(sgn)*/);
  //return boost::dynamic_pointer_cast<AbstractObject>(cpMap->get(ml));
  // if(p.expr) assigned.insert(p.expr);
  // // GB 2012-09-05 : Need this to handle lhs of SgAssignOps where the lhs is a memory location.
  // if(p.mem) assigned.insert(p.mem);
  assigned.insert(p);
}
/*void LiveDeadMemTransfer::assign(AbstractObjectPtr mem)
{
    assigned.insert(mem);
}*/

// Note that the variable corresponding to this expression is used
void LiveDeadMemTransfer::use(SgNode *sgn, SgExpression* operand)
{
  //dbg << "part->outEdgeToAny()="<<part->outEdgeToAny()->str()<<endl;
  // MemLocObjectPtrPair p = composer->OperandExpr2MemLoc(sgn, operand, part->outEdgeToAny(), ldma);//ceml->Expr2Obj(sgn);
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::use(sgn="<<SgNode2Str(sgn)<<", operand="<<SgNode2Str(operand)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  MemLocObjectPtr p = composer->OperandExpr2MemLoc(sgn, operand, parts.NodeState()->outEdgeToAny(), ldma);//ceml->Expr2Obj(sgn);
  //dbg << "LiveDeadMemTransfer::use(sgn=["<<escape(sgn->unparseToString())<<" | "<<sgn->class_name()<<"]"<<endl;
  //dbg << "p="<<p->str()<<endl;
  // In almost all cases we only need expressions to use their operands, which are also expressions.
  // if(p.expr) used.insert(p.expr);
  // At statement boundaries SgVarRefExp and SgArrPntrRefExp refer to real memory locations that were written by prior
  // statements.
  // if((isSgVarRefExp(operand) || isSgPntrArrRefExp(operand)) && p.mem)  used.insert(p.mem);
  // #SA
  // expression is either memory or temporary neither both
  // add the object returned by OperandExpr2MemLoc to used ??
  //
  if(p) used.insert(p);
}
// Note that the memory location denoted by the corresponding SgInitializedName is used
void LiveDeadMemTransfer::useMem(SgInitializedName* name)
{
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::useMem(name="<<SgNode2Str(name)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  //dbg << "name="<<SgNode2Str(name)<<endl;
  MemLocObjectPtr p = composer->Expr2MemLoc(name, parts.NodeState()->outEdgeToAny(), ldma);
  SIGHT_VERB(dbg << "LiveDeadMemTransfer::useMem(SgInitializedName)("<<SgNode2Str(name)<<")"<<endl, 1, liveDeadAnalysisDebugLevel)
  // used.insert(p.mem);
  used.insert(p);
}
// Note that the memory location denoted by the corresponding SgVarRefExp is used
void LiveDeadMemTransfer::useMem(SgVarRefExp* sgn)
{
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::useMem(name="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  // MemLocObjectPtrPair p = composer->Expr2MemLoc(sgn, part->outEdgeToAny(), ldma);//ceml->Expr2Obj(sgn);
  MemLocObjectPtr p = composer->Expr2MemLoc(sgn, parts.NodeState()->outEdgeToAny(), ldma);//ceml->Expr2Obj(sgn);
  SIGHT_VERB(dbg << "LiveDeadMemTransfer::useMem(SgVarRefExp)("<<SgNode2Str(sgn)<<")"<<endl, 1, liveDeadAnalysisDebugLevel)
  // used.insert(p.mem);
  used.insert(p);
}
// Note that the memory location denoted by the corresponding SgPntrArrRefExp is used
void LiveDeadMemTransfer::useMem(SgPntrArrRefExp* sgn)
{
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::useMem(sgn="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  // We use the SgPntrArrRefExp itself as well as all of its parent SgPntrArrRefExp because to reach
  // this index we need to access all the indexes that precede it in the expression, as well as the root
  // (array in array[1][2][3]) that identifies the base pointer.

  // !!! NOTE: This is not the best solution for two reasons:
  // !!! - In statically-allocated multi-dimensional arrays you don't actually use one dimension to access another
  // !!! - A more general treatment of this would build up referents of SgPntrArrRefExps incrementally, meaning that
  // !!!   this SgPntrArrRefExp would only use its immediate parent. However, to pull this off we'd need for
  // !!!   OrthogonalArrayAnalysis to also manage SgPntrArrRefExps incrementally, rather than a base address and a
  // !!!   multi-dimensional offset. This should be resolved when we move to an object-offset representation for
  // !!!   MemLocs that makes it easy to not explicitly maintain array indexes.

  SIGHT_VERB_IF(1, liveDeadAnalysisDebugLevel)
#ifndef DISABLE_SIGHT
    dbg << "LiveDeadMemTransfer::useMem(SgPntrArrRefExp)("<<SgNode2Str(sgn)<<")"<<endl;
    dbg << "LHS="<<SgNode2Str(sgn->get_lhs_operand())<<endl;
    dbg << "RHS="<<SgNode2Str(sgn->get_rhs_operand())<<endl;
#endif
  SIGHT_VERB_FI()
  MemLocObjectPtr p = composer->Expr2MemLoc(sgn, parts.NodeState()->outEdgeToAny(), ldma);
  // If a memory object is available, insert it. Not all SgPntrArrRefExps correspond to a real memory location.
  // e.g. in array2d[a][b] the expression array2D[a] doesn't denote a memory location if array2d is allocated statically.
  if(p) used.insert(p);

  /*do {
    if(liveDeadAnalysisDebugLevel()>=1) { dbg << "LHS="<<SgNode2Str(sgn->get_lhs_operand())<<endl; }
    MemLocObjectPtr p = composer->Expr2MemLoc(sgn->get_lhs_operand(), part->outEdgeToAny(), ldma);
    // If a memory object is available, insert it. Not all SgPntrArrRefExps correspond to a real memory location.
    // e.g. in array2d[a][b] the expression array2D[a] doesn't denote a memory location if array2d is allocated statically.
    if(p) used.insert(p);
    sgn = isSgPntrArrRefExp(sgn->get_lhs_operand());
  } while(sgn);*/
}

// Returns true if the given expression is currently live and false otherwise
bool LiveDeadMemTransfer::isMemLocLive(SgExpression* sgn, SgExpression* operand) {
  // MemLocObjectPtrPair p = composer->OperandExpr2MemLoc(sgn, operand, part->outEdgeToAny(), ldma);//ceml->Expr2Obj(expr);
  // return (p.expr ? liveLat->containsMay(p.expr) : false) ||
  //        (p.mem  ? liveLat->containsMay(p.mem)  : false);
  MemLocObjectPtr p = composer->OperandExpr2MemLoc(sgn, operand, parts.NodeState()->outEdgeToAny(), ldma);//ceml->Expr2Obj(expr);
  return (p ? liveLat->containsMay(p) : false);

}

// Returns true if the given expression is currently live and false otherwise
bool LiveDeadMemTransfer::isMemLocLive(SgExpression* sgn) {
  // MemLocObjectPtrPair p = composer->Expr2MemLoc(sgn, part->outEdgeToAny(), ldma);//ceml->Expr2Obj(expr);
  // return (p.expr ? liveLat->containsMay(p.expr) : false) ||
  //        (p.mem  ? liveLat->containsMay(p.mem)  : false);
#ifndef DISABLE_SIGHT
dbg << "LiveDeadMemTransfer::isMemLocLive("<<SgNode2Str(sgn)<<")"<<endl;
dbg << "liveLat="<<liveLat->str()<<endl;
#endif
  MemLocObjectPtr p = composer->Expr2MemLoc(sgn, parts.NodeState()->outEdgeToAny(), ldma);//ceml->Expr2Obj(expr);
#ifndef DISABLE_SIGHT
dbg << "p="<<p->str()<<endl;
#endif
  return (p ? liveLat->containsMay(p) : false);
}

void LiveDeadMemTransfer::visit(SgExpression *sgn)
{
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::visit(sgn="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)

  //AbstractMemoryObject::ObjSet* objset = SgExpr2ObjSet(sgn);
  // MemLocObjectPtrPair p = composer->Expr2MemLoc(sgn, part->outEdgeToAny(), ldma);//ceml->Expr2Obj(sgn);
  MemLocObjectPtr p = composer->Expr2MemLoc(sgn, parts.NodeState()->outEdgeToAny(), ldma);//ceml->Expr2Obj(sgn);
  LDMAExpressionTransfer helper(*this);
  sgn->accept(helper);

  // Remove the expression object itself since it has no uses above itself.
  // Do not remove the memory location object since it has just been used.
  /*if(!isSgVarRefExp(sgn) && !isSgPntrArrRefExp(sgn)) // Liao 4/5/2012, we should not remove SgVarRef or SgPntrArrRefExp since it may have uses above itself
  {
    //if(liveDeadAnalysisDebugLevel()>=1) dbg << "   Removing "<< mem.str("         ") <<endl;
    //modified = liveLat->remove(mem) || modified;
    if(assigned.insert(mem); /// ????
  }*/
  // if(p.expr) assigned.insert(p.expr);
  // #SA
  // Should we discard expressions that are memory here ?
  // # GB
  // No, we should not. Fixed.
  if(!MemLocObject::isMemExpr(sgn) && p) assigned.insert(p);
}

void LiveDeadMemTransfer::visit(SgInitializedName *sgn) {
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::visit(sgn="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  /*SgVarRefExp* exp = SageBuilder::buildVarRefExp(sgn);
  dbg << "LiveDeadMemTransfer::visit(SgInitializedName: sgn=["<<escape(sgn->unparseToString())<<" | "<<sgn->class_name()<<"]"<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;exp="<<exp<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;exp=["<<escape(exp->unparseToString())<<" | "<<exp->class_name()<<"]"<<endl;*/
  // MemLocObjectPtrPair p = composer->Expr2MemLoc(sgn, part->outEdgeToAny(), ldma); //ceml->Expr2Obj(exp);
  MemLocObjectPtr p = composer->Expr2MemLoc(sgn, parts.NodeState()->outEdgeToAny(), ldma); //ceml->Expr2Obj(exp);
  assert(p);
  bool isLive = (p  ? liveLat->containsMay(p)  : false);
  /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;isLive="<<isLive<<endl;
  if(liveDeadAnalysisDebugLevel()>=1)
    dbg << indent << (isLive ? "Live Expression" : "Dead Expression") <<endl;*/

  // If this is a live variable and this is the instance of SgInitializedName that occurs immediately after
  // the declaration's initializer AND this declaration has an initializer, add it as a use
  if(isLive && sgn->get_initializer())
    use(sgn, sgn->get_initializer());
  //if(p.expr) assigned.insert(p.expr);
  //if(p.mem)  assigned.insert(p.mem);
  // assigned.insert(p.mem);
  assigned.insert(p);
}

void LiveDeadMemTransfer::visit(SgReturnStmt *sgn) {
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::visit(sgn="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
/*
  WE CURRENTLY ASSUME THAT THE EXPRESSION OF A RETURN STATEMENT IS ALWAYS USED
  SHOULD ONLY ASSUME THIS FOR RETURN STA```TEMENT OF main()*/

  use(sgn, sgn->get_expression());
}
void LiveDeadMemTransfer::visit(SgExprStatement *sgn) {
    /*use(sgn, sgn->get_expression());*/
}
void LiveDeadMemTransfer::visit(SgSwitchStatement *sgn) {
  // The operand of a control-flow statement is automatically live only
  // if the control-flow statement leads to multiple control options
  // This is always true for switch statements since they can always skip all the cases.
  // However, in the future may want to check for whether the value of the switch is a constant.
  // GB 2012-09-19 : Which object corresponds to the expression that chooses the case?
  assert(0);
  //use(sgn, sgn->get_item_selector());
}
void LiveDeadMemTransfer::visit(SgCaseOptionStmt *sgn) {
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::visit(sgn="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  use(sgn, sgn->get_key());
  use(sgn, sgn->get_key_range_end());
}
void LiveDeadMemTransfer::visit(SgIfStmt *sgn) {
  // The operand of a control-flow statement is automatically live only
  // if the control-flow statement leads to multiple control options
  // This is always true for if statements since there's a difference between the true body executing and not executing
  // even if there is no false body.
  // However, in the future may want to check for whether the value of the switch is a constant.

  assert(isSgExprStatement(sgn->get_conditional()));
  use(sgn, isSgExprStatement(sgn->get_conditional())->get_expression());
}

void LiveDeadMemTransfer::visit(SgForStatement *sgn) {
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::visit(sgn="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  //dbg << "test="<<escape(sgn->get_test()->unparseToString()) << " | " << sgn->get_test()->class_name()<<endl;
  //dbg << "increment="<<escape(sgn->get_increment()->unparseToString()) << " | " << sgn->get_increment()->class_name()<<endl;

  // The operands of a for statement are automatically live since loops have an unknown
  // number of iterations that is decided based on these operands
  assert(isSgExprStatement(sgn->get_test()));
  use(sgn, isSgExprStatement(sgn->get_test())->get_expression());
  use(sgn, sgn->get_increment());
}
void LiveDeadMemTransfer::visit(SgWhileStmt *sgn) {
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::visit(sgn="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  // The operands of a while statement are automatically live since loops have an unknown
  // number of iterations that is decided based on these operands
  assert(isSgExprStatement(sgn->get_condition()));
  //dbg << "condition="<<escape(sgn->get_condition()->unparseToString()) << " | " << sgn->get_condition()->class_name()<<endl;
  use(sgn, isSgExprStatement(sgn->get_condition())->get_expression());
}
void LiveDeadMemTransfer::visit(SgDoWhileStmt *sgn) {
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::visit(sgn="<<SgNode2Str(sgn)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  // The operands of a do-while statement are automatically live since loops have an unknown
  // number of iterations that is decided based on these operands
  assert(isSgExprStatement(sgn->get_condition()));
  //dbg << "condition="<<escape(sgn->get_condition()->unparseToString()) << " | " << sgn->get_condition()->class_name()<<endl;
  use(sgn, isSgExprStatement(sgn->get_condition())->get_expression());
}
void LiveDeadMemTransfer::visit(SgFunctionDefinition* def) {
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::visit(sgn="<<SgNode2Str(def)<<")", scope::medium), 1, liveDeadAnalysisDebugLevel)
  Function func(def);
  //dbg << "Definition "<<SgNode2Str(def)<<" attributeExists="<<def->attributeExists("fuse:UnknownSideEffects")<<endl;

  // The function's parameters are used
  SgFunctionParameterList* params = func.get_params();
  for(SgInitializedNamePtrList::iterator p=params->get_args().begin(); p!=params->get_args().end(); p++)
    useMem(*p);
}
bool LiveDeadMemTransfer::finish()
{
  SIGHT_VERB(scope reg(txt()<<"LiveDeadMemTransfer::finish()", scope::medium), 1, liveDeadAnalysisDebugLevel)

  // First process assignments, then uses since we may assign and use the same variable
  // and in the end we want to first remove it and then re-insert it.
  SIGHT_VERB_IF(1, liveDeadAnalysisDebugLevel)
#ifndef DISABLE_SIGHT
    dbg << "used="<<endl;
    { indent ind;
    for(AbstractObjectSet::const_iterator asgn=used.begin(); asgn!=used.end(); asgn++) {
      dbg << (*asgn)->str()<<endl;
    } }

    dbg << "assigned="<<endl;
    { indent ind;
    for(AbstractObjectSet::const_iterator asgn=assigned.begin(); asgn!=assigned.end(); asgn++) {
      dbg << (*asgn)->str() << endl;
    } }

    dbg << "liveLat="<<endl;
    {indent ind;
     dbg << liveLat->str("")<<endl;}
#endif
  SIGHT_VERB_FI()

  /* Live-In (node) = use(node) + (Live-Out (node) - Assigned (b))
   * Live-Out (node) is the lattice after merging ???
   * */
  // Record for each assigned expression:
  //    If the expression corresponds to a variable, record that the variable is dead.
  //    Otherwise, record that the expression that computes the assigned memory location is live
  for(AbstractObjectSet::const_iterator asgn=assigned.begin(); asgn!=assigned.end(); asgn++) {
    // If the lhs is a variable reference, remove it from live variables unless we also use this variable
    if(!used.containsMay(*asgn)) // if not found in use, then remove it, Liao 4/5/2012
    {
      // if(liveDeadAnalysisDebugLevel()>=1) {
      //     dbg << indent << "    removing assigned expr <" << (*asgn)->class_name() <<":"<<(*asgn)->unparseToString();
      //     dbg << ">"<<endl;
      // }
      modified = liveLat->remove(*asgn) || modified;
    }
    else
    {
      modified = liveLat->insert(*asgn) || modified;
      // if(liveDeadAnalysisDebugLevel()>=1) {
      //     dbg << indent << "    add assigned expr as live <" << (*asgn)->class_name() <<":"<<(*asgn)->unparseToString();
      // }
    }
  } // end for

  // Record that the used variables are live
  for(AbstractObjectSet::const_iterator var=used.begin(); var!=used.end(); var++)
    modified = liveLat->insert(*var) || modified;

  return modified;
}

/*******************************
 ***** LiveDeadMemAnalysis *****
 *******************************/

// Maps the given SgNode to an implementation of the MemLocObject abstraction.
MemLocObjectPtr LiveDeadMemAnalysis::Expr2MemLoc(SgNode* n, PartEdgePtr pedge)
{
  // MemLocObjectPtrPair p = composer->Expr2MemLoc(n, pedge, this);
  // dbg << "LiveDeadMemAnalysis::Expr2MemLoc() p="<<p.strp(pedge)<<endl;
  // if(p.mem) return createLDMemLocObjectCategory(n, p.mem, this);
  // else      return p.expr;
  MemLocObjectPtr p = composer->Expr2MemLoc(n, pedge, this);
  SIGHT_VERB(dbg << "LiveDeadMemAnalysis::Expr2MemLoc() p="<<p->strp(pedge)<<endl, 1, liveDeadAnalysisDebugLevel)
  // #SA
  // createLDMemLocObject for objects returned by composer for now
  return boost::make_shared<LDMemLocObject>(n, p, this);
}

/**************************
 ***** LDMemLocObject *****
 **************************/
LDMemLocObject::LDMemLocObject(SgNode* n, MemLocObjectPtr parent_, LiveDeadMemAnalysis* ldma)
  : MemLocObject(n), parent(parent_), ldma(ldma)
{
}

LDMemLocObject::LDMemLocObject(const LDMemLocObject& that) :
    MemLocObject(that), parent(that.parent), ldma(that.ldma)
{}

bool LDMemLocObject::mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge)
{
  LDMemLocObjectPtr that = boost::dynamic_pointer_cast<LDMemLocObject>(o);
  bool isThisLive = isLiveML(pedge);
  bool isThatLive = that->isLiveML(pedge);

  /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;LDMemLocObject::mayEqual"<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;this (live="<<isThisLive<<")="<<str("")<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;that (live="<<isThatLive<<")="<<o->str("")<<endl;*/

  if(!that) { /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;FALSE"<<endl;*/ return false; }
  //dbg << "    isThisLive="<<isThisLive<<" isThisLive="<<isThisLive<<endl;
  // If both objects may be live, use the parents' equality operator
  if(isThisLive && isThatLive) {
    bool tmp=parent->mayEqual(that->parent, pedge, ldma->getComposer(), ldma);
    //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;"<<(tmp?"TRUE":"FALSE")<<endl;
    return tmp;
  // If both objects are definitely not live, they're counted as being equal
  } else if(!isThisLive && !isThatLive) {
     //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;TRUE"<<endl;
     return true;
  // Otherwise, they're in different classes and thus unequal
  } else
  { /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;FALSE"<<endl;*/ return false; }
}

bool LDMemLocObject::mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge)
{
  LDMemLocObjectPtr that = boost::dynamic_pointer_cast<LDMemLocObject>(o);
  bool isThisLive = isLiveML(pedge);
  bool isThatLive = that->isLiveML(pedge);

  /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;LDMemLocObject::mustEqual"<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;this (live="<<isThisLive<<")="<<str("")<<endl;
  dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;that (live="<<isThatLive<<")="<<o->str("")<<endl;/*/

  if(!that) { /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;FALSE"<<endl;*/ return false; }
  // If both objects may be live, use the parents' equality operator
  if(isThisLive && isThisLive) {
    bool tmp=parent->mustEqual(that->parent, pedge, ldma->getComposer(), ldma);
    //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;"<<(tmp?"TRUE":"FALSE")<<endl;
    return tmp;
  // If both objects are definitely not live, they're counted as being equal
  } else if(!isThisLive && !isThatLive) {
    //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;TRUE"<<endl;
    return true;
  // Otherwise, they're in different classes and thus unequal
  } else
  { /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;FALSE"<<endl;*/ return false; }
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool LDMemLocObject::equalSet(AbstractObjectPtr o, PartEdgePtr pedge)
{
  LDMemLocObjectPtr that = boost::dynamic_pointer_cast<LDMemLocObject>(o);
  bool isThisLive = isLiveML(pedge);
  bool isThatLive = that->isLiveML(pedge);

  if(!that) { /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;FALSE"<<endl;*/ return false; }
  // If both objects may be live, use the parents' equality operator
  if(isThisLive && isThisLive) {
    bool tmp=parent->equalSet(that->parent, pedge, ldma->getComposer(), ldma);
    //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;"<<(tmp?"TRUE":"FALSE")<<endl;
    return tmp;
  // If both objects are definitely not live, they're counted as being equal
  } else if(!isThisLive && !isThatLive) {
    //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;TRUE"<<endl;
    return true;
  // Otherwise, they're in different classes and thus unequal
  } else
  { /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;FALSE"<<endl;*/ return false; }
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool LDMemLocObject::subSet(AbstractObjectPtr o, PartEdgePtr pedge) {
  LDMemLocObjectPtr that = boost::dynamic_pointer_cast<LDMemLocObject>(o);
  bool isThisLive = isLiveML(pedge);
  bool isThatLive = that->isLiveML(pedge);

  if(!that) { /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;FALSE"<<endl;*/ return false; }
  // If both objects may be live, use the parents' equality operator
  if(isThisLive && isThisLive) {
    bool tmp=parent->subSet(that->parent, pedge, ldma->getComposer(), ldma);
    //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;"<<(tmp?"TRUE":"FALSE")<<endl;
    return tmp;
  // If both objects are definitely not live, they're counted as being equal
  } else if(!isThisLive && !isThatLive) {
    //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;TRUE"<<endl;
    return true;
  // Otherwise, they're in different classes and thus disjoint
  } else
  { /*dbg << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;==&gt;FALSE"<<endl;*/ return false; }
}

// Returns true if this object is live at the given part and false otherwise
bool LDMemLocObject::isLiveML(PartEdgePtr pedge)
{
  //dbg << "LDMemLocObject::isLive() pedge="<<pedge->str()<<endl;
  //dbg << "parent str="<<parent->str()<<endl;
  //dbg << "parent strp="<<parent->strp(pedge)<<endl;
  //dbg << "live="<<isLiveMay(parent, ldma, pedge, "")<<endl;
  //dbg << "state="<<NodeState::getNodeState(ldma, pedge->target())->str(ldma)<<endl;*/

  // The MemLocObject for the SgFunctionDefinition is special since it denotes the return values of a function
  // It should always be live.
  /*MemLocObject funcDeclML =

          getComposer()->Expr2MemLoc(SageInterface::getEnclosingProcedure(parent)
          func.get_declaration()->search_for_symbol_from_symbol_table(), part->inEdgeFromAny(), analysis),

  if(parent->isFunctionMemLoc()) return true;
  else */
  bool live;
  std::map<PartEdgePtr, bool>::iterator liveIt = isLive_cache.find(pedge);
  if(liveIt == isLive_cache.end()) {
    live = isLiveMay(parent, ldma, pedge, "");
    isLive_cache[pedge]=live;
  }  else
    live = liveIt->second;
  return live;
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool LDMemLocObject::meetUpdateML(MemLocObjectPtr o, PartEdgePtr pedge)
{
  LDMemLocObjectPtr that = boost::dynamic_pointer_cast<LDMemLocObject>(o);
  assert(that);
  return parent->meetUpdate(that->parent, pedge, ldma->getComposer(), ldma);
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool LDMemLocObject::isFullML(PartEdgePtr pedge)
{
  return parent->isFull(pedge, ldma->getComposer(), ldma);
}

// Returns whether this AbstractObject denotes the empty set.
bool LDMemLocObject::isEmptyML(PartEdgePtr pedge)
{
  return parent->isEmpty(pedge, ldma->getComposer(), ldma);
}

// pretty print for the object
std::string LDMemLocObject::str(std::string indent) const
{
  ostringstream oss;

  // Choose the string description to use based on the sub-type of this LDMemLocObject
  string name = "LDML";
  /*     if(dynamic_cast<const LDScalar*>(this))         name = "LDScalar";
  else if(dynamic_cast<const LDFunctionMemLoc*>(this)) name = "LDFunctionMemLoc";
  else if(dynamic_cast<const LDArray*>(this))          name = "LDArray";
  else if(dynamic_cast<const LDPointer*>(this))        name = "LDPointer";*/

  oss << "["<<name<<": "<<parent->str("    ")<<"]";
  return oss.str();
}

std::string LDMemLocObject::strp(PartEdgePtr pedge, std::string indent)
{
  ostringstream oss;
  if(isLiveML(pedge))
    oss << "[LDMemLocObject: LIVE: "<<parent->str("    ")<<"]";
  else
    oss << "[LDMemLocObject: DEAD]";
  return oss.str();
}

// Allocates a copy of this object and returns a pointer to it
MemLocObjectPtr LDMemLocObject::copyML() const
{
  return boost::make_shared<LDMemLocObject>(*this);
}

// Initialize vars to hold all the variables and expressions that are live at PartEdgePtr pedge
void getAllLiveMemAt(LiveDeadMemAnalysis* ldma, PartEdgePtr pedge, const NodeState& state, set<AbstractObjectPtr>& vars, string indent)
{
  AbstractObjectSet* liveL = dynamic_cast<AbstractObjectSet*>(state.getLatticeAbove(ldma, pedge, 0));
  assert(liveL);
  for(AbstractObjectSet::const_iterator var=liveL->begin(); var!=liveL->end(); var++)
    vars.insert(*var);
}

// Returns the set of variables and expressions that are live at PartEdgePtr pedge
set<AbstractObjectPtr> getAllLiveMemAt(LiveDeadMemAnalysis* ldma, PartEdgePtr pedge, const NodeState& state, string indent)
{
    set<AbstractObjectPtr> vars;
    getAllLiveMemAt(ldma, pedge, state, vars, indent);
    return vars;
}

// Returns true if the given MemLocObject must be live at the given PartEdgePtr pedge
bool isLiveMust(MemLocObjectPtr mem, LiveDeadMemAnalysis* ldma, PartEdgePtr pedge, NodeState& state, string indent)
{
  AbstractObjectSet* liveL = dynamic_cast<AbstractObjectSet*>(state.getLatticeAbove(ldma, pedge, 0));
  assert(liveL);
  if(liveL->containsMust(mem)) return true;

  return false;
}

// Returns true if the given MemLocObject may be live at the given PartEdgePtr pedge
bool isLiveMay(MemLocObjectPtr mem, LiveDeadMemAnalysis* ldma, PartEdgePtr pedge, string indent)
{
  SIGHT_VERB(scope reg("isLiveMay()", scope::medium), 1, liveDeadAnalysisDebugLevel)
  SIGHT_VERB_IF(1, liveDeadAnalysisDebugLevel)
#ifndef DISABLE_SIGHT
    dbg << "mem="<<mem->str("")<<endl;
    dbg << "pedge="<<pedge->str()<<endl;
#endif
  SIGHT_VERB_FI()

  AnalysisPartEdges pedges = ldma->NodeState2All(pedge);

  // If this is not a wildcard edge, check if mem is live along it
  if(pedge->source() && pedge->target()) {
    NodeState* state = NodeState::getNodeState(ldma, pedge->target());
    //dbg << "state="<<state->str(ldma)<<endl;

    AbstractObjectSet* liveL = dynamic_cast<AbstractObjectSet*>(state->getLatticeAbove(ldma, pedges.index(), 0)); assert(liveL);
    SIGHT_VERB(dbg << "isLiveMay: liveLAbove="<<liveL->str("")<<endl, 1, liveDeadAnalysisDebugLevel)
    if(liveL->containsMay(mem)) {
      SIGHT_VERB(dbg << "<b>LIVE</b>"<<endl, 1, liveDeadAnalysisDebugLevel)
      return true;
    }
  // If the source of this edge is a wildcard, mem is live if it is live along any incoming edge or
  // the outgoing edge (for SgInitializedName we want to say that it is live immediately before its
  // declaration so that clients can ask for its MemLocObject at edges coming into the declaration).
  } else if(pedge->target()) {
    NodeState* state = NodeState::getNodeState(ldma, pedge->target());
    //dbg << "state="<<state->str(ldma)<<endl;

    {SIGHT_VERB(scope regAbv("Checking containment above", scope::low), 1, liveDeadAnalysisDebugLevel)
    map<PartEdgePtr, std::vector<Lattice*> >& e2lats = state->getLatticeAboveAllMod(ldma);
    assert(e2lats.size()>=1);
    for(map<PartEdgePtr, std::vector<Lattice*> >::iterator lats=e2lats.begin(); lats!=e2lats.end(); lats++) {
      PartEdge* p = lats->first.get();
      assert(p->target() == pedge.get()->target());
      AbstractObjectSet* liveL = dynamic_cast<AbstractObjectSet*>(*(lats->second.begin())); assert(liveL);
      SIGHT_VERB(scope regAbvEdge(txt()<<"edge="<<p->str(), scope::low), 1, liveDeadAnalysisDebugLevel)
      SIGHT_VERB(dbg << "liveLAbove="<<liveL->str("")<<endl, 1, liveDeadAnalysisDebugLevel)

      if(liveL->containsMay(mem)) {
        SIGHT_VERB(dbg << "<b>LIVE</b>"<<endl, 1, liveDeadAnalysisDebugLevel)
        return true;
      }
    }}

    { SIGHT_VERB(scope regAbv("Checking containment below", scope::low), 1, liveDeadAnalysisDebugLevel)
    PartPtr curIndexPart = pedges.index()->target();
    //#SA: query for dfInfo on outEdgeToAny corresponging to current part
    std::vector<Lattice*>& lats = state->getLatticeBelowMod(ldma, curIndexPart->outEdgeToAny());
    AbstractObjectSet* liveL = dynamic_cast<AbstractObjectSet*>(*(lats.begin())); assert(liveL);
    SIGHT_VERB(dbg << "isLiveMay: liveLBelow="<<liveL->str("")<<endl, 1, liveDeadAnalysisDebugLevel)
    //dbg << "isLiveMay: liveLBelow="<<liveL->str("")<<endl;
    if(liveL->containsMay(mem)) {
      SIGHT_VERB(dbg << "<b>LIVE</b>"<<endl, 1, liveDeadAnalysisDebugLevel)
      return true;
    }}
  // If the target of this edge is a wildcard, mem is live if it is live along any outgoing edge
  } else if(pedge->source()) {
    NodeState* state = NodeState::getNodeState(ldma, pedges.NodeState()->source());
    //dbg << "isLiveMay: state="<<state->str(ldma)<<endl;

    //#SA: dfInfo is associated with outEdgeToAny
    std::vector<Lattice*>& lats = state->getLatticeBelowMod(ldma, pedges.index());
    AbstractObjectSet* liveL = dynamic_cast<AbstractObjectSet*>(*(lats.begin()));
    assert(liveL);
    SIGHT_VERB(dbg << "isLiveMay: liveLBelow="<<liveL->str("")<<endl, 1, liveDeadAnalysisDebugLevel)
    if(liveL->containsMay(mem)) {
      SIGHT_VERB(dbg << "<b>LIVE</b>"<<endl, 1, liveDeadAnalysisDebugLevel)
      return true;
    }
  }

  SIGHT_VERB(dbg << "<b>DEAD</b>"<<endl, 1, liveDeadAnalysisDebugLevel)
  return false;
}

}; // namespace fuse
