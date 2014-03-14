#include "sage3basic.h"
#include "abstract_object.h"
#include "stx_analysis.h"
#include <iostream>

using namespace std;


namespace fuse
{

// Sriram: gcc 4.1.2 complains of undefined references to unused to template functions
// fix: explicit template instantiation
template class CombinedCodeLocObject<true>;
template class CombinedCodeLocObject<false>;
template class CombinedValueObject<true>;
template class CombinedValueObject<false>;
template class CombinedMemLocObject<true>;
template class CombinedMemLocObject<false>;
template class CombinedMemRegionObject<true>;
template class CombinedMemRegionObject<false>;

// Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
// classes to save the cost of a dynamic cast.
bool AbstractObject::isValueObject()     { return dynamic_cast<ValueObject*>    (this); }
bool AbstractObject::isCodeLocObject()   { return dynamic_cast<CodeLocObject*>  (this); }
bool AbstractObject::isMemRegionObject() { return dynamic_cast<MemRegionObject*>(this); }
bool AbstractObject::isMemLocObject()    { return dynamic_cast<MemLocObject*>   (this); }

/*
// Simple equality test that just checks whether the two objects correspond to the same expression
bool AbstractObject::mustEqualExpr(AbstractObjectPtr o, PartEdgePtr pedge)
{
  // GREG: I'm not sure if this is actually valid since the same expression can denote different sets at different loop iterations
  
  // If both AbstractObjects have non-NULL bases, we can tell that they're must-equal by simply confirming
  // that their bases are equal
  if(base && o->base && base==o->base) {
    dbg << "AbstractObject::mustEqualExpr() base="<<SgNode2Str(base)<<" o->base="<<SgNode2Str(o->base)<<endl;
    return true;
  // Otherwise, we don't know and must answer conservatively
  } else return false;
}*/

// General versions of equalSet() that accounts for framework details before routing the call to the 
// derived class' equalSet() check. Specifically, it routes the call through the composer to make 
// sure the equalSet() call gets the right PartEdge.
/*bool AbstractObject::equalSet(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  / * GB 2013-09-16 - No longer checking for equality to FuncResultMemLocObject since the FuncResultMemRegionObject
   *                 checks for this correctly on its own. We may return this code if we discover that checking this
   *                 early before we propagate the query through the composer is an important performance gain.
   * // If either this or that are FuncResultMemLocObject, they denote the same set if they both are
  if(isMemLocObject() && that->isMemLocObject()) {
    MemLocObject*   thisML = dynamic_cast<MemLocObject*>(this);
    MemLocObjectPtr thatML = boost::dynamic_pointer_cast<MemLocObject*>(that);

    FuncResultMemRegionObject* frmlcoThis  = dynamic_cast<FuncResultMemLocObject*>(thisML->getRegion());
    FuncResultMemRegionObjectPtr frmlcoThat = boost::dynamic_pointer_cast<FuncResultMemLocObject>(thatML->getRegion());
    if(frmlcoThis) return frmlcoThat;
    else if(frmlcoThat) return false;
    
  } else if(isMemRegionObject() && that->isMemRegionObject()) {
    MemRegionObject*   thisMR = dynamic_cast<MemRegionObject*>(this);
    MemRegionObjectPtr thatMR = boost::dynamic_pointer_cast<MemRegionObject*>(that);
    
    // If either this or that are FuncResultMemLocObject, they denote the same set if they both are
    FuncResultMemRegionObject* frmlcoThis  = dynamic_cast<FuncResultMemLocObject*>(thisMR);
    FuncResultMemRegionObjectPtr frmlcoThat = boost::dynamic_pointer_cast<FuncResultMemLocObject>(thatMR);
    if(frmlcoThis) return frmlcoThat;
    else if(frmlcoThat) return false;
  }* /
  
  return comp->equalSet(shared_from_this(), that, pedge, analysis);
}*/

// General versions of equalSet() that accounts for framework details before routing the call to the 
// derived class' subSet() check. Specifically, it routes the call through the composer to make 
// sure the subSet() call gets the right PartEdge.
/*bool AbstractObject::subSet(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  / * GB 2013-09-16 - No longer checking for equality to FuncResultMemLocObject since the FuncResultMemRegionObject
   *                 checks for this correctly on its own. We may return this code if we discover that checking this
   *                 early before we propagate the query through the composer is an important performance gain.
  if(isMemLocObject() && that->isMemLocObject()) {
    // If either this or that are FuncResultMemLocObject, they denote the same set if they both are
    FuncResultMemLocObject* frmlcoThis  = dynamic_cast<FuncResultMemLocObject*>(this);
    FuncResultMemLocObjectPtr frmlcoThat = boost::dynamic_pointer_cast<FuncResultMemLocObject>(that);
    if(frmlcoThis) return frmlcoThat;
    else if(frmlcoThat) return false;
  }* /
  return comp->subSet(shared_from_this(), that, pedge, analysis);
}*/

/*// General versions of isFull() and isEmpty that account for framework details before routing the call to the 
// derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make 
// sure the isFull(PartEdgePtr) and isEmpty(PartEdgePtr) call gets the right PartEdge.
bool AbstractObject::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->isFull(shared_from_this(), pedge, analysis); }

bool AbstractObject::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->isEmpty(shared_from_this(), pedge, analysis); }*/

/* #########################
   ##### CodeLocObject ##### 
   ######################### */

CodeLocObjectPtr NULLCodeLocObject;

// Returns whether this object may/must be equal to o within the given Part p
// These methods are called by composers and should not be called by analyses.
bool CodeLocObject::mayEqualCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  // If either object denotes the set of all parts, they're may-equal
  if(part==NULLPart || that->part==NULLPart)
    return true;
  
  // If the two objects denote different parts, they're not may-equal
  if(part!=that->part)
    return false;
  
  // If either object denotes the set of all CFGNodes within the same part, they're may-equal
  if(cfgNode.getNode()==NULL || that->cfgNode.getNode()==NULL)
    return true;
  
  // If the two objects denote different CFGNodes within the same part, they're not equal
  if(cfgNode!=that->cfgNode)
    return false;
  
  // The two objects denote the same part and CFGNode within it
  return true;
}

bool CodeLocObject::mustEqualCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  // The two objects are must-equal if they denote the same concrete part and CFGNode within it
  return part!=NULLPart          && that->part!=NULLPart          && part    == that->part   &&
         cfgNode.getNode()!=NULL && that->cfgNode.getNode()!=NULL && cfgNode == that->cfgNode;
}

// General version of mayEqual and mustEqual that implements may/must equality with respect to ExprObj
// and uses the derived class' may/mustEqual check for all the other cases
// GREG: Currently nothing interesting here since we don't support ExprObjs for CodeLocObjects
bool CodeLocObject::mayEqual(CodeLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return mayEqualCL(that, pedge); }

bool CodeLocObject::mustEqual(CodeLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return mustEqualCL(that, pedge); }

bool CodeLocObject::mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  CodeLocObjectPtr co = boost::dynamic_pointer_cast<CodeLocObject>(o);
  if(co) return mayEqual(co, pedge, comp, analysis);
  else   return false;
}

bool CodeLocObject::mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  //if(AbstractObject::mustEqualExpr(o, pedge)) return true;

  CodeLocObjectPtr co = boost::dynamic_pointer_cast<CodeLocObject>(o);
  if(co) return mustEqual(co, pedge, comp, analysis);
  else   return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
// These methods are called by composers and should not be called by analyses.
bool CodeLocObject::equalSetCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  return mayEqualCL(that, pedge);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
// These methods are called by composers and should not be called by analyses.
bool CodeLocObject::subSetCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  // If that object denotes the set of all parts, this is a subset of that
  if(that->part==NULLPart)
    return true;
  
  // If this object denotes the set of all parts, but that does not, this is not a subset
  if(part==NULLPart)
    return false;
  
  // If the two objects denote different parts, this is not a subset
  if(part!=that->part)
    return false;
  
  // If that object denotes the set of all CFGNodes within the same part, this is a subset
  if(that->cfgNode.getNode()==NULL)
    return true;
  
  // If this object denotes the set of all CFGNodes within the same part but that does not, this is not a subset
  if(cfgNode.getNode()==NULL)
    return false;
  
  // If the two objects denote different CFGNodes within the same part, this is not a subset
  if(cfgNode!=that->cfgNode)
    return false;
  
  // The two objects denote the same part and CFGNode within it
  return true;
}

bool CodeLocObject::equalSet(CodeLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return equalSetCL(that, pedge); }

bool CodeLocObject::subSet(CodeLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return subSetCL(that, pedge); }


bool CodeLocObject::equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  CodeLocObjectPtr co = boost::dynamic_pointer_cast<CodeLocObject>(o);
  if(co) return equalSet(co, pedge, comp, analysis);
  else   return false;
}

bool CodeLocObject::subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  CodeLocObjectPtr co = boost::dynamic_pointer_cast<CodeLocObject>(o);
  if(co) return subSet(co, pedge, comp, analysis);
  else   return false;
}

// Returns true if this object is live at the given part and false otherwise.
// This method is called by composers and should not be called by analyses.
bool CodeLocObject::isLiveCL(PartEdgePtr pedge) {
  // Code Locations are live by definition
  return true;
}

// General version of isLive that accounts for framework details before routing the call to the derived class' 
// isLiveCL check. Specifically, it routes the call through the composer to make sure the isLiveCL call gets the 
// right PartEdge
bool CodeLocObject::isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return isLiveCL(pedge); }

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool CodeLocObject::meetUpdateCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  // If the two objects denote different parts, set the meet to denote the set of all parts and all CFGNodes
  if(part != that->part) {
    part = NULLPart;
    cfgNode = CFGNode();
    return true;
  }
  
  // If the two objects denote the same part but different CFGNodes within it, set the meet to denote
  // the set of all CFGNodes within this part
  if(cfgNode != that->cfgNode) {
    cfgNode = CFGNode();
    return true;
  }
  
  // The two objects are identical
  return true;
}

// General version of meetUpdate that accounts for framework details before routing the call to the derived class' 
// meetUpdateCL check. Specifically, it routes the call through the composer to make sure the meetUpdateCL 
// call gets the right PartEdge
bool CodeLocObject::meetUpdate(CodeLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return meetUpdateCL(that, pedge); }

bool CodeLocObject::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  CodeLocObjectPtr cl = boost::dynamic_pointer_cast<CodeLocObject>(that);
  assert(cl);
  return meetUpdate(cl, pedge, comp, analysis);
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool CodeLocObject::isFullCL(PartEdgePtr pedge) {
 return part==NULLPart && cfgNode.getNode()==NULL;
}

// Returns whether this AbstractObject denotes the empty set.
bool CodeLocObject::isEmptyCL(PartEdgePtr pedge) {
  // It is not possible to create an empty CodeLocObject
  return false;
}

// General versions of isFull() and isEmpty that account for framework details before routing the call to the 
// derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make 
// sure the isFull(PartEdgePtr) and isEmpty(PartEdgePtr) call gets the right PartEdge.
// These functions are just aliases for the real implementations in AbstractObject
bool CodeLocObject::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return isFullCL(pedge); }

bool CodeLocObject::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return isEmptyCL(pedge); }

// Allocates a copy of this object and returns a shared pointer to it
CodeLocObjectPtr CodeLocObject::copyCL() const
{ return boost::make_shared<CodeLocObject>(*this); }

// Allocates a copy of this object and returns a regular pointer to it
CodeLocObject* CodeLocObject::copyCLPtr() const
{ return new CodeLocObject(*this); }

std::string CodeLocObject::str(std::string indent) { // pretty print for the object
  ostringstream oss; 
  oss << "[CodeLocObject: part="<<(part? part->str(indent+"    "): "ANY")<<", "<<endl;
  oss << indent << "                cfgNode="<<(cfgNode.getNode()? CFGNode2Str(cfgNode): "ANY")<<"]";
  return oss.str();
}

/* #################################
   ##### CombinedCodeLocObject ##### 
   ################################# */

/*template <bool defaultMayEq>
CombinedCodeLocObject<defaultMayEq>::CombinedCodeLocObject(CodeLocObjectPtr codeLoc): CodeLocObject(NULL) {
  codeLocs.push_back(codeLoc);
}

template <bool defaultMayEq>
CombinedCodeLocObject<defaultMayEq>::CombinedCodeLocObject(const list<CodeLocObjectPtr>& codeLocs) : CodeLocObject(NULL), codeLocs(codeLocs) {}

template <bool defaultMayEq>
void CombinedCodeLocObject<defaultMayEq>::add(CodeLocObjectPtr codeLoc) {
  codeLocs.push_back(codeLoc);
}*/

// Returns whether this object may/must be equal to o within the given Part p
// These methods are private to prevent analyses from calling them directly.
template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::mayEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
  // If the two combination objects include different numbers of CodeLocObjects, say that they may be equal since 
  // we can't be sure either way.
  if(codeLocs.size() != that->codeLocs.size()) return true;
  
  // Compare all the pairs of CodeLocObjects in codeLocs and that.codeLocs, returning defaultMayEq if any pair
  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true) 
  // answer that any CodeLocObject in codeLocs can give
  for(list<CodeLocObjectPtr>::iterator thisIt=codeLocs.begin(), thatIt=that->codeLocs.begin();
      thisIt!=codeLocs.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->mayEqualCL(*thatIt, pedge) == defaultMayEq) return defaultMayEq;
  }
  
  return !defaultMayEq;
}

template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::mustEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
  // If the two combination  objects include different numbers of CodeLocObjects, say that they are not must equal since 
  // we can't be sure either way.
  if(codeLocs.size() != that->codeLocs.size()) return false;
  
  // Compare all the pairs of CodeLocObjects in codeLocs and that.codeLocs, returning !defaultMayEq if any pair
  // returns !defaultMayEqual since we're looking for the tightest answer that any CodeLocObject in codeLocs can give
  for(list<CodeLocObjectPtr>::iterator thisIt=codeLocs.begin(), thatIt=that->codeLocs.begin();
      thisIt!=codeLocs.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->mustEqualCL(*thatIt, pedge) == !defaultMayEq) return !defaultMayEq;
  }
  
  return defaultMayEq;
}

// Returns whether the two abstract objects denote the same set of concrete objects
template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::equalSetCL(CodeLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
  assert(that);
  assert(codeLocs.size() == that->codeLocs.size());
  
  // Two unions and intersections denote the same set of their components individually denote the same set
  // (we can get a more precise answer if we could check set containment relations as well)
  list<CodeLocObjectPtr>::const_iterator clThis = codeLocs.begin();
  list<CodeLocObjectPtr>::const_iterator clThat = that->codeLocs.begin();
  for(; clThis!=codeLocs.end(); clThis++, clThat++)
    if(!(*clThis)->equalSetCL(*clThat, pedge)) return false;
  return true;
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::subSetCL(CodeLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
  assert(that);
  assert(codeLocs.size() == that->codeLocs.size());
  
  // Compare all the pairs of CodeLocObjects in memLocs and that.memLocs, returning defaultMayEq if any pair
  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true) 
  // answer that any CodeLocObject in memLocs can give
  for(list<CodeLocObjectPtr>::iterator thisIt=codeLocs.begin(), thatIt=that->codeLocs.begin();
      thisIt!=codeLocs.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->subSetCL(*thatIt, pedge) == defaultMayEq) return defaultMayEq;
  }
  return !defaultMayEq;
}

// Returns true if this object is live at the given part and false otherwise
template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::isLiveCL(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is live if any of its components are live (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is dead if any of its components are dead (strongest constraint)
  for(list<CodeLocObjectPtr>::const_iterator cl=codeLocs.begin(); cl!=codeLocs.end(); cl++)
    if((*cl)->isLiveCL(pedge) == defaultMayEq) return defaultMayEq;
  
  return !defaultMayEq;
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::meetUpdateCL(CodeLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
  assert(that);
  assert(codeLocs.size() == that->codeLocs.size());
  bool modified = false;
  
  // Perform the meetUpdate operation on all member codeLocss
  list<CodeLocObjectPtr>::const_iterator clThis = codeLocs.begin();
  list<CodeLocObjectPtr>::const_iterator clThat = that->codeLocs.begin();
  for(; clThis!=codeLocs.end(); clThis++, clThat++)
    modified = (*clThis)->meetUpdateCL(*clThat, pedge) || modified;
  return modified;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::isFullCL(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is full if any of its components are full (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is not full if any of its components are not full (strongest constraint)
  for(list<CodeLocObjectPtr>::const_iterator cl=codeLocs.begin(); cl!=codeLocs.end(); cl++)
    if((*cl)->isFullCL(pedge) == defaultMayEq) return defaultMayEq;
  
  return !defaultMayEq;
}


// Returns whether this AbstractObject denotes the empty set.
template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::isEmptyCL(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is not empty if any of its components are not empty (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is empty if any of its components are empty (strongest constraint)
  for(list<CodeLocObjectPtr>::const_iterator cl=codeLocs.begin(); cl!=codeLocs.end(); cl++)
    if((*cl)->isEmptyCL(pedge) != defaultMayEq) return !defaultMayEq;
  
  return defaultMayEq;
}

/*// Returns true if this object is live at the given part and false otherwise
template <bool defaultMayEq>
bool CombinedCodeLocObject<defaultMayEq>::isLive(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is live if any of its components are live (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is dead if any of its components are dead (strongest constraint)
  for(list<CodeLocObjectPtr>::const_iterator cl=codeLocs.begin(); cl!=codeLocs.end(); cl++)
    if((*cl)->isLive(pedge) == defaultMayEq) return defaultMayEq;
  
  return !defaultMayEq;
}*/

// Allocates a copy of this object and returns a pointer to it
template <bool defaultMayEq>
CodeLocObjectPtr CombinedCodeLocObject<defaultMayEq>::copyCL() const
{ return boost::make_shared<CombinedCodeLocObject>(codeLocs); }

template <bool defaultMayEq>
std::string CombinedCodeLocObject<defaultMayEq>::str(std::string indent)
{
  ostringstream oss;
  if(codeLocs.size()>1) oss << "["<<(defaultMayEq? "UnionCL" : "IntersectCL")<<": ";
  if(codeLocs.size()>1) oss << endl;
  for(list<CodeLocObjectPtr>::iterator cl=codeLocs.begin(); cl!=codeLocs.end(); ) {
    if(cl!=codeLocs.begin()) oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
    oss << (*cl)->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;");
    cl++;
    if(cl!=codeLocs.end()) oss << endl;
  }
  if(codeLocs.size()>1) oss << "]";
  
  return oss.str();
}

// Create a function that uses examples of combined objects to force the compiler to generate these classes
static void exampleCombinedCodeLocObjects2(CodeLocObjectPtr cl, std::list<CodeLocObjectPtr> cls, IntersectCodeLocObject& i, UnionCodeLocObject& u, IntersectCodeLocObject& i2, UnionCodeLocObject& u2);
static void exampleCombinedCodeLocObjects(CodeLocObjectPtr cl, std::list<CodeLocObjectPtr> cls)
{
  IntersectCodeLocObject exampleIntersectObject(cl);
  UnionCodeLocObject     exampleUnionObject(cl);
  IntersectCodeLocObject exampleIntersectObject2(cls);
  UnionCodeLocObject     exampleUnionObject2(cls);
  exampleCombinedCodeLocObjects2(cl, cls, exampleIntersectObject, exampleUnionObject, exampleIntersectObject2, exampleUnionObject2);
}
static void exampleCombinedCodeLocObjects2(CodeLocObjectPtr cl, std::list<CodeLocObjectPtr> cls, IntersectCodeLocObject& i, UnionCodeLocObject& u, IntersectCodeLocObject& i2, UnionCodeLocObject& u2)
{
  exampleCombinedCodeLocObjects(cl, cls);
}

/* #######################
   ##### ValueObject ##### 
   ####################### */

ValueObjectPtr NULLValueObject;

// Returns whether this object may/must be equal to o within the given Part p
// by propagating the call through the composer
bool ValueObject::mayEqual(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) { 
  /*if(mayEqualCache.find(o) == mayEqualCache.end())
    mayEqualCache[o] = comp->mayEqualV(shared_from_this(), o, pedge, analysis);
  return mayEqualCache[o];*/
  return comp->mayEqualV(shared_from_this(), o, pedge, analysis);
}

// Returns whether this object may/must be equal to o within the given Part p
  // by propagating the call through the composer
bool ValueObject::mustEqual(ValueObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) { 
  /*if(mustEqualCache.find(o) == mustEqualCache.end())
    mustEqualCache[o] = comp->mustEqualV(shared_from_this(), o, pedge, analysis);
  return mustEqualCache[o];*/
  return comp->mustEqualV(shared_from_this(), o, pedge, analysis);
}

// Returns whether this object may/must be equal to o within the given Part p
// by propagating the call through the composer
bool ValueObject::mayEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  //if(AbstractObject::mustEqualExpr(boost::static_pointer_cast<AbstractObject>(o), pedge)) return true;
  
  ValueObjectPtr vo = boost::dynamic_pointer_cast<ValueObject>(o);
  if(vo) return mayEqual(vo, pedge, comp, analysis);
  else   return false;
}

// Returns whether this object may/must be equal to o within the given Part p
// by propagating the call through the composer
bool ValueObject::mustEqual(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  //if(AbstractObject::mustEqualExpr(boost::static_pointer_cast<AbstractObject>(o), pedge)) return true;
  
  ValueObjectPtr vo = boost::dynamic_pointer_cast<ValueObject>(o);
  if(vo) return mustEqual(vo, pedge, comp, analysis);
  else   return false;
}

bool ValueObject::equalSet(ValueObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) { 
  /*if(equalSetCache.find(that) == equalSetCache.end())
    equalSetCache[that] = comp->equalSetV(shared_from_this(), that, pedge, analysis);
  return equalSetCache[that];*/
  return comp->equalSetV(shared_from_this(), that, pedge, analysis);
}

bool ValueObject::subSet(ValueObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) { 
  /*if(subSetCache.find(that) == subSetCache.end())
    subSetCache[that] = comp->equalSetV(shared_from_this(), that, pedge, analysis);
  return subSetCache[that];*/
  return comp->equalSetV(shared_from_this(), that, pedge, analysis);
}


bool ValueObject::equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  ValueObjectPtr co = boost::dynamic_pointer_cast<ValueObject>(o);
  if(co) return equalSet(co, pedge, comp, analysis);
  else   return false;
}

bool ValueObject::subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  ValueObjectPtr co = boost::dynamic_pointer_cast<ValueObject>(o);
  if(co) return subSet(co, pedge, comp, analysis);
  else   return false;
}

// General version of meetUpdate that accounts for framework details before routing the call to the derived class' 
// meetUpdateV check. Specifically, it routes the call through the composer to make sure the meetUpdateV 
// call gets the right PartEdge
bool ValueObject::meetUpdate(ValueObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->meetUpdateV(shared_from_this(), that, pedge, analysis); }

bool ValueObject::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  ValueObjectPtr v = boost::dynamic_pointer_cast<ValueObject>(that);
  assert(v);
  return meetUpdate(v, pedge, comp, analysis);
}

// General version of isFull/isEmpty that accounts for framework details before routing the call to the 
// derived class' isFullV/isEmptyV check. Specifically, it routes the call through the composer to make 
// sure the isFullV/isEmptyV call gets the right PartEdge
bool ValueObject::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->isFullV(shared_from_this(), pedge, analysis); }

bool ValueObject::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->isEmptyV(shared_from_this(), pedge, analysis); }

// Returns true if this SgValueExp is convertible into a boolean
bool ValueObject::isValueBoolCompatible(boost::shared_ptr<SgValueExp> val)
{
  return isSgCharVal(val.get())                ||
         isSgBoolValExp(val.get())             ||
         isSgEnumVal(val.get())                ||
         isSgIntVal(val.get())                 ||
         isSgLongIntVal(val.get())             ||
         isSgLongLongIntVal(val.get())         ||
         isSgShortVal(val.get())               ||
         isSgUnsignedCharVal(val.get())        ||
         isSgUnsignedLongVal(val.get())        ||
         isSgUnsignedLongLongIntVal(val.get()) ||
         isSgUnsignedShortVal(val.get())       ||
         isSgWcharVal(val.get());
}

// Convert the value of the given SgValueExp, cast to a boolean
bool ValueObject::SgValue2Bool(boost::shared_ptr<SgValueExp> val)
{
       if(isSgCharVal(val.get()))                return isSgCharVal(val.get())->get_value();
  else if(isSgBoolValExp(val.get()))             return isSgBoolValExp(val.get())->get_value();
  else if(isSgEnumVal(val.get()))                return isSgEnumVal(val.get())->get_value();
  else if(isSgIntVal(val.get()))                 return isSgIntVal(val.get())->get_value();
  else if(isSgLongIntVal(val.get()))             return isSgLongIntVal(val.get())->get_value();
  else if(isSgLongLongIntVal(val.get()))         return isSgLongLongIntVal(val.get())->get_value();
  else if(isSgShortVal(val.get()))               return isSgShortVal(val.get())->get_value();
  else if(isSgUnsignedCharVal(val.get()))        return isSgUnsignedCharVal(val.get())->get_value();
  else if(isSgUnsignedLongVal(val.get()))        return isSgUnsignedLongVal(val.get())->get_value();
  else if(isSgUnsignedLongLongIntVal(val.get())) return isSgUnsignedLongLongIntVal(val.get())->get_value();
  else if(isSgUnsignedShortVal(val.get()))       return isSgUnsignedShortVal(val.get())->get_value();
  else if(isSgWcharVal(val.get()))               return isSgWcharVal(val.get())->get_valueUL();
  else {
    dbg << "val="<<SgNode2Str(val.get())<<endl;
    assert(0);
  }
}

// Allocates a copy of this object and returns a pointer to it
AbstractObjectPtr ValueObject::copyAO() const
{ return copyV(); }

// Returns true if the two SgValueExps correspond to the same value when cast to the given type (if t!=NULL)
bool ValueObject::equalValueExp(SgValueExp* e1, SgValueExp* e2, SgType* t)
{
  // Currently not handling type conversions
  assert(t==NULL);
  
  if(e1->variantT() != e2->variantT()) return false;
  
  if(isSgBoolValExp(e1))             { return isSgBoolValExp(e1)->get_value()             == isSgBoolValExp(e2)->get_value(); }
  if(isSgCharVal(e1))                { return isSgCharVal(e1)->get_value()                == isSgCharVal(e2)->get_value(); }
  if(isSgComplexVal(e1))             { return equalValueExp(isSgComplexVal(e1)->get_real_value(),      isSgComplexVal(e2)->get_real_value()) &&
                                              equalValueExp(isSgComplexVal(e1)->get_imaginary_value(), isSgComplexVal(e2)->get_imaginary_value()); }
  if(isSgDoubleVal(e1))              { return isSgDoubleVal(e1)->get_value()              == isSgDoubleVal(e2)->get_value(); }
  if(isSgEnumVal(e1))                { return isSgEnumVal(e1)->get_value()                == isSgEnumVal(e2)->get_value(); }
  if(isSgFloatVal(e1))               { return isSgFloatVal(e1)->get_value()               == isSgFloatVal(e2)->get_value(); }
  if(isSgIntVal(e1))                 { return isSgIntVal(e1)->get_value()                 == isSgIntVal(e2)->get_value(); }
  if(isSgLongDoubleVal(e1))          { return isSgLongDoubleVal(e1)->get_value()          == isSgLongDoubleVal(e2)->get_value(); }
  if(isSgLongIntVal(e1))             { return isSgLongIntVal(e1)->get_value()             == isSgLongIntVal(e2)->get_value(); }
  if(isSgLongLongIntVal(e1))         { return isSgLongLongIntVal(e1)->get_value()         == isSgLongLongIntVal(e2)->get_value(); }
  if(isSgShortVal(e1))               { return isSgShortVal(e1)->get_value()               == isSgShortVal(e2)->get_value(); }
  if(isSgStringVal(e1))              { return isSgStringVal(e1)->get_value()              == isSgStringVal(e2)->get_value(); }
  if(isSgWcharVal(e1))               { return isSgWcharVal(e1)->get_value()               == isSgWcharVal(e2)->get_value(); }
  if(isSgUnsignedCharVal(e1))        { return isSgUnsignedCharVal(e1)->get_value()        == isSgUnsignedCharVal(e2)->get_value(); }
  if(isSgUnsignedIntVal(e1))         { return isSgUnsignedIntVal(e1)->get_value()         == isSgUnsignedIntVal(e2)->get_value(); }
  if(isSgUnsignedLongLongIntVal(e1)) { return isSgUnsignedLongLongIntVal(e1)->get_value() == isSgUnsignedLongLongIntVal(e2)->get_value(); }
  if(isSgUnsignedLongVal(e1))        { return isSgUnsignedLongVal(e1)->get_value()        == isSgUnsignedLongVal(e2)->get_value(); }
  if(isSgUnsignedShortVal(e1))       { return isSgWcharVal(e1)->get_value()               == isSgUnsignedShortVal(e2)->get_value(); }
  if(isSgUpcMythread(e1))            { return isSgUpcMythread(e1)->get_value()            == isSgUpcMythread(e2)->get_value(); }
  if(isSgUpcThreads(e1))             { return isSgUpcThreads(e1)->get_value()             == isSgUpcThreads(e2)->get_value(); }
  
  assert(0);
}

/* ##############################
   ##### UnknownValueObject ##### 
   ############################## */

// Returns whether this object may/must be equal to o within the given Part p
// These methods are private to prevent analyses from calling them directly.
bool UnknownValueObject::mayEqualV(ValueObjectPtr o, PartEdgePtr pedge)  {
  // Since this object denotes the set of all values, it may-equals all value sets
  return true;
}

bool UnknownValueObject::mustEqualV(ValueObjectPtr o, PartEdgePtr pedge) {
  // Since this object denotes the set of all values, which has unbounded size, it is not must-equal to any value set
  return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool UnknownValueObject::equalSetV(ValueObjectPtr o, PartEdgePtr pedge)  {
  // This object is only equal to objects that also denote the set of all values
  return o->isFullV(pedge); 
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool UnknownValueObject::subSetV(ValueObjectPtr o, PartEdgePtr pedge) {
  // This object is only a subset of objects that also denote the set of all values
  return o->isFullV(pedge); 
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool UnknownValueObject::meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge) {
  // There is no way to make this object denote a larger set of values since it already denotes
  // the set of all values
  return false;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool UnknownValueObject::isFullV(PartEdgePtr pedge)
{ return true; }

// Returns whether this AbstractObject denotes the empty set.
bool UnknownValueObject::isEmptyV(PartEdgePtr pedge)
{ return false; }

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool UnknownValueObject::isConcrete()
{ return false; }

// Returns the type of the concrete value (if there is one)
SgType* UnknownValueObject::getConcreteType()
{ return NULL; }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
set<boost::shared_ptr<SgValueExp> > UnknownValueObject::getConcreteValue()
{ return set<boost::shared_ptr<SgValueExp> >(); }

// Allocates a copy of this object and returns a pointer to it
ValueObjectPtr UnknownValueObject::copyV() const
{ return boost::make_shared<UnknownValueObject>(); }

std::string UnknownValueObject::str(std::string indent)
{ return "[UnknownValueObject]"; }

/* ################################
   ##### CombinedValueObject ##### 
   ################################ */

/*template <bool defaultMayEq>
CombinedValueObject<defaultMayEq>::CombinedValueObject(ValueObjectPtr val) : ValueObject(NULL) {
  vals.push_back(val);
}*/

/*template <bool defaultMayEq>
CombinedValueObject<defaultMayEq>::CombinedValueObject(const list<ValueObjectPtr>& vals) : ValueObject(NULL), vals(vals) {}*/

template <bool defaultMayEq>
void CombinedValueObject<defaultMayEq>::add(ValueObjectPtr val) {
  vals.push_back(val);
}

// Returns whether this object may/must be equal to o within the given Part p
// These methods are private to prevent analyses from calling them directly.
template <bool defaultMayEq>
bool CombinedValueObject<defaultMayEq>::mayEqualV(ValueObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedValueObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedValueObject<defaultMayEq> >(o);
  // If the two combination objects include different numbers of ValueObjects, say that they may be equal since 
  // we can't be sure either way.
  if(vals.size() != that->vals.size()) return true;
  
  // Compare all the pairs of ValueObjects in vals and that.vals, returning defaultMayEq if any pair
  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true) 
  // answer that any ValueObject in vals can give
  for(list<ValueObjectPtr>::iterator thisIt=vals.begin(), thatIt=that->vals.begin();
      thisIt!=vals.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->mayEqualV(*thatIt, pedge) == defaultMayEq) return defaultMayEq;
  }
  
  return !defaultMayEq;
}

template <bool defaultMayEq>
bool CombinedValueObject<defaultMayEq>::mustEqualV(ValueObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedValueObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedValueObject<defaultMayEq> >(o);
  // If the two combination  objects include different numbers of ValueObjects, say that they are not must equal since 
  // we can't be sure either way.
  if(vals.size() != that->vals.size()) return false;
  
  // Compare all the pairs of ValueObjects in vals and that.vals, returning !defaultMayEq if any pair
  // returns !defaultMayEqual since we're looking for the tightest answer that any ValueObject in vals can give
  for(list<ValueObjectPtr>::iterator thisIt=vals.begin(), thatIt=that->vals.begin();
      thisIt!=vals.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->mustEqualV(*thatIt, pedge) == !defaultMayEq) return !defaultMayEq;
  }
  
  return defaultMayEq;
}

// Returns whether the two abstract objects denote the same set of concrete objects
template <bool defaultMayEq>
bool CombinedValueObject<defaultMayEq>::equalSetV(ValueObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedValueObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedValueObject<defaultMayEq> >(o);
  assert(that);
  assert(vals.size() == that->vals.size());
  
  // Two unions and intersections denote the same set of their components individually denote the same set
  // (we can get a more precise answer if we could check set containment relations as well)
  list<ValueObjectPtr>::const_iterator vThis = vals.begin();
  list<ValueObjectPtr>::const_iterator vThat = that->vals.begin();
  for(; vThis!=vals.end(); vThis++, vThat++)
    if(!(*vThis)->equalSetV(*vThat, pedge)) return false;
  return true;
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
template <bool defaultMayEq>
bool CombinedValueObject<defaultMayEq>::subSetV(ValueObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedValueObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedValueObject<defaultMayEq> >(o);
  assert(that);
  assert(vals.size() == that->vals.size());
  
  // Compare all the pairs of ValueObjects in memLocs and that.memLocs, returning defaultMayEq if any pair
  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true) 
  // answer that any ValueObject in memLocs can give
  
  // Two unions and intersections denote the same set of their components individually denote the same set
  // (we can get a more precise answer if we could check set containment relations as well)
  for(list<ValueObjectPtr>::iterator thisIt=vals.begin(), thatIt=that->vals.begin();
      thisIt!=vals.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->subSetV(*thatIt, pedge) == defaultMayEq) return defaultMayEq;
  }
  return !defaultMayEq;
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
template <bool defaultMayEq>
bool CombinedValueObject<defaultMayEq>::meetUpdateV(ValueObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedValueObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedValueObject<defaultMayEq> >(o);
  assert(that);
  assert(vals.size() == that->vals.size());
  bool modified = false;
  
  // Perform the meetUpdate operation on all member Values
  list<ValueObjectPtr>::const_iterator vThis = vals.begin();
  list<ValueObjectPtr>::const_iterator vThat = that->vals.begin();
  for(; vThis!=vals.end(); vThis++, vThat++)
    modified = (*vThis)->meetUpdateV(*vThat, pedge) || modified;
  return modified;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
template <bool defaultMayEq>
bool CombinedValueObject<defaultMayEq>::isFullV(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is full if any of its components are full (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is not full if any of its components are not full (strongest constraint)
  for(list<ValueObjectPtr>::const_iterator v=vals.begin(); v!=vals.end(); v++)
    if((*v)->isFullV(pedge) == defaultMayEq) return defaultMayEq;
  
  return !defaultMayEq;
}


// Returns whether this AbstractObject denotes the empty set.
template <bool defaultMayEq>
bool CombinedValueObject<defaultMayEq>::isEmptyV(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is not empty if any of its components are not empty (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is empty if any of its components are empty (strongest constraint)
  for(list<ValueObjectPtr>::const_iterator v=vals.begin(); v!=vals.end(); v++)
    if((*v)->isEmptyV(pedge) != defaultMayEq) return !defaultMayEq;
  
  return defaultMayEq;
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
template <bool defaultMayEq>
bool CombinedValueObject<defaultMayEq>::isConcrete()
{
  // The combined object is concrete if 
  // intersect (defaultMayEq=false) : any sub-value is concrete
  // union (defaultMayEq=true) : all the sub-values are concrete and have the same type and value
  
  // Intersection
  if(defaultMayEq==false) {
    for(list<ValueObjectPtr>::iterator v=vals.begin(); v!=vals.end(); v++) {
      if((*v)->isConcrete()) return true;
    }
    return false;
  // Union
  } else {
    assert(vals.size()>0);
    list<ValueObjectPtr>::iterator firstI = vals.begin();
    ValueObjectPtr first = *firstI;
    
    // The union is not concrete if 
    for(list<ValueObjectPtr>::iterator v=vals.begin(); v!=vals.end(); v++) {
      // Any sub-value is not concrete, OR
      if(!(*v)->isConcrete())/* || 
         // Any pair of sub-values have different types, OR
         (*v)->getConcreteType()->variantT() != first->getConcreteType()->variantT() ||
         // Any pair of sub-values have different values
         !ValueObject::equalValueExp((*v)->getConcreteValue().get(), first->getConcreteValue().get())) {*/
        return false;
      //}
    }
    return true;
  }
}

// Returns the type of the concrete value (if there is one)
template <bool defaultMayEq>
SgType* CombinedValueObject<defaultMayEq>::getConcreteType()
{
  assert(isConcrete());
  
  return (*vals.begin())->getConcreteType();
}

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
template <bool defaultMayEq>
std::set<boost::shared_ptr<SgValueExp> > CombinedValueObject<defaultMayEq>::getConcreteValue()
{
  assert(isConcrete());
  
  //return (*vals.begin())->getConcreteValue();
  std::set<boost::shared_ptr<SgValueExp> > concreteVals;
  for(list<ValueObjectPtr>::iterator v=vals.begin(); v!=vals.end(); v++) {
    // Iterate through the current sub-ValueObject's concrete values and add to concreteVals
    // any values that are not already in it.
    std::set<boost::shared_ptr<SgValueExp> > curConcr = (*v)->getConcreteValue();
    for(std::set<boost::shared_ptr<SgValueExp> >::iterator i1=curConcr.begin(); i1!=curConcr.end(); i1++) {
      std::set<boost::shared_ptr<SgValueExp> >::iterator i2=concreteVals.begin();
      for(; i2!=concreteVals.end(); i2++) {
        // If the given value from the current sub-ValueObject is already in concreteVals, break out
        if(ValueObject::equalValueExp(i1->get(), i2->get()))
          break;
      }
      
      // If the given value from the current sub-ValueObject is not already in concreteVals, add it
      if(i2==concreteVals.end())
        concreteVals.insert(*i1);
    }
  }
  return concreteVals;
}

// Allocates a copy of this object and returns a pointer to it
template <bool defaultMayEq>
ValueObjectPtr CombinedValueObject<defaultMayEq>::copyV() const
{ return boost::make_shared<CombinedValueObject>(vals); }

template <bool defaultMayEq>
std::string CombinedValueObject<defaultMayEq>::str(std::string indent)
{
  ostringstream oss;
  if(vals.size()>1) oss << "["<<(defaultMayEq? "UnionV" : "IntersectV")<<": ";
  if(vals.size()>1) oss << endl;
  for(list<ValueObjectPtr>::iterator v=vals.begin(); v!=vals.end(); ) {
    if(v!=vals.begin()) oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
    oss << (*v)->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;");
    v++;
    if(v!=vals.end()) oss << endl;
  }
  if(vals.size()>1) oss << "]";
  
  return oss.str();
}

// GREG: Since Sriram's fix to explicitly declare templated Combined*Objects, we don't need the code below
/*
// Create a function that uses examples of combined objects to force the compiler to generate these classes
static void exampleCombinedValueObjects2(ValueObjectPtr val, std::list<ValueObjectPtr> vals, IntersectValueObject& i, UnionValueObject& u, IntersectValueObject& i2, UnionValueObject& u2);
static void exampleCombinedValueObjects(ValueObjectPtr val, std::list<ValueObjectPtr> vals)
{
  IntersectValueObject exampleIntersectObject(val);
  UnionValueObject     exampleUnionObject(val);
  IntersectValueObject exampleIntersectObject2(vals);
  UnionValueObject     exampleUnionObject2(vals);
  exampleCombinedValueObjects2(val, vals, exampleIntersectObject, exampleUnionObject, exampleIntersectObject2, exampleUnionObject2);
}
static void exampleCombinedValueObjects2(ValueObjectPtr val, std::list<ValueObjectPtr> vals, IntersectValueObject& i, UnionValueObject& u, IntersectValueObject& i2, UnionValueObject& u2)
{
  exampleCombinedValueObjects(val, vals);
}
*/

/* ###########################
   ##### MemRegionObject ##### 
   ########################### */

MemRegionObjectPtr NULLMemRegionObject;
  
// General version of mayEqual and mustEqual that accounts for framework details before routing the call to the 
// derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
// the call through the composer to make sure the may/mustEqual call gets the right PartEdge
bool MemRegionObject::mayEqual(MemRegionObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  // If either this or that are FuncResultMemRegionObject, they mayEqual iff they correspond to the same function
  FuncResultMemRegionObject* frmlcoThis  = dynamic_cast<FuncResultMemRegionObject*>(this);
  FuncResultMemRegionObjectPtr frmlcoThat = boost::dynamic_pointer_cast<FuncResultMemRegionObject>(that);
  if(frmlcoThis) return frmlcoThis->mayEqualMR(frmlcoThat, pedge);
  else if(frmlcoThat) return false;

  // If both this and that are both expression objects or both not expression objects, use the
  // derived class' equality check
  //dbg << "MemRegionObject::mayEqual() dynamic_cast<const ExprObj*>(this)="<<dynamic_cast<const ExprObj*>(this)<<" dynamic_cast<const ExprObj*>(o.get())="<<dynamic_cast<const ExprObj*>(o.get())<<endl;
  
  // GB 2013-09-16 - Commenting this out since we currently don't have code to return specifically expression
  //                 regions but this capability may be brought back for performance reasons (equality checks
  //                 can be sped up with we keep track of expression objects, for which identity is easy to check)
  // if((dynamic_cast<const ExprObj*>(this)  && dynamic_cast<const ExprObj*>(that.get())) ||
  //    (!dynamic_cast<const ExprObj*>(this) && !dynamic_cast<const ExprObj*>(that.get())))
  // { return mayEqualML(that, pedge); }
  /*if(mayEqualCache.find(that) == mayEqualCache.end())
    // Route the check through the composer, which makes sure to call the derived class' check at the correct PartEdge
    mayEqualCache[that] = comp->mayEqualMR(shared_from_this(), that, pedge, analysis);
  return mayEqualCache[that];*/
  return comp->mayEqualMR(shared_from_this(), that, pedge, analysis);
}

// General version of mayEqual and mustEqual that accounts for framework details before routing the call to the 
// derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
// the call through the composer to make sure the may/mustEqual call gets the right PartEdge
bool MemRegionObject::mustEqual(MemRegionObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  // If either this or that are FuncResultMemRegionObject, they mustEqual iff they both are
  FuncResultMemRegionObject* frmlcoThis  = dynamic_cast<FuncResultMemRegionObject*>(this);
  FuncResultMemRegionObjectPtr frmlcoThat = boost::dynamic_pointer_cast<FuncResultMemRegionObject>(that);
  if(frmlcoThis) return frmlcoThat->mustEqualMR(frmlcoThat, pedge);
  else if(frmlcoThat) return frmlcoThis;
  
  // Efficiently compute must equality for simple cases where the two MemRegionObjects correspond to the same SgNode
  //if(AbstractObject::mustEqualExpr(boost::static_pointer_cast<AbstractObject>(that), pedge)) return true;
  
  // GB 2013-09-16 - Commenting this out since we currently don't have code to return specifically expression
  //                 regions but this capability may be brought back for performance reasons (equality checks
  //                 can be sped up with we keep track of expression objects, for which identity is easy to check)
  // // If both this and that are both expression objects or both not expression objects, use the
  // // derived class' equality check
  // //dbg << "MemRegionObject::mustEqual() dynamic_cast<const ExprObj*>(this)="<<dynamic_cast<const ExprObj*>(this)<<"="<<const_cast<MemRegionObject*>(this)->str("")<<endl;
  // //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;dynamic_cast<const ExprObj*>(o.get())="<<dynamic_cast<const ExprObj*>(o.get())<<"="<<o->str("")<<endl;
  // if((dynamic_cast<const ExprObj*>(this)  && dynamic_cast<const ExprObj*>(that.get())) ||
  //    (!dynamic_cast<const ExprObj*>(this) && !dynamic_cast<const ExprObj*>(that.get())))
  // //{ return mustEqualML(that, pedge); }
  
  /*if(mustEqualCache.find(that) == mustEqualCache.end())
    // Route the check through the composer, which makes sure to call the derived class' check at the correct PartEdge
    mustEqualCache[that] = comp->mustEqualMR(shared_from_this(), that, pedge, analysis);
  return mustEqualCache[that];*/
  return comp->mustEqualMR(shared_from_this(), that, pedge, analysis);
}

// Check whether that is a MemRegionObject and if so, call the version of mayEqual specific to MemRegionObjects
bool MemRegionObject::mayEqual(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  MemRegionObjectPtr mo = boost::dynamic_pointer_cast<MemRegionObject>(that);
  if(mo) return mayEqual(mo, pedge, comp, analysis);
  else   return false;
}

// Check whether that is a MemRegionObject and if so, call the version of mustEqual specific to MemRegionObjects
bool MemRegionObject::mustEqual(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  //if(AbstractObject::mustEqualExpr(that, pedge)) return true;
  
  MemRegionObjectPtr mo = boost::dynamic_pointer_cast<MemRegionObject>(that);
  if(mo) return mustEqual(mo, pedge, comp, analysis);
  else   return false;
}

bool MemRegionObject::equalSet(MemRegionObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  // If either this or that are FuncResultMemRegionObject, they denote equal sets iff they correspond to the same function
  FuncResultMemRegionObject* frmlcoThis  = dynamic_cast<FuncResultMemRegionObject*>(this);
  FuncResultMemRegionObjectPtr frmlcoThat = boost::dynamic_pointer_cast<FuncResultMemRegionObject>(that);
  if(frmlcoThis) return frmlcoThis->equalSetMR(frmlcoThat, pedge);
  else if(frmlcoThat) return false;
 
  /*if(equalSetCache.find(that) == equalSetCache.end())
    equalSetCache[that] = comp->equalSetMR(shared_from_this(), that, pedge, analysis);
  return equalSetCache[that];*/
  return comp->equalSetMR(shared_from_this(), that, pedge, analysis);
}

bool MemRegionObject::subSet(MemRegionObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  // If either this or that are FuncResultMemRegionObject, they denote equal sets iff they correspond to the same function
  FuncResultMemRegionObject* frmlcoThis  = dynamic_cast<FuncResultMemRegionObject*>(this);
  FuncResultMemRegionObjectPtr frmlcoThat = boost::dynamic_pointer_cast<FuncResultMemRegionObject>(that);
  if(frmlcoThis) return frmlcoThis->subSetMR(frmlcoThat, pedge);
  else if(frmlcoThat) return false;
  
  /*if(subSetCache.find(that) == subSetCache.end())
    subSetCache[that] = comp->subSetMR(shared_from_this(), that, pedge, analysis);
  return subSetCache[that];*/
  return comp->subSetMR(shared_from_this(), that, pedge, analysis);
}


bool MemRegionObject::equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  MemRegionObjectPtr co = boost::dynamic_pointer_cast<MemRegionObject>(o);
  if(co) return equalSet(co, pedge, comp, analysis);
  else   return false;
}

bool MemRegionObject::subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  MemRegionObjectPtr co = boost::dynamic_pointer_cast<MemRegionObject>(o);
  if(co) return subSet(co, pedge, comp, analysis);
  else   return false;
}

// General version of isLive that accounts for framework details before routing the call to the derived class' 
// isLiveML check. Specifically, it routes the call through the composer to make sure the isLiveML call gets the 
// right PartEdge
bool MemRegionObject::isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->isLiveMR(shared_from_this(), pedge, analysis); }

// General version of meetUpdate() that accounts for framework details before routing the call to the derived class' 
// meetUpdateMR check. Specifically, it routes the call through the composer to make sure the meetUpdateMR
// call gets the right PartEdge
bool MemRegionObject::meetUpdate(MemRegionObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->meetUpdateMR(shared_from_this(), that, pedge, analysis); }

bool MemRegionObject::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  MemRegionObjectPtr mr = boost::dynamic_pointer_cast<MemRegionObject>(that);
  assert(mr);
  return meetUpdate(mr, pedge, comp, analysis);
}

// General version of isFull/isEmpty that accounts for framework details before routing the call to the 
  // derived class' isFullMR/isEmptyMR check. Specifically, it routes the call through the composer to make 
  // sure the isFullMR/isEmptyMR call gets the right PartEdge
bool MemRegionObject::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->isFullMR(shared_from_this(), pedge, analysis); }

bool MemRegionObject::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return comp->isEmptyMR(shared_from_this(), pedge, analysis); }

/* #####################################
   ##### FuncResultMemRegionObject ##### 
   ##################################### */

// Special MemLocObject used internally by the framework to associate with the return value of a function

FuncResultMemRegionObject::FuncResultMemRegionObject(Function func) : 
    MemRegionObject(NULL), func(func)
{ }

// Returns whether this object may/must be equal to o within the given Part p
bool FuncResultMemRegionObject::mayEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge) {
// FuncResultMemRegionObjects are disjoint. They're either equal or not subsets.
  return mustEqualMR(o, pedge);
}

bool FuncResultMemRegionObject::mustEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge) {
  //The two objects denote the same set iff they're both FuncResultMemRegionObjects that correspond to the same function
  FuncResultMemRegionObjectPtr that = boost::dynamic_pointer_cast<FuncResultMemRegionObject>(o);
  return that && func==that->func;
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool FuncResultMemRegionObject::equalSetMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  // FuncResultMemRegionObjects are disjoint. They're either equal or not subsets.
  return mustEqualMR(o, pedge);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool FuncResultMemRegionObject::subSetMR(MemRegionObjectPtr o, PartEdgePtr pedge) {
  // FuncResultMemRegionObjects are disjoint. They're either equal or not subsets.
  return mustEqualMR(o, pedge);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool FuncResultMemRegionObject::meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge)
{
  assert(0);
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool FuncResultMemRegionObject::isFullMR(PartEdgePtr pedge)
{
  return false;
}

// Returns whether this AbstractObject denotes the empty set.
bool FuncResultMemRegionObject::isEmptyMR(PartEdgePtr pedge)
{
  return false;
}

// Returns a ValueObject that denotes the size of this memory region
ValueObjectPtr FuncResultMemRegionObject::getRegionSize(PartEdgePtr pedge) const
{
  // The size of a function result is irrelevant since its internals cannot be accessed directly
  // (its possible to access the internals of its SgFunctionCallExp though) so we return an unknown size.
  return boost::make_shared<UnknownValueObject>();
}

MemRegionObjectPtr FuncResultMemRegionObject::copyMR() const
{
  return boost::make_shared<FuncResultMemRegionObject>(func);
}

/* ###################################
   ##### CombinedMemRegionObject ##### 
   ################################### */

/* // Creates a new CombinedMemRegionObject instance of the generic CombinedMemRegionObject class.
template <bool defaultMayEq>
boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > CombinedMemRegionObject<defaultMayEq>::create(MemRegionObjectPtr memReg)
{
  list<MemRegionObjectPtr> memRegions; memRegions.push_back(memReg);
  return boost::make_shared<CombinedMemRegionObject<defaultMayEq> >(memRegions);
  
  // dbg << "<font color=\"$#ff0000\">"<<memLoc->str()<<"</font>"<<endl;
}

// Sriram: gcc 4.1.2 complains of undefined references to unused to template functions
// fix: explicit template instantiation
template boost::shared_ptr<CombinedMemRegionObject<true> > CombinedMemRegionObject<true>::create(MemRegionObjectPtr memReg);
template boost::shared_ptr<CombinedMemRegionObject<false> > CombinedMemRegionObject<false>::create(MemRegionObjectPtr memReg);

template <bool defaultMayEq>
boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > CombinedMemRegionObject<defaultMayEq>::create(const std::list<MemRegionObjectPtr>& memRegions)
{
  //dbg << "CombinedMemRegionObject<"<<defaultMayEq<<">::create() generic "<< endl;
  return boost::make_shared<CombinedMemRegionObject<defaultMayEq> >(memRegions);
}

// Sriram: gcc 4.1.2 complains of undefined references to unused to template functions
// fix: explicit template instantiation
template boost::shared_ptr<CombinedMemRegionObject<true> > CombinedMemRegionObject<true>::create(const std::list<MemRegionObjectPtr>& memRegions);
template boost::shared_ptr<CombinedMemRegionObject<false> > CombinedMemRegionObject<false>::create(const std::list<MemRegionObjectPtr>& memRegions);

template <bool defaultMayEq>
void CombinedMemRegionObject<defaultMayEq>::add(MemRegionObjectPtr memReg) {
  memRegions.push_back(memReg);
}
*/

// Returns whether this object may/must be equal to o within the given Part p
template <bool defaultMayEq>
bool CombinedMemRegionObject<defaultMayEq>::mayEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  //dbg << "Comparing " << this->str("    ") << "with " << o->str("    ") << endl;
  boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemRegionObject<defaultMayEq> >(o);
  assert(that);
  
  // If the two combination objects include different numbers of MemRegionObjects, say that they may be equal since 
  // we can't be sure either way.
  if(memRegions.size() != that->memRegions.size()) return true;
  
  assert(memRegions.size()>0);
  
  // Compare all the pairs of MemRegionObjects in memLocs and that.memLocs, returning defaultMayEq if any pair
  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true) 
  // answer that any MemRegionObject in memLocs can give
  for(list<MemRegionObjectPtr>::iterator thisIt=memRegions.begin(), thatIt=that->memRegions.begin();
      thisIt!=memRegions.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->mayEqualMR(*thatIt, pedge) == defaultMayEq) return defaultMayEq;
  }
  
  return !defaultMayEq;
}

template <bool defaultMayEq>
bool CombinedMemRegionObject<defaultMayEq>::mustEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemRegionObject<defaultMayEq> >(o);
  assert(that);
  
  // If the two combination  objects include different numbers of MemRegionObjects, say that they are not must equal since 
  // we can't be sure either way.
  if(memRegions.size() != that->memRegions.size()) return false;
  
  assert(memRegions.size()>0);
  
  // Compare all the pairs of MemRegionObjects in memLocs and that.memLocs, returning !defaultMayEq if any pair
  // returns !defaultMayEqual since we're looking for the tightest answer that any MemRegionObject in memLocs can give
  for(list<MemRegionObjectPtr>::iterator thisIt=memRegions.begin(), thatIt=that->memRegions.begin();
      thisIt!=memRegions.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->mustEqualMR(*thatIt, pedge) == !defaultMayEq) return !defaultMayEq;
  }
  
  return defaultMayEq;
}

// Returns whether the two abstract objects denote the same set of concrete objects
template <bool defaultMayEq>
bool CombinedMemRegionObject<defaultMayEq>::equalSetMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemRegionObject<defaultMayEq> >(o);
  assert(that);
  assert(memRegions.size() == that->memRegions.size());
  assert(memRegions.size()>0);
  
  // Two unions and intersections denote the same set of their components individually denote the same set
  // (we can get a more precise answer if we could check set containment relations as well)
  list<MemRegionObjectPtr>::const_iterator mrThis = memRegions.begin();
  list<MemRegionObjectPtr>::const_iterator mrThat = that->memRegions.begin();
  for(; mrThis!=memRegions.end(); mrThis++, mrThat++)
    if(!(*mrThis)->equalSetMR(*mrThat, pedge)) return false;
  return true;
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
template <bool defaultMayEq>
bool CombinedMemRegionObject<defaultMayEq>::subSetMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemRegionObject<defaultMayEq> >(o);
  assert(that);
  assert(memRegions.size() == that->memRegions.size());
  assert(memRegions.size()>0);
  
  // Compare all the pairs of MemRegionObjects in memLocs and that.memLocs, returning defaultMayEq if any pair
  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true) 
  // answer that any MemRegionObject in memLocs can give
  for(list<MemRegionObjectPtr>::iterator thisIt=memRegions.begin(), thatIt=that->memRegions.begin();
      thisIt!=memRegions.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->subSetMR(*thatIt, pedge) == defaultMayEq) return defaultMayEq;
  }
  return !defaultMayEq;
}

// Allocates a copy of this object and returns a pointer to it
template <bool defaultMayEq>
MemRegionObjectPtr CombinedMemRegionObject<defaultMayEq>::copyMR() const
{ return boost::make_shared<CombinedMemRegionObject>(memRegions); }

// Returns true if this object is live at the given part and false otherwise
template <bool defaultMayEq>
bool CombinedMemRegionObject<defaultMayEq>::isLiveMR(PartEdgePtr pedge)
{
  assert(memRegions.size()>0);
  
  // If this is a union type (defaultMayEq=true), an object is live if any of its components are live (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is dead if any of its components are dead (strongest constraint)
  for(list<MemRegionObjectPtr>::const_iterator mr=memRegions.begin(); mr!=memRegions.end(); mr++)
    if((*mr)->isLiveMR(pedge) == defaultMayEq) return defaultMayEq;
  
  return !defaultMayEq;
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
template <bool defaultMayEq>
bool CombinedMemRegionObject<defaultMayEq>::meetUpdateMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedMemRegionObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemRegionObject<defaultMayEq> >(o);
  assert(that);
  assert(memRegions.size() == that->memRegions.size());
  assert(memRegions.size()>0);
  bool modified = false;
  
  // Perform the meetUpdate operation on all member MemLocs
  list<MemRegionObjectPtr>::const_iterator mrThis = memRegions.begin();
  list<MemRegionObjectPtr>::const_iterator mrThat = that->memRegions.begin();
  for(; mrThis!=memRegions.end(); mrThis++, mrThat++)
    modified = (*mrThis)->meetUpdateMR(*mrThat, pedge) || modified;
  return modified;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
template <bool defaultMayEq>
bool CombinedMemRegionObject<defaultMayEq>::isFullMR(PartEdgePtr pedge)
{
  assert(memRegions.size()>0);
  
  // If this is a union type (defaultMayEq=true), an object is full if any of its components are full (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is not full if any of its components are not full (strongest constraint)
  for(list<MemRegionObjectPtr>::const_iterator mr=memRegions.begin(); mr!=memRegions.end(); mr++)
    if((*mr)->isFullMR(pedge) == defaultMayEq) return defaultMayEq;
  
  return !defaultMayEq;
}

// Returns whether this AbstractObject denotes the empty set.
template <bool defaultMayEq>
bool CombinedMemRegionObject<defaultMayEq>::isEmptyMR(PartEdgePtr pedge)
{
  assert(memRegions.size()>0);

  // If this is a union type (defaultMayEq=true), an object is not empty if any of its components are not empty (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is empty if any of its components are empty (strongest constraint)
  for(list<MemRegionObjectPtr>::const_iterator mr=memRegions.begin(); mr!=memRegions.end(); mr++)
    if((*mr)->isEmptyMR(pedge) != defaultMayEq) return !defaultMayEq;
  
  return defaultMayEq;
}

// Returns a ValueObject that denotes the size of this memory region
template <bool defaultMayEq>
ValueObjectPtr CombinedMemRegionObject<defaultMayEq>::getRegionSize(PartEdgePtr pedge) const
{
  assert(memRegions.size()>0);
  
  ValueObjectPtr res;
  // Merge the ValueObjects returned by calls to gerRegionSize on all sub-regions
  for(list<MemRegionObjectPtr>::const_iterator mr=memRegions.begin(); mr!=memRegions.end(); mr++) {
    if(mr==memRegions.begin())
      res = (*mr)->getRegionSize(pedge)->copyV();
    else
      // !!! GB 2012-09-16 : this is not quite right. We should be calling meetUpdate() to make sure
      //        the call gets routed through the generic ValueObject machinery, if any, before being 
      //        forwarded to the meetUpdateV() method.
      res->meetUpdateV((*mr)->getRegionSize(pedge), pedge);
  }
  return res;
}

template <bool defaultMayEq>
std::string CombinedMemRegionObject<defaultMayEq>::str(std::string indent)
{
  ostringstream oss;
  if(memRegions.size()>1) oss << "["<<(defaultMayEq? "UnionMR" : "IntersectMR")<<": ";
  if(memRegions.size()>1) oss << endl;
  for(list<MemRegionObjectPtr>::iterator mr=memRegions.begin(); mr!=memRegions.end(); ) {
    if(mr!=memRegions.begin()) oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
    oss << (*mr)->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;");
    mr++;
    if(mr!=memRegions.end()) oss << endl;
  }
  if(memRegions.size()>1) oss << "]";
  
  return oss.str();
}

/* ########################
   ##### MemLocObject ##### 
   ######################## */

MemLocObjectPtr NULLMemLocObject;
  
// Returns whether this object may/must be equal to o within the given Part p
// These methods are called by composers and should not be called by analyses.
bool MemLocObject::mayEqualML(MemLocObjectPtr that, PartEdgePtr pedge) {
  /*scope s("MemLocObject::mayEqualML");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that->str()<<endl;
  
  dbg << "eqRegion="<<(region->mayEqualMR(that->getRegion(), pedge))<<endl;
  if(index && that->index) dbg << "eqIndex="<<(index->mayEqualV(that->getIndex(), pedge))<<endl;*/

  return region && region->mayEqualMR(that->getRegion(), pedge) && 
         ((!index && !that->index) || index->mayEqualV(that->getIndex(), pedge));
}

bool MemLocObject::mustEqualML(MemLocObjectPtr that, PartEdgePtr pedge) {
  /*scope s("MemLocObject::mustEqualML");
  dbg << "eqRegion="<<(region->mustEqualMR(that->getRegion(), pedge))<<endl;
  if(index && that->index) dbg << "eqIndex="<<(index->mustEqualV(that->getIndex(), pedge))<<endl;*/
  
  return region->mustEqualMR(that->getRegion(), pedge) && 
         ((!index && !that->index) || index->mustEqualV(that->getIndex(), pedge));
}

// General version of mayEqual and mustEqual that accounts for framework details before routing the call to the 
// derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
// the call through the composer to make sure the may/mustEqual call gets the right PartEdge
bool MemLocObject::mayEqual(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // If this is a FuncResultMemLocObject and that is not or vice versa, they're not may-equal
  if((dynamic_cast<FuncResultMemLocObject*>(this)==NULL) !=
         (boost::dynamic_pointer_cast<FuncResultMemLocObject>(that)==NULL))
    return false;
  
  /*scope s("MemLocObject::mayEqual");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that->str()<<endl;*/
  
  return mayEqualML(that, pedge);
}

// General version of mayEqual and mustEqual that accounts for framework details before routing the call to the 
// derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
// the call through the composer to make sure the may/mustEqual call gets the right PartEdge
bool MemLocObject::mustEqual(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // If this is a FuncResultMemLocObject and that is not or vice versa, they're not must-equal
  if((dynamic_cast<FuncResultMemLocObject*>(this)==NULL) !=
         (boost::dynamic_pointer_cast<FuncResultMemLocObject>(that)==NULL))
    return false;
  return mustEqualML(that, pedge);
}

// Check whether that is a MemLocObject and if so, call the version of mayEqual specific to MemLocObjects
bool MemLocObject::mayEqual(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  MemLocObjectPtr mo = boost::dynamic_pointer_cast<MemLocObject>(that);
  if(mo) return mayEqual(mo, pedge, comp, analysis);
  else   return false;
}

// Check whether that is a MemLocObject and if so, call the version of mustEqual specific to MemLocObjects
bool MemLocObject::mustEqual(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
  //if(AbstractObject::mustEqualExpr(that, pedge)) return true;
  
  MemLocObjectPtr mo = boost::dynamic_pointer_cast<MemLocObject>(that);
  if(mo) return mustEqual(mo, pedge, comp, analysis);
  else   return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
// These methods are called by composers and should not be called by analyses.
bool MemLocObject::equalSetML(MemLocObjectPtr that, PartEdgePtr pedge) {
  return region->equalSetMR(that->getRegion(), pedge) && 
         ((!index && !that->index) || index->equalSetV(that->getIndex(), pedge));
}
// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
// These methods are called by composers and should not be called by analyses.
bool MemLocObject::subSetML(MemLocObjectPtr that, PartEdgePtr pedge) {
  return region->subSetMR(that->getRegion(), pedge) && 
         ((!index && !that->index) || index->subSetV(that->getIndex(), pedge));
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool MemLocObject::equalSet(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // If this is a FuncResultMemLocObject and that is not or vice versa, the sets they denote are not equal
  if((dynamic_cast<FuncResultMemLocObject*>(this)==NULL) !=
         (boost::dynamic_pointer_cast<FuncResultMemLocObject>(that)==NULL))
    return false;
  
  return equalSetML(that, pedge);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool MemLocObject::subSet(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // If this is a FuncResultMemLocObject and that is not or vice versa, one is not a subset of the other
  if((dynamic_cast<FuncResultMemLocObject*>(this)==NULL) !=
         (boost::dynamic_pointer_cast<FuncResultMemLocObject>(that)==NULL))
    return false;
  
  return subSetML(that, pedge);
}

bool MemLocObject::equalSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  MemLocObjectPtr co = boost::dynamic_pointer_cast<MemLocObject>(o);
  if(co) return equalSet(co, pedge, comp, analysis);
  else   return false;
}

bool MemLocObject::subSet(AbstractObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ 
  MemLocObjectPtr co = boost::dynamic_pointer_cast<MemLocObject>(o);
  if(co) return subSet(co, pedge, comp, analysis);
  else   return false;
}

// Returns true if this object is live at the given part and false otherwise
bool MemLocObject::isLiveML(PartEdgePtr pedge) {
  return region->isLiveMR(pedge);
}

// General version of isLive that accounts for framework details before routing the call to the derived class' 
// isLiveML check. Specifically, it routes the call through the composer to make sure the isLiveML call gets the 
// right PartEdge
bool MemLocObject::isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return isLiveML(pedge); }

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool MemLocObject::meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge) { 
  bool modified = false;
  modified = region->meetUpdateMR(that->getRegion(), pedge) || modified;
  if(index) modified = index->meetUpdateV(that->getIndex(), pedge) || modified;
  return modified;
}

// General version of meetUpdate() that accounts for framework details before routing the call to the derived class' 
// meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML 
// call gets the right PartEdge
bool MemLocObject::meetUpdate(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // If this is a FuncResultMemLocObject and that is not or vice versa, we'll need to make this object full
  if((dynamic_cast<FuncResultMemLocObject*>(this)==NULL) !=
         (boost::dynamic_pointer_cast<FuncResultMemLocObject>(that)==NULL))
    assert(0);
  
  return meetUpdateML(that, pedge);
}

bool MemLocObject::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  MemLocObjectPtr ml = boost::dynamic_pointer_cast<MemLocObject>(that);
  assert(ml);
  return meetUpdate(ml, pedge, comp, analysis);
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool MemLocObject::isFullML(PartEdgePtr pedge) {
 return region->isFullMR(pedge) && (!index || index->isFullV(pedge));
}

// Returns whether this AbstractObject denotes the empty set.
bool MemLocObject::isEmptyML(PartEdgePtr pedge) {
  return region->isEmptyMR(pedge) && (!index || index->isEmptyV(pedge));
}

// General versions of isFull() and isEmpty that account for framework details before routing the call to the 
// derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make 
// sure the isFull(PartEdgePtr) and isEmpty(PartEdgePtr) call gets the right PartEdge.
// These functions are just aliases for the real implementations in AbstractObject
bool MemLocObject::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return isFullML(pedge); }

bool MemLocObject::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return isEmptyML(pedge); }

// Allocates a copy of this object and returns a shared pointer to it
MemLocObjectPtr MemLocObject::copyML() const
{ return boost::make_shared<MemLocObject>(*this); }

// Allocates a copy of this object and returns a regular pointer to it
MemLocObject* MemLocObject::copyMLPtr() const
{ return new MemLocObject(*this); }


std::string MemLocObject::str(std::string indent) { // pretty print for the object
  ostringstream oss; 
  oss << "[MemLocObject region="<<region->str(indent+"    ");
  if(index) {
    oss <<", "<<endl<<
         indent << "             index="<<index->str(indent+"    ");
  }
  oss <<"]";
  return oss.str();
}

/* ##################################
   ##### FuncResultMemLocObject ##### 
   ################################## */

// Special MemLocObject used internally by the framework to associate with the return value of a function
// This is just a MemLoc dedicated to wrapping FuncResultMemLocRegion
FuncResultMemLocObject::FuncResultMemLocObject(Function func) : 
    MemLocObject(boost::make_shared<FuncResultMemRegionObject>(func), NULLValueObject, NULL)
{ }

FuncResultMemLocObject::FuncResultMemLocObject(const FuncResultMemLocObject& that) : 
    MemLocObject(that)
{ }

MemLocObjectPtr FuncResultMemLocObject::copyML() const
{
  return boost::make_shared<FuncResultMemLocObject>(*this);
}

/* ################################
   ##### CombinedMemLocObject ##### 
   ################################ */

/*template <bool defaultMayEq>
boost::shared_ptr<CombinedMemLocObject<defaultMayEq> > CombinedMemLocObject<defaultMayEq>::create(const std::list<MemLocObjectPtr>& memLocs)
{
  //dbg << "CombinedMemLocObject<"<<defaultMayEq<<">::create() generic "<< endl;
  return boost::make_shared<CombinedMemLocObject<defaultMayEq> >(memLocs);
}

// Sriram: gcc 4.1.2 complains of undefined references to unused to template functions
// fix: explicit template instantiation
template boost::shared_ptr<CombinedMemLocObject<true> > CombinedMemLocObject<true>::create(const std::list<MemLocObjectPtr>& memLocs);
template boost::shared_ptr<CombinedMemLocObject<false> > CombinedMemLocObject<false>::create(const std::list<MemLocObjectPtr>& memLocs);
*/

template <bool defaultMayEq>
void CombinedMemLocObject<defaultMayEq>::add(MemLocObjectPtr memLoc) {
  memLocs.push_back(memLoc);
}

// Returns whether this object may/must be equal to o within the given Part p
template <bool defaultMayEq>
bool CombinedMemLocObject<defaultMayEq>::mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge)
{
  //dbg << "Comparing " << this->str("    ") << "with " << o->str("    ") << endl;
  boost::shared_ptr<CombinedMemLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemLocObject<defaultMayEq> >(o);
  assert(that);
  
  // If the two combination objects include different numbers of MemLocObjects, say that they may be equal since 
  // we can't be sure either way.
  if(memLocs.size() != that->memLocs.size()) return true;
  
  // Compare all the pairs of MemLocObjects in memLocs and that.memLocs, returning defaultMayEq if any pair
  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true) 
  // answer that any MemLocObject in memLocs can give
  for(list<MemLocObjectPtr>::iterator thisIt=memLocs.begin(), thatIt=that->memLocs.begin();
      thisIt!=memLocs.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->mayEqualML(*thatIt, pedge) == defaultMayEq) return defaultMayEq;
  }
  
  return !defaultMayEq;
}

template <bool defaultMayEq>
bool CombinedMemLocObject<defaultMayEq>::mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedMemLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemLocObject<defaultMayEq> >(o);
  assert(that);
  
  // If the two combination  objects include different numbers of MemLocObjects, say that they are not must equal since 
  // we can't be sure either way.
  if(memLocs.size() != that->memLocs.size()) return false;
  
  // Compare all the pairs of MemLocObjects in memLocs and that.memLocs, returning !defaultMayEq if any pair
  // returns !defaultMayEqual since we're looking for the tightest answer that any MemLocObject in memLocs can give
  for(list<MemLocObjectPtr>::iterator thisIt=memLocs.begin(), thatIt=that->memLocs.begin();
      thisIt!=memLocs.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->mustEqualML(*thatIt, pedge) == !defaultMayEq) return !defaultMayEq;
  }
  
  return defaultMayEq;
}

// Returns whether the two abstract objects denote the same set of concrete objects
template <bool defaultMayEq>
bool CombinedMemLocObject<defaultMayEq>::equalSetML(MemLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedMemLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemLocObject<defaultMayEq> >(o);
  assert(that);
  assert(memLocs.size() == that->memLocs.size());
  
  // Two unions and intersections denote the same set of their components individually denote the same set
  // (we can get a more precise answer if we could check set containment relations as well)
  list<MemLocObjectPtr>::const_iterator mlThis = memLocs.begin();
  list<MemLocObjectPtr>::const_iterator mlThat = that->memLocs.begin();
  for(; mlThis!=memLocs.end(); mlThis++, mlThat++)
    if(!(*mlThis)->equalSetML(*mlThat, pedge)) return false;
  return true;
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
template <bool defaultMayEq>
bool CombinedMemLocObject<defaultMayEq>::subSetML(MemLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedMemLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemLocObject<defaultMayEq> >(o);
  assert(that);
  assert(memLocs.size() == that->memLocs.size());
  
  // Compare all the pairs of MemLocObjects in memLocs and that.memLocs, returning defaultMayEq if any pair
  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true) 
  // answer that any MemLocObject in memLocs can give
  for(list<MemLocObjectPtr>::iterator thisIt=memLocs.begin(), thatIt=that->memLocs.begin();
      thisIt!=memLocs.end();
      thisIt++, thatIt++)
  {
    if((*thisIt)->subSetML(*thatIt, pedge) == defaultMayEq) return defaultMayEq;
  }
  return !defaultMayEq;
}

// Returns true if this object is live at the given part and false otherwise
template <bool defaultMayEq>
bool CombinedMemLocObject<defaultMayEq>::isLiveML(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is live if any of its components are live (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is dead if any of its components are dead (strongest constraint)
  for(list<MemLocObjectPtr>::const_iterator ml=memLocs.begin(); ml!=memLocs.end(); ml++)
    if((*ml)->isLiveML(pedge) == defaultMayEq) return defaultMayEq;
  
  return !defaultMayEq;
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
template <bool defaultMayEq>
bool CombinedMemLocObject<defaultMayEq>::meetUpdateML(MemLocObjectPtr o, PartEdgePtr pedge)
{
  boost::shared_ptr<CombinedMemLocObject<defaultMayEq> > that = boost::dynamic_pointer_cast<CombinedMemLocObject<defaultMayEq> >(o);
  assert(that);
  assert(memLocs.size() == that->memLocs.size());
  bool modified = false;
  
  // Perform the meetUpdate operation on all member MemLocs
  list<MemLocObjectPtr>::const_iterator mlThis = memLocs.begin();
  list<MemLocObjectPtr>::const_iterator mlThat = that->memLocs.begin();
  for(; mlThis!=memLocs.end(); mlThis++, mlThat++)
    modified = (*mlThis)->meetUpdateML(*mlThat, pedge) || modified;
  return modified;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
template <bool defaultMayEq>
bool CombinedMemLocObject<defaultMayEq>::isFullML(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is full if any of its components are full (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is not full if any of its components are not full (strongest constraint)
  for(list<MemLocObjectPtr>::const_iterator ml=memLocs.begin(); ml!=memLocs.end(); ml++)
    if((*ml)->isFullML(pedge) == defaultMayEq) return defaultMayEq;
  
  return !defaultMayEq;
}

// Returns whether this AbstractObject denotes the empty set.
template <bool defaultMayEq>
bool CombinedMemLocObject<defaultMayEq>::isEmptyML(PartEdgePtr pedge)
{
  // If this is a union type (defaultMayEq=true), an object is not empty if any of its components are not empty (weakest constraint)
  // If this is an intersection type (defaultMayEq=false), an object is empty if any of its components are empty (strongest constraint)
  for(list<MemLocObjectPtr>::const_iterator ml=memLocs.begin(); ml!=memLocs.end(); ml++)
    if((*ml)->isEmptyML(pedge) != defaultMayEq) return !defaultMayEq;
  
  return defaultMayEq;
}

// Allocates a copy of this object and returns a pointer to it
template <bool defaultMayEq>
MemLocObjectPtr CombinedMemLocObject<defaultMayEq>::copyML() const
{ return boost::make_shared<CombinedMemLocObject>(memLocs); }

template <bool defaultMayEq>
std::string CombinedMemLocObject<defaultMayEq>::str(std::string indent)
{
  ostringstream oss;
  if(memLocs.size()>1) oss << "["<<(defaultMayEq? "UnionML" : "IntersectML")<<": ";
  if(memLocs.size()>1) oss << endl;
  for(list<MemLocObjectPtr>::iterator ml=memLocs.begin(); ml!=memLocs.end(); ) {
    if(ml!=memLocs.begin()) oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
    oss << (*ml)->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;");
    ml++;
    if(ml!=memLocs.end()) oss << endl;
  }
  if(memLocs.size()>1) oss << "]";
  
  return oss.str();
}

/* #######################
   ##### IndexVector ##### 
   ####################### */

//std::string IndexVector::str(const string& indent)
// pretty print for the object
std::string IndexVector::str(std::string indent)
{
 dbg<<"Error. Direct call to base class (IndexVector)'s str() is not allowed."<<endl;
 //assert (false);
 return "";  
}
bool IndexVector::mayEqual (IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
 cerr<<"Error. Direct call to base class (IndexVector)'s mayEqual() is not allowed."<<endl;
 assert (false);
 return false;  
}
bool IndexVector::mustEqual (IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
 cerr<<"Error. Direct call to base class (IndexVector)'s mustEqual() is not allowed."<<endl;
 assert (false);
 return false;  
}

// Returns whether the two abstract index vectors denote the same set of concrete vectors.
bool IndexVector::equalSet(IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
 cerr<<"Error. Direct call to base class (IndexVector)'s equalSet() is not allowed."<<endl;
 assert (false);
 return false;  
}

// Returns whether this abstract index vector denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract index vector.
bool IndexVector::subSet(IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
 cerr<<"Error. Direct call to base class (IndexVector)'s subSet() is not allowed."<<endl;
 assert (false);
 return false;  
}

bool IndexVector::meetUpdate (IndexVectorPtr other, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
 cerr<<"Error. Direct call to base class (IndexVector)'s meetUpdate() is not allowed."<<endl;
 assert (false);
 return false;  
}

bool IndexVector::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
 cerr<<"Error. Direct call to base class (IndexVector)'s isFull() is not allowed."<<endl;
 assert (false);
 return false;  
}

bool IndexVector::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{
 cerr<<"Error. Direct call to base class (IndexVector)'s isEmpty() is not allowed."<<endl;
 assert (false);
 return false;  
}

} //namespace fuse
