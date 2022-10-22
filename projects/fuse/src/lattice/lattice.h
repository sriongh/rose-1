#ifndef LATTICE_H
#define LATTICE_H

#include "CallGraphTraverse.h"
//#include "variables.h"
#include "partitions.h"
#include "abstract_object.h"
#include <string>
#include <map>

namespace fuse {
class Lattice;
typedef boost::shared_ptr<Lattice> LatticePtr;
class Lattice: public Abstraction
{
  public:
  PartEdgePtr latPEdge;
  //Lattice() {}
  Lattice(PartEdgePtr latPEdge) : latPEdge(latPEdge) {}
  
  // This virtual destructor is required to ensure boost::shared_ptrs identify types of Lattices correctly
  virtual ~Lattice();

  public:
  // Sets the PartEdge that this Lattice's information corresponds to. 
  // Returns true if this causes the edge to change and false otherwise
  virtual bool setPartEdge(PartEdgePtr latPEdge);
  
  // Returns the PartEdge that this Lattice's information corresponds to
  virtual PartEdgePtr getPartEdge() const;
  
  // initializes this Lattice to its default state, if it is not already initialized
  virtual void initialize()=0;
  // returns a copy of this lattice
  virtual Lattice* copy() const=0;
  LatticePtr copySharedPtr() const { return boost::shared_ptr<Lattice>(copy()); }
  // overwrites the state of this Lattice with that of that Lattice
  virtual void copy(Lattice* that)
  { latPEdge = that->latPEdge; }
  
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
  virtual Lattice* remapML(const std::set<MLMapping>& ml2ml, PartEdgePtr fromPEdge) {
    return copy();
  }
  
  // Adds information about the MemLocObjects in newL to this Lattice, overwriting any information previously 
  //    maintained in this lattice about them.
  // Returns true if the Lattice state is modified and false otherwise.
  virtual bool replaceML(Lattice* newL)
  {
    return false;
  }
  bool replaceML(LatticePtr newL) { return replaceML(newL.get()); }
  
  // Propagate information from a set of defs to a single use. Return true if this causes the Lattice to change.
  virtual bool propagateDefs2Use(MemLocObjectPtr use, const std::set<MemLocObjectPtr>& defs) { std::cerr << "ERROR: function propagateDefs2Use should be implemented for this Lattice! "<<str()<<std::endl; ROSE_ASSERT(0); }

  // Propagate information from a single defs to a set of uses. Return true if this causes the Lattice to change.
  virtual bool propagateDef2Uses(const std::set<MemLocObjectPtr>& uses, MemLocObjectPtr def) { std::cerr << "ERROR: function propagateDef2Uses should be implemented for this Lattice! "<<str()<<std::endl; ROSE_ASSERT(0); }

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool meetUpdate(Lattice* that)=0;
  bool meetUpdate(LatticePtr that) { return meetUpdate(that.get()); }
  
  // Computes the intersection of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool intersectUpdate(Lattice* that)=0;
  bool intersectUpdate(LatticePtr that) { return intersectUpdate(that.get()); }

  // Returns true if this Lattice implies that lattice (its constraints are equal to or tighter than those of 
  // that Lattice) and false otherwise.
  virtual bool implies(Lattice* that) {
    // this is tighter than that if meeting that into this causes this to change (that contains possibilities 
    // not already in this) but not vice versa (all the possibilities in this already exist in that)
    Lattice* thisCopy = copy();
    if(!thisCopy->meetUpdate(that)) { 
      delete thisCopy;
      return false;
    }
    delete thisCopy;
    
    Lattice* thatCopy = that->copy();
    if(thatCopy->meetUpdate(this)) {
      delete thatCopy;
      return false;
    }
    delete thatCopy;
    return true;
  }
  bool implies(LatticePtr that) { return implies(that.get()); }
  
  // Returns true if this Lattice is semantically equivalent to that lattice (both correspond to the same set
  // of application executions).
  virtual bool equiv(Lattice* that) {
    // this and that are equivalent if meeting either one with the other causes no changes
    Lattice* thisCopy = copy();
    if(thisCopy->meetUpdate(that)) { 
      delete thisCopy;
      return false;
    }
    delete thisCopy;
    
    Lattice* thatCopy = that->copy();
    if(thatCopy->meetUpdate(this)) {
      delete thatCopy;
      return false;
    }
    delete thatCopy;
    return true;
  }
  bool equiv(LatticePtr that) { return equiv(that.get()); }
  
  // Computes the meet of this and that and returns the result
  virtual bool finiteLattice()=0;
  
  virtual bool operator==(Lattice* that) /*const*/=0;
  bool operator!=(Lattice* that) {
    return !(*this == that);
  }
  bool operator==(Lattice& that) {
    return *this == &that;
  }
  bool operator!=(Lattice& that) {
    return !(*this == that);
  }
  
  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  virtual bool setToFull()=0;
  // Set this Lattice object to represent the of no execution prefixes (empty set).
  // Return true if this causes the object to change and false otherwise.
  virtual bool setToEmpty()=0;
  
  // Set all the value information that this Lattice object associates with this MemLocObjectPtr to full.
  // Return true if this causes the object to change and false otherwise.
  virtual bool setMLValueToFull(MemLocObjectPtr ml)=0;
  
  // Functions used to inform this lattice that a given variable is now in use (e.g. a variable has entered 
  //    scope or an expression is being analyzed) or is no longer in use (e.g. a variable has exited scope or
  //    an expression or variable is dead).
  // It is assumed that a newly-added variable has not been added before and that a variable that is being
  //    removed was previously added
  /*virtual void addVar(varID var)=0;
  virtual void remVar(varID var)=0;*/
      
  // The string that represents ths object
  // If indent!="", every line of this string must be prefixed by indent
  // The last character of the returned string should not be '\n', even if it is a multi-line string.
  //virtual string str(string indent="") /*const*/=0;

  //virtual std::string str(std::string indent="") const=0;

  // From Abstraction:
  // -----------------
  // Returns a copy of this Abstraction
  AbstractionPtr copyA() const
  { return boost::static_pointer_cast<Abstraction>(copySharedPtr()); }

  // Returns whether this object may/must be equal to o within the given Part p
  bool mayEqual(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { assert(0); }
  bool mustEqual(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { assert(0); }

  // General versions of equalSet() that accounts for framework details before routing the call to the
  // derived class' equalSet() check. Specifically, it routes the call through the composer to make
  // sure the equalSet() call gets the right PartEdge.
  bool equalSet(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return equiv(boost::static_pointer_cast<Lattice>(o)); }

  // General versions of subSet() that accounts for framework details before routing the call to the
  // derived class' subSet() check. Specifically, it routes the call through the composer to make
  // sure the subSet() call gets the right PartEdge.
  bool subSet(AbstractionPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { assert(0); }

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdate(AbstractionPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return meetUpdate(boost::static_pointer_cast<Lattice>(that)); }

  // Computes the intersection of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool intersectUpdate(AbstractionPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return intersectUpdate(boost::static_pointer_cast<Lattice>(that)); }

  // Set this Lattice object to represent the set of all possible execution prefixes.
  // Return true if this causes the object to change and false otherwise.
  bool setToFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return setToFull(); }

  // Set this Lattice object to represent the of no execution prefixes (empty set).
  // Return true if this causes the object to change and false otherwise.
  bool setToEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return setToEmpty(); }

  // General versions of isFull() and isEmpty that account for framework details before routing the call to the
  // derived class' isFull() and isEmpty()  check. Specifically, it routes the call through the composer to make
  // sure the isFullAO() and isEmptyAO() call gets the right PartEdge.
  virtual bool isFull()=0;
  bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return isFull(); }

  virtual bool isEmpty()=0;
  bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis=NULL)
  { return isEmpty(); }

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
  AbstractionPtr genUnion(boost::shared_ptr<MAOMap> maoMap);

  AbstractionPtr genIntersection(boost::shared_ptr<MAOMap> maoMap);
};

class FiniteLattice : public virtual Lattice
{
  public:
  //FiniteLattice() {}
  FiniteLattice(PartEdgePtr latPEdge) : Lattice(latPEdge) {}
  
  bool finiteLattice()
  { return true;  }
};

class InfiniteLattice : public virtual Lattice
{
  public:
  //InfiniteLattice() {}
  InfiniteLattice(PartEdgePtr latPEdge) : Lattice(latPEdge) {}
  
  bool finiteLattice()
  { return false; }
  
  // widens this from that and saves the result in this
  // returns true if this causes this to change and false otherwise
  virtual bool widenUpdate(InfiniteLattice* that)=0;
};

}; // namespace fuse
#endif
