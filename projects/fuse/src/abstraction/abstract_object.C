#include "sage3basic.h"


using namespace std;

#include "abstract_object.h"
#include <iostream>
#ifndef DISABLE_SIGHT
#include "sight.h"
#endif

#include "compose.h"


namespace fuse {

AbstractObjectPtr NULLAbstractObject;

AbstractObject::~AbstractObject() {
}

// Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
// classes to save the cost of a dynamic cast.
bool AbstractObject::isValueObject() {
  //return dynamic_cast<ValueObject*>(this);
  return getAOType() == Value;
}
bool AbstractObject::isCodeLocObject() {
  //return dynamic_cast<CodeLocObject*>(this);
  return getAOType() == CodeLoc;
}
bool AbstractObject::isMemRegionObject() {
  //return dynamic_cast<MemRegionObject*>(this);
  return getAOType() == MemRegion;
}
bool AbstractObject::isMemLocObject() {
  //return dynamic_cast<MemLocObject*>(this);
  return getAOType() == MemLoc;
}

bool AbstractObject::isMappedAO()
{
/*  cout << "AbstractObject::isMappedAO() "<<endl;
  cout << str()<<endl;
  cout << "boost::dynamic_cast<MappedCodeLocObject*>(this)="  <<dynamic_cast<MappedCodeLocObject<ComposedAnalysis*>*>(this)  <<endl;
  cout << "boost::dynamic_cast<MappedValueObject*>(this)="    <<dynamic_cast<MappedValueObject<ComposedAnalysis*>*>(this)    <<endl;
  cout << "boost::dynamic_cast<MappedMemLocObject*>(this)="   <<dynamic_cast<MappedMemLocObject<ComposedAnalysis*>*>(this)   <<endl;
  cout << "boost::dynamic_cast<MappedMemRegionObject*>(this)="<<dynamic_cast<MappedMemRegionObject<ComposedAnalysis*>*>(this)<<endl;
  //return false;
  return dynamic_cast<MappedCodeLocObject<ComposedAnalysis*>*>(this) ||
         dynamic_cast<MappedValueObject<ComposedAnalysis*>*>(this)   ||
         dynamic_cast<MappedMemLocObject<ComposedAnalysis*>*>(this)  ||
         dynamic_cast<MappedMemRegionObject<ComposedAnalysis*>*>(this);*/
  return false;
}

/*******************
 ***** hierKey *****
 *******************/

AbstractionHierarchy::hierKey::hierKey(bool endOfHierarchy) :
    endOfHierarchy(endOfHierarchy) {
}
AbstractionHierarchy::hierKey::hierKey(
    const std::list<comparablePtr>& keyList, bool endOfHierarchy) :
    keyList(keyList), endOfHierarchy(endOfHierarchy) {
}
AbstractionHierarchy::hierKey::hierKey(comparablePtr subKey,
    bool endOfHierarchy) :
    endOfHierarchy(endOfHierarchy) {
  keyList.push_back(subKey);
}
AbstractionHierarchy::hierKey::hierKey(hierKeyPtr that) {
  keyList = that->keyList;
  endOfHierarchy = that->endOfHierarchy;
}

std::list<comparablePtr>::const_iterator AbstractionHierarchy::hierKey::begin() {
  return keyList.begin();
}
std::list<comparablePtr>::const_iterator AbstractionHierarchy::hierKey::end() {
  return keyList.end();
}
void AbstractionHierarchy::hierKey::add(comparablePtr c) {
  if (!endOfHierarchy)
    keyList.push_back(c);
}
void AbstractionHierarchy::hierKey::add(
    std::list<comparablePtr>::const_iterator beginIt,
    std::list<comparablePtr>::const_iterator endIt) {
  if (!endOfHierarchy)
    keyList.insert(keyList.end(), beginIt, endIt);
}
const std::list<comparablePtr>& AbstractionHierarchy::hierKey::getList() {
  return keyList;
}

void AbstractionHierarchy::hierKey::reachedEndOfHierarchy() {
  endOfHierarchy = true;
}

bool AbstractionHierarchy::hierKey::equal(
    const comparable& that_arg) const {
  const hierKey& that = dynamic_cast<const hierKey&>(that_arg);
  if (keyList.size() != that.keyList.size())
    return false;

  list<comparablePtr>::const_iterator iThis = keyList.begin(), iThat =
      that.keyList.begin();
  for (; iThis != keyList.end(); ++iThis, ++iThat)
    if (*iThis != *iThat)
      return false;

  return true;
}

bool AbstractionHierarchy::hierKey::less(
    const comparable& that_arg) const {
  const hierKey& that = dynamic_cast<const hierKey&>(that_arg);
  if (keyList.size() != that.keyList.size())
    return keyList.size() < that.keyList.size();

  list<comparablePtr>::const_iterator iThis = keyList.begin(), iThat =
      that.keyList.begin();
  for (; iThis != keyList.end(); ++iThis, ++iThat) {
    if (*iThis < *iThat)
      return true;
    if (*iThis > *iThat)
      return false;
  }

  // this == that
  return false;
}

// Stringification of hierKeys
std::ostream& operator<<(std::ostream& s, AbstractionHierarchy::hierKey* k) {
  s << "[hierKey: " << k->getList() << ", endOfHierarchy="
      << k->endOfHierarchy << "]";
  return s;
}
std::ostream& operator<<(std::ostream& s, AbstractionHierarchy::hierKeyPtr k) {
  s << k.get();
  return s;
}

// IntersectHierKeys a lists of intersectComparables, each of which contains a list of comparablePtrs from
// different analyses.
// Equality checking is implemented as:
//    equal: if all comparable objects within an IntersectComparable says that the objects are equal
//    not equal: if any say they're not equal.
// Less checking:
//    - Iterate through the comparable objects within an IntersectComparable in fixed order
//    - Find the first that says they're less, use its less operator
// This will not lead to cycles in the partial order. Suppose our IntersectHierKey is built from key lists
// produced by two analyses, that impose the following partial order
//    Analysis 1: objA < objB = objC
//    Analysis 2: objA > objB > objC
// The above algorithm will induce the following order:
//                objA < objB > objC
// The order among objB and objC is irrelevant to analysis 1 and if they're oddly ordered, it
// must be consistent with its order because from its perspective objB=objC.

/******************************
 ***** IntersectComparable ****
 ******************************/

AbstractionHierarchy::IntersectComparable::IntersectComparable() {}
AbstractionHierarchy::IntersectComparable::IntersectComparable(const std::list<comparablePtr>& subComp) :
    subComp(subComp) {}

// Add a new comparablePtr to this intersection
void AbstractionHierarchy::IntersectComparable::add(comparablePtr newC)
{ subComp.push_back(newC); }

// This == That
bool AbstractionHierarchy::IntersectComparable::equal(const comparable& that_arg) const {
  const IntersectComparable& that = dynamic_cast<const IntersectComparable&>(that_arg);

  if(subComp.size() != that.subComp.size()) return false;

  list<comparablePtr>::const_iterator thisI=subComp.begin(),
                                      thatI=that.subComp.begin();
  for(; thisI!=subComp.end(); ++thisI, ++thatI)
    if(*thisI != *thatI) return false;

  return true;
}

// This < That
bool AbstractionHierarchy::IntersectComparable::less(const comparable& that_arg) const {
  const IntersectComparable& that = dynamic_cast<const IntersectComparable&>(that_arg);

  if(subComp.size() < that.subComp.size()) return true;
  if(subComp.size() > that.subComp.size()) return false;

  list<comparablePtr>::const_iterator thisI=subComp.begin(),
                                      thatI=that.subComp.begin();
  for(; thisI!=subComp.end(); ++thisI, ++thatI) {
    if(*thisI < *thatI) return true;
    if(*thisI > *thatI) return false;
  }

  // If we reach this point, this == that
  return false;
}

// String method
std::string AbstractionHierarchy::IntersectComparable::str(std::string indent) const {
  ostringstream s;

  if(subComp.size()==1) {
    s << "[IComp:"<<(*subComp.begin())->str(indent)<<"]";
    //return (*subComp.begin())->str(indent);
  } else {
    //s << "[IComp:"<<endl;
    s << "<table border=1>";
    for(list<comparablePtr>::const_iterator i=subComp.begin(); i!=subComp.end(); ++i) {
      s << "<tr><td>"<<(*i)->str(indent)<<"</td></tr>";//<<endl;
    }
    s << "</table>";
    //s << "]";
  }
  return s.str();
}

/************************************
 ***** IntersectMappedComparable ****
 ************************************/

// IntersectMappedComparable works just like IntersectComparable except that it allows for the
// possibility different analyses that contributed comparable objects to a full key, provided
// different numbers of keys. We assume that if a given IntersectMappedComparable doesn't have a
// comparable object from a given analysis, appropriate differentiating info was provided in earlier
// objects in the list and thus, at this point the remaining comparable objects (mapped to
// the appropriate analyses) are to be used for comparison.
// Given two IntersectMappedComparable objects with comparable objects mapped to different keys,
// the common keys are used for comparison.

template<typename Key>
AbstractionHierarchy::IntersectMappedComparable<Key>::IntersectMappedComparable() {}

template<typename Key>
AbstractionHierarchy::IntersectMappedComparable<Key>::IntersectMappedComparable(const std::map<Key, comparablePtr>& subComp) :
    subComp(subComp) {}

// Add a new comparablePtr to this intersection
template<typename Key>
void AbstractionHierarchy::IntersectMappedComparable<Key>::add(Key k, comparablePtr newC)
{ subComp[k] = newC; }

// This == That
template<typename Key>
bool AbstractionHierarchy::IntersectMappedComparable<Key>::equal(const comparable& that_arg) const {
  const IntersectMappedComparable<Key>& that = dynamic_cast<const IntersectMappedComparable<Key>&>(that_arg);

  int numSharedKeys=0;
  for(typename map<Key, comparablePtr>::const_iterator thisI=subComp.begin(); thisI!=subComp.end(); ++thisI) {
    typename map<Key, comparablePtr>::const_iterator thatI = that.subComp.find(thisI->first);
    if(thatI != that.subComp.end()) {
      if(thisI->second != thatI->second) return false;
      ++numSharedKeys;
    }
  }

  // If we couldn't find a difference between the two objects they're equal, OR
  if(numSharedKeys>0) return true;
  // If they don't share any keys, not sure what to do here since these objects denote unrelatable sets.
  // However, this should never happen
  else
    ROSE_ASSERT(0);
}

// This < That
template<typename Key>
bool AbstractionHierarchy::IntersectMappedComparable<Key>::less(const comparable& that_arg) const {
  const IntersectMappedComparable<Key>& that = dynamic_cast<const IntersectMappedComparable<Key>&>(that_arg);

  int numSharedKeys=0;
  for(typename map<Key, comparablePtr>::const_iterator thisI=subComp.begin(); thisI!=subComp.end(); ++thisI) {
    typename map<Key, comparablePtr>::const_iterator thatI = that.subComp.find(thisI->first);
    if(thatI != that.subComp.end()) {
      if(thisI->second < thatI->second) return true;
      if(thisI->second > thatI->second) return false;
      ++numSharedKeys;
    }
  }

  // If we couldn't find a key along which this object is different from that, they're equal, OR
  if(numSharedKeys>0) return false;
  // If they don't share any keys, not sure what to do here since these objects denote unrelatable sets.
  // However, this should never happen
  else
    ROSE_ASSERT(0);
}

// String method
template<typename Key>
std::string AbstractionHierarchy::IntersectMappedComparable<Key>::str(std::string indent) const {
  ostringstream s;

  if(subComp.size()==1) {
    s << "[IMComp:"<<subComp.begin()->second->str(indent)<<"]";
    //return (*subComp.begin())->str(indent);
  } else {
    //s << "[IComp:"<<endl;
    s << "<table border=1>";
    for(typename map<Key, comparablePtr>::const_iterator i=subComp.begin(); i!=subComp.end(); ++i) {
      s << "<tr><td>"<<i->first->str("")<<"</td><td>"<<i->second->str(indent)<<"</td></tr>";//<<endl;
    }
    s << "</table>";
    //s << "]";
  }
  return s.str();
}

/****************************
 ***** IntersectHierKey *****
 ****************************/

AbstractionHierarchy::IntersectHierKey::IntersectHierKey(const std::list<hierKeyPtr>& subKeys) : subKeys(subKeys) {
  vector<list<comparablePtr>::const_iterator> curSubKeyList;
  vector<list<comparablePtr>::const_iterator> endSubKeyList;

#ifndef DISABLE_SIGHT
  scope s("AbstractionHierarchy::IntersectHierKey::IntersectHierKey");
  for (list<hierKeyPtr>::const_iterator cur = subKeys.begin(); cur != subKeys.end(); ++cur) {
    dbg << *cur << endl;
  }
#endif

  // Initialize curSubKeyList/endSubKeyList to point to the start/end of the keyLists of all the subKeys and
  // compute their maximum length
  unsigned int maxLength = 0;
  for (list<hierKeyPtr>::const_iterator cur = subKeys.begin();
      cur != subKeys.end(); ++cur) {
    curSubKeyList.push_back((*cur)->keyList.begin());
    endSubKeyList.push_back((*cur)->keyList.end());

    maxLength = (
        (*cur)->keyList.size() > maxLength ?
            (*cur)->keyList.size() : maxLength);
    //cout << "(*cur)->keyList.size()="<<(*cur)->keyList.size()<<", maxLength="<<maxLength<<endl;
  }

  // If all the subKeys are prefixes of each other, the intersection object is just equal
  // to the longest subKey that the others are prefixes of.
  // Otherwise, it is the empty set.

  // Iterate over all the subKeys and find the common hierarchy among them or detect
  // the difference among the hierarchies.
  for (unsigned int i = 0; i < maxLength; ++i) {
#ifndef DISABLE_SIGHT
    dbg << i << ":" << endl;
#endif
    IntersectComparablePtr intersectComp = makePtr<IntersectComparable>();
    for (unsigned int j = 0; j < curSubKeyList.size(); ++j) {
      // If we haven't yet reached the end of the current subKey
      if (curSubKeyList[j] != endSubKeyList[j]) {
        /*cout << "    curSubKeyList[" << j << "]=" << curSubKeyList[j]->str()
            << ", intersectComp=" << intersectComp << endl;
        // If this is the first subKey for which we have the i-th entry, record its comparable object
        if (!intersectComp) {
          intersectComp = *curSubKeyList[j];
          //cout << "hi"<<endl;
        }
        // If it is not the first, make sure that they're equal. Otherwise, this object denotes the
        // empty set, which we currently don't handle.
        else {
          if (!(*intersectComp == **curSubKeyList[j]))
            ROSE_ASSERT(0);
        }*/
        /*// Add it to keyList
        keyList.push_back(*curSubKeyList[j]);*/

        intersectComp->add(*curSubKeyList[j]);
#ifndef DISABLE_SIGHT
        dbg << "Pushing "<<(*curSubKeyList[j])->str()<<endl;
#endif

        // Advance the current iterator
        ++curSubKeyList[j];
      }
    }

    // Push the comparison object that is common to all the subKeys at this level
    // onto the intersection's keyList
    ROSE_ASSERT(intersectComp);
    keyList.push_back(intersectComp);
  }

  /*// endOfHierarchy is true if it is true for all the subKeys of length=maxLength
  endOfHierarchy = true;
  for (list<hierKeyPtr>::const_iterator cur = subKeys.begin();
      cur != subKeys.end(); ++cur) {
    if ((*cur)->keyList.size() == maxLength
        && (*cur)->endOfHierarchy == false)
      endOfHierarchy = false;
  }*/

  // endOfHierarchy is true if it is true for all of the subKeys
  endOfHierarchy = true;
  for (list<hierKeyPtr>::const_iterator cur = subKeys.begin(); cur != subKeys.end(); ++cur) {
    endOfHierarchy = endOfHierarchy && (*cur)->endOfHierarchy;
  }

#ifndef DISABLE_SIGHT
  ostringstream oss; oss << "this="<<((hierKey*)this)<<endl;
  dbg << oss.str();
#endif
}

// Returns whether the set denoted by key is live at the given PartEdge
bool AbstractionHierarchy::IntersectHierKey::isLive(PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // An intersection is dead if any of the sub-objects is dead
  for (list<hierKeyPtr>::iterator cur = subKeys.begin(); cur != subKeys.end(); ++cur) {
    if (!(*cur)->isLive(pedge, comp, analysis))
      return false;
  }
  return true;
}

/**********************************
 ***** IntersectMappedHierKey *****
 **********************************/

template<class Key>
AbstractionHierarchy::IntersectMappedHierKey<Key>::IntersectMappedHierKey
                        (const std::map<Key, hierKeyPtr>& subKeys) : subKeys(subKeys) {
  // The key fields in the subKeys map
  vector<Key> KeyInSubKey;
  // The begin and end iterators to the hierKeys in the subKeys map
  vector<list<comparablePtr>::const_iterator> curSubKeyList;
  vector<list<comparablePtr>::const_iterator> endSubKeyList;

#ifndef DISABLE_SIGHT
  scope s("AbstractionHierarchy::IntersectMappedHierKey::IntersectMappedHierKey");
  for (typename std::map<Key, hierKeyPtr>::const_iterator cur = subKeys.begin(); cur != subKeys.end(); ++cur) {
    dbg << cur->first->str() << ": "<<cur->second << endl;
  }
#endif

  // Initialize KeyInSubKey, curSubKeyList end SubKeyList to point to the start/end of the keyLists of all the
  // subKeys and compute their maximum length
  unsigned int maxLength = 0;
  for (typename std::map<Key, hierKeyPtr>::const_iterator cur = subKeys.begin();
      cur != subKeys.end(); ++cur) {
    KeyInSubKey.push_back(cur->first);
    curSubKeyList.push_back(cur->second->keyList.begin());
    endSubKeyList.push_back(cur->second->keyList.end());

    maxLength = (
        cur->second->keyList.size() > maxLength ?
            cur->second->keyList.size() : maxLength);
    //cout << "(*cur)->keyList.size()="<<(*cur)->keyList.size()<<", maxLength="<<maxLength<<endl;
  }

  // Iterate over all the subKeys and find the common hierarchy among them or detect
  // the difference among the hierarchies.
  for (unsigned int i = 0; i < maxLength; ++i) {
#ifndef DISABLE_SIGHT
    dbg << i << ":" << endl;
#endif
    CompSharedPtr<IntersectMappedComparable<Key> > intersectComp = makePtr<IntersectMappedComparable<Key> >();
    for (unsigned int j = 0; j < curSubKeyList.size(); ++j) {
      // If we haven't yet reached the end of the current subKey
      if (curSubKeyList[j] != endSubKeyList[j]) {
        /*cout << "    curSubKeyList[" << j << "]=" << curSubKeyList[j]->str()
            << ", intersectComp=" << intersectComp << endl;
        // If this is the first subKey for which we have the i-th entry, record its comparable object
        if (!intersectComp) {
          intersectComp = *curSubKeyList[j];
          //cout << "hi"<<endl;
        }
        // If it is not the first, make sure that they're equal. Otherwise, this object denotes the
        // empty set, which we currently don't handle.
        else {
          if (!(*intersectComp == **curSubKeyList[j]))
            ROSE_ASSERT(0);
        }*/
        /*// Add it to keyList
        keyList.push_back(*curSubKeyList[j]);*/

        intersectComp->add(KeyInSubKey[j], *curSubKeyList[j]);
#ifndef DISABLE_SIGHT
        dbg << "Pushing "<<(*curSubKeyList[j])->str()<<endl;
#endif

        // Advance the current iterator
        ++curSubKeyList[j];
      }
    }

    // Push the comparison object that is common to all the subKeys at this level
    // onto the intersection's keyList
    ROSE_ASSERT(intersectComp);
    keyList.push_back(intersectComp);
  }

  /*// endOfHierarchy is true if it is true for all the subKeys of length=maxLength
  endOfHierarchy = true;
  for (list<hierKeyPtr>::const_iterator cur = subKeys.begin();
      cur != subKeys.end(); ++cur) {
    if ((*cur)->keyList.size() == maxLength
        && (*cur)->endOfHierarchy == false)
      endOfHierarchy = false;
  }*/

  // endOfHierarchy is true if it is true for all of the subKeys
  endOfHierarchy = true;
  for(typename map<Key, hierKeyPtr>::const_iterator cur = subKeys.begin(); cur != subKeys.end(); ++cur) {
    endOfHierarchy = endOfHierarchy && cur->second->endOfHierarchy;
  }

#ifndef DISABLE_SIGHT
  ostringstream oss; oss << "this="<<((hierKey*)this)<<endl;
  dbg << oss.str();
#endif
}

// Returns whether the set denoted by key is live at the given PartEdge
template<class Key>
bool AbstractionHierarchy::IntersectMappedHierKey<Key>::isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // An intersection is dead if any of the sub-objects is dead
  for(typename map<Key, hierKeyPtr>::const_iterator cur = subKeys.begin(); cur != subKeys.end(); ++cur) {
    if (!cur->second->isLive(pedge, comp, analysis))
      return false;
  }
  return true;
}

/********************************
 ***** AbstractionHierarchy *****
 ********************************/

AbstractionHierarchy::AOSHierKey::AOSHierKey(AbstractObjectPtr obj,
    bool endOfHierarchy) :
    hierKey(endOfHierarchy), obj(obj) {
}
//AOSHierKey(AbstractObjectPtr obj, const std::list<comparablePtr>& keyList, bool endOfHierarchy=false): hierKey(keyList, endOfHierarchy), obj(obj) {}
//AOSHierKey(AbstractObjectPtr obj, comparablePtr subKey, bool endOfHierarchy=false): hierKey(subKey, endOfHierarchy), obj(obj) { }
AbstractionHierarchy::AOSHierKey::AOSHierKey(AbstractObjectPtr obj,
    hierKeyPtr subHierKey) :
    hierKey(subHierKey), obj(obj) {
}

bool AbstractionHierarchy::AOSHierKey::isLive(PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
/*  scope s(txt()<<"AbstractionHierarchy::AOSHierKey::isLive: "<<obj->isLive(pedge, comp, analysis));
  dbg << "obj="<<obj->str()<<endl;
  dbg << "pedge="<<pedge->str()<<endl;*/
  return obj->isLive(pedge, comp, analysis);
}

AbstractionHierarchy::hierRel AbstractionHierarchy::hierCompare(
    AbstractionHierarchyPtr left, AbstractionHierarchyPtr right) {
  const AbstractionHierarchy::hierKeyPtr& leftKey = left->getHierKey();
  const AbstractionHierarchy::hierKeyPtr& rightKey = right->getHierKey();
  std::list<comparablePtr>::const_iterator l = leftKey->begin(), r =
      rightKey->begin();
  for (; l != leftKey->end() && r != rightKey->end(); l++, r++) {
    if (*l != *r)
      return disjoint;
  }
  // If we've reached the end of the right key but not the left and the prefix of the
  // left key is equal to the entirety of the right key
  if (l != leftKey->end())
    return rightContains;
  // Inverse of above
  if (r != rightKey->end())
    return leftContains;

  // The two keys must be the same
  return equal;
}

/******************************************
 ***** AbstractObjectDisjointHierWrap *****
 ****************************************** /

 // Returns a key that uniquely identifies this particular AbstractObject in the
 // set hierarchy.
 const AbstractionHierarchy::hierKeyPtr& AbstractObjectDisjointHierWrap::getHierKey() const {
 if(!isHierKeyCached) {
 // Set key to refer to the keyCode, which is allocated on the spot to wrap the hashCode()
 // It is assumed that the hashCode doesn't change during an object's lifetime.
 ((AbstractObjectDisjointHierWrap*)this)->cachedHierKey = boost::make_shared<AOSHierKey>(shared_from_this(), makePtr<hashCodeWrapper>(getHashCode()));
 ((AbstractObjectDisjointHierWrap*)this)->isHierKeyCached = true;
 }
 return cachedHierKey;
 }
 */

/* ##########################
   ##### AbstractObject #####
   ########################## */

// It is often useful to create an Abstraction object that denotes the union or intersection
// of multiple other objects. There are multiple ways to do this:
// - We can create a copy of one of the objects to be unioned and call its meetUpdate() and
//   intersectUpdate() methods to union/intersect the others in. This is supported for all
//   Abstractions.
// - We can create an object that maps some keys to Abstractions and implements all relevant operations
//   by forwarding them to the Abstractions within it and returning the most (intersection) or least (union)
//   conservative answer. This only works for AbstractObjects via the MappedAbstractObject class.
//   It is made more challenging by the fact that the keys may be any type, which hidden from the users
//   of these objects.
// The functions below allow either of the above methods to be used to create unions and intersections.
// They take as argument a MAOMap, which maps some unknown keys to Abstractions. This is a good choice
// because it allows users to iterate over the mapped abstractions without knowing anything about the
// type of the keys, and because they can return MappedAbstractObjects that contain the Abstractions
// that are mapped inside of them MAOMap::getMappedObj().
AbstractionPtr AbstractObject::genUnion(MAOMapPtr maoMap) {
  // Create a MappedAbstractObject that denotes the union of the AbstractObjects in mapMap
  return maoMap->getMappedObj(Union);
}

AbstractionPtr AbstractObject::genIntersection(MAOMapPtr maoMap) {
  // Create a MappedAbstractObject that denotes the union of the AbstractObjects in mapMap
  return maoMap->getMappedObj(Intersection);
}

/* ################################
   ##### MappedAbstractObject #####
   ################################ */

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::
      MappedAbstractObject(uiType ui, int nFull, ComposedAnalysis* analysis) :
  AOSubType(NULL), MappedAbstractionBase(ui, nFull, analysis)
{
  initializedPOKey=false;
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::
      MappedAbstractObject(uiType ui, int nFull, ComposedAnalysis* analysis, const std::map<Key, AOSubTypePtr >& aoMap) :
  AOSubType(NULL), MappedAbstractionBase(ui, nFull, analysis), aoMap(aoMap)
{
  initializedPOKey=false;
}


template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::
      MappedAbstractObject(const MappedAbstractObject& that) :
  AOSubType(that), MappedAbstractionBase(that.ui, that.nFull, that.analysis), aoMap(that.aoMap)
{
  initializedPOKey=false;
}


// Returns true if this is a MappedAbstractObject and false otherwise
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isMappedAO()
{
  //cout << "MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isMappedAO()"<<endl;
  return true; }

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
SgNode* MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::getBase() const {
  // Returns the base SgNode shared by all the MemLocs in this object or NULL, if the base SgNodes are different
  SgNode* base = NULL;
  for (typename map<Key, AOSubTypePtr >::const_iterator it = aoMap.begin(); it != aoMap.end(); ++it) {
    if (it == aoMap.begin())
      base = it->second->getBase();
    else if (base != it->second->getBase())
      return NULL;
  }

  return base;
}

/*  // Allocates a copy of this object and returns a pointer to it
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
AbstractObjectPtr MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::copyAO() const {
  return copyAOType();
}*/


// Functions that identify the type of AbstractObject this is. Should be over-ridden by derived
// classes to save the cost of a dynamic cast.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
AbstractObject::AOType MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::getAOType() const
{ return type; }

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::add(Key key, AOSubTypePtr ao,
                                               PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // If the object is already full don't add anything
  if (isUnion() && isFull(pedge, comp, analysis))
    return;

  // If the ao is not full add/update the map
  if (!ao->isFull(pedge, comp, analysis)) {
    aoMap[key] = ao;
  } else {
    nFull++;
    if (isUnion())
      setAOToFull();
  }
}

// Allocates a copy of this object and returns a pointer to it
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
AOSubTypePtr MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::copyAOType() const {
  map<Key, AOSubTypePtr > newAOMap;
  for(typename map<Key, AOSubTypePtr >::const_iterator it = aoMap.begin(); it != aoMap.end(); ++it)
    newAOMap[it->first] = it->second->copyAOType();

  return boost::make_shared<MappedAOSubType >(ui, nFull, analysis, newAOMap);
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::mayEqualWithKey(Key key,
    const map<Key, AOSubTypePtr >& thatAOMap, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  typename map<Key, AOSubTypePtr >::const_iterator s_it = thatAOMap.find(key);
  if (s_it == thatAOMap.end())
    return true;
  return aoMap[key]->mayEqual(s_it->second, pedge, comp, analysis);
}

//! Two AO objects are may equals if there is atleast one execution or sub-exectuion
//! in which they represent the same code location.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise, the conservative answer for mayEqualAO is true.
//! Mapped AOs are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed AO object correspond to some dataflow facts computed by Key=Analysis* or
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MayEquality check on mapped AO is performed on intersection of sub-executions
//! or union of sub-executions over the keyed AO objects.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::mayEqualAO(AOSubTypePtr thatAO, PartEdgePtr pedge)
{ return mayEqual(thatAO, pedge, analysis->getComposer(), analysis); }

// Returns whether this object may/must be equal to o within the given Part p
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::mayEqual(AbstractObjectPtr thatAO, PartEdgePtr pedge,
                                                    Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > thatAO_p =
      boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(thatAO);
  assert(thatAO_p);

  // This object denotes full set of AO (full set of executions)
  if (isFull(pedge, comp, analysis))
    return true;

  // denotes empty set
  if (isEmpty(pedge, comp, analysis))
    return false;

  // presence of one more full objects will result in full set over union
  if (isUnion() && nFull > 0)
    return true;

  // Two cases reach here [1] isUnion()=true && nFull_AO=0 [2] intersect=true && nFullAO=0 or nFull_AO!=0.
  // For both cases iterate on the AO map and discharge the mayEqualAO query to individual objects
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, AOSubTypePtr > thatAOMap = thatAO_p->getAOMap();
  typename map<Key, AOSubTypePtr >::iterator it;
  for (it = aoMap.begin(); it != aoMap.end(); ++it) {
    // discharge query
    bool isMayEq = mayEqualWithKey(it->first, thatAOMap, pedge, comp, analysis);

    // 1. Union of sub-executions and the object does not contain any full objects.
    // If the discharged query comes back as true for this case then we have found atleast one execution
    // under which the two objects are same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If false we iterate further as any AO can add more executions under which the objects are may equals.
    if (isUnion() && isMayEq == true)
      return true;

    // 2. Intersection of sub-executions and the object may contain full objects (nFull != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns false then return false.
    // We did not find one execution in which the two objects are may equals.
    // Note that set of executions are contained over keyed objects (analyses are conservative).
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns true iterate further as some executions corresponding to true may be dropped.
    else if (isIntersection() && isMayEq == false)
      return false;
  }

  // All the keyed objects returned false for the discharged query under union.
  // We haven't found a single execution under which the two objects are may equals.
  if (isUnion())
    return false;
  // All the keyed objects returned true for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are may equals.
  else if (isIntersection())
    return true;
  else
    assert(0);
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::mustEqualWithKey(Key key,
    const map<Key, AOSubTypePtr >& thatAOMap, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  typename map<Key, AOSubTypePtr >::const_iterator s_it = thatAOMap.find(key);
  if (s_it == thatAOMap.end())
    return false;
  return aoMap[key]->mustEqual(s_it->second, pedge, comp, analysis);
}

//! Two AO objects are must equals if they represent the same code
//! location on all executions.
//! Analyses are conservative as they start with full set of executions.
//! Dataflow facts (predicates) shrink the set of sub-executions.
//! We do not explicity store set of sub-executions and they are described
//! by the abstract objects computed from dataflow fact exported by the analysis.
//! Unless the analyses discover otherwise conservative answer for mustEqualAO is false.
//! Mapped AOs are keyed using either ComposedAnalysis* or PartEdgePtr.
//! Each keyed AO object correspond to some dataflow facts computed by Key=Analysis* or
//! computed at Key=PartEdgePtrthat describes some sets of executions.
//! MustEquality check on mapped AO is performed on intersection (mostAccurate=true) of sub-executions
//! or union (mostAccurate=false) of sub-executions over the keyed AO objects.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::mustEqualAO(AOSubTypePtr thatAO, PartEdgePtr pedge)
{ return mustEqual(thatAO, pedge, analysis->getComposer(), analysis); }

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::mustEqual(AbstractObjectPtr thatAO, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > thatAO_p =
      boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(thatAO);
  assert(thatAO_p);

  // This object denotes full set of AO (full set of executions)
  if (isFull(pedge, comp, analysis))
    return false;

  // denotes empty set
  if (isEmpty(pedge, comp, analysis))
    return false;

  // presence of one more full objects will result in full set over union
  if (isUnion() && nFull > 0)
    return true;

  // Two cases reach here [1] isUnion()=true && nFull_AO=0 [2] intersect=true && nFullAO=0 or nFull_AO!=0.
  // For both cases iterate on the AO map and discharge the mayEqualAO query to individual objects
  // which are answered based on its set of sub-executions (or its dataflow facts) computed by the corresponding analysis.
  const map<Key, AOSubTypePtr > thatAOMap = thatAO_p->getAOMap();
  typename map<Key, AOSubTypePtr >::iterator it;
  for (it = aoMap.begin(); it != aoMap.end(); ++it) {
    // discharge query
    bool isMustEq = mustEqualWithKey(it->first, thatAOMap, pedge, comp, analysis);

    // 1. Union of sub-executions and the object does not contain any full objects
    // If the discharged query comes back as false for this case then we have found atleast one execution
    // under which the two objects are not same and the set can only grow and the result of this query is not going
    // to change due to union.
    // If it returns true we iterate further as any AO can add more executions under which the objects are not must equals.
    if (isUnion() && isMustEq == false)
      return false;

    // 2. Intersection of sub-executions and the object may contain full objects (nFull != 0).
    // The sub-executions are intersected and therefore it does not matter if we have full objects.
    // If the discharged query returns true then return true.
    // Under all sub-executions (corresponding to the AO) the two objects must equal.
    // Note that set of executions are contained over keyed objects as the analyses are conservative.
    // This set only shrinks during intersection and it is not going to affect the result of this query.
    // If it returns false iterate further as some executions corresponding to false may be dropped.
    else if (isIntersection() && isMustEq == true)
      return true;
  }

  // All the keyed objects returned true for the discharged query under union.
  // We haven't found a single execution under which the two objects are not equal.
  if (isUnion())
    return true;
  // All the keyed objects returned false for the discharged query under intersection.
  // We have atleast one execution in common in which the two objects are not equal.
  else if (isIntersection())
    return false;
  else
    assert(0);
}

//! Discharge the query to the corresponding AO
//! If key not found in thatAOMap return false
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::equalSetWithKey(Key key,
    const map<Key, AOSubTypePtr >& thatAOMap, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  typename map<Key, AOSubTypePtr >::const_iterator s_it = thatAOMap.find(key);
  if (s_it == thatAOMap.end())
    return false;
  return aoMap[key]->equalSet(s_it->second, pedge, comp, analysis);
}

//! Two objects are equal sets if they denote the same set of memory locations
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed CodeLoc objects
//! If all the discharged queries come back equal then the two objects are equal otherwise not.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::equalSetAO(AOSubTypePtr thatAO, PartEdgePtr pedge)
{ return equalSet(thatAO, pedge, analysis->getComposer(), analysis); }

// general versions of equalset() that accounts for framework details before routing the call to the
// derived class' equalset() check. specifically, it routes the call through the composer to make
// sure the equalset() call gets the right partedge.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::equalSet(AbstractObjectPtr thatAO, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > thatAO_p =
      boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(thatAO);
  assert(thatAO_p);

  // This object denotes full set of AO (full set of executions)
  if (isFull(pedge, comp, analysis))
    return thatAO_p->isFull(pedge, comp, analysis);

  // denotes empty set
  if (isEmpty(pedge, comp, analysis))
    return thatAO_p->isEmpty(pedge, comp, analysis);

  const map<Key, AOSubTypePtr > thatAOMap = thatAO_p->getAOMap();
  typename map<Key, AOSubTypePtr >::iterator it;
  for (it = aoMap.begin(); it != aoMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if (equalSetWithKey(it->first, thatAOMap, pedge, comp, analysis) == false)
      return false;
  }

  return true;
}

//! Discharge the query to the corresponding AO
//! If key not found in thatAOMap return true as the
//! keyed object on thatAOMap denotes full set
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::subSetWithKey(Key key,
    const map<Key, AOSubTypePtr >& thatAOMap, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  typename map<Key, AOSubTypePtr >::const_iterator s_it = thatAOMap.find(key);
  if (s_it == thatAOMap.end())
    return true;
  return aoMap[key]->equalSet(s_it->second, pedge, comp, analysis);
}

//! This object is a non-strict subset of the other if the set of memory locations denoted by this
//! is a subset of the set of memory locations denoted by that.
//! The boolean parameter mostAccurate is not releveant as this query is not
//! answered based on union or intersection of sub-executions.
//! Simply discharge the queries to all keyed CodeLoc objects
//! If all the discharged queries come back true then this is a subset of that otherwise not.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::subSetAO(AOSubTypePtr thatAO, PartEdgePtr pedge)
{ return subSet(thatAO, pedge, analysis->getComposer(), analysis); }

// General versions of subSet() that accounts for framework details before routing the call to the
// derived class' subSet() check. Specifically, it routes the call through the composer to make
// sure the subSet() call gets the right PartEdge.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::subSet(AbstractObjectPtr thatAO, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > thatAO_p =
      boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(thatAO);
  assert(thatAO_p);

  // This object denotes full set of AO (full set of executions)
  if (isFull(pedge, comp, analysis))
    return thatAO_p->isFull(pedge, comp, analysis);

  // denotes empty set
  // thatAO could be empty or non-empty eitherway this will be a non-strict subset of that.
  if (isEmpty(pedge, comp, analysis))
    return true;

  // If both objects have the same keys discharge
  // If this object has a key and that does not then
  // the keyed object is subset of that (return true) implemented by subsetAOWithKey
  // If any of the discharged query return false then return false.
  const map<Key, AOSubTypePtr > thatAOMap = thatAO_p->getAOMap();
  typename map<Key, AOSubTypePtr >::iterator it;
  for (it = aoMap.begin(); it != aoMap.end(); ++it) {
    // discharge query
    // break even if one of them returns false
    if (subSetWithKey(it->first, thatAOMap, pedge, comp, analysis) == false)
      return false;
  }

  // If this object doesn't have the key and that object has the key then
  // return false as this object has full object mapped to the key
  typename map<Key, AOSubTypePtr >::const_iterator c_it;
  for (c_it = thatAOMap.begin(); c_it != thatAOMap.end() && (nFull != 0);
      ++c_it) {
    if (aoMap.find(c_it->first) == aoMap.end())
      return false;
  }

  return true;
}

//! Mapped object liveness is determined based on finding executions
//! in which it may be live.
//! It can be answered based on union (mostAccurate=false) or intersection
//! (mostAccurate=true) of executions
//! The conservative answer is to assume that the object is live
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isLiveAO(PartEdgePtr pedge)
{ return isLive(pedge, analysis->getComposer(), analysis); }

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool  MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // If this object is full return the conservative answer
  if (isFull(pedge, comp, analysis))
    return true;

  // If it has one or more full objects added to it
  // and if the object has mostAccurate=false then return true (weakest answer)
  if (nFull > 0 && isUnion())
    return true;

  // 1. This object may have have one or more full objects under intersection
  // 2. This object doesnt have any full objects added to it under union
  // Under both cases the answer is based on how individual analysis respond to the query
  typename map<Key, AOSubTypePtr >::iterator it = aoMap.begin();
  for (; it != aoMap.end(); ++it) {
    bool isLive = it->second->isLive(pedge, comp, analysis);
    if (isUnion() && isLive == true)
      return true;
    else if (isIntersection() && isLive == false)
      return false;
  }

  // leftover cases
  if (isUnion())
    return false;
  else if (isIntersection())
    return true;
  else
    assert(0);
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::meetUpdateAO(AOSubTypePtr thatAO, PartEdgePtr pedge)
{ return meetUpdate(thatAO, pedge, analysis->getComposer(), analysis); }

//! meetUpdateAO performs the join operation of abstractions of two mls
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > thatAO_p =
      boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(
          that);
  assert(thatAO_p);

  // if this object is already full
  if (isFull(pedge, comp, analysis))
    return false;

  // If that object is full set this object to full
  if (thatAO_p->isFull(pedge, comp, analysis)) {
    nFull++;
    setAOToFull();
    return true;
  }

  // Both objects are not full
  const map<Key, AOSubTypePtr > thatAOMap = thatAO_p->getAOMap();

  typename map<Key, AOSubTypePtr >::iterator it = aoMap.begin();
  typename map<Key, AOSubTypePtr >::const_iterator s_it; // search iterator for thatAOMap

  bool modified = false;
  while (it != aoMap.end()) {
    s_it = thatAOMap.find(it->first);
    // If two objects have the same key then discharge meetUpdate to the corresponding keyed AO objects
    if (s_it != thatAOMap.end()) {
      modified = (it->second)->meetUpdate(s_it->second, pedge, comp, analysis) || modified;
    }

    // Remove the current AO object (current iterator it) from the map if the mapepd object is full.
    // Two cases under which the current AO object can be full.
    // (1) If current key is not found in thatAOMap then the mapped object
    // in thatAOMap is full and the meetUpdate of the current AO with that is also full.
    // (2) meetUpdateAO above of the two keyed objects resulted in this mapped object being full.
    // Under both cases remove the mapped ml from this map
    if (s_it == thatAOMap.end() || (it->second)->isFull(pedge, comp, analysis)) {
      // Current mapped AO has become full as a result of (1) or (2).
      // Remove the item from the map.
      // Note that post-increment which increments the iterator and returns the old value for deletion.
      aoMap.erase(it++);
      nFull++;
      modified = true;

      // If mostAccurate=false then set this entire object to full and return
      if (isUnion()) {
        setAOToFull();
        return true;
      }
    } else
      ++it;
  }
  return modified;
}

//! Method that sets this mapped object to full
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::setAOToFull() {
  assert(nFull > 0);
  if (aoMap.size() > 0)
    aoMap.clear();
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isFullAO(PartEdgePtr pedge)
{ return isFull(pedge, analysis->getComposer(), analysis); }

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  if (nFull > 0 && aoMap.size() == 0)
    return true;
  return false;
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isEmptyAO(PartEdgePtr pedge)
{ return isEmpty(pedge, analysis->getComposer(), analysis); }

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  if (nFull == 0 && aoMap.size() == 0)
    return true;
  return false;
}

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
AOSubTypePtr MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::
                                 project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis) {
  /*scope s("MappedAbstractObject::project()");
  dbg << "this="<<str()<<endl;
  dbg << "KeyIsComposedAnalysis="<<KeyIsComposedAnalysis<<endl;*/

  // If they key is a ComposedAnalysis*
  if(KeyIsComposedAnalysis) {
    // Return the sub-AO mapped under the given analysis key
    typename std::map<Key, AOSubTypePtr >::iterator ao = aoMap.find(analysis);
    assert(ao != aoMap.end());
    return ao->second;

  // If the key is another type
  } else {
    // Apply the project method on all the sub-AOs of this map and return the combination of the returned values (union or intersection,
    // depending on the type of combination this object provides.
    assert(aoMap.size()>0);

    // The object that will hold the combination of the results of project() methods on sub-AOs
    AOSubTypePtr combinedObject;
    for(typename std::map<Key, AOSubTypePtr >::iterator ao=aoMap.begin(); ao!=aoMap.end(); ++ao) {
      /*scope s("Current");
      dbg << "ao="<<ao->second->str()<<endl;*/
      // In the first iteration initialize combinedObject to be the result of calling project() on the first sub-AO
      if(ao==aoMap.begin())
        combinedObject = boost::dynamic_pointer_cast<AOSubType>(ao->second->project(analysis, pedge, comp, clientAnalysis)->copyAO());
      else {
        if(isIntersection()) combinedObject->intersectUpdate(ao->second->project(analysis, pedge, comp, clientAnalysis), pedge, comp, clientAnalysis);
        else                 combinedObject->meetUpdate     (ao->second->project(analysis, pedge, comp, clientAnalysis), pedge, comp, clientAnalysis);
      }
      //dbg << "combinedObject="<<combinedObject->str()<<endl;
    }

    assert(combinedObject);
    return combinedObject;
  }
}

// Returns true if this AbstractObject corresponds to a concrete value that is statically-known
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isConcrete() {
  if(isUnion() && nFull>0) return false;

  typename map<Key, AOSubTypePtr >::iterator it;
  for (it = aoMap.begin(); it != aoMap.end(); ++it) {
    if (isIntersection() && it->second->isConcrete()) return true;
    if (isUnion()        && !it->second->isConcrete()) return false;
  }
  if (isIntersection()) return false;
  else                  return true;
}

// Returns the number of concrete values in this set
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
int MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::concreteSetSize() {
  // This is an over-approximation of the set size that assumes that all the concrete sets of
  // the sub-CodeLocs are disjoint
  int size = 0;
  typename map<Key, AOSubTypePtr >::iterator it;
  for (it = aoMap.begin(); it != aoMap.end(); ++it)
    if (it->second->isConcrete())
      size += it->second->concreteSetSize();
  return size;
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
string MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::str(string indent) const {
  ostringstream oss;
  oss << "<table border=\"1\">";
  oss << "<tr>";
  oss << "<th>"
      << (isUnion() ?
          "UnionMappedAO" : "IntersectMappedAO:")
      << "</th>";
  if (nFull > 0 && aoMap.size() == 0)
    oss << "<th> Full </th> </tr>";
  else if (nFull == 0 && aoMap.size() == 0)
    oss << "<th> Empty </th> </tr>";
  else {
    oss << "</tr>";
    typename map<Key, AOSubTypePtr >::const_iterator it =
        aoMap.begin();
    for (; it != aoMap.end(); ++it) {
      oss << "<tr>";
      oss << "<td>" << (it->first)->str(indent) << "</td>";
      oss << "<td>" << (it->second)->str(indent) << "</td>";
      oss << "</tr>";
    }
  }
  oss << "</table>";
  return oss.str();
}

// Returns whether all instances of this class form a hierarchy. Every instance of the same
// class created by the same analysis must return the same value from this method!
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isHierarchy() const {
  /*scope s("MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isHierarchy()");
  dbg << "#aoMap="<<aoMap.size()<<", aoMap.begin()->second->isHierarchy()="<<aoMap.begin()->second->isHierarchy()<<endl;
  dbg << "aoMap.begin()->second="<<aoMap.begin()->second->str()<<endl;*/
  if(aoMap.size()==1 && aoMap.begin()->second->isHierarchy()) return true;
  else return false;
/*  // Combined CodeLocs form hierarchy if:
  // - All the sub-CodeLocs form hierarchies of their own, AND
  // - The combination is an intersection.
  //   If the combination is a union then consider the following:
  //            MLa     MLb
  //   comb1 = {a,b}, {w, x}
  //   comb2 = {a,b}, {y, z}
  //   Note that MLs from analyses a and b are either identical sets or disjoint
  //   However, comb1 U comb2 = {a, b, w, x, y, z}, for which this property does
  //   not hold unless we work out a new hierarchy for MLb.

  // Unions are not hierarchical unless they're singletons
  if (isUnion()) {
    if (aoMap.size() == 1)
      return aoMap.begin()->second->isHierarchy();
    else
      return false;
  }

  typename map<Key, AOSubTypePtr >::const_iterator it;
  for (it = aoMap.begin(); it != aoMap.end(); ++it)
    if (!it->second->isHierarchy())
      return false;
  return true;*/
}

// Returns whether all instances of all the AbstractObjects within this MAO form a hierachy
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::membersIsHierarchy() const {
  typename map<Key, AOSubTypePtr >::const_iterator it;
  for (it = aoMap.begin(); it != aoMap.end(); ++it)
    if (!it->second->isHierarchy())
      return false;
  return true;
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
const AbstractionHierarchy::hierKeyPtr& MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::getHierKey() const {
  assert(aoMap.size()==1);
  /*scope s("MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::getHierKey()");
  dbg << "this="<<str()<<endl;
  dbg << "aoMap.begin()->second="<<aoMap.begin()->second->str()<<endl;*/
  assert(aoMap.begin()->second->isHierarchy());

  if(aoMap.size()==1 && aoMap.begin()->second->isHierarchy()) return aoMap.begin()->second->getHierKey();
  assert(0);
}
//  // The intersection of multiple objects is just a filtering process from the full
//  // set of objects to the exact one, with each key in the hierarchy further filtering
//  // the set down. As such, a hierarchical key for the intersection of multiple objects
//  // is just the concatenation of the keys of all the individual objects.
//  if (!AOSubType::isHierKeyCached) {
//    /*((MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>*) this)->cachedHierKey =
//     boost::make_shared<AOSHierKey>(
//     ((MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>*) this)->shared_from_this());
//
//     typename map<Key, AOSubTypePtr >::const_iterator it;
//     for (it = aoMap.begin(); it != aoMap.end(); ++it) {
//     AbstractionHierarchyPtr hierIt = boost::dynamic_pointer_cast<
//     AbstractionHierarchy>(it->second);
//     ROSE_ASSERT(hierIt);
//
//     ((MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>*) this)->cachedHierKey->add(
//     hierIt->getHierKey()->begin(), hierIt->getHierKey()->end());
//     }*/
//
//    map<Key, typename AOSubType::hierKeyPtr> subHierKeys;
//    for(typename map<Key, AOSubTypePtr >::const_iterator it = aoMap.begin();
//        it != aoMap.end(); ++it) {
//      AbstractionHierarchyPtr hierIt = boost::dynamic_pointer_cast<AbstractionHierarchy>(it->second);
//      ROSE_ASSERT(hierIt);
//      subHierKeys[it->first] = hierIt->getHierKey();
//    }
//
//    ((MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>*) this)->cachedHierKey =
//        boost::make_shared<AbstractionHierarchy::IntersectMappedHierKey<Key> >(subHierKeys);
//
//    ((MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>*) this)->AOSubType::isHierKeyCached = true;
//  }
//  return AOSubType::cachedHierKey;
//}

// Returns whether sets of the given type form a partial order
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::isPartialOrder()
{ return membersIsHierarchy(); }

// If isPartialOrder() returns true, classes must implement the methods below to establish
// the partial order relationship among class instances.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::MappedPartialOrderKey::isPOLessThan(const PartialOrderKey& that_arg)
{
  try{
    const MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::MappedPartialOrderKey& that =
        dynamic_cast<const MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::MappedPartialOrderKey& >(that_arg);
    return key < that.key;
  } catch(std::bad_cast c)
  { assert(0); }
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
bool MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::MappedPartialOrderKey::isPOEqual(const PartialOrderKey& that_arg)
{
  try{
    const MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::MappedPartialOrderKey& that =
        dynamic_cast<const MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::MappedPartialOrderKey& >(that_arg);
    return key == that.key;
  } catch(std::bad_cast c)
  { assert(0); }
}

// Computes the partial order key that describes this object and stores it in partialOrderKey
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::computePartialOrderKey() {
  // The intersection of multiple objects is just a filtering process from the full
  // set of objects to the exact one, with each key in the hierarchy further filtering
  // the set down. As such, a hierarchical key for the intersection of multiple objects
  // is just the concatenation of the keys of all the individual objects.
  if(!initializedPOKey) {
    assert(isPartialOrder());

    partialOrderKey = boost::make_shared<MappedPartialOrderKey>();
    for(typename map<Key, AOSubTypePtr >::const_iterator it = aoMap.begin();
        it != aoMap.end(); ++it) {
      AbstractionHierarchyPtr hierIt = boost::dynamic_pointer_cast<AbstractionHierarchy>(it->second);
      ROSE_ASSERT(hierIt);
      partialOrderKey->key[it->first] = hierIt->getHierKey();
    }
    initializedPOKey=true;
  }
}

// Returns the PartialOrderKey object that defines this Abstraction's location within its partial order
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
AbstractionPartialOrder::PartialOrderKeyPtr MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::getPOKey() {
  computePartialOrderKey();
  return partialOrderKey;
}

/* ################################################
   ##### MappedAbstractObject::ConcreteMAOMap #####
   ################################################ */

// Creates an empty map with the same keys as the given MappedAbstractObject
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::ConcreteMAOMap
    (boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > parent): parent(parent) {
  // Initialize data to map the keys in the parent MappedAO to NULL pointers
  for(typename map<Key, AOSubTypePtr >::const_iterator aoIter = parent->aoMap.begin();
      aoIter!=parent->aoMap.end(); ++aoIter)
  data[aoIter->first] = boost::shared_ptr<void>();
}

// Creates a new map based on the given map:
// If emptyMap==true: the new map will be created empty but initialized with the keys of the given map
// Otherwise: the new map will be a shallow copy of the given map (the copy methods of the keys and values are not called)
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::ConcreteMAOMap
    (const ConcreteMAOMap& that, bool emptyMap) {
  for(typename map<Key, boost::shared_ptr<void> >::const_iterator dIter = that.data.begin(); dIter != that.data.end(); ++dIter)
    data[dIter->first] = (emptyMap? boost::shared_ptr<void>():
                                    dIter->second);
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::set(AbstractionPtr mappedAO_arg, setMapFunc& f) {
  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > mappedAO = boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(mappedAO_arg);
  assert(mappedAO);

  for(typename map<Key, AOSubTypePtr >::const_iterator aoIter = mappedAO->aoMap.begin();
      aoIter != mappedAO->aoMap.end(); ++aoIter) {
    typename map<Key, boost::shared_ptr<void> >::iterator dIter = data.find(aoIter->first);
    if(dIter != data.end()) dIter->second = f(aoIter->second, dIter->second, true);
    else                    data[aoIter->first] = f(aoIter->second, boost::shared_ptr<void>(), false);
  }
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::get(AbstractionPtr mappedAO_arg, getMapFunc& f)  {
  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > mappedAO = boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(mappedAO_arg);
  assert(mappedAO);

  for(typename map<Key, AOSubTypePtr >::const_iterator aoIter = mappedAO->aoMap.begin();
      aoIter != mappedAO->aoMap.end(); ++aoIter) {
    typename map<Key, boost::shared_ptr<void> >::iterator dIter = data.find(aoIter->first);
    if(dIter != data.end()) f(aoIter->second, dIter->second, true);
    else                    f(aoIter->second, boost::shared_ptr<void>(), false);
  }
}

// Applies the given functor to all the keys in this map
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::applyStr(applyStrMapFunc& f)  {
  for(typename map<Key, boost::shared_ptr<void> >::iterator dIter = data.begin(); dIter != data.end(); ++dIter) {
    f(dIter->first->str(), dIter->second);
  }
}

// Applies the given functor to all the keys in this map
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::apply(applyMapFunc& f)  {
  for(typename map<Key, boost::shared_ptr<void> >::iterator dIter = data.begin(); dIter != data.end(); ++dIter) {
    f(dIter->second);
  }
}

// Applies the given functor to all the keys in this map and sets the value returned by each invocation
// of the function to the key that corresponds to the call
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::setApply(setApplyMapFunc& f) {
  for(typename map<Key, boost::shared_ptr<void> >::iterator dIter = data.begin(); dIter != data.end(); ++dIter) {
      dIter->second = f(dIter->second);
    }
}

// Applies f to the keys shared by this map and that map
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::applyJoin(MAOMapPtr that_arg, applyJoinMapFunc& f) {
  boost::shared_ptr<ConcreteMAOMap> that = boost::dynamic_pointer_cast<ConcreteMAOMap>(that_arg);
  assert(that);

  for(typename map<Key, boost::shared_ptr<void> >::iterator dThisIter = data.begin(); dThisIter != data.end(); ++dThisIter) {
    typename map<Key, boost::shared_ptr<void> >::const_iterator dThatIter = that->data.find(dThisIter->first);
    if(dThatIter != that->data.end())
      f(dThisIter->second, dThatIter->second);
  }
}

// Applies f to the keys shared by this map and that map. Assigns the return value of each function
// call to the key for which the function was called
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::setJoin(MAOMapPtr that_arg, setJoinMapFunc& f) {
  boost::shared_ptr<ConcreteMAOMap> that = boost::dynamic_pointer_cast<ConcreteMAOMap>(that_arg);
  assert(that);

  for(typename map<Key, boost::shared_ptr<void> >::iterator dThisIter = data.begin(); dThisIter != data.end(); ++dThisIter) {
    typename map<Key, boost::shared_ptr<void> >::const_iterator dThatIter = that->data.find(dThisIter->first);
    if(dThatIter != that->data.end())
      dThisIter->second = f(dThisIter->second, dThatIter->second);
  }
}

// Applies f to the keys shared by maps this map and all the objects in objs.
// Assigns the return value of each function call to the key for which the function was called
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::setObjVecJoin
                 (const std::vector<AbstractionPtr>& objs, setMapObjVecJoinMapFunc& f) {
  // Convert objs, which is a set of generic Abstractions into a vector of MappedAbstractObjects and store it in mappedAOs
  //scope s("MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::setObjVecJoin");
  vector<boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > > mappedAOs;
  for(vector<AbstractionPtr>::const_iterator o=objs.begin(); o!=objs.end(); ++o) {
    //dbg << "o="<<(*o? (*o)->str():"NULL")<<endl;
    if(*o) {
      boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > mappedAO =
          boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(*o);
      //assert(mappedAO);
      if(mappedAO) mappedAOs.push_back(mappedAO);
      else mappedAOs.push_back(boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >());
    // If this object is NULL, add a NULL value for it
    } else
      mappedAOs.push_back(boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >());
  }

  // Iterate over all the keys in this map
  for(typename map<Key, boost::shared_ptr<void> >::iterator dThisIter = data.begin(); dThisIter != data.end(); ++dThisIter) {
    // Find the current key of this map in all the objects in mappedAOs.
    // Store the values mapped in those objects at the current key in subAOs.
    vector<AbstractObjectPtr> subAOs;
    unsigned int i=0;
    for(typename vector<typename boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > >::iterator o=mappedAOs.begin();
        o!=mappedAOs.end(); ++o, ++i) {
      if(*o) {
        typename map<Key, AOSubTypePtr >::const_iterator i=(*o)->aoMap.find(dThisIter->first);
        // If the current key is not found in the current object, skip this key
        if(i==(*o)->aoMap.end()) break;
        subAOs.push_back(i->second);
      // If this object is NULL, add a NULL value for this key
      } else {
        //subAOs.push_back(NULLAbstractObject);
        subAOs.push_back(boost::dynamic_pointer_cast<AbstractObject>(objs[i]));
      }
    }

    // If all the objects had the current key mapped, run the function
    if(subAOs.size() == objs.size())
      dThisIter->second = f(dThisIter->second, subAOs);
  }
}

// Applies f to the keys shared by maps this map, obj and thatMap. Assigns the return value of each function
// call to the key for which the function was called
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::setMapObjMapJoin
      (AbstractionPtr mappedAO_arg, MAOMapPtr thatMap_arg, setMapObjMapJoinMapFunc& f) {
  boost::shared_ptr<ConcreteMAOMap> thatMap = boost::dynamic_pointer_cast<ConcreteMAOMap>(thatMap_arg);
  assert(thatMap);

  boost::shared_ptr<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> > mappedAO = boost::dynamic_pointer_cast<MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType> >(mappedAO_arg);
  assert(mappedAO);

  for(typename map<Key, boost::shared_ptr<void> >::iterator dThisIter = data.begin(); dThisIter != data.end(); ++dThisIter) {
    typename map<Key, AOSubTypePtr >::const_iterator aoIter = mappedAO->aoMap.find(dThisIter->first);
    typename map<Key, boost::shared_ptr<void> >::const_iterator dThatIter = thatMap->data.find(dThisIter->first);
    if(aoIter != mappedAO->aoMap.end() && dThatIter != thatMap->data.end())
      dThisIter->second = f(dThisIter->second, aoIter->second, dThatIter->second);
  }
}

// Applies f to the keys shared by maps this map and the other two maps. Assigns the return value of each function
// call to the key for which the function was called
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
void MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::set3MapJoin
      (MAOMapPtr thatMap1_arg, MAOMapPtr thatMap2_arg, set3MapJoinMapFunc& f) {
  boost::shared_ptr<ConcreteMAOMap> thatMap1 = boost::dynamic_pointer_cast<ConcreteMAOMap>(thatMap1_arg);
  assert(thatMap1);
  boost::shared_ptr<ConcreteMAOMap> thatMap2 = boost::dynamic_pointer_cast<ConcreteMAOMap>(thatMap2_arg);
  assert(thatMap2);

  for(typename map<Key, boost::shared_ptr<void> >::iterator dThisIter = data.begin(); dThisIter != data.end(); ++dThisIter) {
    typename map<Key, boost::shared_ptr<void> >::const_iterator dThatIter1 = thatMap1->data.find(dThisIter->first);
    typename map<Key, boost::shared_ptr<void> >::const_iterator dThatIter2 = thatMap2->data.find(dThisIter->first);
    if(dThatIter1 != thatMap1->data.end() && dThatIter2 != thatMap2->data.end())
      dThisIter->second = f(dThisIter->second, dThatIter1->second, dThatIter2->second);
  }
}

// Returns a freshly-allocated empty instance of this map type that maps
// the keys of this map to void pointers
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
MAOMapPtr MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::create() const
{ return boost::make_shared<ConcreteMAOMap>(*this, /* emptyMap */ true); }

// Returns a freshly-allocated copy of this map that maps the same keys to the same values as the originl
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
MAOMapPtr MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::copy() const
{ return boost::make_shared<ConcreteMAOMap>(*this, /* emptyMap */ false); }

// Returns an instance of a Mapped Abstraction that corresponds to the
// values and keys within this map. It is assumed by this call that the values
// in this map are a sub-type of Abstraction that is compatible with the map's
// implementation. Concretely, maps focused on AbstractObjects will have
// AbstractObject values and maps focused on Lattices will have Lattice values.
template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
AbstractionPtr MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::getMappedObj(uiType ui) {
#ifndef DISABLE_SIGHT
  scope s("MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::ConcreteMAOMap::getMappedObj(uiType ui)");
#endif
  // Convert data, which maps Keys to void pointers into a map where AOSubType pointers are values
  map<Key, AOSubTypePtr > aoMap;
  for(typename map<Key, boost::shared_ptr<void> >::iterator dIter=data.begin(); dIter!=data.end(); ++dIter) {
    AOSubTypePtr val = boost::static_pointer_cast<AOSubType>(dIter->second);
    assert(val);
    aoMap[dIter->first] = val;
  }
#ifndef DISABLE_SIGHT
  dbg << "#aoMap="<<aoMap.size()<<endl;
#endif
  return boost::make_shared<MappedAOSubType>(ui, 0, parent->analysis, aoMap);
}

template<class Key, bool KeyIsComposedAnalysis, class AOSubType, class AOSubTypePtr, AbstractObject::AOType type, class MappedAOSubType>
MAOMapPtr MappedAbstractObject<Key, KeyIsComposedAnalysis, AOSubType, AOSubTypePtr, type, MappedAOSubType>::genMappedAOMap()
{ return boost::make_shared<ConcreteMAOMap>(shared_from_this()); }

/* #########################
   ##### CodeLocObject #####
   ######################### */

CodeLocObjectPtr NULLCodeLocObject;

// Returns whether this object may/must be equal to o within the given Part p
// These methods are called by composers and should not be called by analyses.
bool CodeLocObject::mayEqualAO(CodeLocObjectPtr that, PartEdgePtr pedge) {
  // If either object denotes the set of all parts, they're may-equal
  if (part == NULLPart || that->part == NULLPart)
    return true;

  // If the two objects denote different parts, they're not may-equal
  if (part != that->part)
    return false;

  // If either object denotes the set of all CFGNodes within the same part, they're may-equal
  if (cfgNode.getNode() == NULL || that->cfgNode.getNode() == NULL)
    return true;

  // If the two objects denote different CFGNodes within the same part, they're not equal
  if (cfgNode != that->cfgNode)
    return false;

  // The two objects denote the same part and CFGNode within it
  return true;
}

bool CodeLocObject::mustEqualAO(CodeLocObjectPtr that, PartEdgePtr pedge) {
  // The two objects are must-equal if they denote the same concrete part and CFGNode within it
  return part != NULLPart && that->part != NULLPart && part == that->part
      && cfgNode.getNode() != NULL && that->cfgNode.getNode() != NULL
      && cfgNode == that->cfgNode;
}

// General version of mayEqual and mustEqual that implements may/must equality with respect to ExprObj
// and uses the derived class' may/mustEqual check for all the other cases
// GREG: Currently nothing interesting here since we don't support ExprObjs for CodeLocObjects
bool CodeLocObject::mayEqual(CodeLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  return mayEqualAO(that, pedge);
}

bool CodeLocObject::mustEqual(CodeLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  return mustEqualAO(that, pedge);
}

bool CodeLocObject::mayEqual(AbstractObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  CodeLocObjectPtr co = boost::dynamic_pointer_cast<CodeLocObject>(o);
  if (co)
    return mayEqual(co, pedge, comp, analysis);
  else
    return false;
}

bool CodeLocObject::mustEqual(AbstractObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  //if(AbstractObject::mustEqualExpr(o, pedge)) return true;

  CodeLocObjectPtr co = boost::dynamic_pointer_cast<CodeLocObject>(o);
  if (co)
    return mustEqual(co, pedge, comp, analysis);
  else
    return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
// These methods are called by composers and should not be called by analyses.
bool CodeLocObject::equalSetAO(CodeLocObjectPtr that, PartEdgePtr pedge) {
  return mayEqualAO(that, pedge);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
// These methods are called by composers and should not be called by analyses.
bool CodeLocObject::subSetAO(CodeLocObjectPtr that, PartEdgePtr pedge) {
  // If that object denotes the set of all parts, this is a subset of that
  if (that->part == NULLPart)
    return true;

  // If this object denotes the set of all parts, but that does not, this is not a subset
  if (part == NULLPart)
    return false;

  // If the two objects denote different parts, this is not a subset
  if (part != that->part)
    return false;

  // If that object denotes the set of all CFGNodes within the same part, this is a subset
  if (that->cfgNode.getNode() == NULL)
    return true;

  // If this object denotes the set of all CFGNodes within the same part but that does not, this is not a subset
  if (cfgNode.getNode() == NULL)
    return false;

  // If the two objects denote different CFGNodes within the same part, this is not a subset
  if (cfgNode != that->cfgNode)
    return false;

  // The two objects denote the same part and CFGNode within it
  return true;
}

bool CodeLocObject::equalSet(CodeLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  return equalSetAO(that, pedge);
}

bool CodeLocObject::subSet(CodeLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  return subSetAO(that, pedge);
}

bool CodeLocObject::equalSet(AbstractObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  CodeLocObjectPtr co = boost::dynamic_pointer_cast<CodeLocObject>(o);
  if (co)
    return equalSet(co, pedge, comp, analysis);
  else
    return false;
}

bool CodeLocObject::subSet(AbstractObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  CodeLocObjectPtr co = boost::dynamic_pointer_cast<CodeLocObject>(o);
  if (co)
    return subSet(co, pedge, comp, analysis);
  else
    return false;
}

// Returns true if this object is live at the given part and false otherwise.
// This method is called by composers and should not be called by analyses.
bool CodeLocObject::isLiveAO(PartEdgePtr pedge) {
  // Code Locations are live by definition
  return true;
}

// General version of isLive that accounts for framework details before routing the call to the derived class'
// isLiveCL check. Specifically, it routes the call through the composer to make sure the isLiveCL call gets the
// right PartEdge
bool CodeLocObject::isLive(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  return isLiveAO(pedge);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool CodeLocObject::meetUpdateAO(CodeLocObjectPtr that, PartEdgePtr pedge) {
  // If the two objects denote different parts, set the meet to denote the set of all parts and all CFGNodes
  if (part != that->part) {
    part = NULLPart;
    cfgNode = CFGNode();
    return true;
  }

  // If the two objects denote the same part but different CFGNodes within it, set the meet to denote
  // the set of all CFGNodes within this part
  if (cfgNode != that->cfgNode) {
    cfgNode = CFGNode();
    return true;
  }

  // The two objects are identical
  return true;
}

// General version of meetUpdate that accounts for framework details before routing the call to the derived class'
// meetUpdateCL check. Specifically, it routes the call through the composer to make sure the meetUpdateCL
// call gets the right PartEdge
bool CodeLocObject::meetUpdate(CodeLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  return meetUpdateAO(that, pedge);
}

bool CodeLocObject::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  CodeLocObjectPtr cl = boost::dynamic_pointer_cast<CodeLocObject>(that);
  assert(cl);
  return meetUpdate(cl, pedge, comp, analysis);
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool CodeLocObject::isFullAO(PartEdgePtr pedge) {
  return part == NULLPart && cfgNode.getNode() == NULL;
}

// Returns whether this AbstractObject denotes the empty set.
bool CodeLocObject::isEmptyAO(PartEdgePtr pedge) {
  // It is not possible to create an empty CodeLocObject
  return false;
}

// General versions of isFull() and isEmpty that account for framework details before routing the call to the
// derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make
// sure the isFull(PartEdgePtr) and isEmpty(PartEdgePtr) call gets the right PartEdge.
// These functions are just aliases for the real implementations in AbstractObject
bool CodeLocObject::isFull(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  return isFullAO(pedge);
}

bool CodeLocObject::isEmpty(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  return isEmptyAO(pedge);
}

// Allocates a copy of this object and returns a pointer to it
AbstractObjectPtr CodeLocObject::copyAO() const {
  return copyAOType();
}

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
CodeLocObjectPtr CodeLocObject::project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis) {
  return shared_from_this();
}

std::string CodeLocObject::str(std::string indent) const { // pretty print for the object
  ostringstream oss;
  oss << "[CodeLocObject: part="
      << (part ? part->str(indent + "    ") : "ANY") << ", " << endl;
  oss << indent << "                cfgNode="
      << (cfgNode.getNode() ? CFGNode2Str(cfgNode) : "ANY") << "]";
  return oss.str();
}

/* #################################
   ##### FullCodeLocObject   #####
   ################################# */

bool FullCodeLocObject::mayEqualAO(CodeLocObjectPtr o, PartEdgePtr pedge) {
  return true;
}

bool FullCodeLocObject::mustEqualAO(CodeLocObjectPtr o, PartEdgePtr pedge) {
  return false;
}

bool FullCodeLocObject::equalSetAO(CodeLocObjectPtr o, PartEdgePtr pedge) {
  return isFullAO(pedge);
}

bool FullCodeLocObject::subSetAO(CodeLocObjectPtr o, PartEdgePtr pedge) {
  return isFullAO(pedge);
}

bool FullCodeLocObject::isLiveAO(PartEdgePtr pedge) {
  return true;
}

bool FullCodeLocObject::meetUpdateAO(CodeLocObjectPtr that,
    PartEdgePtr pedge) {
  return false;
}

bool FullCodeLocObject::isFullAO(PartEdgePtr pedge) {
  return true;
}

bool FullCodeLocObject::isEmptyAO(PartEdgePtr pedge) {
  return false;
}

// Returns true if this AbstractObject corresponds to a concrete value that is statically-known
bool FullCodeLocObject::isConcrete() {
  return false;
}

// Returns the number of concrete values in this set
int FullCodeLocObject::concreteSetSize() {
  return -1;
}

CodeLocObjectPtr FullCodeLocObject::copyAOType() const {
  return boost::make_shared<FullCodeLocObject>();
}

string FullCodeLocObject::str(string indent) const {
  return "FullCodeLocObject";
}

/* #################################
   ##### CombinedCodeLocObject   #####
   ################################# */

///*template <bool defaultMayEq>
// CombinedCodeLocObject<defaultMayEq>::CombinedCodeLocObject(CodeLocObjectPtr codeLoc): CodeLocObject(NULL) {
// codeLocs.push_back(codeLoc);
// }
//
// template <bool defaultMayEq>
// CombinedCodeLocObject<defaultMayEq>::CombinedCodeLocObject(const list<CodeLocObjectPtr>& codeLocs) : CodeLocObject(NULL), codeLocs(codeLocs) {}
//
// template <bool defaultMayEq>
// void CombinedCodeLocObject<defaultMayEq>::add(CodeLocObjectPtr codeLoc) {
// codeLocs.push_back(codeLoc);
// }*/
//
//template<bool defaultMayEq>
//SgNode* CombinedCodeLocObject<defaultMayEq>::getBase() const {
//  // Returns the base SgNode shared by all the MemLocs in this object or NULL, if the base SgNodes are different
//  SgNode* base = NULL;
//  for (list<CodeLocObjectPtr>::const_iterator it = codeLocs.begin();
//      it != codeLocs.end(); it++) {
//    if (it == codeLocs.begin())
//      base = (*it)->getBase();
//    else if (base != (*it)->getBase())
//      return NULL;
//  }
//
//  return base;
//}
//
//// Returns whether this object may/must be equal to o within the given Part p
//// These methods are private to prevent analyses from calling them directly.
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::mayEqualAO(CodeLocObjectPtr o,
//    PartEdgePtr pedge) {
//  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that =
//      boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
//  // If the two combination objects include different numbers of CodeLocObjects, say that they may be equal since
//  // we can't be sure either way.
//  if (codeLocs.size() != that->codeLocs.size())
//    return true;
//
//  // Compare all the pairs of CodeLocObjects in codeLocs and that.codeLocs, returning defaultMayEq if any pair
//  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true)
//  // answer that any CodeLocObject in codeLocs can give
//  for (list<CodeLocObjectPtr>::iterator thisIt = codeLocs.begin(), thatIt =
//      that->codeLocs.begin(); thisIt != codeLocs.end(); thisIt++, thatIt++) {
//    if ((*thisIt)->mayEqualAO(*thatIt, pedge) == defaultMayEq)
//      return defaultMayEq;
//  }
//
//  return !defaultMayEq;
//}
//
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::mustEqualAO(CodeLocObjectPtr o,
//    PartEdgePtr pedge) {
//  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that =
//      boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
//  // If the two combination  objects include different numbers of CodeLocObjects, say that they are not must equal since
//  // we can't be sure either way.
//  if (codeLocs.size() != that->codeLocs.size())
//    return false;
//
//  // Compare all the pairs of CodeLocObjects in codeLocs and that.codeLocs, returning !defaultMayEq if any pair
//  // returns !defaultMayEqual since we're looking for the tightest answer that any CodeLocObject in codeLocs can give
//  for (list<CodeLocObjectPtr>::iterator thisIt = codeLocs.begin(), thatIt =
//      that->codeLocs.begin(); thisIt != codeLocs.end(); thisIt++, thatIt++) {
//    if ((*thisIt)->mustEqualAO(*thatIt, pedge) == !defaultMayEq)
//      return !defaultMayEq;
//  }
//
//  return defaultMayEq;
//}
//
//// Returns whether the two abstract objects denote the same set of concrete objects
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::equalSetAO(CodeLocObjectPtr o,
//    PartEdgePtr pedge) {
//  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that =
//      boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
//  assert(that);
//  assert(codeLocs.size() == that->codeLocs.size());
//
//  // Two unions and intersections denote the same set of their components individually denote the same set
//  // (we can get a more precise answer if we could check set containment relations as well)
//  list<CodeLocObjectPtr>::const_iterator clThis = codeLocs.begin();
//  list<CodeLocObjectPtr>::const_iterator clThat = that->codeLocs.begin();
//  for (; clThis != codeLocs.end(); clThis++, clThat++)
//    if (!(*clThis)->equalSetAO(*clThat, pedge))
//      return false;
//  return true;
//}
//
//// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
//// by the given abstract object.
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::subSetAO(CodeLocObjectPtr o,
//    PartEdgePtr pedge) {
//  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that =
//      boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
//  assert(that);
//  assert(codeLocs.size() == that->codeLocs.size());
//
//  // Compare all the pairs of CodeLocObjects in memLocs and that.memLocs, returning defaultMayEq if any pair
//  // returns defaultMayEq since we're looking for the tightest (if defaultMayEq=false) / loosest (if defaultMayEq=true)
//  // answer that any CodeLocObject in memLocs can give
//  for (list<CodeLocObjectPtr>::iterator thisIt = codeLocs.begin(), thatIt =
//      that->codeLocs.begin(); thisIt != codeLocs.end(); thisIt++, thatIt++) {
//    if ((*thisIt)->subSetAO(*thatIt, pedge) == defaultMayEq)
//      return defaultMayEq;
//  }
//  return !defaultMayEq;
//}
//
//// Returns true if this object is live at the given part and false otherwise
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::isLiveAO(PartEdgePtr pedge) {
//  // If this is a union type (defaultMayEq=true), an object is live if any of its components are live (weakest constraint)
//  // If this is an intersection type (defaultMayEq=false), an object is dead if any of its components are dead (strongest constraint)
//  for (list<CodeLocObjectPtr>::const_iterator cl = codeLocs.begin();
//      cl != codeLocs.end(); cl++)
//    if ((*cl)->isLiveAO(pedge) == defaultMayEq)
//      return defaultMayEq;
//
//  return !defaultMayEq;
//}
//
//// Computes the meet of this and that and saves the result in this
//// returns true if this causes this to change and false otherwise
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::meetUpdateAO(CodeLocObjectPtr o,
//    PartEdgePtr pedge) {
//  boost::shared_ptr<CombinedCodeLocObject<defaultMayEq> > that =
//      boost::dynamic_pointer_cast<CombinedCodeLocObject<defaultMayEq> >(o);
//  assert(that);
//  assert(codeLocs.size() == that->codeLocs.size());
//  bool modified = false;
//
//  // Perform the meetUpdate operation on all member codeLocss
//  list<CodeLocObjectPtr>::const_iterator clThis = codeLocs.begin();
//  list<CodeLocObjectPtr>::const_iterator clThat = that->codeLocs.begin();
//  for (; clThis != codeLocs.end(); clThis++, clThat++)
//    modified = (*clThis)->meetUpdateAO(*clThat, pedge) || modified;
//  return modified;
//}
//
//// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::isFullAO(PartEdgePtr pedge) {
//  // If this is a union type (defaultMayEq=true), an object is full if any of its components are full (weakest constraint)
//  // If this is an intersection type (defaultMayEq=false), an object is not full if any of its components are not full (strongest constraint)
//  for (list<CodeLocObjectPtr>::const_iterator cl = codeLocs.begin();
//      cl != codeLocs.end(); cl++)
//    if ((*cl)->isFullAO(pedge) == defaultMayEq)
//      return defaultMayEq;
//
//  return !defaultMayEq;
//}
//
//// Returns whether this AbstractObject denotes the empty set.
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::isEmptyAO(PartEdgePtr pedge) {
//  // If this is a union type (defaultMayEq=true), an object is not empty if any of its components are not empty (weakest constraint)
//  // If this is an intersection type (defaultMayEq=false), an object is empty if any of its components are empty (strongest constraint)
//  for (list<CodeLocObjectPtr>::const_iterator cl = codeLocs.begin();
//      cl != codeLocs.end(); cl++)
//    if ((*cl)->isEmptyAO(pedge) != defaultMayEq)
//      return !defaultMayEq;
//
//  return defaultMayEq;
//}
//
///*// Returns true if this object is live at the given part and false otherwise
// template <bool defaultMayEq>
// bool CombinedCodeLocObject<defaultMayEq>::isLive(PartEdgePtr pedge)
// {
// // If this is a union type (defaultMayEq=true), an object is live if any of its components are live (weakest constraint)
// // If this is an intersection type (defaultMayEq=false), an object is dead if any of its components are dead (strongest constraint)
// for(list<CodeLocObjectPtr>::const_iterator cl=codeLocs.begin(); cl!=codeLocs.end(); cl++)
// if((*cl)->isLive(pedge) == defaultMayEq) return defaultMayEq;
//
// return !defaultMayEq;
// }*/
//
//// Returns true if this CodeLocObject corresponds to a concrete value that is statically-known
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::isConcrete() {
//  // The combined object is concrete if
//  // intersect (defaultMayEq=false) : any sub-cl is concrete
//  // union (defaultMayEq=true) : all the sub-cl are concrete
//
//  // Intersection
//  if (defaultMayEq == false) {
//    for (list<CodeLocObjectPtr>::iterator cl = codeLocs.begin();
//        cl != codeLocs.end(); cl++) {
//      if ((*cl)->isConcrete())
//        return true;
//    }
//    return false;
//    // Union
//  } else {
//    assert(codeLocs.size() > 0);
//    // The union is not concrete if
//    for (list<CodeLocObjectPtr>::iterator cl = codeLocs.begin();
//        cl != codeLocs.end(); cl++) {
//      // Any sub-value is not concrete
//      if (!(*cl)->isConcrete())
//        return false;
//    }
//    return true;
//  }
//}
//
//// Returns the number of concrete values in this set
//template<bool defaultMayEq>
//int CombinedCodeLocObject<defaultMayEq>::concreteSetSize() {
//  assert(isConcrete());
//  // This is an over-approximation of the set size that assumes that all the concrete sets of
//  // the sub-CodeLocs are disjoint
//  int size = 0;
//  for (list<CodeLocObjectPtr>::const_iterator cl = codeLocs.begin();
//      cl != codeLocs.end(); cl++)
//    size += (*cl)->concreteSetSize();
//  return size;
//}
//
//// Allocates a copy of this object and returns a pointer to it
//template<bool defaultMayEq>
//CodeLocObjectPtr CombinedCodeLocObject<defaultMayEq>::copyAOType() const {
//  return boost::make_shared<CombinedCodeLocObject>(codeLocs);
//}
//
//template<bool defaultMayEq>
//std::string CombinedCodeLocObject<defaultMayEq>::str(
//    std::string indent) const {
//  ostringstream oss;
//  if (codeLocs.size() > 1)
//    oss << "[" << (defaultMayEq ? "UnionCL" : "IntersectCL") << ": ";
//  if (codeLocs.size() > 1)
//    oss << endl;
//  for (list<CodeLocObjectPtr>::const_iterator cl = codeLocs.begin();
//      cl != codeLocs.end();) {
//    if (cl != codeLocs.begin())
//      oss << indent << "&nbsp;&nbsp;&nbsp;&nbsp;";
//    oss << (*cl)->str(indent + "&nbsp;&nbsp;&nbsp;&nbsp;");
//    cl++;
//    if (cl != codeLocs.end())
//      oss << endl;
//  }
//  if (codeLocs.size() > 1)
//    oss << "]";
//
//  return oss.str();
//}
//
//// Returns whether all instances of this class form a hierarchy. Every instance of the same
//// class created by the same analysis must return the same value from this method!
//template<bool defaultMayEq>
//bool CombinedCodeLocObject<defaultMayEq>::isHierarchy() const {
//  // Combined CodeLocs form hierarchy if:
//  // - All the sub-CodeLocs form hierarchies of their own, AND
//  // - The combination is an intersection.
//  //   If the combination is a union then consider the following:
//  //            MLa     MLb
//  //   comb1 = {a,b}, {w, x}
//  //   comb2 = {a,b}, {y, z}
//  //   Note that MLs from analyses a and b are either identical sets or disjoint
//  //   However, comb1 U comb2 = {a, b, w, x, y, z}, for which this property does
//  //   not hold unless we work out a new hierarchy for MLb.
//
//  // Unions are not hierarchical unless they're singletons
//  if (defaultMayEq) {
//    if (codeLocs.size() == 1)
//      return (*codeLocs.begin())->isHierarchy();
//    else
//      return false;
//  }
//
//  for (list<CodeLocObjectPtr>::const_iterator ml = codeLocs.begin();
//      ml != codeLocs.end(); ml++)
//    if (!(*ml)->isHierarchy())
//      return false;
//  return true;
//}
//
//// Returns a key that uniquely identifies this particular AbstractObject in the
//// set hierarchy.
//template<bool defaultMayEq>
//const AbstractionHierarchy::hierKeyPtr& CombinedCodeLocObject<defaultMayEq>::getHierKey() const {
//  // The intersection of multiple objects is just a filtering process from the full
//  // set of objects to the exact one, with each key in the hierarchy further filtering
//  // the set down. As such, a hierarchical key for the intersection of multiple objects
//  // is just the concatenation of the keys of all the individual objects.
//  if (!isHierKeyCached) {
//    /*((CombinedCodeLocObject<defaultMayEq>*)this)->cachedHierKey = boost::make_shared<AOSHierKey>(((CombinedCodeLocObject<defaultMayEq>*)this)->shared_from_this());
//
//     for(list<CodeLocObjectPtr>::const_iterator i=codeLocs.begin(); i!=codeLocs.end(); i++) {
//     AbstractionHierarchyPtr hierIt = boost::dynamic_pointer_cast<AbstractionHierarchy>(*i);
//     ROSE_ASSERT(hierIt);
//
//     ((CombinedCodeLocObject<defaultMayEq>*)this)->cachedHierKey->add(hierIt->getHierKey()->begin(), hierIt->getHierKey()->end());
//     }*/
//    list<hierKeyPtr> subHierKeys;
//    for (list<CodeLocObjectPtr>::const_iterator i = codeLocs.begin(); i != codeLocs.end(); i++) {
//      AbstractionHierarchyPtr hierIt = boost::dynamic_pointer_cast<AbstractionHierarchy>(*i);
//      ROSE_ASSERT(hierIt);
//      subHierKeys.push_back(hierIt->getHierKey());
//    }
//
//    ((CombinedCodeLocObject<defaultMayEq>*) this)->cachedHierKey = boost::make_shared<IntersectHierKey>(subHierKeys);
//
//    ((CombinedCodeLocObject<defaultMayEq>*) this)->isHierKeyCached = true;
//  }
//  return cachedHierKey;
//}
//
//// Create a function that uses examples of combined objects to force the compiler to generate these classes
//static void exampleCombinedCodeLocObjects2(CodeLocObjectPtr cl,
//    std::list<CodeLocObjectPtr> cls, IntersectCodeLocObject& i,
//    UnionCodeLocObject& u, IntersectCodeLocObject& i2,
//    UnionCodeLocObject& u2);
//static void exampleCombinedCodeLocObjects(CodeLocObjectPtr cl,
//    std::list<CodeLocObjectPtr> cls) {
//  IntersectCodeLocObject exampleIntersectObject(cl);
//  UnionCodeLocObject exampleUnionObject(cl);
//  IntersectCodeLocObject exampleIntersectObject2(cls);
//  UnionCodeLocObject exampleUnionObject2(cls);
//  exampleCombinedCodeLocObjects2(cl, cls, exampleIntersectObject,
//      exampleUnionObject, exampleIntersectObject2, exampleUnionObject2);
//}
//static void exampleCombinedCodeLocObjects2(CodeLocObjectPtr cl,
//    std::list<CodeLocObjectPtr> cls, IntersectCodeLocObject& i,
//    UnionCodeLocObject& u, IntersectCodeLocObject& i2,
//    UnionCodeLocObject& u2) {
//  exampleCombinedCodeLocObjects(cl, cls);
//}

/* ##############################
   # PartEdgeUnionCodeLocObject #
   ############################## */

PartEdgeUnionCodeLocObject::PartEdgeUnionCodeLocObject() :
    CodeLocObject(NULL) {
}

PartEdgeUnionCodeLocObject::PartEdgeUnionCodeLocObject(
    const PartEdgeUnionCodeLocObject& thatCL) :
    CodeLocObject(thatCL), unionCL_p(thatCL.unionCL_p->copyAOType()) {
}

SgNode* PartEdgeUnionCodeLocObject::getBase() const {
  return unionCL_p->getBase();
}

void PartEdgeUnionCodeLocObject::add(CodeLocObjectPtr cl_p, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // If this is the very first object
  if (!unionCL_p)
    unionCL_p = cl_p->copyAOType();
  // If Full return without adding
  else if (isFullAO(pedge))
    return;
  // Else meetUpdate with the existing unionCL_p
  else
    unionCL_p->meetUpdateAO(cl_p, pedge);
}

bool PartEdgeUnionCodeLocObject::mayEqualAO(CodeLocObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p =
      boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->mayEqualAO(thatCL_p->getUnionAO(), pedge);
}

bool PartEdgeUnionCodeLocObject::mustEqualAO(CodeLocObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p =
      boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->mustEqualAO(thatCL_p->getUnionAO(), pedge);
}

bool PartEdgeUnionCodeLocObject::equalSetAO(CodeLocObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p =
      boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->equalSetAO(thatCL_p->getUnionAO(), pedge);
}

bool PartEdgeUnionCodeLocObject::subSetAO(CodeLocObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p =
      boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->subSetAO(thatCL_p->getUnionAO(), pedge);
}

bool PartEdgeUnionCodeLocObject::meetUpdateAO(CodeLocObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionCodeLocObject> thatCL_p =
      boost::dynamic_pointer_cast<PartEdgeUnionCodeLocObject>(that);
  assert(thatCL_p);
  return unionCL_p->meetUpdateAO(thatCL_p->getUnionAO(), pedge);
}

bool PartEdgeUnionCodeLocObject::isLiveAO(PartEdgePtr pedge) {
  return unionCL_p->isLiveAO(pedge);
}

bool PartEdgeUnionCodeLocObject::isFullAO(PartEdgePtr pedge) {
  return unionCL_p->isFullAO(pedge);
}

bool PartEdgeUnionCodeLocObject::isEmptyAO(PartEdgePtr pedge) {
  return unionCL_p->isEmptyAO(pedge);
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool PartEdgeUnionCodeLocObject::isConcrete() {
  return unionCL_p->isConcrete();
}

// Returns the number of concrete values in this set
int PartEdgeUnionCodeLocObject::concreteSetSize() {
  return unionCL_p->concreteSetSize();
}

CodeLocObjectPtr PartEdgeUnionCodeLocObject::copyAOType() const {
  return boost::make_shared<PartEdgeUnionCodeLocObject>(*this);
}

void PartEdgeUnionCodeLocObject::setAOToFull() {
  unionCL_p = boost::make_shared<FullCodeLocObject>();
}

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
CodeLocObjectPtr PartEdgeUnionCodeLocObject::project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis)
{ return unionCL_p->project(analysis, pedge, comp, clientAnalysis); }

string PartEdgeUnionCodeLocObject::str(string indent) const {
  ostringstream oss;
  oss << "[UnionCL=" << unionCL_p->str(indent) << "]";
  return oss.str();
}

// Returns whether all instances of this class form a hierarchy. Every instance of the same
// class created by the same analysis must return the same value from this method!
bool PartEdgeUnionCodeLocObject::isHierarchy() const {
  return unionCL_p->isHierarchy();
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& PartEdgeUnionCodeLocObject::getHierKey() const {
  return unionCL_p->getHierKey();
}

// ----------------------------------------
// Objects that denote disjoint sets. Because no two sets may overlap, they can be
// represented using unique numbers, which enables efficient data structure implementations.
bool PartEdgeUnionCodeLocObject::isDisjoint() const
{ return unionCL_p->isDisjoint(); }

/* #######################
   ##### ValueObject #####
   ####################### */

ValueObjectPtr NULLValueObject;

// Returns whether this object may/must be equal to o within the given Part p
// by propagating the call through the composer
bool ValueObject::mayEqual(ValueObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  /*if(mayEqualCache.find(o) == mayEqualCache.end())
   mayEqualCache[o] = comp->mayEqualAO(shared_from_this(), o, pedge, analysis);
   return mayEqualCache[o];*/
  return comp->mayEqualV(shared_from_this(), o, pedge, analysis);
}

// Returns whether this object may/must be equal to o within the given Part p
// by propagating the call through the composer
bool ValueObject::mustEqual(ValueObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  /*if(mustEqualCache.find(o) == mustEqualCache.end())
   mustEqualCache[o] = comp->mustEqualAO(shared_from_this(), o, pedge, analysis);
   return mustEqualCache[o];*/
  return comp->mustEqualV(shared_from_this(), o, pedge, analysis);
}

// Returns whether this object may/must be equal to o within the given Part p
// by propagating the call through the composer
bool ValueObject::mayEqual(AbstractObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  //if(AbstractObject::mustEqualExpr(boost::static_pointer_cast<AbstractObject>(o), pedge)) return true;

  ValueObjectPtr vo = boost::dynamic_pointer_cast<ValueObject>(o);
  if (vo)
    return mayEqual(vo, pedge, comp, analysis);
  else
    return false;
}

// Returns whether this object may/must be equal to o within the given Part p
// by propagating the call through the composer
bool ValueObject::mustEqual(AbstractObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  //if(AbstractObject::mustEqualExpr(boost::static_pointer_cast<AbstractObject>(o), pedge)) return true;

  ValueObjectPtr vo = boost::dynamic_pointer_cast<ValueObject>(o);
  if (vo)
    return mustEqual(vo, pedge, comp, analysis);
  else
    return false;
}

bool ValueObject::equalSet(ValueObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  /*if(equalSetCache.find(that) == equalSetCache.end())
   equalSetCache[that] = comp->equalSetAO(shared_from_this(), that, pedge, analysis);
   return equalSetCache[that];*/
  return comp->equalSetV(shared_from_this(), that, pedge, analysis);
}

bool ValueObject::subSet(ValueObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  /*if(subSetCache.find(that) == subSetCache.end())
   subSetCache[that] = comp->equalSetAO(shared_from_this(), that, pedge, analysis);
   return subSetCache[that];*/
  return comp->equalSetV(shared_from_this(), that, pedge, analysis);
}

bool ValueObject::equalSet(AbstractObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  ValueObjectPtr co = boost::dynamic_pointer_cast<ValueObject>(o);
  if (co)
    return equalSet(co, pedge, comp, analysis);
  else
    return false;
}

bool ValueObject::subSet(AbstractObjectPtr o, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  ValueObjectPtr co = boost::dynamic_pointer_cast<ValueObject>(o);
  if (co)
    return subSet(co, pedge, comp, analysis);
  else
    return false;
}

// General version of meetUpdate that accounts for framework details before routing the call to the derived class'
// meetUpdateV check. Specifically, it routes the call through the composer to make sure the meetUpdateV
// call gets the right PartEdge
bool ValueObject::meetUpdate(ValueObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  return comp->meetUpdateV(shared_from_this(), that, pedge, analysis);
}

bool ValueObject::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  ValueObjectPtr v = boost::dynamic_pointer_cast<ValueObject>(that);
  assert(v);
  return meetUpdate(v, pedge, comp, analysis);
}

// General version of isFull/isEmpty that accounts for framework details before routing the call to the
// derived class' isFullV/isEmptyV check. Specifically, it routes the call through the composer to make
// sure the isFullV/isEmptyV call gets the right PartEdge
bool ValueObject::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  return comp->isFullV(shared_from_this(), pedge, analysis);
}

bool ValueObject::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  return comp->isEmptyV(shared_from_this(), pedge, analysis);
}

// Returns true if this SgValueExp is convertible into a boolean
bool ValueObject::isValueBoolCompatible(boost::shared_ptr<SgValueExp> val) {
  return isSgCharVal(val.get()) || isSgBoolValExp(val.get())
      || isSgEnumVal(val.get()) || isSgIntVal(val.get())
      || isSgLongIntVal(val.get()) || isSgLongLongIntVal(val.get())
      || isSgShortVal(val.get()) || isSgUnsignedCharVal(val.get())
      || isSgUnsignedLongVal(val.get())
      || isSgUnsignedLongLongIntVal(val.get())
      || isSgUnsignedShortVal(val.get()) || isSgWcharVal(val.get());
}

// Convert the value of the given SgValueExp, cast to a boolean
bool ValueObject::SgValue2Bool(boost::shared_ptr<SgValueExp> val) {
  if (isSgCharVal(val.get()))
    return isSgCharVal(val.get())->get_value();
  else if (isSgBoolValExp(val.get()))
    return isSgBoolValExp(val.get())->get_value();
  else if (isSgEnumVal(val.get()))
    return isSgEnumVal(val.get())->get_value();
  else if (isSgIntVal(val.get()))
    return isSgIntVal(val.get())->get_value();
  else if (isSgLongIntVal(val.get()))
    return isSgLongIntVal(val.get())->get_value();
  else if (isSgLongLongIntVal(val.get()))
    return isSgLongLongIntVal(val.get())->get_value();
  else if (isSgShortVal(val.get()))
    return isSgShortVal(val.get())->get_value();
  else if (isSgUnsignedCharVal(val.get()))
    return isSgUnsignedCharVal(val.get())->get_value();
  else if (isSgUnsignedLongVal(val.get()))
    return isSgUnsignedLongVal(val.get())->get_value();
  else if (isSgUnsignedLongLongIntVal(val.get()))
    return isSgUnsignedLongLongIntVal(val.get())->get_value();
  else if (isSgUnsignedShortVal(val.get()))
    return isSgUnsignedShortVal(val.get())->get_value();
  else if (isSgWcharVal(val.get()))
    return isSgWcharVal(val.get())->get_valueUL();
  else {
#ifndef DISABLE_SIGHT
    dbg << "val=" << SgNode2Str(val.get()) << endl;
#endif
    assert(0);
  }
}

// Allocates a copy of this object and returns a pointer to it
AbstractObjectPtr ValueObject::copyAO() const {
  return copyAOType();
}

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
ValueObjectPtr ValueObject::project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis) {
  return shared_from_this();
}

// Returns true if the two SgValueExps correspond to the same value when cast to the given type (if t!=NULL)
bool ValueObject::equalValueExp(SgValueExp* e1, SgValueExp* e2, SgType* t) {
  // Currently not handling type conversions
  assert(t==NULL);

  if (e1->variantT() != e2->variantT())
    return false;

  if (isSgBoolValExp(e1)) {
    return isSgBoolValExp(e1)->get_value() == isSgBoolValExp(e2)->get_value();
  }
  if (isSgCharVal(e1)) {
    return isSgCharVal(e1)->get_value() == isSgCharVal(e2)->get_value();
  }
  if (isSgComplexVal(e1)) {
    return equalValueExp(isSgComplexVal(e1)->get_real_value(),
        isSgComplexVal(e2)->get_real_value())
        && equalValueExp(isSgComplexVal(e1)->get_imaginary_value(),
            isSgComplexVal(e2)->get_imaginary_value());
  }
  if (isSgDoubleVal(e1)) {
    return isSgDoubleVal(e1)->get_value() == isSgDoubleVal(e2)->get_value();
  }
  if (isSgEnumVal(e1)) {
    return isSgEnumVal(e1)->get_value() == isSgEnumVal(e2)->get_value();
  }
  if (isSgFloatVal(e1)) {
    return isSgFloatVal(e1)->get_value() == isSgFloatVal(e2)->get_value();
  }
  if (isSgIntVal(e1)) {
    return isSgIntVal(e1)->get_value() == isSgIntVal(e2)->get_value();
  }
  if (isSgLongDoubleVal(e1)) {
    return isSgLongDoubleVal(e1)->get_value()
        == isSgLongDoubleVal(e2)->get_value();
  }
  if (isSgLongIntVal(e1)) {
    return isSgLongIntVal(e1)->get_value() == isSgLongIntVal(e2)->get_value();
  }
  if (isSgLongLongIntVal(e1)) {
    return isSgLongLongIntVal(e1)->get_value()
        == isSgLongLongIntVal(e2)->get_value();
  }
  if (isSgShortVal(e1)) {
    return isSgShortVal(e1)->get_value() == isSgShortVal(e2)->get_value();
  }
  if (isSgStringVal(e1)) {
    return isSgStringVal(e1)->get_value() == isSgStringVal(e2)->get_value();
  }
  if (isSgWcharVal(e1)) {
    return isSgWcharVal(e1)->get_value() == isSgWcharVal(e2)->get_value();
  }
  if (isSgUnsignedCharVal(e1)) {
    return isSgUnsignedCharVal(e1)->get_value()
        == isSgUnsignedCharVal(e2)->get_value();
  }
  if (isSgUnsignedIntVal(e1)) {
    return isSgUnsignedIntVal(e1)->get_value()
        == isSgUnsignedIntVal(e2)->get_value();
  }
  if (isSgUnsignedLongLongIntVal(e1)) {
    return isSgUnsignedLongLongIntVal(e1)->get_value()
        == isSgUnsignedLongLongIntVal(e2)->get_value();
  }
  if (isSgUnsignedLongVal(e1)) {
    return isSgUnsignedLongVal(e1)->get_value()
        == isSgUnsignedLongVal(e2)->get_value();
  }
  if (isSgUnsignedShortVal(e1)) {
    return isSgWcharVal(e1)->get_value()
        == isSgUnsignedShortVal(e2)->get_value();
  }
  if (isSgUpcMythread(e1)) {
    return isSgUpcMythread(e1)->get_value()
        == isSgUpcMythread(e2)->get_value();
  }
  if (isSgUpcThreads(e1)) {
    return isSgUpcThreads(e1)->get_value() == isSgUpcThreads(e2)->get_value();
  }

  assert(0);
}

/* ##############################
   ##### FullValueObject   #####
   ############################## */

// Returns whether this object may/must be equal to o within the given Part p
// These methods are private to prevent analyses from calling them directly.
bool FullValueObject::mayEqualAO(ValueObjectPtr o, PartEdgePtr pedge) {
  // Since this object denotes the set of all values, it may-equals all value sets
  return true;
}

bool FullValueObject::mustEqualAO(ValueObjectPtr o, PartEdgePtr pedge) {
  // Since this object denotes the set of all values, which has unbounded size, it is not must-equal to any value set
  return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool FullValueObject::equalSetAO(ValueObjectPtr o, PartEdgePtr pedge) {
  // This object is only equal to objects that also denote the set of all values
  return o->isFullAO(pedge);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool FullValueObject::subSetAO(ValueObjectPtr o, PartEdgePtr pedge) {
  // This object is only a subset of objects that also denote the set of all values
  return o->isFullAO(pedge);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool FullValueObject::meetUpdateAO(ValueObjectPtr that, PartEdgePtr pedge) {
  // There is no way to make this object denote a larger set of values since it already denotes
  // the set of all values
  return false;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool FullValueObject::isFullAO(PartEdgePtr pedge) {
  return true;
}

// Returns whether this AbstractObject denotes the empty set.
bool FullValueObject::isEmptyAO(PartEdgePtr pedge) {
  return false;
}

// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool FullValueObject::isConcrete() {
  return false;
}

// Returns the number of concrete values in this set
int FullValueObject::concreteSetSize() {
  return -1;
}

// Returns the type of the concrete value (if there is one)
SgType* FullValueObject::getConcreteType() {
  return NULL;
}

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
set<boost::shared_ptr<SgValueExp> > FullValueObject::getConcreteValue() {
  return set<boost::shared_ptr<SgValueExp> >();
}

// Allocates a copy of this object and returns a pointer to it
ValueObjectPtr FullValueObject::copyAOType() const {
  return boost::make_shared<FullValueObject>();
}

std::string FullValueObject::str(std::string indent) const {
  return "[FullValueObject]";
}

/* ###############################
   ##### CombinedValueObject #####
   ############################### */

/* ###########################
   #### MappedValueObject ####
   ########################### */

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType>
SgType* MappedValueObject<Key, KeyIsComposedAnalysis, MappedAOSubType>::getConcreteType() {
  if(!MappedValueObject<Key, KeyIsComposedAnalysis, MappedAOSubType>::isConcrete()) assert(0);
  SgType* c_type = NULL;
  // assert that all other objects have the same type
  for(typename map<Key, ValueObjectPtr>::iterator it =
          MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>::aoMap.begin();
      it != MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>::aoMap.end(); ++it) {
    if (it->second->isConcrete()) {
      SgType* votype = it->second->getConcreteType();
      if(c_type==NULL) c_type = votype;
      else             assert(c_type == votype);
    }
  }
  return c_type;
}

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType>
set<boost::shared_ptr<SgValueExp> > MappedValueObject<Key, KeyIsComposedAnalysis, MappedAOSubType>::getConcreteValue() {
  //scope s("MappedValueObject<Key, KeyIsComposedAnalysis, MappedAOSubType>::getConcreteValue()");
  if(!MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>::isConcrete()) assert(0);
  // If this is a union type (defaultMayEq=true), the result is the Union of the sets returned by getConcrete() on all the memRegions.
  // If this is an intersection type (defaultMayEq=false), an object is their Intersection.

  // Maps each concrete value to the number of elements in aoMap for which it was returned
  std::map<boost::shared_ptr<SgValueExp>, size_t> concreteVals;
  // Number of concrete sub-objects
  unsigned int numConcrete=0;
  for(typename map<Key, ValueObjectPtr>::iterator v_it =
          MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>::aoMap.begin();
      v_it != MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>::aoMap.end(); ++v_it) {
    /*scope s(txt()<<"Value "<<v_it->second->str());
    dbg << "isConcrete = "<<v_it->second->isConcrete()<<endl;*/
    if (v_it->second->isConcrete()) {
      // Iterate through the current sub-MemRegion's concrete values and increment each
      // concrete value's counter in concreteMRs.
      std::set<boost::shared_ptr<SgValueExp> > c_valueSet =
          v_it->second->getConcreteValue();
      //dbg << "#c_valueSet="<<c_valueSet.size()<<endl;
      for(std::set<boost::shared_ptr<SgValueExp> >::iterator s_it =
          c_valueSet.begin(); s_it != c_valueSet.end(); ++s_it) {
        //indent ind;
        //dbg << SgNode2Str((*s_it).get());
        map<boost::shared_ptr<SgValueExp>, size_t>::iterator c_it =
            concreteVals.begin();
        for (; c_it != concreteVals.end(); ++c_it) {
          // If we've found the same value, increment its counter
          if (ValueObject::equalValueExp(c_it->first.get(), (*s_it).get())) {
#ifndef DISABLE_SIGHT
            indent ind;
#endif
            c_it->second++;
            //dbg << "found, count="<<c_it->second<<endl;
            break;
          }
        }

        // If we did not find the value, add it to concreteVals;
        if (c_it == concreteVals.end())
          concreteVals[*s_it] = 1;
      }

      ++numConcrete;
    }
  }

  // Collect the union or intersection of all results from concreteMRs as a set
  std::set<boost::shared_ptr<SgValueExp> > ret;
  for (std::map<boost::shared_ptr<SgValueExp>, size_t>::iterator i =
      concreteVals.begin(); i != concreteVals.end(); i++) {
    // Union: add every key in concreteMRs to ret
    if (MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>::isUnion())
      ret.insert(i->first);
    // Intersection: only add the keys that appear in every MemRegion in memRegions
    else if (MappedAbstractObject<Key, KeyIsComposedAnalysis, ValueObject, ValueObjectPtr, AbstractObject::Value, MappedAOSubType>::isIntersection() &&
             i->second == numConcrete)
      ret.insert(i->first);
  }

  return ret;
}

/* ##################################
   #### PartEdgeUnionValueObject ####
   ################################## */

PartEdgeUnionValueObject::PartEdgeUnionValueObject() :
    ValueObject(NULL) {
}

PartEdgeUnionValueObject::PartEdgeUnionValueObject(
    const PartEdgeUnionValueObject& thatV) :
    ValueObject(thatV), unionV_p(thatV.unionV_p->copyAOType()) {
}

SgNode* PartEdgeUnionValueObject::getBase() const {
  return unionV_p->getBase();
}

void PartEdgeUnionValueObject::add(ValueObjectPtr v_p, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // If this is the very first object
  if (!unionV_p)
    unionV_p = v_p->copyAOType();
  // If Full return without adding
  else if (isFullAO(pedge))
    return;
  // Else meetUpdate with the existing unionV_p
  else
    unionV_p->meetUpdateAO(v_p, pedge);
}

bool PartEdgeUnionValueObject::mayEqualAO(ValueObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p =
      boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->mayEqualAO(thatV_p->getUnionAO(), pedge);
}

bool PartEdgeUnionValueObject::mustEqualAO(ValueObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p =
      boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->mustEqualAO(thatV_p->getUnionAO(), pedge);
}

bool PartEdgeUnionValueObject::equalSetAO(ValueObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p =
      boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->equalSetAO(thatV_p->getUnionAO(), pedge);
}

bool PartEdgeUnionValueObject::subSetAO(ValueObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p =
      boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->subSetAO(thatV_p->getUnionAO(), pedge);
}

bool PartEdgeUnionValueObject::meetUpdateAO(ValueObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionValueObject> thatV_p =
      boost::dynamic_pointer_cast<PartEdgeUnionValueObject>(that);
  assert(thatV_p);
  return unionV_p->meetUpdateAO(thatV_p->getUnionAO(), pedge);
}

bool PartEdgeUnionValueObject::isLiveAO(PartEdgePtr pedge) {
  return unionV_p->isLiveAO(pedge);
}

bool PartEdgeUnionValueObject::isFullAO(PartEdgePtr pedge) {
  return unionV_p->isFullAO(pedge);
}

bool PartEdgeUnionValueObject::isEmptyAO(PartEdgePtr pedge) {
  return unionV_p->isEmptyAO(pedge);
}

ValueObjectPtr PartEdgeUnionValueObject::copyAOType() const {
  return boost::make_shared<PartEdgeUnionValueObject>(*this);
}

void PartEdgeUnionValueObject::setAOToFull() {
  unionV_p = boost::make_shared<FullValueObject>();
}

bool PartEdgeUnionValueObject::isConcrete() {
  return unionV_p->isConcrete();
}

// Returns the number of concrete values in this set
int PartEdgeUnionValueObject::concreteSetSize() {
  return unionV_p->concreteSetSize();
}

SgType* PartEdgeUnionValueObject::getConcreteType() {
  return unionV_p->getConcreteType();
}

set<boost::shared_ptr<SgValueExp> > PartEdgeUnionValueObject::getConcreteValue() {
  return unionV_p->getConcreteValue();
}

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
ValueObjectPtr PartEdgeUnionValueObject::project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis)
{ return unionV_p->project(analysis, pedge, comp, clientAnalysis); }

string PartEdgeUnionValueObject::str(string indent) const {
  ostringstream oss;
  oss << "[UnionV=" << (unionV_p? unionV_p->str(indent): "NULL") << "]";
  return oss.str();
}


// Returns whether all instances of this class form a hierarchy. Every instance of the same
// class created by the same analysis must return the same value from this method!
bool PartEdgeUnionValueObject::isHierarchy() const {
  return unionV_p->isHierarchy();
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& PartEdgeUnionValueObject::getHierKey() const {
  return unionV_p->getHierKey();
}

// ----------------------------------------
// Objects that denote disjoint sets. Because no two sets may overlap, they can be
// represented using unique numbers, which enables efficient data structure implementations.
bool PartEdgeUnionValueObject::isDisjoint() const
{ return unionV_p->isDisjoint(); }

/* ###########################
   ##### MemRegionObject #####
   ########################### */

MemRegionObjectPtr NULLMemRegionObject;

// Returns the relationship between the given AbstractObjects, considering whether either
// or both are FuncResultMemRegionObjects and if they refer to the same function
FuncResultRelationType MemRegionObject::getFuncResultRel(AbstractObject* one, AbstractObjectPtr two, PartEdgePtr pedge) {
  // If either this or that are FuncResultMemRegionObject, they mayEqual iff they correspond to the same function
  FuncResultMemRegionObject* frmlcoOne = dynamic_cast<FuncResultMemRegionObject*>(one);
  FuncResultMemRegionObjectPtr frmlcoTwo = boost::dynamic_pointer_cast<FuncResultMemRegionObject>(two);
  if (frmlcoOne) {
    if(frmlcoOne->mustEqualAO(frmlcoTwo, pedge)) return FuncResultSameFunc;
    else                                         return FuncResultUnequal;
  } else if (frmlcoTwo)
    return FuncResultUnequal;
  else
    return NeitherFuncResult;
}

// General version of mayEqual and mustEqual that accounts for framework details before routing the call to the
// derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
// the call through the composer to make sure the may/mustEqual call gets the right PartEdge
bool MemRegionObject::mayEqual(MemRegionObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // If both this and that are both expression objects or both not expression objects, use the
  // derived class' equality check
  //dbg << "MemRegionObject::mayEqual() dynamic_cast<const ExprObj*>(this)="<<dynamic_cast<const ExprObj*>(this)<<" dynamic_cast<const ExprObj*>(o.get())="<<dynamic_cast<const ExprObj*>(o.get())<<endl;

  // GB 2013-09-16 - Commenting this out since we currently don't have code to return specifically expression
  //                 regions but this capability may be brought back for performance reasons (equality checks
  //                 can be sped up with we keep track of expression objects, for which identity is easy to check)
  // if((dynamic_cast<const ExprObj*>(this)  && dynamic_cast<const ExprObj*>(that.get())) ||
  //    (!dynamic_cast<const ExprObj*>(this) && !dynamic_cast<const ExprObj*>(that.get())))
  // { return mayEqualAO(that, pedge); }
  /*if(mayEqualCache.find(that) == mayEqualCache.end())
   // Route the check through the composer, which makes sure to call the derived class' check at the correct PartEdge
   mayEqualCache[that] = comp->mayEqualAO(shared_from_this(), that, pedge, analysis);
   return mayEqualCache[that];*/
  return comp->mayEqualMR(shared_from_this(), that, pedge, analysis);
}

// General version of mayEqual and mustEqual that accounts for framework details before routing the call to the
// derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
// the call through the composer to make sure the may/mustEqual call gets the right PartEdge
bool MemRegionObject::mustEqual(MemRegionObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {

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
  // //{ return mustEqualAO(that, pedge); }

  /*if(mustEqualCache.find(that) == mustEqualCache.end())
   // Route the check through the composer, which makes sure to call the derived class' check at the correct PartEdge
   mustEqualCache[that] = comp->mustEqualAO(shared_from_this(), that, pedge, analysis);
   return mustEqualCache[that];*/
  return comp->mustEqualMR(shared_from_this(), that, pedge, analysis);
}

// Check whether that is a MemRegionObject and if so, call the version of mayEqual specific to MemRegionObjects
bool MemRegionObject::mayEqual(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemRegionObject, the two objects only overlap if either is full.
  // We do this check early because casting to FullMemRegionObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that)) return true;
  if (dynamic_cast<FullMemRegionObject*>(this))               return true;

  MemRegionObjectPtr mo = boost::dynamic_pointer_cast<MemRegionObject>(that);
  if (mo) return mayEqual(mo, pedge, comp, analysis);
  else    return false;
}

// Check whether that is a MemRegionObject and if so, call the version of mustEqual specific to MemRegionObjects
bool MemRegionObject::mustEqual(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemRegionObject, the two objects only must-equal if neither is full.
  // We do this check early because casting to FullMemRegionObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that)) return false;
  if (dynamic_cast<FullMemRegionObject*>(this))               return false;

  MemRegionObjectPtr mo = boost::dynamic_pointer_cast<MemRegionObject>(that);
  if (mo) return mustEqual(mo, pedge, comp, analysis);
  else    return false;
}

bool MemRegionObject::equalSet(MemRegionObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  /*if(equalSetCache.find(that) == equalSetCache.end())
   equalSetCache[that] = comp->equalSetAO(shared_from_this(), that, pedge, analysis);
   return equalSetCache[that];*/
  return comp->equalSetMR(shared_from_this(), that, pedge, analysis);
}

bool MemRegionObject::subSet(MemRegionObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  return comp->subSetMR(shared_from_this(), that, pedge, analysis);
}

bool MemRegionObject::equalSet(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemRegionObject, the two objects only equal only if they're both full.
  // We do this check early because casting to FullMemRegionObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that)) return isFull(pedge, comp, analysis);
  if (dynamic_cast<FullMemRegionObject*>(this))               return that->isFull(pedge, comp, analysis);

  MemRegionObjectPtr co = boost::dynamic_pointer_cast<MemRegionObject>(that);
  if (co) return equalSet(co, pedge, comp, analysis);
  else    return false;
}

bool MemRegionObject::subSet(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemRegionObject, this is a subset of that if they're equal or of that is full.
  // We do this check early because casting to FullMemRegionObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that)) return true;
  if (dynamic_cast<FullMemRegionObject*>(this))               return that->isFull(pedge, comp, analysis);

  MemRegionObjectPtr co = boost::dynamic_pointer_cast<MemRegionObject>(that);
  if (co) return subSet(co, pedge, comp, analysis);
  else    return false;
}

// General version of isLive that accounts for framework details before routing the call to the derived class'
// isLiveML check. Specifically, it routes the call through the composer to make sure the isLiveML call gets the
// right PartEdge
bool MemRegionObject::isLive(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  return comp->isLiveMR(shared_from_this(), pedge, analysis);
}

// General version of meetUpdate() that accounts for framework details before routing the call to the derived class'
// meetUpdateMR check. Specifically, it routes the call through the composer to make sure the meetUpdateMR
// call gets the right PartEdge
bool MemRegionObject::meetUpdate(MemRegionObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  return comp->meetUpdateMR(shared_from_this(), that, pedge, analysis);
}

bool MemRegionObject::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return false;
    case FuncResultUnequal: return setToFull(pedge, comp, analysis);
    case NeitherFuncResult: break;
  }
  // If that is a FullMemRegionObject but this is not, we'll need to make this object full
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that) && !isFull(pedge, comp, analysis))
    return setToFull(pedge, comp, analysis);

  MemRegionObjectPtr mr = boost::dynamic_pointer_cast<MemRegionObject>(that);
  assert(mr);
  return meetUpdate(mr, pedge, comp, analysis);
}

// General version of isFull/isEmpty that accounts for framework details before routing the call to the
// derived class' isFullMR/isEmptyMR check. Specifically, it routes the call through the composer to make
// sure the isFullMR/isEmptyMR call gets the right PartEdge
bool MemRegionObject::isFull(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  return comp->isFullMR(shared_from_this(), pedge, analysis);
}

bool MemRegionObject::isEmpty(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  return comp->isEmptyMR(shared_from_this(), pedge, analysis);
}

// General version of getRegionSize that accounts for framework details before routing the call to the
// derived class' getRegionSizeAO(). Specifically, it routes the call through the composer to make
// sure the getRegionSize call gets the right PartEdge
ValueObjectPtr MemRegionObject::getRegionSize(PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  return comp->getRegionSizeMR(shared_from_this(), pedge, analysis);
}

/* #####################################
   ##### FuncResultMemRegionObject #####
   ##################################### */

// Special MemRegionObject used internally by the framework to associate with the return value of a function
bool FuncResultMemRegionObject::FuncResultMemRegionObject_comparable::equal(
    const comparable& that_arg) const {
  try {
    const FuncResultMemRegionObject_comparable& that =
        dynamic_cast<const FuncResultMemRegionObject_comparable&>(that_arg);
    // Since the argument has FuncResultMemRegionObject type, they're equal
    return true;
  } catch (std::bad_cast bc) {
    // Since the argument does not have FuncResultMemRegionObject type, they're not equal
    return false;
  }
}

bool FuncResultMemRegionObject::FuncResultMemRegionObject_comparable::less(
    const comparable& that_arg) const {
  try {
    const FuncResultMemRegionObject_comparable& that =
        dynamic_cast<const FuncResultMemRegionObject_comparable&>(that_arg);
    // Since the argument has FuncResultMemRe1gionObject type, they're equal, but not less-than
    return false;
  } catch (std::bad_cast bc) {
    // Since the argument does not have FuncResultMemRegionObject type, they're not equal.
    // We'll order FuncResultMemRegionObjects before all others, so return that this < that
    return true;
  }
}

FuncResultMemRegionObject::FuncResultMemRegionObject(Function func) :
    MemRegionObject(NULL), func(func) {
}

// Returns whether this object may/must be equal to o within the given Part p
bool FuncResultMemRegionObject::mayEqualAO(MemRegionObjectPtr o,
    PartEdgePtr pedge) {
// FuncResultMemRegionObjects are disjoint. They're either equal or not subsets.
  return mustEqualAO(o, pedge);
}

bool FuncResultMemRegionObject::mustEqualAO(MemRegionObjectPtr o,
    PartEdgePtr pedge) {
  //The two objects denote the same set iff they're both FuncResultMemRegionObjects that correspond to the same function
  FuncResultMemRegionObjectPtr that = boost::dynamic_pointer_cast<
      FuncResultMemRegionObject>(o);
  return that && func == that->func;
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool FuncResultMemRegionObject::equalSetAO(MemRegionObjectPtr o,
    PartEdgePtr pedge) {
  // FuncResultMemRegionObjects are disjoint. They're either equal or not subsets.
  return mustEqualAO(o, pedge);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool FuncResultMemRegionObject::subSetAO(MemRegionObjectPtr o,
    PartEdgePtr pedge) {
  // FuncResultMemRegionObjects are disjoint. They're either equal or not subsets.
  return mustEqualAO(o, pedge);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool FuncResultMemRegionObject::meetUpdateAO(MemRegionObjectPtr that,
    PartEdgePtr pedge) {
  assert(0);
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool FuncResultMemRegionObject::isFullAO(PartEdgePtr pedge) {
  return false;
}

// Returns whether this AbstractObject denotes the empty set.
bool FuncResultMemRegionObject::isEmptyAO(PartEdgePtr pedge) {
  return false;
}

// Returns a ValueObject that denotes the size of this memory region
ValueObjectPtr FuncResultMemRegionObject::getRegionSizeAO(PartEdgePtr pedge) {
  // The size of a function result is irrelevant since its internals cannot be accessed directly
  // (its possible to access the internals of its SgFunctionCallExp though) so we return an unknown size.
  return boost::make_shared<FullValueObject>();
}

MemRegionObjectPtr FuncResultMemRegionObject::copyAOType() const {
  return boost::make_shared<FuncResultMemRegionObject>(func);
}

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
MemRegionObjectPtr MemRegionObject::project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis) {
  return shared_from_this();
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& FuncResultMemRegionObject::getHierKey() const {
  if (!isHierKeyCached) {
    ((FuncResultMemRegionObject*) this)->cachedHierKey = boost::make_shared<
        AOSHierKey>(((FuncResultMemRegionObject*) this)->shared_from_this());

    ((FuncResultMemRegionObject*) this)->cachedHierKey->add(
        boost::make_shared<FuncResultMemRegionObject_comparable>());
    ((FuncResultMemRegionObject*) this)->isHierKeyCached = true;
  }
  return cachedHierKey;
}

/* ##################################
   ##### FullMemRegionObject   #####
   ################################## */

bool FullMemRegionObject::mayEqualAO(MemRegionObjectPtr o,
    PartEdgePtr pedge) {
  return true;
}

bool FullMemRegionObject::mustEqualAO(MemRegionObjectPtr o,
    PartEdgePtr pedge) {
  return false;
}

bool FullMemRegionObject::equalSetAO(MemRegionObjectPtr o,
    PartEdgePtr pedge) {
  return isFullAO(pedge);
}

bool FullMemRegionObject::subSetAO(MemRegionObjectPtr o, PartEdgePtr pedge) {
  return isFullAO(pedge);
}

MemRegionObjectPtr FullMemRegionObject::copyAOType() const {
  return boost::make_shared<FullMemRegionObject>();
}

bool FullMemRegionObject::isLiveAO(PartEdgePtr pedge) {
  return true;
}

bool FullMemRegionObject::meetUpdateAO(MemRegionObjectPtr that,
    PartEdgePtr pedge) {
  return false;
}

bool FullMemRegionObject::isFullAO(PartEdgePtr pedge) {
  return true;
}

bool FullMemRegionObject::isEmptyAO(PartEdgePtr pedge) {
  return false;
}

// Returns true if this AbstractObject corresponds to a concrete value that is statically-known
bool FullMemRegionObject::isConcrete() {
  return false;
}

// Returns the number of concrete values in this set
int FullMemRegionObject::concreteSetSize() {
  return -1;
}

ValueObjectPtr FullMemRegionObject::getRegionSizeAO(PartEdgePtr pedge) {
  return boost::make_shared<FullValueObject>();
}

string FullMemRegionObject::str(string indent) const {
  return "FullMemRegionObject";
}

/* ###################################
   ##### CombinedMemRegionObject #####
   ################################### */

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
SgType* MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::getConcreteType() {
  if(!MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::isConcrete()) assert(0);
  typename map<Key, MemRegionObjectPtr>::iterator it = MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::aoMap.begin();
  SgType* c_type = it->second->getConcreteType();
  // assert that all other objects have the same type
  for (++it; it != MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::aoMap.end(); ++it) {
    SgType* votype = it->second->getConcreteType();
    assert(c_type == votype);
  }
  return c_type;
}

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
set<SgNode*> MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::getConcrete() {
  if(!MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::isConcrete()) assert(0);
  // If this is a union type (defaultMayEq=true), the result is the Union of the sets returned by getConcrete() on all the memRegions.
  // If this is an intersection type (defaultMayEq=false), an object is their Intersection.

  // Maps each concrete value to the number of elements in memRegions for which it was returned
  std::map<SgNode*, size_t> concreteMRs;
  for (typename map<Key, MemRegionObjectPtr>::iterator mr_it = MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::aoMap.begin();
       mr_it != MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::aoMap.end(); ++mr_it) {
    // Iterate through the current sub-MemRegion's concrete values and increment each
    // concrete value's counter in concreteMRs.
    std::set<SgNode*> c_memregionSet = mr_it->second->getConcrete();
    for (std::set<SgNode*>::iterator s_it = c_memregionSet.begin();
        s_it != c_memregionSet.end(); s_it++) {
      // Find the key in concrete memRegions with an equivalent SgExpression to *s_it
      std::map<SgNode*, size_t>::iterator c_it = concreteMRs.find(*s_it);
      if (c_it != concreteMRs.end())
        ++c_it->second;
      // If the current concrete value *s_it does not appear in concreteMRs, add it
      else
        concreteMRs[*s_it] = 1;
    }
  }

  // Collect the union or intersection of all results from concreteMRs as a set
  std::set<SgNode*> ret;
  for (std::map<SgNode*, size_t>::iterator i = concreteMRs.begin();
      i != concreteMRs.end(); i++) {
    // Union: add every key in concreteMRs to ret
    if (MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::isUnion())
      ret.insert(i->first);
    // Intersection: only add the keys that appear in every MemRegion in memRegions
    else if (MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::isIntersection() &&
             i->second == MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::aoMap.size())
      ret.insert(i->first);
  }

  return ret;
}

//! Size of the memory region denoted by this memory object represented by a ValueObject
//! Useful only if the object is not full
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
ValueObjectPtr MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::getRegionSizeAO(
    PartEdgePtr pedge) {
  // Assert for atleast one element in the map
  // Should we handle full MR by returning FullValueObject?
  if(!MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::aoMap.size() > 0) assert(0);

  // getRegionSize on each object returns different ValueObject for each key
  // We cannot do meetUpdate as the objects are from different analysis
  // Return a MappedValueObject based on those objects and the corresponding key
  boost::shared_ptr<MappedValueObject<Key, KeyIsComposedAnalysis, MappedAOValueType> > mvo_p =
      boost::make_shared<MappedValueObject<Key, KeyIsComposedAnalysis, MappedAOValueType> >(MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::ui,
                                                                     MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::analysis);
  typename map<Key, MemRegionObjectPtr>::const_iterator it =
      MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::aoMap.begin();
  for (; it != MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::aoMap.end(); ++it) {
    ValueObjectPtr vo_p = it->second->getRegionSizeAO(pedge);
    Key k = it->first;
    mvo_p->add(k, vo_p, pedge, /*comp, analysis*/NULL, NULL);
  }

  return mvo_p;
}

// Check whether that is a MemRegionObject and if so, call the version of mayEqual specific to MemRegionObjects
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
bool MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::mayEqual
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemRegionObject, the two objects only overlap if either is full.
  // We do this check early because casting to FullMemRegionObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that)) return true;
  if (dynamic_cast<FullMemRegionObject*>(this))               return true;

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::mayEqual(that, pedge, comp, analysis);
}

// Check whether that is a MemRegionObject and if so, call the version of mustEqual specific to MemRegionObjects
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
bool MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::mustEqual
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemRegionObject, the two objects only must-equal if neither is full.
  // We do this check early because casting to FullMemRegionObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that)) return false;
  if (dynamic_cast<FullMemRegionObject*>(this))               return false;

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::mustEqual(that, pedge, comp, analysis);
}

// Returns whether the two abstract objects denote the same set of concrete objects
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
bool MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::equalSet
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemRegionObject, the two objects only equal only if they're both full.
  // We do this check early because casting to FullMemRegionObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that)) return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::isFull(pedge, comp, analysis);
  if (dynamic_cast<FullMemRegionObject*>(this))               return that->isFull(pedge, comp, analysis);

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::equalSet(that, pedge, comp, analysis);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
bool MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::subSet
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemRegionObject, this is a subset of that if they're equal or of that is full.
  // We do this check early because casting to FullMemRegionObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that)) return true;
  if (dynamic_cast<FullMemRegionObject*>(this))               return that->isFull(pedge, comp, analysis);

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::subSet(that, pedge, comp, analysis);
}

// General version of meetUpdate() that accounts for framework details before routing the call to the derived class'
// meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML
// call gets the right PartEdge
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType>
bool MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType>::meetUpdate
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemRegionObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return false;
    case FuncResultUnequal: return this->setToFull(pedge, comp, analysis);
    case NeitherFuncResult: break;
  }
  // If that is a FullMemRegionObject but this is not, we'll need to make this object full
  if (boost::dynamic_pointer_cast<FullMemRegionObject>(that) &&
      !MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::isFull(pedge, comp, analysis))
    return this->setToFull(pedge, comp, analysis);

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, MappedAOSubType>::meetUpdate(that, pedge, comp, analysis);
}

/* ################################
   # PartEdgeUnionMemRegionObject #
   ################################ */

PartEdgeUnionMemRegionObject::PartEdgeUnionMemRegionObject() :
    MemRegionObject(NULL) {
}

PartEdgeUnionMemRegionObject::PartEdgeUnionMemRegionObject(
    const PartEdgeUnionMemRegionObject& thatMR) :
    MemRegionObject(thatMR), unionMR_p(thatMR.unionMR_p->copyAOType()) {
}

SgNode* PartEdgeUnionMemRegionObject::getBase() const {
  return unionMR_p->getBase();
}

void PartEdgeUnionMemRegionObject::add(MemRegionObjectPtr mr_p,
    PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // If this is the very first object
  if (!unionMR_p.get())
    unionMR_p = mr_p->copyAOType();
  // If Full return without adding
  else if (isFullAO(pedge))
    return;
  // Else meetUpdate with the existing unionMR_p
  else
    unionMR_p->meetUpdateAO(mr_p, pedge);
}

bool PartEdgeUnionMemRegionObject::mayEqualAO(MemRegionObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->mayEqualAO(thatMR_p->getUnionAO(), pedge);
}

bool PartEdgeUnionMemRegionObject::mustEqualAO(MemRegionObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->mustEqualAO(thatMR_p->getUnionAO(), pedge);
}

bool PartEdgeUnionMemRegionObject::equalSetAO(MemRegionObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->equalSetAO(thatMR_p->getUnionAO(), pedge);
}

bool PartEdgeUnionMemRegionObject::subSetAO(MemRegionObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->subSetAO(thatMR_p->getUnionAO(), pedge);
}

bool PartEdgeUnionMemRegionObject::meetUpdateAO(MemRegionObjectPtr that,
    PartEdgePtr pedge) {
  boost::shared_ptr<PartEdgeUnionMemRegionObject> thatMR_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemRegionObject>(that);
  assert(thatMR_p);
  return unionMR_p->meetUpdateAO(thatMR_p->getUnionAO(), pedge);
}

bool PartEdgeUnionMemRegionObject::isLiveAO(PartEdgePtr pedge) {
  return unionMR_p->isLiveAO(pedge);
}

bool PartEdgeUnionMemRegionObject::isFullAO(PartEdgePtr pedge) {
  return unionMR_p->isFullAO(pedge);
}

bool PartEdgeUnionMemRegionObject::isEmptyAO(PartEdgePtr pedge) {
  return unionMR_p->isEmptyAO(pedge);
}

MemRegionObjectPtr PartEdgeUnionMemRegionObject::copyAOType() const {
  return boost::make_shared<PartEdgeUnionMemRegionObject>(*this);
}

void PartEdgeUnionMemRegionObject::setAOToFull() {
  unionMR_p = boost::make_shared<FullMemRegionObject>();
}

// Returns true if this MemRegionObject denotes a finite set of concrete regions
bool PartEdgeUnionMemRegionObject::isConcrete() {
  return unionMR_p->isConcrete();
}
// Returns the number of concrete values in this set
int PartEdgeUnionMemRegionObject::concreteSetSize() {
  return unionMR_p->concreteSetSize();
}
// Returns the type of the concrete regions (if there is one)
SgType* PartEdgeUnionMemRegionObject::getConcreteType() {
  return unionMR_p->getConcreteType();
}
// Returns the set of concrete memory regions as SgExpressions, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<SgNode*> PartEdgeUnionMemRegionObject::getConcrete() {
  return unionMR_p->getConcrete();
}

ValueObjectPtr PartEdgeUnionMemRegionObject::getRegionSizeAO(PartEdgePtr pedge) {
  return unionMR_p->getRegionSizeAO(pedge);
}

// Returns whether all instances of this class form a hierarchy. Every instance of the same
// class created by the same analysis must return the same value from this method!
bool PartEdgeUnionMemRegionObject::isHierarchy() const
{ return unionMR_p->isHierarchy(); }

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy
const AbstractionHierarchy::hierKeyPtr& PartEdgeUnionMemRegionObject::getHierKey() const
{ return unionMR_p->getHierKey(); }

// ----------------------------------------
// Objects that denote disjoint sets. Because no two sets may overlap, they can be
// represented using unique numbers, which enables efficient data structure implementations.
bool PartEdgeUnionMemRegionObject::isDisjoint() const
{ return unionMR_p->isDisjoint(); }

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
MemRegionObjectPtr PartEdgeUnionMemRegionObject::project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis)
{ return unionMR_p->project(analysis, pedge, comp, clientAnalysis); }

string PartEdgeUnionMemRegionObject::str(string indent) const {
  ostringstream oss;
  oss << "[UnionMR=" << unionMR_p.get() << ", " << unionMR_p->str(indent)
      << "]";
  return oss.str();
}

/* ########################
   ##### MemLocObject #####
   ######################## */

MemLocObjectPtr NULLMemLocObject;

MemLocObject::MemLocObject(const MemLocObject& that) :
    AbstractObject(that), /*AbstractionHierarchy(that),*/ region(
        that.region != NULLMemRegionObject ?
            that.region->copyAOType() : NULLMemRegionObject), index(
        that.index != NULLValueObject ? that.index->copyAOType() : NULLValueObject) {
  //cout << "MemLocObject::MemLocObject("<<this<<") isHierKeyCached="<<isHierKeyCached<<", region="<<region.get()<<" that.region="<<that.region.get()<<"="<<(that.region? that.region->str(): "NULL")<<endl;

}

MemRegionObjectPtr MemLocObject::getRegion() const { /*cout << "MemLocObject::getRegion("<<this<<") region="<<(region==NULLMemRegionObject?"NULL":region->str())<<endl; */
  return region;
}

ValueObjectPtr MemLocObject::getIndex() const { /*cout << "MemLocObject::getIndex("<<this<<") index="<<(index==NULLValueObject?"NULL":index->str())<<endl; */
  return index;
}

// Returns the relationship between the given AbstractObjects, considering whether either
// or both are FuncResultMemLocObjects and if they refer to the same function
FuncResultRelationType MemLocObject::getFuncResultRel(AbstractObject* one, AbstractObjectPtr two, PartEdgePtr pedge) {
  // If either this or that are FuncResultMemLocObject, they mayEqual iff they correspond to the same function
  FuncResultMemLocObject* frmlcoOne = dynamic_cast<FuncResultMemLocObject*>(one);
  FuncResultMemLocObjectPtr frmlcoTwo = boost::dynamic_pointer_cast<FuncResultMemLocObject>(two);
  if (frmlcoOne) {
    if(frmlcoOne->mustEqualAO(frmlcoTwo, pedge)) return FuncResultSameFunc;
    else                                         return FuncResultUnequal;
  } else if (frmlcoTwo)
    return FuncResultUnequal;
  else
    return NeitherFuncResult;
}

// General version of mayEqual and mustEqual that accounts for framework details before routing the call to the
// derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
// the call through the composer to make sure the may/mustEqual call gets the right PartEdge
bool MemLocObject::mayEqual(MemLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  /*scope s("MemLocObject::mayEqual");
   dbg << "this="<<str()<<endl;
   dbg << "that="<<that->str()<<endl;*/

  // Returns true only if both this region and this index denot the same set as that region and index, respectively
  // If this and that region overlap
  if (getRegion()->mayEqual(that->getRegion(), pedge, comp, analysis)) {
    if (getIndex() == NULLValueObject) {
      // If both this->index and that->index are NULL, then their sets (full) overlap
      if (that->getIndex() == NULLValueObject)
        return true;
      // If both this->index is NULL (denotes the full set) but that->index is not, they overlap
      else
        return true;
    } else {
      // If both that->index is NULL (denotes the full set) but this->index is not, they overlap
      if (that->getIndex() == NULLValueObject)
        return true;
      // If both are not NULL, forward the query to the implementation of this->index
      else
        return getIndex()->mayEqual(that->getIndex(), pedge, comp, analysis);
    }
  } else
    return false;
}

// General version of mayEqual and mustEqual that accounts for framework details before routing the call to the
// derived class' may/mustEqual check. Specifically, it checks may/must equality with respect to ExprObj and routes
// the call through the composer to make sure the may/mustEqual call gets the right PartEdge
bool MemLocObject::mustEqual(MemLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Returns true only if both this region and this index denot the same set as that region and index, respectively
  // If this region denotes the same set as that region in all executions
  if (getRegion()->mustEqual(that->getRegion(), pedge, comp, analysis)) {
    if (getIndex() == NULLValueObject) {
      // If both this->index and that->index are NULL, we assume they're not mustEqual since
      // NULL corresponds to the Full set of values
      if (that->getIndex() == NULLValueObject)
        return false;
      // If both this->index is NULL (denotes the full set) but that->index is not, they're not mustEqual
      else
        return false;
    } else {
      // If both that->index is NULL (denotes the full set) but this->index is not, they're not mustEqual
      if (that->getIndex() == NULLValueObject)
        return false;
      // If both are not NULL, forward the query to the implementation of this->index
      else
        return getIndex()->mustEqual(that->getIndex(), pedge, comp, analysis);
    }
  } else
    return false;
}

// Check whether that is a MemLocObject and if so, call the version of mayEqual specific to MemLocObjects
bool MemLocObject::mayEqual(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {

  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemLocObject, the two objects only overlap if either is full.
  // We do this check early because casting to FullMemLocObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that)) return true;
  if (dynamic_cast<FullMemLocObject*>(this))               return true;

  MemLocObjectPtr mo = boost::dynamic_pointer_cast<MemLocObject>(that);
  if (mo)
    return mayEqual(mo, pedge, comp, analysis);
  else
    return false;
}

// Check whether that is a MemLocObject and if so, call the version of mustEqual specific to MemLocObjects
bool MemLocObject::mustEqual(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemLocObject, the two objects only must-equal if neither is full.
  // We do this check early because casting to FullMemLocObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that)) return false;
  if (dynamic_cast<FullMemLocObject*>(this))               return false;

  MemLocObjectPtr mo = boost::dynamic_pointer_cast<MemLocObject>(that);
  if (mo)
    return mustEqual(mo, pedge, comp, analysis);
  else
    return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool MemLocObject::equalSet(MemLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // That is known to be a generic MemLocObject

  // Returns true only if both this region and this index denote the same set as that region and index, respectively

  /*cout << "getRegion()="<<(region?region->str():"NULL")<<endl;
   cout << "getRegion="<<(getRegion()?getRegion()->str():"NULL")<<endl;
   cout << "this="<<str()<<endl;

   cout << "that->region="<<(that->region?that->region->str():"NULL")<<endl;
   cout << "that->getRegion="<<(that->getRegion()?that->getRegion()->str():"NULL")<<endl;
   cout << "that="<<that->str()<<endl;*/

  // If this region is denotes the same set as that region
  if (getRegion()->equalSet(that->getRegion(), pedge, comp, analysis)) {
    if (getIndex() == NULLValueObject) {
      // If both this->index and that->index are NULL, then the subset property holds
      if (that->getIndex() == NULLValueObject)
        return true;
      // If both this->index is NULL (denotes the full set) but that->index is not,
      // this->index is a equal to that->index only if that->index is Full
      else
        return that->getIndex()->isFull(pedge, comp, analysis);
    } else {
      // If both that->index is NULL (denotes the full set) but this->index is not,
      // this->index is a equal to that->index only if this->index is Full
      if (that->getIndex() == NULLValueObject)
        return getIndex()->isFull(pedge, comp, analysis);
      // If both are not NULL, forward the query to the implementation of this->index
      else
        return getIndex()->equalSet(that->getIndex(), pedge, comp, analysis);
    }
  } else
    return false;
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool MemLocObject::subSet(MemLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {

  // Returns true only if both this region and this index are a subset of that region and index, respectively

  // If this region is a subset of that region
  if (getRegion()->subSet(that->getRegion(), pedge, comp, analysis)) {
    if (getIndex() == NULLValueObject) {
      // If both this->index and that->index are NULL, then the subset property holds
      if (that->getIndex() == NULLValueObject)
        return true;
      // If both this->index is NULL (denotes the full set) but that->index is not,
      // this->index is a subset of that->index only if that->index is Full
      else
        return that->getIndex()->isFull(pedge, comp, analysis);
    } else {
      // If that->index is NULL (denotes the full set) but this->index is not
      if (that->getIndex() == NULLValueObject)
        return true;
      // If both are not NULL, forward the query to the implementation of this->index
      else
        return getIndex()->subSet(that->getIndex(), pedge, comp, analysis);
    }
  } else
    return false;
}

bool MemLocObject::equalSet(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemLocObject, the two objects only equal only if they're both full.
  // We do this check early because casting to FullMemLocObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that)) return isFull(pedge, comp, analysis);
  if (dynamic_cast<FullMemLocObject*>(this))               return that->isFull(pedge, comp, analysis);

  MemLocObjectPtr co = boost::dynamic_pointer_cast<MemLocObject>(that);
  if (co)
    return equalSet(co, pedge, comp, analysis);
  else
    return false;
}

bool MemLocObject::subSet(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemLocObject, this is a subset of that if they're equal or of that is full.
  // We do this check early because casting to FullMemLocObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that)) return true;
  if (dynamic_cast<FullMemLocObject*>(this))               return that->isFull(pedge, comp, analysis);

  MemLocObjectPtr co = boost::dynamic_pointer_cast<MemLocObject>(that);
  if (co)
    return subSet(co, pedge, comp, analysis);
  else
    return false;
}

// General version of isLive that accounts for framework details before routing the call to the derived class'
// isLiveML check. Specifically, it routes the call through the composer to make sure the isLiveML call gets the
// right PartEdge
bool MemLocObject::isLive(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis)
{
/*  scope s("MemLocObject::isLive");
  dbg << "getRegion()="<<getRegion()->str()<<", getRegion()->isLive(pedge, comp, analysis)="<<getRegion()->isLive(pedge, comp, analysis)<<endl;
  if(getIndex())
    dbg << "getIndex()="<<getIndex()->str()<<", getIndex()->isLive(pedge, comp, analysis)="<<getIndex()->isLive(pedge, comp, analysis)<<endl;
*/
  return getRegion()->isLive(pedge, comp, analysis)
      && (!getIndex() || getIndex()->isLive(pedge, comp, analysis));
}


// General version of meetUpdate() that accounts for framework details before routing the call to the derived class'
// meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML
// call gets the right PartEdge
bool MemLocObject::meetUpdate(MemLocObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  bool modified = false;
  modified = getRegion()->meetUpdate(that->getRegion(), pedge, comp, analysis)
      || modified;
  if (getIndex() != NULLValueObject)
    modified = getIndex()->meetUpdate(that->getIndex(), pedge, comp, analysis)
        || modified;
  return modified;
}

bool MemLocObject::meetUpdate(AbstractObjectPtr that, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return false;
    case FuncResultUnequal: return setToFull(pedge, comp, analysis);
    case NeitherFuncResult: break;
  }
  // If that is a FullMemLocObject but this is not, we'll need to make this object full
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that) && !isFull(pedge, comp, analysis))
    return setToFull(pedge, comp, analysis);

  MemLocObjectPtr ml = boost::dynamic_pointer_cast<MemLocObject>(that);
  assert(ml);
  return meetUpdate(ml, pedge, comp, analysis);
}


// General versions of isFull() and isEmpty that account for framework details before routing the call to the
// derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make
// sure the isFull(PartEdgePtr) and isEmpty(PartEdgePtr) call gets the right PartEdge.
// These functions are just aliases for the real implementations in AbstractObject
bool MemLocObject::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
// This MemLocObject is full if is region is Full and its index is either NULL or says its Full
{
  assert(getRegion());
  return getRegion()->isFull(pedge, comp, analysis)
      && (!getIndex() || getIndex()->isFull(pedge, comp, analysis));
}

bool MemLocObject::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
// This MemLocObject is Empty if either its region is empty or its index is empty,
// with a NULL index considered to denote the full set
{
  assert(getRegion());
  return getRegion()->isEmpty(pedge, comp, analysis)
      || (!getIndex() && getIndex()->isEmpty(pedge, comp, analysis));
}

// Returns true if this AbstractObject corresponds to a concrete value that is statically-known
bool MemLocObject::isConcrete() {
  /*dbg << "MemLocObject::isConcrete()"<<endl;
  dbg << "getRegion()="<<(getRegion()? getRegion()->str(): "NULL")<<endl;
  dbg << "getIndex()="<<(getIndex()? getIndex()->str(): "NULL")<<endl;*/
  return getRegion()->isConcrete()
      && (!getIndex() || getIndex()->isConcrete());
}

// Returns the number of concrete values in this set
int MemLocObject::concreteSetSize() {
  return getRegion()->concreteSetSize()
      && (getIndex() ? getIndex()->concreteSetSize() : 1);
}

// Allocates a copy of this object and returns a shared pointer to it
MemLocObjectPtr MemLocObject::copyAOType() const {
  return boost::make_shared<MemLocObject>(*this);
}

// Allocates a copy of this object and returns a regular pointer to it
MemLocObject* MemLocObject::copyMLPtr() const {
  return new MemLocObject(*this);
}

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
MemLocObjectPtr MemLocObject::project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis) {
  return shared_from_this();
}

std::string MemLocObject::str(std::string indent) const { // pretty print for the object
  ostringstream oss;
  oss << "[MemLocObject region=" << getRegion()->str(indent + "    ");
  if (getIndex()) {
    oss << ", " << endl << indent << "             index=" << getIndex()->str(indent + "    ");
  }
  oss << "]";
  return oss.str();
}

// Returns whether all instances of this class form a hierarchy. Every instance of the same
// class created by the same analysis must return the same value from this method!
bool MemLocObject::isHierarchy() const {
  //dbg << "MemLocObject::isHierarchy() getRegion()->isHierarchy()="<<getRegion()->isHierarchy()<<", getIndex()->isHierarchy()="<<(getIndex()? getIndex()->isHierarchy(): -1)<<endl;  // MemLocs form a hierarchy only if their regions and indexes do
  return getRegion()->isHierarchy() && (!getIndex() || getIndex()->isHierarchy());
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& MemLocObject::getHierKey() const {
  // The MemLoc denoted by region and index is essentially the intersection of the
  // constraints imposed by each one. Thus, the key of a MemLoc combines
  // their individual keys.
  //dbg << "MemLocObject::getHierKey() isHierKeyCached="<<isHierKeyCached<<endl;
  if (!isHierKeyCached) {
    AbstractionHierarchyPtr hierRegion = boost::dynamic_pointer_cast<AbstractionHierarchy>(getRegion());
    ROSE_ASSERT(hierRegion);

    ((MemLocObject*) this)->cachedHierKey = boost::make_shared<AOSHierKey>(((MemLocObject*) this)->shared_from_this(), hierRegion->getHierKey());

    if (getIndex()) {
      AbstractionHierarchyPtr hierIndex = boost::dynamic_pointer_cast<AbstractionHierarchy>(getIndex());
      ROSE_ASSERT(hierIndex);

      //scope s(txt()<< "index="<<getIndex()->str());
      ((MemLocObject*) this)->cachedHierKey->add(hierIndex->getHierKey()->begin(), hierIndex->getHierKey()->end());
    }

    ((MemLocObject*) this)->isHierKeyCached = true;
  }
  return cachedHierKey;
}

/* ##################################
   ##### FuncResultMemLocObject #####
   ################################## */

// Special MemLocObject used internally by the framework to associate with the return value of a function
// This is just a MemLoc dedicated to wrapping FuncResultMemLocRegion
FuncResultMemLocObject::FuncResultMemLocObject(Function func) :
    MemLocObject(boost::make_shared<FuncResultMemRegionObject>(func),
        NULLValueObject, NULL) {
}

FuncResultMemLocObject::FuncResultMemLocObject(
    const FuncResultMemLocObject& that) :
    MemLocObject(that) {
}

MemLocObjectPtr FuncResultMemLocObject::copyAOType() const {
  return boost::make_shared<FuncResultMemLocObject>(*this);
}

bool FuncResultMemLocObject::mustEqualAO(MemLocObjectPtr that_arg, PartEdgePtr pedge) {
  //The two objects denote the same set iff they're both FuncResultMemLocObjects with
  // FuncResultMemRegioObject that correspond to the same function
  FuncResultMemLocObjectPtr that = boost::dynamic_pointer_cast<FuncResultMemLocObject>(that_arg);
  assert(getRegion());
  assert(that->getRegion());
  return getRegion()->mustEqualAO(that->getRegion(), pedge);
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& FuncResultMemLocObject::getHierKey() const {
  AbstractionHierarchyPtr hierRegion = boost::dynamic_pointer_cast<
      AbstractionHierarchy>(getRegion());
  ROSE_ASSERT(hierRegion);
  return hierRegion->getHierKey();
}

/* ##########################
   #### FullMemLocObject ####
   ########################## */

bool FullMemLocObject::mayEqualAO(MemLocObjectPtr o, PartEdgePtr pedge) {
  return true;
}

bool FullMemLocObject::mustEqualAO(MemLocObjectPtr o, PartEdgePtr pedge) {
  return false;
}

bool FullMemLocObject::equalSetAO(MemLocObjectPtr o, PartEdgePtr pedge) {
  return isFullAO(pedge);
}

bool FullMemLocObject::subSetAO(MemLocObjectPtr o, PartEdgePtr pedge) {
  return isFullAO(pedge);
}

MemLocObjectPtr FullMemLocObject::copyAOType() const {
  return boost::make_shared<FullMemLocObject>();
}

bool FullMemLocObject::isLiveAO(PartEdgePtr pedge) {
  return true;
}

bool FullMemLocObject::meetUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge) {
  return false;
}

bool FullMemLocObject::isFullAO(PartEdgePtr pedge) {
  return true;
}

bool FullMemLocObject::isEmptyAO(PartEdgePtr pedge) {
  return false;
}

// Returns true if this AbstractObject corresponds to a concrete value that is statically-known
bool FullMemLocObject::isConcrete() {
  return false;
}

// Returns the number of concrete values in this set
int FullMemLocObject::concreteSetSize() {
  return -1;
}

string FullMemLocObject::str(string indent) const {
  return "FullMemLocObject";
}

/* ################################
   ##### CombinedMemLocObject #####
   ################################ */



/* ##############################
   ##### MappedMemLocObject #####
   ############################## */

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType, class MappedAOMemRegionType>
MemRegionObjectPtr MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::getRegion() const {
  if (MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::region == NULLMemRegionObject) {
    // Collect all the memRegions of the memLocs in this object and create a CombinedMemRegionObject out of them
    map<Key, MemRegionObjectPtr> memRegions;
    for (typename map<Key, MemLocObjectPtr>::const_iterator it = MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::aoMap.begin();
         it != MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::aoMap.end(); ++it) {
      memRegions[it->first] = it->second->getRegion();
    }

    ((MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>*) this)->region =
        boost::make_shared<MappedMemRegionObject<Key, KeyIsComposedAnalysis, MappedAOMemRegionType, MappedAOValueType> >(
            MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::ui,
            MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::analysis,
            memRegions);
  }
  return MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::region;
}

template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType, class MappedAOMemRegionType>
ValueObjectPtr MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::getIndex() const {
  if (MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::index == NULLValueObject) {
    // Collect all the indexes of the memlocs in aoMap and create a CombinedValueObject out of them
    map<Key, ValueObjectPtr> indexes;
    for (typename map<Key, MemLocObjectPtr>::const_iterator it = MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::aoMap.begin();
         it != MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::aoMap.end(); ++it) {
      indexes[it->first] = it->second->getIndex();
    }

    ((MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>*) this)->index =
        boost::make_shared<MappedValueObject<Key, KeyIsComposedAnalysis, MappedAOValueType> >(
            MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::ui,
            MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::analysis,
            indexes);
  }
  return MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::index;
}

// Check whether that is a MemLocObject and if so, call the version of mayEqual specific to MemLocObjects
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType, class MappedAOMemRegionType>
bool MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::mayEqual
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemLocObject, the two objects only overlap if either is full.
  // We do this check early because casting to FullMemLocObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that)) return true;
  if (dynamic_cast<FullMemLocObject*>(this))               return true;

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>::mayEqual(that, pedge, comp, analysis);
}

// Check whether that is a MemLocObject and if so, call the version of mustEqual specific to MemLocObjects
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType, class MappedAOMemRegionType>
bool MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::mustEqual
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemLocObject, the two objects only must-equal if neither is full.
  // We do this check early because casting to FullMemLocObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that)) return false;
  if (dynamic_cast<FullMemLocObject*>(this))               return false;

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>::mustEqual(that, pedge, comp, analysis);
}

// Returns whether the two abstract objects denote the same set of concrete objects
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType, class MappedAOMemRegionType>
bool MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::equalSet
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemLocObject, the two objects only equal only if they're both full.
  // We do this check early because casting to FullMemLocObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that)) return this->isFull(pedge, comp, analysis);
  if (dynamic_cast<FullMemLocObject*>(this))               return that->isFull(pedge, comp, analysis);

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>::equalSet(that, pedge, comp, analysis);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType, class MappedAOMemRegionType>
bool MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::subSet
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return true;
    case FuncResultUnequal: return false;
    case NeitherFuncResult: break;
  }
  // If either this or that is a FullMemLocObject, this is a subset of that if they're equal or of that is full.
  // We do this check early because casting to FullMemLocObject is more efficient than calling isFull.
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that)) return true;
  if (dynamic_cast<FullMemLocObject*>(this))               return that->isFull(pedge, comp, analysis);

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>::subSet(that, pedge, comp, analysis);
}

// General version of meetUpdate() that accounts for framework details before routing the call to the derived class'
// meetUpdateML check. Specifically, it routes the call through the composer to make sure the meetUpdateML
// call gets the right PartEdge
template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType, class MappedAOValueType, class MappedAOMemRegionType>
bool MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType, MappedAOValueType, MappedAOMemRegionType>::meetUpdate
                  (AbstractObjectPtr that, PartEdgePtr pedge,Composer* comp, ComposedAnalysis* analysis) {
  // Identical FuncResultMemLocObject denote the same set and different ones denote disjoint sets
  FuncResultRelationType rel = this->getFuncResultRel(this, that, pedge);
  switch(rel) {
    case FuncResultSameFunc: return false;
    case FuncResultUnequal: return this->setToFull(pedge, comp, analysis);
    case NeitherFuncResult: break;
  }
  // If that is a FullMemLocObject but this is not, we'll need to make this object full
  if (boost::dynamic_pointer_cast<FullMemLocObject>(that) && !this->isFull(pedge, comp, analysis))
    return this->setToFull(pedge, comp, analysis);

  return MappedAbstractObject<Key, KeyIsComposedAnalysis, MemLocObject, MemLocObjectPtr, AbstractObject::MemLoc, MappedAOSubType>::meetUpdate(that, pedge, comp, analysis);
}


//
////! Method to add mls to the map.
////! MLs that are full are never added to the map.
////! If ml_p is FullML or ml_p->isFullML=true then mapped ML is set to full only if mostAccurate=false.
//  template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType>
//  void MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType>::add(Key key, MemLocObjectPtr ml_p,
//      PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
//    // If the object is already full don't add anything
//    //if(isUnion() && isFullAO(pedge)) return;
//    if (isUnion() && isFull(pedge, comp, analysis))
//      return;
//
//    // If the ml_p is not full add/update the map
//    //if(!ml_p->isFullAO(pedge)) {
//    if (!ml_p->isFull(pedge, comp, analysis)) {
//      aoMap[key] = ml_p;
//    } else {
//      nFull++;
//      if (isUnion())
//        setMLToFull();
//    }
//  }
//
//  template<class Key, bool KeyIsComposedAnalysis, class MappedAOSubType>
//  bool MappedMemLocObject<Key, KeyIsComposedAnalysis, MappedAOSubType>::mayEqualMLWithKey(Key key,
//      const map<Key, MemLocObjectPtr>& thatMLMap, PartEdgePtr pedge,
//      Composer* comp, ComposedAnalysis* analysis) {
//    typename map<Key, MemLocObjectPtr>::const_iterator s_it;
//    s_it = thatMLMap.find(key);
//    if (s_it == thatMLMap.end())
//      return true;
//    //return aoMap[key]->mayEqualAO(s_it->second, pedge);
//    return aoMap[key]->mayEqual(s_it->second, pedge, comp, analysis);
//  }
//
////! Two ML objects are may equals if there is atleast one execution or sub-exectuion
////! in which they represent the same memory location.
////! Analyses are conservative as they start with full set of executions.
////! Dataflow facts (predicates) shrink the set of sub-executions.
////! We do not explicity store set of sub-executions and they are described
////! by the abstract objects computed from dataflow fact exported by the analysis.
////! Unless the analyses discover otherwise, the conservative answer for mayEqualML is true.
////! Mapped MLs are keyed using either ComposedAnalysis* or PartEdgePtr.
////! Each keyed ML object correspond to some dataflow facts computed by Key=Analysis* or
////! computed at Key=PartEdgePtrthat describes some sets of executions.
////! MayEquality check on mapped ML is performed on intersection of sub-executions
////! or union of sub-executions over the keyed ML objects.

/* #############################
   # PartEdgeUnionMemLocObject #
   ############################# */

PartEdgeUnionMemLocObject::PartEdgeUnionMemLocObject() :
    MemLocObject(NULL) {
}

PartEdgeUnionMemLocObject::PartEdgeUnionMemLocObject(
    const PartEdgeUnionMemLocObject& thatML) :
    MemLocObject(thatML), unionML_p(thatML.unionML_p->copyAOType()) {
}

SgNode* PartEdgeUnionMemLocObject::getBase() const {
  return unionML_p->getBase();
}

MemRegionObjectPtr PartEdgeUnionMemLocObject::getRegion() const {
  return unionML_p->getRegion();
}

ValueObjectPtr     PartEdgeUnionMemLocObject::getIndex() const {
  return unionML_p->getIndex();
}

void PartEdgeUnionMemLocObject::add(MemLocObjectPtr ml_p, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  /*scope s("PartEdgeUnionMemLocObject::add");
  dbg << "Init: unionML_p="<<(unionML_p? unionML_p->str(): "NULL")<<endl;
  dbg << "ml_p="<<ml_p->str()<<endl;*/

  // If this is the very first object
  if (!unionML_p)
    unionML_p = ml_p->copyAOType();
  // If Full return without adding
  //else if(isFullAO(pedge)) return;
  else if (isFull(pedge, comp, analysis))
    return;
  // Else meetUpdate with the existing unionML_p
  //else unionML_p->meetUpdateAO(ml_p, pedge);
  else
    unionML_p->meetUpdate(ml_p, pedge, comp, analysis);


  //dbg << "Final: unionML_p="<<(unionML_p? unionML_p->str(): "NULL")<<endl;
}

//bool PartEdgeUnionMemLocObject::mayEqualAO(MemLocObjectPtr that, PartEdgePtr pedge) {
bool PartEdgeUnionMemLocObject::mayEqual(MemLocObjectPtr that,
    PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  //return unionML_p->mayEqualAO(thatML_p->getUnionAO(), pedge);
  return unionML_p->mayEqual(thatML_p->getUnionAO(), pedge, comp, analysis);
}

//bool PartEdgeUnionMemLocObject::mustEqualAO(MemLocObjectPtr that, PartEdgePtr pedge) {
bool PartEdgeUnionMemLocObject::mustEqual(MemLocObjectPtr that,
    PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  //return unionML_p->mustEqualAO(thatML_p->getUnionAO(), pedge);
  return unionML_p->mustEqual(thatML_p->getUnionAO(), pedge, comp, analysis);
}

//bool PartEdgeUnionMemLocObject::equalSetAO(MemLocObjectPtr that, PartEdgePtr pedge) {
bool PartEdgeUnionMemLocObject::equalSet(MemLocObjectPtr that,
    PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  //return unionML_p->equalSetAO(thatML_p->getUnionAO(), pedge);
  return unionML_p->equalSet(thatML_p->getUnionAO(), pedge, comp, analysis);
}

//bool PartEdgeUnionMemLocObject::subSetAO(MemLocObjectPtr that, PartEdgePtr pedge) {
bool PartEdgeUnionMemLocObject::subSet(MemLocObjectPtr that,
    PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  //return unionML_p->subSetAO(thatML_p->getUnionAO(), pedge);
  return unionML_p->subSet(thatML_p->getUnionAO(), pedge, comp, analysis);
}

//bool PartEdgeUnionMemLocObject::meetUpdateAO(MemLocObjectPtr that, PartEdgePtr pedge) {
bool PartEdgeUnionMemLocObject::meetUpdate(MemLocObjectPtr that,
    PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  boost::shared_ptr<PartEdgeUnionMemLocObject> thatML_p =
      boost::dynamic_pointer_cast<PartEdgeUnionMemLocObject>(that);
  assert(thatML_p);

  assert(unionML_p);
  //return unionML_p->meetUpdateAO(thatML_p->getUnionAO(), pedge);
  return unionML_p->meetUpdate(thatML_p->getUnionAO(), pedge, comp, analysis);
}

//bool PartEdgeUnionMemLocObject::isLiveAO(PartEdgePtr pedge) {
bool PartEdgeUnionMemLocObject::isLive(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  assert(unionML_p);
  //return unionML_p->isLiveAO(pedge);
  return unionML_p->isLive(pedge, comp, analysis);
}

//bool PartEdgeUnionMemLocObject::isFullAO(PartEdgePtr pedge) {
bool PartEdgeUnionMemLocObject::isFull(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  assert(unionML_p);

  /*scope s("PartEdgeUnionMemLocObject::isFull");
  dbg << "unionML_p="<<unionML_p->str()<<endl;*/

  return unionML_p->isFull(pedge, comp, analysis);
}

//bool PartEdgeUnionMemLocObject::isEmptyAO(PartEdgePtr pedge) {
bool PartEdgeUnionMemLocObject::isEmpty(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  assert(unionML_p);
  //return unionML_p->isEmptyAO(pedge);
  return unionML_p->isEmpty(pedge, comp, analysis);
}

// Returns true if this AbstractObject corresponds to a concrete value that is statically-known
bool PartEdgeUnionMemLocObject::isConcrete() {
  return unionML_p->isConcrete();
}

// Returns the number of concrete values in this set
int PartEdgeUnionMemLocObject::concreteSetSize() {
  return unionML_p->concreteSetSize();
}

MemLocObjectPtr PartEdgeUnionMemLocObject::copyAOType() const {
  return boost::make_shared<PartEdgeUnionMemLocObject>(*this);
}

void PartEdgeUnionMemLocObject::setAOToFull() {
  unionML_p = boost::make_shared<FullMemLocObject>();
}

// Gets the portion of this object computed by a given analysis. For individual AbstractObjects this method
// returns the object itself. For compound objects it searches through the sub-objects inside of it for the
// individual objects that came from a given analysis and returns their combination. For example, a Union object
// will implement this method by invoking it on its members, calling meetUpdate on these objects and returning
// the resulting object.
MemLocObjectPtr PartEdgeUnionMemLocObject::project(ComposedAnalysis* analysis, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* clientAnalysis)
{ return unionML_p->project(analysis, pedge, comp, clientAnalysis); }

string PartEdgeUnionMemLocObject::str(string indent) const {
  ostringstream oss;
  oss << "[PEUnionML=" << (unionML_p? unionML_p->str(indent): "NULL") << "]";
  return oss.str();
}

// Returns whether all instances of this class form a hierarchy. Every instance of the same
// class created by the same analysis must return the same value from this method!
bool PartEdgeUnionMemLocObject::isHierarchy() const {
  return unionML_p->isHierarchy();
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& PartEdgeUnionMemLocObject::getHierKey() const {
  return unionML_p->getHierKey();
}

// ----------------------------------------
// Objects that denote disjoint sets. Because no two sets may overlap, they can be
// represented using unique numbers, which enables efficient data structure implementations.
bool PartEdgeUnionMemLocObject::isDisjoint() const
{ return unionML_p->isDisjoint(); }

/* #######################
   ##### IndexVector #####
   ####################### */

//std::string IndexVector::str(const string& indent)
// pretty print for the object
std::string IndexVector::str(std::string indent) const {
#ifndef DISABLE_SIGHT
  dbg
      << "Error. Direct call to base class (IndexVector)'s str() is not allowed."
      << endl;
#endif
  //assert (false);
  return "";
}
bool IndexVector::mayEqual(IndexVectorPtr other, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  cerr
      << "Error. Direct call to base class (IndexVector)'s mayEqual() is not allowed."
      << endl;
  assert(false);
  return false;
}
bool IndexVector::mustEqual(IndexVectorPtr other, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  cerr
      << "Error. Direct call to base class (IndexVector)'s mustEqual() is not allowed."
      << endl;
  assert(false);
  return false;
}

// Returns whether the two abstract index vectors denote the same set of concrete vectors.
bool IndexVector::equalSet(IndexVectorPtr other, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  cerr
      << "Error. Direct call to base class (IndexVector)'s equalSet() is not allowed."
      << endl;
  assert(false);
  return false;
}

// Returns whether this abstract index vector denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract index vector.
bool IndexVector::subSet(IndexVectorPtr other, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  cerr
      << "Error. Direct call to base class (IndexVector)'s subSet() is not allowed."
      << endl;
  assert(false);
  return false;
}

bool IndexVector::meetUpdate(IndexVectorPtr other, PartEdgePtr pedge,
    Composer* comp, ComposedAnalysis* analysis) {
  cerr
      << "Error. Direct call to base class (IndexVector)'s meetUpdate() is not allowed."
      << endl;
  assert(false);
  return false;
}

bool IndexVector::isFull(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  cerr
      << "Error. Direct call to base class (IndexVector)'s isFull() is not allowed."
      << endl;
  assert(false);
  return false;
}

bool IndexVector::isEmpty(PartEdgePtr pedge, Composer* comp,
    ComposedAnalysis* analysis) {
  cerr
      << "Error. Direct call to base class (IndexVector)'s isEmpty() is not allowed."
      << endl;
  assert(false);
  return false;
}

template class GenericMappedCodeLocObject  <ComposedAnalysis*, true>;
template class GenericMappedValueObject    <ComposedAnalysis*, true>;
template class GenericMappedMemRegionObject<ComposedAnalysis*, true>;
template class GenericMappedMemLocObject   <ComposedAnalysis*, true>;
template class MappedCodeLocObject  <ComposedAnalysis*, true, GenericMappedCodeLocObject<ComposedAnalysis*, true> >;
template class MappedValueObject    <ComposedAnalysis*, true, GenericMappedValueObject<ComposedAnalysis*, true> > ;
template class MappedMemRegionObject<ComposedAnalysis*, true, GenericMappedMemRegionObject<ComposedAnalysis*, true>, GenericMappedValueObject<ComposedAnalysis*, true> >;
template class MappedMemLocObject   <ComposedAnalysis*, true, GenericMappedMemLocObject<ComposedAnalysis*, true>, GenericMappedValueObject<ComposedAnalysis*, true>, GenericMappedMemRegionObject<ComposedAnalysis*, true> >;
template class MappedAbstractObject<ComposedAnalysis*, true, CodeLocObject,   CodeLocObjectPtr,   AbstractObject::CodeLoc,   GenericMappedCodeLocObject<ComposedAnalysis*, true> >;
template class MappedAbstractObject<ComposedAnalysis*, true, ValueObject,     ValueObjectPtr,     AbstractObject::Value,     GenericMappedValueObject<ComposedAnalysis*, true> >;
template class MappedAbstractObject<ComposedAnalysis*, true, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, GenericMappedMemRegionObject<ComposedAnalysis*, true> >;
template class MappedAbstractObject<ComposedAnalysis*, true, MemLocObject,    MemLocObjectPtr,    AbstractObject::MemLoc,    GenericMappedMemLocObject<ComposedAnalysis*, true> >;

template class MappedCodeLocObject  <UInt, false, CombinedCodeLocObject>;
template class MappedValueObject    <UInt, false, CombinedValueObject>;
template class MappedMemRegionObject<UInt, false, CombinedMemRegionObject, CombinedValueObject>;
template class MappedMemLocObject   <UInt, false, CombinedMemLocObject, CombinedValueObject, CombinedMemRegionObject>;
template class MappedAbstractObject<UInt, false, CodeLocObject,   CodeLocObjectPtr,   AbstractObject::CodeLoc,   CombinedCodeLocObject>;
template class MappedAbstractObject<UInt, false, ValueObject,     ValueObjectPtr,     AbstractObject::Value,     CombinedValueObject>;
template class MappedAbstractObject<UInt, false, MemRegionObject, MemRegionObjectPtr, AbstractObject::MemRegion, CombinedMemRegionObject>;
template class MappedAbstractObject<UInt, false, MemLocObject,    MemLocObjectPtr,    AbstractObject::MemLoc,    CombinedMemLocObject>;

} //namespace fuse
