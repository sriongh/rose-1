#ifndef _ORTHO_ARRAY_ANALYSIS_H
#define _ORTHO_ARRAY_ANALYSIS_H

#include "compose.h"

namespace fuse {
  class ArrayAnalysis;

  /**************
   * BaseMLType *
   **************/
  class BaseMLType;
  typedef boost::shared_ptr<BaseMLType> BaseMLTypePtr;
  class BaseMLType {
  public:
    typedef enum {empty, array, notarray, full} MType;
  protected:
    MType mtype;
  public:
    BaseMLType(MType mtype) : mtype(mtype) { }

    MType getMType() const { 
      return mtype; 
    }

    virtual BaseMLTypePtr copyMLType() const = 0;

    virtual bool mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) = 0;
    virtual bool mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) = 0;
    
    // Returns whether the two abstract objects denote the same set of concrete objects
    virtual bool equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) = 0;
    
    // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the given abstract object.
    virtual bool subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) = 0;   
    virtual bool isLiveMLType(PartEdgePtr pedge) = 0;
    
    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    virtual bool isFullMLType(PartEdgePtr pedge) = 0;
    // Returns whether this AbstractObject denotes the empty set.
    virtual bool isEmptyMLType(PartEdgePtr pedge) = 0;
    // pretty print
    virtual std::string str(std::string indent="") const =0;
  };

  /***************
   * EmptyMLType *
   ***************/
  class EmptyMLType : public BaseMLType {
  public:
    EmptyMLType();
    EmptyMLType(const EmptyMLType& that);

    BaseMLTypePtr copyMLType() const;

    bool mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge);
    bool mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge);
    
    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge);
    
    // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the given abstract object.
    bool subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge);   
    bool isLiveMLType(PartEdgePtr pedge);
    
    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    bool isFullMLType(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the empty set.
    bool isEmptyMLType(PartEdgePtr pedge);
    // pretty print
    std::string str(std::string indent="") const;
  };

  typedef boost::shared_ptr<EmptyMLType> EmptyMLTypePtr;

  /******************
   * NonArrayMLType *
   ******************/
  class NonArrayMLType;
  typedef boost::shared_ptr<NonArrayMLType> NonArrayMLTypePtr;
  class NonArrayMLType : public BaseMLType {
    MemLocObjectPtr ml;
  public:
    NonArrayMLType(MemLocObjectPtr ml);
    NonArrayMLType(const NonArrayMLType& that);

    MemLocObjectPtr getML() const;

    BaseMLTypePtr copyMLType() const;

    bool mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge);
    bool mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge);
    
    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge);
    
    // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the given abstract object.
    bool subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge);   
    bool isLiveMLType(PartEdgePtr pedge);
    
    // Computes the meet of this and that and saves the result in this
    // returns true if this causes this to change and false otherwise
    bool meetUpdateMLType(NonArrayMLTypePtr that, PartEdgePtr pedge);
    
    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    bool isFullMLType(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the empty set.
    bool isEmptyMLType(PartEdgePtr pedge);
    // pretty print
    std::string str(std::string indent="") const;
  };

  /********************
   * ArrayIndexVector *
   ********************/
  class ArrayIndexVector;
  typedef boost::shared_ptr<ArrayIndexVector> ArrayIndexVectorPtr;
  class ArrayIndexVector {
    std::list<ValueObjectPtr> indices;
  public:
    ArrayIndexVector(std::list<ValueObjectPtr> indices);
    ArrayIndexVector(const ArrayIndexVector& that);
    ArrayIndexVectorPtr copyV() const;

    std::list<ValueObjectPtr> getArrayIndexVector() const;
    bool mayEqualV(ArrayIndexVectorPtr that, PartEdgePtr pedge);
    bool mustEqualV(ArrayIndexVectorPtr that, PartEdgePtr pedge);
    bool equalSetV(ArrayIndexVectorPtr that, PartEdgePtr pedge);
    bool subSetV(ArrayIndexVectorPtr that, PartEdgePtr pedge);
    bool meetUpdateV(ArrayIndexVectorPtr that, PartEdgePtr pedge);
    std::string str(std::string indent="") const;
  };  

  /***************
   * ArrayMLType *
   ***************/
  // Memory object wrapping the information about array
  class ArrayMLType;
  typedef boost::shared_ptr<ArrayMLType> ArrayMLTypePtr;
  class ArrayMLType : public BaseMLType {
  protected:
    // The parent array reference
    SgNode *array_ref;
    // memory object for the top level array object
    MemRegionObjectPtr region;
    ArrayIndexVectorPtr indices;
    // This represents ML from the Composer
    MemLocObjectPtr origML;
    
  public:
    ArrayMLType(SgNode* sgn, MemRegionObjectPtr arrayML, ArrayIndexVectorPtr iv,
                MemLocObjectPtr origML);
    ArrayMLType(const ArrayMLType& that);

    MemRegionObjectPtr getArrayMR() const;
    ArrayIndexVectorPtr getArrayIndexVector() const; 
    MemLocObjectPtr getOrigML() const;

    BaseMLTypePtr copyMLType() const;
    bool mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge);
    bool mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge);   
    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge);    
    // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the given abstract object.
    bool subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge);   
    bool meetUpdateMLType(ArrayMLTypePtr that, PartEdgePtr pedge);
    bool isLiveMLType(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    bool isFullMLType(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the empty set.
    bool isEmptyMLType(PartEdgePtr pedge);
    // pretty print
    std::string str(std::string indent="") const;
  };
    
  /**************
   * FullMLType *
   **************/
  class FullMLType : public BaseMLType {
  public:
    FullMLType();
    FullMLType(const FullMLType& that);
    BaseMLTypePtr copyMLType() const;
    bool mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge);
    bool mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge);   
    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge);    
    // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the given abstract object.
    bool subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge);   
    bool isLiveMLType(PartEdgePtr pedge);    
    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    bool isFullMLType(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the empty set.
    bool isEmptyMLType(PartEdgePtr pedge);
    // pretty print
    std::string str(std::string indent="") const;

  };

  typedef boost::shared_ptr<FullMLType> FullMLTypePtr;
    
  /***********
   * ArrayML *
   ***********/
  class ArrayML : public MemLocObject {
    BaseMLTypePtr mltype;
  public:
    ArrayML(BaseMLTypePtr mltype);
    ArrayML(const ArrayML& that);

    BaseMLTypePtr getArrayMLType() const;

    MemLocObjectPtr copyML() const;
    bool mayEqualML(MemLocObjectPtr that, PartEdgePtr pedge);
    bool mustEqualML(MemLocObjectPtr that, PartEdgePtr pedge);
    
    // Returns whether the two abstract objects denote the same set of concrete objects
    bool equalSetML(MemLocObjectPtr that, PartEdgePtr pedge);
    
    // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
    // by the given abstract object.
    bool subSetML(MemLocObjectPtr that, PartEdgePtr pedge);
    
    bool isLiveML(PartEdgePtr pedge);
    
    // Computes the meet of this and that and saves the result in this
    // returns true if this causes this to change and false otherwise
    bool meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge);
    
    // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
    bool isFullML(PartEdgePtr pedge);
    // Returns whether this AbstractObject denotes the empty set.
    bool isEmptyML(PartEdgePtr pedge);
    // pretty print
    std::string str(std::string indent="") const;
  };        
  
  typedef boost::shared_ptr<ArrayML> ArrayMLPtr;

  /*****************
   * ArrayAnalysis *
   *****************/
  class ArrayAnalysis : public UndirDataflow {

    // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
    ComposedAnalysisPtr copy() { return boost::make_shared<ArrayAnalysis>(); }
    
    // The genInitLattice, genInitFact and transfer functions are not implemented since this 
    // is not a dataflow analysis.
   
    // Maps the given SgNode to an implementation of the MemLocObject abstraction.
    // Variant of Expr2Val where Part field is ignored since it makes no difference for the syntactic analysis.
    MemLocObjectPtr  Expr2MemLoc(SgNode* n, PartEdgePtr pedge);
    bool implementsExpr2MemLoc() { return true; }
  
    // pretty print for the object
    std::string str(std::string indent="") const
    { return "ArrayAnalysis"; }
  };
}; // end namespace
#endif
