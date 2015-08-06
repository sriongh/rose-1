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
  // some utility functions for sets of VariableId

  class VariableIdSetPrettyPrint
  {
  public:
    std::string static str(VariableIdSet& vset, VariableIdMapping& vidm);
    std::string static str(VariableIdSet& vset);
  };

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
    VariableIdMapping* vidm_p;
    VariableIdSet addressTakenSet;
    VariableIdSet arrayTypeSet;
    VariableIdSet pointerTypeSet;
    VariableIdSet referenceTypeSet;
    bool sound;

  public:
    FlowInSensAddrTakenAnalysis(SgProject* project);
    FlowInSensAddrTakenAnalysis(const FlowInSensAddrTakenAnalysis& that);

    ComposedAnalysisPtr copy();
    void runAnalysis();
    ~FlowInSensAddrTakenAnalysis();
    std::string str(std::string indent="") const;
    bool implementsExpr2Val() { return false; }
    bool implementsExpr2MemRegion() { return true; }
    bool implementsExpr2MemLoc() { return true; }
    bool implementsATSGraph() { return false; }
    MemRegionObjectPtr Expr2MemRegion(SgNode* node, PartEdgePtr pedge);
    MemLocObjectPtr Expr2MemLoc(SgNode* node, PartEdgePtr pedge);
  };

  /****************
   * ATAnalMRType *
   ****************/
  class ATAnalMRType : public sight::printable {
  protected:
    enum MRType{named,
                aliasing,
                expr,
                unknown
    };
    MRType type;
  public:
    ATAnalMRType(MRType type);
    ATAnalMRType(const ATAnalMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType()=0;

    MRType getType() const;
    virtual bool mayEqualMRType(ATAnalMRTypePtr that)=0;
    virtual bool mustEqualMRType(ATAnalMRTypePtr that)=0;
    virtual bool equalSetMRType(ATAnalMRTypePtr that)=0;
    virtual bool subSetMRType(ATAnalMRTypePtr that)=0;
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
  public:
    ATAnalNamedMRType(MRType type, VariableId id);
    ATAnalNamedMRType(const ATAnalNamedMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType();

    VariableId getId()const;
    virtual bool mayEqualMRType(ATAnalMRTypePtr that);
    virtual bool mustEqualMRType(ATAnalMRTypePtr that);
    virtual bool equalSetMRType(ATAnalMRTypePtr that);
    virtual bool subSetMRType(ATAnalMRTypePtr that);
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
  public:
    ATAnalAliasingMRType(MRType type, VariableIdSet aliasingSet);
    ATAnalAliasingMRType(const ATAnalAliasingMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType();

    const VariableIdSet& getAliasingSet() const;
    bool contains(VariableId id) const;
    bool singleton() const;
    bool set_equal(ATAnalAliasingMRTypePtr that) const;
    bool set_subset(ATAnalAliasingMRTypePtr that) const;
    
    virtual bool mayEqualMRType(ATAnalMRTypePtr that);
    virtual bool mustEqualMRType(ATAnalMRTypePtr that);
    virtual bool equalSetMRType(ATAnalMRTypePtr that);
    virtual bool subSetMRType(ATAnalMRTypePtr that);
    virtual std::string str(std::string indent="") const;
  };


  ATAnalAliasingMRTypePtr isATAnalAliasingMRType(ATAnalMRTypePtr type);

  /********************
   * ATAnalExprMRType *
   ********************/
  // Type for all temporary memory locations
  class ATAnalExprMRType : public ATAnalMRType {
  public:
    ATAnalExprMRType(MRType type);
    ATAnalExprMRType(const ATAnalExprMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType();
    
    virtual bool mayEqualMRType(ATAnalMRTypePtr that);
    virtual bool mustEqualMRType(ATAnalMRTypePtr that);
    virtual bool equalSetMRType(ATAnalMRTypePtr that);
    virtual bool subSetMRType(ATAnalMRTypePtr that);
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
    ATAnalUnknownMRType(MRType type);
    ATAnalUnknownMRType(const ATAnalUnknownMRType& that);
    virtual ATAnalMRTypePtr copyATAnalMRType();
    
    virtual bool mayEqualMRType(ATAnalMRTypePtr that);
    virtual bool mustEqualMRType(ATAnalMRTypePtr that);
    virtual bool equalSetMRType(ATAnalMRTypePtr that);
    virtual bool subSetMRType(ATAnalMRTypePtr that);
    virtual std::string str(std::string indent="") const;
  };

  typedef boost::shared_ptr<ATAnalUnknownMRType> ATAnalUnknownMRTypePtr;
  ATAnalUnknownMRTypePtr isATAnalUnknownMRType(ATAnalMRTypePtr type);

  /**********************
   * FlowInSensATAnalMR *
   **********************/
  class FlowInSensATAnalMR: public MemRegionObject {
    ATAnalMRTypePtr type;
  };
  
}// end namespace
#endif
