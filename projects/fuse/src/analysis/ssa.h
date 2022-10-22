#pragma once

#include <boost/function.hpp>
#include "cfgUtils.h"
#include "comp_shared_ptr.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include "abstract_object.h"

namespace fuse {

/*
// Analysis algorithm on top of SSA
while(workflow != empty) {
  ATSnode atsN = workflow.grabNext();
  // If this atsNode is preceded by a phi node
  if(atsN.hasPhiNodeBefore) {
    // All usedefs in ssaN are guaranteed to denote disjoint MemLoc sets
    foreach usedef (SSAMemLocObject) in ssaN {
      foreach def (SSAMemLocObject) in usedefNode.defs() {
        Lattice@def = get info of def in global Lattice
        Lattice@usedef union= Lattice@def
      }
    }
  }

  transfer :
  {
    For each use (MemLocObject => SSAMemLocObject(MemLoc, atsN, useExpr, use)):
      find def (SSAMemLocObject) mapped to this use
      get up its info in global Lattice
    For each def (MemLocObject => SSAMemLocObject(MemLoc, atsN, defExpr, def)):
      set its info in global lattice
  }

  // If this atsNode is followed by a phi node
  if(atsN.hasPhiNodeAfter) {
    // All usedefs in ssaN are guaranteed to denote disjoint MemLoc sets
    foreach usedef (SSAMemLocObject) in ssaN {
      foreach def (SSAMemLocObject) in usedefNode.defs() {
        Lattice@def = get info of def in global Lattice
        Lattice@usedef union= Lattice@def
      }
    }
  }
}

Requirements:

SSAMemLocObject {
  // SSAMemLocObjects need the following info to be unique (so they can be used as keys in datastructures)
  ATSNode
  SgNode that denotes the expression that is defined or used (operations like "a++" both use and def "a")
  Whether the ML is a def, a use or a phi def
  If a phi def, the index of this SSAMemLoc within its container phi (this index provides uniqueness in the same was as SgNode* does it for regular defs and uses)

  // For maintaining the MemLocObject API
  MemLocObject being wrapped
}

Phi Nodes {
  must contain a map from each SSAMemLocObject Q they def to the set of def SSAMemLocObjects that target Q
  Each key in this map is guaranteed to denote a disjoint set of MemLocs
}

Regular ATS Nodes {
  Mapping from uses (SSAMemLocObject) to the defs that target each one
*/

// Wraps regular MemLocObjects with enough information to guarantee the static single assignment
// property from the SSAGraph. An SSA def is uniquely defined by
// - the Part where it occurs,
// - the SgNode at that part that is being used or defined
// - whether it is being used or defined (in expressions like a++ the same expression a is
//   both used and defined) or accessed in a Phi Node (usedef accessType below)
class SSAMemLocObject;
class SSAMLHierKey;
class Fuse;
class FuseCFGNode;
typedef CompSharedPtr<FuseCFGNode> FuseCFGNodePtr;
//typedef boost::shared_ptr<SSAMemLocObject> SSAMemLocObjectPtr;
typedef CompSharedPtr<SSAMemLocObject> SSAMemLocObjectPtr;
class SSAMemLocObject : public MemLocObject, public comparable {
  friend class SSAMLHierKey;
  public:
  MemLocObjectPtr baseML;
  PartPtr loc;
  SgNode* sgn;
  int phiID; // The index of this SSAMemLoc at a phi node (-1 if the SSAMemloc denotes a regular use or def)

  typedef enum {def, use, // Def or use of a MemLoc at a regular ATS node
                phiDef // Def at a phi node. Incoming defs or phiDefs are mapped to phiDefs are phi
                       // nodes without the need for additional use SSAMemLocObjects
               } accessType;
  accessType access;

  SSAMemLocObject(MemLocObjectPtr baseML, PartPtr loc, SgNode* sgn, accessType access, int phiID=-1) :
    MemLocObject(baseML->getBase()), baseML(baseML), loc(loc), sgn(sgn), phiID(phiID), access(access){}

  SSAMemLocObject(const SSAMemLocObject& that);

  PartPtr getLoc() const { return loc; }

  // Return the FuseCFGNode of the given Fuse object that wraps the location Part of this SSAMemLocObject
  FuseCFGNodePtr getFuseCFGNodeLoc(Fuse* fuseAnalysis) const;

  SgNode* getSgNode() const { return sgn; }

  std::string accessType2Str(accessType access) const {
    return (access==def?   "def":
           (access==use?   "use":
           (access==phiDef?"phiDef":"???")));
  }
  accessType getAccess() const { return access; }

  // pretty print
  std::string str(std::string indent="") const;

  // copy this object and return a pointer to it
  MemLocObjectPtr copyML() const;

  SgNode* getBase() const;
  MemRegionObjectPtr getRegion() const;
  ValueObjectPtr     getIndex() const;

  bool operator==(const SSAMemLocObjectPtr& that) const;
  bool operator< (const SSAMemLocObjectPtr& that) const;
  bool operator!=(const SSAMemLocObjectPtr& that) const;
  bool operator>=(const SSAMemLocObjectPtr& that) const;
  bool operator<=(const SSAMemLocObjectPtr& that) const;
  bool operator> (const SSAMemLocObjectPtr& that) const;

  bool mayEqual(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  bool mustEqual(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool equalSet(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the given abstract object.
  bool subSet(MemLocObjectPtr o, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Computes the meet of this and that and saves the result in this
  // returns true if this causes this to change and false otherwise
  bool meetUpdate(MemLocObjectPtr that, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Returns whether this AbstractObject denotes the set of all possible execution prefixes.
  bool isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
  // Returns whether this AbstractObject denotes the empty set.
  bool isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);

  // Set this object to represent the set of all possible MemLocs
  // Return true if this causes the object to change and false otherwise.
  bool setToFull();

  // Set this Lattice object to represent the empty set of MemLocs.
  // Return true if this causes the object to change and false otherwise.
  bool setToEmpty();

  // Returns whether all instances of this class form a hierarchy. Every instance of the same
  // class created by the same analysis must return the same value from this method!
  bool isHierarchy() const;
  // AbstractObjects that form a hierarchy must inherit from the AbstractionHierarchy class

  // Returns a key that uniquely identifies this particular AbstractObject in the
  // set hierarchy.
  const AbstractionHierarchy::hierKeyPtr& getHierKey() const;

  // Methods for the comparable API
  bool equal(const comparable& that) const;
  bool less(const comparable& that) const;
}; // class SSAMemLocObject

extern SSAMemLocObjectPtr NULLSSAMemLocObject;

class SSAMLHierKey: public AbstractionHierarchy::hierKey {
  SSAMemLocObjectPtr ssaML;
  public:
  SSAMLHierKey(SSAMemLocObjectPtr ssaML);
  bool isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis);
}; // class SSAMLHierKey

class SSAGraph: public boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, PartPtr, PartEdgePtr>
{
  protected:
  Composer* comp;
  ComposedAnalysis* analysis;

  public:
  SSAGraph(Composer* comp, ComposedAnalysis* analysis);

  protected:
  typedef boost::graph_traits<SSAGraph>   GraphTraits;
  typedef GraphTraits::vertex_descriptor  Vertex;
  typedef GraphTraits::edge_descriptor    Edge;

  typedef std::map<Vertex, Vertex>        VertexVertexMap;
  typedef std::set<Vertex>                Vertices;

  void buildCFG(PartPtr node,
                std::map<PartPtr, Vertex>& nodesAdded,
                std::set<PartPtr>& nodesProcessed);

  // -------------------------------
  // Functionality for building control dominator trees and dominance frontiers
  // -------------------------------

  std::set<PartPtr> startNodes;

  //! A map from a CFG node to the corresponding vertex
  std::map<PartPtr, Vertex> nodesToVertices;

  //! The dominator tree of this CFG.
  mutable VertexVertexMap dominatorTree;

  //! The postdominator tree of this CFG.
  mutable VertexVertexMap postdominatorTree;

  std::map<PartPtr, PartPtr> iDominatorMap;
  std::map<PartPtr, PartPtr> iPostDominatorMap;

  // Maps each Part to the set of Parts on its dominance frontier
  std::map<PartPtr, std::set<PartPtr> > dominanceFrontiers;

  // Inverse of dominanceFrontiers that maps each Part to the set of Parts that have it on their dominance frontiers
  std::map<PartPtr, std::set<PartPtr> > domFrontierOf;

  public:
  // Return the entry Parts of the ATS
  std::set<PartPtr> GetStartAStates();

  // Return the exit Parts of the ATS
  std::set<PartPtr> GetEndAStates();

  const SSAGraph::VertexVertexMap& getDominatorTree();// const;

  const SSAGraph::VertexVertexMap& getPostdominatorTree();// const;

  Vertex getVertexForNode(PartPtr node) const;

  std::set<PartPtr> calculateIteratedDominanceFrontier(const std::vector<PartPtr>& startNodes);

  void calculateDominanceFrontiers();

  void showDominatorTree();
  void showDominanceFrontier();

  // -------------------------------
  // Functionality for building SSA graphs
  // -------------------------------

  protected:
  // Map each Part to the set of MemLocs defined and used at this Part.
  // The SSAMemLocObjects in each set of defs must denote disjoint sets of MemLocs.
  std::map<PartPtr, std::set<SSAMemLocObjectPtr> > defs;
  // Uses are not constrained in this way.
  std::map<PartPtr, std::set<SSAMemLocObjectPtr> > uses;

  /* // Map each function to the MemLocs that denote the uses of its parameters (the definitions are arguments
  // in corresponding function calls).
  // These are not part of regular defs and uses to keep track of their order in the argument list.
  std::map<Function, std::list<MemLocObjectPtr> > funcParamUses;

  // Map each Part that corresponds to the outgoing side of a function call to the list
  // of SSAMemLocPtrs that denote the defs of its arguments.
  // These are not part of regular defs and uses to keep track of their order.
  std::map<PartPtr, std::list<SSAMemLocObjectPtr> > funcArgDefs;*/

  // Maps each Part that corresponds to the outgoing side of a function call to the list that
  // records for each argument (i-th list element corresponds to the i-th argument) the set
  // of SSAMemLocObjectPtrs that denote the function parameters this argument matches.
  std::map<PartPtr, std::list<std::pair<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > > > funcArg2Param;

  static std::set<SSAMemLocObjectPtr>  emptySSAMemLocObjectSet;
  static std::list<SSAMemLocObjectPtr> emptySSAMemLocObjectList;
  static std::list<std::pair<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > > emptySSAMemLocObjectMapping;

  public:
  // Given a set of SSAMemLocObjectPtrs returns a corresponding set of MemLocObjectPtrs
  static std::set<MemLocObjectPtr> SSAMLSet2MLSet(const std::set<SSAMemLocObjectPtr>& s);

  // Return the set of uses at this part
  const std::set<SSAMemLocObjectPtr>& getUses(PartPtr part) const;
  std::set<MemLocObjectPtr> getUsesML(PartPtr part) const;

  // Return the set of defs at this part
  const std::set<SSAMemLocObjectPtr>& getDefs(PartPtr part) const;
  std::set<MemLocObjectPtr> getDefsML(PartPtr part) const;

  // Get the list of definitions of the arguments within the function call at the given part
  // Get the mapping from each argument of the function call at the given part to the corresponding
  // parameters the argument defines
  const std::list<std::pair<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& getFunc2Params(PartPtr part) const;

  // Collects all the defs and uses at each Part and stores them into defs and uses
  void collectDefsUses();

  void showDefsUses();

  // Maps each Part where a phi node should be placed before or after a Part to the
  // MemLocs that will be defined at this phi node. The MemLocs are organized as a
  // map from phiDef SSAMemLocs that denote the definitions at the phi node to the
  // def or phiDef SSAMemLocs that reach this particular phiDef. The phiDefs that
  // are the keys of this map must denote non-overlapping sets of MemLocs.
  std::map<PartPtr, std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > > phiDefs;
  //std::map<PartPtr, std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > > phiMLsAfter;
  //std::map<PartPtr, int> phiNodeID;

  // Finds all the Parts where phi nodes should be placed and identifies the MemLocs they must define
  //void placePhiNodes();

  void showPhiNodes();

  // Returns whether the given Part has a phi node before or after it
  bool isPhiNode(PartPtr part) const {
    std::map<PartPtr, std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >::const_iterator i=phiDefs.find(part);
    return i != phiDefs.end() && i->second.size()>0;
  }

  // Returns the ID of the given phiNode part
  //int getPhiNodeID(PartPtr part);

  /*bool isPhiNodeBefore(PartPtr part) const { return phiMLsBefore.find(part) != phiMLsBefore.end(); }
  bool isPhiNodeAfter (PartPtr part) const { return phiMLsAfter.find(part)  != phiMLsAfter.end(); }*/

  // Returns the mapping of phiDef MemLocs at the given phiNode before the given part
  // to the defs and phiDefs that reach them.
  const std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> >& getDefsUsesAtPhiNode(PartPtr part) const;

  // Returns the set of defs and phiDefs that reach the given phiDef
  const std::set<SSAMemLocObjectPtr>& getReachingDefsAtPhiDef(SSAMemLocObjectPtr pd) const;

  /*const std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> >& getDefsUsesAtPhiNodeBefore(PartPtr part) const {
    ROSE_ASSERT(isPhiNodeBefore(part));
    std::map<PartPtr, std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >::const_iterator i=phiDefsBefore.find(part);
    ROSE_ASSERT(i!=phiMLsBefore.end());
    return i->second;
  }
  // Returns the mapping of phiDef MemLocs at the given phiNode after the given part
  // to the defs and phiDefs that reach them.
  const std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> >& getDefsUsesAtPhiNodeAfter(PartPtr part) const {
    ROSE_ASSERT(isPhiNodeAfter(part));
    std::map<PartPtr, std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >::const_iterator i=phiMLsAfter.find(part);
    ROSE_ASSERT(i!=phiMLsAfter.end());
    return i->second;
  }*/

  // Returns whether the given def terminates at the given phi node
  bool defTerminatesAtPhiNode(PartPtr phiPart, SSAMemLocObjectPtr def) const;

  protected:
  // Maps each regular use or phi def to the set of defs or phiDefs that reach it
  std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > use2def;
  // Maps each def or phiDef to the set of uses or phiDefs that it reaches
  std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > def2use;

  // Connect a def/phiDef to a use/phiDef that it reaches
  void addDefUseLink(SSAMemLocObjectPtr use, SSAMemLocObjectPtr def);

  // Erases all the regular non-phi uses recorded for the given part
  void eraseUsesFromU2D(PartPtr part);

  // For each use->def link recorded in use2def and phiDefs, adds a corresponding
  // reverse link in def2use
  void setDef2Use();

  public:
  // Returns the SSA uses for the given def
  const std::set<SSAMemLocObjectPtr>& getUses(SSAMemLocObjectPtr def) const;
  std::set<MemLocObjectPtr> getUsesML(SSAMemLocObjectPtr def) const;

  // Returns the SSA defs for the given use
  const std::set<SSAMemLocObjectPtr>& getDefs(SSAMemLocObjectPtr use) const;
  std::set<MemLocObjectPtr> getDefsML(SSAMemLocObjectPtr use) const;

  // Returns the defs that transitively reach the given use through zero or more phi nodes
  std::set<SSAMemLocObjectPtr> getTransitiveDefs(SSAMemLocObjectPtr use) const;
  std::set<MemLocObjectPtr> getTransitiveDefsML(SSAMemLocObjectPtr use) const;

  /* // Matches the given use to the given def at the given edge.
  // If the def must-defines the use, the use is removed from liveUses, a def->use connection 
  //    is recorded and true is returned. 
  // Otherwise, if def may-defines use, then use is not removed, a def->use connection is recorded
  //    and false is returned.
  // Otherwise, nothing is changed and false is returned.
  bool matchUseToDef(PartEdgePtr pedge, SSAMemLocObjectPtr use, SSAMemLocObjectPtr def, 
                     std::set<SSAMemLocObjectPtr>& liveUses);*/

  // Simulate a definition of all the defs after the definitions of all the defs in curLiveDefs, updating
  // curLiveDefs with the result.
  // Thus, if a SSAMemLocObject in defs must-equals a MemLocObject that is a group key (first element in
  //   pair) in curLiveDefs, the old mapping is replaced with the SSAMemLocObject from defs (the new
  //   mapping has the same baseML but new definition location.
  // However, if it is may-equals but not must-equals, the mapping from defs is added to the group (the
  //   second element in pair) without removing the original.
  void assignDefAfterLiveDefs(std::list<std::pair<MemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& curLiveDefs,
                              std::set<SSAMemLocObjectPtr>& defs,
                              PartEdgePtr pedge);

  // Merge the two given lists of live definition groups, updating toLiveDefs with the result
  // Returns true if toLiveDefs is modified and false otherwise.
  bool mergeLiveDefs(std::list<std::pair<MemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& toLiveDefs,
                     std::list<std::pair<MemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& fromLiveDefs,
                     PartEdgePtr pedge);
  
  // Returns whether the two collections of live defs contain the same defs
  bool equalLiveDefs(const std::list<std::pair<MemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& aLD,
                     const std::list<std::pair<MemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& bLD);

  // Computes the mapping from MemLoc uses to their defs
  void computeUse2Def();

  void showUse2Def();

  // Records whether the SSA has already been built
  bool ssaBuilt;

  // Creates all the look-aside data structures required to represent the ATS in SSA form
  void buildSSA();
}; // class SSAGraph

}; // namespace fuse
