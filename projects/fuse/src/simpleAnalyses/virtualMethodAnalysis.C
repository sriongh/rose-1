#include "sage3basic.h"
#include "virtualMethodAnalysis.h"
#include "sight.h"
#include "sageInterface.h"
#include "midend/programAnalysis/CallGraphAnalysis/CallGraph.h"
using namespace std;
using namespace sight;
using namespace SageInterface;

namespace fuse {
#define VirtualMethodAnalysisDebugLevel 0

/********************************
 ***** ClassInheritanceTree *****
 ********************************/

// Points to the object that stores the entire class hierarchy of all types
ClassHierarchyWrapper* ClassInheritanceTree::classHierarchy=NULL;

// Initializes the ClassHierarchyGraph, if needed
void ClassInheritanceTree::initCHG() {
  if(classHierarchy==NULL) {
    classHierarchy = new ClassHierarchyWrapper(getProject());
    ROSE_ASSERT(classHierarchy);
  }
}

ClassInheritanceTree::ClassInheritanceTree(PartEdgePtr latPEdge): Lattice(latPEdge), FiniteLattice(latPEdge) {
  state = empty;
  inclusive = false;
}

// Creates a tree that captures the inheritance of the given SgClassDefinition
ClassInheritanceTree::ClassInheritanceTree(SgClassDefinition* classDecl, PartEdgePtr latPEdge): Lattice(latPEdge), FiniteLattice(latPEdge) {
  SIGHT_VERB(scope s(txt()<<"ClassInheritanceTree::ClassInheritanceTree("<<SgNode2Str(classDecl)<<")"), 1, VirtualMethodAnalysisDebugLevel)
  state = tree;
  inclusive = true;
  initCHG();

  // Records the SgClassDefinition sets from which classDecl inherits along different
  // paths as a list of individual SgClassDefinitions from which classDecl
  // inherits and their immediate ancestors.
  std::list<std::pair<NodePtr, ClassHierarchyWrapper::ClassDefSet> > worklist;

  // Create a Node for classDecl and set it to be the tree's leaf
  NodePtr classDeclNode = makePtr<Node>(classDecl);
  leaves.push_back(classDeclNode);

  // Initialize the worklist with the single set of classDecl's parents
  ClassHierarchyWrapper::ClassDefSet parents = classHierarchy->getDirectAncestorClasses(classDecl);
  if(parents.size()>0) {
    worklist.push_back(make_pair(classDeclNode, parents));

    // Keep working over the inheritance hierarchy while the worklist is still non-empty
    while(worklist.size()>0) {
      // Grab another element from the worklist
      std::pair<NodePtr, ClassHierarchyWrapper::ClassDefSet> cur = worklist.front();
      worklist.pop_front();
      scope s("worklist iter");
      SIGHT_VERB(dbg << "cur.first="<<cur.first->str()<<", #cur.second="<<cur.second.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)

      // Iterate over all the SgClassDefinitions in the current ancestor set
      for(ClassHierarchyWrapper::ClassDefSet::const_iterator i=cur.second.begin(); i!=cur.second.end(); i++) {
        SIGHT_VERB(dbg << "    ancestor="<<SgNode2Str(*i)<<endl, 1, VirtualMethodAnalysisDebugLevel)
        // Create a tree node for the current parents SgClassDefinition
        NodePtr parentNode = makePtr<Node>(*i);

        // Connect the derived class' node to the current ancestor and vice versa
        cur.first->parents.insert(parentNode);
        parentNode->child = cur.first;

        // Add the current parent node's parents to the worklist
        ClassHierarchyWrapper::ClassDefSet grandparents = classHierarchy->getDirectAncestorClasses(*i);
        SIGHT_VERB(dbg << "#grandparents="<<grandparents.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)
        if(grandparents.size()>0)
          worklist.push_back(make_pair(parentNode, grandparents));
        // If this parent SgClassDefinition has no parents, record it as a base node of the tree
        else
          bases.push_back(parentNode);
      }
    }
  } else {
    // If this class doesn't inherit from another, add it as a base as well as a leaf
    bases.push_back(classDeclNode);
  }

  SIGHT_VERB(dbg << "final:"<<endl<<str()<<endl, 1, VirtualMethodAnalysisDebugLevel)
}

ClassInheritanceTree::ClassInheritanceTree(const ClassInheritanceTree& that) : Lattice(that), FiniteLattice(that) {
  copyFrom(that);
}

ClassInheritanceTree& ClassInheritanceTree::operator=(const ClassInheritanceTree& that) {
  copyFrom(that);
  return *this;
}

// Copies the state of that tree over this tree.
void ClassInheritanceTree::copyFrom(const ClassInheritanceTree& that) {
  state = that.state;
  inclusive = that.inclusive;

  list<pair<NodePtr, NodePtr> > worklist;
  for(list<NodePtr>::const_iterator l=that.leaves.begin(); l!=that.leaves.end(); l++) {
    NodePtr newLeaf = makePtr<Node>((*l)->def);
    leaves.push_back(newLeaf);
    worklist.push_back(make_pair(newLeaf, *l));
  }

  while(worklist.size()>0) {
    // Get the next node in the tree
    pair<NodePtr, NodePtr> cur=worklist.front();
    worklist.pop_front();

    if(cur.second->parents.size()==0)
      bases.push_back(cur.first);
    else {
      // Create copies of the current node's parents and push them onto the worklist
      for(std::set<NodePtr>::const_iterator p=cur.second->parents.begin(); p!=cur.second->parents.end(); p++) {
        NodePtr newNode = makePtr<Node>((*p)->def);
        newNode->child = cur.first;
        cur.first->parents.insert(newNode);
        worklist.push_front(make_pair(newNode, *p));
      }
    }
  }
}

// returns a copy of this lattice
Lattice* ClassInheritanceTree::copy() const {
  return new ClassInheritanceTree(*this);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
// The part of this object is to be used for AbstractObject comparisons.
// Meet update finds the common tree prefix among the two treeS
bool ClassInheritanceTree::meetUpdate(Lattice* that_arg) {
  SIGHT_VERB_DECL(scope, ("ClassInheritanceTree::meetUpdate()", scope::medium), 1, VirtualMethodAnalysisDebugLevel)
  ClassInheritanceTree* that = dynamic_cast<ClassInheritanceTree*>(that_arg);
  ROSE_ASSERT(that);

  SIGHT_VERB_IF(1, VirtualMethodAnalysisDebugLevel)
    dbg << "state="<<state2Str(state)<<", that->state="<<state2Str(that->state)<<endl;
    { scope s("this", scope::low); dbg << "this="<<str()<<endl; }
    { scope s("that", scope::low); dbg << "that="<<that->str()<<endl; }
  SIGHT_VERB_FI()
  // Full trees cannot become any more relaxed
  if(state==full) return false;
  // If that tree is full, make this tree full as well
  if(that->state==full) {
    if(state==full) return false;
    else { setToFull(); return true; }
  }
  // Neither tree is full

  // If this tree is empty, copy the state of that tree over this tree
  if(state==empty) {
    if(that->state==empty) return false;
    else { copyFrom(*that); return true; }
  }
  // If that is empty, don't modify this
  if(that->state==empty) return false;

  // Both trees are neither full nor empty

  // If the two inheritance trees don't have the same base classes, they have
  // nothing in common, which makes this the full hierarchy (no constraints)
  if(bases.size() != that->bases.size()) {
    SIGHT_VERB(dbg << "Inconsistent bases: setting to Full. #bases="<<bases.size()<<", #that->bases="<<that->bases.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)
    setToFull();
    return true;
  }

  // Records whether this tree was modified during the merge
  bool modified = false;

  // Iterate from the bases to their children looking for the point in the
  // iteration where the trees stop being identical. The identical portions
  // are left in the merged tree and the non-identical ones are removed.

  // The worklist contains a list of pairs, one node from each tree
  std::list<std::pair<NodePtr, NodePtr> > worklist;

  // Initialize the list with the base nodes
  for(std::list<NodePtr>::iterator thisB=bases.begin(), thatB=that->bases.begin();
      thisB!=bases.end(); ++thisB, ++thatB) {
    // If the two inheritance trees don't have the same base classes, they have
    // nothing in common, which makes this the full hierarchy (no constraints)
    if((*thisB)->def != (*thatB)->def) {
      SIGHT_VERB(dbg << "Inconsistent bases: setting to Full. (*thisB)->def="<<(*thisB)->def<<"="<<SgNode2Str((*thisB)->def)<<" != (*thatB)->def="<<(*thatB)->def<<"="<<SgNode2Str((*thatB)->def)<<endl, 1, VirtualMethodAnalysisDebugLevel)
      setToFull();
      return true;
    }
    worklist.push_back(make_pair(*thisB, *thatB));
  }
  SIGHT_VERB(dbg << "#worklist="<<worklist.size()<<", #bases="<<bases.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)

  // Erase the bases and leaves of this tree. The leaves will be regenerated as the set of
  // nodes that are common among the two trees the children of which are
  // non-existent or different between the trees
  bases.clear();
  leaves.clear();

  // The merged tree will be inclusive if the two trees agree on any leaf.
  // Thus, we initialize inclusive to false and set it to true if we ever see leaf agreement.
  bool origInclusive=inclusive;
  inclusive = false;

  // Keep iterating through the worklist until as long as the node pairs
  // have identical SgClassDefinitions.
  while(worklist.size()>0) {
    SIGHT_VERB(scope reg("worklist", scope::low), 1, VirtualMethodAnalysisDebugLevel)
    SIGHT_VERB(dbg << "#worklist="<<worklist.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)

    pair<NodePtr, NodePtr> cur = worklist.front();
    worklist.pop_front();
    SIGHT_VERB(dbg << "cur=<"<<cur.first->str()<<","<<endl<<"    "<<cur.second->str()<<">"<<endl, 1, VirtualMethodAnalysisDebugLevel)

    // If the children of the current nodes on both trees are the same
    if(cur.first->child == cur.second->child) {
      // If they're both NULL, add the current node in this tree to leaves
      // and don't add anything else to the worklist
      if(!cur.first->child) {
        SIGHT_VERB(dbg << "NULL children"<<endl, 1, VirtualMethodAnalysisDebugLevel)
        leaves.push_back(cur.first);

        // The merged tree is inclusive since both the trees agreed on at least
        // one path from bases to a leaf
        inclusive = true;
      // If they're not NULL, move on to the children of the current nodes
      // by pushing them onto the worklist
      } else {
        SIGHT_VERB(dbg << "same non-NULL children"<<endl, 1, VirtualMethodAnalysisDebugLevel)
        worklist.push_back(make_pair(cur.first->child, cur.second->child));
      }

      // If the cur corresponds to a base, add it to bases
      if(cur.first->parents.size()==0)
        bases.push_back(cur.first);

    // Otherwise, cut the current tree at this node by replacing the node
    // with another one that has no children. We do the replacement instead
    // of modifying the node directly to keep the all instances of Node immutable.
    // This makes it possible for multiple nodes to share Node instances
    // without worrying about code in one tree corrupting another tree.
    } else {
      SIGHT_VERB(dbg << "different children, #cur.first->parents="<<cur.first->parents.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)
      // Create the new node
      NodePtr newLeaf = makePtr<Node>(cur.first->def);

      // Point all of cur.first's parents in the tree to newLeaf
      // (we know that we've already passed them in the loop)
      for(set<NodePtr>::iterator p=cur.first->parents.begin(); p!=cur.first->parents.end(); ++p) {
        NodePtr parent = *p;
        parent->child = newLeaf;
        newLeaf->parents.insert(*p);
      }

      // The new node is now a leaf of the merged tree
      leaves.push_back(newLeaf);

      // If the cur corresponds to a base, add newLeaf to bases
      if(cur.first->parents.size()==0)
        bases.push_back(newLeaf);

      // This tree was modified as a result of the merge
      modified = true;
    }
  }

  if(inclusive!=origInclusive) modified = true;

  SIGHT_VERB_IF(1, VirtualMethodAnalysisDebugLevel)
    dbg << "Leaves="<<endl;
    for(list<NodePtr>::iterator l=leaves.begin(); l!=leaves.end(); l++) {
      dbg << "    "<<SgNode2Str((*l)->def)<<endl;
    }
  SIGHT_VERB_FI()

  return modified;
}

// Computes the intersection of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
// The part of this object is to be used for AbstractObject comparisons.
// Meet update finds the common tree prefix among the two treeS
bool ClassInheritanceTree::intersectUpdate(Lattice* that_arg) {
  SIGHT_VERB(scope reg("ClassInheritanceTree::intersectUpdate()", scope::medium), 1, VirtualMethodAnalysisDebugLevel)
  ClassInheritanceTree* that = dynamic_cast<ClassInheritanceTree*>(that_arg);
  ROSE_ASSERT(that);

  SIGHT_VERB_IF(1, VirtualMethodAnalysisDebugLevel)
    dbg << "state="<<state2Str(state)<<", that->state="<<state2Str(that->state)<<endl;
    { scope s("this", scope::low); dbg << "this="<<str()<<endl; }
    { scope s("that", scope::low); dbg << "that="<<that->str()<<endl; }
  SIGHT_VERB_FI()
  // Empty Intersect * = Empty
  if(state==empty) return false;
  // If that tree is empty, make this tree empty as well
  if(that->state==empty) {
    if(state==empty) return false;
    else { setToEmpty(); return true; }
  }
  // Neither tree is full

  // If this tree is full, copy the state of that tree over this tree
  if(state==full) {
    if(that->state==full) return false;
    else { copyFrom(*that); return true; }
  }
  // If that is full, don't modify this
  if(that->state==empty) return false;

  // Both trees are neither full nor empty

  // If the two inheritance trees don't have the same base classes, they have
  // nothing in common, which makes this the empty hierarchy (no class hierarchy
  // satisfies all the constraints and thus the set of hierarchies denoted is empty)
  if(bases.size() != that->bases.size()) {
    SIGHT_VERB(dbg << "Inconsistent base sizes: setting to Empty. #bases="<<bases.size()<<", #that->bases="<<that->bases.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)
    setToEmpty();
    return true;
  }

  // Iterate from the bases to their children looking for the point in the
  // iteration where the trees stop being identical. If the trees are not identical but
  // one is a suffix of the other, then the intersection contains the suffix sub-tree is the intersection is equal to the suffix tree.
  // If they're not identical in other ways, their intersection is empty because
  // in that case they specify contradictory constraints on the class hierarchy.

  // The worklist contains a list of pairs, one node from each tree
  std::list<std::pair<NodePtr, NodePtr> > worklist;

  // Initialize the list with the base nodes
  for(std::list<NodePtr>::iterator thisB=bases.begin(), thatB=that->bases.begin();
      thisB!=bases.end(); ++thisB, ++thatB) {
    // If the two inheritance trees don't have the same base classes, they have
    // nothing in common, which makes this the empty hierarchy (over-constrained)
    if((*thisB)->def != (*thatB)->def) {
      SIGHT_VERB(dbg << "Inconsistent bases: setting to Empty. (*thisB)->def="<<(*thisB)->def<<"="<<SgNode2Str((*thisB)->def)<<" != (*thatB)->def="<<(*thatB)->def<<"="<<SgNode2Str((*thatB)->def)<<endl, 1, VirtualMethodAnalysisDebugLevel)
      setToFull();
      return true;
    }
    worklist.push_back(make_pair(*thisB, *thatB));
  }
  SIGHT_VERB(dbg << "#worklist="<<worklist.size()<<", #bases="<<bases.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)

  // Determine which case we're in:
  // - this == that
  // - this is a suffix of that: intersection == this
  // - that is a suffix of this: intersection == that
  // - other: intersection = empty

  // To discover this we iterate through both trees. Initially allEqual==true.
  // - If we see a difference where one sub-tree is different from the other, we set the
  //   intersection to empty and exit
  // - If we see a difference where one sub-tree is a suffix of another, and
  //   - If allEqual == true, we set allEqual=false, and set thisSuffixOfThat or
  //     thatSuffixOfThis to true, as appropriate.
  //   - Else, if the type of suffix disagrees with the current value of thisSuffixOfThat or
  //     thatSuffixOfThis, the trees are incompatible and we set their intersection
  //     to empty and exit
  // - If we see any other kind of difference, we set their intersection to empty and exit

  bool allEqual = true;
  bool thisSuffixOfThat = false;
  bool thatSuffixOfThis = false;

  // Keep iterating through the worklist as long as the node pairs
  // have identical SgClassDefinitions.
  while(worklist.size()>0) {
    SIGHT_VERB(scope reg("worklist", scope::low), 1, VirtualMethodAnalysisDebugLevel)
    SIGHT_VERB(dbg << "#worklist="<<worklist.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)

    pair<NodePtr, NodePtr> cur = worklist.front();
    worklist.pop_front();
    SIGHT_VERB(dbg << "cur=<"<<cur.first->str()<<","<<endl<<"    "<<cur.second->str()<<">"<<endl, 1, VirtualMethodAnalysisDebugLevel)

    // If the children of the current nodes on both trees are the same
    if(cur.first->child == cur.second->child) {
      // If they're not NULL, move on to the children of the current nodes
      // by pushing them onto the worklist
      if(cur.first->child) {
        SIGHT_VERB(dbg << "same non-NULL children"<<endl, 1, VirtualMethodAnalysisDebugLevel)
        worklist.push_back(make_pair(cur.first->child, cur.second->child));
      }
    // Otherwise, if that is a suffix of this (that is not NULL, while this is)
    } else if(!cur.first->child) {
      if(allEqual) {
        allEqual = false;
        assert(!thisSuffixOfThat && !thatSuffixOfThis);
        thatSuffixOfThis = true;
      } else if(thisSuffixOfThat) {
        setToEmpty();
        return true;
      }
    // Otherwise, if this is a suffix of that (this is not NULL, while that is)
    } else if(!cur.second->child) {
      if(allEqual) {
        allEqual = false;
        assert(!thisSuffixOfThat && !thatSuffixOfThis);
        thisSuffixOfThat = true;
      } else if(thisSuffixOfThat) {
        setToEmpty();
        return true;
      }
    // Otherwise, if the children are generally unequal, the intersection is empty
    } else {
      setToEmpty();
      return true;
    }
  }

  // If the trees are equal or this is a suffix of that, this is already the intersection of this and that
  if(allEqual || thisSuffixOfThat)
    return false;
  // Otherwise, copy that to this
  else {
    bases = that->bases;
    leaves = that->leaves;
    inclusive = that->inclusive;
    return true;
  }
}

// Returns the set of functions the given SgFunctionCallExp may refer to given
// the type constraints encoded in this tree
set<SgFunctionDeclaration*> ClassInheritanceTree::getCalleeDefs(SgFunctionCallExp* call) {
  SIGHT_VERB(scope s(txt()<<"ClassInheritanceTree::getCalleeDefs("<<SgNode2Str(call)<<")"), 1, VirtualMethodAnalysisDebugLevel)
  ClassHierarchyWrapper::ClassDefSet allSubClasses;
  // Add to allSubClasses all of the tree's leaf classes (if inclusive) and
  // all the classes that inherit from them
  for(list<NodePtr>::iterator l=leaves.begin(); l!=leaves.end(); l++) {
    if(inclusive) allSubClasses.insert((*l)->def);
    const ClassHierarchyWrapper::ClassDefSet& children = classHierarchy->getSubclasses((*l)->def);
    allSubClasses.insert(children.begin(), children.end());
  }

  SIGHT_VERB_IF(2, VirtualMethodAnalysisDebugLevel)
    scope s1("allSubClasses");
    for(ClassHierarchyWrapper::ClassDefSet::iterator sub=allSubClasses.begin(); sub!=allSubClasses.end(); sub++)
      dbg << "    "<<SgNode2Str(*sub)<<endl;
  SIGHT_VERB_FI()

  // Get all the functions that may possibly be referred to by call
  Rose_STL_Container<SgFunctionDeclaration*> approxCallees;
  CallTargetSet::getDeclarationsForExpression(call, classHierarchy, approxCallees);

  SIGHT_VERB_IF(2, VirtualMethodAnalysisDebugLevel)
    scope s1("approxCallees");
    for(Rose_STL_Container<SgFunctionDeclaration*>::iterator c=approxCallees.begin(); c!=approxCallees.end(); c++)
      dbg << "    "<<SgNode2Str(*c)<<endl;
  SIGHT_VERB_FI()

  // Copy to callees the functions in approxCallees that belong to some
  // class in allSubClasses
  set<SgFunctionDeclaration*> callees;
  for(Rose_STL_Container<SgFunctionDeclaration*>::iterator c=approxCallees.begin();
      c!=approxCallees.end(); c++) {
    // Check if the current callee is actually possible given the known class type constraints
    SgClassDefinition* scope = isSgClassDefinition((*c)->get_scope());
    SIGHT_VERB(dbg << "scope="<<SgNode2Str(scope)<<endl, 2, VirtualMethodAnalysisDebugLevel)
    ROSE_ASSERT(scope);
    if(allSubClasses.find(scope) != allSubClasses.end()) {
      SIGHT_VERB(dbg << "   Possible callee: "<<SgNode2Str(*c)<<endl, 2, VirtualMethodAnalysisDebugLevel)
      callees.insert(isSgFunctionDeclaration((*c)->get_definingDeclaration()));
    }
  }
  return callees;
}

bool ClassInheritanceTree::operator==(Lattice* that_arg) {
  ClassInheritanceTree* that = dynamic_cast<ClassInheritanceTree*>(that_arg);
  ROSE_ASSERT(that);

  // These trees are identical if their leaves lists contain the same SgClassDefinitions.
  // This is because the ancestry of a given SgClassDefinitions is the same and
  // acyclical no matter where it appears. Thus, even if the tree got truncated
  // because multiple trees were merged together and their un-shared leaves got
  // chopped, if the new leaves are the same, this means that all the deeper parts
  // of the tree (their ancestors) must be the same.
  // NOTE: this check assumes that the same leaf SgClassDefinitions appear in the
  //       same order in multiple ClassInheritanceTrees.
  if(leaves.size() != that->leaves.size()) return false;

  for(std::list<NodePtr>::iterator thisL=leaves.begin(), thatL=that->leaves.begin();
      thisL!=leaves.end(); ++thisL, ++thatL) {
    if((*thisL)->def != (*thatL)->def) return false;
  }

  if(inclusive != that->inclusive) return false;
  return true;
}

// Set this Lattice object to represent the set of all possible execution prefixes.
// Return true if this causes the object to change and false otherwise.
bool ClassInheritanceTree::setToFull() {
  if(state==full) return false;
  state = full;
  bases.clear();
  leaves.clear();
  inclusive = false;
  return true;
}

// Set this Lattice object to represent the of no execution prefixes (empty set).
// Return true if this causes the object to change and false otherwise.
bool ClassInheritanceTree::setToEmpty() {
  if(state==empty) return false;
  state = empty;
  bases.clear();
  leaves.clear();
  inclusive = false;
  return true;
}

// Set all the value information that this Lattice object associates with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
bool ClassInheritanceTree::setMLValueToFull(MemLocObjectPtr ml) {
  // Do nothing since this object does not maintain information at the granularity of memory locations
  return false;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool ClassInheritanceTree::isFull() { return state == full; }

// Returns whether this lattice denotes the empty set.
bool ClassInheritanceTree::isEmpty() { return state == empty; }

std::string ClassInheritanceTree::str(std::string indent) const {
  if(state == full)  return "[ClassInheritanceTree: FULL]";
  if(state == empty) return "[ClassInheritanceTree: EMPTY]";

  ostringstream s;
  s << "<u>ClassInheritanceTree (inclusive="<<inclusive<<") :</u>"<<endl;
  s << "<table border=1>"<<endl;
  // The worklist is a list of Nodes that have not yet been printed, along with
  // the indentation space to place before each one, which is based on its depth
  // in the tree.
  std::list<std::pair<NodePtr, std::string> > worklist;
  for(std::list<NodePtr>::const_iterator l=leaves.begin(); l!=leaves.end(); l++)
    worklist.push_back(make_pair(*l, "  "));

  while(worklist.size()>0) {
    // Get the next node in the tree
    std::pair<NodePtr, std::string> cur=worklist.front();
    worklist.pop_front();

    // Print it out with appropriate indentation
    s << "<tr><td>"<<cur.second << cur.first->str()<<"</td></tr>"<<endl;

    // Push its parents onto the front of the worklist so that they're printed
    // immediately after it, with deeper indentation.
    for(std::set<NodePtr>::iterator p=cur.first->parents.begin(); p!=cur.first->parents.end(); p++)
      worklist.push_front(make_pair(*p, cur.second+"    "));
  }
  s << "</table>"<<endl;

  s << "bases=";
  s << "<table border=1>"<<endl;
  for(list<NodePtr>::const_iterator b=bases.begin(); b!=bases.end(); b++)
    s << "<tr><td>"<<(*b)->str()<<"</td></tr>"<<endl;
  s << "</table>"<<endl;

  s << "leaves=";
  s << "<table border=1>"<<endl;
  for(list<NodePtr>::const_iterator l=leaves.begin(); l!=leaves.end(); l++)
    s << "<tr><td>"<<(*l)->str()<<"</td></tr>"<<endl;
  s << "</table>"<<endl;
  return s.str();
}

/******************************
 ***** MethodEntryExitMap *****
 ******************************/

MethodEntryExitMap::MethodEntryExitMap(PartEdgePtr latPEdge):
    Lattice(latPEdge),
    FiniteLattice(latPEdge),
    fullEntryPoints(false)
{ }

MethodEntryExitMap::MethodEntryExitMap(const MethodEntryExitMap& that) : Lattice(that), FiniteLattice(that) {
  entryPoints = that.entryPoints;
  fullEntryPoints = that.fullEntryPoints;
}

MethodEntryExitMap& MethodEntryExitMap::operator=(const MethodEntryExitMap& that) {
  entryPoints = that.entryPoints;
  fullEntryPoints = that.fullEntryPoints;
  return *this;
}

// returns a copy of this lattice
Lattice* MethodEntryExitMap::copy() const
{
  return new MethodEntryExitMap(*this);
}

// Adds a new entry point for the given function. Returns true if this causes this
// object to change and false otherwise
bool MethodEntryExitMap::add(const Function& func, PartEdgePtr entry) {
  map<Function, set<PartEdgePtr> >::iterator funcLoc = entryPoints.find(func);
  // If we don't have any entry points currently recorded for this function
  if(funcLoc == entryPoints.end()) {
    entryPoints[func].insert(entry);
    return true;
  // If we already have some points recorded, add the new entry point
  } else {
    pair<set<PartEdgePtr>::iterator,bool> ret = funcLoc->second.insert(entry);
    return ret.second;
  }
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
// The part of this object is to be used for AbstractObject comparisons.
// meetUpdate finds the common tree prefix among the two treeS
bool MethodEntryExitMap::meetUpdate(Lattice* that_arg) {
  MethodEntryExitMap* that = dynamic_cast<MethodEntryExitMap*>(that_arg);
  assert(that);

/*  cout << "meetUpdate"<<endl;
  cout << "    this="<<str("    ")<<endl;
  cout << "    that="<<that->str("    ")<<endl;*/

  // If that is full, make this full as well
  if(that->isFull()) return setToFull();
  bool modified = false;

  // Iterate over all the functions in that.entryPoints
  for(map<Function, set<PartEdgePtr> >::iterator i=that->entryPoints.begin(); i!=that->entryPoints.end(); ++i) {
    // Add all the entry points for the current function in that->entryPoints under the same
    // function in this->entryPoints.
    for(set<PartEdgePtr>::iterator entry=i->second.begin(); entry!=i->second.end(); ++entry) {
      //cout << "        adding "<<SgNode2Str(i->first.get_declaration())<<": "<<(*entry)->str()<<endl;

      pair<set<PartEdgePtr>::iterator, bool> ret = entryPoints[i->first].insert(*entry);
      modified = modified || ret.second;
    }
  }

  return modified;
}

// Computes the intersection of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
// The part of this object is to be used for AbstractObject comparisons.
// intersectUpdate finds the common tree prefix among the two treeS
bool MethodEntryExitMap::intersectUpdate(Lattice* that_arg)  {
  MethodEntryExitMap* that = dynamic_cast<MethodEntryExitMap*>(that_arg);
  assert(that);

  // If that is empty, make this empty as well
  if(that->isEmpty()) return setToEmpty();

  bool modified = false;

  // Iterate over all the functions in this->entryPoints
  for(map<Function, set<PartEdgePtr> >::iterator i=entryPoints.begin(); i!=entryPoints.end(); ) {
    // If the current entry in this->entryPoints doesn't exist in that->entryPoints, remove it
    if(that->entryPoints.find(i->first) == that->entryPoints.end())
      entryPoints.erase(i++);
    // Otherwise, keep it
    else {
      // Iterate over all the entry points in i->second.
      for(set<PartEdgePtr>::iterator entry=i->second.begin(); entry!=i->second.end(); ) {
        // If the current entry point in this->entryPoints doesn't exist in that->entryPoints, remove it
        if(that->entryPoints[i->first].find(*entry) == that->entryPoints[i->first].end()) {
          i->second.erase(entry++);
          modified = true;
        // Otherwise, keep it and advance to the next entry point
        } else
          ++entry;
      }

      // If the current Function in this->entryPoints has no more entry points, remove it
      if(i->second.size()==0) {
        entryPoints.erase(i++);
        modified = true;
      // Otherwise, leave it alone and advance to the next Function
      } else
        ++i;
    }
  }

  return modified;
}

bool MethodEntryExitMap::operator==(Lattice* that_arg) {
  MethodEntryExitMap* that = dynamic_cast<MethodEntryExitMap*>(that_arg);
  assert(that);

  return fullEntryPoints == that->fullEntryPoints &&
         entryPoints     == that->entryPoints;
}

// Set this Lattice object to represent the set of all possible execution prefixes.
// Return true if this causes the object to change and false otherwise.
bool MethodEntryExitMap::setToFull() {
  if(fullEntryPoints) return false;

  fullEntryPoints = true;
  entryPoints.clear();
  return true;
}

// Set this Lattice object to represent the of no execution prefixes (empty set).
// Return true if this causes the object to change and false otherwise.
bool MethodEntryExitMap::setToEmpty() {
  bool modified = (entryPoints.size()!=0 || !fullEntryPoints);
  entryPoints.size();
  fullEntryPoints = false;
  return modified;
}

// Set all the value information that this Lattice object associates with this MemLocObjectPtr to full.
// Return true if this causes the object to change and false otherwise.
bool MethodEntryExitMap::setMLValueToFull(MemLocObjectPtr ml) {
  return false;
}

// Returns whether this lattice denotes the set of all possible execution prefixes.
bool MethodEntryExitMap::isFull() {
  return fullEntryPoints;
}

// Returns whether this lattice denotes the empty set.
bool MethodEntryExitMap::isEmpty() {
  return !fullEntryPoints && entryPoints.size()==0;
}

std::string MethodEntryExitMap::str(std::string indent) const {
  ostringstream oss;

  oss << "[MethodEntryExitMap: ";
  if(fullEntryPoints) oss << "FULL";
  else {
    oss << endl;
    for(map<Function, set<PartEdgePtr> >::const_iterator i=entryPoints.begin(); i!=entryPoints.end(); ++i) {
      oss << indent << "    "<< i->first.str()<<endl;
      for(set<PartEdgePtr>::const_iterator entry=i->second.begin(); entry!=i->second.end(); ++entry) {
        oss << indent << "        "<<(*entry)->str()<<endl;
      }
    }
  }

  oss << "]";

  return oss.str();
}

/****************************
 ***** VirtualMethodPart *****
 ****************************/

VirtualMethodPart::VirtualMethodPart(PartPtr base, ComposedAnalysis* analysis) :
  Part(analysis, base)
{
  cacheInitialized_outEdges=false;
  cacheInitialized_inEdges=false;
  cacheInitialized_CFGNodes=false;
  cacheInitialized_matchingCallParts=false;
  cacheInitialized_matchingEntryExitParts=false;
  cacheInitialized_inEdgeFromAny=false;
  cacheInitialized_outEdgeToAny=false;
}

VirtualMethodPart::VirtualMethodPart(const VirtualMethodPart& that) :
  Part((const Part&)that)
{
  cacheInitialized_outEdges               = that.cacheInitialized_outEdges;
  cache_outEdges                          = that.cache_outEdges;
  cacheInitialized_inEdges                = that.cacheInitialized_inEdges;
  cache_inEdges                           = that.cache_inEdges;
  cacheInitialized_CFGNodes               = that.cacheInitialized_CFGNodes;
  cache_CFGNodes                          = that.cache_CFGNodes;
  cacheInitialized_matchingCallParts      = that.cacheInitialized_matchingCallParts;
  cache_matchingCallParts                 = that.cache_matchingCallParts;
  cacheInitialized_matchingEntryExitParts = that.cacheInitialized_matchingEntryExitParts;
  cache_matchingEntryExitParts            = that.cache_matchingEntryExitParts;
  cacheInitialized_inEdgeFromAny          = that.cacheInitialized_inEdgeFromAny;
  cache_inEdgeFromAny                     = that.cache_inEdgeFromAny;
  cacheInitialized_outEdgeToAny           = that.cacheInitialized_outEdgeToAny;
  cache_outEdgeToAny                      = that.cache_outEdgeToAny;
  cache_equal                             = that.cache_equal;
  cache_less                              = that.cache_less;
}

// Returns a shared pointer to this of type VirtualMethodPartPtr
VirtualMethodPartPtr VirtualMethodPart::get_shared_this()
{ return dynamicPtrCast<VirtualMethodPart>(makePtrFromThis(shared_from_this())); }

// -------------------------------------------
// Functions that need to be defined for Parts
// -------------------------------------------

// Returns whether the given function call according to the available information
// about object types
bool VirtualMethodPart::isPossibleFunctionCall(const Function& calleeFunc, SgFunctionCallExp* call,
                                               AbstractObjectMap* aom/*, PartEdgePtr NodeStatePartEdge*/) {
  /*dbg << "call->get_function()="<<SgNode2Str(call->get_function())<<endl;
  if(isSgArrowExp(call->get_function())) {
    dbg << "isSgArrowExp(call->get_function())->get_lhs_operand()="<<SgNode2Str(isSgArrowExp(call->get_function())->get_lhs_operand())<<endl;
    dbg << "isSgArrowExp(call->get_function())->get_lhs_operand()="<<SgNode2Str(unwrapCasts(isSgArrowExp(call->get_function())->get_lhs_operand()))<<endl;
  }*/
  // If this is a regular function call or a call on the current object
  if(isSgFunctionRefExp(call->get_function()) ||
     (isSgArrowExp(call->get_function()) && isSgThisExp(unwrapCasts(isSgArrowExp(call->get_function())->get_lhs_operand())))) {
    SIGHT_VERB(dbg << "regular call. Adding."<<endl, 2, VirtualMethodAnalysisDebugLevel)
    // This is not a virtual call, so we conservatively say it is possible since
    // type information says nothing about this call
    return true;
  // If this function call is done through a variable (E.g. var.foo())
  } else {
    //MemLocObjectPtr ml = analysis->getComposer()->Expr2MemLoc(call->get_function(), getNodeStateLocPart()->inEdgeFromAny(), analysis);
    SIGHT_VERB(dbg << "call through expression "<<SgNode2Str(call->get_function())<<endl, 2, VirtualMethodAnalysisDebugLevel)
    MemLocObjectPtr callObj;
    // Get the MemLoc that denotes the object a method of which is being called
    if(SgDotExp* dotCall = isSgDotExp(call->get_function()))
      callObj = analysis->getComposer()->Expr2MemLoc(dotCall->get_lhs_operand(), getNodeStateLocPart()->inEdgeFromAny(), analysis);
    else if(SgArrowExp* arrowCall = isSgArrowExp(call->get_function()))
      // Currently Fuse doesn't provide a way to create new expressions and run queries against them
      // (in this case SgPointerDerefExp(call->get_function())) but this will be done in the future.
      assert(false);
    else
      assert(false);

    SIGHT_VERB(dbg << "calling through MemLoc "<<callObj->str()<<endl, 2, VirtualMethodAnalysisDebugLevel)
    //{ scope s("key"); dbg << dynamic_pointer_cast<AbstractionHierarchy>(callObj)->getHierKey()->str()<<endl;}
    //{ scope s(txt()<<"aom ("<<aom<<")"); dbg << aom->str()<<endl; }

    ClassInheritanceTreePtr tree = dynamic_pointer_cast<ClassInheritanceTree>(aom->get(callObj));
    SIGHT_VERB(dbg << "tree at variable="<<endl<<tree->str()<<endl, 2, VirtualMethodAnalysisDebugLevel)
    ROSE_ASSERT(tree);
    set<SgFunctionDeclaration*> callees = tree->getCalleeDefs(call);
    /*{ scope s("Checking for possibility");
    dbg << "calleeFunc.get_declaration()="<<calleeFunc.get_declaration()<<"="<<SgNode2Str(calleeFunc.get_declaration())<<endl;
    dbg << "calleeFunc.get_declaration()->get_definingDeclaration()="<<calleeFunc.get_declaration()->get_definingDeclaration()<<"="<<SgNode2Str(calleeFunc.get_declaration()->get_definingDeclaration())<<endl;
    dbg << "callees="<<endl;
    for(set<SgFunctionDeclaration*>::iterator c=callees.begin(); c!=callees.end(); c++) {
      dbg << "    base callee:"<<(*c)<<"="<<SgNode2Str(*c)<<endl;
      dbg << "    definingcallee: "<<(*c)->get_definingDeclaration()<<"="<<SgNode2Str((*c)->get_definingDeclaration())<<endl;
    }
    dbg << "found="<<(callees.find(calleeFunc.get_declaration()) != callees.end())<<endl;*/
    // This call is possible if the callee function is a member of callees
    if(callees.find(calleeFunc.get_definingDeclaration()) != callees.end()) {
      SIGHT_VERB(dbg << "call edge is possible. Adding."<<endl, 2, VirtualMethodAnalysisDebugLevel)
      return true;
    }
  }

  SIGHT_VERB(dbg << "call edge NOT is possible. Omitting."<<endl, 2, VirtualMethodAnalysisDebugLevel)
  return false;
}

// Given baseEdges, a list of edges from the server analysis' ATS, set cache_Edges to contain the edges in
// the VMAnalysis' ATS that wrap them
void VirtualMethodPart::wrapEdges(std::list<PartEdgePtr>& cache_Edges, const AnalysisPartEdgeLists& baseEdges)
{
  SIGHT_VERB_DECL(scope, (txt() << "wrapEdges() #baseEdges="<<baseEdges.size()), 2, VirtualMethodAnalysisDebugLevel)

  // Consider all the VirtualMethodParts along all of this part's outgoing edges. Since this is a forward
  // analysis, they are maintained separately
  for(AnalysisPartEdgeLists::const_iterator edge=baseEdges.begin(); edge!=baseEdges.end(); ++edge) {
    SIGHT_VERB_DECL(scope, (txt()<<"edge="<<edge->input()->str(), scope::low), 2, VirtualMethodAnalysisDebugLevel)

    // If this edge enters a function
    set<CFGNode> matchNodes;
    if(edge->input()->source()->mustOutgoingFuncCall(matchNodes)) {
      // The entry point of the called function
      SgFunctionParameterList* calleeEntry = edge->input()->target()->mustSgNodeAll<SgFunctionParameterList>();
      ROSE_ASSERT(calleeEntry);
      // The descriptor of the called function
      Function curCalleeFunc(SageInterface::getEnclosingFunctionDeclaration(calleeEntry));
      SIGHT_VERB(dbg << "edge is call to function "<<curCalleeFunc.str()<<endl, 2, VirtualMethodAnalysisDebugLevel)

      NodeState* callPartState = NodeState::getNodeState(analysis, /*edge->NodeState()->source()*/getNodeStateLocPart());
      dbg << "callPartState="<<callPartState->str()<<endl;
      AbstractObjectMap* curPartAOM =
          dynamic_cast<AbstractObjectMap*>(
              callPartState->getLatticeBelow(analysis, edge->index(), 0));
      ROSE_ASSERT(curPartAOM);

      MethodEntryExitMap* curPartMEEMap =
          dynamic_cast<MethodEntryExitMap*>(
              callPartState->getLatticeBelow(analysis, edge->index(), 1));
      ROSE_ASSERT(curPartMEEMap);

      //dbg << "edge->input()->source()="<<edge->input()->source()->src()<<endl;
      set<CFGNode> nodes=edge->input()->source()->CFGNodes();
      //dbg << "#nodes="<<nodes.size()<<endl;
      for(set<CFGNode>::iterator n=nodes.begin(); n!=nodes.end(); n++) {
        SIGHT_VERB_DECL(scope, (txt()<<"    n="<<CFGNode2Str(*n)), 2, VirtualMethodAnalysisDebugLevel)
        SgFunctionCallExp* call = isSgFunctionCallExp(n->getNode());
        if(call && n->getIndex()==2 && isPossibleFunctionCall(curCalleeFunc, call, curPartAOM/*, edge->NodeState()*/)) {
          dbg << "This is a possible function call."<<endl;
          PartEdgePtr entryEdge = VirtualMethodPartEdge::create(edge->input(), analysis);
          cache_Edges.push_back(entryEdge);

          // Record this entry point to the current function
          curPartMEEMap->add(curCalleeFunc, entryEdge);
        }
      }

      dbg << "AFTER: callPartState="<<callPartState->str()<<endl;

    // If this edge exits a function
    } else if(edge->input()->target()->mustIncomingFuncCall(matchNodes)) {
      // The definition of the function being exited
      SgFunctionDefinition* calleeExit = edge->input()->source()->mustSgNodeAll<SgFunctionDefinition>();
      ROSE_ASSERT(calleeExit);
      // The descriptor of the function being exited
      Function curCalleeFunc(calleeExit);
      SIGHT_VERB(dbg << "edge is an exit from function "<<curCalleeFunc.str()<<endl, 2, VirtualMethodAnalysisDebugLevel);

      // The parts that denote the entry point into this function
      set<PartPtr> entryPartsNodeState = getNodeStateLocPart()->matchingEntryExitParts();

      // The parts within the input and index ATSs that denote the call site of the function being exited
      set<PartPtr> callPartsInput = edge->input()->target()->matchingCallParts();
      assert(callPartsInput.size()==1);
      SIGHT_VERB_IF(2, VirtualMethodAnalysisDebugLevel)
      scope s("callPartsInput");
      for(set<PartPtr>::iterator p=callPartsInput.begin(); p!=callPartsInput.end(); ++p)
        dbg << (*p)->str()<<endl;
      SIGHT_VERB_FI()
      std::set<PartPtr> callPartsIndex = edge->index()->target()->matchingCallParts();
      assert(callPartsIndex.size()==1);

      // Determine if any of the incoming edges to any part in entryPartsNodeState are possible function calls
      bool anyPossible=false;
      for(set<PartPtr>::iterator entryP=entryPartsNodeState.begin(); entryP!=entryPartsNodeState.end(); ++entryP) {
        SIGHT_VERB_DECL(scope, (txt()<< "entryP="<<(*entryP)->str()), 2, VirtualMethodAnalysisDebugLevel);
        // Iterate through all the incoming edges of the current entry part to find the ones that correspond to
        // an edge in callPartsInput (callPartsInput edges are from the input ATS and these incom
        // edges are from the NodeState ATS)
        list<PartEdgePtr> inE = (*entryP)->inEdges();
        for(list<PartEdgePtr>::iterator inNodeStateEdge=inE.begin(); inNodeStateEdge!=inE.end() && !anyPossible; ++inNodeStateEdge) {
          SIGHT_VERB_DECL(scope, (txt()<< "inNodeStateEdge="<<(*inNodeStateEdge)->str()), 2, VirtualMethodAnalysisDebugLevel);
          // If the source of input edge of the current incoming NodeState edge corresponds to a call entry
          // that matches the current call exit.
          if(callPartsInput.find((*inNodeStateEdge)->getInputPartEdge()->source()) != callPartsInput.end()) {
            SIGHT_VERB(dbg << "Edge found in callPartsInput"<<endl, 2, VirtualMethodAnalysisDebugLevel);
            // Look up the analysis results at the function entry NodeState Part

            // The NodeState at the call entry part of this call exit edge
            NodeState* callPartState = NodeState::getNodeState(analysis, (*inNodeStateEdge)->source());
            //dbg << "callPartState="<<callPartState->str(analysis)<<endl;
            SIGHT_VERB(dbg << "callPartState="<<callPartState->str(analysis)<<endl, 2, VirtualMethodAnalysisDebugLevel)
            AbstractObjectMap* callPartAOM =
                dynamic_cast<AbstractObjectMap*>(
                    callPartState->getLatticeAbove(analysis, (*callPartsIndex.begin())->inEdgeFromAny(), 0));
            ROSE_ASSERT(callPartAOM);

            // Iterate over all the SgNodes that correspond to the SgFunctionCallExps at the current call
            // and use the callPartAOM to determine whether any of them correspond to a valid function call.
            set<CFGNode> nodes=edge->input()->target()->CFGNodes();
            for(set<CFGNode>::iterator n=nodes.begin(); n!=nodes.end() && !anyPossible; n++) {
              SIGHT_VERB(dbg << "    n="<<CFGNode2Str(*n)<<endl, 2, VirtualMethodAnalysisDebugLevel)
              SgFunctionCallExp* call = isSgFunctionCallExp(n->getNode());
              // If this exit corresponds to a possible function call according to the VMAnalysis, add it to the cache_Edges
              if(call && n->getIndex()==3 && isPossibleFunctionCall(curCalleeFunc, call, callPartAOM/*, edge->NodeState()*/)) {
                anyPossible = true;
              }
            }
          } else {
            SIGHT_VERB(dbg << "Edge NOT found in callPartsInput"<<endl, 2, VirtualMethodAnalysisDebugLevel);
          }
        }
      }

      // If this exit corresponds to a possible function call according to the VMAnalysis, add it to the cache_Edges
      if(anyPossible)
        cache_Edges.push_back(VirtualMethodPartEdge::create(edge->input(), analysis));

    // Not a function call, so we leave this edge as it is
    } else
      cache_Edges.push_back(VirtualMethodPartEdge::create(edge->input(), analysis));
  }

  SIGHT_VERB_IF(2, VirtualMethodAnalysisDebugLevel)
  scope s("cache_Edges");
  for(list<PartEdgePtr>::iterator e=cache_Edges.begin(); e!=cache_Edges.end(); e++)
    dbg << "    "<<(*e)->str()<<endl;
  SIGHT_VERB_FI()
}

list<PartEdgePtr> VirtualMethodPart::outEdges()
{
/*Need to re-evaluate outEdges under tight composition*/
    cache_outEdges.clear();
/*Need to re-evaluate outEdges under tight composition* /
  if(!cacheInitialized_outEdges) {*/
    SIGHT_VERB_DECL(scope, (txt()<<"VirtualMethodPart::outEdges()", scope::medium), 2, VirtualMethodAnalysisDebugLevel)
    AnalysisPartEdgeLists baseEdges =
        getAnalysis()->NodeState2All(getNodeStateLocPart()).
                            outEdgesIndexInput(*getAnalysis()->getComposer());// /*NodeState_valid*/ false, /*index_valid*/ true, /*input_valid*/ true);

    dbg << "baseEdges="<<baseEdges.str()<<endl;
    wrapEdges(cache_outEdges, baseEdges);

/*Need to re-evaluate outEdges under tight composition* /
    cacheInitialized_outEdges=true;
  }*/

  SIGHT_VERB_IF(2, VirtualMethodAnalysisDebugLevel)
  { scope s("outEdges() cache_outEdges");
  for(list<PartEdgePtr>::iterator e=cache_outEdges.begin(); e!=cache_outEdges.end(); e++)
    dbg << "    "<<(*e)->str()<<endl;}
  SIGHT_VERB_FI()

  return cache_outEdges;
}

list<PartEdgePtr> VirtualMethodPart::inEdges()
{
  if(!cacheInitialized_inEdges) {
    SIGHT_VERB_DECL(scope, (txt()<<"VirtualMethodPart::inEdges()", scope::medium), 2, VirtualMethodAnalysisDebugLevel)
    AnalysisPartEdgeLists baseEdges =
        getAnalysis()->NodeState2All(getNodeStateLocPart()).
                            inEdgesIndexInput(*getAnalysis()->getComposer());// /*NodeState_valid*/ false, /*index_valid*/ true, /*input_valid*/ true);

    wrapEdges(cache_inEdges, baseEdges);

    cacheInitialized_inEdges=true;
  }

  SIGHT_VERB_IF(2, VirtualMethodAnalysisDebugLevel)
  scope s("inEdges() cache_inEdges");
  for(list<PartEdgePtr>::iterator e=cache_inEdges.begin(); e!=cache_inEdges.end(); e++)
    dbg << "    "<<(*e)->str()<<endl;
  SIGHT_VERB_FI()


  return cache_inEdges;
}

set<CFGNode> VirtualMethodPart::CFGNodes() const
{
  if(!cacheInitialized_CFGNodes) {
    const_cast<VirtualMethodPart*>(this)->cache_CFGNodes = getInputPart()->CFGNodes();
    const_cast<VirtualMethodPart*>(this)->cacheInitialized_CFGNodes = true;
  }
  return cache_CFGNodes;
}

// If this Part corresponds to a function call/return, returns the set of Parts that contain
// its corresponding return/call, respectively.
set<PartPtr> VirtualMethodPart::matchingCallParts() const {
  if(!cacheInitialized_matchingCallParts) {
    // Wrap the parts returned by the call to the parent Part with VirtualMethodPart
    set<PartPtr> parentMatchParts = getInputPart()->matchingCallParts();
    for(set<PartPtr>::iterator mp=parentMatchParts.begin(); mp!=parentMatchParts.end(); mp++) {
      const_cast<VirtualMethodPart*>(this)->cache_matchingCallParts.insert(VirtualMethodPart::create(*mp, analysis));
    }
    const_cast<VirtualMethodPart*>(this)->cacheInitialized_matchingCallParts=true;
  }
  return cache_matchingCallParts;
}

// If this Part corresponds to a function entry/exit, returns the set of Parts that contain
// its corresponding exit/entry, respectively.
set<PartPtr> VirtualMethodPart::matchingEntryExitParts() const {
  if(!cacheInitialized_matchingEntryExitParts) {
    // Wrap the parts returned by the call to the parent Part with VirtualMethodPart
    set<PartPtr> parentMatchParts = getInputPart()->matchingEntryExitParts();
    for(set<PartPtr>::iterator mp=parentMatchParts.begin(); mp!=parentMatchParts.end(); mp++) {
      const_cast<VirtualMethodPart*>(this)->cache_matchingEntryExitParts.insert(VirtualMethodPart::create(*mp, analysis));
    }
    const_cast<VirtualMethodPart*>(this)->cacheInitialized_matchingEntryExitParts=true;
  }
  return cache_matchingEntryExitParts;
}


// Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
PartEdgePtr VirtualMethodPart::inEdgeFromAny() {
  if(!cacheInitialized_inEdgeFromAny) {
    cache_inEdgeFromAny = VirtualMethodPartEdge::create(getInputPart()->inEdgeFromAny(), analysis);
    cacheInitialized_inEdgeFromAny=true;
  }
  return cache_inEdgeFromAny;
}

// Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
PartEdgePtr VirtualMethodPart::outEdgeToAny() {
  if(!cacheInitialized_outEdgeToAny) {
    cache_outEdgeToAny = VirtualMethodPartEdge::create(getInputPart()->outEdgeToAny(), analysis);
    cacheInitialized_outEdgeToAny=true;
  }
  return cache_outEdgeToAny;
}

bool VirtualMethodPart::equal(const PartPtr& o) const
{
  const VirtualMethodPartPtr that = dynamicConstPtrCast<VirtualMethodPart>(o);
  assert(that.get());
  assert(analysis == that->analysis);

  /*if(cache_equal.find(that.get()) == cache_equal.end())
    const_cast<VirtualMethodPart*>(this)->cache_equal[that.get()] = (getInputPart() == that->getInputPart());
  return const_cast<VirtualMethodPart*>(this)->cache_equal[that.get()];*/
/*  cout << "          VirtualMethodPart::equal()"<<endl;
  cout << "              this->input="<<getInputPart()->str()<<endl;
  cout << "              that->input="<<that->getInputPart()->str()<<endl;
  cout << "              eq = "<<(getInputPart() == that->getInputPart())<<endl;*/
  return getInputPart() == that->getInputPart();
}

bool VirtualMethodPart::less(const PartPtr& o) const
{
  const VirtualMethodPartPtr that = dynamicConstPtrCast<VirtualMethodPart>(o);
  assert(that.get());
  assert(analysis == that->analysis);

  /*if(cache_less.find(that.get()) == cache_less.end())
    const_cast<VirtualMethodPart*>(this)->cache_less[that.get()] = (getInputPart() < that->getInputPart());
  return const_cast<VirtualMethodPart*>(this)->cache_less[that.get()];*/
  return getInputPart() < that->getInputPart();
}

// Pretty print for the object
std::string VirtualMethodPart::str(std::string indent) const
{
  ostringstream oss;
  oss << "[VMPart: "<<getInputPart()->str()<<"]";
  return oss.str();
}

/************************************
 ***** VirtualMethodPartEdge *****
 ************************************/

// Constructor to be used when traversing the part graph created by the VirtualMethodAnalysis, after
// all the VirtualMethodPartEdges have been constructed and stored in NodeStates.
VirtualMethodPartEdge::VirtualMethodPartEdge(PartEdgePtr baseEdge, ComposedAnalysis* analysis) :
        PartEdge(analysis, baseEdge)
{
  // Look up this edge in the results of the VirtualMethodAnalysis results and copy data from that edge into this object

  if(baseEdge->source()) src=VirtualMethodPart::create(baseEdge->source(), analysis);
  if(baseEdge->target()) tgt=VirtualMethodPart::create(baseEdge->target(), analysis);

  cacheInitialized_getPredicateValue=false;
}

VirtualMethodPartEdge::VirtualMethodPartEdge(const VirtualMethodPartEdge& that) :
  PartEdge((const PartEdge&)that),
  src(that.src), tgt(that.tgt)
{
  cache_getOperandPartEdge           = that.cache_getOperandPartEdge;
  cacheInitialized_getPredicateValue = that.cacheInitialized_getPredicateValue;
}

// Returns a shared pointer to this of type VirtualMethodPartEdgePtr
VirtualMethodPartEdgePtr VirtualMethodPartEdge::get_shared_this()
{ return dynamicPtrCast<VirtualMethodPartEdge>(makePtrFromThis(shared_from_this())); }


PartPtr VirtualMethodPartEdge::source() const
{ return src; }

PartPtr VirtualMethodPartEdge::target() const
{ return tgt; }

// Sets this PartEdge's parent
void VirtualMethodPartEdge::setInputPartEdge(PartEdgePtr parent)
{
  PartEdge::setInputPartEdge(parent);
}

// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    PartEdge that denotes a subset of A (the function is called on this PartEdge),
//    it returns a list of PartEdges that partition O.
std::list<PartEdgePtr> VirtualMethodPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand)
{
  if(cache_getOperandPartEdge.find(anchor) == cache_getOperandPartEdge.end() ||
     cache_getOperandPartEdge[anchor].find(operand) == cache_getOperandPartEdge[anchor].end()) {

    // operand precedes anchor in the CFG, either immediately or at some distance. As such, the edge
    //   we're looking for is not necessarily the edge from operand to anchor but rather the first
    //   edge along the path from operand to anchor. Since operand is part of anchor's expression
    //   tree we're guaranteed that there is only one such path.
    // The implementor of the partition we're running on may have created multiple parts for
    //   operand to provide path sensitivity and indeed, may have created additional outgoing edges
    //   from each of the operand's parts. Fortunately, since in the original program the original
    //   edge led from operand to anchor and the implementor of the partition could have only hierarchically
    //   refined the original partition, all the new edges must also lead from operand to anchor.
    //   As such, the returned list contains all the outgoing edges from all the parts that correspond
    //   to operand.
    // Note: if the partitioning process is not hierarchical we may run into minor trouble since the
    //   new edges from operand may lead to parts other than anchor. However, this is just an issue
    //   of precision since we'll account for paths that are actually infeasible.

    // The target of this edge identifies the termination point of all the execution prefixes
    // denoted by this edge. We thus use it to query for the parts of the operands and only both
    // if this part is itself live.
    SIGHT_VERB(scope reg("VirtualMethodPartEdge::getOperandPartEdge()", scope::medium), 1, VirtualMethodAnalysisDebugLevel)
    SIGHT_VERB(dbg << "anchor="<<SgNode2Str(anchor)<<" operand="<<SgNode2Str(operand)<<endl, 1, VirtualMethodAnalysisDebugLevel)

    std::list<PartEdgePtr> baseEdges = getInputPartEdge()->getOperandPartEdge(anchor, operand);
    for(std::list<PartEdgePtr>::iterator edge=baseEdges.begin(); edge!=baseEdges.end(); ++edge) {
      SIGHT_VERB(dbg << "edge="<<edge->str()<<endl, 1, VirtualMethodAnalysisDebugLevel)
      PartEdgePtr dpeEdge = VirtualMethodPartEdge::create(*edge, analysis);
      SIGHT_VERB(scope reg("vmEdge", scope::low), 2, VirtualMethodAnalysisDebugLevel)
      SIGHT_VERB(dbg<<dpeEdge->str()<<endl, 1, VirtualMethodAnalysisDebugLevel)
      cache_getOperandPartEdge[anchor][operand].push_back(dpeEdge);
    }
  }
  return cache_getOperandPartEdge[anchor][operand];
}

// If the source Part corresponds to a conditional of some sort (if, switch, while test, etc.)
// it must evaluate some predicate and depending on its value continue, execution along one of the
// outgoing edges. The value associated with each outgoing edge is fixed and known statically.
// getPredicateValue() returns the value associated with this particular edge. Since a single
// Part may correspond to multiple CFGNodes getPredicateValue() returns a map from each CFG node
// within its source part that corresponds to a conditional to the value of its predicate along
// this edge.
std::map<CFGNode, boost::shared_ptr<SgValueExp> > VirtualMethodPartEdge::getPredicateValue()
{
  if(!cacheInitialized_getPredicateValue) {
    cache_getPredicateValue = getInputPartEdge()->getPredicateValue();
    cacheInitialized_getPredicateValue = true;
  }
  return cache_getPredicateValue;
  //return latPEdge->getPredicateValue();
}

// Adds a mapping from a CFGNode to the outcome of its predicate
void VirtualMethodPartEdge::mapPred2Val(CFGNode n, boost::shared_ptr<SgValueExp> val)
{
  predVals[n] = val;
}

// Empties out the mapping of CFGNodes to the outcomes of their predicates
void VirtualMethodPartEdge::clearPred2Val()
{
  predVals.clear();
}

bool VirtualMethodPartEdge::equal(const PartEdgePtr& o) const
{
  const VirtualMethodPartEdgePtr that = dynamicConstPtrCast<VirtualMethodPartEdge>(o);
  assert(that.get());
  return src==that->src && tgt==that->tgt;
}

bool VirtualMethodPartEdge::less(const PartEdgePtr& o)  const
{
  const VirtualMethodPartEdgePtr that = dynamicConstPtrCast<VirtualMethodPartEdge>(o);
  assert(that.get());

  return (src < that->src) ||
         (src==that->src && tgt<that->tgt);
}

// Pretty print for the object
std::string VirtualMethodPartEdge::str(std::string indent) const
{
  ostringstream oss;
  oss << "[VMPEdge: "<<
                      (src ? src->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;"): "NULL")<<" ==&gt; " <<
                      (tgt ? tgt->str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;"): "NULL");/*<<
                      ", "<<endl;
  oss << indent <<", parent=<"<<getSupersetPartEdge()->str()*/
  oss <<"]";
  return oss.str();
}

/*********************************
 ***** VirtualMethodAnalysis *****
 *********************************/

VirtualMethodAnalysis::VirtualMethodAnalysis(bool trackBase2RefinedPartEdgeMapping) :
      FWDataflow(trackBase2RefinedPartEdgeMapping, /*useSSA*/ false) {
  cacheInitialized_GetStartAStates_Spec = false;
  cacheInitialized_GetEndAStates_Spec = false;
}

void VirtualMethodAnalysis::genInitLattice(const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                                      std::vector<Lattice*>& initLattices)
{
  AbstractObjectMap* productlattice = new AbstractObjectMap(boost::make_shared<ClassInheritanceTree>(pedges.NodeState()),
                                                            pedges.NodeState(),
                                                            getComposer(),
                                                            this);
  initLattices.push_back(productlattice);

  initLattices.push_back(new MethodEntryExitMap(pedges.NodeState()));
}


boost::shared_ptr<DFTransferVisitor>
VirtualMethodAnalysis::getTransferVisitor(AnalysisParts& parts, CFGNode cn, NodeState& state,
                                          std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)
{
  VirtualMethodAnalysisTransfer* ptat = new VirtualMethodAnalysisTransfer(parts, cn, state, dfInfo, getComposer(), this);
  return boost::shared_ptr<DFTransferVisitor>(ptat);
}

// Return the anchor Parts of the application
set<PartPtr> VirtualMethodAnalysis::GetStartAStates_Spec()
{
  SIGHT_VERB(scope reg("VirtualMethodAnalysis::GetStartAStates_Spec()", scope::medium), 1, VirtualMethodAnalysisDebugLevel)
  if(!cacheInitialized_GetStartAStates_Spec) {
    set<PartPtr> baseStartParts = getComposer()->GetStartAStates(this);
    SIGHT_VERB(dbg << "#baseStartParts="<<baseStartParts.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)
    for(set<PartPtr>::iterator baseSPart=baseStartParts.begin(); baseSPart!=baseStartParts.end(); baseSPart++) {
      SIGHT_VERB(dbg << "    "<<(*baseSPart)->str()<<endl, 1, VirtualMethodAnalysisDebugLevel)
      cache_GetStartAStates_Spec.insert(VirtualMethodPart::create(*baseSPart, this));
    }
    SIGHT_VERB(dbg << "#cache_GetStartAStates_Spec="<<cache_GetStartAStates_Spec.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)
    cacheInitialized_GetStartAStates_Spec = true;
  }
  return cache_GetStartAStates_Spec;
}

set<PartPtr> VirtualMethodAnalysis::GetEndAStates_Spec()
{
  SIGHT_VERB(scope reg("VirtualMethodAnalysis::GetEndAStates_Spec()", scope::medium), 1, VirtualMethodAnalysisDebugLevel)
  if(!cacheInitialized_GetEndAStates_Spec) {
    set<PartPtr> endParts = getComposer()->GetEndAStates(this);
    SIGHT_VERB(dbg << "#endParts="<<endParts.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)
    for(set<PartPtr>::iterator baseEPart=endParts.begin(); baseEPart!=endParts.end(); baseEPart++) {
      SIGHT_VERB(dbg << "    "<<(*baseEPart)->str()<<endl, 1, VirtualMethodAnalysisDebugLevel)
      cache_GetEndAStates_Spec.insert(VirtualMethodPart::create(*baseEPart, this));
    }
    SIGHT_VERB(dbg << "#cache_GetEndAStates_Spec="<<cache_GetEndAStates_Spec.size()<<endl, 1, VirtualMethodAnalysisDebugLevel)
    cacheInitialized_GetEndAStates_Spec = true;
  }
  return cache_GetEndAStates_Spec;
}

/*****************************************
 ***** VirtualMethodAnalysisTransfer *****
 *****************************************/

VirtualMethodAnalysisTransfer::VirtualMethodAnalysisTransfer(
          AnalysisParts& parts, CFGNode cn, NodeState& state,
          map<PartEdgePtr, vector<Lattice*> >& dfInfo,
          Composer* composer, VirtualMethodAnalysis* analysis)
   : VariableStateTransfer<ClassInheritanceTree, VirtualMethodAnalysis>
                       (state, dfInfo, boost::make_shared<ClassInheritanceTree>(parts.NodeState()->inEdgeFromAny()),
                        composer, analysis, parts, cn,
                        VirtualMethodAnalysisDebugLevel, "VirtualMethodAnalysisDebugLevel")
{}

void VirtualMethodAnalysisTransfer::visit(SgVarRefExp *vref) {
  SIGHT_VERB(scope reg("VirtualMethodAnalysisTransfer::visit(SgVarRefExp)", scope::medium), 1, VirtualMethodAnalysisDebugLevel)

  if(SgClassType* ctype = isSgClassType(vref->get_type())) {
    SgClassDeclaration* cdecl=isSgClassDeclaration(ctype->get_declaration()->get_definingDeclaration());
    ROSE_ASSERT(cdecl);
    SIGHT_VERB(dbg << "cdecl="<<SgNode2Str(cdecl)<<endl, 1, VirtualMethodAnalysisDebugLevel)
    SgClassDefinition* cdef = isSgClassDefinition(cdecl->get_definition());
    ROSE_ASSERT(cdef);
    SIGHT_VERB(dbg << "cdef="<<SgNode2Str(cdef)<<endl, 1, VirtualMethodAnalysisDebugLevel)

    setLattice(vref, boost::make_shared<ClassInheritanceTree>(cdef, parts.NodeState()->inEdgeFromAny()));
  }
}

void VirtualMethodAnalysisTransfer::visit(SgInitializedName *name) {
  SIGHT_VERB(scope reg("VirtualMethodAnalysisTransfer::visit(SgInitializedName)", scope::medium), 1, VirtualMethodAnalysisDebugLevel)

  if(SgClassType* ctype = isSgClassType(name->get_type())) {
    SgClassDeclaration* cdecl=isSgClassDeclaration(ctype->get_declaration()->get_definingDeclaration());
    ROSE_ASSERT(cdecl);
    SIGHT_VERB(dbg << "cdecl="<<SgNode2Str(cdecl)<<endl, 1, VirtualMethodAnalysisDebugLevel);
    SIGHT_VERB(dbg << "cdef="<<SgNode2Str(cdecl->get_definition())<<endl, 1, VirtualMethodAnalysisDebugLevel);
    SgClassDefinition* cdef = isSgClassDefinition(cdecl->get_definition());
    ROSE_ASSERT(cdef);
    SIGHT_VERB(dbg << "cdef="<<SgNode2Str(cdef)<<endl, 1, VirtualMethodAnalysisDebugLevel)

    setLattice(name, boost::make_shared<ClassInheritanceTree>(cdef, parts.NodeState()->inEdgeFromAny()));
  }
}

} // namespace fuse
