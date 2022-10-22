#include "sage3basic.h"

using namespace std;

#include "analysis.h"
#include "composed_analysis.h"
#include "compose.h"
#include "printAnalysisStates.h"
#include "const_prop_count.h"

#ifndef DISABLE_SIGHT
#include "ats_graph_structure.h"
#endif

namespace fuse {
  ConstPropCountAnalysis::ConstPropCountAnalysis() :FWDataflow(false, false),
						    m_assignExpCount(0),
						    m_unassignExpCount(0),
						    m_knownCondCount(0),
						    m_unknownCondCount(0),
						    m_edges(0) {
    
    nodess.clear(); edgess.clear();        
    // nodess << "\n subgraph cluster {\n";
    indent = "    ";
  }

  ComposedAnalysisPtr ConstPropCountAnalysis::copy() {
    // return boost::make_shared<ConstPropCountAnalysis>(*this);
    return boost::make_shared<ConstPropCountAnalysis>();
  }

  ValueObjectPtr ConstPropCountAnalysis::Expr2Val(SgNode* sgn, PartEdgePtr pedge) {
    assert(false);
  }

  bool ConstPropCountAnalysis::implementsExpr2Val() {
    return false;
  }

  void ConstPropCountAnalysis::genInitLattice(/*PartPtr part, PartEdgePtr pedge, PartPtr supersetPart,*/
					      const AnalysisParts& parts, const AnalysisPartEdges& pedges,
					      std::vector<Lattice*>& initLattices) {
    initLattices.push_back((Lattice*)(new BoolAndLattice(0, pedges.NodeState())));
  }

  bool ConstPropCountAnalysis::countConst(AnalysisParts& parts, CFGNode cn, NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    // set<CFGNode>& nodes = parts.NodeState()->CFGNodes();
    // for(set<CFGNode>::iterator n=nodes.begin(); n!=nodes.end(); n++) {
    //   // if visiting for first time
    //   // collect statistics only once
    //   SgAssignOp* assignOp = isSgAssignOp(n->getNode());
    //   if(assignOp && m_visited.find(assignOp) == m_visited.end()) {
    //     m_visited.insert(assignOp);
    //     // std::cout << assignOp << SgNode2Str(assignOp) << std::endl;
    //     ValueObjectPtr v = getComposer()->OperandExpr2Val(assignOp, assignOp, parts.NodeState()->outEdgeToAny(), this);
    //     // cout << v->str() << endl;
    //     if(v->isConcrete()) {
    //       m_assignExpCount++;
    //     }
    //     else {
    //       m_unassignExpCount++;
    //     }
    //   }
    //   else if(SgIfStmt* ifstmt = isSgIfStmt(n->getNode())) {
    //     SgExprStatement* exprStmt = isSgExprStatement(ifstmt->get_conditional());
    //     if(exprStmt) {
    //       SgExpression* expr = exprStmt->get_expression();
    //       ValueObjectPtr v = getComposer()->OperandExpr2Val(expr, expr, parts.NodeState()->outEdgeToAny(), this);
    //       if(v->isConcrete()) {
    //         m_knownCondCount++;
    //       }
    //       else {
    //         m_unknownCondCount++;
    //       }
    //     }
    //     else {
    //       cerr << "Warning: SgIfStmt()->get_conditional() != SgExprStatement\n";
    //     }
    //   }
    // }
  }
  
  bool ConstPropCountAnalysis::transfer(AnalysisParts& parts, CFGNode cn, NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    m_edges++;

    PartPtr part = parts.index();
    list<PartEdgePtr> outEdges = parts.index()->outEdges();

    nodess << indent << part2dot(part);

    list<PartEdgePtr>::iterator oe = outEdges.begin();
    for( ; oe != outEdges.end(); oe++) {
      edgess << indent << partedge2dot(*oe);
    }

    PartEdgePtr accessEdge = parts.index()->inEdgeFromAny();
    return dynamic_cast<BoolAndLattice*>(dfInfo[accessEdge][0])->set(true);
  }

  void ConstPropCountAnalysis::print_stats() {
    cout << "m_assignExpCount=" << m_assignExpCount << endl;
    cout << "m_unassignExpCount=" << m_unassignExpCount << endl;
    cout << "m_knownCondCount=" << m_knownCondCount << endl;
    cout << "m_unknownCondCount=" << m_unknownCondCount << endl;
    cout << "edges=" << m_edges << endl;
  }

  string ConstPropCountAnalysis::str(string indent) {
    return "ConstPropCountAnalysis";
  }

  std::string ConstPropCountAnalysis::cfgn2label(CFGNode cfgn) {
    ostringstream oss;
    string node_s;
    SgNode* sgn = cfgn.getNode();
    // node_s = SageInterface::get_name(sgn);
    node_s = CFGNode2Str(cfgn);
    if(node_s.length() > 20) {
      node_s.resize(20); node_s += "...";
    }
    oss << node_s << "\n<" << sgn->class_name() << "> "
        << " line:" << sgn->get_startOfConstruct()->get_line();
    return oss.str();
  }

  string ConstPropCountAnalysis::part2label(PartPtr part) {
    set<CFGNode> cfgnset = part->CFGNodes();
    ostringstream oss;
    set<CFGNode>::iterator ci = cfgnset.begin();
    for( ; ci != cfgnset.end(); ) {      
      oss << cfgn2label(*ci);
      ++ci;
      if(ci != cfgnset.end()) oss << ", ";      
    }
    return oss.str();
  }

  
  string ConstPropCountAnalysis::newcontext2str() {
    ostringstream oss;
    oss << "cc" << contextid++;
    return oss.str();
  }

  string ConstPropCountAnalysis::context2dotid(PartContextPtr pcontext) {
    map<PartContextPtr, string>::const_iterator f = context2strMap.find(pcontext);
    if(f == context2strMap.end()) {
      string ctx2str = newcontext2str();
      context2strMap[pcontext] = ctx2str;
    }
    return context2strMap[pcontext];
  }

  string ConstPropCountAnalysis::part2dotid(PartPtr part) {
    set<CFGNode> cfgnset = part->CFGNodes();
    assert(cfgnset.size() == 1);
    CFGNode cn = *cfgnset.begin();

    ostringstream oss;    
    oss << cn.id() << context2dotid(part->getPartContext());
    string id = oss.str();

    // Sanity checks
    std::replace(id.begin(), id.end(), ' ', '_');
    std::replace(id.begin(), id.end(), ':', '_');

    return id;
  }
  
  string ConstPropCountAnalysis::part2dot(PartPtr part) {
    ostringstream oss;
    string nodeColor = "black";
    if (part->mustSgNodeAll<SgStatement>()) nodeColor = "blue";   
    else if (part->mustSgNodeAll<SgExpression>()) nodeColor = "darkgreen";
    else if (part->mustSgNodeAll<SgInitializedName>()) nodeColor = "brown";
    // First write the id
    oss << part2dotid(part) << " ";
    // Write the attributes
    oss << "[label=\"" << escapeString(part2label(part)) << "\""
        << ", color=" << nodeColor << "];\n";
    return oss.str();
  }

  string ConstPropCountAnalysis::partedge2dot(PartEdgePtr pedge) {
    ostringstream oss;
    PartPtr s = pedge->source();
    PartPtr t = pedge->target();
    // First write the dot edge
    oss << part2dotid(s) << " -> " << part2dotid(t);
    oss << " [color=\"black\"];" << endl;
    return oss.str();
  }

  
  void ConstPropCountAnalysis::generateDot() {
    // Composer* composer = analysis->getComposer();
    // set<PartPtr> initial = composer->GetStartAStates(analysis);
    // fw_dataflowGraphEdgeIterator<PartEdgePtr, PartPtr> ei(/* not working on incremental graph */
    //     						  false,
    //     						  selectIterOrderFromEnvironment());

    // set<PartPtr>::iterator ip = initial.begin();
    // for( ; ip != initial.end(); ++ip) {
    //   ei.addStart(*ip);
    // }

    // nodess.clear(); edgess.clear();

    // nodess << "\n subgraph cluster {\n";
    // string indent = "    ";

    // while(!ei.isEnd()) {
    //   PartPtr part = ei.getPart();
    //   // Write the node
    //   nodess << indent << part2dot(part);

    //   // Write the edges
    //   list<PartEdgePtr> oedges = part->outEdges();
    //   list<PartEdgePtr>::iterator oe = oedges.begin();
    //   for( ; oe != oedges.end(); ++oe) {
    //     edgess << indent << partedge2dot(*oe);               
    //   }

    //   ei.pushAllDescendants();      
    //   ei++;
    // }
    // nodess << indent << "}" << endl;
  }

  void ConstPropCountAnalysis::generateDotFile(int instance) {
    ofstream file;
    SgProject* project = SageInterface::getProject();
    SgFile* sgfile = project->get_files()[0]; assert(sgfile);
    ostringstream namess;
    namess << Rose::StringUtility::stripPathFromFileName(sgfile->getFileName()) << "-ats-" << instance << ".dot";
    string filename = namess.str();
    file.open(filename.c_str());
    file << "digraph G {" << endl;
    file << nodess.str();
    file << edgess.str() << endl;
    file << "}";
    file.close();
  }

} // end namespace
