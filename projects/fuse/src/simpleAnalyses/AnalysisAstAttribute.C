#include "sage3basic.h"

using namespace std;

#include "AnalysisAstAttribute.h"
#include "const_prop_analysis.h"

#include <typeinfo>

/*****************************************
 * author: Sriram Aananthakrishnan, 2016 *
 *****************************************/

namespace fuse {

#define AnalysisAstAttributeDL 2
#if AnalysisAstAttributeDL==0
#define DISABLE_SIGHT
#endif
  
  /***************************
   * FuseMultiCPAstAttribute *
   ***************************/
  FuseMultiCPAstAttribute::FuseMultiCPAstAttribute(SgNode* sgn) : AstAttribute(), sgn(sgn) { }

  FuseMultiCPAstAttribute::FuseMultiCPAstAttribute(SgNode* sgn, MemLocObjectPtr ml_) 
    : AstAttribute(), sgn(sgn), ml(ml_->copyAOType()) { }

  FuseMultiCPAstAttribute::FuseMultiCPAstAttribute(const FuseMultiCPAstAttribute& that)
    : AstAttribute(), sgn(that.sgn), ml(that.ml), valuesMap(that.valuesMap) { }

  void FuseMultiCPAstAttribute::add(Analysis* analysis, ValueObjectPtr value) { 
    valuesMap[analysis] = value->copyAOType();
  }

  FuseMultiCPAnnotMapIterator FuseMultiCPAstAttribute::find(Analysis* analysis) {
    return valuesMap.find(analysis);
  }

  ValueObjectPtr FuseMultiCPAstAttribute::get(FuseMultiCPAnnotMapIterator it) {
    assert(it != valuesMap.end());
    return it->second;
  }

  FuseMultiCPAnnotMap FuseMultiCPAstAttribute::getAnnotMap() {
    return valuesMap;
  }

  FuseMultiCPAnnotMapIterator FuseMultiCPAstAttribute::vMapEnd() {
    return valuesMap.end();
  }
  
  std::string FuseMultiCPAstAttribute::str(std::string indent) const { 
    ostringstream oss;
    oss << "<table border=\"1\">";
    map<Analysis*, ValueObjectPtr>::const_iterator m = valuesMap.begin();
    for( ; m != valuesMap.end(); ++m) {
      oss << "<tr>";
      oss << "<td>" << m->first << "</td>";
      oss << "<td>" << (m->second)->str() << "</td>";
      oss << "</tr>";
    }
    oss << "</table>";
    return oss.str();
  } 

  /*********************
   * Utility Functions *
   *********************/
  void addFuseAnnot(SgAssignOp* sgn, CPValueLatticePtr vlat, Analysis* analysis) {
    assert(vlat);
    FuseMultiCPAstAttribute* attr;
    if(sgn->attributeExists("fuse:FuseMultiCPAstAttribute")) {
      attr = dynamic_cast<FuseMultiCPAstAttribute*>(sgn->getAttribute("fuse:FuseMultiCPAstAttribute"));
    }
    else {
      attr = new FuseMultiCPAstAttribute(sgn);
      sgn->addNewAttribute("fuse:FuseMultiCPAstAttribute", attr);
    }

    ValueObjectPtr val = boost::make_shared<CPValueObject>(vlat); assert(val);
    attr->add((Analysis*)analysis, val);
    sgn->updateAttribute("fuse:FuseMultiCPAstAttribute", attr);    
  }

  void addFuseAnnot(SgCompoundAssignOp* sgn, CPValueLatticePtr vlat, Analysis* analysis) {
    assert(vlat);
    FuseMultiCPAstAttribute* attr;
    if(sgn->attributeExists("fuse:FuseMultiCPAstAttribute")) {
      attr = dynamic_cast<FuseMultiCPAstAttribute*>(sgn->getAttribute("fuse:FuseMultiCPAstAttribute"));
    }
    else {
      attr = new FuseMultiCPAstAttribute(sgn);
      sgn->addNewAttribute("fuse:FuseMultiCPAstAttribute", attr);
    }

    ValueObjectPtr val = boost::make_shared<CPValueObject>(vlat); assert(val);
    attr->add((Analysis*)analysis, val);
    sgn->updateAttribute("fuse:FuseMultiCPAstAttribute", attr);    
  }

  /**********************
   * FuseAnnotTraversal *
   **********************/
  FuseAnnotTraversal::FuseAnnotTraversal(list<ComposedAnalysis*> sanalyses_) : nassign(0), sanalyses(sanalyses_) { }

  FuseAnnotTraversal::~FuseAnnotTraversal() { }

  list<ComposedAnalysis*> FuseAnnotTraversal::getConstantPropagation() {
    list<ComposedAnalysis*> cpAnalyses;
    list<ComposedAnalysis*>::iterator c = sanalyses.begin();
    for( ; c != sanalyses.end(); ++c) {
      ConstantPropagationAnalysis* cp = dynamic_cast<ConstantPropagationAnalysis*>(*c);
      if(cp) cpAnalyses.push_back(cp);      
    }
    return cpAnalyses;
  }

  void FuseAnnotTraversal::visit(SgNode* sgn) {
    if(isSgAssignOp(sgn) || isSgCompoundAssignOp(sgn)) {
      nassign++;
      if(sgn->attributeExists("fuse:FuseMultiCPAstAttribute")) {          
        SIGHT_VERB_DECL(scope, (sight::txt() << "sgn=" << SgNode2Str(sgn), scope::medium), 2, AnalysisAstAttributeDL)
        FuseMultiCPAstAttribute* attr = dynamic_cast<FuseMultiCPAstAttribute*>(sgn->getAttribute("fuse:FuseMultiCPAstAttribute"));
        SIGHT_VERB(dbg << attr->str(), 2, AnalysisAstAttributeDL)
        // First stat
        // Collect constants assigned by each analysis
        list<ComposedAnalysis*> cpAnalyses = getConstantPropagation();
        cpAnalyses.sort();
        list<ComposedAnalysis*>::iterator c = cpAnalyses.begin();
        for( ; c != cpAnalyses.end(); ++c) {
          FuseMultiCPAnnotMapIterator f = attr->find(*c);
          if(f != attr->vMapEnd()) {
            ValueObjectPtr val = attr->get(f);
            if(val->isConcrete()) {
              if(constantCountMap.find(*c) == constantCountMap.end())
                constantCountMap[*c] = 1;
              else
                constantCountMap[*c] = constantCountMap[*c] + 1; 
            }
          }
        } // end for
        cpAnalyses.sort();
        c = cpAnalyses.begin();
        for( ; c != cpAnalyses.end(); ++c) {
          FuseMultiCPAnnotMapIterator f = attr->find(*c);
          if(f != attr->vMapEnd()) {
            ValueObjectPtr val = attr->get(f);
            if(val->isConcrete()) {
              assignOpConstantFirstAnalysisMap[sgn] = *c;
              if(assignOpConstantFirstAnalysisCountMap.find(*c) == assignOpConstantFirstAnalysisCountMap.end())
                assignOpConstantFirstAnalysisCountMap[*c] = 1;
              else
                assignOpConstantFirstAnalysisCountMap[*c]++;
              break;
            }
          }
        } // end for        
      } // end if(sgn->attributeExists..)
    } // end if(isSgAssignOp..)
  }

  void FuseAnnotTraversal::printConstantCountMapStats() {
    std::map<ComposedAnalysis*, int>::iterator c = constantCountMap.begin();
    cout << "nassign=" << nassign << endl;
    for(int i=0 ; c != constantCountMap.end(); ++c, ++i) {
      cout << (*c).first << "_CP_" << i << ", " << (*c).second << endl;
    }
  }

  void FuseAnnotTraversal::printAssignOpConstantFirstAnalysisCount() {
    std::map<ComposedAnalysis*, int>::iterator c = assignOpConstantFirstAnalysisCountMap.begin();
    for(int i=0 ; c != assignOpConstantFirstAnalysisCountMap.end(); ++c, ++i) {
      cout << c->first << "_CP_" << i << ", " << c->second << endl;
    }
  }
}; // end namespace
