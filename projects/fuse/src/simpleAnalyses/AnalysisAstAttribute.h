#ifndef _ANALYSISASTATTRIBUTE_H
#define _ANALYSISASTATTRIBUTE_H

/*****************************************
 * author: Sriram Aananthakrishnan, 2016 *
 *****************************************/

#include "compose.h"
#include "sight.h"

namespace fuse {

  /***************************
   * FuseMultiCPAstAttribute *
   ***************************/
  typedef std::map<Analysis*, ValueObjectPtr> FuseMultiCPAnnotMap;
  typedef std::map<Analysis*, ValueObjectPtr>::iterator FuseMultiCPAnnotMapIterator;
  class FuseMultiCPAstAttribute : public AstAttribute {
    SgNode* sgn;
    MemLocObjectPtr ml;
    FuseMultiCPAnnotMap valuesMap;
  public:
    FuseMultiCPAstAttribute(SgNode* sgn);
    FuseMultiCPAstAttribute(SgNode* sgn, MemLocObjectPtr ml_);
    FuseMultiCPAstAttribute(const FuseMultiCPAstAttribute& that);
    ValueObjectPtr get(std::map<Analysis*, ValueObjectPtr>::iterator it);
    FuseMultiCPAnnotMapIterator find(Analysis* analysis);    
    void add(Analysis* analysis, ValueObjectPtr value);
    FuseMultiCPAnnotMapIterator vMapEnd();
    FuseMultiCPAnnotMap getAnnotMap();
    std::string str(std::string indent="") const; 
  };

  /********************
   * Utility Function *
   ********************/
  class CPValueLattice;
  typedef boost::shared_ptr<CPValueLattice> CPValueLatticePtr;
  void addFuseAnnot(SgAssignOp* sgn, CPValueLatticePtr vlat, Analysis* analysis);
  void addFuseAnnot(SgCompoundAssignOp* sgn, CPValueLatticePtr vlat, Analysis* analysis);

  /**********************
   * FuseAnnotTraversal *
   **********************/
  class FuseAnnotTraversal : public AstSimpleProcessing {
    int nassign;
    std::list<ComposedAnalysis*> sanalyses;
    std::map<ComposedAnalysis*, int> constantCountMap;
    std::map<SgNode*, ComposedAnalysis*> assignOpConstantFirstAnalysisMap;
    std::map<ComposedAnalysis*, int> assignOpConstantFirstAnalysisCountMap;
  public:
    FuseAnnotTraversal(list<ComposedAnalysis*> sanalyses);
    list<ComposedAnalysis*> getConstantPropagation();
    void visit(SgNode* sgn);
    ~FuseAnnotTraversal();
    void printConstantCountMapStats();
    void printAssignOpConstantFirstAnalysisCount();
  };
} // end namespace

#endif
