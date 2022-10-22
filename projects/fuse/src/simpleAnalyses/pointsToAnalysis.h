#ifndef _POINTSTOANALYSIS_H
#define _POINTSTOANALYSIS_H

/*
 * Simple  PointsTo Analysis
 * AbstractObjectMap (ProductLattice) stores an AbstractObjectSet for each MemLocObjectPtr
 * author: sriram@cs.utah.edu
 */

#include "compose.h"
#include "composed_analysis.h"
#include "abstract_object_map.h"
#include "abstract_object_set.h"

namespace fuse
{
  class PointsToAnalysis;

  /****************************
   * PointsToAnalysisTransfer *
   ****************************/

  //! Transfer functions for the PointsTo analysis
  class PointsToAnalysisTransfer : public DFTransferVisitor
  {
    typedef boost::shared_ptr<AbstractObjectSet> AbstractObjectSetPtr;

    Composer* composer;
    PointsToAnalysis* analysis;
    // pointer to node state of the analysis at this part
    AbstractObjectMap* productLattice;
    // used by the analysis to determine if the states modified or not
    bool modified;
  public:
    PointsToAnalysisTransfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                             std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                             Composer* composer, PointsToAnalysis* analysis);

    // Set the pointer of AbstractObjectMap at this PartEdge
    void initLattice();

    bool finish();

    // Lattice access functions from the map (product lattice)
    AbstractObjectSetPtr getLattice(SgExpression* sgexp);
    AbstractObjectSetPtr getLatticeOperand(SgNode* sgn, SgExpression* operand);
    AbstractObjectSetPtr getLatticeCommon(MemLocObjectPtr ml);
    // Lattice* getLattice(const AbstractObjectPtr o);

    bool setLattice(SgExpression* sgexp, AbstractObjectSetPtr lat);
    bool setLatticeOperand(SgNode* sgn, SgExpression* operand, AbstractObjectSetPtr lat);
    bool setLatticeCommon(MemLocObjectPtr ml, AbstractObjectSetPtr lat);
    // void setLattice(const AbstractObjectPtr o, Lattice* aos);

    // Transfer functions
    void visit(SgAssignOp* sgn);
    void visit(SgPointerDerefExp* sgn);
  };

  /********************
   * PointsToAnalysis *
   ********************/

  class PointsToAnalysis : public virtual FWDataflow
  {
  public:
    PointsToAnalysis(bool useSSA) : FWDataflow(/*trackBase2RefinedPartEdgeMapping*/ false, useSSA) { }

    // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
    ComposedAnalysisPtr copy() { return boost::make_shared<PointsToAnalysis>(useSSA); }

    void genInitLattice(const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                        std::vector<Lattice*>& initLattices);

    bool transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) { assert(false); return false; }

    boost::shared_ptr<DFTransferVisitor>
    getTransferVisitor(AnalysisParts& parts, CFGNode cn, NodeState& state,
                       std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo);

    // functions called by composer
    MemLocObjectPtr Expr2MemLoc(SgNode* sgn, PartEdgePtr pedge);
    bool implementsExpr2MemLoc   () { return true; }
    implTightness Expr2MemLocTightness()    { return loose; }
    std::string str(std::string indent) const;

    friend class PointsToAnalysisTransfer;

    // helper function to copy elements from abstract object set
    // void copyAbstractObjectSet(const AbstractObjectSet& aos, std::list<MemLocObjectPtr>& list);

    // get the pointsToSet from the given map
    // boost::shared_ptr<AbstractObjectSet> getPointsToSet(SgNode* sgn, PartEdgePtr pedge, AbstractObjectMap *aom);

    // wrap the given set PointsToMLPtr
    // PointsToMLPtr Expr2PointsToMLPtr(SgNode* sgn, PartEdgePtr pedge, boost::shared_ptr<AbstractObjectSet> aom);
  };

  /******************
   * PTMemLocObject *
   ******************/

  //! MemLocObject exported by PointsToAnalysis.
  //! PTMemLocObject is a set of locations denoted by an expression.
  //! Locations are represented by MemLocObjects.
  //! AbstractObjectSet is used to store the set of MemLocObjects denoted by an expression.
  class PTMemLocObject : public MemLocObject {
    boost::shared_ptr<AbstractObjectSet> aos_p;
    PointsToAnalysis* ptanalysis;
  public:
    PTMemLocObject(PartEdgePtr pedge, Composer* composer, PointsToAnalysis* ptanalysis);
    PTMemLocObject(const PTMemLocObject& thatPTML);

    MemRegionObjectPtr getRegion() const;
    ValueObjectPtr     getIndex() const;

    // Allocates a copy of this object and returns a pointer to it
    MemLocObjectPtr copyAOType() const;

    void add(MemLocObjectPtr ml_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    void add(boost::shared_ptr<AbstractObjectSet> thataos_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    const AbstractObjectSet& getMLSet() const;
    boost::shared_ptr<AbstractObjectSet> getMLSetPtr() const;
    Lattice* getMLSetLatticePtr() const;
    virtual bool mayEqual(MemLocObjectPtr ml_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    virtual bool mustEqual(MemLocObjectPtr ml_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    virtual bool equalSet(MemLocObjectPtr ml_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    virtual bool subSet(MemLocObjectPtr ml_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    virtual bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    virtual bool meetUpdate(MemLocObjectPtr ml_p, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    virtual bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    virtual bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
    virtual MemLocObjectPtr copyML() const;
    virtual MemLocObject* copyMLPtr() const;
    virtual std::string str(std::string indent="") const;

    // Returns whether all instances of this class form a hierarchy. Every instance of the same
    // class created by the same analysis must return the same value from this method!
    virtual bool isHierarchy() const { return false; }
    // AbstractObjects that form a hierarchy must inherit from the AbstractionHierarchy class

    // Returns a key that uniquely identifies this particular AbstractObject in the
    // set hierarchy.
    virtual const hierKeyPtr& getHierKey() const { assert(0); }
  };

  typedef boost::shared_ptr<PTMemLocObject> PTMemLocObjectPtr;

  // used to handle Expr2MemLoc queries by the composer
  // for various SgNode
  // class Expr2MemLocTraversal : public ROSE_VisitorPatternDefaultBase
  // {
  //   Composer* composer;
  //   PointsToAnalysis* analysis;
  //   PartEdgePtr pedge;
  //   AbstractObjectMap* aom;
  //   // returned by this class for a given SgNode*
  //   boost::shared_ptr<AbstractObjectSet> p_aos;
  // public:
  //   Expr2MemLocTraversal(Composer* _composer,
  //                        PointsToAnalysis* _analysis,
  //                        PartEdgePtr _pedge,
  //                        AbstractObjectMap* _aom) :
  //   composer(_composer),
  //   analysis(_analysis),
  //   pedge(_pedge), aom(_aom),
  //   p_aos(boost::shared_ptr<AbstractObjectSet>()) { }
  //   void visit(SgPointerDerefExp* sgn);
  //   void visit(SgVarRefExp* sgn);
  //   void visit(SgAssignOp* sgn);
  //   boost::shared_ptr<AbstractObjectSet> getPointsToSet() { return p_aos; }
  // };
};

#endif
