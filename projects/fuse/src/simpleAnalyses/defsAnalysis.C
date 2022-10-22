//#include "defsAnalysis.h"
//#include "abstract_object.h"
//#include "stx_analysis.h"
//#include <ostream>
//#include "sight.h"
//using namespace sight;
//
//namespace fuse {
//
//#define defsDebugLevel 0
//
//  /************************
//   ***** DefsAnalysis *****
//   ************************/
//
//  void DefsAnalysis::addDef(SgNode* expr, PartEdgePtr pedge) {
//    MemLocObjectPtr ml = composer->Expr2MemLoc(expr, pedge, cdip);
//    SIGHT_VERB_DECL(scope, (txt()<<"DefsAnalysis::addDef(expr=" << SgNode2Str(expr) << ")"), 1, defsDebugLevel)
//    SIGHT_VERB(dbg << "        ml=" << ml->str() << endl, 1, defsDebugLevel)
//    MemRegionObjectPtr mr = ml->getRegion();
//    if (ml->getRegion()->isConcrete()) {
//      set<SgNode*> defSyms = ml->getRegion()->getConcrete();
//      for (set<SgNode*>::iterator d = defSyms.begin(); d != defSyms.end(); d++)
//        if (isSgVariableSymbol(*d)) {
//          SIGHT_VERB(dbg << "        Adding def " << SgNode2Str(isSgVariableSymbol(*d)) << " : " << vIDMap.variableId(isSgVariableSymbol(*d)).toString() << endl, 1, defsDebugLevel)
//          defs.insert(vIDMap.variableId(isSgVariableSymbol(*d)));
//        }
//    }
//  }
//
//  void DefsAnalysis::operator()(const CFGNode& n) {
//    SIGHT_VERB_DECL(scope, (txt() << "DefsAnalysis::operator(" << CFGNode2Str(n) << ")"), 1, defsDebugLevel)
//    std::set<PartEdgePtr> refined;
//    //collectRefinedEdges(composer, refined, n.outEdges());
//    collectOutgoingRefinedEdges(composer, refined, n);
//    SIGHT_VERB(dbg << "#refined=" << refined.size() << endl, 1, defsDebugLevel)
//
//    for (std::set<PartEdgePtr>::iterator r = refined.begin(); r != refined.end(); r++) {
//      SIGHT_VERB(dbg << "    edge=" << (*r)->str() << endl, 1, defsDebugLevel)
//      set < MemLocObjectPtr > defs;
//      if (isSgBinaryOp(n.getNode())) {
//        if (isSgAssignOp(n.getNode()) || isSgCompoundAssignOp(n.getNode()))
//          addDef(isSgBinaryOp(n.getNode())->get_lhs_operand(), *r);
//      } else if (isSgUnaryOp(n.getNode())) {
//        if (isSgPlusPlusOp(n.getNode()) || isSgMinusMinusOp(n.getNode()))
//          addDef(isSgUnaryOp(n.getNode())->get_operand(), *r);
//      } else if (isSgInitializedName(n.getNode())) {
//        addDef(isSgInitializedName(n.getNode()), *r);
//      }
//    }
//  }
//
//  void DefsAnalysis::runAnalysis(SgNode* target) {
//    if (isSgExprStatement(target)) {
//      runAnalysis(isSgExprStatement(target)->get_expression());
//    } else if (isSgReturnStmt(target)) {
//      runAnalysis(isSgReturnStmt(target)->get_expression());
//    } else if (isSgIfStmt(target)) {
//      runAnalysis(isSgIfStmt(target)->get_conditional());
//    } else if (isSgCastExp(target)) {
//      runAnalysis(isSgCastExp(target)->get_operand());
//    } else if (isSgVariableDeclaration(target)) {
//      const SgInitializedNamePtrList & declVars = isSgVariableDeclaration(
//          target)->get_variables();
//      for (SgInitializedNamePtrList::const_iterator var = declVars.begin();
//          var != declVars.end(); ++var) {
//        runAnalysis(*var);
//      }
//    } else if (isSgBasicBlock(target)) {
//      // Recursively call toString on the first statement in the block
//      const SgStatementPtrList & stmt =
//          isSgBasicBlock(target)->get_statements();
//      if (stmt.size() > 0)
//        runAnalysis(*stmt.begin());
//    } else if (isSgFunctionDeclaration(target) || isSgFunctionDefinition(target)
//        || isSgPragmaDeclaration(target)) {
//    } else if ((isSgExpression(target) && isSgVarRefExp(target))
//        || isSgInitializedName(target)) {
//      SIGHT_VERB_DECL(scope, (txt() << "DefsAnalysis::runAnalysis(" << SgNode2Str(target) << ")"), 1, defsDebugLevel)
//      /*CFGNode start;
//       if(isSgInitializedName(target)) start = CFGNode(target,  1);
//       else if(isSgBinaryOp(target)) start = CFGNode(target,  2);
//       else if(isSgUnaryOp(target)) {
//       if(isSgCastExp(target)) start = CFGNode(target,  0);
//       else if(isSgAddressOfOp(target) || isSgPointerDerefExp(target) || isSgPlusPlusOp(target) || isSgMinusMinusOp(target)) start = CFGNode(target,  1);
//       else                   start = CFGNode(target,  2);
//       }
//       else if(isSgValueExp(target))    start = CFGNode(target,  1);
//       else                        start = CFGNode(target,  0);*/
//      /*CFGNode start = cfgBeginningOfConstruct(target);
//       std::vector<CFGEdge> startOut = start.outEdges();
//       ROSE_ASSERT(startOut.size()>0);
//       std::vector<CFGEdge> startOutIn = startOut.begin()->target().inEdges();
//       ROSE_ASSERT(startOutIn.size()>0);
//       start = startOutIn.begin()->source();
//
//       CFGNode end = cfgEndOfConstruct(target);
//       std::vector<CFGEdge> endIn = end.inEdges();
//       ROSE_ASSERT(endIn.size()>0);
//       std::vector<CFGEdge> endInOut = endIn.begin()->target().outEdges();
//       ROSE_ASSERT(endInOut.size()>0);
//       end = endInOut.begin()->target();*/
////      cout << "start="<<CFGNode2Str(start)<<", end="<<CFGNode2Str(end)<<endl;
////      mapPath(*this, start, start);
//      StxPartPtr start;
//      /*if(isSgInitializedName(target))
//       start = StxPart::create(cfgEndOfConstruct(target), NULL);
//       else*/
//      start = StxPart::create(cfgBeginningOfConstruct(target), NULL);
//      SIGHT_VERB(dbg << "Initial start=" << start.str() << endl, 1, defsDebugLevel)
//
//      StxPartPtr end = StxPart::create(cfgEndOfConstruct(target), NULL);
//      SIGHT_VERB(dbg << "Initial end=" << end.str() << endl, 1, defsDebugLevel)
//
//      // Advance start to its successor if it is not identical to end
//      if(start != end) {
//        std::list<StxPartEdgePtr> startOut = start->outStxEdges();
//        ROSE_ASSERT(startOut.size() > 0);
//        SIGHT_VERB(dbg << "    start->out=" << (*startOut.begin())->str() << endl, 1, defsDebugLevel)
//        /*      std::list<StxPartEdgePtr> startOutIn = (*startOut.begin())->stxTarget()->inStxEdges();
//         ROSE_ASSERT(startOutIn.size()>0);
//         start = (*startOutIn.begin())->source();*/
//        start = (*startOut.begin())->stxTarget();
//        SIGHT_VERB(dbg << "    Final start=" << start.str() << endl, 1, defsDebugLevel)
//      }
//
//      // Set endNodes to be all the siblings of end (successors of end's predecessors)
//      std::set<CFGNode> endNodes;
//      std::list<StxPartEdgePtr> endIn = end->inStxEdges();
//      ROSE_ASSERT(endIn.size() > 0);
//      for (std::list<StxPartEdgePtr>::iterator in = endIn.begin(); in != endIn.end(); ++in) {
//        SIGHT_VERB(dbg << "    end->in=" << (*in)->str() << endl, 1, defsDebugLevel)
//        std::list<StxPartEdgePtr> endInOut = (*in)->stxSource()->outStxEdges();
//        SIGHT_VERB(dbg << "    endInOut(#" << endInOut.size() << ")" << endl, 1, defsDebugLevel)
//        ROSE_ASSERT(endInOut.size() > 0);
//        for (std::list<StxPartEdgePtr>::iterator inout = endInOut.begin(); inout != endInOut.end(); ++inout) {
//          SIGHT_VERB(dbg << "        inout=" << (*inout)->str() << endl, 1, defsDebugLevel)
//          StxPartPtr endTarget = (*inout)->target();
//          SIGHT_VERB(dbg << "        endTarget(#" << endTarget->CFGNodes().size() << ")="<< endTarget->str() << endl, 1, defsDebugLevel)
//          std::set<CFGNode> endTargetNodes = endTarget->CFGNodes();
//          for (std::set<CFGNode>::iterator cn = endTargetNodes.begin(); cn != endTargetNodes.end(); ++cn) {
//            endNodes.insert(*cn);
//            SIGHT_VERB(dbg << "        endNode=" << CFGNode2Str(*cn) << endl, 1, defsDebugLevel)
//          }
//        }
//      }
//
//      SIGHT_VERB_IF(1, defsDebugLevel)
//      dbg << "start=" << start.str() << ", end=" << end.str() << endl;
//      dbg << "endNodes=" << endl;
//      for (std::set<CFGNode>::iterator n = endNodes.begin(); n != endNodes.end(); ++n)
//        dbg << "      " << CFGNode2Str(*n) << endl;
//      SIGHT_VERB_FI()
//      std::set<CFGNode> startNodes = start->CFGNodes();
//      assert(startNodes.size() == 1);
//      //std::set<CFGNode> endNodes   = end->CFGNodes();   assert(endNodes.size()==1);
//      mapPath(*this, *startNodes.begin(), endNodes, /*includeEndNode*/
//          isSgInitializedName(target));
//    }
//  }
//
//  /******************************
//   ***** FuseRDAstAttribute *****
//   ******************************/
//
//// Maps labels in the AST to a reference to the set of defs at the label.
//  std::map<Label, VariableIdMapping::VariableIdSet> FuseRDAstAttribute::label2defsMap;
//// Alternate representation that keeps the same data a set of pairs
//  std::set<std::pair<Label, VariableId> > FuseRDAstAttribute::label2defsSet;
//
//// All the variables that were defined
//  std::set<VariableId> FuseRDAstAttribute::allDefs;
//
//// Maps each variableId to all of its definition labels
//  std::map<VariableId, LabelSet> FuseRDAstAttribute::def2labels;
//
//// All the labels that were annotated
//  LabelSet FuseRDAstAttribute::allLabelsSet;
//
//  FuseRDAstAttribute::FuseRDAstAttribute(Composer* composer,
//      checkDataflowInfoPass* cdip, VariableIdMapping& vIDMap, Label label,
//      Labeler& labeler) :
//      composer(composer), cdip(cdip), vIDMap(vIDMap) {
//    target = labeler.getNode(label);
//    DefsAnalysis analysis(composer, cdip, vIDMap, defs);
//    analysis.runAnalysis(labeler.getNode(label));
//    label2defsMap[label] = defs;
//    for (VariableIdMapping::VariableIdSet::iterator d = defs.begin();
//        d != defs.end(); d++) {
//      label2defsSet.insert(make_pair(label, *d));
//      allDefs.insert(*d);
//      def2labels[*d].insert(label);
//    }
//    allLabelsSet.insert(label);
//  }
//
//  FuseRDAstAttribute::~FuseRDAstAttribute() {
//  }
//
//  string FuseRDAstAttribute::toString() {
//    ostringstream s;
//    s << "[FuseRDAstAttribute: " << SgNode2Str(target) << ": #defs="
//        << defs.size() << ": ";
//    for (VariableIdMapping::VariableIdSet::iterator d = defs.begin();
//        d != defs.end(); d++) {
//      if (d != defs.begin())
//        s << ",";
//      s << vIDMap.getSymbol(*d)->get_name().getString();
//    }
//    s << "]";
//    return s.str();
//  }
//
//  void FuseRDAstAttribute::placeLabeler(Composer* composer,
//      checkDataflowInfoPass* cdip, VariableIdMapping& vIDMap,
//      Labeler& labeler) {
//    for (Labeler::iterator i = labeler.begin(); i != labeler.end(); ++i) {
//      SgNode* node = labeler.getNode(*i);
//      ROSE_ASSERT(node);
//      SIGHT_VERB_DECL(scope, (txt() << "Placing label at " << SgNode2Str(node)), 1, defsDebugLevel)
//      //node->setAttribute("fuse_cp_above", new ValueASTAttribute(node, composer, cdip, above, "fuse_cp_above"));
//      node->setAttribute("fuse_rd",
//          new FuseRDAstAttribute(composer, cdip, vIDMap, *i, labeler));
//    }
//  }
//
//
//FuseRDAstAttribute::FuseRDAstAttribute(Fuse& f, Label label, Labeler& labeler, VariableIdMapping& varIDMap) : f(f), label(label)
//{
//  // Convert the label to the most likely CFGNode it refers to
//  SgNode *sgn = labeler.getNode(label);
//  if(isSgInitializedName(sgn)) cn = CFGNode(sgn,  1);
//  else if(isSgBinaryOp(sgn))   cn = CFGNode(sgn,  2);
//  else if(isSgUnaryOp(sgn)) {
//    if(isSgCastExp(sgn))       cn = CFGNode(sgn,  0);
//    else if(isSgAddressOfOp(sgn) || isSgPointerDerefExp(sgn) || isSgPlusPlusOp(sgn) || isSgMinusMinusOp(sgn)) cn = CFGNode(sgn,  1);
//    else                       cn = CFGNode(sgn,  2);
//  }
//  else if(isSgValueExp(sgn))   cn = CFGNode(sgn,  1);
//  else                         cn = CFGNode(sgn,  0);
//
//  // Get the FuseCFGNodes at this label
//  ATSNodes = f.GetATSNodes(cn);
//
//  // Collect all the definitions at these FuseCFGNodes
//  for(set<FuseCFGNodePtr>::iterator i=ATSNodes.begin(); i!=ATSNodes.end(); ++i) {
//    const set<SSAMemLocObjectPtr>& curDefs = (*i)->getDefs();
//    defsML.insert(curDefs.begin(), curDefs.end());
//  }
//
//  defsVID = memlocs2varids(defsML, varIDMap)
//}
//
//FuseRDAstAttribute::~FuseRDAstAttribute();
//
//set<VariableId> memlocs2varids(VariableIdMapping& varIDMap) {
//  set<VariableId> varids;
//  for(set<SSAMemLocObjectPtr>::const_iterator ml=defsML.begin(); ml!=defsML.end(); ++ml) {
//    SgNode* curSGN = (*ml)->getBase();
//    if(SgInitializedName* iname = isSgInitializedName(curSGN))) {
//      VariableID vID(varIDMap.variableId(iname));
//      defsVID.insert(vID);
//      vid2ML[vID].insert(*ml);
//    } else if(SgVarRefExp* var = isSgVarRefExp(curSGN))) {
//      VariableID vID(varIDMap.variableId(var));
//      defsVID.insert(vID);
//      vid2ML[vID].insert(*ml);
//    }
//  }
//  return varids;
//}
//
//
//
//
//};
//// namespace fuse
//
