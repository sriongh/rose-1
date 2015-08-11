#ifndef _ADDRESSTAKENANALYSIS_H
#define _ADDRESSTAKENANALYSIS_H

/*************************************************************
 * Copyright: (C) 2015 by Sriram Aananthakrishnan            *
 * Author   : Sriram Aananthakrishnan                        *
 * email    : aananthakris1@llnl.gov                         *
 *************************************************************/

#include "AstMatching.h"
#include "VariableIdMapping.h"
#include "compose.h"
#include <set>

namespace fuse {

  /*******************
   * VariableIdUtils *
   *******************/

  // utility function to union two VariableIdSet into rset
  void set_union(const VariableIdSet& set1, const VariableIdSet& set2, VariableIdSet& rset);


  /*************************************************
   ***************** ProcessQuery  *****************
   *************************************************/
  // AST Query Processor
  // common functor to process any query and build match result

  class ProcessQuery
  {
    // the root node on which the AST matching needs to be performed
    MatchResult match_result;

  public:
    ProcessQuery() { }
    // functor to operate on the given query
    MatchResult& operator()(std::string query, SgNode* root);
    MatchResult& getMatchResult();
    void printMatchResult();
    void clearMatchResult();
  };

  typedef boost::shared_ptr<VariableIdMapping> VariableIdMappingPtr;

  /*************************************************
   ************* ComputeAddressTakenInfo  **********
   *************************************************/
  class ComputeAddressTakenInfo
  {
    VariableIdMapping& vidm;
    // result to be computed by this analysis
    // bool is set to true when operand of SgAddressOfExp is a complicated
    // expression for which VariableId cannot be determined
    // example: &(*p)
    VariableIdSet addressTakenSet;
    bool sound;
    //friend class OperandToVariableId;
  public:
    ComputeAddressTakenInfo(VariableIdMapping& _vidm) : vidm(_vidm)
    {
      sound = true;
    }

    // address can be taken for any expression that is lvalue
    // The purpose of this class is to traverse arbitrary
    // expressions that are operands of SgAddressOfOp and find the
    // variable whose address is actually taken.
    // For example in expression &(a.b->c),  'c' address is
    // actually taken. This class simply traverses the operand
    // of SgAddressOfOp to identify 
    // the variable whose address is taken
    // 
    class OperandToVariableId : public ROSE_VisitorPatternDefaultBase
    {
      ComputeAddressTakenInfo& cati;
      int debuglevel;
    public:
      OperandToVariableId(ComputeAddressTakenInfo& _cati) : cati(_cati), debuglevel(0) { }
      void visit(SgVarRefExp*);
      void visit(SgDotExp*);
      void visit(SgArrowExp*);
      void visit(SgPointerDerefExp*);
      void visit(SgPntrArrRefExp*);
      void visit(SgAssignOp* sgn);
      void visit(SgPlusPlusOp* sgn);
      void visit(SgMinusMinusOp* sgn);
      void visit(SgCommaOpExp* sgn);
      void visit(SgConditionalExp* sgn);
      void visit(SgCastExp* sgn);
      void visit(SgFunctionRefExp* sgn);
      void visit(SgMemberFunctionRefExp* sgn);
      void visit(SgTemplateFunctionRefExp* sgn);
      void visit(SgTemplateMemberFunctionRefExp* sgn);
      void visit(SgFunctionCallExp* sgn);
      void visit(SgNode* sgn);
      void debugPrint(SgNode* sgn);
    };

    void computeAddressTakenInfo(SgNode* root);
    VariableIdSet getAddressTakenSet();
    bool isSound();
  };

  /*************************************************
   *************** CollectTypeInfo *****************
   *************************************************/
  class CollectTypeInfo
  {
    VariableIdMapping& vidm;
    VariableIdSet pointerTypeSet;
    VariableIdSet arrayTypeSet;
    VariableIdSet referenceTypeSet;
    bool initialized;

    // collect type info for only these if set by constructor
    VariableIdSet varsUsed;

  public:
    CollectTypeInfo(VariableIdMapping& _vidm) : vidm(_vidm) { }
    CollectTypeInfo(VariableIdMapping& _vidm,
                    VariableIdSet _usedVarsInFunctions) 
      : vidm(_vidm),
        varsUsed(_usedVarsInFunctions) { }

    void collectTypes();
    VariableIdSet getPointerTypeSet();
    VariableIdSet getArrayTypeSet();
    VariableIdSet getReferenceTypeSet();
  };

  /**************************************************
   ********** FlowInSensAddrTakenAnalysis  **********
   **************************************************/
  class ATAnalMRType;
  typedef boost::shared_ptr<ATAnalMRType> ATAnalMRTypePtr;
  
  class FlowInSensAddrTakenAnalysis : public UndirDataflow
  {
    SgProject* root;
    VariableIdMappingPtr vidm_p;
    VariableIdSet addressTakenSet;
    VariableIdSet arrayTypeSet;
    VariableIdSet pointerTypeSet;
    VariableIdSet referenceTypeSet;
    VariableIdSet aliasingSet;
    bool sound;

  public:
    FlowInSensAddrTakenAnalysis(SgProject* project);
    FlowInSensAddrTakenAnalysis(const FlowInSensAddrTakenAnalysis& that);

    ComposedAnalysisPtr copy();
    void runAnalysis();
    std::string str(std::string indent="") const;
    bool implementsExpr2Val() { return false; }
    bool implementsExpr2MemRegion() { return true; }
    bool implementsExpr2MemLoc() { return true; }
    bool implementsATSGraph() { return false; }

    class Expr2MemRegionCreate : public ROSE_VisitorPatternDefaultBase {
      FlowInSensAddrTakenAnalysis& addrTakenAnalysis;
      Composer* composer;
      PartEdgePtr pedge;
      ATAnalMRTypePtr type;
    public:
      Expr2MemRegionCreate(FlowInSensAddrTakenAnalysis& addrTakenAnalysis, Composer* composer, PartEdgePtr pedge);
      ATAnalMRTypePtr getATAnalMRType() const;
      bool contains(const VariableIdSet& vIdSet, VariableId id) const;
      void createATAnalNamedMRType(VariableId id, MemRegionObjectPtr parent, SgNode* sgn);
      
      void visit(SgVarRefExp* sgn);
      void visit(SgInitializedName* sgn);
      void visit(SgDotExp* sgn);
      void visit(SgArrowExp* sgn);
      void visit(SgPointerDerefExp* sgn);
      void visit(SgExpression* sgn);
    };

    VariableIdMappingPtr getVariableIdMapping() const;

    friend class Expr2MemRegionCreate;
    MemRegionObjectPtr Expr2MemRegion(SgNode* node, PartEdgePtr pedge);
    MemLocObjectPtr Expr2MemLoc(SgNode* node, PartEdgePtr pedge);
  };

  /****************
   * ATAnalMRType *
   ****************/
  class ATAnalMRType : public sight::printable {
  protected:
    MemRegionObjectPtr parent;
    SgNode* base;
  public:
    ATAnalMRType(MemRegionObjectPtr parent, SgNode* base);
    ATAnalMRType(const ATAnalMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType() const=0;

    virtual bool mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge)=0;
    virtual bool mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge)=0;
    virtual bool equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge)=0;
    virtual bool subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge)=0;
    virtual bool isFullMRType(PartEdgePtr pedge)=0;
    virtual bool isLiveMRType(PartEdgePtr pedge)=0;
    virtual std::string str(std::string indent="") const=0;
  };

  /*********************
   * ATAnalNamedMRType *
   *********************/
  // Type for all expressions identifiable by a single VariableId
  // SgInitializedName
  // SgVarRefExp
  // SgDotExp, SgArrowExp where leaf=SgVarRefExp
  class ATAnalNamedMRType : public ATAnalMRType {
    VariableId id;
    bool addrtaken;
    VariableIdMappingPtr vidm_p;
  public:
    ATAnalNamedMRType(MemRegionObjectPtr parent, SgNode* base,  VariableId id, bool addrtaken, VariableIdMappingPtr vidm_p);
    ATAnalNamedMRType(const ATAnalNamedMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType() const;

    VariableId getId()const;
    virtual bool mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool isFullMRType(PartEdgePtr pedge);
    virtual bool isLiveMRType(PartEdgePtr pedge);
    virtual std::string str(std::string indent="") const;
  };
  typedef boost::shared_ptr<ATAnalNamedMRType> ATAnalNamedMRTypePtr;
  ATAnalNamedMRTypePtr isATAnalNamedMRType(ATAnalMRTypePtr type);

  /************************
   * ATAnalAliasingMRType *
   ************************/

  // Type for all expressions that is a set of all address taken variables
  // SgPointerDerefExp
  class ATAnalAliasingMRType;
  typedef boost::shared_ptr<ATAnalAliasingMRType> ATAnalAliasingMRTypePtr;
  class ATAnalAliasingMRType : public ATAnalMRType {
    VariableIdSet aliasingSet;
    VariableIdMappingPtr vidm_p;
  public:
    ATAnalAliasingMRType(MemRegionObjectPtr parent, SgNode* base, VariableIdSet aliasingSet, VariableIdMappingPtr vidm_p);
    ATAnalAliasingMRType(const ATAnalAliasingMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType() const;

    const VariableIdSet& getAliasingSet() const;
    bool contains(VariableId id) const;
    bool singleton() const;
    bool set_equal(ATAnalAliasingMRTypePtr that) const;
    bool set_subset(ATAnalAliasingMRTypePtr that) const;
    bool set_intersect(ATAnalAliasingMRTypePtr that) const;
    
    virtual bool mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool isFullMRType(PartEdgePtr pedge);
    virtual bool isLiveMRType(PartEdgePtr pedge);
    virtual std::string str(std::string indent="") const;
  };


  ATAnalAliasingMRTypePtr isATAnalAliasingMRType(ATAnalMRTypePtr type);

  /********************
   * ATAnalExprMRType *
   ********************/
  // Type for all temporary memory locations
  class ATAnalExprMRType : public ATAnalMRType {
    SgExpression* expr;
  public:
    ATAnalExprMRType(MemRegionObjectPtr parent, SgNode* base);
    ATAnalExprMRType(const ATAnalExprMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType() const;
    MemRegionObjectPtr getParent() const;
    
    virtual bool mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool isFullMRType(PartEdgePtr pedge);
    virtual bool isLiveMRType(PartEdgePtr pedge);
    virtual std::string str(std::string indent="") const;
  };

  typedef boost::shared_ptr<ATAnalExprMRType> ATAnalExprMRTypePtr;
  ATAnalExprMRTypePtr isATAnalExprMRType(ATAnalMRTypePtr type);

  /***********************
   * ATAnalUnknownMRType *
   ***********************/
  // Type for heap regions?
  class ATAnalUnknownMRType : public ATAnalMRType {
  public:
    ATAnalUnknownMRType(MemRegionObjectPtr parent, SgNode* base);
    ATAnalUnknownMRType(const ATAnalUnknownMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType() const;
    
    virtual bool mayEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool mustEqualMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool equalSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool subSetMRType(ATAnalMRTypePtr that, PartEdgePtr pedge);
    virtual bool isFullMRType(PartEdgePtr pedge);
    virtual bool isLiveMRType(PartEdgePtr pedge);
    virtual std::string str(std::string indent="") const;
  };

  typedef boost::shared_ptr<ATAnalUnknownMRType> ATAnalUnknownMRTypePtr;
  ATAnalUnknownMRTypePtr isATAnalUnknownMRType(ATAnalMRTypePtr type);

  /**********************
   * FlowInSensATAnalMR *
   **********************/
  class FlowInSensATAnalMR: public MemRegionObject {
    SgNode* sgn;
    ATAnalMRTypePtr type;
    MemRegionObjectPtr parent;
  public:
    FlowInSensATAnalMR(SgNode* sgn, ATAnalMRTypePtr type, MemRegionObjectPtr parent);
    FlowInSensATAnalMR(const FlowInSensATAnalMR& that);
    MemRegionObjectPtr copyMR() const;
    ATAnalMRTypePtr getATAnalMRTypePtr() const;
    
    bool mayEqualMR(MemRegionObjectPtr that, PartEdgePtr pedge);
    bool mustEqualMR(MemRegionObjectPtr that, PartEdgePtr pedge);
    bool equalSetMR(MemRegionObjectPtr that, PartEdgePtr pedge);
    bool subSetMR(MemRegionObjectPtr that, PartEdgePtr pedge);
    bool isLiveMR(PartEdgePtr pedge);
    bool meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge);
    bool isEmptyMR(PartEdgePtr pedge);
    bool isFullMR(PartEdgePtr pedge);
    ValueObjectPtr getRegionSize(PartEdgePtr pedge) const;
    std::string str(std::string indent="") const;
  };

  typedef boost::shared_ptr<FlowInSensATAnalMR> FlowInSensATAnalMRPtr;
  FlowInSensATAnalMRPtr isFlowInSensATAnalMR(MemRegionObjectPtr that);
  
}// end namespace
#endif
