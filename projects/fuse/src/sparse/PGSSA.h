#pragma once
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
#include "uniqueNameTraversal.h"
#include "heapReachingDef.h"
#include "heapSSA.h"
#include "abstract_object.h"
#include "abstract_object_map.h"
#include "compose.h"

#include <vector>

namespace hssa_private {
  using namespace boost;
  using namespace std;

  using namespace fuse;

  class PGSSA;
  class PGReachingDef;
  class PGSSAAnalysis;

  typedef map<SgNode*, SSAMemLocPtr> NodeAMOTable;
  typedef map<SgNode*, HeapReachingDefPtr> NodeHeapReachingDefTable;
  typedef boost::unordered_map<SgNode*, std::set<SgNode* > > LocalDefUseTable_;

  typedef set<PartPtr> PartSet;
  typedef map<SgNode*, SSAMemLocPtr> Node2MemLoc;
  typedef map<SgNode* , PartPtr> Node2Part;
  typedef map<SgNode*, int> Node2ID;
  typedef boost::shared_ptr<PGReachingDef> PGReachingDefPtr;
  typedef map<PartPtr, map<SSAMemLocPtr, PGReachingDefPtr> > Part2SSAMemLoc;
  typedef map<SSAMemLocPtr, PartPtr> SSAMemLoc2Part;
  typedef map<SSAMemLocPtr, SgNode* > SSAMemLoc2Node;
  typedef map<SSAMemLocPtr, CFGNode> SSAMemLoc2CFGNode;
  typedef map<PartPtr, boost::unordered_map<SgNode*, std::set<StaticSingleAssignment::VarName> > > PartLocalDefUseTable;
  typedef map<PartPtr, boost::unordered_map<SgNode*, set<SgNode* > > > PartLocalDefUseTable_;
  typedef map<PartPtr, map<SgNode*, map<StaticSingleAssignment::VarName, PGReachingDefPtr > > > PartReachingDefTable;
  typedef map<PartPtr, map<SgNode*, PGReachingDefPtr > > PartReachingDefTable_;
  typedef map<StaticSingleAssignment::VarName, PGReachingDefPtr> NodePGReachingDefTable;
  typedef map<SgNode*, std::set<StaticSingleAssignment::VarName > > PIDefTable;
  typedef map<PartPtr, set<PartPtr > > PartUseTable;
  typedef set<SSAMemLocPtr> SSAMemLocSet;
  typedef set<PGReachingDefPtr> ReachingDefSet;

  class PGChildUses {
  protected:
    /// An assignment to the current expression in the AST would define this variable 
    SgVarRefExp* currentVar;
    /// An assignment to the current operation in the AST would def/use this heap operations
    SgNode* currentOp;

    /// Stores all the varRefs that are used in the current subTree. 
    std::vector<SgNode*> uses;

  public:

    /// Create the attribute with no refs. 
    PGChildUses() : currentVar(NULL), currentOp(NULL) {};
      
      PGChildUses(SgNode* useNode, SgVarRefExp* var) {
	uses.push_back(useNode);
	currentVar = var;
	currentOp = NULL;
      }

      /// Create the attribute with the def and list of uses
      /// @param useTree The vector of uses to add, or an empty vector.
      PGChildUses(const std::vector<SgNode*>& useTree, SgVarRefExp* var = NULL) {
	if (useTree.size() > 0)
	  uses.assign(useTree.begin(), useTree.end());
	currentVar = var;
      }

      PGChildUses(const std::vector<SgNode*>& useTree, SgNode* op, bool isHeap) {
	if (useTree.size() > 0)
          uses.assign(useTree.begin(), useTree.end());
        if (isHeap && op != NULL)
	  currentOp = op;
	currentVar = NULL;
      }

      /// Get the uses for this node and below
      /// @return A constant reference to the use list
      std::vector<SgNode*>& getUses() { return uses; };

      /// Set the uses for this node and below.
      /// @param newUses A constant reference to the uses to copy to this node.
      void setUses(const std::vector<SgNode*>& newUses) {
	uses.assign(newUses.begin(), newUses.end());
      };

      SgVarRefExp* getCurrentVar() const { return currentVar; };

      SgNode* getCurrentOp() const { return currentOp; };

      bool isVar() { return currentVar != NULL; };
      bool isHeap() { return currentOp != NULL; };
  };

  class PGDefsAndUsesTraversal : public AstBottomUpProcessing<PGChildUses> {
  protected:
    PGSSA * pgssa;
    PartPtr currPart;
    CFGNode cfgNode;

    //! If true, modifications to a value pointed to by a pointer will count as defs for the pointer itself.
    //! For example, (delete p) would be considered to modify p.
    const bool treatPointersAsStructs;
        
  public:

    /// @param treatPointersAsStructs If true, modifications to a value pointed to by a pointer will 
    ///     count as defs for the pointer itself. For example, (delete p) would be considered to modify p.
    PGDefsAndUsesTraversal(PGSSA* ssa, bool treatPointersAsStructs = true) : pgssa(ssa),
      treatPointersAsStructs(treatPointersAsStructs) {};
      
      PGDefsAndUsesTraversal(PGSSA* ssa, PartPtr part, CFGNode cfgNode_, bool treatPointersAsStructs = true) 
	: pgssa(ssa), currPart(part), cfgNode(cfgNode_), 
	treatPointersAsStructs(treatPointersAsStructs) {};

      /** Called to evaluate the synthesized attribute on every node.
       *
       * This function will handle passing all variables that are defined and used by a given operation.
       *
       * @param node The node being evaluated.
       * @param attr The attributes from the child nodes.
       * @return The attribute at this node.
       */
      virtual PGChildUses evaluateSynthesizedAttribute(SgNode* node, SynthesizedAttributesList attrs);

      static void collectDefsAndUses(PGSSA* pgssa, PartPtr startN, const std::set<PartPtr>& endNs, 
				     PartSet& visitedParts);

  protected:

      /// Mark all the variable/heap uses as occurring at the specified node. 
      void addUsesToNode(SgNode* node, std::vector<SgNode*> uses);

      /// Mark the given variable as being defined at the node. 
      void addDefForVarAtNode(SgVarRefExp* currentVar, SgNode* defNode);

      /// Mark the given variable as a PI definition at the given define node, which is a condition expression
      void addDefForPIAtNode(SgVarRefExp* currentVar, SgNode* defNode);

      /// Register the heap def/use
      void addHeapDefAndUse(SgExpression* sgn, bool isDef);

      /// Remove the pre-defined heap use
      void removeHeapUse(SgExpression* sgn);

      /// Add the Def phi function for heap def node
      void addDPhiForNode(SgNode* sgn);
  };

  class PGReachingDef {
  public:
    enum Type { ORIGINAL_DEF, EXPANDED_DEF, PHI, DPHI, PI };

  public:
    PGReachingDef(PartPtr defPart_, SgNode* defNode_, Type type)
      : defPart(defPart_), defNode(defNode_), defType(type), id(-1) {};
    PGReachingDef(PartPtr defPart_, SgNode* defNode_, int id_)
      : defPart(defPart_), defNode(defNode_), defType(PI), id(id_) {};
    PGReachingDef(PartPtr defPart_, SgNode* defNode_, Type type, PGReachingDefPtr prevDef)
      : defPart(defPart_), defNode(defNode_), defType(type), id(-1), prevDef_(prevDef) {};
    ~PGReachingDef() {}

    bool isPhiFunction() const { return defType == PHI; };
    bool isDPhiFunction() const { return defType == DPHI; };
    virtual bool isPiFunction() const { return defType == PI; };
    
    PartPtr getPart() { return defPart; };
    SgNode* getNode() { return defNode; };

    /// Set the renaming number (SSA index) of this def. 
    void setRenamingNumber(int n) { renamingNumber = n; };
    /// Get the renaming number (SSA index) of this def.
    int getRenamingNumber() { return renamingNumber; };
    
    SgNode* getDefNode() { return defNode; };
    
    PartPtr getDefPart() { return defPart; };

    void addJoinedDef(PGReachingDefPtr newDef, PartEdgePtr edge);
    void addJoinedDPhi(PGReachingDefPtr currDPhi, PGReachingDefPtr prevDPhi) {
      prevDef_ = prevDPhi;
    };
    
    bool operator==(const PGReachingDef& other) const;
    
    PGReachingDefPtr getPrevDef() { return prevDef_; };
    
    /// Get the SSA incoming edges for the phi function
    map<PGReachingDefPtr, PartEdgePtr >& getParentDefs() { 
      return parentDefs;
    };

    /// Get the ID of condition branch
    int getId() { return id; };

    /// Print information
    std::string str();

  protected:
      /// The type of reaching def
      Type defType;

      /// The definition part
      PartPtr defPart;
      // CFGNode cfgNode;

      /// The definition node
      SgNode* defNode;

      /// Renaming number
      int renamingNumber;

      /// PI function ID
      int id;
      
      /// If this is a phi node, here we store all the joined definitions and all the edges 
      /// associated with each one.
      map<PGReachingDefPtr, PartEdgePtr > parentDefs;
      
      PGReachingDefPtr prevDef_;
  };

  class PGSSA : public HeapSSA {
  public:
    PGSSA() : HeapSSA(NULL), needPiNode(false), hasDeref(false), hasAddrTaken(false), hasArrOrFieldAcc(false), 
      analysis(NULL), heapVarName(NULL), hasUniqueNames(false) {};
    PGSSA(SgProject* proj, const Function& func, PGSSAAnalysis* analysis_) : HeapSSA(proj), needPiNode(false), 
      hasDeref(false), hasAddrTaken(false), hasArrOrFieldAcc(false), analysis(analysis_), heapVarName(NULL), hasUniqueNames(false) {
      setFunction(func);
    };
    PGSSA(SgProject* proj, const Function& func, PGSSAAnalysis* analysis_, bool needPiNode_) : HeapSSA(proj), 
      needPiNode(needPiNode_), hasDeref(false), hasAddrTaken(false), hasArrOrFieldAcc(false), analysis(analysis_),
      heapVarName(NULL), hasUniqueNames(false) {
      setFunction(func);
    };

      ~PGSSA() {};
      
      // ChainComposer* composer;
      PGSSAAnalysis* getAnalysis() { return analysis; };
      void setAnalysis(PGSSAAnalysis* analysis_) { analysis = analysis_; };

      /// Build partition graph based SSA form
      bool build(SgProject* proj, bool interprocedural, bool treatPointersAsStructures);

      /// Build partition graph based SSA form
      bool build(const std::set<PartPtr>& startNodes, 
		 const std::set<PartPtr>& endNodes);

      bool build(const std::set<PartPtr>& startNodes, 
		 const std::set<PartPtr>& endNodes, bool needPiNode_);

      /// Test functions
      void postDominator(SgProject* proj);
      void createDominator(const Function& function, const std::set<PartPtr>& startNs, 
			   const std::set<PartPtr>& endNs);
      
      /// Register the SgNode to Part
      void registerNode(PartPtr part, SgNode* sgn) {
	node2Part[sgn] = part;
      };

      /// Register MemLocObject with SgNode and part
      void registerMemLocObj(SgNode* sgn, PartPtr part, CFGNode cfgNode);
      
      /// Reregister the memory location
      bool reRegisterMemLocObj(SgNode* sgn, SSAMemLocPtr oldMemLoc, SSAMemLocPtr memLoc);

      /// Get current heap variable that is corresponding to current function
      const StaticSingleAssignment::VarName& getHeapVarName(SgNode* sgn);
      
      /// Get part based local uses table
      PartLocalDefUseTable& getPartLocalUsesTable() {
	return partLocalUsesTable;
      };

      /// Get part based original definition table
      PartLocalDefUseTable& getPartOriginalDefTable() {
	return partOriginalDefTable;
      };

      /// Get part based heap uses table
      PartLocalDefUseTable_& getPartHeapUsesTable() {
	return partHeapLocalUses;
      };

      /// Set current function to PGSSA 
      void setFunction(const Function& func) {
	currFunc = func.get_declaration()->get_definition();
      };

      /// Check if the given SgNode contains definition
      bool hasDef(SgNode* node);

      /// Check if the given variable is a heap variable
      bool isHeapVar(const VarName& var) {
	ROSE_ASSERT(heapVarName);
	return var == heapVarName->getKey();
      };

      /// Check if the part has dphi function
      bool hasDPhi(PartPtr part);

      /// Check if the <part, SgNode> has dphi function 
      bool hasDPhi(PartPtr part, SgNode* sgn);

      /// Get <part, SgNode>'s dphi function
      PGReachingDefPtr getDPhi(PartPtr part, SgNode * sgn);

      /// Check if the part contains heap uses
      bool hasHeapUse(PartPtr part, SgNode* sgn);

      /// Set given node's all use SgNode with the input reaching def which is a dphi       
      bool updateHeapUseReachingDef(PartPtr part, SgNode* sgn, PGReachingDefPtr reachignDef);

      /// Add heap def/use to corresponding tables
      void addHeapDefAndUse(SgExpression* sgn, bool isDef);

      /// Create dphi function for <part, CFGNode> --> sgn
      void addDPhi(PartPtr part, CFGNode cfgNode, SgNode* sgn);

      /// Generate unique name for the SgNodes
      static bool generateUniqueName(SgProject* project, bool treatPointersAsStructures);

      /// Get the reaching def for the given part and SgNode
      PGReachingDefPtr getReachingDef(PartPtr part, SgNode* sgn);

      /// Get the reaching defs for the given part and SgNode, all phi function's incoming edges are collected
      void getReachingDefs(SgNode* sgn, set<PGReachingDefPtr>& reachingDefs);
      
      void getReachingDefs(PartPtr part, SgNode* sgn, set<PGReachingDefPtr>& reachingDefs);

      /// Get the set of reaching defs for the given expression at the given part
      void getReachingDefsAtPart(PartPtr part, SgExpression* expr, set<PGReachingDefPtr>& reachingDefs);

      /// Get the reaching def for the given part, CFGNode and MemLocObject
      PGReachingDefPtr getReachingDef(PartPtr part, SSAMemLocPtr memLoc);

      const map<PartPtr, SgNode* >& getReachingDef(PartPtr part, CFGNode cfgNode, 
						   SSAMemLocPtr memLoc);

      /// Get the given SgNode's MemLoc
      SSAMemLocPtr getMemLocObject(SgNode* sgn);

      /// Update the memory location for the given SgNode
      void updateMemLocObject(SgNode* sgn, SSAMemLocPtr memLoc);

      /// Get the given expression's may MemLocObject, this can be reaching def for either scalar 
      /// or heap reaching def
      void getDefMemLocs(SgExpression* expr, PartPtr part, SSAMemLocSet& memLocs, bool mustDef = false);

      /// Collect the heap reaching defs (SSAMemLoc based) for the given SSAMemLoc, til mustEqual 
      void collectHeapRDs(PGReachingDefPtr heapRD, SSAMemLocPtr memLoc, SSAMemLocSet& memLocs, 
			  ReachingDefSet& visited, bool mustDef = false);

      /// Get one of dphi node from the given part, here we just pick up the 1st one, since using the single 
      /// heap variable
      PGReachingDefPtr getAnyDPhi(PartPtr part);

      /// Get the given SgNode's partition graph node
      PartPtr getPart(SgNode* sgn) {
	if (node2Part.count(sgn) > 0)
	  return node2Part[sgn];
	else
	  return EmptyPart;
      };

      PIDefTable& getPIDefTable() {
	return piDefTable;
      };

      /// Check the conditional statements, e.g. SgIfStatement, SgSwitchStatement, to insert the PI function
      void handleCondStmt(SgStatement* condStmt);

      /// Add the PI reaching defs for the given conditional statement
      void addPiReachingDefs(SgStatement* stmt, SgStatement* trueStmt, SgStatement* falseStmt, 
			     SgExpression* condExpr, int trueId, int falseId);

      SgNode* getNode(PartPtr part, SSAMemLocPtr memLoc) {
	if (memLoc2Node.find(memLoc) != memLoc2Node.end())
	  return memLoc2Node[memLoc];
	else
	  return NULL;
      }

      /// Collect the Scalar SSA uses based on part
      void collectScalarUseParts(PartPtr part, set<PartPtr>& partSet);
      /// Collect the Heap SSA uses based on part
      void collectHeapUseParts(PartPtr part, set<PartPtr>& partSet);

      /// Collect the SSA uses based on part
      void collectUseParts(PartPtr part, SgNode* sgn, set<PartPtr>& partSet);

      /// Check if the PGSSA has pointer dereference
      bool hasPointerDeref() { return hasDeref; };

      /// Set the pointer dereference flag
      void setPointerDeref(bool flag) { hasDeref = flag; };

      /// Check if the PGSSA has address taken
      bool hasAddressTaken() { return hasAddrTaken; };

      /// Set the address taken flag
      void setAddressTaken(bool flag) { hasAddrTaken = flag; };

      /// Check if the PGSSA has array access or class/struct field access
      bool hasArrayAcc() { return hasArrOrFieldAcc; };

      /// Set the array access or class/struct field access
      void setArrayAcc(bool flag) { hasArrOrFieldAcc = true; };

  protected:
      /// Clean the internal tables
      void clearTables();

      /// Insert the def for the external variables
      void insertDefsForExternalVariables(SgFunctionDeclaration* function);
      
      /// Create the dummy def for the heap variable at the start node of partition graph
      void getDummyDefForHeapVar(PartLocalDefUseTable& partOrigDefTable, 
				 const std::set<PartPtr>& startNodes);
				 // SgFunctionDeclaration* function);

      /// Create PGReachingDef objects for each scalar local def and insert them in the local def table.
      void populateLocalDefsTable();
      void populateLocalDefsTable(SgFunctionDeclaration* function);

      /// Renumber all definitions for variables
      void renumberAllDefinitions(// SgFunctionDefinition* func, 
				  const std::set<PartPtr>& startNodes, 
				  const std::set<PartPtr>& endNodes);
      
      /// Once all the local definitions have been inserted in the ssaLocalDefsTable and phi functions have been 
      /// inserted in the reaching defs table, propagate reaching definitions along the partition graph
      void runDefUseDataFlow(// SgFunctionDefinition* func, 
			     const std::set<PartPtr>& startNodes, 
			     const std::set<PartPtr>& endNodes);
      
      /// Performs the data-flow update for one individual node, populating the reachingDefsTable for that node.
      /// @returns true if the OUT defs from the node changed, false if they stayed the same.
      bool propagateDefs(PartPtr part);

      /// Take all the outgoing defs from previous nodes and merge them as the incoming defs
      /// of the current node.
      void updateIncomingPropagatedDefs(PartPtr part);
      
      /// Insert phi functions at join points
      void insertPhiFunctions(// SgFunctionDefinition* func, 
			      const std::set<PartPtr>& startNode, 
			      const std::set<PartPtr>& endNodes);
      
      void buildSSAUseTable(const std::set<PartPtr>& endNodes);

      /// Check if the given expression is in the given SgNode
      bool isNodeInNode(SgNode* expr, SgNode* sgn);

      /// Get the real definition for the given variable and Pi function
      PGReachingDefPtr getOrigDefForPi(PGReachingDefPtr piDef, const StaticSingleAssignment::VarName& var);

      /// Check if the PGSSA support PI node
      bool needPiNodeSupport() { return needPiNode; };

  protected:
      /// Current analysis that PGSSA is constructed for
      PGSSAAnalysis* analysis;

      /// Does current PGSSA support PI function
      bool needPiNode;

      /// The VarName data structure for constructing heap variable
      VarUniqueName* heapVarName;

      /// The function that the PGSSA constructed on
      SgFunctionDefinition* currFunc;
      
      /// The part based local uses table
      PartLocalDefUseTable partLocalUsesTable;

      /// The part based original local definition table
      PartLocalDefUseTable partOriginalDefTable;
      
      /// The part based heap variable local uses
      PartLocalDefUseTable_ partHeapLocalUses;

      /// The local definition table used for propagating definitions
      boost::unordered_map<PartPtr, NodePGReachingDefTable> pgssaLocalDefTable;

      /// PartPtr --> SgNode --> PGReachingDefPtr
      PartReachingDefTable_ partDPhiTable;

      /// PartPtr --> SgNode --> VarName --> PGReachignDefPtr
      PartReachingDefTable partInReachingDefTable;
      PartReachingDefTable partOutReachingDefTable;

      /// PartPtr --> {PartPtr}
      PartUseTable partUseTable;
      /// PartPtr --> {PartPtr} for heap operations
      PartUseTable heapPartUseTable;

      /// SgNode --> Part
      Node2Part node2Part;

      /// SgNode --> PI part
      Node2Part node2PI;
      
      /// SgNode --> set<var> this is for PI functions 
      PIDefTable piDefTable;

      /// SgNode --> SSAMemLocPtr
      Node2MemLoc node2MemLoc;

      /// SSAMemLocPtr --> SgNode
      SSAMemLoc2Node memLoc2Node;

      /// SSAMemLocPtr --> PartPtr
      SSAMemLoc2Part memLoc2Part;

      /// SSAMemLocPtr --> CFGNode
      SSAMemLoc2CFGNode memLoc2CFGNode;
    
      /// The function has pointer dereference
      bool hasDeref;
     
      /// The function has address taken
      bool hasAddrTaken;
      
      /// The function has array access or struct/class field access
      bool hasArrOrFieldAcc;

      /// The unique names has been generated or not
      bool hasUniqueNames;

  public:
      /// The empty Reaching Def
      static PGReachingDefPtr EmptyReachingDef;

      /// The empty SSA memory location object pointer
      static SSAMemLocPtr EmptySSAMemLoc;

      /// The empty part pointer
      static PartPtr EmptyPart;

      /// The empty value object pointer
      static ValueObjectPtr EmptyValueObject;

      /// The empty lattice object pointer
      static LatticePtr EmptyLattice;
      
      /// Create PGSSA memory location object
      static SSAMemLocPtr createSSAMemLoc(SgNode* expr, PartPtr part, PGSSA* pgssa);
  };

  class PGSSAObjectMap : public AbstractObjectMap {
  protected:
    map<SSAMemLocPtr, LatticePtr> internalTable;

    PGSSA* pgssa;

  public:
    PGSSAObjectMap(const AbstractObjectMap& that);
    PGSSAObjectMap(LatticePtr defaultLat_, PartEdgePtr pedge, Composer* comp,
		   ComposedAnalysis * analysis);
    ~PGSSAObjectMap() {};
	
    Lattice* remapML(const std::set<pair<MemLocObjectPtr, MemLocObjectPtr> >& ml2ml,
		     PartEdgePtr newPEdge);
    
    /// Insert lattice
    bool insertValue(SSAMemLocPtr memLoc, LatticePtr valObj);
  
    /// Remove lattice
    bool removeValue(SSAMemLocPtr memLoc, LatticePtr valObj);
    
    /// Get lattice 
    LatticePtr getValue(SSAMemLocPtr memLoc);

  protected:
    /// Get the lattice for a given Phi function
    LatticePtr getPhiValue(PGReachingDefPtr phiRD);
  };

  /// The PGSSA based intra-procedural transfer visitor
  class PGSSAIntraProcTransferVisitor : public DFTransferVisitor { // IntraDFTransferVisitor {
  protected:
    // Partition graph based SSA form
    PGSSA* pgssa;

    Composer* composer;

  public:
    PGSSAIntraProcTransferVisitor(// const Function &f, 
				  PartPtr p, CFGNode cn, NodeState &s,
				  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, 
				  PGSSA* pgssa_, Composer* composer_)
      : DFTransferVisitor(p, cn, s, dfInfo), pgssa(pgssa_), composer(composer_) {};

    virtual bool finish() = 0;
    virtual ~PGSSAIntraProcTransferVisitor() {};

    virtual set<PartPtr> getParts() = 0;

    virtual void setLatticeMap(PGSSAObjectMap* objMap_) = 0;

  protected:
    /// Modified parts
    set<PartPtr> reanalysisParts;

    /// Query functions
    /// Get SgNode's corresponding SSA MemLocObject
    virtual SSAMemLocPtr getMemLocObject(SgNode* sgn);

    /// Collect the parts by SSA use edges
    void collectUseParts(PartPtr part);

    void collectUseParts(PartPtr part, SgNode* sgn);
  
    /// Get reaching def
    PGReachingDefPtr getReachingDef(SgNode* sgn);
    
    PGReachingDefPtr getReachingDef(PartPtr part, SgNode* sgn);

    /// Get composer
    Composer* getComposer() { return composer; };
  };

  /// The PGSSA based intra-procedura transfer visitor with templete type of ValueObject
  template <class LatticeType>
  class PGSSAValueTransferVisitor : public PGSSAIntraProcTransferVisitor {
  protected:
    /// Lattice Ptr
    typedef boost::shared_ptr<LatticeType> UserLatticePtr;
  
  public:
    PGSSAValueTransferVisitor(// const Function &f, 
			      PartPtr p, CFGNode cn, NodeState &s,
			      std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, 
			      PGSSA* pgssa, Composer* composer)
      : PGSSAIntraProcTransferVisitor(p, cn, s, dfInfo, pgssa, composer), objMap(NULL) {};
      PGSSAValueTransferVisitor(// const Function &f, 
				PartPtr p, CFGNode cn, NodeState &s,
				std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, 
				PGSSA* pgssa, Composer* composer,
				AbstractObjectMap* objMap_)
	: PGSSAIntraProcTransferVisitor(p, cn, s, dfInfo, pgssa, composer), objMap(objMap_) {};

      virtual ~PGSSAValueTransferVisitor() {};

      /// Get the given SgNode's corresponding Lattice
      virtual UserLatticePtr getLattice(SgExpression *sgn) {
	// Check internal table
	SSAMemLocPtr memLoc = getMemLocObject(sgn);
	
	LatticePtr valObj = objMap->getValue(memLoc);
	if (valObj != PGSSA::EmptyLattice)
	  return boost::dynamic_pointer_cast<LatticeType>(valObj);
       
	// Check global abstract object map and reaching defs
	PartPtr part = pgssa->getPart(sgn);
	LatticeType* newLattice = new LatticeType(part->inEdgeFromAny());
	SSAMemLocSet memLocs;
	pgssa->getDefMemLocs(sgn, part, memLocs);
	for (SSAMemLocSet::iterator it = memLocs.begin(); it != memLocs.end(); it ++) {
	  SgExpression* expr = (* it)->getVarExpr();
	  if (expr)
	    newLattice->meetUpdate(getValueObject(expr));
	}
	return UserLatticePtr(newLattice);
      };

      /// Get the given SgNode's corresponding Lattice
      virtual UserLatticePtr getOrigLattice(SgExpression *sgn) {
        // Check internal table
        SSAMemLocPtr memLoc = getMemLocObject(sgn);

        LatticePtr valObj = objMap->getValue(memLoc);
        return boost::dynamic_pointer_cast<LatticeType>(valObj);
      };

      /// Get the given SgNode's corresponding Lattice
      virtual UserLatticePtr getCurrentLattice(SgExpression *sgn, bool mustDef = false) {
	// Check global abstract object map and reaching defs
	SSAMemLocPtr memLoc = getMemLocObject(sgn);
	LatticePtr valObj = objMap->getValue(memLoc);
        
	if (valObj != PGSSA::EmptyLattice)
          return boost::dynamic_pointer_cast<LatticeType>(valObj);
	
	PartPtr part = pgssa->getPart(sgn);
        LatticeType* newLattice = new LatticeType(part->inEdgeFromAny());
	
	SSAMemLocSet memLocs;
	pgssa->getDefMemLocs(sgn, part, memLocs, mustDef);
        for (SSAMemLocSet::iterator it = memLocs.begin(); it != memLocs.end(); it ++) {
	  SgExpression* expr = (* it)->getVarExpr();
	  if (expr)
	    newLattice->meetUpdate(getValueObject(expr));
	}
    
	return UserLatticePtr(newLattice);
      };

      /// Get the given SgExpression's corresponding Lattice at the given Partition Graph node 
      /// The expression itself may not be in current part
      virtual UserLatticePtr getLatticeAtPart(SgExpression* expr, PartPtr part) {
	// Get the definition part
	PartPtr defPart = pgssa->getPart(expr);

	if (defPart == part) {
	  // The definition part is the given part
	  // Check internal table
	  SSAMemLocPtr memLoc = getMemLocObject(expr);

	  if (memLoc != PGSSA::EmptySSAMemLoc) {
	    LatticePtr valObj = objMap->getValue(memLoc);
	    if (valObj != PGSSA::EmptyLattice)
	      // If we can find lattice, then return it
	      return boost::dynamic_pointer_cast<LatticeType>(valObj);
	  }
	}

        // Check global abstract object map and reaching defs
        LatticeType* newLattice = new LatticeType(part->inEdgeFromAny());

	SSAMemLocSet memLocs;
        pgssa->getDefMemLocs(expr, part, memLocs);
        for (SSAMemLocSet::iterator it = memLocs.begin(); it != memLocs.end(); it ++) {
	  SgExpression* expr = (* it)->getVarExpr();
	  if (expr)
	    newLattice->meetUpdate(getValueObject(expr));
	}

        return UserLatticePtr(newLattice);
      };

      virtual bool setLattice(SgNode* sgn, UserLatticePtr valObj) {
	return setValueObject(sgn, valObj, true);
      };

      virtual bool setLattice(SgNode* sgn, UserLatticePtr valObj, bool flag) {
	return setValueObject(sgn, valObj, flag);
      };

      void setLatticeMap(PGSSAObjectMap* objMap_) {
	objMap = objMap_;
      };

      virtual set<PartPtr> getParts() {
	return reanalysisParts;
      };

  protected:
      /// The global lattice map
      PGSSAObjectMap* objMap;
      /// Internal lattice table used for storing the value for the SgNodes that are not presented as MemLocObject
      
      ///Get the given SgNode's corresponding ValueObject, i.e. Lattice
      virtual UserLatticePtr getValueObject(SgNode* sgn) {
	SSAMemLocPtr memLoc = getMemLocObject(sgn);
	return boost::dynamic_pointer_cast<LatticeType>(objMap->get(memLoc));
      };

      /// Get the given part + MemLoc's corresponding ValueObject, i.e. Lattice
      virtual UserLatticePtr getValueObject(PartPtr part, SSAMemLocPtr memLoc) {
	// TODO: handle part?
	return boost::dynamic_pointer_cast<LatticeType>(objMap->get(memLoc));
      };

      /// Set the given SgNode's corresponding ValueObject, i.e. Lattice
      virtual bool setValueObject(SgNode* sgn, UserLatticePtr valObj, bool needReanalysis) {
	SSAMemLocPtr memLoc = getMemLocObject(sgn);
	if (memLoc == PGSSA::EmptySSAMemLoc)
	  return false;

	if (objMap->insertValue(memLoc, valObj) && needReanalysis) {
	  PartPtr part = pgssa->getPart(sgn);
	  if (part != PGSSA::EmptyPart) {
	    collectUseParts(part, sgn);
	    return true;
	  } 
	}

	return false;
      };

      /// Set thegiven part + MemLoc's corresponding ValueObject, i.e. Lattice
      virtual bool setValueObject(PartPtr part, SSAMemLocPtr memLoc, UserLatticePtr valObj, bool needReanalysis) {
	// TODO: handle part?
	if (objMap->insertValue(memLoc, valObj) && needReanalysis) {
	  collectUseParts(part);

	  return true;
	}

	return false;
      };

      /// Handle the phi functions 
      virtual UserLatticePtr getPhiLattice(SgNode* sgn) = 0;
  };

  /// A default implementation of PGSSA based transfer class
  class DefaultPGSSATransferVisitor : public PGSSAIntraProcTransferVisitor {

  public:
    DefaultPGSSATransferVisitor(// const Function &f, 
				PartPtr p, CFGNode cn, NodeState &s,
				std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, 
				PGSSA* pgssa, Composer* composer)
      : PGSSAIntraProcTransferVisitor(// f, 
				      p, cn, s, dfInfo, pgssa, composer) {}
      virtual ~DefaultPGSSATransferVisitor() {};

      virtual bool finish() { return false; };
      virtual set<PartPtr> getParts();

      virtual void setLatticeMap(PGSSAObjectMap* objMap_) {};
  };

  typedef boost::shared_ptr<PGSSAIntraProcTransferVisitor> PGSSAIntraProcTransferVisitorPtr;

  /// The PGSSA based analysis 
  class PGSSAAnalysis : public virtual ComposedAnalysis {
  protected: 
    /// Current project
    SgProject* project;

    /// Partition graph based SSA object
    PGSSA pgssa;

    /// The global lattice map
    // std::map<SgFunctionDefinition*, PGSSAObjectMap* > funcObjMap;
    PGSSAObjectMap* currentObjMap_;

    /// The current function definition 
    SgFunctionDefinition* currFuncDef;

  public:
    PGSSAAnalysis() : currFuncDef(NULL), currentObjMap_(NULL) {};
    PGSSAAnalysis(SgProject* project_) : project(project_), currFuncDef(NULL), currentObjMap_(NULL) {};

      // NodeState* initializeFunctionNodeState(const Function &func, NodeState *fState);

    /// Get the initial work list (i.e. FlowWorkList) element for Sparse analysis
    // std::list<PartPtr> getInitialWorklist(const Function &func, bool analyzeDueToCallers, 
      //					  const set<Function> &calleesUpdated, NodeState *fState);
      virtual std::set<PartPtr> getInitialWorklist();
    
    /// These 4 functions may not be useful
    std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticeAnte(NodeState *state);
    std::map<PartEdgePtr, std::vector<Lattice*> >& getLatticePost(NodeState *state);
   
    void setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite);
    void setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite);
  
    list<PartPtr> getDescendants(PartPtr p);
    list<PartEdgePtr> getEdgesToDescendants(PartPtr part);
    std::set<PartPtr> getUltimate(); // (const Function &func);
    dataflowPartEdgeIterator* getIterator(const Function &func);
  
    PGSSA* getPGSSA() { return &pgssa; };

    /// Use forward direction
    direction getDirection() { return fw; }

    /// Remaps the given Lattice across the scope transition (if any) of the given edge
    void remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat);

    // void runAnalysis(const Function& func, NodeState* fState, bool analyzeFromDirectionStart, 
    // 			     std::set<Function> calleesUpdated);
    void runAnalysis();

    virtual void initNodeState(PartPtr part) {
      //#SA (8/18/14) TODO: Implement support for tight composition
      assert(false);
    }

    //! TightComposer implents the following method by calling generic version of this function on each analysis.
    //! FWDataflow, BWDataflow which are dataflow anlayses implements this method by passing itself to the generic version.
    virtual void transferPropagateAState(PartPtr part, std::set<PartPtr>& visited, bool firstVisit, 
                                         std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt, anchor curPartAnchor, 
                                         sight::structure::graph& worklistGraph, std::map<PartPtr, std::set<anchor> >& toAnchors,
                                         std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors) {
      //#SA (8/18/14) TODO: Implement support for tight composition
      assert(false);
    }

    virtual bool transfer(const Function& func, PartPtr part, CFGNode cn, NodeState& state,
			  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);
    virtual std::string str(std::string);

  protected:
    /// Visit the partition graph node, including all CFG nodes within the part
    bool visitPart(// const Function& func, 
		   PartPtr part, list<PartPtr>& SSAWorkList);

    /// Get the PGSSA based transfer visitor
    virtual PGSSAIntraProcTransferVisitorPtr getSSATransferVisitor(// const Function& func, 
								   PartPtr part, CFGNode cn,
								   NodeState& state,
								   map<PartEdgePtr, vector<Lattice*> >& dfInfo);
  
    /// Collect the lattices corresponding to the given part
    void collectDefsLattice(PartPtr part, vector<Lattice*>& dfInfo);
  
    /// Get current object map with given function and part edge
    PGSSAObjectMap* currentObjMap(const Function& func, PartEdgePtr pedge);

    /// Get current object map with part edge
    PGSSAObjectMap* currentObjMap(PartEdgePtr pedge);

    /// Get current object map
    PGSSAObjectMap* currentObjMap();

    /// Create a new object map
    virtual PGSSAObjectMap* getObjectMap(PartEdgePtr pedge) = 0;
  };
};
