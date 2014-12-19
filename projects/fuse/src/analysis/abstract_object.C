#include "sage3basic.h"
#include "abstract_object.h"
#include "stx_analysis.h"
#include <iostream>

using namespace std;
using namespace sight;

namespace fuse
{

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

std::string CodeLocObject::str(std::string indent) const { // pretty print for the object
  ostringstream oss; 
  oss << "[CodeLocObject: part="<<(part? part->str(indent+"    "): "ANY")<<", "<<endl;
  oss << indent << "                cfgNode="<<(cfgNode.getNode()? CFGNode2Str(cfgNode): "ANY")<<"]";
  return oss.str();
}

/* #################################
   ##### FullCodeLocObject ##### 
   ################################# */

bool FullCodeLocObject::mayEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge) {
  return true;
}

bool FullCodeLocObject::mustEqualCL(CodeLocObjectPtr o, PartEdgePtr pedge) {
  return false;
}
  
bool FullCodeLocObject::equalSetCL(CodeLocObjectPtr o, PartEdgePtr pedge) {
  return isFullCL(pedge);
}
  
bool FullCodeLocObject::subSetCL(CodeLocObjectPtr o, PartEdgePtr pedge) {
  return isFullCL(pedge);
}
  
bool FullCodeLocObject::isLiveCL(PartEdgePtr pedge) {
  return true;
}
  
bool FullCodeLocObject::meetUpdateCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  return false;
}
  
bool FullCodeLocObject::isFullCL(PartEdgePtr pedge) {
  return true;
}

bool FullCodeLocObject::isEmptyCL(PartEdgePtr pedge) {
  return false;
}
  
CodeLocObjectPtr FullCodeLocObject::copyCL() const {
  return boost::make_shared<FullCodeLocObject>();
}
  
string FullCodeLocObject::str(string indent) const {
  return "FullCodeLocObject";    
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
std::string CombinedCodeLocObject<defaultMayEq>::str(std::string indent) const
{
  ostringstream oss;
  if(codeLocs.size()>1) oss << "["<<(defaultMayEq? "UnionCL" : "IntersectCL")<<": ";
  if(codeLocs.size()>1) oss << endl;
  for(list<CodeLocObjectPtr>::const_iterator cl=codeLocs.begin(); cl!=codeLocs.end(); ) {
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

/* ###############################
   ##### MappedCodeLocObject ##### 
   ############################### */

template<class Key, bool mostAccurate>
void MappedCodeLocObject<Key, mostAccurate>::add(Key key, CodeLocObjectPtr cl_p, PartEdgePtr pedge) {
  // If the object is already full don't add anything
  if(union_ && isFullCL(pedge)) return;

  // If the cl_p is not full add/update the map
  if(!cl_p->isFullCL(pedge)) {
    codeLocsMap[key] = cl_p;
  }
  else {
    n_FullCL++;
    if(union_) setCLToFull();
  }
}

template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::mayEqualCLWithKey(Key key,
                                                              const map<Key, CodeLocObjectPtr>& thatCLMap, 
                                                              PartEdgePtr pedge) {
  typename map<Key, CodeLocObjectPtr>::const_iterator s_it;
  s_it = thatCLMap.find(key);
  if(s_it == thatCLMap.end()) return true;
  return codeLocsMap[key]->mayEqualCL(s_it->second, pedge);
}


//! Two CL objects are may equals if there is atleast one execution or sub-exectuion
//! in which they represent the same code location.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described 
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise, the conservative answer for mayEqualCL is true.
//! Mapped CLs are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed CL object correspond to some dataflow facts computed by Key=Analysis* or 
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MayEquality check on mapped CL is performed on intersection of sub-executions
//! or union of sub-executions over the keyed CL objects. 
template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::mayEqualCL(CodeLocObjectPtr thatCL, PartEdgePtr pedge) {
  boost::shared_ptr<MappedCodeLocObject<Key, mostAccurate> > thatCL_p = 
    boost::dynamic_pointer_cast<MappedCodeLocObject<Key, mostAccurate> >(thatCL);
  assert(thatCL_p);

  // This object denotes full set of CL (full set of executions)
  if(isFullCL(pedge)) return true;

  // denotes empty set
  if(isEmptyCL(pedge)) return false;

  // presence of one more full objects will result in full set over union
  if(union_ && n_FullCL > 0) return true;

  // Two cases reach here [1] union_=true && nFull_CL=0 [2] intersect=true && nFullCL=0 or nFull_CL!=0.
  // For both cases iterate on the CL map and discharge the mayEqualCL query to individual objects 
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, CodeLocObjectPtr> thatCLMap = thatCL_p->getCodeLocsMap();
  typename map<Key, CodeLocObjectPtr>::iterator it;
  for(it = codeLocsMap.begin(); it != codeLocsMap.end(); ++it) {
    // discharge query
    bool isMayEq = mayEqualCLWithKey(it->first, thatCLMap, pedge);

    // 1. Union of sub-executions and the object does not contain any full objects.
    // If the discharged query comes back as true for this case then we have found atleast one execution
    // under which the two objects are same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If false we iterate further as any CL can add more executions under which the objects are may equals.
    if(union_ && isMayEq==true) return true;

    // 2. Intersection of sub-executions and the object may contain full objects (n_FullCL != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns false then return false.
    // We did not find one execution in which the two objects are may equals. 
    // Note that set of executions are contained over keyed objects (analyses are conservative).
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns true iterate further as some executions corresponding to true may be dropped.
    else if(intersect_ && isMayEq==false) return false;
  }


  // All the keyed objects returned false for the discharged query under union.
  // We haven't found a single execution under which the two objects are may equals.
  if(union_) return false;
  // All the keyed objects returned true for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are may equals.
  else if(intersect_) return true;
  else assert(0);
}

template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::mustEqualCLWithKey(Key key,
                                                               const map<Key, CodeLocObjectPtr>& thatCLMap, 
                                                               PartEdgePtr pedge) {
  typename map<Key, CodeLocObjectPtr>::const_iterator s_it;
  s_it = thatCLMap.find(key);
  if(s_it == thatCLMap.end()) return false;
  return codeLocsMap[key]->mustEqualCL(s_it->second, pedge);
}

//! Two CL objects are must equals if they represent the same code 
//! location on all executions.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described 
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise conservative answer for mustEqualCL is false.
//! Mapped CLs are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed CL object correspond to some dataflow facts computed by Key=Analysis* or 
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MustEquality check on mapped CL is performed on intersection (mostAccurate=true) of sub-executions
//! or union (mostAccurate=false) of sub-executions over the keyed CL objects. 
template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::mustEqualCL(CodeLocObjectPtr thatCL, PartEdgePtr pedge) {
  boost::shared_ptr<MappedCodeLocObject<Key, mostAccurate> > thatCL_p = 
    boost::dynamic_pointer_cast<MappedCodeLocObject<Key, mostAccurate> >(thatCL);
  assert(thatCL_p);

  // This object denotes full set of CL (full set of executions)
  if(isFullCL(pedge)) return false;

  // denotes empty set
  if(isEmptyCL(pedge)) return false;

  // presence of one more full objects will result in full set over union
  if(union_ && n_FullCL > 0) return true;

  // Two cases reach here [1] union_=true && nFull_CL=0 [2] intersect=true && nFullCL=0 or nFull_CL!=0.
  // For both cases iterate on the CL map and discharge the mayEqualCL query to individual objects 
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, CodeLocObjectPtr> thatCLMap = thatCL_p->getCodeLocsMap();
  typename map<Key, CodeLocObjectPtr>::iterator it;
  for(it = codeLocsMap.begin(); it != codeLocsMap.end(); ++it) {
    // discharge query
    bool isMustEq = mustEqualCLWithKey(it->first, thatCLMap, pedge);

    // 1. Union of sub-executions and the object does not contain any full objects
    // If the discharged query comes back as false for this case then we have found atleast one execution
    // under which the two objects are not same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If it returns true we iterate further as any CL can add more executions under which the objects are not must equals.
    if(union_ && isMustEq==false) return false;

    // 2. Intersection of sub-executions and the object may contain full objects (n_FullCL != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns true then return true. 
    // Under all sub-executions (corresponding to the CL) the two objects must equal.
    // Note that set of executions are contained over keyed objects as the analyses are conservative.
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns false iterate further as some executions corresponding to false may be dropped.
    else if(intersect_ && isMustEq==true) return true;
  }

  // All the keyed objects returned true for the discharged query under union.
  // We haven't found a single execution under which the two objects are not equal.
  if(union_) return true;
  // All the keyed objects returned false for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are not equal.
  else if(intersect_) return false;
  else assert(0);
}

//! Discharge the query to the corresponding CL
//! If key not found in thatCLMap return false
template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::equalSetCLWithKey(Key key,
                                                              const map<Key, CodeLocObjectPtr>& thatCLMap, 
                                                              PartEdgePtr pedge) {
  typename map<Key, CodeLocObjectPtr>::const_iterator s_it;
  s_it = thatCLMap.find(key);
  if(s_it == thatCLMap.end()) return false;
  return codeLocsMap[key]->equalSetCL(s_it->second, pedge);
}

//! Two objects are equal sets if they denote the same set of memory locations
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed CodeLoc objects
//! If all the discharged queries come back equal then the two objects are equal otherwise not.
template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::equalSetCL(CodeLocObjectPtr thatCL, PartEdgePtr pedge) {
  boost::shared_ptr<MappedCodeLocObject<Key, mostAccurate> > thatCL_p = 
    boost::dynamic_pointer_cast<MappedCodeLocObject<Key, mostAccurate> >(thatCL);  
  assert(thatCL_p);

  // This object denotes full set of CL (full set of executions)
  if(isFullCL(pedge)) return thatCL_p->isFullCL(pedge);

  // denotes empty set
  if(isEmptyCL(pedge)) return thatCL_p->isEmptyCL(pedge);

  const map<Key, CodeLocObjectPtr> thatCLMap = thatCL_p->getCodeLocsMap();
  typename map<Key, CodeLocObjectPtr>::iterator it;
  for(it = codeLocsMap.begin(); it != codeLocsMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if(equalSetCLWithKey(it->first, thatCLMap, pedge) == false) return false;
  }

  return true;
}

//! Discharge the query to the corresponding CL
//! If key not found in thatCLMap return true as the
//! keyed object on thatCLMap denotes full set
template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::subSetCLWithKey(Key key,
                                                            const map<Key, CodeLocObjectPtr>& thatCLMap, 
                                                            PartEdgePtr pedge) {
  typename map<Key, CodeLocObjectPtr>::const_iterator s_it;
  s_it = thatCLMap.find(key);
  if(s_it == thatCLMap.end()) return true;
  return codeLocsMap[key]->equalSetCL(s_it->second, pedge);
}

//! This object is a non-strict subset of the other if the set of memory locations denoted by this
//! is a subset of the set of memory locations denoted by that.
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed CodeLoc objects
//! If all the discharged queries come back true then this is a subset of that otherwise not.
template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::subSetCL(CodeLocObjectPtr thatCL, PartEdgePtr pedge) {
  boost::shared_ptr<MappedCodeLocObject<Key, mostAccurate> > thatCL_p = 
    boost::dynamic_pointer_cast<MappedCodeLocObject<Key, mostAccurate> >(thatCL);  
  assert(thatCL_p);

  // This object denotes full set of CL (full set of executions)
  if(isFullCL(pedge)) return thatCL_p->isFullCL(pedge);

  // denotes empty set
  // thatCL could be empty or non-empty eitherway this will be a non-strict subset of that.
  if(isEmptyCL(pedge)) return true;

  // If both objects have the same keys discharge
  // If this object has a key and that does not then 
  // the keyed object is subset of that (return true) implemented by subsetCLWithKey
  // If any of the discharged query return false then return false.
  const map<Key, CodeLocObjectPtr> thatCLMap = thatCL_p->getCodeLocsMap();
  typename map<Key, CodeLocObjectPtr>::iterator it;
  for(it = codeLocsMap.begin(); it != codeLocsMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if(subSetCLWithKey(it->first, thatCLMap, pedge) == false) return false;
  }

  // If this object doesn't have the key and that object has the key then 
  // return false as this object has full object mapped to the key
  typename map<Key, CodeLocObjectPtr>::const_iterator c_it;
  for(c_it = thatCLMap.begin(); c_it != thatCLMap.end() && (n_FullCL != 0); ++c_it) {
    if(codeLocsMap.find(c_it->first) == codeLocsMap.end()) return false;
  }

  return true;
}

//! Mapped object liveness is determined based on finding executions
//! in which it may be live.
//! It can be answered based on union (mostAccurate=false) or intersection
//! (mostAccurate=true) of executions
//! The conservative answer is to assume that the object is live
template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::isLiveCL(PartEdgePtr pedge) {
  // If this object is full return the conservative answer
  if(isFullCL(pedge)) return true;

  // If it has one or more full objects added to it
  // and if the object has mostAccurate=false then return true (weakest answer)
  if(n_FullCL > 0 && union_) return true;

  // 1. This object may have have one or more full objects under intersection
  // 2. This object doesnt have any full objects added to it under union
  // Under both cases the answer is based on how individual analysis respond to the query
  typename map<Key, CodeLocObjectPtr>::iterator it = codeLocsMap.begin();
  for( ; it != codeLocsMap.end(); ++it) {
    bool isLive = it->second->isLiveCL(pedge);
    if(union_ && isLive==true) return true;
    else if(intersect_ && isLive==false) return false;
  }
  
  // leftover cases
  if(union_) return false;
  else if(intersect_) return true;
  else assert(0);
}

//! meetUpdateCL performs the join operation of abstractions of two mls
template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::meetUpdateCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<MappedCodeLocObject<Key, mostAccurate> > thatCL_p =
    boost::dynamic_pointer_cast<MappedCodeLocObject<Key, mostAccurate> >(that);  
  assert(thatCL_p);

  // if this object is already full
  if(isFullCL(pedge)) return false;

  // If that object is full set this object to full
  if(thatCL_p->isFullCL(pedge)) {
    n_FullCL++;
    setCLToFull();
    return true;
  }

  // Both objects are not full
  const map<Key, CodeLocObjectPtr> thatCLMap = thatCL_p->getCodeLocsMap();
  
  typename map<Key, CodeLocObjectPtr>::iterator it = codeLocsMap.begin();
  typename map<Key, CodeLocObjectPtr>::const_iterator s_it;   // search iterator for thatCLMap

  bool modified = false;
  while(it != codeLocsMap.end()) {
    s_it = thatCLMap.find(it->first);
    // If two objects have the same key then discharge meetUpdate to the corresponding keyed CL objects
    if(s_it != thatCLMap.end()) {
      modified = (it->second)->meetUpdateCL(s_it->second, pedge) || modified;
    }

    // Remove the current CL object (current iterator it) from the map if the mapepd object is full.
    // Two cases under which the current CL object can be full.
    // (1) If current key is not found in thatCLMap then the mapped object
    // in thatCLMap is full and the meetUpdate of the current CL with that is also full.
    // (2) meetUpdateCL above of the two keyed objects resulted in this mapped object being full.
    // Under both cases remove the mapped ml from this map
    if(s_it == thatCLMap.end() || (it->second)->isFullCL(pedge)) {
      // Current mapped CL has become full as a result of (1) or (2).
      // Remove the item from the map.
      // Note that post-increment which increments the iterator and returns the old value for deletion.
      codeLocsMap.erase(it++);
      n_FullCL++;
      modified = true;

      // If mostAccurate=false then set this entire object to full and return
      if(!mostAccurate) {
        setCLToFull();
        return true;
      }
    }
    else ++it;
  }
  return modified;
}

//! Method that sets this mapped object to full
template<class Key, bool mostAccurate>
void MappedCodeLocObject<Key, mostAccurate>::setCLToFull() {
  assert(n_FullCL > 0);
  if(codeLocsMap.size() > 0) codeLocsMap.clear();
}

template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::isFullCL(PartEdgePtr pedge) {
  if(n_FullCL > 0 && codeLocsMap.size() == 0) return true;
  return false;
}

template<class Key, bool mostAccurate>
bool MappedCodeLocObject<Key, mostAccurate>::isEmptyCL(PartEdgePtr pedge) {
  if(n_FullCL == 0 && codeLocsMap.size() == 0) return true;
  return false;
}

template<class Key, bool mostAccurate>
CodeLocObjectPtr MappedCodeLocObject<Key, mostAccurate>::copyCL() const {
  return boost::make_shared<MappedCodeLocObject<Key, mostAccurate> >(*this);
}

template<class Key, bool mostAccurate>
string MappedCodeLocObject<Key, mostAccurate>::str(string indent) const {
  ostringstream oss;
  oss << "<table border=\"1\">";
  oss << "<tr>";
  oss << "<th>" << (union_? "UnionMappedCodeLocObject:": "IntersectMappedCodeLocObject:") << "</th>";
  if(n_FullCL > 0 && codeLocsMap.size() == 0) oss << "<th> Full </th> </tr>";
  else if(n_FullCL == 0 && codeLocsMap.size() == 0) oss << "<th> Empty </th> </tr>";
  else {
    oss << "</tr>";
    typename map<Key, CodeLocObjectPtr>::const_iterator it = codeLocsMap.begin();
    for( ; it != codeLocsMap.end(); ++it) {
      oss << "<tr>";
      oss << "<td>" << (it->first)->str(indent) << "</td>";
      oss << "<td>" << (it->second)->str(indent) << "</td>";
      oss << "</tr>";
    }    
  }
  oss << "</table>";
  return oss.str();
}

/* ############################
   # PartEdgeUnionCodeLocObject #
   ############################ */

PartEdgeUnionCodeLocObject::PartEdgeUnionCodeLocObject() : 
  CodeLocObject(NULL) {
}

PartEdgeUnionCodeLocObject::PartEdgeUnionCodeLocObject(const PartEdgeUnionCodeLocObject& thatCL) :
  CodeLocObject(thatCL), unionCL_p(thatCL.unionCL_p->copyCL()) {
}

void PartEdgeUnionCodeLocObject::add(CodeLocObjectPtr cl_p, PartEdgePtr pedge) {
  // If this is the very first object
  if(!unionCL_p) unionCL_p = cl_p->copyCL();  
  // If Full return without adding
  else if(isFullCL(pedge)) return; 
  // Else meetUpdate with the existing unionCL_p
  else unionCL_p->meetUpdateCL(cl_p, pedge);
}

bool PartEdgeUnionCodeLocObject::mayEqualCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->mayEqualCL(thatCL_p->getUnionCL(), pedge);
}

bool PartEdgeUnionCodeLocObject::mustEqualCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->mustEqualCL(thatCL_p->getUnionCL(), pedge);
}

bool PartEdgeUnionCodeLocObject::equalSetCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->equalSetCL(thatCL_p->getUnionCL(), pedge);
}

bool PartEdgeUnionCodeLocObject::subSetCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->subSetCL(thatCL_p->getUnionCL(), pedge);
}

bool PartEdgeUnionCodeLocObject::meetUpdateCL(CodeLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->meetUpdateCL(thatCL_p->getUnionCL(), pedge);
}
  
bool PartEdgeUnionCodeLocObject::isLiveCL(PartEdgePtr pedge) {
  return unionCL_p->isLiveCL(pedge);
}

bool PartEdgeUnionCodeLocObject::isFullCL(PartEdgePtr pedge) {
  return unionCL_p->isFullCL(pedge);
}

bool PartEdgeUnionCodeLocObject::isEmptyCL(PartEdgePtr pedge) {
  return unionCL_p->isEmptyCL(pedge);
}

CodeLocObjectPtr PartEdgeUnionCodeLocObject::copyCL() const {
  return boost::make_shared<PartEdgeUnionCodeLocObject>(*this);
}

void PartEdgeUnionCodeLocObject::setCLToFull() {
  unionCL_p = boost::make_shared<FullCodeLocObject>();
}

string PartEdgeUnionCodeLocObject::str(string indent) const {
  ostringstream oss;
  oss << "[UnionCL=" << unionCL_p->str(indent) << "]";
  return oss.str();
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
   ##### FullValueObject ##### 
   ############################## */

// Returns whether this object may/must be equal to o within the given Part p
// These methods are private to prevent analyses from calling them directly.
bool FullValueObject::mayEqualV(ValueObjectPtr o, PartEdgePtr pedge)  {
  // Since this object denotes the set of all values, it may-equals all value sets
  return true;
}

bool FullValueObject::mustEqualV(ValueObjectPtr o, PartEdgePtr pedge) {
  // Since this object denotes the set of all values, which has unbounded size, it is not must-equal to any value set
  return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool FullValueObject::equalSetV(ValueObjectPtr o, PartEdgePtr pedge)  {
  // This object is only equal to objects that also denote the set of all values
  return o->isFullV(pedge); 
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool FullValueObject::subSetV(ValueObjectPtr o, PartEdgePtr pedge) {
  // This object is only a subset of objects that also denote the set of all values
  return o->isFullV(pedge); 
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool FullValueObject::meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge) {
  // There is no way to make this object denote a larger set of values since it already denotes
  // the set of all values
  return false;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool FullValueObject::isFullV(PartEdgePtr pedge)
{ return true; }

// Returns whether this AbstractObject denotes the empty set.
bool FullValueObject::isEmptyV(PartEdgePtr pedge)
{ return false; }

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool FullValueObject::isConcrete()
{ return false; }

// Returns the type of the concrete value (if there is one)
SgType* FullValueObject::getConcreteType()
{ return NULL; }

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
set<boost::shared_ptr<SgValueExp> > FullValueObject::getConcreteValue()
{ return set<boost::shared_ptr<SgValueExp> >(); }

// Allocates a copy of this object and returns a pointer to it
ValueObjectPtr FullValueObject::copyV() const
{ return boost::make_shared<FullValueObject>(); }

std::string FullValueObject::str(std::string indent) const
{ return "[FullValueObject]"; }

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
std::string CombinedValueObject<defaultMayEq>::str(std::string indent) const
{
  ostringstream oss;
  if(vals.size()>1) oss << "["<<(defaultMayEq? "UnionV" : "IntersectV")<<": ";
  if(vals.size()>1) oss << endl;
  for(list<ValueObjectPtr>::const_iterator v=vals.begin(); v!=vals.end(); ) {
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
   #### MappedValueObject ####
   ########################### */

//! Method to add values to the map.
//! Vs that are full are never added to the map.
//! If v_p is FullV or v_p->isFullV=true then mapped V is set to full only if mostAccurate=false.
template<class Key, bool mostAccurate>
void MappedValueObject<Key, mostAccurate>::add(Key key, ValueObjectPtr v_p, PartEdgePtr pedge) {
  // If the object is already full don't add anything
  if(union_ && isFullV(pedge)) return;

  // If the v_p is not full add/update the map
  if(!v_p->isFullV(pedge)) {
    valuesMap[key] = v_p;
  }
  else {
    n_FullV++;
    if(union_) setVToFull();
  }
}

template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::mayEqualVWithKey(Key key,
                                                              const map<Key, ValueObjectPtr>& thatVMap, 
                                                              PartEdgePtr pedge) {
  typename map<Key, ValueObjectPtr>::const_iterator s_it;
  s_it = thatVMap.find(key);
  if(s_it == thatVMap.end()) return true;
  return valuesMap[key]->mayEqualV(s_it->second, pedge);
}

//! Two Value objects are may equals if there is atleast one execution or sub-exectuion
//! in which they represent the same value.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described 
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise, the conservative answer for mayEqualV is true.
//! Mapped Values are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed Value object correspond to some dataflow facts computed by Key=Analysis* or 
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MayEquality check on mapped Value is performed on intersection of sub-executions
//! or union of sub-executions over the keyed Value objects. 
template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::mayEqualV(ValueObjectPtr thatV, PartEdgePtr pedge) {
  boost::shared_ptr<MappedValueObject<Key, mostAccurate> > thatV_p = 
    boost::dynamic_pointer_cast<MappedValueObject<Key, mostAccurate> >(thatV);
  assert(thatV_p);

  // This object denotes full set of Values (full set of executions)
  if(isFullV(pedge)) return true;

  // denotes empty set
  if(isEmptyV(pedge)) return false;

  // presence of one more full objects will result in full set over union
  if(union_ && n_FullV > 0) return true;

  // Two cases reach here [1] union_=true && nFull_V=0 [2] intersect=true && nFullV=0 or nFull_V!=0.
  // For both cases iterate on the Value map and discharge the mayEqualV query to individual objects 
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, ValueObjectPtr> thatVMap = thatV_p->getValuesMap();
  typename map<Key, ValueObjectPtr>::iterator it;
  for(it = valuesMap.begin(); it != valuesMap.end(); ++it) {
    // discharge query
    bool isMayEq = mayEqualVWithKey(it->first, thatVMap, pedge);

    // 1. Union of sub-executions and the object does not contain any full objects.
    // If the discharged query comes back as true for this case then we have found atleast one execution
    // under which the two objects are same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If false we iterate further as any Value can add more executions under which the objects are may equals.
    if(union_ && isMayEq==true) return true;

    // 2. Intersection of sub-executions and the object may contain full objects (n_FullV != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns false then return false.
    // We did not find one execution in which the two objects are may equals. 
    // Note that set of executions are contained over keyed objects (analyses are conservative).
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns true iterate further as some executions corresponding to true may be dropped.
    else if(intersect_ && isMayEq==false) return false;
  }


  // All the keyed objects returned false for the discharged query under union.
  // We haven't found a single execution under which the two objects are may equals.
  if(union_) return false;
  // All the keyed objects returned true for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are may equals.
  else if(intersect_) return true;
  else assert(0);
}

template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::mustEqualVWithKey(Key key,
                                                               const map<Key, ValueObjectPtr>& thatVMap, 
                                                               PartEdgePtr pedge) {
  typename map<Key, ValueObjectPtr>::const_iterator s_it;
  s_it = thatVMap.find(key);
  if(s_it == thatVMap.end()) return false;
  return valuesMap[key]->mustEqualV(s_it->second, pedge);
}

//! Two Value objects are must equals if they represent the same value
//! on all executions.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described 
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise conservative answer for mustEqualV is false.
//! Mapped Values are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed Value object correspond to some dataflow facts computed by Key=Analysis* or 
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MustEquality check on mapped Value is performed on intersection (mostAccurate=true) of sub-executions
//! or union (mostAccurate=false) of sub-executions over the keyed Value objects. 
template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::mustEqualV(ValueObjectPtr thatV, PartEdgePtr pedge) {
  boost::shared_ptr<MappedValueObject<Key, mostAccurate> > thatV_p = 
    boost::dynamic_pointer_cast<MappedValueObject<Key, mostAccurate> >(thatV);
  assert(thatV_p);

  // This object denotes full set of Value (full set of executions)
  if(isFullV(pedge)) return false;

  // denotes empty set
  if(isEmptyV(pedge)) return false;

  // presence of one more full objects will result in full set over union
  if(union_ && n_FullV > 0) return true;

  // Two cases reach here [1] union_=true && nFull_V=0 [2] intersect=true && nFullV=0 or nFull_V!=0.
  // For both cases iterate on the Value map and discharge the mayEqualV query to individual objects 
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, ValueObjectPtr> thatVMap = thatV_p->getValuesMap();
  typename map<Key, ValueObjectPtr>::iterator it;
  for(it = valuesMap.begin(); it != valuesMap.end(); ++it) {
    // discharge query
    bool isMustEq = mustEqualVWithKey(it->first, thatVMap, pedge);

    // 1. Union of sub-executions and the object does not contain any full objects
    // If the discharged query comes back as false for this case then we have found atleast one execution
    // under which the two objects are not same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If it returns true we iterate further as any Value can add more executions under which the objects are not must equals.
    if(union_ && isMustEq==false) return false;

    // 2. Intersection of sub-executions and the object may contain full objects (n_FullV != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns true then return true. 
    // Under all sub-executions (corresponding to the Value) the two objects must equal.
    // Note that set of executions are contained over keyed objects as the analyses are conservative.
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns false iterate further as some executions corresponding to false may be dropped.
    else if(intersect_ && isMustEq==true) return true;
  }

  // All the keyed objects returned true for the discharged query under union.
  // We haven't found a single execution under which the two objects are not equal.
  if(union_) return true;
  // All the keyed objects returned false for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are not equal.
  else if(intersect_) return false;
  else assert(0);
}

//! Discharge the query to the corresponding VO
//! If key not found in thatVMap return false
template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::equalSetVWithKey(Key key,
                                                              const map<Key, ValueObjectPtr>& thatVMap, 
                                                              PartEdgePtr pedge) {
  typename map<Key, ValueObjectPtr>::const_iterator s_it;
  s_it = thatVMap.find(key);
  if(s_it == thatVMap.end()) return false;
  return valuesMap[key]->equalSetV(s_it->second, pedge);
}

//! Two objects are equal sets if they denote the same set of values.
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed Value objects
//! If all the discharged queries come back equal then the two objects are equal otherwise not.
template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::equalSetV(ValueObjectPtr thatV, PartEdgePtr pedge) {
  boost::shared_ptr<MappedValueObject<Key, mostAccurate> > thatV_p = 
    boost::dynamic_pointer_cast<MappedValueObject<Key, mostAccurate> >(thatV);  
  assert(thatV_p);

  // This object denotes full set of Values (full set of executions)
  if(isFullV(pedge)) return thatV_p->isFullV(pedge);

  // denotes empty set
  if(isEmptyV(pedge)) return thatV_p->isEmptyV(pedge);

  const map<Key, ValueObjectPtr> thatVMap = thatV_p->getValuesMap();
  typename map<Key, ValueObjectPtr>::iterator it;
  for(it = valuesMap.begin(); it != valuesMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if(equalSetVWithKey(it->first, thatVMap, pedge) == false) return false;
  }

  return true;
}

//! Discharge the query to the corresponding VO
//! If key not found in thatVMap return true as the
//! keyed object on thatVMap denotes full set
template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::subSetVWithKey(Key key,
                                                            const map<Key, ValueObjectPtr>& thatVMap, 
                                                            PartEdgePtr pedge) {
  typename map<Key, ValueObjectPtr>::const_iterator s_it;
  s_it = thatVMap.find(key);
  if(s_it == thatVMap.end()) return true;
  return valuesMap[key]->subSetV(s_it->second, pedge);
}

//! This object is a non-strict subset of the other if the set of values denoted by this
//! is a subset of the set of values denoted by that.
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed Value objects
//! If all the discharged queries come back true then this is a subset of that otherwise not.
template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::subSetV(ValueObjectPtr thatV, PartEdgePtr pedge) {
  boost::shared_ptr<MappedValueObject<Key, mostAccurate> > thatV_p = 
    boost::dynamic_pointer_cast<MappedValueObject<Key, mostAccurate> >(thatV);  
  assert(thatV_p);

  // This object denotes full set of Values (full set of executions)
  if(isFullV(pedge)) return thatV_p->isFullV(pedge);

  // denotes empty set
  // thatV could be empty or non-empty eitherway this will be a non-strict subset of that.
  if(isEmptyV(pedge)) return true;

  // If both objects have the same keys discharge
  // If this object has a key and that does not then 
  // the keyed object is subset of that (return true) implemented by subsetVWithKey
  // If any of the discharged query return false then return false.
  const map<Key, ValueObjectPtr> thatVMap = thatV_p->getValuesMap();
  typename map<Key, ValueObjectPtr>::iterator it;
  for(it = valuesMap.begin(); it != valuesMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if(subSetVWithKey(it->first, thatVMap, pedge) == false) return false;
  }

  // If this object doesn't have the key and that object has the key then 
  // return false as this object has full object mapped to the key
  typename map<Key, ValueObjectPtr>::const_iterator c_it;
  for(c_it = thatVMap.begin(); c_it != thatVMap.end() && (n_FullV != 0); ++c_it) {
    if(valuesMap.find(c_it->first) == valuesMap.end()) return false;
  }

  return true;
}


template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::isLiveV(PartEdgePtr pedge) {
  // If this object is full return the conservative answer
  if(isFullV(pedge)) return true;

  // If it has one or more full objects added to it
  // and if the object has mostAccurate=false then return true (weakest answer)
  if(n_FullV > 0 && union_) return true;

  // 1. This object may have have one or more full objects under intersection
  // 2. This object doesnt have any full objects added to it under union
  // Under both cases the answer is based on how individual analysis respond to the query
  typename map<Key, ValueObjectPtr>::iterator it = valuesMap.begin();
  for( ; it != valuesMap.end(); ++it) {
    bool isLive = it->second->isLiveV(pedge);
    if(union_ && isLive==true) return true;
    else if(intersect_ && isLive==false) return false;
  }
  
  // leftover cases
  if(union_) return false;
  else if(intersect_) return true;
  else assert(0);
}

//! meetUpdateV performs the join operation of abstractions of two value objects
template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<MappedValueObject<Key, mostAccurate> > thatV_p =
    boost::dynamic_pointer_cast<MappedValueObject<Key, mostAccurate> >(that);  
  assert(thatV_p);

  // if this object is already full
  if(isFullV(pedge)) return false;

  // If that object is full set this object to full
  if(thatV_p->isFullV(pedge)) {
    n_FullV++;
    setVToFull();
    return true;
  }

  // Both objects are not full
  const map<Key, ValueObjectPtr> thatVMap = thatV_p->getValuesMap();
  
  typename map<Key, ValueObjectPtr>::iterator it = valuesMap.begin();
  typename map<Key, ValueObjectPtr>::const_iterator s_it;   // search iterator for thatVMap

  bool modified = false;
  while(it != valuesMap.end()) {
    s_it = thatVMap.find(it->first);
    // If two objects have the same key then discharge meetUpdate to the corresponding keyed Value objects
    if(s_it != thatVMap.end()) {
      modified = (it->second)->meetUpdateV(s_it->second, pedge) || modified;
    }

    // Remove the current Value object (current iterator it) from the map if the mapepd object is full.
    // Two cases under which the current Value object can be full.
    // (1) If current key is not found in thatVMap then the mapped object
    // in thatVMap is full and the meetUpdate of the current V with that is also full.
    // (2) meetUpdateV above of the two keyed objects resulted in this mapped object being full.
    // Under both cases remove the mapped ml from this map
    if(s_it == thatVMap.end() || (it->second)->isFullV(pedge)) {
      // Current mapped Value object has become full as a result of (1) or (2).
      // Remove the item from the map.
      // Note that post-increment which increments the iterator and returns the old value for deletion.
      valuesMap.erase(it++);
      n_FullV++;
      modified = true;

      // If union then set this entire object to full and return
      if(union_) {
        setVToFull();
        return true;
      }
    }
    else ++it;
  }
  return modified;
}

//! Method that sets this mapped object to full
template<class Key, bool mostAccurate>
void MappedValueObject<Key, mostAccurate>::setVToFull() {
  assert(n_FullV > 0);
  if(valuesMap.size() > 0) valuesMap.clear();
}

template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::isFullV() {
  if(n_FullV > 0 && valuesMap.size() == 0) return true;
  return false;
}

template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::isFullV(PartEdgePtr pedge) {
  return isFullV();
}

template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::isEmptyV() {
  if(n_FullV == 0 && valuesMap.size() == 0) return true;
  return false;
}

template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::isEmptyV(PartEdgePtr pedge) {
  return isEmptyV();
}

template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::isConcrete() {

  if(isFullV()) return false;
  if(isEmptyV()) return false;
  if(union_ && n_FullV > 0) return false;
  
  typename map<Key, ValueObjectPtr>::iterator it = valuesMap.begin();
  for( ; it != valuesMap.end(); ++it) {
    bool isConc = it->second->isConcrete();
    // we have atleast one that is not concrete under union
    if(union_ && !isConc) return false;
    // we have atleast one that is concrete under intersection
    else if(intersect_ && isConc) return true;
  }

  // All the objects are concrete (return true above) union
  if(union_) return true;
  // All the objects not concrete (return false above) under intersection
  else if(intersect_) return false;
  else assert(0);
}

template<class Key, bool mostAccurate>
SgType* MappedValueObject<Key, mostAccurate>::getConcreteType() {
  assert(isConcrete());
  typename map<Key, ValueObjectPtr>::iterator it = valuesMap.begin();
  SgType* c_type = it->second->getConcreteType();
  // assert that all other objects have the same type
  for( ++it; it != valuesMap.end(); ++it) {
    SgType* votype = it->second->getConcreteType();
    assert(c_type == votype);
  }
  return c_type;
}

template<class Key, bool mostAccurate>
bool MappedValueObject<Key, mostAccurate>::find(boost::shared_ptr<SgValueExp> item,
                                                const set<boost::shared_ptr<SgValueExp> >& valueSet) {
  set<boost::shared_ptr<SgValueExp> >::const_iterator c_it = valueSet.begin();
  for( ; c_it != valueSet.end(); ++c_it) {
    // if found return immediately
    if(ValueObject::equalValueExp(c_it->get(), item.get())) return true;
  }
  // item not found
  return false;
}


template<class Key, bool mostAccurate>
set<boost::shared_ptr<SgValueExp> > MappedValueObject<Key, mostAccurate>::getConcreteValue() {
  assert(isConcrete());
  set<boost::shared_ptr<SgValueExp> > concValues;
  // Iterate through value objects in the map.
  // Get the set of values for each value object.
  // Iterate through the values set and build the set of concrete values (concValues).
  // Not very efficient as the objects to be compared are pointers.
  typename map<Key, ValueObjectPtr>::iterator v_it = valuesMap.begin();
  for( ; v_it != valuesMap.end(); ++v_it) {
    set<boost::shared_ptr<SgValueExp> > c_valueSet = v_it->second->getConcreteValue();
    set<boost::shared_ptr<SgValueExp> >::iterator s_it = c_valueSet.begin();
    for( ; s_it != c_valueSet.end(); ++s_it) {
      // if item not found add it to concValues
      if(!find(*s_it, concValues)) concValues.insert(*s_it);
    }
  }
  return concValues;
}

template<class Key, bool mostAccurate>
ValueObjectPtr MappedValueObject<Key, mostAccurate>::copyV() const {
  return boost::make_shared<MappedValueObject<Key, mostAccurate> >(*this);
}

template<class Key, bool mostAccurate>
string MappedValueObject<Key, mostAccurate>::str(string indent) const {
  ostringstream oss;
  oss << "<table border=\"1\">";
  oss << "<tr>";
  oss << "<th>" << (union_? "UnionMappedValueObject:": "IntersectMappedValueObject:") << "</th>";
  if(n_FullV > 0 && valuesMap.size() == 0) oss << "<th> Full </th> </tr>";
  else if(n_FullV == 0 && valuesMap.size() == 0) oss << "<th> Empty </th> </tr>";
  else {
    oss << "</tr>";
    typename map<Key, ValueObjectPtr>::const_iterator it = valuesMap.begin();
    for( ; it != valuesMap.end(); ++it) {
      oss << "<tr>";
      oss << "<td>" << (it->first)->str(indent) << "</td>";
      oss << "<td>" << (it->second)->str(indent) << "</td>";
      oss << "</tr>";
    }
  }
  oss << "</table>";
  return oss.str();
}

/* ############################
   # PartEdgeUnionValueObject #
   ############################ */

PartEdgeUnionValueObject::PartEdgeUnionValueObject() : 
  ValueObject(NULL) {
}

PartEdgeUnionValueObject::PartEdgeUnionValueObject(const PartEdgeUnionValueObject& thatV) :
  ValueObject(thatV), unionV_p(thatV.unionV_p->copyV()) {
}

void PartEdgeUnionValueObject::add(ValueObjectPtr v_p, PartEdgePtr pedge) {
  // If this is the very first object
  if(!unionV_p) unionV_p = v_p->copyV();  
  // If Full return without adding
  else if(isFullV(pedge)) return; 
  // Else meetUpdate with the existing unionV_p
  else unionV_p->meetUpdateV(v_p, pedge);
}

bool PartEdgeUnionValueObject::mayEqualV(ValueObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->mayEqualV(thatV_p->getUnionV(), pedge);
}

bool PartEdgeUnionValueObject::mustEqualV(ValueObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->mustEqualV(thatV_p->getUnionV(), pedge);
}

bool PartEdgeUnionValueObject::equalSetV(ValueObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->equalSetV(thatV_p->getUnionV(), pedge);
}

bool PartEdgeUnionValueObject::subSetV(ValueObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->subSetV(thatV_p->getUnionV(), pedge);
}

bool PartEdgeUnionValueObject::meetUpdateV(ValueObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->meetUpdateV(thatV_p->getUnionV(), pedge);
}
  
bool PartEdgeUnionValueObject::isLiveV(PartEdgePtr pedge) {
  return unionV_p->isLiveV(pedge);
}

bool PartEdgeUnionValueObject::isFullV(PartEdgePtr pedge) {
  return unionV_p->isFullV(pedge);
}

bool PartEdgeUnionValueObject::isEmptyV(PartEdgePtr pedge) {
  return unionV_p->isEmptyV(pedge);
}

ValueObjectPtr PartEdgeUnionValueObject::copyV() const {
  return boost::make_shared<PartEdgeUnionValueObject>(*this);
}

void PartEdgeUnionValueObject::setVToFull() {
  unionV_p = boost::make_shared<FullValueObject>();
}

bool PartEdgeUnionValueObject::isConcrete() {
  return unionV_p->isConcrete();
}

SgType* PartEdgeUnionValueObject::getConcreteType() {
  return unionV_p->getConcreteType();
}

set<boost::shared_ptr<SgValueExp> > PartEdgeUnionValueObject::getConcreteValue() {
  return unionV_p->getConcreteValue();
}

string PartEdgeUnionValueObject::str(string indent) const {
  ostringstream oss;
  oss << "[UnionV=" << unionV_p->str(indent) << "]";
  return oss.str();
}

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
  return boost::make_shared<FullValueObject>();
}

MemRegionObjectPtr FuncResultMemRegionObject::copyMR() const
{
  return boost::make_shared<FuncResultMemRegionObject>(func);
}

/* ##################################
   ##### FullMemRegionObject ##### 
   ################################## */

bool FullMemRegionObject::mayEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge) {
  return true;
}

bool FullMemRegionObject::mustEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge) {  
  return false;
}

bool FullMemRegionObject::equalSetMR(MemRegionObjectPtr o, PartEdgePtr pedge) {
  return isFullMR(pedge);
}
                                                                                                                                                                                                        
bool FullMemRegionObject::subSetMR(MemRegionObjectPtr o, PartEdgePtr pedge) {
  return isFullMR(pedge);
}
                                     
MemRegionObjectPtr  FullMemRegionObject::copyMR() const {
  return boost::make_shared<FullMemRegionObject>();
}

bool FullMemRegionObject::isLiveMR(PartEdgePtr pedge) {
  return true;
}
                                    
bool FullMemRegionObject::meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
    return false;
}

bool FullMemRegionObject::isFullMR(PartEdgePtr pedge) {
  return true;
}

bool FullMemRegionObject::isEmptyMR(PartEdgePtr pedge) {
  return false;
}

ValueObjectPtr FullMemRegionObject::getRegionSize(PartEdgePtr pedge) const {
  return boost::make_shared<FullValueObject>();
}

string FullMemRegionObject::str(string indent) const {
  return "FullMemRegionObject";
}

/* ###################################
   ##### CombinedMemRegionObject ##### 
   ################################### */

/* 
// Creates a new CombinedMemRegionObject instance of the generic CombinedMemRegionObject class.
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
*/

template <bool defaultMayEq>
void CombinedMemRegionObject<defaultMayEq>::add(MemRegionObjectPtr memReg) {
  memRegions.push_back(memReg);
}

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
      // SA 2014/6/13: This will probably break if the ValueObjects stored in this collection are different
      res->meetUpdateV((*mr)->getRegionSize(pedge), pedge);
  }
  return res;
}

template <bool defaultMayEq>
std::string CombinedMemRegionObject<defaultMayEq>::str(std::string indent) const
{
  ostringstream oss;
  if(memRegions.size()>1) oss << "["<<(defaultMayEq? "UnionMR" : "IntersectMR")<<": ";
  if(memRegions.size()>1) oss << endl;
  for(list<MemRegionObjectPtr>::const_iterator mr=memRegions.begin(); mr!=memRegions.end(); ) {
    if(mr!=memRegions.begin()) oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
    oss << (*mr)->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;");
    mr++;
    if(mr!=memRegions.end()) oss << endl;
  }
  if(memRegions.size()>1) oss << "]";
  
  return oss.str();
}

/* #################################
   ##### MappedMemRegionObject #####
   ################################# */

//! Method to add mrs to the map.
//! MRs that are full are never added to the map.
//! If mr_p is FullMR or mr_p->isFullMR=true then mapped MR is set to full only if mostAccurate=false.
template<class Key, bool mostAccurate>
void MappedMemRegionObject<Key, mostAccurate>::add(Key key, MemRegionObjectPtr mr_p, PartEdgePtr pedge) {
  // If the object is already full don't add anything
  if(union_ && isFullMR(pedge)) return;

  // If the mr_p is not full add/update the map
  if(!mr_p->isFullMR(pedge)) {
    memRegionsMap[key] = mr_p;
  }
  else {
    n_FullMR++;
    if(union_) setMRToFull();
  }
}

template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::mayEqualMRWithKey(Key key,
                                                              const map<Key, MemRegionObjectPtr>& thatMRMap, 
                                                              PartEdgePtr pedge) {
  typename map<Key, MemRegionObjectPtr>::const_iterator s_it;
  s_it = thatMRMap.find(key);
  if(s_it == thatMRMap.end()) return true;
  return memRegionsMap[key]->mayEqualMR(s_it->second, pedge);
}

//! Two MR objects are may equals if there is atleast one execution or sub-exectuion
//! in which they represent the same memory location.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described 
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise, the conservative answer for mayEqualMR is true.
//! Mapped MRs are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed MR object correspond to some dataflow facts computed by Key=Analysis* or 
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MayEquality check on mapped MR is performed on intersection of sub-executions
//! or union of sub-executions over the keyed MR objects. 
template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::mayEqualMR(MemRegionObjectPtr thatMR, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemRegionObject<Key, mostAccurate> > thatMR_p = 
    boost::dynamic_pointer_cast<MappedMemRegionObject<Key, mostAccurate> >(thatMR);
  assert(thatMR_p);

  // This object denotes full set of MR (full set of executions)
  if(isFullMR(pedge)) return true;

  // denotes empty set
  if(isEmptyMR(pedge)) return false;

  // presence of one more full objects will result in full set over union
  if(union_ && n_FullMR > 0) return true;

  // Two cases reach here [1] union_=true && nFull_MR=0 [2] intersect=true && nFullMR=0 or nFull_MR!=0.
  // For both cases iterate on the MR map and discharge the mayEqualMR query to individual objects 
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, MemRegionObjectPtr> thatMRMap = thatMR_p->getMemRegionsMap();
  typename map<Key, MemRegionObjectPtr>::iterator it;
  for(it = memRegionsMap.begin(); it != memRegionsMap.end(); ++it) {
    // discharge query
    bool isMayEq = mayEqualMRWithKey(it->first, thatMRMap, pedge);

    // 1. Union of sub-executions and the object does not contain any full objects.
    // If the discharged query comes back as true for this case then we have found atleast one execution
    // under which the two objects are same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If false we iterate further as any MR can add more executions under which the objects are may equals.
    if(union_ && isMayEq==true) return true;

    // 2. Intersection of sub-executions and the object may contain full objects (n_FullMR != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns false then return false.
    // We did not find one execution in which the two objects are may equals. 
    // Note that set of executions are contained over keyed objects (analyses are conservative).
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns true iterate further as some executions corresponding to true may be dropped.
    else if(intersect_ && isMayEq==false) return false;
  }


  // All the keyed objects returned false for the discharged query under union.
  // We haven't found a single execution under which the two objects are may equals.
  if(union_) return false;
  // All the keyed objects returned true for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are may equals.
  else if(intersect_) return true;
  else assert(0);
}

template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::mustEqualMRWithKey(Key key,
                                                               const map<Key, MemRegionObjectPtr>& thatMRMap, 
                                                               PartEdgePtr pedge) {
  typename map<Key, MemRegionObjectPtr>::const_iterator s_it;
  s_it = thatMRMap.find(key);
  if(s_it == thatMRMap.end()) return false;
  return memRegionsMap[key]->mustEqualMR(s_it->second, pedge);
}

//! Two MR objects are must equals if they represent the same single memory 
//! location on all executions.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described 
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise conservative answer for mustEqualMR is false.
//! Mapped MRs are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed MR object correspond to some dataflow facts computed by Key=Analysis* or 
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MustEquality check on mapped MR is performed on intersection (mostAccurate=true) of sub-executions
//! or union (mostAccurate=false) of sub-executions over the keyed MR objects. 
template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::mustEqualMR(MemRegionObjectPtr thatMR, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemRegionObject<Key, mostAccurate> > thatMR_p = 
    boost::dynamic_pointer_cast<MappedMemRegionObject<Key, mostAccurate> >(thatMR);
  assert(thatMR_p);

  // This object denotes full set of MR (full set of executions)
  if(isFullMR(pedge)) return false;

  // denotes empty set
  if(isEmptyMR(pedge)) return false;

  // presence of one more full objects will result in full set over union
  if(union_ && n_FullMR > 0) return true;

  // Two cases reach here [1] union_=true && nFull_MR=0 [2] intersect=true && nFullMR=0 or nFull_MR!=0.
  // For both cases iterate on the MR map and discharge the mayEqualMR query to individual objects 
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, MemRegionObjectPtr> thatMRMap = thatMR_p->getMemRegionsMap();
  typename map<Key, MemRegionObjectPtr>::iterator it;
  for(it = memRegionsMap.begin(); it != memRegionsMap.end(); ++it) {
    // discharge query
    bool isMustEq = mustEqualMRWithKey(it->first, thatMRMap, pedge);

    // 1. Union of sub-executions and the object does not contain any full objects
    // If the discharged query comes back as false for this case then we have found atleast one execution
    // under which the two objects are not same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If it returns true we iterate further as any MR can add more executions under which the objects are not must equals.
    if(union_ && isMustEq==false) return false;

    // 2. Intersection of sub-executions and the object may contain full objects (n_FullMR != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns true then return true. 
    // Under all sub-executions (corresponding to the MR) the two objects must equal.
    // Note that set of executions are contained over keyed objects as the analyses are conservative.
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns false iterate further as some executions corresponding to false may be dropped.
    else if(intersect_ && isMustEq==true) return true;
  }

  // All the keyed objects returned true for the discharged query under union.
  // We haven't found a single execution under which the two objects are not equal.
  if(union_) return true;
  // All the keyed objects returned false for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are not equal.
  else if(intersect_) return false;
  else assert(0);
}

//! Discharge the query to the corresponding MR
//! If key not found in thatMRMap return false
template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::equalSetMRWithKey(Key key,
                                                              const map<Key, MemRegionObjectPtr>& thatMRMap, 
                                                              PartEdgePtr pedge) {
  typename map<Key, MemRegionObjectPtr>::const_iterator s_it;
  s_it = thatMRMap.find(key);
  if(s_it == thatMRMap.end()) return false;
  return memRegionsMap[key]->equalSetMR(s_it->second, pedge);
}

//! Two objects are equal sets if they denote the same set of memory locations
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed MemRegion objects
//! If all the discharged queries come back equal then the two objects are equal otherwise not.
template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::equalSetMR(MemRegionObjectPtr thatMR, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemRegionObject<Key, mostAccurate> > thatMR_p = 
    boost::dynamic_pointer_cast<MappedMemRegionObject<Key, mostAccurate> >(thatMR);  
  assert(thatMR_p);

  // This object denotes full set of MR (full set of executions)
  if(isFullMR(pedge)) return thatMR_p->isFullMR(pedge);

  // denotes empty set
  if(isEmptyMR(pedge)) return thatMR_p->isEmptyMR(pedge);

  const map<Key, MemRegionObjectPtr> thatMRMap = thatMR_p->getMemRegionsMap();
  typename map<Key, MemRegionObjectPtr>::iterator it;
  for(it = memRegionsMap.begin(); it != memRegionsMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if(equalSetMRWithKey(it->first, thatMRMap, pedge) == false) return false;
  }

  return true;
}

//! Discharge the query to the corresponding MR
//! If key not found in thatMRMap return true as the
//! keyed object on thatMRMap denotes full set
template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::subSetMRWithKey(Key key,
                                                            const map<Key, MemRegionObjectPtr>& thatMRMap, 
                                                            PartEdgePtr pedge) {
  typename map<Key, MemRegionObjectPtr>::const_iterator s_it;
  s_it = thatMRMap.find(key);
  if(s_it == thatMRMap.end()) return true;
  return memRegionsMap[key]->subSetMR(s_it->second, pedge);
}

//! This object is a non-strict subset of the other if the set of memory locations denoted by this
//! is a subset of the set of memory locations denoted by that.
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed MemRegion objects
//! If all the discharged queries come back true then this is a subset of that otherwise not.
template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::subSetMR(MemRegionObjectPtr thatMR, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemRegionObject<Key, mostAccurate> > thatMR_p = 
    boost::dynamic_pointer_cast<MappedMemRegionObject<Key, mostAccurate> >(thatMR);  
  assert(thatMR_p);

  // This object denotes full set of MR (full set of executions)
  if(isFullMR(pedge)) return thatMR_p->isFullMR(pedge);

  // denotes empty set
  // thatMR could be empty or non-empty eitherway this will be a non-strict subset of that.
  if(isEmptyMR(pedge)) return true;

  // If both objects have the same keys discharge
  // If this object has a key and that does not then 
  // the keyed object is subset of that (return true) implemented by subsetMRWithKey
  // If any of the discharged query return false then return false.
  const map<Key, MemRegionObjectPtr> thatMRMap = thatMR_p->getMemRegionsMap();
  typename map<Key, MemRegionObjectPtr>::iterator it;
  for(it = memRegionsMap.begin(); it != memRegionsMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if(subSetMRWithKey(it->first, thatMRMap, pedge) == false) return false;
  }

  // If this object doesn't have the key and that object has the key then 
  // return false as this object has full object mapped to the key
  typename map<Key, MemRegionObjectPtr>::const_iterator c_it;
  for(c_it = thatMRMap.begin(); c_it != thatMRMap.end() && (n_FullMR != 0); ++c_it) {
    if(memRegionsMap.find(c_it->first) == memRegionsMap.end()) return false;
  }

  return true;
}

//! Mapped object liveness is determined based on finding executions
//! in which it may be live.
//! It can be answered based on union (mostAccurate=false) or intersection
//! (mostAccurate=true) of executions
//! The conservative answer is to assume that the object is live
template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::isLiveMR(PartEdgePtr pedge) {
  // If this object is full return the conservative answer
  if(isFullMR(pedge)) return true;

  // If it has one or more full objects added to it
  // and if the object has mostAccurate=false then return true (weakest answer)
  if(n_FullMR > 0 && union_) return true;

  // 1. This object may have have one or more full objects under intersection
  // 2. This object doesnt have any full objects added to it under union
  // Under both cases the answer is based on how individual analysis respond to the query
  typename map<Key, MemRegionObjectPtr>::iterator it = memRegionsMap.begin();
  for( ; it != memRegionsMap.end(); ++it) {
    bool isLive = it->second->isLiveMR(pedge);
    if(union_ && isLive==true) return true;
    else if(intersect_ && isLive==false) return false;
  }
  
  // leftover cases
  if(union_) return false;
  else if(intersect_) return true;
  else assert(0);
}

//! meetUpdateMR performs the join operation of abstractions of two mls
template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemRegionObject<Key, mostAccurate> > thatMR_p =
    boost::dynamic_pointer_cast<MappedMemRegionObject<Key, mostAccurate> >(that);  
  assert(thatMR_p);

  // if this object is already full
  if(isFullMR(pedge)) return false;

  // If that object is full set this object to full
  if(thatMR_p->isFullMR(pedge)) {
    n_FullMR++;
    setMRToFull();
    return true;
  }

  // Both objects are not full
  const map<Key, MemRegionObjectPtr> thatMRMap = thatMR_p->getMemRegionsMap();
  
  typename map<Key, MemRegionObjectPtr>::iterator it = memRegionsMap.begin();
  typename map<Key, MemRegionObjectPtr>::const_iterator s_it;   // search iterator for thatMRMap

  bool modified = false;
  while(it != memRegionsMap.end()) {
    s_it = thatMRMap.find(it->first);
    // If two objects have the same key then discharge meetUpdate to the corresponding keyed MR objects
    if(s_it != thatMRMap.end()) {
      modified = (it->second)->meetUpdateMR(s_it->second, pedge) || modified;
    }

    // Remove the current MR object (current iterator it) from the map if the mapepd object is full.
    // Two cases under which the current MR object can be full.
    // (1) If current key is not found in thatMRMap then the mapped object
    // in thatMRMap is full and the meetUpdate of the current MR with that is also full.
    // (2) meetUpdateMR above of the two keyed objects resulted in this mapped object being full.
    // Under both cases remove the mapped ml from this map
    if(s_it == thatMRMap.end() || (it->second)->isFullMR(pedge)) {
      // Current mapped MR has become full as a result of (1) or (2).
      // Remove the item from the map.
      // Note that post-increment which increments the iterator and returns the old value for deletion.
      memRegionsMap.erase(it++);
      n_FullMR++;
      modified = true;

      // If union then set this entire object to full and return
      if(union_) {
        setMRToFull();
        return true;
      }
    }
    else ++it;
  }
  return modified;
}

//! Method that sets this mapped object to full
template<class Key, bool mostAccurate>
void MappedMemRegionObject<Key, mostAccurate>::setMRToFull() {
  assert(n_FullMR > 0);
  if(memRegionsMap.size() > 0) memRegionsMap.clear();
}

template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::isFullMR(PartEdgePtr pedge) {
  if(n_FullMR > 0 && memRegionsMap.size() == 0) return true;
  return false;
}

template<class Key, bool mostAccurate>
bool MappedMemRegionObject<Key, mostAccurate>::isEmptyMR(PartEdgePtr pedge) {
  if(n_FullMR == 0 && memRegionsMap.size() == 0) return true;
  return false;
}

//! Size of the memory region denoted by this memory object represented by a ValueObject
//! Useful only if the object is not full
template<class Key, bool mostAccurate>
ValueObjectPtr MappedMemRegionObject<Key, mostAccurate>::getRegionSize(PartEdgePtr pedge) const {
  
  // Assert for atleast one element in the map
  // Should we handle full MR by returning FullValueObject?
  assert(memRegionsMap.size() > 0);

  // getRegionSize on each object returns different ValueObject for each key
  // We cannot do meetUpdate as the objects are from different analysis
  // Return a MappedValueObject based on those objects and the corresponding key
  boost::shared_ptr<MappedValueObject<Key, mostAccurate> > mvo_p = boost::make_shared<MappedValueObject<Key, mostAccurate> >();
  typename map<Key, MemRegionObjectPtr>::const_iterator it = memRegionsMap.begin();
  for( ; it != memRegionsMap.end(); ++it) {
    ValueObjectPtr vo_p = it->second->getRegionSize(pedge);
    Key k = it->first;    
    mvo_p->add(k, vo_p, pedge);
  }

  return mvo_p;
}

template<class Key, bool mostAccurate>
MemRegionObjectPtr MappedMemRegionObject<Key, mostAccurate>::copyMR() const {
  return boost::make_shared<MappedMemRegionObject<Key, mostAccurate> >(*this);
}

template<class Key, bool mostAccurate>
string MappedMemRegionObject<Key, mostAccurate>::str(string indent) const {
  ostringstream oss;
  oss << "<table border=\"1\">";
  oss << "<tr>";
  oss << "<th>" << (union_? "UnionMappedMemRegionObject:": "IntersectMappedMemRegionObject:") << "</th>";
  if(n_FullMR > 0 && memRegionsMap.size() == 0) oss << "<th> Full </th> </tr>";
  else if(n_FullMR == 0 && memRegionsMap.size() == 0) oss << "<th> Empty </th> </tr>";
  else {
    oss << "</tr>";
    typename map<Key, MemRegionObjectPtr>::const_iterator it = memRegionsMap.begin();
    for( ; it != memRegionsMap.end(); ++it) {
      oss << "<tr>";
      oss << "<td>" << (it->first)->str(indent) << "</td>";
      oss << "<td>" << (it->second)->str(indent) << "</td>";
      oss << "</tr>";
    }
  }
  oss << "</table>";
  return oss.str();
}

/* ################################
   # PartEdgeUnionMemRegionObject #
   ################################ */

PartEdgeUnionMemRegionObject::PartEdgeUnionMemRegionObject() : 
  MemRegionObject(NULL) {
}

PartEdgeUnionMemRegionObject::PartEdgeUnionMemRegionObject(const PartEdgeUnionMemRegionObject& thatMR) :
  MemRegionObject(thatMR), unionMR_p(thatMR.unionMR_p->copyMR()) {
}

void PartEdgeUnionMemRegionObject::add(MemRegionObjectPtr mr_p, PartEdgePtr pedge) {
  // If this is the very first object
  if(!unionMR_p) unionMR_p = mr_p->copyMR();  
  // If Full return without adding
  else if(isFullMR(pedge)) return; 
  // Else meetUpdate with the existing unionMR_p
  else unionMR_p->meetUpdateMR(mr_p, pedge);
}

bool PartEdgeUnionMemRegionObject::mayEqualMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->mayEqualMR(thatMR_p->getUnionMR(), pedge);
}

bool PartEdgeUnionMemRegionObject::mustEqualMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->mustEqualMR(thatMR_p->getUnionMR(), pedge);
}

bool PartEdgeUnionMemRegionObject::equalSetMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->equalSetMR(thatMR_p->getUnionMR(), pedge);
}

bool PartEdgeUnionMemRegionObject::subSetMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->subSetMR(thatMR_p->getUnionMR(), pedge);
}

bool PartEdgeUnionMemRegionObject::meetUpdateMR(MemRegionObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->meetUpdateMR(thatMR_p->getUnionMR(), pedge);
}
  
bool PartEdgeUnionMemRegionObject::isLiveMR(PartEdgePtr pedge) {
  return unionMR_p->isLiveMR(pedge);
}

bool PartEdgeUnionMemRegionObject::isFullMR(PartEdgePtr pedge) {
  return unionMR_p->isFullMR(pedge);
}

bool PartEdgeUnionMemRegionObject::isEmptyMR(PartEdgePtr pedge) {
  return unionMR_p->isEmptyMR(pedge);
}

MemRegionObjectPtr PartEdgeUnionMemRegionObject::copyMR() const {
  return boost::make_shared<PartEdgeUnionMemRegionObject>(*this);
}

void PartEdgeUnionMemRegionObject::setMRToFull() {
  unionMR_p = boost::make_shared<FullMemRegionObject>();
}

ValueObjectPtr PartEdgeUnionMemRegionObject::getRegionSize(PartEdgePtr pedge) const {
  return unionMR_p->getRegionSize(pedge);
}

string PartEdgeUnionMemRegionObject::str(string indent) const {
  ostringstream oss;
  oss << "[UnionMR=" << unionMR_p.get() << ", " << unionMR_p->str(indent) << "]";
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


std::string MemLocObject::str(std::string indent) const { // pretty print for the object
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

/* #############################
   #### FullMemLocObject ####
   ############################# */

bool FullMemLocObject::mayEqualML(MemLocObjectPtr o, PartEdgePtr pedge) {
  return true;
}

bool FullMemLocObject::mustEqualML(MemLocObjectPtr o, PartEdgePtr pedge) {
  return false;
}
  
bool FullMemLocObject::equalSetML(MemLocObjectPtr o, PartEdgePtr pedge) {
  return isFullML(pedge);
}
  
bool FullMemLocObject::subSetML(MemLocObjectPtr o, PartEdgePtr pedge) {
  return isFullML(pedge);
}
  
MemLocObjectPtr FullMemLocObject::copyML() const {
  return boost::make_shared<FullMemLocObject>();
}
  
bool FullMemLocObject::isLiveML(PartEdgePtr pedge) {
  return true;
}
  
bool FullMemLocObject::meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge) {
  return false;
}
  
bool FullMemLocObject::isFullML(PartEdgePtr pedge) {
  return true;
}

bool FullMemLocObject::isEmptyML(PartEdgePtr pedge) {
  return false;
}
  
string FullMemLocObject::str(string indent) const {
  return "FullMemLocObject";
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
std::string CombinedMemLocObject<defaultMayEq>::str(std::string indent) const
{
  ostringstream oss;
  if(memLocs.size()>1) oss << "["<<(defaultMayEq? "UnionML" : "IntersectML")<<": ";
  if(memLocs.size()>1) oss << endl;
  for(list<MemLocObjectPtr>::const_iterator ml=memLocs.begin(); ml!=memLocs.end(); ) {
    if(ml!=memLocs.begin()) oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
    oss << (*ml)->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;");
    ml++;
    if(ml!=memLocs.end()) oss << endl;
  }
  if(memLocs.size()>1) oss << "]";
  
  return oss.str();
}

/* ##############################
   ##### MappedMemLocObject #####
   ############################## */

//! Method to add mls to the map.
//! MLs that are full are never added to the map.
//! If ml_p is FullML or ml_p->isFullML=true then mapped ML is set to full only if mostAccurate=false.
template<class Key, bool mostAccurate>
void MappedMemLocObject<Key, mostAccurate>::add(Key key, MemLocObjectPtr ml_p, PartEdgePtr pedge) {
  // If the object is already full don't add anything
  if(union_ && isFullML(pedge)) return;

  // If the ml_p is not full add/update the map
  if(!ml_p->isFullML(pedge)) {
    memLocsMap[key] = ml_p;
  }
  else {
    n_FullML++;
    if(union_) setMLToFull();
  }
}

template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::mayEqualMLWithKey(Key key,
                                                              const map<Key, MemLocObjectPtr>& thatMLMap, 
                                                              PartEdgePtr pedge) {
  typename map<Key, MemLocObjectPtr>::const_iterator s_it;
  s_it = thatMLMap.find(key);
  if(s_it == thatMLMap.end()) return true;
  return memLocsMap[key]->mayEqualML(s_it->second, pedge);
}

//! Two ML objects are may equals if there is atleast one execution or sub-exectuion
//! in which they represent the same memory location.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described 
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise, the conservative answer for mayEqualML is true.
//! Mapped MLs are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed ML object correspond to some dataflow facts computed by Key=Analysis* or 
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MayEquality check on mapped ML is performed on intersection of sub-executions
//! or union of sub-executions over the keyed ML objects. 
template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::mayEqualML(MemLocObjectPtr thatML, PartEdgePtr pedge) {
  // scope s(txt()<<"MappedMemLocObject::mayEqualML", scope::medium);
  boost::shared_ptr<MappedMemLocObject<Key, mostAccurate> > thatML_p = 
    boost::dynamic_pointer_cast<MappedMemLocObject<Key, mostAccurate> >(thatML);
  assert(thatML_p);

  // dbg << "thisML=" << str() << endl;
  // dbg << "thatML=" << thatML_p->str() << endl;

  // This object denotes full set of ML (full set of executions)
  if(isFullML(pedge)) return true;

  // denotes empty set
  if(isEmptyML(pedge)) return false;

  // presence of one more full objects will result in full set over union
  if(union_ && n_FullML > 0) return true;

  // Two cases reach here [1] union_=true && nFull_ML=0 [2] intersect=true && nFullML=0 or nFull_ML!=0.
  // For both cases iterate on the ML map and discharge the mayEqualML query to individual objects 
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, MemLocObjectPtr> thatMLMap = thatML_p->getMemLocsMap();
  typename map<Key, MemLocObjectPtr>::iterator it;
  for(it = memLocsMap.begin(); it != memLocsMap.end(); ++it) {
    // discharge query
    bool isMayEq = mayEqualMLWithKey(it->first, thatMLMap, pedge);
    
    // dbg << "key=" << (it->first)->str() << ", isMayEq=" << isMayEq << endl;

    // 1. Union of sub-executions and the object does not contain any full objects.
    // If the discharged query comes back as true for this case then we have found atleast one execution
    // under which the two objects are same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If false we iterate further as any ML can add more executions under which the objects are may equals.
    if(union_ && isMayEq==true) return true;

    // 2. Intersection of sub-executions and the object may contain full objects (n_FullML != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns false then return false.
    // We did not find one execution in which the two objects are may equals. 
    // Note that set of executions are contained over keyed objects (analyses are conservative).
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns true iterate further as some executions corresponding to true may be dropped.
    else if(intersect_ && isMayEq==false) return false;
  }


  // All the keyed objects returned false for the discharged query under union.
  // We haven't found a single execution under which the two objects are may equals.
  if(union_) return false;
  // All the keyed objects returned true for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are may equals.
  else if(intersect_) return true;
  else assert(0);
}

template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::mustEqualMLWithKey(Key key,
                                                               const map<Key, MemLocObjectPtr>& thatMLMap, 
                                                               PartEdgePtr pedge) {
  typename map<Key, MemLocObjectPtr>::const_iterator s_it;
  s_it = thatMLMap.find(key);
  if(s_it == thatMLMap.end()) return false;
  return memLocsMap[key]->mustEqualML(s_it->second, pedge);
}

//! Two ML objects are must equals if they represent the same single memory 
//! location on all executions.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described 
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise conservative answer for mustEqualML is false.
//! Mapped MLs are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed ML object correspond to some dataflow facts computed by Key=Analysis* or 
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MustEquality check on mapped ML is performed on intersection (mostAccurate=true) of sub-executions
//! or union (mostAccurate=false) of sub-executions over the keyed ML objects. 
template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::mustEqualML(MemLocObjectPtr thatML, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemLocObject<Key, mostAccurate> > thatML_p = 
    boost::dynamic_pointer_cast<MappedMemLocObject<Key, mostAccurate> >(thatML);
  assert(thatML_p);

  // This object denotes full set of ML (full set of executions)
  if(isFullML(pedge)) return false;

  // denotes empty set
  if(isEmptyML(pedge)) return false;

  // presence of one more full objects will result in full set over union
  if(union_ && n_FullML > 0) return true;

  // Two cases reach here [1] union_=true && nFull_ML=0 [2] intersect=true && nFullML=0 or nFull_ML!=0.
  // For both cases iterate on the ML map and discharge the mayEqualML query to individual objects 
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, MemLocObjectPtr> thatMLMap = thatML_p->getMemLocsMap();
  typename map<Key, MemLocObjectPtr>::iterator it;
  for(it = memLocsMap.begin(); it != memLocsMap.end(); ++it) {
    // discharge query
    bool isMustEq = mustEqualMLWithKey(it->first, thatMLMap, pedge);

    // 1. Union of sub-executions and the object does not contain any full objects
    // If the discharged query comes back as false for this case then we have found atleast one execution
    // under which the two objects are not same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If it returns true we iterate further as any ML can add more executions under which the objects are not must equals.
    if(union_ && isMustEq==false) return false;

    // 2. Intersection of sub-executions and the object may contain full objects (n_FullML != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns true then return true. 
    // Under all sub-executions (corresponding to the ML) the two objects must equal.
    // Note that set of executions are contained over keyed objects as the analyses are conservative.
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns false iterate further as some executions corresponding to false may be dropped.
    else if(intersect_ && isMustEq==true) return true;
  }

  // All the keyed objects returned true for the discharged query under union.
  // We haven't found a single execution under which the two objects are not equal.
  if(union_) return true;
  // All the keyed objects returned false for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are not equal.
  else if(intersect_) return false;
  else assert(0);
}


//! Discharge the query to the corresponding ML
//! If key not found in thatMLMap return false
template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::equalSetMLWithKey(Key key,
                                                              const map<Key, MemLocObjectPtr>& thatMLMap, 
                                                              PartEdgePtr pedge) {
  typename map<Key, MemLocObjectPtr>::const_iterator s_it;
  s_it = thatMLMap.find(key);
  if(s_it == thatMLMap.end()) return false;
  return memLocsMap[key]->equalSetML(s_it->second, pedge);
}

//! Two objects are equal sets if they denote the same set of memory locations
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed MemLoc objects
//! If all the discharged queries come back equal then the two objects are equal otherwise not.
template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::equalSetML(MemLocObjectPtr thatML, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemLocObject<Key, mostAccurate> > thatML_p = 
    boost::dynamic_pointer_cast<MappedMemLocObject<Key, mostAccurate> >(thatML);  
  assert(thatML_p);

  // This object denotes full set of ML (full set of executions)
  if(isFullML(pedge)) return thatML_p->isFullML(pedge);

  // denotes empty set
  if(isEmptyML(pedge)) return thatML_p->isEmptyML(pedge);

  const map<Key, MemLocObjectPtr> thatMLMap = thatML_p->getMemLocsMap();
  typename map<Key, MemLocObjectPtr>::iterator it;
  for(it = memLocsMap.begin(); it != memLocsMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if(equalSetMLWithKey(it->first, thatMLMap, pedge) == false) return false;
  }

  return true;
}

//! Discharge the query to the corresponding ML
//! If key not found in thatMLMap return true as the
//! keyed object on thatMLMap denotes full set
template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::subSetMLWithKey(Key key,
                                                            const map<Key, MemLocObjectPtr>& thatMLMap, 
                                                            PartEdgePtr pedge) {
  typename map<Key, MemLocObjectPtr>::const_iterator s_it;
  s_it = thatMLMap.find(key);
  if(s_it == thatMLMap.end()) return true;
  return memLocsMap[key]->subSetML(s_it->second, pedge);
}

//! This object is a non-strict subset of the other if the set of memory locations denoted by this
//! is a subset of the set of memory locations denoted by that.
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed MemLoc objects
//! If all the discharged queries come back true then this is a subset of that otherwise not.
template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::subSetML(MemLocObjectPtr thatML, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemLocObject<Key, mostAccurate> > thatML_p = 
    boost::dynamic_pointer_cast<MappedMemLocObject<Key, mostAccurate> >(thatML);  
  assert(thatML_p);

  // This object denotes full set of ML (full set of executions)
  if(isFullML(pedge)) return thatML_p->isFullML(pedge);

  // denotes empty set
  // thatML could be empty or non-empty eitherway this will be a non-strict subset of that.
  if(isEmptyML(pedge)) return true;

  // If both objects have the same keys discharge
  // If this object has a key and that does not then 
  // the keyed object is subset of that (return true) implemented by subsetMLWithKey
  // If any of the discharged query return false then return false.
  const map<Key, MemLocObjectPtr> thatMLMap = thatML_p->getMemLocsMap();
  typename map<Key, MemLocObjectPtr>::iterator it;
  for(it = memLocsMap.begin(); it != memLocsMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if(subSetMLWithKey(it->first, thatMLMap, pedge) == false) return false;
  }

  // If this object doesn't have the key and that object has the key then 
  // return false as this object has full object mapped to the key
  typename map<Key, MemLocObjectPtr>::const_iterator c_it;
  for(c_it = thatMLMap.begin(); c_it != thatMLMap.end() && (n_FullML != 0); ++c_it) {
    if(memLocsMap.find(c_it->first) == memLocsMap.end()) return false;
  }

  return true;
}

//! Mapped object liveness is determined based on finding executions
//! in which it may be live.
//! It can be answered based on union (mostAccurate=false) or intersection
//! (mostAccurate=true) of executions
//! The conservative answer is to assume that the object is live
template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::isLiveML(PartEdgePtr pedge) {
  // If this object is full return the conservative answer
  if(isFullML(pedge)) return true;

  // If it has one or more full objects added to it
  // and if the object has mostAccurate=false then return true (weakest answer)
  if(n_FullML > 0 && union_) return true;

  // 1. This object may have have one or more full objects under intersection
  // 2. This object doesnt have any full objects added to it under union
  // Under both cases the answer is based on how individual analysis respond to the query
  typename map<Key, MemLocObjectPtr>::iterator it = memLocsMap.begin();
  for( ; it != memLocsMap.end(); ++it) {
    bool isLive = it->second->isLiveML(pedge);
    if(union_ && isLive==true) return true;
    else if(intersect_ && isLive==false) return false;
  }
  
  // leftover cases
  if(union_) return false;
  else if(intersect_) return true;
  else assert(0);
}

//! meetUpdateML performs the join operation of abstractions of two mls
template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<MappedMemLocObject<Key, mostAccurate> > thatML_p =
    boost::dynamic_pointer_cast<MappedMemLocObject<Key, mostAccurate> >(that);  
  assert(thatML_p);

  // if this object is already full
  if(isFullML(pedge)) return false;

  // If that object is full set this object to full
  if(thatML_p->isFullML(pedge)) {
    n_FullML++;
    setMLToFull();
    return true;
  }

  // Both objects are not full
  const map<Key, MemLocObjectPtr> thatMLMap = thatML_p->getMemLocsMap();
  
  typename map<Key, MemLocObjectPtr>::iterator it = memLocsMap.begin();
  typename map<Key, MemLocObjectPtr>::const_iterator s_it;   // search iterator for thatMLMap

  bool modified = false;
  while(it != memLocsMap.end()) {
    s_it = thatMLMap.find(it->first);
    // If two objects have the same key then discharge meetUpdate to the corresponding keyed ML objects
    if(s_it != thatMLMap.end()) {
      modified = (it->second)->meetUpdateML(s_it->second, pedge) || modified;
    }

    // Remove the current ML object (current iterator it) from the map if the mapepd object is full.
    // Two cases under which the current ML object can be full.
    // (1) If current key is not found in thatMLMap then the mapped object
    // in thatMLMap is full and the meetUpdate of the current ML with that is also full.
    // (2) meetUpdateML above of the two keyed objects resulted in this mapped object being full.
    // Under both cases remove the mapped ml from this map
    if(s_it == thatMLMap.end() || (it->second)->isFullML(pedge)) {
      // Current mapped ML has become full as a result of (1) or (2).
      // Remove the item from the map.
      // Note that post-increment which increments the iterator and returns the old value for deletion.
      memLocsMap.erase(it++);
      n_FullML++;
      modified = true;

      // If union then set this entire object to full and return
      if(union_) {
        setMLToFull();
        return true;
      }
    }
    else ++it;
  }
  return modified;
}

//! Method that sets this mapped object to full
template<class Key, bool mostAccurate>
void MappedMemLocObject<Key, mostAccurate>::setMLToFull() {
  assert(n_FullML > 0);
  if(memLocsMap.size() > 0) memLocsMap.clear();
}

template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::isFullML(PartEdgePtr pedge) {
  if(n_FullML > 0 && memLocsMap.size() == 0) return true;
  return false;
}

template<class Key, bool mostAccurate>
bool MappedMemLocObject<Key, mostAccurate>::isEmptyML(PartEdgePtr pedge) {
  if(n_FullML == 0 && memLocsMap.size() == 0) return true;
  return false;
}

template<class Key, bool mostAccurate>
MemLocObjectPtr MappedMemLocObject<Key, mostAccurate>::copyML() const {
  return boost::make_shared<MappedMemLocObject<Key, mostAccurate> >(*this);
}

template<class Key, bool mostAccurate>
string MappedMemLocObject<Key, mostAccurate>::str(string indent) const {
  ostringstream oss;
  oss << "<table border=\"1\">";
  oss << "<tr>";
  oss << "<th>" << (union_? "UnionMappedML:": "IntersectMappedML:") << "</th>";
  if(n_FullML > 0 && memLocsMap.size() == 0) oss << "<th> Full </th> </tr>";
  else if(n_FullML == 0 && memLocsMap.size() == 0) oss << "<th> Empty </th> </tr>";
  else {
    oss << "</tr>";
    typename map<Key, MemLocObjectPtr>::const_iterator it = memLocsMap.begin();
    for( ; it != memLocsMap.end(); ++it) {
      oss << "<tr>";
      oss << "<td>" << (it->first)->str(indent) << "</td>";
      oss << "<td>" << (it->second)->str(indent) << "</td>";
      oss << "</tr>";
    }
  }
  oss << "</table>";
  return oss.str();
}

/* #############################
   # PartEdgeUnionMemLocObject #
   ############################# */

PartEdgeUnionMemLocObject::PartEdgeUnionMemLocObject() : 
  MemLocObject(NULL) {
}

PartEdgeUnionMemLocObject::PartEdgeUnionMemLocObject(const PartEdgeUnionMemLocObject& thatML) :
  MemLocObject(thatML), unionML_p(thatML.unionML_p->copyML()) {
}

void PartEdgeUnionMemLocObject::add(MemLocObjectPtr ml_p, PartEdgePtr pedge) {
  // If this is the very first object
  if(!unionML_p) unionML_p = ml_p->copyML();  
  // If Full return without adding
  else if(isFullML(pedge)) return; 
  // Else meetUpdate with the existing unionML_p
  else unionML_p->meetUpdateML(ml_p, pedge);
}

bool PartEdgeUnionMemLocObject::mayEqualML(MemLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  return unionML_p->mayEqualML(thatML_p->getUnionML(), pedge);
}

bool PartEdgeUnionMemLocObject::mustEqualML(MemLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  return unionML_p->mustEqualML(thatML_p->getUnionML(), pedge);
}

bool PartEdgeUnionMemLocObject::equalSetML(MemLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  return unionML_p->equalSetML(thatML_p->getUnionML(), pedge);
}

bool PartEdgeUnionMemLocObject::subSetML(MemLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  return unionML_p->subSetML(thatML_p->getUnionML(), pedge);
}

bool PartEdgeUnionMemLocObject::meetUpdateML(MemLocObjectPtr that, PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p = 
    boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  return unionML_p->meetUpdateML(thatML_p->getUnionML(), pedge);
}
  
bool PartEdgeUnionMemLocObject::isLiveML(PartEdgePtr pedge) {
  assert(unionML_p);
  return unionML_p->isLiveML(pedge);
}

bool PartEdgeUnionMemLocObject::isFullML(PartEdgePtr pedge) {
  assert(unionML_p);
  return unionML_p->isFullML(pedge);
}

bool PartEdgeUnionMemLocObject::isEmptyML(PartEdgePtr pedge) {
  assert(unionML_p);
  return unionML_p->isEmptyML(pedge);
}

MemLocObjectPtr PartEdgeUnionMemLocObject::copyML() const {
  return boost::make_shared<PartEdgeUnionMemLocObject>(*this);
}

void PartEdgeUnionMemLocObject::setMLToFull() {
  unionML_p = boost::make_shared<FullMemLocObject>();
}

string PartEdgeUnionMemLocObject::str(string indent) const {
  ostringstream oss;
  assert(unionML_p);
  oss << "[UnionML=" << unionML_p->str(indent) << "]";
  return oss.str();
}

/* #######################
   ##### IndexVector ##### 
   ####################### */

//std::string IndexVector::str(const string& indent)
// pretty print for the object
std::string IndexVector::str(std::string indent) const
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

template class MappedCodeLocObject<ComposedAnalysis*, true>;
template class MappedCodeLocObject<ComposedAnalysis*, false>;
template class MappedValueObject<ComposedAnalysis*, true>;
template class MappedValueObject<ComposedAnalysis*, false>;
template class MappedMemRegionObject<ComposedAnalysis*, true>;
template class MappedMemRegionObject<ComposedAnalysis*, false>;
template class MappedMemLocObject<ComposedAnalysis*, true>;
template class MappedMemLocObject<ComposedAnalysis*, false>;
} //namespace fuse
