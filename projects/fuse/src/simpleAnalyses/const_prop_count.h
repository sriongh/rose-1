/*****************************************
 * Author: Sriram Aananthakrishnan, 2017 *
 *****************************************/
#pragma once

#include "compose.h"

namespace fuse {
  /**************************
   * ConstPropCountAnalysis *
   **************************/
  class ConstPropCountAnalysis : virtual public FWDataflow {
    std::set<SgAssignOp*> m_visited;
    unsigned int m_assignExpCount;
    unsigned int m_unassignExpCount;
    long m_edges;
    unsigned int m_knownCondCount;
    unsigned int m_unknownCondCount;
    // DOT generation info
    std::stringstream nodess;
    std::stringstream edgess;
    std::string indent;
    int contextid;
    std::map<PartContextPtr, std::string> context2strMap;
  public:
    ConstPropCountAnalysis();
    ComposedAnalysisPtr copy();

    ValueObjectPtr Expr2Val(SgNode* sgn, PartEdgePtr pedge);
    bool implementsExpr2Val();

    void genInitLattice(/*PartPtr part, PartEdgePtr pedge, PartPtr supersetPart,*/
			const AnalysisParts& parts, const AnalysisPartEdges& pedges,
			std::vector<Lattice*>& initLattices);
    bool countConst(AnalysisParts& parts, CFGNode cn, NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);
    bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

    std::string str(std::string indent="");
    void print_stats();

    // utility functions for DOT generation
    std::string cfgn2label(CFGNode cfgn);
    std::string part2label(PartPtr part);
    std::string part2dot(PartPtr part);
    std::string partedge2dot(PartEdgePtr pedge);
    // Helper
    std::string newcontext2str();
    std::string context2dotid(PartContextPtr pcontext);
    std::string part2dotid(PartPtr);
    void generateDot();
    void generateDotFile(int instance=0);    
  };
};
