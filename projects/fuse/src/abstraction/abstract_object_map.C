#include "sage3basic.h"

using namespace std;

#include <iostream>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include "abstract_object_map.h"
#include "nodeState.h"
#include "analysis.h"

#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

using namespace std;
using namespace boost;

#ifndef DISABLE_SIGHT
using namespace sight;
#endif

namespace fuse {
#define AOMDebugLevel 0


AbstractObjectMapKindPtr NULLAbstractObjectMapKind;

/*****************************
 ***** AbstractObjectMap *****
 *****************************/
AbstractObjectMap::AbstractObjectMap(const AbstractObjectMap& that) :
                                   Lattice(that.latPEdge)/*,
                                   defaultLat  (that.defaultLat),
                                   mapState    (that.mapState),
                                   comp        (that.comp),
                                   analysis    (that.analysis)*/
{
  defaultLat = that.defaultLat;
  mapState   = that.mapState;
  comp       = that.comp;
  analysis   = that.analysis;

  if(mapState==between) {
    ROSE_ASSERT(that.implementation);
    implementation = that.implementation->copy();
    implementation->setParent(this);
  }
}

// Create a copy of that object but using the given implementation. The implementation object
// is incorporated directly without making any additional copies.
AbstractObjectMap::AbstractObjectMap(const AbstractObjectMap& that, AbstractObjectMapKindPtr implementation) :
                                   Lattice(that.latPEdge),
                                   implementation(implementation),
                                   defaultLat  (that.defaultLat),
                                   mapState    (that.mapState),
                                   comp        (that.comp),
                                   analysis    (that.analysis)
{
  implementation->setParent(this);
}
AbstractObjectMap::AbstractObjectMap(AbstractionPtr defaultLat_, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) :
  Lattice(pedge), defaultLat(defaultLat_), mapState(empty), comp(comp), analysis(analysis) { }

AbstractObjectMap::~AbstractObjectMap() {
  //cout << "AbstractObjectMap::~AbstractObjectMap()"<<endl;
}

// Initializes the AOM implementation that will be used inside this AbstractObjectMap.
// The choice of implementation will be based on the semantics that are implemented
// by the key provided as the argument, which are assumed to be implemented by all
// other keys that may ever be provided.
void AbstractObjectMap::initImplementation(AbstractObjectPtr key) {
  if(implementation==NULL) {
    implementation = createAOMKind(key, this);
  }
}

// Returns a newly-allocated AOMKind object that can map the given key type to values
AbstractObjectMapKindPtr AbstractObjectMap::createAOMKind(AbstractObjectPtr key, AbstractObjectMap* parent) {
    SIGHT_VERB_DECL(scope, ("AbstractObjectMap::createAOMKind()"), 1, AOMDebugLevel)
    SIGHT_VERB(dbg << "key="<<key->str()<<endl, 1, AOMDebugLevel)
    SIGHT_VERB(dbg << "key->isMappedAO()="<<key->isMappedAO()<<", key->isHierarchy()="<<key->isHierarchy()<<endl, 1, AOMDebugLevel)
                             /*", getenv(\"DISABLE_HIER_AO\")="<<(getenv("DISABLE_HIER_AO")?getenv("DISABLE_HIER_AO"):"NULL")<<endl;*/
    /*if(key->isDisjoint()) implementation = ???;
     else */
    if(key->isMappedAO()) {
      return boost::make_shared<MappedAOMKind>(key, parent);
    } else {
      if(key->isHierarchy() && getenv("DISABLE_HIER_AO")==NULL)
        return boost::make_shared<HierarchicalAOM>(parent);
      else
        return boost::make_shared<GenericAOM>(parent);
    }
  }

// Add a new memory object --> lattice pair to the frontier.
// Return true if this causes the map to change and false otherwise.
// It is assumed that the given Lattice is now owned by the AbstractObjectMap and can be modified and deleted by it.
bool AbstractObjectMap::insert(AbstractObjectPtr key, AbstractionPtr val) {
  initImplementation(key);
  // If the map was empty, the addition of a new mapping will make it non-empty
  // If it was full, the new mapping will constrain it so that it no longer denotes all possible mappings
  mapState = between;
  return implementation->insert(key, val);
}

// Removes the key matching the argument from the frontier.
// Return true if this causes the map to change and false otherwise.
bool AbstractObjectMap::remove(AbstractObjectPtr key) {
  // If this map corresponds to all possible mappings, all removals are redundant
  if(mapState==full) { return false; }
  ROSE_ASSERT(mapState==between);
  initImplementation(key);
  return implementation->remove(key);
}

// Get all x-frontier for a given abstract memory object
AbstractionPtr AbstractObjectMap::get(AbstractObjectPtr key) {
  // If this map corresponds to all possible mappings, the only mapping that exists for any object is the full lattice
  if(mapState==full) {
    //AbstractionPtr emptyLat(defaultLat->copy());
    AbstractionPtr fullLat = defaultLat->copyA();
    fullLat->setToFull(latPEdge, comp, analysis);
    return fullLat;
  }
  // If this map corresponds to the empty set of mappings, the only mapping that exists for any object is the empty lattice
  else if(mapState==empty) {
    AbstractionPtr emptyLat = defaultLat->copyA();
    emptyLat->setToEmpty(latPEdge, comp, analysis);
    return emptyLat;
  } else {
    //struct timeval aggrStart, aggrEnd; gettimeofday(&aggrStart, NULL);

    ROSE_ASSERT(mapState==between);
    initImplementation(key);
    AbstractionPtr ret = implementation->get(key);
    //gettimeofday(&aggrEnd, NULL); cout << "AbstractObjectMap::get()\t"<<(((aggrEnd.tv_sec*1000000 + aggrEnd.tv_usec) - (aggrStart.tv_sec*1000000 + aggrStart.tv_usec)) / 1000000.0)<<endl;

    return ret;
  }
}

// Set this Lattice object to represent the set of all possible execution prefixes.
// Return true if this causes the object to change and false otherwise.
bool AbstractObjectMap::setToFull() {
  bool modified = (mapState != full);
  mapState=full;
  // Remove the implementation since we don't need to maintain the prior mappings
  implementation = NULLAbstractObjectMapKind;
  return modified;
}

// Set this Lattice object to represent the of no execution prefixes (empty set).
// Return true if this causes the object to change and false otherwise.
bool AbstractObjectMap::setToEmpty() {
  bool modified = (mapState != empty);
  mapState=empty;
  // Remove the implementation since we don't need to maintain the prior mappings
  implementation = NULLAbstractObjectMapKind;
  return modified;
}

// Set all the information associated Lattice object with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
bool AbstractObjectMap::setMLValueToFull(MemLocObjectPtr ml) {
  // If the map is full, there is nothing to do. IF it is empty, there is no key to set to full.
  if(mapState == full || mapState == empty) return false;
  else {
    ROSE_ASSERT(implementation);
    return implementation->setMLValueToFull(ml);
  }
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool AbstractObjectMap::isFull() {
  if(mapState == full) return true;
  else if(mapState == empty) return false;
  else {
    ROSE_ASSERT(implementation);
    return implementation->isFull();
  }
}

// Returns whether this lattice denotes the empty set.
bool AbstractObjectMap::isEmpty() {
  if(mapState == full) return false;
  else if(mapState == empty) return true;
  else {
    ROSE_ASSERT(implementation);
    return implementation->isEmpty();
  }
}

std::string AbstractObjectMap::str(std::string indent) const {
  if(mapState == full)  return txt()<<"[AbstractObjectMap: FULL, pedge = "<<(latPEdge? latPEdge->str(): "NULL")<<"]";
  else if(mapState == empty) return txt()<<"[AbstractObjectMap: EMPTY, pedge = "<<(latPEdge? latPEdge->str(): "NULL")<<"]";
  else {
    ROSE_ASSERT(implementation);
    /*cout << "AOM(this="<<this<<", latPEdge="<<latPEdge->str()<<")"<<endl;
    ostringstream s;
    s << "AOM(this="<<this<<", latPEdge="<<latPEdge->str()<<")"<<endl<<implementation->str(indent);
    return s.str();*/
    return implementation->str(indent);
  }
}

// Variant of the str method that can produce information specific to the current Part.
// Useful since AbstractObjects can change from one Part to another.
std::string AbstractObjectMap::strp(PartEdgePtr pedge, std::string indent) const {
  if(mapState == full)  return "[AbstractObjectMap: FULL]";
  else if(mapState == empty) return "[AbstractObjectMap: EMPTY]";
  else {
    ROSE_ASSERT(implementation);
    return implementation->str(indent);
  }
}

// -----------------
// Lattice methods
// initializes this Lattice to its default state, if it is not already initialized
void AbstractObjectMap::initialize() {
  if(mapState == between) {
    ROSE_ASSERT(implementation);
    return implementation->initialize();
  }
}

// returns a copy of this lattice
Lattice* AbstractObjectMap::copy() const {
  return new AbstractObjectMap(*this);
}

// overwrites the state of this Lattice with that of that Lattice
void AbstractObjectMap::copy(Lattice* thatL) {
  AbstractObjectMap* that = dynamic_cast<AbstractObjectMap*>(thatL);
  ROSE_ASSERT(that);

  if(that->mapState==full) setToFull();
  else if(that->mapState==empty) setToEmpty();
  else {
    ROSE_ASSERT(that->implementation);
    mapState = between;
    if(implementation) implementation->copy(that->implementation);
    else {
      implementation = that->implementation->copy();
      implementation->setParent(this);
    }
  }
}

// Called by analyses to transfer this lattice's contents from across function scopes from a caller function
//    to a callee's scope and vice versa. If this this lattice maintains any information on the basis of
//    individual MemLocObjects these mappings must be converted, with MemLocObjects that are keys of the ml2ml
//    replaced with their corresponding values. If a given key of ml2ml does not appear in the lattice, it must
//    be added to the lattice and assigned a default initial value. In many cases (e.g. over-approximate sets
//    of MemLocObjects) this may not require any actual insertions. If the value of a given ml2ml mapping is
//    NULL (empty boost::shared_ptr), any information for MemLocObjects that must-equal to the key should be
//    deleted.
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    the keys in ml2ml are in scope on one side of Part, while the values on the other side. Specifically, it is
//    guaranteed that the keys are in scope at fromPEdge while the values are in scope at the edge returned
//    by getPartEdge().
// remapML must return a freshly-allocated object.
Lattice* AbstractObjectMap::remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge) {
  if(mapState==between) {
    ROSE_ASSERT(implementation);
    return new AbstractObjectMap(*this, implementation->remapML(ml2ml, fromPEdge));
  // If this is a full or empty map, its remapped version is itself
  } else
    return copy();
}

// Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously
//    maintained in this lattice about them.
// Returns true if the Lattice state is modified and false otherwise.
bool AbstractObjectMap::replaceML(Lattice* newL) {
  if(mapState==between) {
    ROSE_ASSERT(implementation);
    AbstractObjectMap* newAOM = dynamic_cast<AbstractObjectMap*>(newL);
    ROSE_ASSERT(newAOM);
    return implementation->replaceML(newAOM->implementation);
  // If this is a full or empty map, nothing is changed
  } else
    return false;
}

// Propagate information from a set of defs to a single use
bool AbstractObjectMap::propagateDefs2Use(MemLocObjectPtr use, const std::set<MemLocObjectPtr>& defs) {
  SIGHT_VERB_DECL(scope, ("AbstractObjectMap::propagateDefs2Use()", scope::medium), 1, AOMDebugLevel)
  if(mapState==between) {
    ROSE_ASSERT(implementation);
    SIGHT_VERB(dbg << "use="<<use->str()<<", #defs="<<defs.size()<<endl, 1, AOMDebugLevel)
    AbstractionPtr unionLat;
    for(std::set<MemLocObjectPtr>::const_iterator d=defs.begin(); d!=defs.end(); ++d) {
      SIGHT_VERB(dbg << "unionLat="<<unionLat<<", def="<<(*d)->str()<<endl, 1, AOMDebugLevel)
      // If unionLat has not been assigned a Lattice, assign it to be a copy of the Lattice mapped to the current def.
      // unionLat is unmodified if nothing is mapped to the current def
      if(!unionLat) {
//        AbstractionPtr tmp = get(*d);
        unionLat = get(*d);
/*        cout << "unionLat="<<(unionLat? unionLat->str(): "NULL")<<endl;
        cout << "unionLat->copy="<<(unionLat? unionLat->copy()->str(): "NULL")<<endl;
        cout << "unionLat="<<(unionLat? unionLat->str(): "NULL")<<endl;*/
        //dbg << "unionLat->copyA="<<(unionLat? unionLat->copyA()->str(): "NULL")<<endl;
        //if(unionLat) unionLat = boost::shared_ptr<Lattice>(unionLat->copy());
//        Lattice* tmp = unionLat->copy();
//        cout << "tmp="<<(tmp? tmp->str(): "NULL")<<endl;
        //if(tmp) unionLat = boost::shared_ptr<Lattice>(tmp);

      // If unionLat already contains the union of all the Lattices mapped to the preceding defs,
      // union into it the Lattice mapped to the current def
      } else {
        if(AbstractionPtr defLat = get(*d))
          unionLat->meetUpdate(defLat, latPEdge, comp, analysis);
      }
    }
    SIGHT_VERB(dbg << "unionLat="<<(unionLat?unionLat->str():"NULL")<<endl, 1, AOMDebugLevel)

    // If we have Lattice information at some of the defs, assign it to the use
    if(unionLat) return insert(use, unionLat);
    else         return false;
  // If this is a full or empty map, nothing is changed
  } else
    return false;
}

// Propagate information from a single defs to a set of uses
bool AbstractObjectMap::propagateDef2Uses(const std::set<MemLocObjectPtr>& uses, MemLocObjectPtr def) {
  SIGHT_VERB_DECL(scope, ("AbstractObjectMap::propagateDef2Uses()", scope::medium), 1, AOMDebugLevel)
  if(mapState==between) {
    /*scope s("AbstractObjectMap::propagateDef2Uses");
    dbg << "latPEdge="<<latPEdge<<", #uses="<<uses.size()<<", def="<<def->str()<<endl;*/
    SIGHT_VERB(dbg << "#uses="<<uses.size()<<", def="<<def->str()<<endl, 1, AOMDebugLevel)

    ROSE_ASSERT(implementation);

    bool modified=false;
    // If there is a lattice mapped to def
    if(AbstractionPtr defLat = get(def)) {
      SIGHT_VERB(dbg << "defLat="<<defLat->str()<<endl, 1, AOMDebugLevel)
      // Assign it to all the uses
      for(std::set<MemLocObjectPtr>::const_iterator u=uses.begin(); u!=uses.end(); ++u) {
        SIGHT_VERB(dbg << "use="<<(*u)->str()<<endl, 1, AOMDebugLevel)
        modified = insert(*u, defLat) || modified;
      }
      //dbg << "modified="<<modified<<endl;
    }
    return modified;
  // If this is a full or empty map, nothing is changed
  } else
    return false;
}

// Computes the meet of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool AbstractObjectMap::meetUpdate(Lattice* thatL) {
  SIGHT_VERB_DECL(scope, (txt()<<"AbstractObjectMap::meetUpdate()", scope::medium), 1, AOMDebugLevel)
  /*dbg <<"this="<<str()<<endl;
  dbg <<"thatL="<<thatL->str()<<endl;*/
  AbstractObjectMap* that = dynamic_cast<AbstractObjectMap*>(thatL);
  ROSE_ASSERT(that);

  SIGHT_VERB(dbg << "mapState="<<state2Str(mapState)<<", that->mapState="<<state2Str(that->mapState)<<endl, 2, AOMDebugLevel)

  // Full sets cannot be expanded via union operations
  if(mapState==full) return false;

  // The union of an empty set and another set is the other set
  else if(mapState==empty) {
    if(that->mapState==full) return setToFull();
    else if(that->mapState==between) {
      ROSE_ASSERT(implementation==NULL);
      implementation = that->implementation->copy();
      implementation->setParent(this);
      mapState=between;
      return true;
    } else
      return false;
  }

  // Forward call to the implementation object
  else if(mapState==between) {
    ROSE_ASSERT(implementation);
    if(that->mapState==full) return setToFull();
    else if(that->mapState==empty) return false;
    else {
      ROSE_ASSERT(that->implementation);
      SIGHT_VERB(dbg << "Calling implementation->meetUpdate()"<<endl, 2, AOMDebugLevel)
      return implementation->meetUpdate(that->implementation);
    }
  }

#ifndef DISABLE_SIGHT
  scope s("AbstractObjectMap::meetUpdate()");
  dbg << "this->latPEdge="<<latPEdge->str()<<endl;
  dbg << "that->latPEdge="<<that->latPEdge->str()<<endl;
#endif

  ROSE_ASSERT(0);
}

// Computes the intersection of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool AbstractObjectMap::intersectUpdate(Lattice* thatL) {
  SIGHT_VERB_DECL(scope, ("AbstractObjectMap::intersectUpdate()", scope::medium), 2, AOMDebugLevel)
  AbstractObjectMap* that = dynamic_cast<AbstractObjectMap*>(thatL);
  ROSE_ASSERT(that);

  SIGHT_VERB(dbg << "mapState="<<state2Str(mapState)<<", that->mapState="<<state2Str(that->mapState)<<endl, 2, AOMDebugLevel)

  // Empty sets cannot be reduced via intersection operations
  if(mapState==empty) return false;

  // The intersection of a full set and another set is the other set
  else if(mapState==full) {
    if(that->mapState==empty) return setToEmpty();
    else if(that->mapState==between) {
      ROSE_ASSERT(implementation==NULL);
      implementation = that->implementation->copy();
      implementation->setParent(this);
      mapState=between;
      return true;
    } else
      return false;
  }

  // Forward call to the implementation object
  else if(mapState==between) {
    ROSE_ASSERT(implementation);
    if(that->mapState==empty) return setToEmpty();
    else if(that->mapState==full) return false;
    else {
      ROSE_ASSERT(that->implementation);
      SIGHT_VERB(dbg << "Calling implementation->intersectUpdate()"<<endl, 2, AOMDebugLevel)
      return implementation->intersectUpdate(that->implementation);
    }
  }

  ROSE_ASSERT(0);
}

bool AbstractObjectMap::finiteLattice() {
  return true;
//  // AbstractObjectMaps can grow to an arbitrary size and are thus inherently not finite
//  return false;
  /*if(mapState==between) {
    ROSE_ASSERT(implementation);
    return implementation->finiteLattice();
  // If this is a full or empty map, assume the lattice is finite
  } else
    return true;*/
}

bool AbstractObjectMap::operator==(Lattice* thatL) {
  AbstractObjectMap* that = dynamic_cast<AbstractObjectMap*>(thatL);
  ROSE_ASSERT(that);

  if(mapState==full || mapState==empty) return mapState==that->mapState;
  else if(that->mapState==between) {
    ROSE_ASSERT(implementation);
    ROSE_ASSERT(that->implementation);
    return *implementation.get() == that->implementation;
  } else
    return false;
}

/**********************
 ***** GenericAOM *****
 **********************/


// Add a new memory object --> lattice pair to the frontier.
// Return true if this causes the map to change and false otherwise.
// It is assumed that the given Lattice is now owned by the AbstractObjectMap and can be modified and deleted by it.
bool GenericAOM::insert(AbstractObjectPtr o, AbstractionPtr lattice) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("GenericAOM::insert()", scope::medium), 1, AOMDebugLevel)
  SIGHT_VERB_IF(1, AOMDebugLevel)
    dbg << "    o="<<(parent->latPEdge? o->strp(parent->latPEdge, ""): "NULL")<<" lattice="<<lattice->str("    ")<<endl;
    dbg << "        "<<str("        ")<<endl;
  SIGHT_VERB_FI()
#endif

  // Do not insert mappings for dead keys
  if(!o->isLive(parent->latPEdge, parent->comp, parent->analysis)) {
    SIGHT_VERB(dbg << "<b>GenericAOM::insert() WARNING: attempt to insert dead mapping "<<o->strp(parent->latPEdge)<<" =&gt; "<<lattice->str()<<"<\b>"<<endl, 1, AOMDebugLevel)
    return false;
  }

  //isFinite = isFinite && lattice->finiteLattice();

  bool retVal = false;
  bool insertDone = false;
  // Points to the Lattice mapped to key o after it has been inserted
  AbstractionPtr insertedLattice;
  bool mustEqualSeen = false;

  // First, check if there is a key on the frontier that must-equals o to determine
  // if we can just combine the new lattice with the old mapping
  list<MapElement>::iterator it;
  int i=0;
  for(it = items.begin(); it != items.end(); i++) {
    AbstractObjectPtr keyElement = it->first;
    SIGHT_VERB(dbg << "    keyElement="<<keyElement->str("            ")<<" mustEqual(o, keyElement, parent->latPEdge)="<<o->mustEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis)<<" insertDone="<<insertDone<<" mustEqualSeen="<<mustEqualSeen<<endl, 2, AOMDebugLevel)

    // If we're done inserting, don't do it again
    if(insertDone) {
      // If o is mustEqual to this element and it is not the first match, remove this element
      if(o->mustEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis)) {
        //if(mustEqualSeen) {
          items.erase(it++);
        //} else
        //  it++;
        mustEqualSeen = true;
      // If o denotes the same set as keyElement and has already been inserted into the map, any get that mayEqual
      // to o is guaranteed to be mayEqual to keyElement. As such, we just meet keyElement's lattice with o's lattice
      // and remove the keyElement's mapping
      } else if(o->equalSet(keyElement, parent->latPEdge, parent->comp, parent->analysis)) {
         retVal = insertedLattice->meetUpdate(it->second, parent->latPEdge, parent->comp, parent->analysis) || retVal;
         items.erase(it++);
      } else
        it++;
      continue;
    }

    // If the o-frontier contains an object that must-equal to
    if(o->mustEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis)) {
      SIGHT_VERB(dbg << "    keyElement="<<keyElement->str("            ")<<" mustEqual(o, keyElement, parent->latPEdge)="<<o->mustEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis)<<" insertDone="<<insertDone<<" mustEqualSeen="<<mustEqualSeen<<endl, 1, AOMDebugLevel)
      SIGHT_VERB_IF(1, AOMDebugLevel)
#ifndef DISABLE_SIGHT
        dbg << "    Must Equal"<<endl;
        dbg << "        lattice="<<lattice->str("        ")<<endl;
        dbg << "        it="<<it->second->str("        ")<<endl;
#endif
      SIGHT_VERB_FI()

      // If the old and new mappings of o are different,  we remove the old mapping and add a new one
      if(!it->second->equalSet(lattice, parent->latPEdge, parent->comp, parent->analysis))
      {
        SIGHT_VERB(dbg << "    keyElement="<<keyElement->str("            ")<<" mustEqual(o, keyElement, parent->latPEdge)="<<o->mustEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis)<<" insertDone="<<insertDone<<" mustEqualSeen="<<mustEqualSeen<<endl, 1, AOMDebugLevel)
        SIGHT_VERB(dbg << "    Removing i="<<i<<", inserting "<<o->strp(parent->latPEdge, "        ")<<"=&gt;"<<lattice->str("        ")<<endl, 1, AOMDebugLevel)
        items.erase(it++);
        items.push_front(MapElement(o, lattice));
        retVal = true;
      } else {
        //dbg << "    No Change"<<endl;
        it++;
        // Otherwise, they're identical and thus there is no need to modify the map
        retVal = false;
      }
      insertDone = true;
      insertedLattice = lattice;
      mustEqualSeen = true;
    // If the new element and the original actually denote the same set
    } else if(o->equalSet(keyElement, parent->latPEdge, parent->comp, parent->analysis)) {
      // Meet their respective lattices of the
      //dbg << "o="<<o->str()<<" <b>equalSet</b> "<<keyElement<<" keyElement="<<keyElement->str()<<endl;
      retVal = it->second->meetUpdate(lattice, parent->latPEdge, parent->comp, parent->analysis) || retVal;
      insertedLattice = it->second;
      it++;
      insertDone = true;
    // If the element on the o-frontier may-equals o (their sets overlap) then insert a new o->lattice mapping
    // since the new lattice cannot be combined with the mapping of *it
    } else if(o->mayEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis)) {
      //dbg << "o="<<o->str()<<" <b>mayEqual</b> "<<keyElement<<" keyElement="<<keyElement->str()<<endl;
      items.push_front(MapElement(o, lattice));
      retVal = true;
      insertedLattice = lattice;
      it++;
      insertDone = true;
    } else
      it++;
  }

  if(!insertDone) {
    // There are no objects within this map on the o-frontier. As such, add an o->lattice mapping
    items.push_front(MapElement(o, lattice));

    retVal = true;
  }

  // Having inserted the new item we need to clean up the map to ensure that it stays bounded in size
  // Step 1: call isEmpty to check for any keys mapped to empty sets
  isEmpty();
  // Step 2: if the map is larger than some fixed bound, merge some key->value mappings together
  // !!! TODO !!!

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(1, AOMDebugLevel)
    indent ind();
    dbg << "retVal="<<retVal<<" insertDone="<<insertDone<<" mustEqualSeen="<<mustEqualSeen<<endl;
    dbg << str()<<endl;
  SIGHT_VERB_FI()
#endif
  return retVal;
};

// Removes the key matching the argument from the frontier.
// Return true if this causes the map to change and false otherwise.
bool GenericAOM::remove(AbstractObjectPtr abstractObjectPtr) {
  for (list<MapElement>::iterator it = items.begin();
       it != items.end(); it++) {
    AbstractObjectPtr keyElement = it->first;
    // For remove operation, we use must equal policy
    if (abstractObjectPtr->mustEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis)) {
      it = items.erase(it);
      return true;
    }
  }
  return false;
};

// Get all x-frontier for a given abstract memory object
AbstractionPtr GenericAOM::get(AbstractObjectPtr abstractObjectPtr) {
  SIGHT_VERB_DECL(scope, ("GenericAOM::get()", scope::medium), 1, AOMDebugLevel)
  SIGHT_VERB_IF(1, AOMDebugLevel)
#ifndef DISABLE_SIGHT
    dbg << "    o="<<abstractObjectPtr->strp(parent->latPEdge, "    ")<<endl;
    dbg << "        "<<str("        ")<<endl;
#endif
  SIGHT_VERB_FI()

  AbstractionPtr ret;
  for (list<MapElement>::iterator it = items.begin();
       it != items.end(); it++) {
    AbstractObjectPtr keyElement = it->first;
    bool eq = abstractObjectPtr->mayEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis);
#ifndef DISABLE_SIGHT
    if(AOMDebugLevel>=2 || (AOMDebugLevel>=1 && eq)) dbg << "    keyElement(equal="<<eq<<")="<<keyElement->str("    ")<<endl;
#endif
    if(eq) {
      // If this is the first matching Lattice, copy this Lattice to ret
      if(!ret) ret = it->second->copyA();
      // Otherwise, merge this latice into ret
      else     ret->meetUpdate(it->second, parent->latPEdge, parent->comp, parent->analysis);

      // If the current key must-equals the given object, its assignment must have overwritten any prior assignment
      // to this object, meaning that prior assignments can be ignored
      if(abstractObjectPtr->mustEqual(keyElement, parent->latPEdge, parent->comp, parent->analysis)) {
        SIGHT_VERB(dbg << "    Stopping search since mustEqual, ret="<<ret->str("    ")<<endl, 1, AOMDebugLevel)
        break;
      }

      SIGHT_VERB(dbg << "    ret="<<ret->str("    ")<<endl, 1, AOMDebugLevel)
    }
  }

  SIGHT_VERB(dbg << "ret="<<(ret ? ret->str("    "): "NULL")<<endl, 1, AOMDebugLevel)
  if(ret) return ret;
  // If there is no match for abstractObjectPtr, return a copy of the default lattice
  return parent->defaultLat->copyA();
};

// Set all the information associated Lattice object with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
bool GenericAOM::setMLValueToFull(MemLocObjectPtr ml)
{
  bool modified = false;

  // Iterate through all the keys in the items list. If any key is mayEqual(ml) then its associated
  // value is set to full. Note that this works even if the keys are not MemLobObjectPtrs since in
  // that case mustEqual will return false.
  for(list<MapElement>::iterator it = items.begin(); it != items.end(); it++) {
    AbstractObjectPtr keyElement = it->first;
    if(keyElement->mayEqual(AbstractObjectPtr(ml), parent->latPEdge, parent->comp, parent->analysis)) {
      modified = it->second->setToFull(parent->latPEdge, parent->comp, parent->analysis) || modified;
    }
  }
  return modified;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool GenericAOM::isFull()
{
  return false;
}

/*set<AbstractObjectPtr> cacheAO;*/
set<AbstractionPtr> cache;

// Returns whether this lattice denotes the empty set.
bool GenericAOM::isEmpty()
{
  SIGHT_VERB_DECL(scope, ("GenericAOM::isEmpty()", scope::medium), 2, AOMDebugLevel)
  SIGHT_VERB(dbg << "this="<<str()<<endl, 2, AOMDebugLevel)

  // Check if all items are empty
  for(std::list<MapElement>::iterator i=items.begin(); i!=items.end();) {
    SIGHT_VERB_IF(2, AOMDebugLevel)
#ifndef DISABLE_SIGHT
      indent ind;
      dbg << "i->first="<<i->first->str()<<endl;
      dbg << "i->second="<<i->second->str()<<endl;
#endif
    SIGHT_VERB_FI()
    // If at least one is not empty, return false
    if(!(i->first)->isEmpty(parent->getPartEdge(), parent->comp, parent->analysis) &&
       !(i->second)->isEmpty(parent->latPEdge, parent->comp, parent->analysis)) return false;

    // If this item mapping is empty, remove it from the items list
    SIGHT_VERB(dbg << "Deleting Empty mapping", 2, AOMDebugLevel)

    //cache.insert(i->second);
    //cacheAO.insert(i->first);
//dbg << "#i->second="<<i->second.use_count()<<endl;
//dbg << "#defaultLat="<<parent->defaultLat.use_count()<<endl;
    items.erase(i++);
//    ++i;
  }
  // If all are empty, return true
  assert(items.size()==0);
  return true;
//  return items.size()>0;
}

std::string GenericAOM::str(std::string indent) const {
  return strp(parent->latPEdge, indent);
}

// Variant of the str method that can produce information specific to the current Part.
// Useful since AbstractObjects can change from one Part to another.
std::string GenericAOM::strp(PartEdgePtr pedge, std::string indent) const
{
  ostringstream oss;
  oss << "<u>GenericAOM:</u>";
  oss << "<table border=1><tr><td>Key</td><td>Value</td>";
  for(list<MapElement>::const_iterator it = items.begin();
       it != items.end(); it++) {
    //printf("\n%s%p =&gt; %p\n", indent.c_str(), it->first.get(), it->second.get()); fflush(stdout);
    oss << "<tr><td>#"<<it->first.use_count()<<"   ";
    oss << it->first->strp(pedge, indent+"    ");
    oss << "</td><td>#"<<it->second.use_count()<<"   ";
    oss << it->second->str(indent+"        ");
    oss << "</td></tr>";
  }
  oss << "</table>"<<endl;
  return oss.str();
}

// initializes this Lattice to its default state, if it is not already initialized
void GenericAOM::initialize() {}

// returns a copy of this lattice
AbstractObjectMapKindPtr GenericAOM::copy() const
{ return boost::make_shared<GenericAOM>(*this); }

// overwrites the state of this Lattice with that of that Lattice
void GenericAOM::copy(AbstractObjectMapKindPtr thatL) {
  GenericAOMPtr that = boost::dynamic_pointer_cast <GenericAOM> (thatL);
  ROSE_ASSERT(that);
  items = that->items;
  //isFinite = that->isFinite;
}

// Called by analyses to transfer this lattice's contents from across function scopes from a caller function
//    to a callee's scope and vice versa. If this this lattice maintains any information on the basis of
//    individual MemLocObjects these mappings must be converted, with MemLocObjects that are keys of the ml2ml
//    replaced with their corresponding values. If a given key of ml2ml does not appear in the lattice, it must
//    be added to the lattice and assigned a default initial value. In many cases (e.g. over-approximate sets
//    of MemLocObjects) this may not require any actual insertions. If the value of a given ml2ml mapping is
//    NULL (empty boost::shared_ptr), any information for MemLocObjects that must-equal to the key should be
//    deleted.
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    the keys in ml2ml are in scope on one side of Part, while the values on the other side. Specifically, it is
//    guaranteed that the keys are in scope at fromPEdge while the values are in scope at the edge returned
//    by parent->getPartEdge().
// remapML must return a freshly-allocated object.
AbstractObjectMapKindPtr GenericAOM::remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge)
{
  // Do nothing on empty maps or those where the keys are not MemLocObjects
  if(items.size()==0 || !(items.begin()->first->isMemLocObject())) { return copy(); }

  SIGHT_VERB_DECL(scope, ("GenericAOM::remapML", scope::medium), 1, AOMDebugLevel)

  SIGHT_VERB_IF(1, AOMDebugLevel)
#ifndef DISABLE_SIGHT
    // If either the key or the value of this mapping is dead within its respective part, we skip it.
    // Print notices of this skipping once
    for(std::set<MLMapping>::const_iterator m=ml2ml.begin(); m!=ml2ml.end(); m++) {
      if(!m->from) continue;
      // If either the key or the value of this mapping is dead within its respective part, skip it
      if(!m->from->isLive(fromPEdge, parent->comp, parent->analysis) || (m->to && !m->to->isLive(parent->latPEdge, parent->comp, parent->analysis)))
        dbg << "<b>GenericAOM::remapML() WARNING: Skipping dead ml2ml mapping "
	    << m->from->strp(fromPEdge)<<"(live="<<m->from->isLive(fromPEdge, parent->comp, parent->analysis)<<") =&gt; "
	    << (m->to ? m->to->strp(parent->latPEdge) : "NULL")
	    << "(live="<<(m->to ? m->to->isLive(parent->latPEdge, parent->comp, parent->analysis) : -1)<<")"<<endl
	    << "    fromPEdge=["<<fromPEdge->str()<<"]"<<endl
	    << "    parent->latPEdge=["<<(parent->latPEdge? parent->latPEdge->str(): "NULL")<<"]</b>"<<endl;

    }

  {
    scope reg("ml2ml", scope::medium);
    for(std::set<MLMapping>::const_iterator m=ml2ml.begin(); m!=ml2ml.end(); m++) {
      if(!m->from) continue;
      dbg << m->from.get()->str() << " =&gt; " << (m->to? m->to.get()->strp(parent->latPEdge): "NULL") << endl;
    }
    //dbg << "this="<<str()<<endl;
  }
#endif
  SIGHT_VERB_FI()

  // Copy of this map where the keys in ml2ml have been remapped to their corresponding values
  boost::shared_ptr<GenericAOM> newM = boost::make_shared<GenericAOM>(*this);

  // Vector of flags that indicate whether a given key in ml2ml has been added to newM or not
  vector<bool> ml2mlAdded;

  // Initialize ml2mlAdded to all false
  for(std::set<MLMapping>::const_iterator m=ml2ml.begin(); m!=ml2ml.end(); m++)
    ml2mlAdded.push_back(false);

  SIGHT_VERB(dbg << "newM="<<newM->str()<<endl, 2, AOMDebugLevel)

  // Iterate over all the mappings <key, val> n ml2ml and for each mapping consider
  // each item in newM. If the key mustEquals to some item newM, that item is replaced
  // by val. If the key mayEquals some item in newM, val is placed at the front of
  // the list. If the key does not appear in newM at all, val is placed at the front of
  // the list.
  for(std::list<MapElement>::iterator i=newM->items.begin(); i!=newM->items.end(); ) {
    SIGHT_VERB_DECL(indent, (), 1, AOMDebugLevel)
    SIGHT_VERB(dbg << "i="<<i->first->str()<<endl, 1, AOMDebugLevel)

    int mIdx=0;
    std::set<MLMapping>::const_iterator m=ml2ml.begin();
    for(; m!=ml2ml.end(); m++, mIdx++) {
      if(!m->from) continue;

      SIGHT_VERB_DECL(indent, (), 1, AOMDebugLevel)
      SIGHT_VERB(dbg << mIdx << ": m-&gt;key="<<m->from->strp(fromPEdge)<<endl, 1, AOMDebugLevel)

      SIGHT_VERB_DECL(indent, (), 1, AOMDebugLevel)
      // If the current item in newM may- or must-equals a key in ml2ml, record this and update newM
      SIGHT_VERB_IF(1, AOMDebugLevel)
#ifndef DISABLE_SIGHT
        dbg << "i-&gt;first mustEqual m-&gt;from = "<<i->first->mustEqual((AbstractObjectPtr)m->from, fromPEdge, parent->comp, parent->analysis)<<endl;
        dbg << "i-&gt;first mayEqual m-&gt;from = "<<i->first->mayEqual((AbstractObjectPtr)m->from, fromPEdge, parent->comp, parent->analysis)<<endl;
#endif
      SIGHT_VERB_FI()
      if(i->first->mustEqual((AbstractObjectPtr)m->from, fromPEdge, parent->comp, parent->analysis) && m->replaceMapping) {
        // If the value of the current ml2ml mapping is not-NULL
        if(m->to) {
          // Replace the current item in newM with the value of the current pair in ml2ml
          *i = make_pair(boost::static_pointer_cast<AbstractObject>(m->to), i->second);

          // Advance onward in newM and remove any items that are must-equal to the value of the current ml2ml mapping
          //scope reg("Deleting items that are must-equal to value", scope::medium));
          std::list<MapElement>::iterator iNext = i; iNext++;
          for(std::list<MapElement>::iterator j=iNext; j!=newM->items.end(); ) {
            SIGHT_VERB_IF(2, AOMDebugLevel)
#ifndef DISABLE_SIGHT
              dbg << "j="<<j->first<<" => "<<j->second<<endl;
              dbg << mIdx << ": m-&gt;value="<<m->to->strp(fromPEdge)<<endl;
              dbg << "j-&gt;first mustEqual m-&gt;to = "<<j->first->mustEqual((AbstractObjectPtr)m->to, fromPEdge, parent->comp, parent->analysis)<<endl;
#endif
            SIGHT_VERB_FI()
            if(j->first->mustEqual((AbstractObjectPtr)m->to, fromPEdge, parent->comp, parent->analysis)) {
              SIGHT_VERB(dbg << "Erasing j="<<j->first->str()<<" => "<<j->second->str()<<endl, 2, AOMDebugLevel)
              j = newM->items.erase(j);
              //break;
            } else
              j++;
          }
        // If the value of the current ml2ml mapping is NULL (i.e. the key is a MemLoc with a lifetime that is limited
        // to a given function and it does not carry over across function boundaries)
        } else {
          // Erase this mapping
          i = newM->items.erase(i);
          break;
        }
        ml2mlAdded[mIdx]=true;
      } else if(i->first->mayEqual((AbstractObjectPtr)m->from, fromPEdge, parent->comp, parent->analysis)) {
        // Insert the value in the current ml2ml mapping immediately before the current item
        SIGHT_VERB(dbg << "Inserting before i: "<<m->to->str()<<" => "<<i->second->str()<<endl, 1, AOMDebugLevel)
        newM->items.insert(i, make_pair(boost::static_pointer_cast<AbstractObject>(m->to), i->second));
        ml2mlAdded[mIdx]=true;
      }
    }

    // If we broke out early, we must have erased the current element in newM, meaning that we shouldn't advance
    // i again. Otherwise, advance i.
    if(m==ml2ml.end())
      i++;
  }

  // Iterate through the false mappings in ml2mlAdded (ml2ml keys that were not mapped to any items in this map)
  // and add to newM a mapping of their values to parent->defaultLat (as long as the values are not NULL)
  int mIdx=0;
  for(std::set<MLMapping>::iterator m=ml2ml.begin(); m!=ml2ml.end(); m++, mIdx++) {
    if(!m->from) continue;

    //dbg << "False mapping "<<m->from->str()<<endl;
    // If either the key or the value of this mapping is dead within its respective part, skip it
    if(!m->from->isLive(fromPEdge, parent->comp, parent->analysis) || !(m->to && m->to->isLive(parent->latPEdge, parent->comp, parent->analysis))) continue;

    if(!ml2mlAdded[mIdx] && m->to)
      newM->items.push_back(make_pair(m->to, parent->defaultLat->copyA()));
  }

  return newM;
}

// Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously
//    maintained in this lattice about them.
// Returns true if the Lattice state is modified and false otherwise.
bool GenericAOM::replaceML(AbstractObjectMapKindPtr newL)
{
  boost::shared_ptr<GenericAOM> calleeAOM = boost::dynamic_pointer_cast<GenericAOM>(newL);
  assert(calleeAOM);

  bool modified = false;

  for(std::list<MapElement>::iterator i=calleeAOM->items.begin(); i!=calleeAOM->items.end(); i++) {
    // Do not copy over mappings with keys that are dead in this map's host PartEdge
    if(!i->first->isLive(parent->latPEdge, parent->comp, parent->analysis)) continue;
    modified = insert(i->first, i->second) || modified;
  }

  return modified;
}

// Computes either the union or intersection of this and that, as specified by the ui parameter,
// and saves the result in this.
// Returns true if this causes this to change and false otherwise
bool GenericAOM::unionIntersectUpdate(AbstractObjectMapKindPtr thatL, uiType ui)
{
  SIGHT_VERB_DECL(scope, (txt()<<"GenericAOM::unionIntersectUpdate("<<(ui==Union?"Union":"Intersect")<<")", scope::medium), 2, AOMDebugLevel);

  // Both incorporateVars() and meetUpdate currently call merge. This is clearly not
  // right but we'll postpone fixing it until we have the right algorithm for merges
  bool modified = false;
  try {
    boost::shared_ptr<GenericAOM> that = boost::dynamic_pointer_cast<GenericAOM>(thatL);
    assert(that);

    SIGHT_VERB_IF(2, AOMDebugLevel)
#ifndef DISABLE_SIGHT
      dbg << "parent->latPEdge="<<(parent->latPEdge? parent->latPEdge->str(): "NULL")<<endl;
      { scope thisreg("this", scope::medium);
      dbg << str()<<endl; }
      { scope thisreg("that", scope::medium);
      dbg << that->str()<<endl; }
#endif
    SIGHT_VERB_FI()

    // This algorithm is based on the following insights:
    // Given two AbstractObjectMaps:
    //                     A: a_0 => a_1 => ... => a_n, and
    //                     B: b_0 => b_1 => ... => b_m
    //    where x => y if x is inserted after y
    // The only non-conservative thing that a merge algorithm can do is take a_i and b_j, where
    //   mustEqual(a_i, b_j) and place one before the other in the merged list. For these cases
    //   it is necessary to insert a single entry a_i -> A[a_i] merge B[b_j].
    // All elements that do not have a mustEqual partner in the same AbstractObjectMap or the other AbstractObjectMap
    //   can be inserted into the new map in arbitrary order. GenericAOM::insert ensures that no two elements
    //   that are mustEqual to each other may be in the same map. For elements across maps the following
    //   holds. If a_p and b_q are mayEqual to each other, then keeping them in the reverse order from
    //   their correct assignment order is conservative but may lose precision. This is because if
    //   a_p => b_q but are stored in the opposite order and for some x it is true that mayEqual(x, b_p), mustEqual(x, a_p),
    //   then GenericAOM::get(x) will merge the lattices stored at the two keys whereas the precise result would
    //   be to return the lattice stored under a_p. If a_p and b_q are not mayEqual to each other,
    //   either order is fine with no loss of precision.
    // As an additional enhancement suppose a_i and b_j are not must-equal but denote the same set. In this case,
    //   although it is conservative to place them in either order, it is also useless since any abstract object
    //   that mayEquals a_i, must also mayEqual b_j. As such, we insert a single entry for a_i -> A[a_i] merge B[b_j]
    //   if a_i and b_j are mustEqual or equalSet. Since mustEqual implies equalSet, we only check equalSet.
    // The algorithm below chooses a simple order that is likely to work well in practice. It connects
    //   the pairs of elements in this->items(A) and that->items(B) that are mustEqual and then
    //   scans over each such pair <a_i, b_j> in the order they appear A, copying all the elements between
    //   the b_j and b_j+1 over to this map between a_i and a_i+1 if they've not already been copied over and
    //   if they don't have a mustEquals partner in A (these are handled by merging, as described above).
    // For example,
    //   A: a_0 => a_1 => r => a_2 => s => a_3 => t
    //   B: b_0 =>        r => b_1 =>             t => b_2 => b_3 => s
    //   A.mergeUpdate(B): a_0 => a_1 => b_0 => r => a_2 => b_1 => b_2 => b_3 => s => a_3 => t
    // Further,

    // For each element x in this->items pointers that is mustEqual to an element y in
    // that->items, keeps the triple
    //    - iterator that points to x in this->items
    //    - iterator that points to y in that->items
    //    - index of y in that->items
    // Maintained in order of this->items.
    list<pair<list<MapElement>::iterator, pair<list<MapElement>::iterator, int> > > thisMustEq2thatMustEq;

    // For each element in that->items keeps true if this element is mustEquals to some
    // element in this->items and false otherwise.
    list<bool> thatMustEq;

    // Initialize thatMustEq to all false
    for(list<MapElement>::iterator itThat=that->items.begin(); itThat!=that->items.end(); itThat++)
      thatMustEq.push_back(false);

    SIGHT_VERB_IF(2, AOMDebugLevel)
#ifndef DISABLE_SIGHT
      scope thisreg("that->items", scope::medium);
      for(list<MapElement>::iterator itThat=that->items.begin(); itThat!=that->items.end(); itThat++)
      dbg << "that: "<<itThat->first->str()<<" ==&gt; "<<itThat->second->str()<<endl;
#endif
    SIGHT_VERB_FI()

    // Determine which elements in this->items are mustEqual to elements in that->items
    // and for these pairs merge the lattices from that->items to this->items.
    for(list<MapElement>::iterator itThis=items.begin();
       itThis!=items.end(); itThis++) {
      SIGHT_VERB(scope thisreg("itThis", scope::medium), 2, AOMDebugLevel)
      SIGHT_VERB(dbg << "this: "<<itThis->first->str()<<" ==&gt; "<<itThis->second->str()<<endl, 2, AOMDebugLevel)

      int i=0;
      list<bool>::iterator thatMEIt=thatMustEq.begin();
      for(list<MapElement>::iterator itThat=that->items.begin();
         itThat!=that->items.end(); itThat++, i++, thatMEIt++) {

        SIGHT_VERB(scope thisreg("itThat", scope::medium), 2, AOMDebugLevel)
        SIGHT_VERB(dbg << "that: "<<itThat->first->str()<<" ==&gt; "<<itThat->second->str()<<endl, 2, AOMDebugLevel)

        // If we've found a pair of keys in this and that that are mustEqual or denote the same set
        //if(mustEqual(itThis->first, itThat->first, parent->latPEdge, parent->comp, parent->analysis)) {
        if(itThis->first->equalSet(itThat->first, parent->latPEdge, parent->comp, parent->analysis)) {
          // Record this pair
          thisMustEq2thatMustEq.push_back(make_pair(itThis, make_pair(itThat, i)));
          *thatMEIt = true;

          SIGHT_VERB(scope meetreg(txt()<<"Meeting", scope::medium), 2, AOMDebugLevel)

          // Update the lattice at *itThis to incorporate information at *itThat
          {
            // First copy the lattice since it may change. We don't deep-copy lattices when we copy
            // AbstractObjectMaps, so multiple maps may contain references to the same lattice.
            // As such, instead of updating lattices in-place (this would update the same lattice
            // in other maps) we first copy them and update into the copy.
            itThis->second = AbstractionPtr(itThis->second->copyA());
            SIGHT_VERB(scope meetreg(txt()<<"Meeting "<<itThis->first->str(), scope::medium), 2, AOMDebugLevel)
            SIGHT_VERB(scope befreg("before", scope::low); dbg << itThis->second->str()<<endl, 2, AOMDebugLevel)
            if(ui==Union) modified = itThis->second->meetUpdate(itThat->second, parent->latPEdge, parent->comp, parent->analysis) || modified;
            else          modified = itThis->second->intersectUpdate(itThat->second, parent->latPEdge, parent->comp, parent->analysis) || modified;
            SIGHT_VERB_IF(2, AOMDebugLevel)
#ifndef DISABLE_SIGHT
            { scope aftreg("after", scope::low); dbg << itThis->second->str()<<endl; }
            dbg << "modified="<<modified<<endl;
#endif
            SIGHT_VERB_FI()
          }
        }
      }
    }
    SIGHT_VERB_IF(2, AOMDebugLevel)
#ifndef DISABLE_SIGHT
      scope eqreg("thisMustEq2thatMustEq", scope::medium);
      for(list<pair<list<MapElement>::iterator, pair<list<MapElement>::iterator, int> > >::iterator it=thisMustEq2thatMustEq.begin();
          it!=thisMustEq2thatMustEq.end(); it++) {
        dbg << (it->first)->first->str() << " =&gt; " << (it->second).first->second->str() << endl;
      }
#endif
    SIGHT_VERB_FI()

    {
      SIGHT_VERB(scope insreg("inserting that->this", scope::medium), 2, AOMDebugLevel);

    // Copy over the mappings of all the elements in that->items that were not mustEqual
    // to any elements in this->items. Although any order will work for these elements,
    // keep them their order in that->items.
    int thatIdx=0;
    list<MapElement>::iterator thatIt = that->items.begin();
    list<bool>::iterator thatMEIt=thatMustEq.begin();
    for(list<pair<list<MapElement>::iterator, pair<list<MapElement>::iterator, int> > >::iterator meIt=thisMustEq2thatMustEq.begin();
       meIt!=thisMustEq2thatMustEq.end(); meIt++) {
      SIGHT_VERB(scope mapreg(txt()<<"mustEqual mapping "<<meIt->second.second<<": "<<(meIt->first)->first->str(), scope::medium), 2, AOMDebugLevel)
      SIGHT_VERB_IF(2, AOMDebugLevel)
#ifndef DISABLE_SIGHT
        dbg << "this: "<<meIt->first->first->str() << " =&gt; " << meIt->first->second->str() <<endl;
        dbg << "that: "<<(meIt->second).first->first->str() << " =&gt; " << (meIt->second).first->second->str() << endl;
        dbg << "thatIdx="<<thatIdx<<endl;
#endif
      SIGHT_VERB_FI()
	

      // Copy over all the mappings from that->items from thatIt to meIt's partner in that->items
      // if they have not already been copied because elements that are mustEqual to each other were ordered
      // differently in this->items and that->items
      if(meIt->second.second >= thatIdx) {
        for(; thatIt!=meIt->second.first; thatIt++, thatIdx++, thatMEIt++) {
          // Copy over the current element from that->items if it doesn't have a mustEqual
          // partner in this->items (i.e. its already been handled)
          if(!(*thatMEIt)) {
#ifndef DISABLE_SIGHT
            SIGHT_VERB(dbg
		       << "Inserting at meIt->first="<<(meIt->first)->first->str()
		       <<" mapping "<<thatIt->first->str()<<" ==&gt; "
		       << thatIt->second->str()
		       << endl, 2, AOMDebugLevel)
#endif
            // NOTE: we do not currently update the part field in the lattice thatIt->second
            //       to refer to this->parent->latPEdge. Perhaps we should make a copy of it and update it.
            items.insert(meIt->first, *thatIt);
            modified = true;
          } else { SIGHT_VERB(dbg << "mustEqual partner exists in this"<<endl, 2, AOMDebugLevel) }
        }
        // Advance thatIt and thatIdx once more to account for the partner in that->items
        // of the current entry in this->items
        thatIt++;
        thatIdx++;
      }
      //if(AOMDebugLevel()>=2) dbg << "modified="<<modified<<endl;
    }

    // Add all the elements from that->items that remain
    for(; thatIt!=that->items.end(); thatIt++) {
      SIGHT_VERB(dbg << "Pushing end "<<thatIt->first->str()<<" ==&gt; "<<thatIt->second->str()<<endl, 2, AOMDebugLevel)
      // NOTE: we do not currently update the part field in the lattice thatIt->second
      //       to refer to this->parent->latPEdge. Perhaps we should make a copy of it and update it.
      items.push_back(*thatIt);
      modified = true;
    }
    //dbg << "    items.size()="<<items.size()<<"\n";
   }

    // parent->compress all the elements from that are now mustEqual to each other in this->parent->latPEdge.
    // Note: the optimal way to do this is to parent->compress that->mustEqual first and then
    //       merge but we're not allowed to modify that so the parent->compression would need
    //       to be done non-destructively via some additional datastructure. We avoid
    //       this parent->complication for now but should revisit this question if we identify
    //       this code region as a performance bottleneck.
    //dbg << "Before mustEq parent->compression "<<str()<<endl;
    // GB: I don't think we need this parent->compression since we've already performed all the mustEqual
    //     (actually equalSet) matching and therefore should not have any keys that are mustEqual to each other.
    //parent->compressMustEq();

    // Remove all the dead keys
    //dbg << "Before dead parent->compression "<<str()<<endl;
    compressDead();

    //dbg << "Final "<<str()<<endl;
  } catch (bad_cast & bc) {
    assert(false);
  }
  SIGHT_VERB(dbg << "Final modified="<<modified<<endl, 2, AOMDebugLevel)
  return modified;
}

// Computes the meet of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool GenericAOM::meetUpdate(AbstractObjectMapKindPtr thatL)
{
  return unionIntersectUpdate(thatL, Union);
}

// Computes the meet of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool GenericAOM::intersectUpdate(AbstractObjectMapKindPtr thatL)
{
  return unionIntersectUpdate(thatL, Intersection);
}

// Identify keys that are must-equal to each other and merge their lattices
// Return true if this causes the object to change and false otherwise.
bool GenericAOM::compressMustEq()
{
  /*dbg << "parent->compressMustEq()"<<endl;
  dbg << "    "<<str("    ")<<endl;*/

  bool modified = false;
  int xIdx=0;
  for(list<MapElement>::iterator x = items.begin(); x != items.end(); x++, xIdx++) {
    //dbg << "    "<<xIdx<<" : x="<<x->first->str("")<<endl;
    // y starts from the element that follows x
    list<MapElement>::iterator y = x;
    y++;
    int yIdx = xIdx+1;
    for(; y != items.end(); yIdx++) {
      //dbg << "        "<<yIdx<<" : y="<<y->first->str("")<<endl;
      // If x and y are equal, merge their lattices and remove the later one
      if(x->first->mustEqual(y->first, parent->latPEdge, parent->comp, parent->analysis)) {
        //dbg << "    MERGING and REMOVING"<<endl;
        // First copy the lattice since it may change. We don't deep-copy lattices when we copy
        // AbstractObjectMaps, so multiple maps may contain references to the same lattice.
        // As such, instead of updating lattices in-place (this would update the same lattice
        // in other maps) we first copy them and update into the copy.
        x->second = x->second->copyA();
        modified = x->second->meetUpdate(y->second, parent->latPEdge, parent->comp, parent->analysis) || modified;

        list<MapElement>::iterator tmp = y;
        y++;
        items.erase(tmp);

        //dbg << "        map="<<str("            ")<<endl;

        modified = true;
      } else
        y++;
    }
  }

  return modified;
};

// Remove all mappings with dead keys from this map.
// Return true if this causes the object to change and false otherwise.
bool GenericAOM::compressDead()
{
  SIGHT_VERB_DECL(scope, ("compressDead", scope::low), 2, AOMDebugLevel)
  bool modified = false;
  for(list<MapElement>::iterator i = items.begin(); i != items.end(); ) {
    SIGHT_VERB(dbg << "i: "<<i->first.get()->str()<<" ==&gt"<<endl<<"          "<<i->second.get()->str()<<endl, 2, AOMDebugLevel)

    // Remove mappings with dead keys
    if(!(i->first->isLive(parent->latPEdge, parent->comp, parent->analysis))) {
      list<MapElement>::iterator nextI = i;
      nextI++;

      SIGHT_VERB(dbg << "Erasing "<<i->first.get()->str()<<endl, 2, AOMDebugLevel)
      items.erase(i);
      modified = true;

      i = nextI;
    } else
      i++;
  }

  return modified;
}

/*bool GenericAOM::finiteLattice()
{
  //return isFinite;
  // AbstractObjectMaps can grow to an arbitrary size and are thus inherently not finite
  return false;
}*/

bool GenericAOM::operator==(AbstractObjectMapKindPtr that_arg)
{
  GenericAOMPtr that = boost::dynamic_pointer_cast<GenericAOM>(that_arg);
  ROSE_ASSERT(that);
  ROSE_ASSERT(parent->latPEdge == that->parent->getPartEdge());
  // This will be written once we have the merging algorithm to test
  // these maps' frontiers for semantic equivalence
  return false;
}

/***************************
 ***** HierarchicalAOM *****
 ***************************/

HierarchicalAOM::NodePtr HierarchicalAOM::NULLNode;

HierarchicalAOM::Node::Node() {
  isObjSingleton = false;
  originalVal = true;
}

// Creates a sub-tree that holds the remaining portions of the key, from subKey until keyEnd
// and places val at the leaf of this sub-tree
HierarchicalAOM::Node::Node(comparablePtr myKey, std::list<comparablePtr>::const_iterator subKey, std::list<comparablePtr>::const_iterator keyEnd,
                            AbstractionHierarchy::hierKeyPtr fullKey, AbstractObjectPtr obj, AbstractionPtr val) {
  //SIGHT_VERB_DECL(scope, ("HierarchicalAOM::Node::init()", scope::medium), 2, AOMDebugLevel)
  //SIGHT_VERB(dbg << "obj="<<obj->str(), 2, AOMDebugLevel)
  std::list<comparablePtr>::const_iterator next = subKey; ++next;
  // If this is the last sub-key in the key, place obj and val inside this Node
  if(subKey==keyEnd) {
    this->isObjSingleton = isSingleton(obj);
    this->fullKey = fullKey;
    this->val = val;
  // Otherwise, create a Node of the next sub-key in the key
  } else {
    this->isObjSingleton = false;
    subsets[*subKey] = boost::make_shared<Node>(*subKey, next, keyEnd, fullKey, obj, val);
  }

  key = myKey;
  originalVal = true;
}

HierarchicalAOM::Node::Node(const NodePtr& that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  init((Node*)that.get(), pedge, comp, analysis);
}

HierarchicalAOM::Node::Node(Node* that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  init(that, pedge, comp, analysis);
}

// This copy constructor variant is called inside the copy constructor. The "that" parameter
// does not have a const qualification because we need to modify the source Node object.
// This change is functionally transparent to users of this object.
void HierarchicalAOM::Node::init(Node* that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::Node::init()", scope::medium), 2, AOMDebugLevel)
  SIGHT_VERB(dbg << "pedge="<<pedge->str()<<endl, 2, AOMDebugLevel)
  SIGHT_VERB(dbg << "that="; that->print(dbg); dbg<<endl, 2, AOMDebugLevel)
  SIGHT_VERB(dbg << "that->key="<<(that->key? that->key->str(): "NULL")<<", that->fullKey="<<(that->fullKey? string(txt()<<that->fullKey): "NULL")<<endl, 2, AOMDebugLevel)

  // Copy over that node's obj and val
  isObjSingleton = that->isObjSingleton;
  key = that->key;
  fullKey = that->fullKey;
  val = that->val;

  // As a result of this copy, both instances of Node refer to the same instance of Lattice.
  // Thus, if either of them modifies this Lattice object the other will be (erroneously)
  // affected. To make sure that both objects are safe from each other, we set originalVal
  // to true in both, which will force them to make a copy of the Lattice object before
  // they modify it.
  that->originalVal = true;
  originalVal = true;

  // Copy subsets, recursively invoking this constructor on this node's children
  for(map<comparablePtr, NodePtr>::iterator sub=that->subsets.begin(); sub!=that->subsets.end(); sub++) {
    SIGHT_VERB(dbg << "sub="<<sub->second<<endl, 2, AOMDebugLevel)
    if(sub->second->fullKey) {
      SIGHT_VERB(dbg << "sub->second->fullKey="<<string(txt()<<sub->second->fullKey)<<endl, 2, AOMDebugLevel)
      SIGHT_VERB(dbg << "live="<<sub->second->fullKey->isLive(pedge, comp, analysis)<<endl, 2, AOMDebugLevel)
    }
    // Only create sub-keys for live keys
    if(!sub->second->fullKey || sub->second->fullKey->isLive(pedge, comp, analysis)) {
      NodePtr newNode = boost::make_shared<Node>(sub->second, pedge, comp, analysis);
      // If the copied sub-tree didn't end up being empty (due to the fullKeys being out of scope), add it
      if(!newNode->isEmptyVal(pedge, comp, analysis))
        subsets[sub->first] = newNode;
    }
  }
}

// Returns whether the set denoted by thet given object is a singleton
bool HierarchicalAOM::Node::isSingleton(AbstractObjectPtr obj)
{ return (obj->isConcrete() && obj->concreteSetSize()==1); }

// Set the value at this node to full.
// Return true if this causes the object to change and false otherwise.
bool HierarchicalAOM::Node::setValToFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  bool modified=false;

  // Set the value of this Node to full, creating a new Lattice object if needed
  if(originalVal) {
    val = val->copyA();
    originalVal = false;
  }

  modified = val->setToFull(pedge, comp, analysis) || modified;

  return modified;
}

// Set the value at this node AND all of its sub-trees to full.
// Return true if this causes the object to change and false otherwise.
bool HierarchicalAOM::Node::setSubTreeToFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  bool modified=false;

  modified = setValToFull(pedge, comp, analysis) || modified;

  // Cut off the children of this node to have the same effect of setting all
  // children's lattices to full
  modified = (subsets.size()>0) || modified;
  subsets.clear();

  return modified;
}

// Removes the given child node from the given parent node
void HierarchicalAOM::Node::remove(NodePtr child) {
  SIGHT_VERB(dbg << "child->key="<<child->key->str()<<endl, 2, AOMDebugLevel)
  ROSE_ASSERT(subsets.erase(child->key)==1);
}

// Union the values of this node and all of its children in to the given lattice.
// Return whether this causes lat to change.
bool HierarchicalAOM::Node::meetUpdate(AbstractionPtr lat, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  bool modified=false;
  if(val) modified = lat->meetUpdate(val, pedge, comp, analysis) || modified;

  for(map<comparablePtr, NodePtr>::iterator sub=subsets.begin(); sub!=subsets.end(); sub++)
    modified = sub->second->meetUpdate(lat, pedge, comp, analysis) || modified;

  return modified;
}

// Intersect the values of this node and all of its children in to the given lattice.
// Return whether this causes lat to change.
bool HierarchicalAOM::Node::intersectUpdate(AbstractionPtr lat, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  bool modified=false;
  if(val) modified = lat->intersectUpdate(val, pedge, comp, analysis) || modified;

  for(map<comparablePtr, NodePtr>::iterator sub=subsets.begin(); sub!=subsets.end(); sub++)
    modified = sub->second->intersectUpdate(lat, pedge, comp, analysis) || modified;

  return modified;
}

#ifndef DISABLE_SIGHT
SightStream& HierarchicalAOM::Node::print(SightStream& s) const {
  s << "Node: key="<<(key?key->str():"NULL")<<", fullKey="<<(fullKey? string(txt()<<fullKey): "NULL")<<", isObjSingleton="<<isObjSingleton<<", val="<<(val?val->str():"NULL")<<endl;
  s << "  subsets="<<endl;
  s << "<table>"<<endl;
  for(map<comparablePtr, NodePtr>::const_iterator sub=subsets.begin(); sub!=subsets.end(); sub++) {
    s << "<tr><td>";
    s << sub->first->str();
    s << "</td><td>";
    sub->second->print(s);
    s << "</td></tr>";
  }
  s << "</table>"<<endl;
  return s;
}
#endif

// Returns whether the fullKey at this node may be live at the given pedge
bool HierarchicalAOM::Node::isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  // It may be live if
  // - fullKey is NULL, so we can't be sure
  if(!fullKey) return true;
  // - fullKey is known to be live
  if(fullKey->isLive(pedge, comp, analysis)) return true;

  return false;
}

// Returns whether the set that this node maps its key to is empty.
// This is the case if all the values in its sub-tree denote the empty set
bool HierarchicalAOM::Node::isEmptyVal(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) const {
  //scope s("HierarchicalAOM::Node::isEmptyVal");
  //dbg << "val="<<(val? val->str(): "NULL")<<", val->isEmpty="<<(val?val->isEmpty(pedge, comp, analysis):false)<<", #subsets="<<subsets.size()<<endl;
  // val is empty
  if(val==NULL || val->isEmpty(pedge, comp, analysis)) {
    for(map<comparablePtr, NodePtr>::const_iterator sub=subsets.begin(); sub!=subsets.end(); sub++) {
      // If some sub-tree is not empty
      if(!sub->second->isEmptyVal(pedge, comp, analysis)) return false;
    }
    return true;
  // val is not empty
  } else
    return false;
}

std::ostream& operator<<(std::ostream& s, const HierarchicalAOM::NodePtr& node) {
  s << "<div style=\"border-style:solid; border-color:#888888; border-width=1px\">";
  s << "Node: "<</*key="<<(node->key?node->key->str():"NULL")<<", */" isObjSingleton="<<node->isObjSingleton<<endl;
  s << "    val(origVal="<<node->originalVal<<")="<<(node->val?node->val->str():"NULL");
  //s << "  <table border=1>";
  for(map<comparablePtr, HierarchicalAOM::NodePtr>::const_iterator sub=node->subsets.begin(); sub!=node->subsets.end(); sub++) {
    s << "  <table border=0>";
    s << "<tr><td>";
    s << sub->first->str();
    s << "</td><td>";
    s << sub->second;
    s << "</td></tr>";
    s << "</table>";
  }
  //s << "</table>"<<endl;
  s << "</div>";
  return s;
}

HierarchicalAOM::HierarchicalAOM(const HierarchicalAOM& that, AbstractObjectMap* parent) :
			   AbstractObjectMapKind(parent),
         tree(that.tree)/*,
			   isFinite(that.isFinite)*/
{ tree = boost::make_shared<Node>(that.tree, parent->latPEdge, parent->comp, parent->analysis); }

HierarchicalAOM::HierarchicalAOM(const HierarchicalAOM& that) :
			   AbstractObjectMapKind(that.parent),
			   tree(that.tree)/*,
			   isFinite(that.isFinite)*/
{ tree = boost::make_shared<Node>(that.tree, parent->latPEdge, parent->comp, parent->analysis); }

HierarchicalAOM::HierarchicalAOM(AbstractObjectMap* parent) : AbstractObjectMapKind(parent)//, isFinite(true)
{ tree = boost::make_shared<Node>(); }

HierarchicalAOM::~HierarchicalAOM() {
//cout << "HierarchicalAOM::~HierarchicalAOM()"<<endl;
}

// Add a new memory object --> lattice pair to the frontier.
// Return true if this causes the map to change and false otherwise.
// It is assumed that the given Lattice is now owned by the AbstractObjectMap and can be modified and deleted by it.
bool HierarchicalAOM::insert(AbstractObjectPtr obj_arg, AbstractionPtr val, bool originalVal) {
  AbstractionHierarchyPtr obj = boost::dynamic_pointer_cast<AbstractionHierarchy>(obj_arg);
  ROSE_ASSERT(obj);

  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::insert()", scope::medium), 1, AOMDebugLevel)
  SIGHT_VERB_IF(1, AOMDebugLevel)
#ifndef DISABLE_SIGHT
    indent ind;
    dbg << "obj(live="<<obj_arg->isLive(parent->latPEdge, parent->comp, parent->analysis)<<")="<<obj_arg->strp(parent->latPEdge, "")<<endl;
    dbg << "    key="<<obj->getHierKey()<<endl;
    dbg << "lattice(empty="<<val->isEmpty(parent->latPEdge, parent->comp, parent->analysis)<<")="<<val->str("    ")<<endl;
    indent ind2;
    dbg << str()<<endl;
#endif
  SIGHT_VERB_FI()

  // Do not insert mappings for dead keys
  if(!obj_arg->isLive(parent->latPEdge, parent->comp, parent->analysis)) {
    SIGHT_VERB(dbg << "<b>HierarchicalAOM::insert() WARNING: attempt to insert dead mapping "<<obj_arg->strp(parent->latPEdge)<<" =&gt; "<<val->str()<<"<\b>"<<endl, 1, AOMDebugLevel)
    return false;
  }

  //isFinite = isFinite && val->finiteLattice();

  // Don't add empty values since that wouldn't have any information content
  if(val->isEmpty(parent->latPEdge, parent->comp, parent->analysis)) return false;

  bool modified = insert(tree, obj->getHierKey()->begin(), obj->getHierKey()->end(), obj->getHierKey(), obj_arg, val, originalVal);
  SIGHT_VERB(dbg << "modified="<<modified<<endl, 1, AOMDebugLevel);
  return modified;
}

// Recursive body of insert
bool HierarchicalAOM::insert(NodePtr subTree,
             std::list<comparablePtr>::const_iterator subKey, std::list<comparablePtr>::const_iterator keyEnd,
             AbstractionHierarchy::hierKeyPtr fullKey, AbstractObjectPtr obj, AbstractionPtr val, bool originalVal)
{
  bool modified = false;
  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::insert()", scope::medium), 1, AOMDebugLevel)
  SIGHT_VERB(dbg << "subKey="<<(subKey!=keyEnd? (*subKey)->str(): "END")<<", end of key="<<(subKey==keyEnd)<<", tree root="<<(subTree==tree)<<endl, 1, AOMDebugLevel)
  SIGHT_VERB(dbg << "subTree="<<subTree<<endl,  1, AOMDebugLevel)
  // If this is the last sub-key in the key
  if(subKey==keyEnd) {
    // If this is the root node of the tree, the key must have been empty/the object
    // must have been the full set. In this case we place val into the Node without
    // placing obj.
    if(subTree == tree) {
      if(subTree->fullKey != fullKey) {
        subTree->fullKey = fullKey;
        modified = true;
      }

      if(!subTree->val->equalSet(val, parent->latPEdge, parent->comp, parent->analysis)) {
        subTree->val = val;
        subTree->originalVal = originalVal;
        modified = true;
      }

      SIGHT_VERB(dbg << "root: modified="<<modified<<endl, 1, AOMDebugLevel)
      return modified;
      // Note: we should return true only if the set was modified but lattices have no equalSet method
    } else {
      // If obj is equal to the current obj in the node in all possible executions, replace the original
      // value with the new one
      ROSE_ASSERT(subTree->isObjSingleton == Node::isSingleton(obj));
      if(subTree->isObjSingleton) {
        ROSE_ASSERT(subTree->fullKey);
        if(!subTree->val->equalSet(val, parent->latPEdge, parent->comp, parent->analysis)) {
          subTree->val = val;
          subTree->originalVal = originalVal;
          modified = true;
        }
        // If an object is must-equal, it must be a singleton set and thus, cannot have any subsets
        ROSE_ASSERT(subTree->subsets.size()==0);

        SIGHT_VERB(dbg << "singleton: modified="<<modified<<endl, 1, AOMDebugLevel)
        return modified;
      } else {
        // The old and new objects denote the same set but in different executions the actual concrete
        // objects they denote may be different, so union the original and new lattices
        //ROSE_ASSERT(subTree->fullKey);

        // If lattice associated with this sub-tree is NULL (denotes an empty set)
        if(!subTree->val) {
          // Copy val into subTree->val if val is not the empty set, in which case don't bother
          if(!val->isEmpty(parent->latPEdge, parent->comp, parent->analysis)) {
            subTree->val = val;
            subTree->originalVal = true;
            modified = true;
          }
        // If the lattice is not NULL (may denote any set, including empty but is at least a valid object)
        } else {
          // Create a fresh copy of subTree->val if we have not yet done so
          if(subTree->originalVal) {
            subTree->val = subTree->val->copyA();
            subTree->originalVal = false;
          }
          modified = subTree->val->meetUpdate(val, parent->latPEdge, parent->comp, parent->analysis);
        }
        SIGHT_VERB(dbg << "overlap: modified="<<modified<<endl, 1, AOMDebugLevel)
        return modified;
      }
    }
  // If this is not the last sub-key, place it deeper in the tree
  } else {
    // If the current sub-key exists in the current tree node, recurse deeper
    map<comparablePtr, NodePtr>::iterator skIt = subTree->subsets.find(*subKey);
    std::list<comparablePtr>::const_iterator next = subKey; ++next;
    if(skIt != subTree->subsets.end()) {
      modified = insert(skIt->second, next, keyEnd, fullKey, obj, val, originalVal);
      SIGHT_VERB(dbg << "recursive: modified="<<modified<<endl, 1, AOMDebugLevel)
      return modified;
    // If it does not exist in the current tree node, add a sub-tree for it, which
    // will contain nodes for each sub-key between subKey and keyEnd
    } else {
      subTree->subsets[*subKey] = boost::make_shared<Node>(*subKey, next, keyEnd, fullKey, obj, val);
      SIGHT_VERB(dbg << "new key: modified=1"<<endl, 1, AOMDebugLevel)
      return true;
    }
  }
}

// Removes the key matching the argument from the frontier.
// Return true if this causes the map to change and false otherwise.
bool HierarchicalAOM::remove(AbstractObjectPtr obj_arg) {
  AbstractionHierarchyPtr obj = boost::dynamic_pointer_cast<AbstractionHierarchy>(obj_arg);
  ROSE_ASSERT(obj);
  pair<pair<NodePtr, NodePtr>, bool> f = find(obj);
  NodePtr foundN = f.first.first;
  NodePtr parentN = f.first.second;
  // For remove operation, we use must equal policy
  if(foundN->isObjSingleton) {
    parentN->remove(foundN);
    return true;
  }
  return false;
}

// Returns the lattice object at the given sub-tree or an empty lattice if there isn't one
AbstractionPtr HierarchicalAOM::getVal(NodePtr subTree) {
  // If val is NULL, create an empty one
  if(!subTree->val) {
    // Create a fresh empty set at this sub-tree, recording that this set is
    // not an original object from the user
    ROSE_ASSERT(subTree->originalVal);
    subTree->val = parent->defaultLat->copyA();
    subTree->val->setToEmpty(parent->latPEdge, parent->comp, parent->analysis);
    subTree->originalVal = false;
  }
  return subTree->val;
}

// Get all x-frontier for a given abstract memory object
AbstractionPtr HierarchicalAOM::get(AbstractObjectPtr obj_arg) {
  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::get()", scope::medium), 1, AOMDebugLevel)
  SIGHT_VERB(dbg << "obj="<<obj_arg->str()<<endl, 1, AOMDebugLevel)
  SIGHT_VERB(dbg << "tree="<<tree<<endl, 1, AOMDebugLevel)
  AbstractionHierarchyPtr obj = boost::dynamic_pointer_cast<AbstractionHierarchy>(obj_arg);
  ROSE_ASSERT(obj);
  SIGHT_VERB(dbg << "key="<<obj->getHierKey()<<endl, 1, AOMDebugLevel)

  //cout << "key="<<obj->getHierKey()<<endl;

  list<comparablePtr>::const_iterator curKey = obj->getHierKey()->begin(),
                                      keyEnd = obj->getHierKey()->end();


  // If the key is empty, return the lattice object at the root node
  if(curKey==keyEnd) return getVal(tree);

  // If the key is non-empty, iterate into the tree, following the key's sub-keys
  // and unioning any lattice objects found along the way.
  NodePtr subTree = tree;

  // The lattice object we'll return
  //AbstractionPtr resLat(parent->defaultLat->copy());
  AbstractionPtr resLat = parent->defaultLat->copyA();
  resLat->setToEmpty(parent->latPEdge, parent->comp, parent->analysis);

  do {
    SIGHT_VERB(dbg << "curKey="<<curKey->str()<<", found="<<(subTree->subsets.find(*curKey)!=subTree->subsets.end())<<endl, 1, AOMDebugLevel)
    SIGHT_VERB(dbg << "subTree="<<subTree<<endl, 2, AOMDebugLevel)

    // Union the current node's lattice object into resLat
    resLat->meetUpdate(getVal(subTree), parent->latPEdge, parent->comp, parent->analysis);
    SIGHT_VERB(dbg << "resLat="<<resLat->str()<<endl, 2, AOMDebugLevel)

    map<comparablePtr, NodePtr>::iterator sub = subTree->subsets.find(*curKey);
    // If the current sub-key exists in subsets
    if(sub!=subTree->subsets.end()) {
      // Advance to its sub-tree
      subTree = sub->second;
    // If we've reached the end of the tree
    } else {
      // Return the lattice object we've already computed
      return resLat;
    }

    ++curKey;
  } while(curKey != keyEnd);

  // We've now reached the end of the key but there may be more information deeper
  // in the tree about subsets of obj. Union their values into resLat.
  subTree->meetUpdate(resLat, parent->latPEdge, parent->comp, parent->analysis);
  SIGHT_VERB(dbg << "after sub-tree meet resLat="<<resLat->str()<<endl, 2, AOMDebugLevel)

  return resLat;
}

// Return < <n:NodePtr, p:NodePtr>, f:bool>, where:
// n - Node that corresponds to the key of the given object,
// p - The parent of n or NULLPart,
// f - boolean that indicates whether the node corresponds to the full key (true) or a prefix
//     of the key (false).
std::pair<std::pair<HierarchicalAOM::NodePtr, HierarchicalAOM::NodePtr>, bool>
                            HierarchicalAOM::find(AbstractionHierarchyPtr obj) {
  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::find()", scope::medium), 2, AOMDebugLevel)
  list<comparablePtr>::const_iterator curKey = obj->getHierKey()->begin(),
                                      keyEnd = obj->getHierKey()->end();

  // If the key is empty, return the lattice object at the root node
  if(curKey==keyEnd) return make_pair(make_pair(tree, NULLNode), true);

  // If the key is non-empty, iterate into the tree, following the key's sub-keys
  // and unioning any lattice objects found along the way.
  NodePtr parent = NULLNode;
  NodePtr subTree = tree;

  do {
    SIGHT_VERB(dbg << "curKey="<<curKey->str()<<endl, 2, AOMDebugLevel)
    SIGHT_VERB(dbg << "subTree="<<subTree<<endl, 2, AOMDebugLevel)
    SIGHT_VERB(dbg << "found="<<(subTree->subsets.find(*curKey)!=subTree->subsets.end())<<endl, 2, AOMDebugLevel)

    map<comparablePtr, NodePtr>::iterator sub = subTree->subsets.find(*curKey);
    // If the current sub-key exists in subsets
    if(sub!=subTree->subsets.end()) {
      // Advance to its sub-tree
      parent = subTree;
      subTree = sub->second;
    // If we've reached the end of the tree
    } else {
      return make_pair(make_pair(subTree, parent), false);
    }

    ++curKey;
  } while(curKey != keyEnd);

  return make_pair(make_pair(subTree, parent), true);
}

// Set all the information associated Lattice object with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
// This function does nothing because it is a set of abstract objects rather than a map from some abstract objects
// to others.
bool HierarchicalAOM::setMLValueToFull(MemLocObjectPtr obj_arg) {
  bool modified=false;
  AbstractionHierarchyPtr obj = boost::dynamic_pointer_cast<AbstractionHierarchy>(obj_arg);
  ROSE_ASSERT(obj);

  // Search for the node that contains the closest key to ml's in the tree,
  // setting their lattices to NULL
  list<comparablePtr>::const_iterator curKey = obj->getHierKey()->begin(),
                                      keyEnd = obj->getHierKey()->end();

  // If the key is non-empty, iterate into the tree, following the key's sub-keys
  // and unioning any lattice objects found along the way.
  NodePtr subTree = tree;

  while(curKey != keyEnd) {
    // Set the current
    modified = subTree->setValToFull(parent->latPEdge, parent->comp, parent->analysis) || modified;

    map<comparablePtr, NodePtr>::iterator sub = subTree->subsets.find(*curKey);
    // If the current sub-key exists in subsets
    if(sub!=subTree->subsets.end()) {
      // Advance to its sub-tree
      subTree = sub->second;
    // If we've reached the end of the tree
    } else {
      return modified;
    }

    ++curKey;
  }

  // If there is a tree node that matches ml's full key, make the whole
  // subtree full
  modified = subTree->setSubTreeToFull(parent->latPEdge, parent->comp, parent->analysis) || modified;

  return modified;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool HierarchicalAOM::isFull() {
  if(tree->val && tree->val->isFull(parent->latPEdge, parent->comp, parent->analysis)) {
    // If the root node has a full lattice, then we clean out its sub-trees since
    // they're now storing redundant info
    tree->setSubTreeToFull(parent->latPEdge, parent->comp, parent->analysis);
    return false;
  }
  return true;
}
// Returns whether this lattice denotes the empty set.
bool HierarchicalAOM::isEmpty() {
  // Return true if the tree is empty
  return !tree->val && tree->subsets.size()==0;
}

std::string HierarchicalAOM::str(std::string indent) const {
  ostringstream s;
  //s << "pedge = "<<(parent->latPEdge? parent->latPEdge->str(): "NULL")<<endl;
  s << "<u>HierarchicalAOM</u>"<<endl;
  s << tree;
  return s.str();
}

// Variant of the str method that can produce information specific to the current Part.
// Useful since AbstractObjects can change from one Part to another.
std::string HierarchicalAOM::strp(PartEdgePtr pedge, std::string indent) const {
  return str(indent);
}

// initializes this Lattice to its default state, if it is not already initialized
void HierarchicalAOM::initialize() {}

// returns a copy of this lattice
AbstractObjectMapKindPtr HierarchicalAOM::copy() const
{ return boost::make_shared<HierarchicalAOM>(*this); }

// overwrites the state of this Lattice with that of that Lattice
void HierarchicalAOM::copy(AbstractObjectMapKindPtr thatL) {
  HierarchicalAOMPtr that = boost::dynamic_pointer_cast <HierarchicalAOM> (thatL);
  ROSE_ASSERT(that);
  tree = boost::make_shared<Node>(that->tree, parent->latPEdge, parent->comp, parent->analysis);
  //isFinite = that->isFinite;
}

// Called by analyses to transfer this lattice's contents from across function scopes from a caller function
//    to a callee's scope and vice versa. If this lattice maintains any information on the basis of
//    individual MemLocObjects these mappings must be converted, with MemLocObjects that are keys of the ml2ml
//    replaced with their corresponding values. If a given key of ml2ml does not appear in the lattice, it must
//    be added to the lattice and assigned a default initial value. In many cases (e.g. over-approximate sets
//    of MemLocObjects) this may not require any actual insertions. If the value of a given ml2ml mapping is
//    NULL (empty boost::shared_ptr), any information for MemLocObjects that must-equal to the key should be
//    deleted.
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    the keys in ml2ml are in scope on one side of Part, while the values on the other side. Specifically, it is
//    guaranteed that the keys are in scope at fromPEdge while the values are in scope at the edge returned
//    by getPartEdge().
// remapML must return a freshly-allocated object.
AbstractObjectMapKindPtr HierarchicalAOM::remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge) {
  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::remapML()", scope::high), 1, AOMDebugLevel)
  HierarchicalAOMPtr newAOM = boost::dynamic_pointer_cast<HierarchicalAOM>(copy());
  for(set<MLMapping>::const_iterator m=ml2ml.begin(); m!=ml2ml.end(); m++) {
    AbstractionHierarchyPtr fromHier = boost::dynamic_pointer_cast<AbstractionHierarchy>(m->from);
    ROSE_ASSERT(fromHier);
    SIGHT_VERB(dbg << "from="<<m->from->str()<<endl, 1, AOMDebugLevel)
    SIGHT_VERB(dbg << "from key="<<fromHier->getHierKey()<<endl, 1, AOMDebugLevel)
    SIGHT_VERB(dbg << "to="<<(m->to?m->to->str():"NULL")<<endl, 1, AOMDebugLevel)

    pair<pair<NodePtr, NodePtr>, bool> f = newAOM->find(fromHier);
    NodePtr foundN = f.first.first;
    NodePtr parentN = f.first.second;
    bool fullKeyFound = f.second;
    if(parentN) { SIGHT_VERB(dbg << "parentN="<<parentN<<endl, 1, AOMDebugLevel) }
    if(foundN)  { SIGHT_VERB(dbg << "foundN="<<foundN<<endl, 1, AOMDebugLevel) }
    SIGHT_VERB(dbg << "fullKeyFound="<<fullKeyFound<<endl, 1, AOMDebugLevel)

    // If m->from mustEqual f->first, remove that node and insert under m->to
    if(fullKeyFound && foundN->isObjSingleton) {
      ROSE_ASSERT(foundN->isObjSingleton == Node::isSingleton(m->from));
      // If we need to replace from with to
      if(m->replaceMapping) {
        parentN->remove(foundN);

        // If the value of the current ml2ml mapping is not NULL (i.e. the key is a MemLoc
        // with a lifetime that is not limited to a given function and it carries over
        // across function boundaries)
        if(m->to)
          newAOM->insert(m->to, foundN->val);

      // If we add to but keep from
      } else {
        // If the value of the current ml2ml mapping is not NULL (i.e. the key is a MemLoc
        // with a lifetime that is not limited to a given function and it carries over
        // across function boundaries)
        if(m->to)
          newAOM->insert(m->to, foundN->val->copyA(), /*originalVal*/ false);
      }
    // If foundN equalsSet m->from or contains it, insert the mapping m->to => val but leave the prior one behind
    } else {
      if(m->to) {
        // Get the lattice object that describes the information just associated with
        // m->from. We focus in this way because f may be a strict superset of m->from
        // and some of the lattice objects in its sub-tree may correspond to objects
        // that are disjoint from m->from.
        AbstractionPtr fromLat = get(m->from);
        // Insert this lattice under m->to, informing the HierarchicalAOM that it must treat
        // fromLat as being owned by user code (must be copied before it is modified) since
        // it appears at multiple locations within the map.
        newAOM->insert(m->to, fromLat, /*originalVal*/ true);
      }
    }
  }
  return newAOM;
}

// Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously
//    maintained in this lattice about them.
// Returns true if the Lattice state is modified and false otherwise.
bool HierarchicalAOM::replaceML(AbstractObjectMapKindPtr that_arg) {
  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::replaceML()", scope::high), 1, AOMDebugLevel)
  HierarchicalAOMPtr that = boost::dynamic_pointer_cast <HierarchicalAOM> (that_arg);
  bool modified = false;
  //modified = (isFinite != (isFinite || that->isFinite)) || modified;
  //isFinite = isFinite || that->isFinite;
  SIGHT_VERB(dbg<<"this="<<str()<<endl, 1, AOMDebugLevel)
  SIGHT_VERB(dbg<<"that="<<that_arg->str()<<endl, 1, AOMDebugLevel)

  return replaceML(tree, that->tree) || modified;
}

// Recursive body of replaceML
bool HierarchicalAOM::replaceML(NodePtr thisST, NodePtr thatST) {
  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::replaceML()", scope::medium), 1, AOMDebugLevel)
  bool modified = false;

  ROSE_ASSERT(thisST->isObjSingleton == thatST->isObjSingleton);

  SIGHT_VERB(dbg<<"thisST->val="<<(thisST->val? thisST->val->str(): "NULL")<<endl, 1, AOMDebugLevel)
  SIGHT_VERB(dbg<<"thatST->val="<<(thatST->val? thatST->val->str(): "NULL")<<endl, 1, AOMDebugLevel)

  // We currently don't check if val was modified since equalSet doesn't exist for Lattices
  modified = true;
  thisST->val = thatST->val;
  thisST->originalVal = true;

  // Iterate over the subsets of thatST, copying from their sub-trees into this
  for(map<comparablePtr, NodePtr>::iterator thatSub=thatST->subsets.begin(); thatSub!=thatST->subsets.end(); thatSub++) {
    SIGHT_VERB(dbg<<"thatSub key="<<thatSub->first->str()<<", found="<<(thisST->subsets.find(thatSub->first)!=thisST->subsets.end())<<endl, 1, AOMDebugLevel)

    // If the current sub-tree of that exists in this
    map<comparablePtr, NodePtr>::iterator thisSub=thisST->subsets.find(thatSub->first);
    if(thisSub!=thisST->subsets.end()) {
      // replace over from that sub-tree to this one
      modified = replaceML(thisSub->second, thatSub->second) || modified;
    // If it doesn't, copy the sub-tree from that into this
    } else {
      //thisST->subsets[thatSub->first] = boost::make_shared<Node>(thatSub->second);
      // Only copy if that sub-tree is live at this AOM's pedge
      if(thatSub->second->isLive(parent->latPEdge, parent->comp, parent->analysis)) {
        NodePtr newNode = boost::make_shared<Node>(thatSub->second, parent->latPEdge, parent->comp, parent->analysis);
        // If the copied sub-tree didn't end up being empty (due to the keys being out of scope), add it
        if(!newNode->isEmptyVal(parent->latPEdge, parent->comp, parent->analysis)) {
          thisST->subsets[thatSub->first] = newNode;
          modified = true;
        }
      }
    }
  }

  return modified;
}

// Computes the union or intersection of this and that, as specified in the ui parameter
// and saves the result in this.
// Returns true if this causes this to change and false otherwise
bool HierarchicalAOM::unionIntersectUpdate(AbstractObjectMapKindPtr that_arg, uiType ui) {
  SIGHT_VERB_DECL(scope, ("HierarchicalAOM::unionIntersectUpdate()", scope::high), 1, AOMDebugLevel)
  HierarchicalAOMPtr that = boost::dynamic_pointer_cast <HierarchicalAOM> (that_arg);

  bool modified = false;
  /*modified = (isFinite != (isFinite || that->isFinite)) || modified;
  isFinite = isFinite || that->isFinite;*/

  return unionIntersectUpdate(tree, that->tree, ui) || modified;
}

// Recursive body of unionIntersectUpdate
bool HierarchicalAOM::unionIntersectUpdate(NodePtr thisST, NodePtr thatST, uiType ui) {
  SIGHT_VERB_DECL(scope, (txt()<<"HierarchicalAOM::unionIntersectUpdate()"), 1, AOMDebugLevel)
  bool modified = false;

  ROSE_ASSERT(thisST->isObjSingleton == thatST->isObjSingleton);

  // Meet the values at this and that node
  SIGHT_VERB(dbg<<"thisST->val="<<(thisST->val? thisST->val->str(): "NULL")<<endl, 1, AOMDebugLevel)
  SIGHT_VERB(dbg<<"thatST->val="<<(thatST->val? thatST->val->str(): "NULL")<<endl, 1, AOMDebugLevel)
  // If they're both non-NULL, call unionIntersectUpdate
  if(thisST->val && thatST->val) {
    // If val is an original copy from the user, copy it before modifying it
    if(thisST->originalVal) {
      thisST->val = thisST->val->copyA();
      thisST->originalVal = false;
    }
    if(ui==Union)
      modified = thisST->val->meetUpdate(thatST->val, parent->latPEdge, parent->comp, parent->analysis) || modified;
    else
      modified = thisST->val->intersectUpdate(thatST->val, parent->latPEdge, parent->comp, parent->analysis) || modified;

  // If this is NULL, and that is not
  } else if(!thisST->val && thatST->val) {
    // If Union, copy from that to this
    if(ui == Union) {
      thisST->val = thatST->val;
      thisST->originalVal = true;
      modified = !thisST->val->isEmpty(parent->latPEdge, parent->comp, parent->analysis) || modified;
    }
    // If Intersection, don't modify since the intersection contains only the common elements of the maps

  // If this is not NULL, and that is
  } else if(thisST->val && !thatST->val) {
    // If Intersection, remove this sub-tree's value since the intersection contains only the common elements of the maps
    if(ui == Intersection) {
      modified = !thisST->val->isEmpty(parent->latPEdge, parent->comp, parent->analysis) || modified;
      thisST->val.reset();
      thisST->originalVal = false;
    }
    // If Union, don't modify it since the union contains the elements that exist in either mep
  }

  SIGHT_VERB(dbg<<"merged thisST->val="<<(thisST->val? thisST->val->str(): "NULL")<<endl, 1, AOMDebugLevel)
  SIGHT_VERB_DECL(indent, (), 1, AOMDebugLevel)
  // Otherwise, this doesn't change

  // Iterate over the subsets of thatST, unioning their sub-trees into this
  for(map<comparablePtr, NodePtr>::iterator thatSub=thatST->subsets.begin(); thatSub!=thatST->subsets.end(); thatSub++) {
    SIGHT_VERB_DECL(scope, (txt()<<"thatSub key="<<thatSub->first->str()<<", found="<<(thisST->subsets.find(thatSub->first)!=thisST->subsets.end())<<", live="<<thatSub->second->isLive(parent->latPEdge, parent->comp, parent->analysis)), 1, AOMDebugLevel)
    SIGHT_VERB(dbg << "thatSub Node="<<thatSub->second<<endl, 1, AOMDebugLevel)

    // If the current sub-tree of that exists in this
    map<comparablePtr, NodePtr>::iterator thisSub=thisST->subsets.find(thatSub->first);
    SIGHT_VERB(dbg << "thatSub->first="<<thatSub->first->str()<<" found="<<(thisSub!=thisST->subsets.end())<<endl, 1, AOMDebugLevel)
    if(thisSub!=thisST->subsets.end()) {
      // Meet both sub-trees
      modified = unionIntersectUpdate(thisSub->second, thatSub->second, ui) || modified;
      SIGHT_VERB(dbg<<"merged thisSub="<<thisSub->second<<endl, 1, AOMDebugLevel)
    // If it doesn't, copy the sub-tree from that into this
    } else {
      //thisST->subsets[thatSub->first] = boost::make_shared<Node>(thatSub->second);
      if(thatSub->second->isLive(parent->latPEdge, parent->comp, parent->analysis)) {
        NodePtr newNode = boost::make_shared<Node>(thatSub->second, parent->latPEdge, parent->comp, parent->analysis);
        SIGHT_VERB(dbg << "emptyVal="<<newNode->isEmptyVal(parent->latPEdge, parent->comp, parent->analysis)<<", in newNode="<<newNode<<endl, 1, AOMDebugLevel)
        SIGHT_VERB(dbg << "thatSub Node="<<thatSub->second<<endl, 1, AOMDebugLevel)

        // If the copied sub-tree didn't end up being empty (due to the keys being out of scope), add it
        if(!newNode->isEmptyVal(parent->latPEdge, parent->comp, parent->analysis)) {
          thisST->subsets[thatSub->first] = newNode;
          SIGHT_VERB_IF(1, AOMDebugLevel)
#ifndef DISABLE_SIGHT
            dbg << "original node "<<thatSub->second<<endl;
            dbg << "new node "<<newNode<<endl;
#endif
          SIGHT_VERB_FI()
          modified = true;
        }
      }
    }
  }
  SIGHT_VERB(dbg<<"modified = "<<modified<<endl, 1, AOMDebugLevel)

  return modified;
}

// Computes the meet of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool HierarchicalAOM::meetUpdate(AbstractObjectMapKindPtr that_arg) {
  return unionIntersectUpdate(that_arg, Union);
}

// Computes the intersection of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool HierarchicalAOM::intersectUpdate(AbstractObjectMapKindPtr that_arg) {
  return unionIntersectUpdate(that_arg, Intersection);
}

/*bool HierarchicalAOM::finiteLattice()
{
  //return isFinite;
  // AbstractObjectMaps can grow to an arbitrary size and are thus inherently not finite
  return false;
}*/

bool HierarchicalAOM::operator==(AbstractObjectMapKindPtr that_arg) {
  HierarchicalAOMPtr that = boost::dynamic_pointer_cast<HierarchicalAOM>(that_arg);
  ROSE_ASSERT(that);
  ROSE_ASSERT(parent->latPEdge == that->parent->getPartEdge());
  // This will be written once we have the merging algorithm to test
  // these maps' frontiers for semantic equivalence
  return false;
}

/*************************
 ***** MappedAOMKind *****
 *************************/

MappedAOMKind::MappedAOMKind(AbstractObjectPtr exampleKey, AbstractObjectMap* parent/*AbstractObjectMapKindFactoryPtr factory, */):
  AbstractObjectMapKind(parent)/*,
  factory(factory)*/
{
  assert(exampleKey->isMappedAO());

  ui = boost::dynamic_pointer_cast<MappedAbstractionBase>(exampleKey)->getUI();
/*
  // Generate a fresh MAOMap that maps the sub-keys of exampleKey to AbstractObjectMaps
  assert(boost::dynamic_pointer_cast<MappedAbstractionBase>(exampleKey));
  mappedAOMap = boost::dynamic_pointer_cast<MappedAbstractionBase>(exampleKey)->genMappedAOMap();
  assert(mappedAOMap);

  class AOMKindCreator: public MAOMap::setMapFunc {
    public:
    AbstractObjectMapKindFactoryPtr factory;
    AOMKindCreator(AbstractObjectMapKindFactoryPtr factory): factory(factory) {}
    // Applied to each Key in a given MappedAbstractObject
    // obj: the AO mapped to the key
    // valMapped: indicates whether there is a value currently mapped to the key in the AOMap
    // curVal: if valMapped is true, curVal contains the current value mapped in AOMap
    // Returns the new value to be mapped to the current key in AOMap
    boost::shared_ptr<void> operator()(AbstractionPtr obj, boost::shared_ptr<void> curVal, bool valMapped) {
      assert(valMapped);

      return factory->create();
    }
  };
  AOMKindCreator x(factory);
  mappedAOMap->set(exampleKey, x);*/

  // Create an empty MAOMap with the same sub-keys as key
  assert(boost::dynamic_pointer_cast<MappedAbstractionBase>(exampleKey));
  mappedAOMap = boost::dynamic_pointer_cast<MappedAbstractionBase>(exampleKey)->genMappedAOMap();
  assert(mappedAOMap);

  // Iterate over the Abstractions in mappedAOMap and make sure they have the same properties
  class AOMKindOp: public MAOMap::setMapFunc {
    AbstractObjectMap* parent;
    public:
    AOMKindOp(AbstractObjectMap* parent): parent(parent) {}
    boost::shared_ptr<void> operator()(AbstractionPtr subA, boost::shared_ptr<void> curVal, bool valMapped) {
      assert(valMapped);
      AbstractObjectPtr subAO = boost::dynamic_pointer_cast<AbstractObject>(subA);
      assert(subAO);
      return AbstractObjectMap::createAOMKind(subAO, parent);
    }
  };
  AOMKindOp x(parent);
  mappedAOMap->set(exampleKey, x);
}
/*
MappedAOMKind::MappedAOMKind(boost::shared_ptr<MAOMap> mappedAOMap, AbstractObjectMap* parent) {
  class setApplyMapFunc {
    public:
    // Applied to each Key in a map. The value returned for each key is assigned to that key.
    // curVal: contains the current value mapped in AOMap
    virtual boost::shared_ptr<void> operator()(boost::shared_ptr<void> curVal)=0;
  };

  // Applies the given functor to all the keys in this map and sets the value returned by each invocation
  // of the function to the key that corresponds to the call
  virtual void setApply(setApplyMapFunc& f)=0;
}*/

MappedAOMKind::MappedAOMKind(const MappedAOMKind& that, bool emptyMap) :
    AbstractObjectMapKind(that.parent), ui(that.ui)
{
  //scope s(txt()<<"MappedAOMKind::MappedAOMKind(emptyMap="<<emptyMap<<")");
  // Copy mappedAOMap's keys but not their values (sub-AOMs)
  mappedAOMap = that.mappedAOMap->create();

  if(!emptyMap) {
    // Initialize mappedAOMap's keys with the copies of the values (sub-AOMaps) in that.mappedAOMap
    class AOMKindOp: public MAOMap::setJoinMapFunc {
      public:
      boost::shared_ptr<void> operator()(boost::shared_ptr<void> curVal1, boost::shared_ptr<void> curVal2) {
        AbstractObjectMapKindPtr thatAOMKind = boost::static_pointer_cast<AbstractObjectMapKind>(curVal2);
        assert(thatAOMKind);
        //dbg << "thatAOMKind="<<thatAOMKind->str()<<endl;
        //AbstractObjectMapKindPtr kindCopy = thatAOMKind->copy();
        //dbg << "kindCopy="<<kindCopy->str()<<endl;
        return thatAOMKind->copy();
      }
    };
    AOMKindOp x;
    mappedAOMap->setJoin(that.mappedAOMap, x);
  }

  // Update the parent pointers of all the sub-maps
  setParent(that.parent);
}

// Variant of setParent that also calls the setParent methods of all the sub-maps
void MappedAOMKind::setParent(AbstractObjectMap* newParent) {
  AbstractObjectMapKind::setParent(newParent);

  // Sets the parent of all the sub-maps to newParent
  class AOMKindOp: public MAOMap::applyMapFunc {
    AbstractObjectMap* newParent;
    public:
    AOMKindOp(AbstractObjectMap* newParent): newParent(newParent) {}
    void operator()(boost::shared_ptr<void> curVal) {
      if(curVal) ((AbstractObjectMapKind*)curVal.get())->setParent(newParent);
    }
  };
  AOMKindOp x(newParent);
  mappedAOMap->apply(x);
}

// Add a new memory object --> lattice pair to the frontier.
// Return true if this causes the map to change and false otherwise.
// It is assumed that the given Lattice is now owned by the AbstractObjectMap and can be modified and deleted by it.
bool MappedAOMKind::insert(AbstractObjectPtr key, AbstractionPtr val) {
  assert(key->isMappedAO());
  class AOMKindOp: public MAOMap::getMapFunc {
    public:
    bool modified;
    AbstractionPtr val;
    AOMKindOp(AbstractionPtr val): modified(false), val(val) {}
    void operator()(AbstractionPtr obj, boost::shared_ptr<void> curVal, bool valMapped) {
      assert(valMapped);
      modified = ((AbstractObjectMapKind*)curVal.get())->insert(
                                boost::dynamic_pointer_cast<AbstractObject>(obj), val) || modified;
    }
  };
  AOMKindOp x(val);
  mappedAOMap->get(key, x);
  return x.modified;
}

// Removes the key matching the argument from the frontier.
// Return true if this causes the map to change and false otherwise.
bool MappedAOMKind::remove(AbstractObjectPtr key) {
  assert(key->isMappedAO());
  class AOMKindOp: public MAOMap::getMapFunc {
    public:
    bool modified;
    AOMKindOp(): modified(false) {}
    void operator()(AbstractionPtr obj, boost::shared_ptr<void> curVal, bool valMapped) {
      assert(valMapped);
      modified = ((AbstractObjectMapKind*)curVal.get())->remove(
                         boost::dynamic_pointer_cast<AbstractObject>(obj)) || modified;
    }
  };
  AOMKindOp x;
  mappedAOMap->get(key, x);
  return x.modified;
}

// Get all x-frontier for a given abstract memory object
AbstractionPtr MappedAOMKind::get(AbstractObjectPtr key) {
  SIGHT_VERB_DECL(scope, (txt()<<"MappedAOMKind::get() ui="<<(ui==Union? "Union": "Intersection"), scope::medium), 1, AOMDebugLevel)
  SIGHT_VERB(dbg << "key="<<key->str()<<endl, 1, AOMDebugLevel)
  assert(key->isMappedAO());

  //struct timeval getStart, getEnd; gettimeofday(&getStart, NULL);

  // Create a fresh map to hold the mapping of the sub-keys inside the MappedAbstractObject key
  // to the results of get() for each sub-key.
  boost::shared_ptr<MAOMap> valMAOMap =
      boost::dynamic_pointer_cast<MappedAbstractionBase>(key)->genMappedAOMap();

  // Iterate over the keys of key and this map (should be identical) and for each
  // sub-map apply get on the corresponding sub-key. The returned AbstractionPtr
  // is then mapped to the corresponding key in valMAOMap.
  class AOMKindOp: public MAOMap::setMapObjMapJoinMapFunc {
    public:
    // A representative value returned from any of the sub-maps. We'll use this value's
    // implementations of getUnion/getIntersection
    AbstractionPtr representativeVal;

    // Applied to all the keys that appear in the three maps. The value returned by the function
    // assigned to the current key in the first map
    // curVal1: contains the current value mapped in the this MAOMap
    // curVal2: contains the current value mapped in the Mapped Abstract Object
    // curVal2: contains the current value mapped in the that MAOMap
    virtual boost::shared_ptr<void> operator()(boost::shared_ptr<void> curValThis,
                                               AbstractObjectPtr curValObj,
                                               boost::shared_ptr<void> curValThat) {
      representativeVal = ((AbstractObjectMapKind*)curValThat.get())->get(curValObj);
      SIGHT_VERB(dbg << "val="<<representativeVal->str()<<endl, 1, AOMDebugLevel)
      return representativeVal;
    }
  };
  AOMKindOp x;
  valMAOMap->setMapObjMapJoin(key, mappedAOMap, x);

  //gettimeofday(&getEnd, NULL); cout << "MappedAOMKind::get() get\t"<<(((getEnd.tv_sec*1000000 + getEnd.tv_usec) - (getStart.tv_sec*1000000 + getStart.tv_usec)) / 1000000.0)<<endl;

  //struct timeval aggrStart, aggrEnd; gettimeofday(&aggrStart, NULL);
  // Now that valMAOMap is populated with a mapping from keys to the results of get()
  // that correspond to these keys, return their union or intersection, whichever is the mode
  // of this MappedAOMKind.

  AbstractionPtr res;
  if(ui==Union) res = x.representativeVal->genUnion(valMAOMap);
  else          res = x.representativeVal->genIntersection(valMAOMap);

  //gettimeofday(&aggrEnd, NULL); cout << "MappedAOMKind::get() aggr\t"<<(((aggrEnd.tv_sec*1000000 + aggrEnd.tv_usec) - (aggrStart.tv_sec*1000000 + aggrStart.tv_usec)) / 1000000.0)<<endl;

  SIGHT_VERB(dbg << "result="<<res->str()<<endl, 1, AOMDebugLevel)

  return res;
}

// Set all the information associated Lattice object with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
// This function does nothing because it is a set of abstract objects rather than a map from some abstract objects
// to others.
bool MappedAOMKind::setMLValueToFull(MemLocObjectPtr ml) {
  assert(ml->isMappedAO());
  class AOMKindOp: public MAOMap::getMapFunc {
    public:
    bool modified;
    AOMKindOp(): modified(false) {}
    void operator()(AbstractionPtr obj, boost::shared_ptr<void> curVal, bool valMapped) {
      assert(valMapped);
      MemLocObjectPtr subML = boost::dynamic_pointer_cast<MemLocObject>(boost::dynamic_pointer_cast<AbstractObject>(obj));
      assert(subML);
      modified = ((AbstractObjectMapKind*)curVal.get())->setMLValueToFull(subML) || modified;
    }
  };
  AOMKindOp x;
  mappedAOMap->get(ml, x);
  return x.modified;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool MappedAOMKind::isFull() {
  class AOMKindOp: public MAOMap::applyMapFunc {
    public:
    bool res;
    AOMKindOp(): res(true) {}
    void operator()(boost::shared_ptr<void> curVal) {
      res = ((AbstractObjectMapKind*)curVal.get())->isFull() && res;
    }
  };
  AOMKindOp x;
  mappedAOMap->apply(x);
  return x.res;
}

// Returns whether this lattice denotes the empty set.
bool MappedAOMKind::isEmpty() {
  class AOMKindOp: public MAOMap::applyMapFunc {
    public:
    bool res;
    AOMKindOp(): res(true) {}
    void operator()(boost::shared_ptr<void> curVal) {
      res = ((AbstractObjectMapKind*)curVal.get())->isEmpty() && res;
    }
  };
  AOMKindOp x;
  mappedAOMap->apply(x);
  return x.res;
}

std::string MappedAOMKind::str(std::string indent) const {
  /*cout << "MappedAOMKind(this="<<this<<", parent="<<parent<<")"<<endl;
  cout << "latPEdge="<<parent->latPEdge<<")"<<endl;
  cout << "latPEdge="<<parent->latPEdge->str()<<")"<<endl;*/
  class AOMKindOp: public MAOMap::applyStrMapFunc {
    public:
    std::ostringstream out;
    std::string indent;
    AbstractObjectMap* parent;
    AOMKindOp(std::string indent, AbstractObjectMap* parent) : indent(indent), parent(parent) {
      //out << "<table><tr><td border=\"1\" colspan=\"2\"><b>MappedAOMKind(this="<<this<<", parent="<<parent<<", latPEdge="<<parent->latPEdge->str()<<")</b>:</td></tr>"<<endl;
      out << "<table><tr><td border=\"1\" colspan=\"2\"><b>MappedAOMKind</b>:</td></tr>"<<endl;
    }
    void complete() {
      out << indent << "</table>";
    }
    void operator()(const std::string& keyStr, boost::shared_ptr<void> curVal) {
      out << "<tr><td>"<<keyStr << "</td><td>"<<((AbstractObjectMapKind*)curVal.get())->str()<<"</td><tr>"<<endl;
    }
  };
  AOMKindOp x(indent, parent);
  mappedAOMap->applyStr(x);
  x.complete();
  return x.out.str();
}

// Variant of the str method that can produce information specific to the current Part.
// Useful since AbstractObjects can change from one Part to another.
std::string MappedAOMKind::strp(PartEdgePtr pedge, std::string indent) const  {
  class AOMKindOp: public MAOMap::applyStrMapFunc {
    public:
    std::ostringstream out;
    PartEdgePtr pedge;
    std::string indent;
    AOMKindOp(PartEdgePtr pedge, std::string indent): pedge(pedge), indent(indent) {
      out << "[MappedAOMKind: "<<endl;
    }
    void complete() {
      out << indent << "]";
    }
    void operator()(const std::string& keyStr, boost::shared_ptr<void> curVal) {
      out << indent << keyStr << ": "<<((AbstractObjectMapKind*)curVal.get())->strp(pedge, indent+"    ")<<endl;
    }
  };
  AOMKindOp x(pedge, indent);
  mappedAOMap->applyStr(x);
  x.complete();
  return x.out.str();
}

// -----------------
// Lattice methods
// initializes this Lattice to its default state, if it is not already initialized
void MappedAOMKind::initialize() {}

// returns a copy of this AbstractObjectMapKind
AbstractObjectMapKindPtr MappedAOMKind::copy() const {
  return boost::make_shared<MappedAOMKind>(*this, /*emptyMap*/ false);
}

// overwrites the state of this Lattice with that of that Lattice
void MappedAOMKind::copy(AbstractObjectMapKindPtr that_arg) {
  MappedAOMKindPtr that = boost::dynamic_pointer_cast<MappedAOMKind>(that_arg);

  assert(ui == that->ui);

  // We do not copy the factory since we want the factory to point to our parent AOM, not that's parent

  // Copy mappedAOMap's keys but not their values (sub-AOMs)
  mappedAOMap = that->mappedAOMap->create();

  // Initialize mappedAOMap's keys with the copies of the values (sub-AOMaps) in that.mappedAOMap
  class AOMKindOp: public MAOMap::setJoinMapFunc {
    public:
    boost::shared_ptr<void> operator()(boost::shared_ptr<void> curVal1, boost::shared_ptr<void> curVal2) {
      AbstractObjectMapKindPtr thatAOMKind = boost::static_pointer_cast<AbstractObjectMapKind>(curVal2);
      assert(thatAOMKind);
      return thatAOMKind->copy();
    }
  };
  AOMKindOp x;
  mappedAOMap->setJoin(that->mappedAOMap, x);

  // Set the parent of all the sub-maps
  setParent(parent);

  /*scope s("MappedAOMKind::copy()");
  dbg << "this->latPEdge="<<parent->latPEdge->str()<<endl;
  dbg << "that->latPEdge="<<that->parent->latPEdge->str()<<endl;*/
}

// Called by analyses to transfer this lattice's contents from across function scopes from a caller function
//    to a callee's scope and vice versa. If this this lattice maintains any information on the basis of
//    individual MemLocObjects these mappings must be converted, with MemLocObjects that are keys of the ml2ml
//    replaced with their corresponding values. If a given key of ml2ml does not appear in the lattice, it must
//    be added to the lattice and assigned a default initial value. In many cases (e.g. over-approximate sets
//    of MemLocObjects) this may not require any actual insertions. If the value of a given ml2ml mapping is
//    NULL (empty boost::shared_ptr), any information for MemLocObjects that must-equal to the key should be
//    deleted.
// Since the function is called for the scope change across some Part, it needs to account for the fact that
//    the keys in ml2ml are in scope on one side of Part, while the values on the other side. Specifically, it is
//    guaranteed that the keys are in scope at fromPEdge while the values are in scope at the edge returned
//    by getPartEdge().
// remapML must return a freshly-allocated object.
AbstractObjectMapKindPtr MappedAOMKind::remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge) {
  //scope s(txt()<<"MappedAOMKind::remapML() #ml2ml="<<ml2ml.size());
  // The MemLocs in the ml2ml mapping are MappedAbstractObjects. To call the remapML method
  // of the AOMKinds within this MappedAOMKind it is necessary to slice them into a separate
  // ml2ml mapping for each sub-key within them. Each sub-ml2ml mapping maps just the ml2 mapped
  // to each sub-key of the original mapping.

  // Create a fresh MAOMap that maps each key to the portions of the ml2ml mapping
  MAOMapPtr subml2ml = mappedAOMap->create();

  // Map each key in subml2ml to a set<MLMapping> that contains just the ml2ml mapping of
  // the sub-MemLocs in ml2ml that correspond to that key
  for(set<MLMapping>::const_iterator i=ml2ml.begin(); i!=ml2ml.end(); ++i) {
    // Convert the current ml->ml pair into a vector of AbstractionPtrs, which can be
    // passed to a call to subml2ml->setObjVecJoin
    vector<AbstractionPtr> objs;
    objs.push_back(i->from);
    objs.push_back(i->to);
    //dbg << "i->from="<<i->from->str()<<endl;
    //dbg << "i->to="<<(i->to?i->to->str():"NULL")<<endl;

    class AOMKindOp: public MAOMap::setMapObjVecJoinMapFunc {
      const MLMapping& origMLMapping;
      public:
      AOMKindOp(const MLMapping& origMLMapping) : origMLMapping(origMLMapping) {}
      boost::shared_ptr<void> operator()(boost::shared_ptr<void> curMapVal,
                                         const std::vector<AbstractObjectPtr>& curObjVals) {
        assert(curObjVals.size()==2);
        //scope s("AOMKindOp()");
        boost::shared_ptr<set<MLMapping> > subML2ML;
        // If we're at the first mapping, initialize subML2ML to be a new set of MLMappings
        if(!curMapVal) subML2ML = boost::make_shared<set<MLMapping> >();
        // Otherwise, set subML2ML to point to the previously-allocated mapping
        else           subML2ML = boost::static_pointer_cast<set<MLMapping> >(curMapVal);

        MemLocObjectPtr from = boost::dynamic_pointer_cast<MemLocObject>(curObjVals[0]);
        //dbg << "curObjVals[0]="<<curObjVals[0]->str()<<endl;
        assert(from);
        MemLocObjectPtr to   = boost::dynamic_pointer_cast<MemLocObject>(curObjVals[1]);
        //dbg << "curObjVals[1]="<<(curObjVals[1]?curObjVals[1]->str():"NULL")<<endl;

        subML2ML->insert(MLMapping(from, to, origMLMapping.replaceMapping));

        return subML2ML;
      }
    };
    AOMKindOp x(*i);
    subml2ml->setObjVecJoin(objs, x);
  }

  // Create a new MappedAOMKind that will contain the remapped version of this MappedAOMKind
  MappedAOMKindPtr newK = boost::make_shared<MappedAOMKind>(*this, /* emptyMap */ true);
  class AOMKindOp2: public MAOMap::set3MapJoinMapFunc {
    const std::set<MLMapping>& ml2ml;
    PartEdgePtr fromPEdge;
    public:
    AOMKindOp2(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge): ml2ml(ml2ml), fromPEdge(fromPEdge) {}
    boost::shared_ptr<void> operator()(boost::shared_ptr<void> curValNewK,
                                       boost::shared_ptr<void> curValMappedAOMap,
                                       boost::shared_ptr<void> curValSubML2ML) {
      if(ml2ml.size()>0)
        return boost::static_pointer_cast<AbstractObjectMapKind>(curValMappedAOMap)->remapML
                  (*boost::static_pointer_cast<set<MLMapping> >(curValSubML2ML).get(), fromPEdge);
      else
        return boost::static_pointer_cast<AbstractObjectMapKind>(curValMappedAOMap)->remapML
                          (ml2ml, fromPEdge);
    }
  };
  AOMKindOp2 y(ml2ml, fromPEdge);
  newK->mappedAOMap->set3MapJoin(mappedAOMap, subml2ml, y);

  /*scope s2("MappedAOMKind::remapML()");
  dbg << "this->parent="<<parent<<endl;
  dbg << "this->latPEdge="<<parent->latPEdge->str()<<endl;
  dbg << "newK->latPEdge="<<newK->parent->latPEdge->str()<<endl;*/

  return newK;
}

// Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously
//    maintained in this lattice about them.
// Returns true if the Lattice state is modified and false otherwise.
bool MappedAOMKind::replaceML(AbstractObjectMapKindPtr newL)  {
  MappedAOMKindPtr thatMappedAOM = boost::dynamic_pointer_cast<MappedAOMKind>(newL);
  class AOMKindOp: public MAOMap::applyJoinMapFunc {
    public:
    bool modified;
    AOMKindOp(): modified(false) {}
    void operator()(boost::shared_ptr<void> curVal1, boost::shared_ptr<void> curVal2) {
      modified = ((AbstractObjectMapKind*)curVal1.get())->replaceML(
                       boost::static_pointer_cast<AbstractObjectMapKind>(curVal2)) || modified;
    }
  };
  AOMKindOp x;
  mappedAOMap->applyJoin(thatMappedAOM->mappedAOMap, x);
  return x.modified;
}

// Computes the meet of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool MappedAOMKind::meetUpdate(AbstractObjectMapKindPtr that) {
  MappedAOMKindPtr thatMappedAOM = boost::dynamic_pointer_cast<MappedAOMKind>(that);
  class AOMKindOp: public MAOMap::applyJoinMapFunc {
    public:
    bool modified;
    AOMKindOp() : modified(false) {}
    void operator()(boost::shared_ptr<void> curVal1, boost::shared_ptr<void> curVal2) {
      modified = ((AbstractObjectMapKind*)curVal1.get())->meetUpdate(boost::static_pointer_cast<AbstractObjectMapKind>(curVal2)) || modified;
    }
  };
  AOMKindOp x;
  mappedAOMap->applyJoin(thatMappedAOM->mappedAOMap, x);
  //dbg << "MappedAOMKind::meetUpdate() modified="<<x.modified<<endl;
  return x.modified;
}

// Computes the intersection of this and that and saves the result in this
// Returns true if this causes this to change and false otherwise
bool MappedAOMKind::intersectUpdate(AbstractObjectMapKindPtr that) {
  MappedAOMKindPtr thatMappedAOM = boost::dynamic_pointer_cast<MappedAOMKind>(that);
  class AOMKindOp: public MAOMap::applyJoinMapFunc {
    public:
    bool modified;
    AOMKindOp() : modified(false) {}
    void operator()(boost::shared_ptr<void> curVal1, boost::shared_ptr<void> curVal2) {
      modified = ((AbstractObjectMapKind*)curVal1.get())->intersectUpdate(boost::static_pointer_cast<AbstractObjectMapKind>(curVal2)) || modified;
    }
  };
  AOMKindOp x;
  mappedAOMap->applyJoin(thatMappedAOM->mappedAOMap, x);
  return x.modified;
}

/*bool MappedAOMKind::finiteLattice() {
  assert(ml->isMappedAO());
  class AOMKindOp: public MAOMap::getMapFunc {
    public:
    bool res;
    AOMKindOp(): res(true) {}
    void operator()(AbstractionPtr obj, boost::shared_ptr<void> curVal, bool valMapped) {
      assert(valMapped);
      MemLocObjectPtr subML = boost::dynamic_pointer_cast<MemLocObject>(obj);
      assert(subML);
      res &&= ((AbstractObjectMapKind*)curVal.get())->finiteLattice();
    }
  };
  AOMKindOp x;
  mappedAOMap->get(key, x);
  return x.res;
}*/

bool MappedAOMKind::operator==(AbstractObjectMapKindPtr that)  {
  MappedAOMKindPtr thatMappedAOM = boost::dynamic_pointer_cast<MappedAOMKind>(that);
  class AOMKindOp: public MAOMap::applyJoinMapFunc {
    public:
    bool differ;
    AOMKindOp() : differ(false) {}
    void operator()(boost::shared_ptr<void> curVal1, boost::shared_ptr<void> curVal2) {
      differ = (*((AbstractObjectMapKind*)curVal1.get()) ==
                boost::static_pointer_cast<AbstractObjectMapKind>(curVal2)) || differ;
    }
  };
  AOMKindOp x;
  mappedAOMap->applyJoin(thatMappedAOM->mappedAOMap, x);
  return x.differ;
}

}; // namespace fuse
