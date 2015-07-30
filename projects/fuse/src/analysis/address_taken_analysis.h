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

  /*************************************************
   ********** FlowInsensitivePointerInfo  **********
   *************************************************/
  class FlowInsensitivePointerAnalysis : public UndirDataflow
  {
    SgProject* root;
    VariableIdMapping* vidm_p;
    VariableIdSet addressTakenSet;
    VariableIdSet arrayTypeSet;
    VariableIdSet pointerTypeSet;
    VariableIdSet referenceTypeSet;
    bool sound;

  public:
    FlowInsensitivePointerAnalysis(SgProject* project);
    FlowInsensitivePointerAnalysis(const FlowInsensitivePointerAnalysis& that);

    ComposedAnalysisPtr copy();
    void runAnalysis();
    ~FlowInsensitivePointerAnalysis();
    std::string str(std::string indent="") const;
    bool implementsExpr2Val() { return false; }
    bool implementsExpr2MemRegion() { return true; }
    bool implementsExpr2MemLoc() { return true; }
    bool implementsATSGraph() { return false; }
    MemRegionPtr Expr2MemRegion(SgNode* node, PartEdgePtr pedge);
    MemLocObjectPtr Expr2MemLoc(SgNode* node, PartEdgePtr pedge);    
  };

  class FlowInSensMRType : public sight::printable {
    MemRegionPtr parent;
  public:
    FlowInSensMRType(MemRegionPtr parent);
    FlowInSensMRType(const FlowInSensMRType& that);
    
    virtual mayEqualMRType(FlowInSensMRType& that)=0;
    virtual mustEqualMRType(FlowInSensMRType& that)=0;
    virtual equalSetMRType(FlowInSensMRType& that)=0;
    virtual subSetMRType(FlowInSensMRType& that)=0;
    virtual isLiveMRType()=0;
    virtual isFullMRType()=0;
    virtual isEmptyMRType()=0;
    virtual setToFullMRType()=0;
    virtual setToEmptyMRType()=0;
    virtual std::string str(std::string indent="") const=0;
  };

  //1. Type for all expressions identifiable by a single VariableId
  // SgInitializedName, SgVarRefExp, all sub-expressions that has SgVarRefExp as leaf
  class FlowInSensNamedMRType {
    VariableId id;
  };

  //2. Type for all expressions that is a set of all address taken variables
  // SgPointerDerefExp
  class FlowInSensPointerMRType {
    set<VariableId> addrTakenVars;
  };

  //3. Type for all temporary memory locations
  class FlowInSensExprMRType {
  };

  //4. Type for heap regions?
  class FlowInSensUnknownMRType {
  };

  class FlowInSensMR : public MemRegionObject {
  };
    

#endif
