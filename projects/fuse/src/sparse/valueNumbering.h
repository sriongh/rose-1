#pragma once

#include <map>
#include <utility>
#include "rose.h"
#include "staticSingleAssignment.h"
#include <boost/foreach.hpp>
#include "VirtualCFGIterator.h"
#include "cfgUtils.h"
#include "analysis.h"
#include "latticeFull.h"
#include "heapSSA.h"
#include "heapReachingDef.h"
#include "compose.h"
#include "PGSSA.h"
#include "abstract_object_map.h"

namespace scc_private
{
  using namespace boost;
  using namespace std;
  // using namespace hssa_private;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ///
  /// Value Numbering on Sparse Framework
  ///
  ////////////////////////////////////////////////////////////////////////////////////////////////////
 
  using namespace hssa_private;
  
  class SVNLattice;
  typedef boost::shared_ptr<SVNLattice> SVNLatticePtr;

  class SVNLattice : public FiniteLattice {
  protected:
    int valId;
    bool full;
    SgExpression* value;

  public:
    SVNLattice(PartEdgePtr pedge);
    SVNLattice(PartEdgePtr pedge, SgExpression* value_, int valId_);
    SVNLattice(const SVNLattice& that) : Lattice(that.pedge), FiniteLattice(that.pedge), full(false), valId(that.valId) {};

      /// Part edge
      PartEdgePtr pedge;

      /// pretty print for the object
      std::string str(std::string indent="") const {
	std::string str = "SVN Lattice";
        return str;
      };
      std::string str(std::string indent="") { return ((const SVNLattice*)this)->str(indent); }
      std::string strp(PartEdgePtr pedge, std::string indent="") const {
        return this->str();
      };

      /// Initialization
      void initialize() {};

      /// Copy lattice
      Lattice* copy() const;

      /// Meet update
      bool meetUpdate(Lattice* lattice);
      bool meetUpdate(SVNLatticePtr lattice);

      bool operator==(Lattice* lattice);

      bool setToFull() { return full = true; };
      
      bool setToEmpty() {
	return true;
      };

      bool isFull() { return full; };

      bool isEmpty() { return this->valId == -1; };

      bool isFullLat() { return isFull(); };

      bool isEmptyLat() { return isEmpty(); };

      /// Set the value number
      void setVN(int valId) { this->valId = valId; };

      /// Get the value number
      int getVN() { return this->valId; };

      bool setMLValueToFull(MemLocObjectPtr memLoc) {
        ROSE_ASSERT(false && "not support");
        return true;
      };

      /// Check equality
      bool equalsVal(SgExpression* valExpr);
 
      /// Return the value expression
      SgExpression* getValueExpr() { return value; };
  };

  class SVNMemLoc;
  typedef boost::shared_ptr<SVNMemLoc> SVNMemLocPtr;

  /// Sparse Value Numbering
  class SparseValueNumbering;

  class SVNMemLoc : virtual public SSAMemLoc {
  public:
    SVNMemLoc(HeapSSA* ssaInstance, SgExpression* expr, MemLocObjectPtr memLoc, PartPtr part)
      : SSAMemLoc(ssaInstance, expr, memLoc, part), MemLocObject(expr), analysis(NULL) {};
    SVNMemLoc(HeapSSA* ssaInstance, SgExpression* expr, MemLocObjectPtr memLoc, PartPtr part,
	      SparseValueNumbering* analysis_)
      : SSAMemLoc(ssaInstance, expr, memLoc, part), MemLocObject(expr), analysis(analysis_) {};
    SVNMemLoc(SSAMemLocPtr memLoc_, SparseValueNumbering* analysis_);

    // pretty print for the object
    std::string str(std::string indent="") const {
      std::string str = "VN[ " + memLoc->str() + "]";
      return str;
    };
    std::string str(std::string indent="") { return ((const SVNMemLoc*)this)->str(indent); };
    std::string strp(PartEdgePtr pedge, std::string indent="") const {
      return this->str();
    };

    /// Compare the may/must equal for the given memory location
    virtual bool mayEqualML(MemLocObjectPtr o) const;
    virtual bool mustEqualML(MemLocObjectPtr o) const;
    
    /// Compare the may/must equal for the given memory location and part edge
    virtual bool mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge);
    virtual bool mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge);
    
    /// Compare the two give expressions are same symbols
    static bool isSameSig(SgExpression* exprL, SgExpression* exprR);
    
    MemLocObjectPtr copyML() const {
      return boost::make_shared<SVNMemLoc>(ssa, expr, memLoc, part, analysis);
    };

    /// Set the value numbering lattice
    void setValue(SVNLatticePtr svnLat) { this->svnLat = svnLat; };
  
    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) {
      return false;
    }

  protected:
    SparseValueNumbering * analysis;

    /// Return the value numbering lattice
    SVNLatticePtr getValue() { return svnLat; };

    /// Value Number Lattice used for comparison
    SVNLatticePtr svnLat;
  };
  
  /// Sparse Value Numbering Transfer
  class SparseValueNumberingTransfer;

  class SparseValueNumbering : virtual public PGSSAAnalysis {

  public:
    SparseValueNumbering() : hasVisitor(false) {};
      SparseValueNumbering(SparseValueNumbering* oldAnalysis) : hasVisitor(false) {};

    void genInitLattice(const Function& func, PartPtr part, PartEdgePtr pedge,
                        std::vector<Lattice*>& initLattices);

    boost::shared_ptr<PGSSAIntraProcTransferVisitor> getSSATransferVisitor(// const Function& func,
                                                                           PartPtr part,
                                                                           CFGNode cn,
                                                                           NodeState& state,
                                                                           std::map<PartEdgePtr, std::vector<Lattice* > >& dfInfo);

    // boost::shared_ptr<ValueObject> Expr2Val(SgNode* n, PartEdgePtr pedge);
    bool implementsExpr2Val() { return false; };
    bool implementsExpr2MemLoc() { return true; };

    MemLocObjectPtr Expr2MemLoc(SgNode* sgn, PartEdgePtr pedge);

    /// Get initial work list
    virtual set<PartPtr> getInitialWorklist() {
      std::cout << "VN worklist" << std::endl;
      return getComposer()->GetStartAStates(this);
    }

    /// Get iterator
    virtual dataflowPartEdgeIterator* getIterator() {
      set<PartPtr> terminalStates = getComposer()->GetEndAStates(this);
      return new fw_dataflowPartEdgeIterator();
    }

    /// Transfer function
    virtual bool transfer(PartPtr p, CFGNode cn, NodeState& state,
			  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
      ROSE_ASSERT(false && "Not support!");
    }

    /// Copy the analyzer
    ComposedAnalysisPtr copy();

    // pretty print for the object
    virtual std::string str(std::string indent="") const { return "Sparse Value Numbering"; }

    friend class SparseConstantAnalysisTransfer;

  protected:
    PGSSAObjectMap* getObjectMap(PartEdgePtr pedge);
    
    bool hasVisitor;
    boost::shared_ptr<SparseValueNumberingTransfer> currVisitor;
  };
  
  class GlobalArithID {
  protected:
    /// Internal maps for arithmatic operators
    map<int, map<int, int> > internal_adds;
    map<int, map<int, int> > internal_subs;
    map<int, map<int, int> > internal_muls;
    map<int, map<int, int> > internal_divs;
    map<int, map<int, int> > internal_mods;
    map<int, map<int, int> > internal_ands;
    map<int, map<int, int> > internal_ors;

    /// ID counter
    int valIdCounter;

  public:
    GlobalArithID() : valIdCounter(0) {};

    /// Get the ID for binary operation, if it is an unsupported binary operation, return -1
    int getArithID(SgBinaryOp* sgn, int lID, int rID);

    /// Get the value ID
    int getValID() { return valIdCounter ++; }; 
  };

  class SparseValueNumberingTransfer : public PGSSAValueTransferVisitor<SVNLattice> {
  public:
    void visit(SgValueExp *sgn);
    void visit(SgVarRefExp* sgn);
    // Arithmetic Operations
    void visit(SgBinaryOp* sgn);
    void visit(SgDotExp* sgn);
    void visit(SgAssignOp *sgn);
    void visit(SgPntrArrRefExp* sgn);
    void visit(SgArrowExp* sgn);

    bool modified;
    bool finish() { return modified; };

    SparseValueNumberingTransfer(// const Function& func, 
				 PartPtr part, CFGNode cn,
				 NodeState& state,
				 std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
				 Composer* composer, SparseValueNumbering* analysis_);

  protected:
    virtual SSAMemLocPtr getMemLocObject(SgNode* sgn);

    SVNLatticePtr transferArithOp(SgExpression* sgn);

    SVNLatticePtr getPhiLattice(SgNode* sgn);

    /// Create a new lattice based on the value expression
    SVNLatticePtr createSVNLattice(SgExpression* expr, PartEdgePtr pedge);

    SparseValueNumbering* analysis;

    /// The set of value expressions
    set<SVNLatticePtr> values;

    /// The static global arithmatic ID manager
    static GlobalArithID gArithID;
  };
};
