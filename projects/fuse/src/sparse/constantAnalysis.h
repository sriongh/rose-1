#pragma once

#include <map>
#include <utility>
#include "rose.h"
#include "staticSingleAssignment.h"
#include <boost/foreach.hpp>
#include "VirtualCFGIterator.h"
#include "cfgUtils.h"
#include "compose.h"
#include "partitions.h"
#include "PGSSA.h"

namespace scc_private
{
  using namespace boost;
  using namespace std;
  using namespace hssa_private;
  // using namespace fuse;
  
  typedef StaticSingleAssignment::FilteredCfgNode FilteredCfgNode;
  typedef StaticSingleAssignment::FilteredCfgEdge FilteredCfgEdge;
  typedef StaticSingleAssignment::VarName VarName;
  typedef StaticSingleAssignment::ReachingDefPtr ReachingDefPtr;
  typedef StaticSingleAssignment::UseTable UseTable;
  typedef StaticSingleAssignment::NodeReachingDefTable NodeReachingDefTable;

  typedef vector<SgNode* > Nodes;
  typedef vector<CFGNode> CFGNodes;
  typedef vector<CFGEdge> CFGEdges;
  typedef set<SgNode* > SgNodeList;
  typedef set<FilteredCfgNode> NodeList;
  typedef set<FilteredCfgEdge> EdgeList;
  // typedef map<VarName, LatticeArith * > VarLatticeMap;
  // typedef map<SgNode *, LatticeArith * > NodeLatticeMap;
  typedef boost::unordered_map<SgNode *, map<VarName, set<ReachingDefPtr> > > UseSetTable;
  typedef boost::unordered_map<SgNode *, map<SgNode *, bool> > SgNodeEdgeTable;
  typedef boost::unordered_map<SgNode *, map<VarName, ReachingDefPtr > > PHINodeTable;
 
  /////////////////////////////////////////////////////////////////////////////////
  ///
  /// Constant Analysis on Sparse Framework
  ///
  ////////////////////////////////////////////////////////////////////////////////
  class CAValueObject;
  typedef boost::shared_ptr<CAValueObject> CAValueObjectPtr;

  class SparseConstantAnalysisTransfer;
  
  class CAValueObject : public FiniteLattice, 
    public ValueObject {
  private:
    // the current value of the variable (if known)
    int value;
  
    // bool uninitialized; // Covered by case of bottom.
    
    bool undefined;
    
  public:
    bool bottom;
    bool top;
    
  public:
    CAValueObject(PartEdgePtr pedge);
      
      CAValueObject(int v, PartEdgePtr pedge);
      
      CAValueObject(int v, bool t, bool b, PartEdgePtr pedge);

      CAValueObject(const CAValueObject& that) : Lattice(that.pedge), FiniteLattice(that.pedge), 
	ValueObject(NULL) { 
      value = that.value;
      top = that.top;
      bottom = that.bottom;
    };
  
    PartEdgePtr pedge;

    // Access functions.
    int getValue() const { return value; };
      
    bool setValue(int x) { value = x; };
  
    bool setBottom() { bottom = true; top = false; };
    bool setTop() { top = true; bottom = false; };
  
    virtual bool isTop() { return top; };
    virtual bool isBottom() { return bottom; };

    // **********************************************
    // Required definition of pure virtual functions.
    // **********************************************
    void initialize() {};
  
    // returns a copy of this lattice
    Lattice* copy() const { return new CAValueObject(value, top, bottom, pedge); };
  
    // overwrites the state of "this" Lattice with "that" Lattice
    void copy(Lattice* that) {};
    
    bool operator==(Lattice* that);
  
    // computes the meet of this and that and saves the result in this
    // returns true if this causes this to change and false otherwise
    bool meetUpdate(Lattice* that);
    
    bool meetUpdate(CAValueObjectPtr that) {
      return meetUpdate(that.get());
    }
  
    // Set this Lattice object to represent the set of all possible execution prefixes.
    // Return true if this causes the object to change and false otherwise.
    bool setToFull();
  
    // Set this Lattice object to represent the of no execution prefixes (empty set)
    // Return true if this causes the object to change and false otherwise.
    bool setToEmpty();
  
    // Returns whether this lattice denotes the set of all possible execution prefixes.
    bool isFullLat();
    // Returns whether this lattice denotes the empty set.
    bool isEmptyLat();
  
    // pretty print for the object
    std::string str(std::string indent="") const;
    std::string str(std::string indent="") { return ((const CAValueObject*)this)->str(indent); }
    std::string strp(PartEdgePtr pedge, std::string indent="") const;
    
    bool mayEqualV(ValueObjectPtr o, PartEdgePtr pedge);
    bool mustEqualV(ValueObjectPtr o, PartEdgePtr pedge);
  
    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSet(AbstractObjectPtr o, PartEdgePtr pedge);
  
    bool equalSetV(ValueObjectPtr o, PartEdgePtr pedge) { ROSE_ASSERT(false && "TODO"); };

    bool subSetV(ValueObjectPtr o, PartEdgePtr pedge) { ROSE_ASSERT(false && "TODO"); };

    bool isFullV(PartEdgePtr pedge) { ROSE_ASSERT(false && "TODO"); };

    bool isEmptyV(PartEdgePtr pedge) { ROSE_ASSERT(false && "TODO"); };

    // Computes the meet of this and that and saves the result in this.
    // Returns true if this causes this to change and false otherwise.
    bool meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge);
  
    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    bool isFull(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the empty set.
    bool isEmpty(PartEdgePtr pedge);
  
    // Allocates a copy of this object and returns a pointer to it
    ValueObjectPtr copyV() const;

    // Returns true if this ValueObject corresponds to a concrete value that is statically-known
    bool isConcrete();
    // Returns the type of the concrete value (if there is one)
    SgType* getConcreteType();
   
    // Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
    // the normal ROSE mechanisms to decode it
    virtual set<boost::shared_ptr<SgValueExp> > getConcreteValue();
      
    virtual bool setMLValueToFull(MemLocObjectPtr memLoc);
    
    virtual bool subSet(AbstractObjectPtr objPtr, PartEdgePtr pedge) { return false; };
  };
  
  typedef boost::shared_ptr<CAValueObject> CAValueObjectPtr;
 
  class SparseConstantAnalysis : virtual public PGSSAAnalysis {
   
  public:
    SparseConstantAnalysis() : hasVisitor(false) {};
      SparseConstantAnalysis(SparseConstantAnalysis* oldAnalysis) : hasVisitor(false) {};

    void genInitLattice(const Function& func, PartPtr part, PartEdgePtr pedge, 
			std::vector<Lattice*>& initLattices);
  
    boost::shared_ptr<PGSSAIntraProcTransferVisitor> getSSATransferVisitor(// const Function& func, 
									   PartPtr part, 
									   CFGNode cn, 
									   NodeState& state, 
									   std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);
    
    boost::shared_ptr<ValueObject> Expr2Val(SgNode* n, PartEdgePtr pedge);
    bool implementsExpr2Val() { return true; }
    bool implementsExpr2MemLoc() { return false; }

    /// Get initial work list
    set<PartPtr> getInitialWorklist() {
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
      return boost::make_shared<SparseConstantAnalysis>(new SparseConstantAnalysis(this));
    }

    // pretty print for the object
    std::string str(std::string indent="") { return "Sparse Constant Analysis"; }
  
    friend class SparseConstantAnalysisTransfer;
  
  protected:
    PGSSAObjectMap* getObjectMap(PartEdgePtr pedge);
    
    bool hasVisitor;
    boost::shared_ptr<SparseConstantAnalysisTransfer> currVisitor;
  };

  class SparseConstantAnalysisTransfer : public PGSSAValueTransferVisitor<CAValueObject> {
  public:
    void visit(SgIntVal *sgn);
    // Arithmetic Operations
    void visit(SgBinaryOp* sgn);
    void visit(SgAssignOp *sgn);
    void visit(SgPntrArrRefExp* sgn);

    bool modified;
    bool finish() { return modified; };
   
    SparseConstantAnalysisTransfer(// const Function& func, 
				   PartPtr part, CFGNode cn, 
				   NodeState& state, 
				   std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, 
				   Composer* composer, SparseConstantAnalysis* analysis_);

  protected:
    void transferArithOp(SgExpression* sgn);
    
    CAValueObjectPtr getPhiLattice(SgNode* sgn);
  
    SparseConstantAnalysis* analysis;
  };
};
