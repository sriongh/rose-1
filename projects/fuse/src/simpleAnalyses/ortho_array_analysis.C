#include "sage3basic.h"
#include "ortho_array_analysis.h"
#include <boost/make_shared.hpp>

using namespace sight;
using namespace std;

namespace fuse {
#define arrayAnalysisDebugLevel 0
#if arrayAnalysisDebugLevel==0
#define DISABLE_SIGHT
#endif
  //  DEBUG_LEVEL(arrayAnalysisDebugLevel, 0);

  /***************
   * EmptyMLType *
   ***************/
  EmptyMLType::EmptyMLType() : BaseMLType(BaseMLType::empty) { }
  EmptyMLType::EmptyMLType(const EmptyMLType& that) : BaseMLType(that.getMType()) { }

  BaseMLTypePtr EmptyMLType::copyMLType() const { 
    return boost::make_shared<EmptyMLType>(*this);
  }

  bool EmptyMLType::mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) {    
    if(that->getMType() == BaseMLType::full) return true;
    return that->getMType() == getMType();
  }

  bool EmptyMLType::mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    return that->getMType() == getMType();
  }
    
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool EmptyMLType::equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    return that->getMType() == getMType();
  }
    
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool EmptyMLType::subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    return true;
  }

  bool EmptyMLType::isLiveMLType(PartEdgePtr pedge) { 
    return true;
  }
        
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool EmptyMLType::isFullMLType(PartEdgePtr pedge) { 
    return false;
  }
  // Returns whether this AbstractObject denotes the empty set.
  bool EmptyMLType::isEmptyMLType(PartEdgePtr pedge) {
    return true;
  }

  // pretty print
  std::string EmptyMLType::str(std::string indent) const { 
    return "empty";
  }

  /******************
   * NonArrayMLType *
   ******************/
  NonArrayMLType::NonArrayMLType(MemLocObjectPtr ml) : BaseMLType(BaseMLType::notarray), ml(ml) { }
  NonArrayMLType::NonArrayMLType(const NonArrayMLType& that) : BaseMLType(that.getMType()), ml(that.ml) { }

  BaseMLTypePtr NonArrayMLType::copyMLType() const { 
    return boost::make_shared<NonArrayMLType>(*this);
  }

  MemLocObjectPtr NonArrayMLType::getML() const {
    return ml;
  }

  bool NonArrayMLType::mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) {
    if(that->getMType() == BaseMLType::full) return true;
    else if(that->getMType() == BaseMLType::empty) return false;
    else if(that->getMType() == BaseMLType::array) {
      return false;
      // NonArrayML could be a storage
      // Ideally we should compare if the MemRegionObject are overlapping
      // MemLocObject::getRegion() is not  defined on UnionMemLocObject
      // workaround is use the ArrayMLType and get the root SgNode
      ArrayMLTypePtr thatArrMT = boost::dynamic_pointer_cast<ArrayMLType>(that);
      // Use the ML from the composer
      MemLocObjectPtr thatArrML = thatArrMT->getOrigML();
      return ml->mayEqualML(thatArrML , pedge);
    }
    else {
      NonArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<NonArrayMLType> (that);
      return ml->mayEqualML(thatMT->getML(), pedge);
    }
  }

  bool NonArrayMLType::mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) {
    if(that->getMType() == BaseMLType::full ||
       that->getMType() == BaseMLType::empty ||
       that->getMType() == BaseMLType::array) return false; 
    else {
      NonArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<NonArrayMLType>(that);
      return ml->mustEqualML(thatMT->getML(), pedge);
    }
  }
    
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool NonArrayMLType::equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    if(that->getMType() == BaseMLType::full ||
       that->getMType() == BaseMLType::empty) return false;
    else if(that->getMType() == BaseMLType::array) {
      return false;
      // NonArrayML could be a storage
      // Ideally we should compare if the MemRegionObject are overlapping
      // MemLocObject::getRegion() is not  defined on UnionMemLocObject
      // workaround is use the ArrayMLType and get the root SgNode
      ArrayMLTypePtr thatArrMT = boost::dynamic_pointer_cast<ArrayMLType>(that);
      // Use the ML from the composer
      MemLocObjectPtr thatArrML = thatArrMT->getOrigML();
      return ml->equalSetML(thatArrML , pedge);
    }
    else {
      NonArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<NonArrayMLType>(that);
      return ml->equalSetML(thatMT->getML(), pedge);
    }
  }
    
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool NonArrayMLType::subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) {
    if(that->getMType() == BaseMLType::full) return true;
    else if(that->getMType() == BaseMLType::empty) return false;
    else if(that->getMType() == BaseMLType::array){
      return false;
      // NonArrayML could be a storage
      // Ideally we should compare if the MemRegionObject are overlapping
      // MemLocObject::getRegion() is not  defined on UnionMemLocObject
      // workaround is use the ArrayMLType and get the root SgNode
      ArrayMLTypePtr thatArrMT = boost::dynamic_pointer_cast<ArrayMLType>(that);
      // Use the ML from the composer
      MemLocObjectPtr thatArrML = thatArrMT->getOrigML();
      return ml->subSetML(thatArrML , pedge);
    }
    else {
      NonArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<NonArrayMLType>(that);
      return ml->subSetML(thatMT->getML(), pedge);
    }
  }

  bool NonArrayMLType::meetUpdateMLType(NonArrayMLTypePtr that, PartEdgePtr pedge) {
    return ml->meetUpdateML(that->getML(), pedge);
  }

  bool NonArrayMLType::isLiveMLType(PartEdgePtr pedge) {
    return ml->isLiveML(pedge);
  }
  
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool NonArrayMLType::isFullMLType(PartEdgePtr pedge) { 
    return ml->isFullML(pedge);
  }
  // Returns whether this AbstractObject denotes the empty set.
  bool NonArrayMLType::isEmptyMLType(PartEdgePtr pedge) { 
    return ml->isEmptyML(pedge);
  }

  // pretty print
  std::string NonArrayMLType::str(std::string indent) const { 
    ostringstream oss;
    oss << "notarray, " << ml->str();
    return oss.str();
  }

  /********************
   * ArrayIndexVector *
   ********************/
  ArrayIndexVector::ArrayIndexVector(std::list<ValueObjectPtr> indices) : indices(indices) { }
  ArrayIndexVector::ArrayIndexVector(const ArrayIndexVector& that) { 
    indices.clear();
    list<ValueObjectPtr> thatIV = that.getArrayIndexVector();
    list<ValueObjectPtr>::const_iterator it = thatIV.begin();
    for( ; it != thatIV.end(); ++it) {
      ValueObjectPtr v = (*it)->copyV();
      indices.push_back(v);
    }
  }

  ArrayIndexVectorPtr ArrayIndexVector::copyV() const {
    return boost::make_shared<ArrayIndexVector>(*this);
  }

  std::list<ValueObjectPtr> ArrayIndexVector::getArrayIndexVector() const {
    return indices;
  }

  bool ArrayIndexVector::mayEqualV(ArrayIndexVectorPtr that, PartEdgePtr pedge) {
    list<ValueObjectPtr> thatIV = that->getArrayIndexVector();
    // If the indices are not of equal length
    if(indices.size() != thatIV.size()) return false;
    list<ValueObjectPtr>::iterator thisIt, thatIt;
    thisIt = indices.begin(), thatIt = thatIV.begin();
    for( ; thisIt != indices.end() && thatIt != indices.end(); 
         ++thisIt, ++thatIt) {
      ValueObjectPtr thisV = *thisIt;
      ValueObjectPtr thatV = *thatIt;
      if(thisV->mayEqualV(thatV, pedge)) return true;
    }
    return false;
  }

  bool ArrayIndexVector::mustEqualV(ArrayIndexVectorPtr that, PartEdgePtr pedge) { 
    list<ValueObjectPtr> thatIV = that->getArrayIndexVector();
    // If the indices are not of equal length
    if(indices.size() != thatIV.size()) return false;
    list<ValueObjectPtr>::iterator thisIt, thatIt;
    thisIt = indices.begin(), thatIt = thatIV.begin();
    for( ; thisIt != indices.end() && thatIt != indices.end(); 
         ++thisIt, ++thatIt) {
      ValueObjectPtr thisV = *thisIt;
      ValueObjectPtr thatV = *thatIt;
      if(!thisV->mustEqualV(thatV, pedge)) return false;
    }
    return true;
  }

  bool ArrayIndexVector::equalSetV(ArrayIndexVectorPtr that, PartEdgePtr pedge) { 
    list<ValueObjectPtr> thatIV = that->getArrayIndexVector();
    // If the indices are not of equal length
    if(indices.size() != thatIV.size()) return false;
    list<ValueObjectPtr>::iterator thisIt, thatIt;
    thisIt = indices.begin(), thatIt = thatIV.begin();
    for( ; thisIt != indices.end() && thatIt != indices.end(); 
         ++thisIt, ++thatIt) {
      ValueObjectPtr thisV = *thisIt;
      ValueObjectPtr thatV = *thatIt;
      if(!thisV->equalSetV(thatV, pedge)) return false;
    }
    return true;
  }

  bool ArrayIndexVector::subSetV(ArrayIndexVectorPtr that, PartEdgePtr pedge) { 
    list<ValueObjectPtr> thatIV = that->getArrayIndexVector();
    // If the indices are not of equal length
    if(indices.size() != thatIV.size()) return false;
    list<ValueObjectPtr>::iterator thisIt, thatIt;
    thisIt = indices.begin(), thatIt = thatIV.begin();
    for( ; thisIt != indices.end() && thatIt != indices.end(); 
         ++thisIt, ++thatIt) {
      ValueObjectPtr thisV = *thisIt;
      ValueObjectPtr thatV = *thatIt;
      if(thisV->subSetV(thatV, pedge)) return true;
    }
    return false;
  }
 
  bool ArrayIndexVector::meetUpdateV(ArrayIndexVectorPtr that, PartEdgePtr pedge) {
    list<ValueObjectPtr> thatIV = that->getArrayIndexVector();
    bool flag = false;
    list<ValueObjectPtr>::iterator thisIt, thatIt;
    thisIt = indices.begin(), thatIt = thatIV.begin();
    for( ; thisIt != indices.end() || thatIt != indices.end(); 
         ++thisIt, ++thatIt) {
      ValueObjectPtr thisV = *thisIt;
      ValueObjectPtr thatV = *thatIt;
      flag = thisV->meetUpdateV(thatV, pedge) || flag;
    }
    return flag;
  }

  std::string ArrayIndexVector::str(std::string indent) const { 
    ostringstream oss;
    list<ValueObjectPtr>::const_iterator it = indices.begin();
    for(int i = 0; it != indices.end(); ++i) {
      oss << "["<<i<<"]=" << (*it)->str();
      ++it;
      if( it != indices.end()) oss << ", ";
    }
    return oss.str();
  }


  /***************
   * ArrayMLType *
   ***************/
  ArrayMLType::ArrayMLType(SgNode* sgn, MemRegionObjectPtr region, 
                           ArrayIndexVectorPtr indices, MemLocObjectPtr origML) 
    : BaseMLType(BaseMLType::array),
      array_ref(sgn), region(region), indices(indices), origML(origML) { }

  ArrayMLType::ArrayMLType(const ArrayMLType& that) 
  : BaseMLType(that.getMType()),
    array_ref(that.array_ref) {
    region = that.region->copyMR();
    indices = that.indices->copyV();
    origML = that.origML->copyML();
  }

  MemRegionObjectPtr ArrayMLType::getArrayMR() const {
    return region;
  }

  ArrayIndexVectorPtr ArrayMLType::getArrayIndexVector() const {
    return indices;
  }

  MemLocObjectPtr ArrayMLType::getOrigML() const {
    return origML;
  }

  BaseMLTypePtr ArrayMLType::copyMLType() const { 
    return boost::make_shared<ArrayMLType>(*this);
  }

  bool ArrayMLType::mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    if(that->getMType() == BaseMLType::full) return true;
    else if(that->getMType() == BaseMLType::empty) return false;
    else if(that->getMType() == BaseMLType::notarray) {
      return false;
      NonArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<NonArrayMLType>(that);
      return origML->mayEqualML(thatMT->getML(), pedge);
    }
    else {
      ArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<ArrayMLType>(that);
      MemRegionObjectPtr thatMR = thatMT->getArrayMR();
      if(!region->mayEqualMR(thatMR, pedge)) return false;
      bool mayeq = false;
      if(region->mayEqualMR(thatMR, pedge)) {
        mayeq = indices->mayEqualV(thatMT->getArrayIndexVector(), pedge);
      }
      return mayeq;
    }
  }

  bool ArrayMLType::mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    if(that->getMType() == BaseMLType::full || 
       that->getMType() == BaseMLType::empty ||
       that->getMType() == BaseMLType::notarray) return false;
    else {
      ArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<ArrayMLType>(that);
      MemRegionObjectPtr thatMR = thatMT->getArrayMR();
      ArrayIndexVectorPtr thatINDICES = thatMT->getArrayIndexVector();
      return region->mustEqualMR(thatMR, pedge) && indices->mustEqualV(thatINDICES, pedge);
    }
  }  
 
  bool ArrayMLType::equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    if(that->getMType() == BaseMLType::full || 
       that->getMType() == BaseMLType::empty) return false;
    else if(that->getMType() == BaseMLType::notarray) {
      return false;
      NonArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<NonArrayMLType>(that);
      return origML->equalSetML(thatMT->getML(), pedge);
    }
    else {
      ArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<ArrayMLType>(that);
      MemRegionObjectPtr thatMR = thatMT->getArrayMR();
      ArrayIndexVectorPtr thatINDICES = thatMT->getArrayIndexVector();
      return region->equalSetMR(thatMR, pedge) && indices->equalSetV(thatINDICES, pedge);
    }
  }    

  bool ArrayMLType::subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    if(that->getMType() == BaseMLType::full) return true;
    else if(that->getMType() == BaseMLType::empty) return false;
    else if(that->getMType() == BaseMLType::notarray) {
      return false;
      NonArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<NonArrayMLType>(that);
      return origML->equalSetML(thatMT->getML(), pedge);
    }
    else {
      ArrayMLTypePtr thatMT = boost::dynamic_pointer_cast<ArrayMLType>(that);
      MemRegionObjectPtr thatMR = thatMT->getArrayMR();
      ArrayIndexVectorPtr thatINDICES = thatMT->getArrayIndexVector();
      return region->subSetMR(thatMR, pedge) && indices->subSetV(thatINDICES, pedge);
    }                                                   
  }   

  bool ArrayMLType::meetUpdateMLType(ArrayMLTypePtr that, PartEdgePtr pedge) {
    bool meet = false;
    meet = region->meetUpdateMR(that->getArrayMR(), pedge);
    meet = indices->meetUpdateV(that->getArrayIndexVector(), pedge) || meet;
    return meet;
  }

  bool ArrayMLType::isLiveMLType(PartEdgePtr pedge) { 
    return region->isLiveMR(pedge);
  }

  bool ArrayMLType::isFullMLType(PartEdgePtr pedge) { 
    return region->isFullMR(pedge);
  }
  
  bool ArrayMLType::isEmptyMLType(PartEdgePtr pedge) { 
    return region->isEmptyMR(pedge);
  }

  // pretty print
  std::string ArrayMLType::str(std::string indent) const { 
    ostringstream oss;
    oss << "arr, region=" << region->str() << "iv=" << indices->str();
    return oss.str();
  }

  /*******************
   * FullArrayMLType *
   *******************/
  FullMLType::FullMLType() : BaseMLType(BaseMLType::full) { }
  FullMLType::FullMLType(const FullMLType& that) : BaseMLType(that.getMType()) { }

  BaseMLTypePtr FullMLType::copyMLType() const { 
    return boost::make_shared<FullMLType>(*this);
  }

  bool FullMLType::mayEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) {    
    return true;
  }

  bool FullMLType::mustEqualMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    return false;
  }
    
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool FullMLType::equalSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    return that->getMType() == getMType();
  }
    
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool FullMLType::subSetMLType(BaseMLTypePtr that, PartEdgePtr pedge) { 
    return that->getMType() == getMType();
  }

  bool FullMLType::isLiveMLType(PartEdgePtr pedge) { 
    return true;
  }
        
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool FullMLType::isFullMLType(PartEdgePtr pedge) { 
    return true;
  }
  // Returns whether this AbstractObject denotes the empty set.
  bool FullMLType::isEmptyMLType(PartEdgePtr pedge) {
    return false;
  }

  // pretty print
  std::string FullMLType::str(std::string indent) const { 
    return "full";
  }

  /***********
   * ArrayML *
   ***********/
  ArrayML::ArrayML(BaseMLTypePtr mltype) : MemLocObject(NULL), mltype(mltype) { }
  ArrayML::ArrayML(const ArrayML& that) : MemLocObject(that) {
    mltype = that.mltype->copyMLType();
  }

  BaseMLTypePtr ArrayML::getArrayMLType() const {
    return mltype;
  }

  MemLocObjectPtr ArrayML::copyML() const { 
    return boost::make_shared<ArrayML>(*this);
  }

  bool ArrayML::mayEqualML(MemLocObjectPtr that, PartEdgePtr pedge) { 
    ArrayMLPtr thatML = boost::dynamic_pointer_cast<ArrayML>(that); assert(thatML);
    return mltype->mayEqualMLType(thatML->getArrayMLType(), pedge);
  }

  bool ArrayML::mustEqualML(MemLocObjectPtr that, PartEdgePtr pedge) { 
    ArrayMLPtr thatML = boost::dynamic_pointer_cast<ArrayML>(that); assert(thatML);
    return mltype->mustEqualMLType(thatML->getArrayMLType(), pedge);
  }
    
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool ArrayML::equalSetML(MemLocObjectPtr that, PartEdgePtr pedge) { 
    ArrayMLPtr thatML = boost::dynamic_pointer_cast<ArrayML>(that); assert(thatML);
    return mltype->equalSetMLType(thatML->getArrayMLType(), pedge);
  }
    
  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool ArrayML::subSetML(MemLocObjectPtr that, PartEdgePtr pedge) { 
    ArrayMLPtr thatML = boost::dynamic_pointer_cast<ArrayML>(that); assert(thatML);
    return mltype->subSetMLType(thatML->getArrayMLType(), pedge);
  }
    
  bool ArrayML::isLiveML(PartEdgePtr pedge) { 
    return mltype->isLiveMLType(pedge);
  }
    
  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool ArrayML::meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge) { 
    ArrayMLPtr thatML = boost::dynamic_pointer_cast<ArrayML>(that); assert(thatML);
    BaseMLTypePtr thatMT = thatML->getArrayMLType();

    if(thatMT->getMType() == BaseMLType::full) {
      mltype = boost::make_shared<FullMLType>();
      return true;
    }
    // If that is empty nothing to change
    else if(thatMT->BaseMLType::empty) return false;
    // If both are non array type
    else if(thatMT->getMType() == BaseMLType::array &&
            mltype->getMType() == BaseMLType::array) {
      ArrayMLTypePtr thisArrMT = boost::dynamic_pointer_cast<ArrayMLType>(mltype);
      ArrayMLTypePtr thatArrMT = boost::dynamic_pointer_cast<ArrayMLType>(thatMT);
      return thisArrMT->meetUpdateMLType(thatArrMT, pedge);      
    }
    else if(thatMT->getMType() == BaseMLType::notarray &&
            mltype->getMType() == BaseMLType::notarray) {
      NonArrayMLTypePtr thisNAMT = boost::dynamic_pointer_cast<NonArrayMLType>(mltype);
      NonArrayMLTypePtr thatNAMT = boost::dynamic_pointer_cast<NonArrayMLType>(thatMT);
      return thisNAMT->meetUpdateMLType(thatNAMT, pedge);
    }
    else {
      mltype = boost::make_shared<FullMLType>();
      return true;
    }
  }
    
  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool ArrayML::isFullML(PartEdgePtr pedge) { 
    return mltype->isFullMLType(pedge);
  }
  // Returns whether this AbstractObject denotes the empty set.
  bool ArrayML::isEmptyML(PartEdgePtr pedge) { 
    return mltype->isEmptyMLType(pedge);
  }
  // pretty print
  std::string ArrayML::str(std::string indent) const {
    ostringstream oss;
    oss << "[ArrayML: " << mltype->str() << "]";
    return oss.str();
  }

  /*****************
   * ArrayAnalysis *
   *****************/  
  MemLocObjectPtr  ArrayAnalysis::Expr2MemLoc(SgNode* n, PartEdgePtr pedge) {
    SIGHT_VERB_DECL(scope, (sight::txt() << "ArrayAnalysis::Expr2MemLoc(n="<<SgNode2Str(n) 
                            << ", pedge="<<pedge->str()<<")", scope::medium), 
                    2, arrayAnalysisDebugLevel)
    MemLocObjectPtr ml;
    // If this is a top level array reference for which we need to create ML
    if(isSgPntrArrRefExp(n) && 
       (!isSgPntrArrRefExp (n->get_parent()) || 
        !isSgPntrArrRefExp (isSgPntrArrRefExp (n->get_parent())->get_lhs_operand()))) {
      MemLocObjectPtr origML = getComposer()->Expr2MemLoc(n, pedge, this);

      // Use SageInterface to get the symbol and its subscripts
      SgExpression* arrayNameExp=0;
      std::vector<SgExpression*>* subscripts = new std::vector<SgExpression*>();
      SageInterface::isArrayReference(isSgExpression(n), &arrayNameExp, &subscripts);
      SIGHT_VERB_IF(2, arrayAnalysisDebugLevel)
        dbg << "arrayNameExpr=" << SgNode2Str(arrayNameExp) << endl;
        std::vector<SgExpression*>::iterator it = subscripts->begin();
        for(int i=0 ; it != subscripts->end(); ++it, ++i) {
          dbg << "[" << i << "]=>" << SgNode2Str(*it) << endl;
        }
      SIGHT_VERB_FI()

      // arrayNameExp is the array symbol
      // subscripts is the index expressions
      MemRegionObjectPtr region = getComposer()->Expr2MemRegion(arrayNameExp, pedge, this);
      list<ValueObjectPtr> subscriptsV;
      assert(subscripts->size() > 0);
      std::vector<SgExpression*>::iterator it = subscripts->begin();
      for( ; it != subscripts->end(); ++it) {
        SgExpression* index = *it;
        ValueObjectPtr v = getComposer()->OperandExpr2Val(n, index, pedge, this);        
        SIGHT_VERB(dbg << "index=" << SgNode2Str(index) << endl, 2, arrayAnalysisDebugLevel)
        SIGHT_VERB(dbg << "indexV=" << v->str() << endl, 2, arrayAnalysisDebugLevel)        
        subscriptsV.push_back(v->copyV());
      }
      ArrayIndexVectorPtr indices = boost::make_shared<ArrayIndexVector>(subscriptsV);
      ArrayMLTypePtr mltype = boost::make_shared<ArrayMLType>(n, region, indices, origML);
      MemLocObjectPtr arrayML = boost::make_shared<ArrayML>(mltype);
      SIGHT_VERB(dbg << arrayML->str() << endl, 2, arrayAnalysisDebugLevel)
      return arrayML;
    }
    else {
      MemLocObjectPtr nonArrML = getComposer()->Expr2MemLoc(n, pedge, this);
      NonArrayMLTypePtr mltype = boost::make_shared<NonArrayMLType>(nonArrML);
      return boost::make_shared<ArrayML>(mltype);
    }  
    assert(false);
  }

}; // end namespace
