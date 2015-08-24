#ifndef _POINTSTOANALYSIS_H
#define _POINTSTOANALYSIS_H

/*********************************
 * Sriram Aananthakrishnan, 2013 *
 *********************************/

/* A simple pointer analysis computing PointsTo relation
 */
#include "compose.h"
#include "abstract_object_map.h"
#include "abstract_object_set.h"

namespace fuse
{
  class PointsToAnalysis;

  /****************************
   * PointsToAnalysisTransfer *
   ****************************/

  typedef std::pair<MemLocObjectPtr, boost::shared_ptr<AbstractObjectSet> > PointsToRelation;

  //! Transfer functions for the PointsTo analysis
  class PointsToAnalysisTransfer : public DFTransferVisitor
  {
    Composer* composer;
    PointsToAnalysis* analysis;
    AbstractObjectMap* latticeMapIn;
    // used by the analysis to determine if the states modified or not
    bool modified;
    friend class PointerExprTransfer;
  public:
    PointsToAnalysisTransfer(PartPtr part, CFGNode cn, NodeState& state,
                             std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo,
                             Composer* composer, PointsToAnalysis* analysis);                             


    //! Visitor pattern to process sub-expressions of expressions involving pointer assignment.
    //! Determine the set of memory locations denoted by the sub-expression.
    class PointerExprTransfer : public ROSE_VisitorPatternDefaultBase {      
      boost::shared_ptr<AbstractObjectSet> latElem;  //!< set to insert the MemLocObjectPtr denoted by the sub-expression
      SgExpression* anchor;                          //!< anchor expression that the sub-expression is part of
      PartEdgePtr pedge;
      AbstractObjectMap* latticeMap;
      PointsToAnalysisTransfer& pointsToAnalysisTransfer;
      bool modified;
    public:
      PointerExprTransfer(boost::shared_ptr<AbstractObjectSet> latElem,
                          SgExpression* anchor,
                          PartEdgePtr pedge,
                          AbstractObjectMap* latticeMap,
                          PointsToAnalysisTransfer& pointsToAnalysisTransfer);
      void visit(SgAddressOfOp*);
      void visit(SgVarRefExp*);
      void visit(SgDotExp*);
      void visit(SgPointerDerefExp*);

      bool isLatElemModified() const;
      MemLocObjectPtr getExpr2MemLoc(SgExpression* sgn, PartEdgePtr pedge);
    };

    // Transfer functions
    void visit(SgAssignOp* sgn);
    void visit(SgPointerDerefExp* sgn);
    void visit(SgFunctionCallExp* sgn);

    // AbstractObjectMap access methods
    bool updateLatticeMap(AbstractObjectMap* latticeMap, PointsToRelation& prel);

    // Helper methods
    PointsToRelation make_pointsto(MemLocObjectPtr key, boost::shared_ptr<AbstractObjectSet> latticeElem);
    MemLocObjectPtr getLatticeMapKeyML(SgExpression* anchor, SgExpression* operand, PartEdgePtr pedge);
    typedef std::pair<SgExpression*, SgInitializedName*> ArgParamMapping;
    typedef std::list<ArgParamMapping> ArgParamMappingList;
    void getArgParamMapping(SgFunctionCallExp* sgnCallExp, SgFunctionParameterList* sgnFuncEntry, ArgParamMappingList& argParamMappingList);
    bool finish();
  };

  /********************
   * PointsToAnalysis *
   ********************/

  class PointsToAnalysis : public virtual FWDataflow
  {
  public:
    PointsToAnalysis() { }
    
    // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
    ComposedAnalysisPtr copy() { return boost::make_shared<PointsToAnalysis>(); }

    void genInitLattice(PartPtr part, PartEdgePtr pedge,
                        std::vector<Lattice*>& initLattices);

    bool transfer(PartPtr part, CFGNode cn, NodeState& state, 
                  std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) { assert(false); return false; }

    boost::shared_ptr<DFTransferVisitor> 
    getTransferVisitor(PartPtr part, CFGNode cn, NodeState& state, 
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
  public:
    PTMemLocObject(PartEdgePtr pedge, Composer* composer, PointsToAnalysis* ptanalysis);
    PTMemLocObject(const PTMemLocObject& thatPTML);

    void add(MemLocObjectPtr ml_p, PartEdgePtr pedge);
    void add(boost::shared_ptr<AbstractObjectSet> thataos_p, PartEdgePtr pedge);
    const AbstractObjectSet& getMLSet() const;
    boost::shared_ptr<AbstractObjectSet> getMLSetPtr() const;
    Lattice* getMLSetLatticePtr() const;
    virtual bool mayEqualML(MemLocObjectPtr ml_p, PartEdgePtr pedge);
    virtual bool mustEqualML(MemLocObjectPtr ml_p, PartEdgePtr pedge);
    virtual bool equalSetML(MemLocObjectPtr ml_p, PartEdgePtr pedge);
    virtual bool subSetML(MemLocObjectPtr ml_p, PartEdgePtr pedge);
    virtual bool isLiveML(PartEdgePtr pedge);
    virtual bool meetUpdateML(MemLocObjectPtr ml_p, PartEdgePtr pedge);
    virtual bool isFullML(PartEdgePtr pedge);
    virtual bool isEmptyML(PartEdgePtr pedge);
    virtual MemLocObjectPtr copyML() const;
    virtual MemLocObject* copyMLPtr() const;
    virtual std::string str(std::string indent="") const;
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
