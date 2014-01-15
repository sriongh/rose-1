#pragma once
// #include "dataflow.h"
#include <rose.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <ostream>
#include <fstream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <filteredCFG.h>
#include "staticSingleAssignment.h"
#include <boost/unordered_map.hpp>
#include "dataflowCfgFilter.h"
// #include "CallGraph.h"
#include "uniqueNameTraversal.h"
#include "heapReachingDef.h"
#include "abstract_object.h"
#include "compose.h"
#include "ssaUnfilteredCfg.h"

#include <vector>

namespace hssa_private
{
  using namespace boost;
  using namespace std;

  using namespace fuse;

  typedef ssa_unfiltered_cfg::FunctionFilter FunctionFilter;

  class HeapSSA;

  class SSAScalar;
  typedef boost::shared_ptr<SSAScalar> SSAScalarPtr;
  class SSAFunctionMemLoc;
  typedef boost::shared_ptr<SSAFunctionMemLoc> SSAFunctionMemLocPtr;
  class SSALabeledAggregate;
  typedef boost::shared_ptr<SSALabeledAggregate> SSALabeledAggregatePtr;
  class SSAArray;
  typedef boost::shared_ptr<SSAArray> SSAArrayPtr;
  class SSAPointer;
  typedef boost::shared_ptr<SSAPointer> SSAPointerPtr;
  class SSADefault;
  typedef boost::shared_ptr<SSADefault> SSADefaultPtr;

  class SSAMemLoc : public virtual MemLocObject {
  protected:
    hssa_private::HeapSSA * ssa;
    SgExpression* expr;
    /// Memory location object retrieved from previous analysis
    MemLocObjectPtr memLoc;
    /// Partiton graph node
    PartPtr part;

  public:
  SSAMemLoc(HeapSSA* ssaInstance) : ssa(ssaInstance), expr(NULL), MemLocObject(NULL) {
    // ROSE_ASSERT(false && "Must initialize MemLoc");
  };
  SSAMemLoc(HeapSSA* ssaInstance, SgExpression* expr_) : ssa(ssaInstance), expr(expr_), 
    MemLocObject(NULL) {
    // ROSE_ASSERT(false && "Must initialize MemLoc");
  };
  SSAMemLoc(SSAMemLoc* ssaMemLoc) : ssa(ssaMemLoc->ssa), expr(ssaMemLoc->expr), 
    MemLocObject(ssaMemLoc->expr) {
    // ROSE_ASSERT(false && "Must initialize MemLoc");
  };
  SSAMemLoc(HeapSSA* ssaInstance, SgExpression* expr_, MemLocObjectPtr memLoc_, PartPtr part_) : ssa(ssaInstance), 
    expr(expr_), memLoc(memLoc_), part(part_), MemLocObject(expr_) {};

  SSAMemLoc(SgNode* sgn) : MemLocObject(sgn), expr(NULL), ssa(NULL) {
    // ROSE_ASSERT(false && "Must initialize MemLoc");
  };

  SSAMemLoc(MemLocObjectPtr memLoc_, PartPtr part_, SgExpression* labelNode, HeapSSA* ssaInstance) 
    : expr(labelNode), ssa(ssaInstance), memLoc(memLoc_), part(part_), MemLocObject(labelNode) {};

    bool mayEqualML(MemLocObjectPtr o, PartPtr p); //  const;
    bool mustEqualML(MemLocObjectPtr o, PartPtr p); //  const;
    
    SgExpression* getVarExpr() const { return expr; };
    void setVarExpr(SgExpression* expr_) { expr = expr_; };
    
    MemLocObjectPtr getLabelMemLoc() { return memLoc; };

    PartPtr getPart() { return part; };

    HeapSSA* getSSAInstance() { return ssa; };

    bool mayEqual(MemLocObjectPtr o, PartPtr p) { return mayEqualML(o, p); };
    bool mustEqual(MemLocObjectPtr o, PartPtr p) { return mustEqualML(o, p); };

    virtual bool mayEqualML(MemLocObjectPtr o) const;
    virtual bool mustEqualML(MemLocObjectPtr o) const;

    virtual bool mayEqual(MemLocObjectPtr o) const { mayEqualML(o); };
    virtual bool mustEqual(MemLocObjectPtr o) const { mayEqualML(o); };
        
    virtual bool mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge) { return mayEqualML(o); };
    virtual bool mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge) { return mustEqualML(o); };

    std::string str(std::string indent="") const { 
      std::string str = "SSA Mem Loc: " + memLoc->str();
      return str;
    };
    std::string str(std::string indent="") {
      std::string str ="SSA Mem Loc: "+ memLoc->str();
      return str;
    };

    /*virtual ScalarPtr isScalar() {
      return boost::dynamic_pointer_cast<Scalar>(copyML());
    }

    virtual FunctionMemLocPtr isFunctionMemLoc() {
      return boost::dynamic_pointer_cast<FunctionMemLoc>(copyML());
    }

    virtual LabeledAggregatePtr isLabeledAggregate() {
      return boost::dynamic_pointer_cast<LabeledAggregate>(copyML());
    }

    virtual ArrayPtr isArray() {
      return boost::dynamic_pointer_cast<Array>(copyML());
    }

    virtual PointerPtr isPointer() {
      return boost::dynamic_pointer_cast<Pointer>(copyML());
      }*/

    // TODO:
    virtual bool equalSet(AbstractObjectPtr objPtr, PartEdgePtr partEdgePtr) {
      return false;
    }

    virtual bool isFull(PartEdgePtr partEdgePtr) {
      return false;
    }

    virtual bool isEmpty(PartEdgePtr partEdgePtr) {
      return false;
    }

    virtual bool isLiveML(PartEdgePtr partEdgePtr) {
      return false;
    }
    
    virtual bool meetUpdateML(MemLocObjectPtr objPtr, PartEdgePtr partEdgePtr) {
      return false;
    }

  protected:
    SgNode* getDefNode(SgExpression* expr) const;
  
    /// Internal comparision, i.e. if the two given expressions are same or not
    static bool isSameSig(SgExpression* exprL, SgExpression* exprR);
  };

  typedef boost::shared_ptr<SSAMemLoc> SSAMemLocPtr;

  /*class SSAScalar : virtual public SSAMemLoc,  virtual public Scalar {
  public:
    SSAScalar(HeapSSA* ssaInstance, SgExpression* expr_)
      : SSAMemLoc(ssaInstance, expr_), MemLocObject(expr_) {
      ROSE_ASSERT(false && "Scalar must initialize MemLoc");
    };
      SSAScalar(HeapSSA* ssaInstance, SgExpression* expr_, MemLocObjectPtr memLoc, PartPtr part)
	: SSAMemLoc(ssaInstance, expr_, memLoc, part), MemLocObject(expr_) {};

    SSAScalar(SgNode* sgn)
      : SSAMemLoc(sgn), MemLocObject(sgn) {
      ROSE_ASSERT(false && "Scalar must initialize MemLoc");
    };

    virtual bool mayEqual(MemLocObjectPtr o) const; 
    virtual bool mustEqual(MemLocObjectPtr o) const; 
  
    bool isLive(PartEdgePtr pedge) const { return true; };

    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) {
      ROSE_ASSERT(false && "Not supported!");
    };

    MemLocObjectPtr copyML() const {
      return boost::make_shared<SSAScalar>(ssa, expr, memLoc, part);
    };
  };

  class SSAFunctionMemLoc : virtual public SSAMemLoc, virtual public FunctionMemLoc {
  public:
    SSAFunctionMemLoc(HeapSSA* ssaInstance, SgFunctionSymbol* funcSymb_)
      : SSAMemLoc(ssaInstance, NULL), funcSymb(funcSymb_), MemLocObject(NULL) {
      ROSE_ASSERT(false && "Function must initialize MemLoc");
    };
    SSAFunctionMemLoc(HeapSSA* ssaInstance, SgFunctionSymbol* funcSymb_, MemLocObjectPtr memLoc, PartPtr part)
      : SSAMemLoc(ssaInstance, NULL, memLoc, part), funcSymb(funcSymb_), MemLocObject(NULL) {};

    SSAFunctionMemLoc(SgNode* sgn)
      : SSAMemLoc(sgn), MemLocObject(sgn) {
      ROSE_ASSERT(false && "Function must initialize MemLoc");
    };

    virtual bool mayEqual(MemLocObjectPtr o) const { mayEqual(o); };
    virtual bool mustEqual(MemLocObjectPtr o) const { mustEqual(o); };

    bool isLive(PartEdgePtr pedge) const { return true; };

    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) {
      ROSE_ASSERT(false && "Not supported!");
    };

    MemLocObjectPtr copyML() const {
      return boost::make_shared<SSAFunctionMemLoc>(ssa, funcSymb, memLoc, part);
    };

  protected:
    SgFunctionSymbol* funcSymb;

  public:
    SgFunctionSymbol* getFuncSymb() { return funcSymb; };
  };

  class SSALabeledAggregate : virtual public SSAMemLoc, virtual public LabeledAggregate {
  public:
    SSALabeledAggregate(HeapSSA* ssaInstance, SgExpression* expr_)
      : SSAMemLoc(ssaInstance, expr_), MemLocObject(expr_) {
      ROSE_ASSERT(false && "Labeled aggregate must initialize MemLoc");
    };
    SSALabeledAggregate(HeapSSA* ssaInstance, SgExpression* expr_, MemLocObjectPtr memLoc, PartPtr part)
      : SSAMemLoc(ssaInstance, expr_, memLoc, part), MemLocObject(expr_) {};

    SSALabeledAggregate(SgNode* sgn)
        : SSAMemLoc(sgn), MemLocObject(sgn) {
      ROSE_ASSERT(false && "Labeled aggregate must initialize MemLoc");
    };

    virtual bool mayEqual(MemLocObjectPtr o) const { mayEqual(o); };
    virtual bool mustEqual(MemLocObjectPtr o) const { mustEqual(o); };

    bool isLive(PartEdgePtr pedge) const { return true; };

    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) {
      ROSE_ASSERT(false && "Not supported!");
    };

    MemLocObjectPtr copyML() const {
      return boost::make_shared<SSALabeledAggregate>(ssa, expr, memLoc, part);
    };
  };

  class SSAArray : virtual public SSAMemLoc, virtual public Array {
  public:
    SSAArray(HeapSSA* ssaInstance, SgExpression* expr_)
      : SSAMemLoc(ssaInstance, expr_), MemLocObject(expr_) {
      ROSE_ASSERT(false && "Array must initialize MemLoc");
    };
    SSAArray(IndexVectorPtr iv_, HeapSSA* ssaInstance, SgExpression* expr_)
      : SSAMemLoc(ssaInstance, expr_), MemLocObject(expr_), iv(iv_) {
      ROSE_ASSERT(false && "Array must initialize MemLoc");
    };
    SSAArray(HeapSSA* ssaInstance, SgExpression* expr_, MemLocObjectPtr memLoc, PartPtr part)
      : SSAMemLoc(ssaInstance, expr_, memLoc, part), MemLocObject(expr_) {};

    SSAArray(SgNode* sgn)
      : SSAMemLoc(sgn), MemLocObject(sgn) {
      ROSE_ASSERT(false && "Array must initialize MemLoc");
    };

    virtual bool mayEqual(MemLocObjectPtr o) const; 
    virtual bool mustEqual(MemLocObjectPtr o) const; 

    bool isLive(PartEdgePtr pedge) const { return true; };

    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) {
      ROSE_ASSERT(false && "Not supported!");
    };

    IndexVectorPtr getIndexVector() { return iv; };

    MemLocObjectPtr copyML() const {
      return boost::make_shared<SSAArray>(ssa, expr, memLoc, part);
    };

  protected:
    IndexVectorPtr iv;
  };

  class SSAPointer : virtual public SSAMemLoc, virtual public Pointer {
  public:
    SSAPointer(HeapSSA* ssaInstance, SgExpression* expr_) 
      : SSAMemLoc(ssaInstance, expr_), MemLocObject(expr_) {
      ROSE_ASSERT(false && "Pointer must initialize MemLoc");
    };
    SSAPointer(HeapSSA* ssaInstance, SgExpression* expr_, MemLocObjectPtr memLoc, PartPtr part)
      : SSAMemLoc(ssaInstance, expr_, memLoc, part), MemLocObject(expr_) {};
      
    SSAPointer(SgNode* sgn)
      : SSAMemLoc(sgn), MemLocObject(sgn) {
      ROSE_ASSERT(false && "Pointer must initialize MemLoc");
    };

    virtual bool mayEqual(MemLocObjectPtr o) const; 
    virtual bool mustEqual(MemLocObjectPtr o) const;

    bool isLive(PartEdgePtr pedge) const { return true; };

    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) {
      ROSE_ASSERT(false && "Not supported!");
    };

    MemLocObjectPtr copyML() const { 
      return boost::make_shared<SSAPointer>(ssa, expr, memLoc, part);
    };
    };*/

  class SSADefault : virtual public SSAMemLoc {
  protected:
    SgInitializedName* initName;

  public:
    SSADefault(HeapSSA* ssaInstance, SgExpression* expr) : SSAMemLoc(ssaInstance, expr), 
      MemLocObject(expr), initName(NULL) {
      ROSE_ASSERT(false && "Default must initialize MemLoc");
    };
      // SSADefault(HeapSSA* ssaInstance, SgExpression* expr, MemLocObjectPtr memLoc, PartPtr part) 
      //  : SSAMemLoc(ssaInstance, expr, memLoc, part), MemLocObject(expr), initName(NULL) {};
    SSADefault(HeapSSA* ssaInstance, SgNode* expr, MemLocObjectPtr memLoc, PartPtr part)
      : SSAMemLoc(ssaInstance, NULL, memLoc, part), MemLocObject(expr) {
      expr = isSgExpression(expr);
      initName = isSgInitializedName(expr);
    };

    virtual bool mayEqual(MemLocObjectPtr o) const { mayEqual(o); };
    virtual bool mustEqual(MemLocObjectPtr o) const { mustEqual(o); };
    
    bool isLive(PartEdgePtr pedge) const { return true; };

    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) {
      ROSE_ASSERT(false && "Not supported!");
    };

    MemLocObjectPtr copyML() const {
      return boost::make_shared<SSADefault>(ssa, expr, memLoc, part);
    };
  };

  class SSAMemLocFactory {
  public:
    static SSAMemLocPtr createSSAMemLoc(SgNode* sgn, HeapSSA* ssa);
    static SSAMemLocPtr createSSAMemLoc(SgNode* expr, MemLocObjectPtr memLoc, HeapSSA* ssa);
  };

  typedef map<SgNode*, SSAMemLocPtr> NodeAMOTable;
  typedef map<SgNode*, HeapReachingDefPtr> NodeHeapReachingDefTable;
  typedef boost::unordered_map<SgNode*, std::set<SgNode* > > LocalDefUseTable_;
  typedef map<SgNode*, ReachingDef::ReachingDefPtr> NodeReachingDefTable_;
  // typedef map<SgNode* , AliasSetPtr> NodeAliasSetTable;
  
  class HeapSSA : public StaticSingleAssignment {
  public:
    HeapSSA(SgProject* proj) : StaticSingleAssignment(proj), alreadyBuild(false) {
      // heapVarManager = new HeapVariableManager(this);
    };
      ~HeapSSA() {};
      
      ChainComposer* composer;

    void build(SgProject* proj, bool interprocedural, bool treatPointersAsStructures);
    void build(bool interprocedural, bool treatPointersAsStructures);
    bool mustBeSame(SSAMemLocPtr memLoc1,SSAMemLocPtr memLoc2);
    bool mustBeSame(SSAMemLoc* memLoc1, SSAMemLoc* memLoc2);
    bool mustBeSame(SSAMemLoc* memLoc1, SSAMemLoc* memLoc2, bool& mayBeSame);
    bool mayBeSame(SSAMemLocPtr memLoc1, SSAMemLocPtr memLoc2);
    bool mayBeSame(SSAMemLoc* memLoc1, SSAMemLoc* memLoc2);
    bool mayBeDifferent(SSAMemLocPtr memLoc1, SSAMemLocPtr memLoc);
    bool mayBeDifferent(SSAMemLoc* memLoc1, SSAMemLoc* memLoc2);
    
  protected:
    void clearTables();
    void uniqueNameTraversal(bool treatPointersAsStructures);
    void insertHeapVariables();
    void insertDPhiFunctions();
    void renumberExtDefinitions(SgFunctionDefinition* func,
				const vector<FilteredCfgNode>& cfgNodesInPostOrder);
    void findOrCreateHeapVariable(SgType );
    void expandParentMemberDefs(SgFunctionDeclaration* function);
    void expandParentMemberUses(SgFunctionDeclaration* function);
    void insertDefsForChildMemberUses(SgFunctionDeclaration* function);
    void insertDefsForExternalVariables(SgFunctionDeclaration* function);
    void populateLocalDefsTable(SgFunctionDeclaration* function);
    multimap< StaticSingleAssignment::FilteredCfgNode,
      pair<StaticSingleAssignment::FilteredCfgNode, StaticSingleAssignment::FilteredCfgEdge> >
    insertPhiFunctions(SgFunctionDefinition* function, const std::vector<FilteredCfgNode>& cfgNodesInPostOrder);
    void renumberAllDefinitions(SgFunctionDefinition* func, const vector<FilteredCfgNode>& cfgNodesInPostOrder);
    
    void runDefUseDataFlow(SgFunctionDefinition* func);
    bool propagateDefs(FilteredCfgNode cfgNode);
    void updateIncomingPropagatedDefs(FilteredCfgNode cfgNode);
    void buildUseTable(const vector<FilteredCfgNode>& cfgNodes);
    
    bool mustBeSame(SgDotExp* dotExp1, SgDotExp* dotExp2);
    bool mustBeSame(SgArrowExp* arrowExp1, SgArrowExp* arrowExp2);
    bool mustBeSame(SgPointerDerefExp* pdrExp1, SgPointerDerefExp* pdrExp2);
    bool mustBeSame(SgDotExp* dotExp, SgPointerDerefExp* pdrExp);
    bool hasSameReachingDef(SgNode* sgn1, SgNode* sgn2);
    
    // Heap variable related functions
    void hv_buildAMOs(SgFunctionDeclaration* function);
    void hv_buildAMO(SgNode* sgn);
    void hv_setCurrentFunction(SgFunctionDefinition* func) { hv_currFunc = func; };
    void hv_getDummyDefForHeapVar(StaticSingleAssignment::LocalDefUseTable& originalDefTable);
    bool hv_hasHeapUse(SgNode* sgn);
    bool hv_updateUseReachingDef(SgNode* sgn, ReachingDef::ReachingDefPtr reachingDef);
    
  public:
    bool hv_hasDPhi(SgNode* sgn);
    HeapReachingDefPtr hv_getDPhi(SgNode* sgn);
    bool hv_isHeapVar(const StaticSingleAssignment::VarName& varName);
    const StaticSingleAssignment::VarName& hv_getCurrentHeapVar();
    bool hv_addHeapDef(SgNode* defNode, SgNode* heapDef);
    bool hv_addDPhi(SgNode* sgn);
    // bool hv_hasHeapLattice(ReachingDef::ReachingDefPtr reachingDef);
    // void hv_addHeapLattice(ReachingDef::ReachingDefPtr reachingDef);
    bool hv_addHeapUse(SgNode* useNode, SgNode* heapUse);
    const StaticSingleAssignment::VarName& hv_getHeapVarName(SgExpression* sgn);
    // SSAMemLocPtr hv_hasHeapLattice(SSAMemLocPtr memLocPtr, HeapLatticePtr heapLatticePtr);
    // HeapLatticePtr hv_getHeapLattice(ReachingDef::ReachingDefPtr reachingDef);
    bool hv_hasHeapReachingDef(SgNode* sgn);
    // void hv_joinHeapSSALattice(HeapLatticePtr heapLattice1, HeapLatticePtr heapLattice2);
    ReachingDef::ReachingDefPtr hv_getHeapReachingDef(SgNode* sgn);
    bool hv_hasAMO(SgNode* sgn);
    SSAMemLocPtr hv_getAMO(SgNode* sgn);
    bool hv_setAMO(SgNode* sgn, SSAMemLocPtr memLocPtr);
    // bool hv_hasAlias(SgNode* sgn);
    // AliasSetPtr hv_getAlias(SgNode* sgn);
    // void hv_addAlias(SgNode* sgn, AliasSetPtr another);
    
  protected:
    bool alreadyBuild;
    
    // SgNode --> AMO map
    NodeAMOTable hv_amoTable;
    
    // SgNode --> def phi function map
    NodeHeapReachingDefTable hv_dphiTable;
    
    // SgNode --> use SgNodes map
    LocalDefUseTable_ hv_localUses;
    
    // SgNode --> def SgNode map
    LocalDefUseTable_ hv_localDefs;
    
    // SgNode --> extended reaching def map
    NodeReachingDefTable_ hv_reachingDefTable;
    
    // Heap Variable names
    map<SgScopeStatement*, VarUniqueName * > hv_varNames;

    // DPhi/Phi --> Heap Product Lattice map
    // map<ReachingDef::ReachingDefPtr, HeapLatticePtr > hv_phiHeapLatticeMap;
    
    // Variable to alias set table                                                                   
    // NodeAliasSetTable hv_aliasTable;
    
    // Current Function
    SgFunctionDefinition* hv_currFunc;
    
    static HeapReachingDefPtr emptyHeapReachingDefPtr;
    static SSAMemLocPtr emptySSAMemLocPtr;
  };
};
