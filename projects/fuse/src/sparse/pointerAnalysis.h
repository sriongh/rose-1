#pragma once

#include <map>
#include <set>
#include <utility>
#include "rose.h"
#include "staticSingleAssignment.h"
#include <boost/foreach.hpp>
#include "VirtualCFGIterator.h"
#include "cfgUtils.h"
#include "CallGraphTraverse.h"
#include "analysis.h"
#include "latticeFull.h"
#include "heapSSA.h"
#include "heapReachingDef.h"
#include "abstract_object_map.h"
#include "PGSSA.h"

namespace scc_private
{
  using namespace boost;
  using namespace std;

  using namespace fuse;

  //////////////////////////////////////////////////////////////////////////////////////
  ///
  /// Pointer Analysis based on Sparse Framework
  ///
  /////////////////////////////////////////////////////////////////////////////////////
  using namespace hssa_private;

  /// Sparse PointTo Memory Location
  class SPTMemLoc;
  typedef boost::shared_ptr<SPTMemLoc> SPTMemLocPtr;
  
  /// Sparse PointTo Analysis Transfer
  class SparsePointToAnalysisTransfer;

  /// Sparse PointTo Analysis
  class SparsePointToAnalysis;

  class SPTMemLoc : virtual public SSAMemLoc {
  public:
    SPTMemLoc(HeapSSA* ssaInstance, SgExpression* expr, MemLocObjectPtr memLoc, PartPtr part) 
      : SSAMemLoc(ssaInstance, expr, memLoc, part), MemLocObject(expr), analysis(NULL) {};
    SPTMemLoc(HeapSSA* ssaInstance, SgExpression* expr, MemLocObjectPtr memLoc, PartPtr part, 
	      SparsePointToAnalysis* analysis_)
      : SSAMemLoc(ssaInstance, expr, memLoc, part), MemLocObject(expr), analysis(analysis_) {};
    SPTMemLoc(SSAMemLocPtr memLoc_, SparsePointToAnalysis* analysis_);

    // pretty print for the object                     
    std::string str(std::string indent="") const {
      std::string str = "PT[ " + memLoc->str() + "]";
      return str;
    };
    std::string str(std::string indent="") { return ((const SPTMemLoc*)this)->str(indent); }
    std::string strp(PartEdgePtr pedge, std::string indent="") const {
      return this->str();
    };

    /// Compare the may/must equal for the given memory location
    virtual bool mayEqualML(MemLocObjectPtr o) const;
    virtual bool mustEqualML(MemLocObjectPtr o) const;

    /// Compare the may/must equal for the given memory location and part edge
    virtual bool mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge);
    virtual bool mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge);

    MemLocObjectPtr copyML() const {
      return boost::make_shared<SPTMemLoc>(ssa, expr, memLoc, part, analysis);
    };

    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) {
      return false;
    }

  protected:
    SparsePointToAnalysis * analysis;
  };

  /// TODO: Should use CompSharedPtr then it can call the customized operator, e.g <=!!
  typedef std::set<SPTMemLocPtr > PTSet;

  class SPTLattice;
  typedef boost::shared_ptr<SPTLattice> SPTLatticePtr;

  class SPTLattice : public FiniteLattice {
  protected:
    PTSet ptSet;
    bool full;
 
  public: 
    SPTLattice(PartEdgePtr pedge);
    SPTLattice(const SPTLattice& that) : Lattice(that.pedge), FiniteLattice(that.pedge), full(false) {};

      /// Part edge
      PartEdgePtr pedge;

      /// pretty print for the object
      std::string str(std::string indent="") const {
	std::string str = "SPT Lattice";
	return str;
      };
      std::string str(std::string indent="") { return ((const SPTLattice*)this)->str(indent); }
      std::string strp(PartEdgePtr pedge, std::string indent="") const {
	return this->str();
      };

      /// Initialization
      void initialize() {};
      
      /// Copy lattice
      Lattice* copy() const;

      void copyPTSet(const PTSet& ptSet_) {
	ptSet = ptSet_;
      }

      /// Update current lattice's point-to set with current lattice's point-to set
      bool update(SPTLatticePtr another);

      /// Add the given memory location to point-to set
      bool add(SPTMemLocPtr memLoc);

      /// Meet update
      bool meetUpdate(Lattice* lattice);
      bool meetUpdate(SPTLatticePtr lattice);

      bool operator==(Lattice* lattice);

      bool setToFull() { return full = true; };
      
      bool setToEmpty() { 
	ptSet.clear(); 
	return true;
      };

      bool isFull() { return full; };
      
      bool isEmpty() { return ptSet.size() == 0; };

      bool isFullLat() { return isFull(); };

      bool isEmptyLat() { return isEmpty(); };

      bool setMLValueToFull(MemLocObjectPtr memLoc) { 
	ROSE_ASSERT(false && "not support"); 
	return true; 
      };

      int getNumOfAlias() {
	return ptSet.size();
      };

      PTSet& getPTSet() {
	return ptSet;
      };

      void dump(PGSSA* pgssa);
  };

  /// Sparse PointTo Analysis
  class SparsePointToAnalysis : virtual public PGSSAAnalysis {
  public:
    SparsePointToAnalysis() : hasVisitor(false) {};
    SparsePointToAnalysis(SparsePointToAnalysis* oldAnalysis) : hasVisitor(false) {};

    void genInitLattice(const Function& func, PartPtr part, PartEdgePtr pedge,
                        std::vector<Lattice*>& initLattices);

    bool transfer(const Function& func, PartPtr part, CFGNode cn, NodeState& state,
                  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) { 
      ROSE_ASSERT(false); 
      return false; 
    };

    boost::shared_ptr<PGSSAIntraProcTransferVisitor>
      getSSATransferVisitor(// const Function& func, 
			    PartPtr part, CFGNode cn, NodeState& state,
			    std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

    /// Query interface called by composer  
    MemLocObjectPtr Expr2MemLoc(SgNode* sgn, PartEdgePtr pedge);
    bool implementsExpr2MemLoc() { return true; }
    bool implementsExpr2Val() { return false; };

    std::string str(std::string indent) { return "Sparse PointTo Analysis"; };

    /// Create the PointTo memory location on demand
    SPTMemLocPtr getSPTMemLoc(SgExpression* expr);
    SPTMemLocPtr getSPTMemLoc(SSAMemLocPtr memLoc);
    
    /// Check may/must alias regarding the point-to map
    bool mayAlias(SgExpression* expr, SPTMemLocPtr memLoc);
    bool mustAlias(SgExpression* expr, SPTMemLocPtr memLoc);
    
    /// Register expression with memory location
    void handleExpr(SgExpression* expr);

    /// Get object map
    PGSSAObjectMap* getObjectMap(PartEdgePtr pedge);

    /// Get initial work list
    set<PartPtr> getInitialWorklist() {
      ROSE_ASSERT(false && "PT init worklist");
      return getComposer()->GetStartAStates(this);
    }

    /// Get iterator
    dataflowPartEdgeIterator* getIterator() {
      set<PartPtr> terminalStates = getComposer()->GetEndAStates(this);
      return new fw_dataflowPartEdgeIterator();
    }

    /// Transfer function
    bool transfer(PartPtr p, CFGNode cn, NodeState& state,
		  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
      ROSE_ASSERT(false && "Not support!");
    }

    /// Copy the analyzer
    ComposedAnalysisPtr copy() { 
      return boost::make_shared<SparsePointToAnalysis>(new SparsePointToAnalysis(this)); 
    }

    /// Dump the point to graph
    void dumpPTG();

    friend class SparsePointToAnalysisTransfer;

  protected:
    /// Point-to graph
    std::map<SSAMemLocPtr, SPTMemLocPtr> memLocMap;
    std::map<SgNode*, SPTMemLocPtr> exprMap;

    // boost::shared_ptr<SparsePointToAnalysisTransfer> getVisitor(const Function& func);
    // SgFunctionDefinition* currentFuncDef;
    // std::map<SgFunctionDefinition* , boost::shared_ptr<SparsePointToAnalysisTransfer> > visitorMap;
    bool hasVisitor;
    boost::shared_ptr<SparsePointToAnalysisTransfer> currVisitor;
  };

  class SparsePointToAnalysisTransfer : public PGSSAValueTransferVisitor<SPTLattice> {
  protected:
    Composer* composer;
    SparsePointToAnalysis* analysis;

    // pointer to node state of the analysis at this part  
    AbstractObjectMap* productLattice;

    // used by the analysis to determine if the states modified or not  
    bool modified;
    int debugLevel;

  public:
    SparsePointToAnalysisTransfer(// const Function& func, 
				  PartPtr part, CFGNode cn, 
				  NodeState& state,
				  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
				  Composer* composer, SparsePointToAnalysis* analysis);
    
    void visit(SgVarRefExp* sgn);
    void visit(SgDotExp* sgn);
    void visit(SgAssignOp* sgn);
    void visit(SgAddressOfOp* sgn);
    void visit(SgArrowExp* sgn);
    void visit(SgPntrArrRefExp* sgn);
    void visit(SgPointerDerefExp* sgn);

    bool finish() { return modified; };
    
  public:
    SPTLatticePtr getPhiLattice(SgNode* sgn);
  };
};
