#include "sage3basic.h"
using namespace std;

#include "partitions.h"
#include "abstract_object.h"
#include "compose.h"
#include <boostGraphCFG.h>
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include "ssa.h"

#ifndef DISABLE_SIGHT

#include "sight.h"
using namespace sight;

#endif

#ifdef DISABLE_SIGHT
#include "sight-disable.h"
#endif

using namespace boost;

namespace fuse {

#ifndef DISABLE_SIGHT
#define atsDebugLevel 0
#define moduleProfile false
#endif

/* ###########################
   ##### SSAMemLocObject #####
   ########################### */

SSAMemLocObjectPtr NULLSSAMemLocObject;

SSAMemLocObject::SSAMemLocObject(const SSAMemLocObject& that) : MemLocObject(that)
{
  baseML = that.baseML;
  loc    = that.loc;
  sgn    = that.sgn;
  access = that.access;
  phiID  = that.phiID;
}
// Function that generates FuseCFGNodes. Used in code regions that need to generate such nodes
// but can't include fuse.h due to circular include dependencies
// FuseCFGNodePtr createFuseCFGNode(Fuse* fuseAnalysis, PartPtr part);

// Return the FuseCFGNode of the given Fuse object that wraps the location Part of this SSAMemLocObject
FuseCFGNodePtr SSAMemLocObject::getFuseCFGNodeLoc(Fuse* fuseAnalysis) const
{ FuseCFGNodePtr empty;
  return empty; }
    //createFuseCFGNode(fuseAnalysis, loc); }

// pretty print
string SSAMemLocObject::str(string indent) const {
  ostringstream oss;
  oss << "[SSAMemLocObject: access="<<accessType2Str(access)<<", phiID="<<phiID<<", baseML="<< baseML->str(indent+"    ")<<endl;
  oss << indent << "    loc=" << loc->str(indent+"    ")<<endl;
  oss << indent << "    sgn=" << SgNode2Str(sgn)<<"]";
  return oss.str();
}

// copy this object and return a pointer to it
MemLocObjectPtr SSAMemLocObject::copyML() const
{ return makePtr<SSAMemLocObject>(*this); }

SgNode* SSAMemLocObject::getBase() const { return baseML->getBase(); }
MemRegionObjectPtr SSAMemLocObject::getRegion() const { return baseML->getRegion(); }
ValueObjectPtr     SSAMemLocObject::getIndex()  const { return baseML->getIndex(); }

bool SSAMemLocObject::operator==(const SSAMemLocObjectPtr& that) const {
  return loc    == that->loc &&
         sgn    == that->sgn &&
         access == that->access &&
         phiID  == that->phiID;
}
bool SSAMemLocObject::operator< (const SSAMemLocObjectPtr& that) const {
 return (loc <  that->loc) ||
        (loc == that->loc && ((sgn <  that->sgn) ||
                              (sgn == that->sgn && ((access <  that->access) ||
                                                    (access == that->access && phiID < that->phiID)))));
}
bool SSAMemLocObject::operator!=(const SSAMemLocObjectPtr& that) const
{ return !(*this == that); }
bool SSAMemLocObject::operator>=(const SSAMemLocObjectPtr& that) const
{ return *this == that || *this > that; }
bool SSAMemLocObject::operator<=(const SSAMemLocObjectPtr& that) const
{ return *this == that || *this < that; }
bool SSAMemLocObject::operator> (const SSAMemLocObjectPtr& that) const
{ return !(*this == that) && !(*this < that); }

bool SSAMemLocObject::mayEqual(MemLocObjectPtr that_arg, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  SSAMemLocObjectPtr that = boost::dynamic_pointer_cast<SSAMemLocObject>(that_arg);
  assert(that);

  // If these MemLocs were defined at the same SSA node and have the same type of access (use, def, usedef)
  if(loc == that->loc && access == that->access)
    return baseML->mayEqual(that->baseML, /*pedge->getParent()*/loc->inEdgeFromAny(), comp, analysis);
  // Otherwise, they may not be equal
  else
    return false;
}

bool SSAMemLocObject::mustEqual(MemLocObjectPtr that_arg, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  SSAMemLocObjectPtr that = boost::dynamic_pointer_cast<SSAMemLocObject>(that_arg);
  assert(that);

  // If these MemLocs were defined at the same SSA node and have the same type of access (use, def, usedef)
  if(loc == that->loc && access == that->access)
    return baseML->mustEqual(that->baseML, /*pedge->getParent()*/loc->inEdgeFromAny(), comp, analysis);
  // Otherwise, they are not must-equal
  else
    return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool SSAMemLocObject::equalSet(MemLocObjectPtr that_arg, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  SSAMemLocObjectPtr that = boost::dynamic_pointer_cast<SSAMemLocObject>(that_arg);
  assert(that);

  // If these MemLocs were defined at the same SSA node and have the same type of access (use, def, usedef)
  if(loc == that->loc && access == that->access)
    return baseML->equalSet(that->baseML, /*pedge->getParent()*/loc->inEdgeFromAny(), comp, analysis);
  // Otherwise, their sets must not be equal
  else
    return false;
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool SSAMemLocObject::subSet(MemLocObjectPtr that_arg, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  SSAMemLocObjectPtr that = boost::dynamic_pointer_cast<SSAMemLocObject>(that_arg);
  assert(that);

  // If these MemLocs were defined at the same SSA node and have the same type of access (use, def, usedef)
  if(loc == that->loc && access == that->access)
    return baseML->subSet(that->baseML, /*pedge->getParent()*/loc->inEdgeFromAny(), comp, analysis);
  // Otherwise, this is not a subset of that
  else
    return false;
}

bool SSAMemLocObject::isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  //dbg << "SSAMemLocObject::isLive(), loc->outEdgeToAny()="<<loc->outEdgeToAny()->str()<<endl;
  // SSAMemLocs are live at exactly the same Parts where the base MemLoc was live since SSA does not affect liveness or scope
  return baseML->isLive(/*pedge->getParent()*/loc->inEdgeFromAny(), comp, analysis);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool SSAMemLocObject::meetUpdate(MemLocObjectPtr that_arg, PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  SSAMemLocObjectPtr that = boost::dynamic_pointer_cast<SSAMemLocObject>(that_arg);
  assert(that);

  bool modified = false;
  // Meet the MemLoc but don't modify the definition of the location
  modified = baseML->meetUpdate(that->baseML, /*pedge->getParent()*/loc->inEdgeFromAny(), comp, analysis) || modified;
  return modified;
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool SSAMemLocObject::isFull(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  return baseML->isFull(/*pedge->getParent()*/loc->inEdgeFromAny(), comp, analysis);
}

// Returns whether this AbstractObject denotes the empty set.
bool SSAMemLocObject::isEmpty(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis) {
  return baseML->isEmpty(/*pedge->getParent()*/loc->inEdgeFromAny(), comp, analysis);
}

// Set this object to represent the set of all possible MemLocs
// Return true if this causes the object to change and false otherwise.
bool SSAMemLocObject::setToFull() {
  bool modified = false;
  assert(0);
  // WE CANNOT IMPLEMENT THIS BECAUSE SETTING baseML TO FULL WOULD CHANGE IT, WHICH WOULD BE BAD FOR
  // ALL OF ITS OTHER USERS. AS SUCH, WE NEED AbstractObjects TO IMPLEMENT A COPY FUNCTIONALITY.
  //modified = ml->setToFull(comp, baseML, /*pedge->getParent()*/loc->inEdgeFromAny(), ccsa) || modified;
  return modified;
}
// Set this Lattice object to represent the empty set of MemLocs.
// Return true if this causes the object to change and false otherwise.
bool SSAMemLocObject::setToEmpty() {
  assert(0);
}

// Returns whether all instances of this class form a hierarchy. Every instance of the same
// class created by the same analysis must return the same value from this method!
bool SSAMemLocObject::isHierarchy() const {
  return baseML->isHierarchy();
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& SSAMemLocObject::getHierKey() const {
/*  AbstractionHierarchyPtr hierBaseML = boost::dynamic_pointer_cast<AbstractionHierarchy>(baseML);
  ROSE_ASSERT(hierBaseML);
  return hierBaseML->getHierKey();*/

  if(!isHierKeyCached) {
    ((SSAMemLocObject*)this)->cachedHierKey = boost::make_shared<SSAMLHierKey>(((SSAMemLocObject*)this)->shared_from_this());
    ((SSAMemLocObject*)this)->isHierKeyCached = true;
  }
  return cachedHierKey;
}

// Methods for the comparable API
bool SSAMemLocObject::equal(const comparable& that_arg) const {
  const SSAMemLocObject& that = dynamic_cast<const SSAMemLocObject&>(that_arg);
  /*scope s("SSAMemLocObject::equal");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that.str()<<endl;*/

  bool ret = (loc    == that.loc &&
          sgn    == that.sgn &&
          phiID  == that.phiID &&
          access == that.access);

  //dbg << "equal="<<ret<<endl;
  return ret;
}

bool SSAMemLocObject::less(const comparable& that_arg) const {
  const SSAMemLocObject& that = dynamic_cast<const SSAMemLocObject&>(that_arg);
  /*scope s("SSAMemLocObject::less");
  dbg << "this="<<str()<<endl;
  dbg << "that="<<that.str()<<endl;*/

  bool ret = ((loc <  that.loc) ||
          (loc == that.loc && ((sgn < that.sgn) ||
                               (sgn == that.sgn && ((phiID <  that.phiID) ||
                                                    (phiID == that.phiID && access < that.access))))));
  //dbg << "less="<<ret<<endl;
  return ret;
}

SSAMLHierKey::SSAMLHierKey(SSAMemLocObjectPtr ssaML): ssaML(ssaML) {
  AbstractionHierarchyPtr hierBaseML = boost::dynamic_pointer_cast<AbstractionHierarchy>(ssaML->baseML);
  ROSE_ASSERT(hierBaseML);

  const list<comparablePtr>& thatList = hierBaseML->getHierKey()->getList();
  for(list<comparablePtr>::const_iterator i=thatList.begin(); i!=thatList.end(); ++i)
    add(*i);
  add(ssaML);
}

bool SSAMLHierKey::isLive(PartEdgePtr pedge, Composer* comp, ComposedAnalysis* analysis)
{ return ssaML->isLive(pedge, comp, analysis); }


/********************
 ***** SSAGraph *****
 ********************/

SSAGraph::SSAGraph(Composer* comp, ComposedAnalysis* analysis) : comp(comp), analysis(analysis)
{
  // The following two variables are used to record the nodes traversed.
  nodesToVertices.clear();
  set<PartPtr> nodesProcessed;
  set<PartPtr> startStates = comp->GetStartAStates(analysis);

  /*cout << "startStates="<<endl;
  BOOST_FOREACH (const PartPtr& s, startStates) {
    cout << "    "<<s->str()<<endl;
  }*/

  /*BOOST_FOREACH (const PartPtr& s, startStates) {
    buildCFG(s, nodesToVertices, nodesProcessed);
  }*/
  ssaBuilt=false;

  buildSSA();
}

void SSAGraph::buildCFG(PartPtr node,
                        std::map<PartPtr, Vertex>& nodesAdded,
                        std::set<PartPtr>& nodesProcessed)
{
  //scope a("buildCFG");
  //dbg << "node="<<node->str()<<endl;
  // Each node is processed exactly once
  if (nodesProcessed.count(node) > 0) return;
  nodesProcessed.insert(node);

  std::map<PartPtr, Vertex>::iterator iter;
  bool inserted;
  Vertex from, to;

  // Add the source node.
  const PartPtr& src = node;

  boost::tie(iter, inserted) = nodesAdded.insert(std::make_pair(src, Vertex()));
  if (inserted)
  {
    from = add_vertex(*this);
    (*this)[from] = src;
    iter->second = from;
  }
  else
  {
    from = iter->second;
  }

  list<PartEdgePtr> outEdges = node->outEdges();
  BOOST_FOREACH (const PartEdgePtr& pedge, outEdges)
  {
    // For each out edge, add the target node.
    PartPtr tar = pedge->target();

    boost::tie(iter, inserted) = nodesAdded.insert(std::make_pair(tar, Vertex()));
    if (inserted)
    {
      to = add_vertex(*this);
      (*this)[to] = tar;
      iter->second = to;
    }
    else
    {
      to = iter->second;
    }

    // Add the edge.
    Edge edge = add_edge(from, to, *this).first;
    (*this)[edge] = pedge;

    // Build the CFG recursively.
    buildCFG(tar, nodesAdded, nodesProcessed);
  }
}

// Return the anchor Parts of a given function
std::set<PartPtr> SSAGraph::GetStartAStates()
{ return comp->GetStartAStates(analysis); }

// There may be multiple terminal points in the application (multiple calls to exit(), returns from main(), etc.)
std::set<PartPtr> SSAGraph::GetEndAStates()
{ return comp->GetEndAStates(analysis); }

SSAGraph::Vertex SSAGraph::getVertexForNode(PartPtr node) const
{
  std::map<PartPtr, Vertex>::const_iterator vertexIter = nodesToVertices.find(node);
  if (vertexIter == nodesToVertices.end())
    return GraphTraits::null_vertex();
  else
  {
    ROSE_ASSERT((*this)[vertexIter->second] == node);
    return vertexIter->second;
  }
}

set<PartPtr> SSAGraph::calculateIteratedDominanceFrontier(const vector<PartPtr>& startNodes)
{
  set<PartPtr> result;
  set<PartPtr> visitedNodes;
  vector<PartPtr> worklist;

  worklist.insert(worklist.end(), startNodes.begin(), startNodes.end());

  while (!worklist.empty())
  {
    PartPtr currentNode = worklist.back();
    worklist.pop_back();
    visitedNodes.insert(currentNode);

    //Get the dominance frontier of the node and add it to the results
    ROSE_ASSERT(dominanceFrontiers.count(currentNode) != 0);
    const set<PartPtr>& dominanceFrontier = dominanceFrontiers.find(currentNode)->second;

    //Add all the children to the result and to the worklist
    BOOST_FOREACH(PartPtr dfNode, dominanceFrontier)
    {
      if (visitedNodes.count(dfNode) > 0)
        continue;

      result.insert(dfNode);
      worklist.push_back(dfNode);
    }
  }

  return result;
}

void SSAGraph::showDominatorTree() {
  ostringstream dot;

  dot << "digraph DominatorTree {"<<endl;

  typedef graph_traits<SSAGraph>::vertex_iterator vertex_iter;
  for(std::pair<vertex_iter, vertex_iter> vp = vertices(*this); vp.first != vp.second; ++vp.first)
    dot << "node"<<*(vp.first)<<" [label=\""<<(*this)[*(vp.first)]->str()<<"\"];"<<endl;

  for(std::map<Vertex, Vertex>::iterator d=dominatorTree.begin(); d!=dominatorTree.end(); d++)
    dot << "node"<<d->second << " -> node"<<d->first<<";"<<endl;

  dot << "}"<<endl;

#ifndef DISABLE_SIGHT
  sight::structure::graph g(dot.str());
#endif
}

void SSAGraph::showDominanceFrontier() {
  ostringstream dot;

  dot << "digraph DominatorTree {"<<endl;
  typedef graph_traits<SSAGraph>::vertex_iterator vertex_iter;
  for(std::pair<vertex_iter, vertex_iter> vp = vertices(*this); vp.first != vp.second; ++vp.first)
    dot << "node"<<*(vp.first)<<" [label=\""<<(*this)[*(vp.first)]->str()<<"\"];"<<endl;

#ifndef DISABLE_SIGHT
  dbg << "showDominanceFrontier:"<<endl;
#endif
  for(map<PartPtr, std::set<PartPtr> >::const_iterator n=dominanceFrontiers.begin(); n!=dominanceFrontiers.end(); n++) {
#ifndef DISABLE_SIGHT
    dbg << "n="<<n->first->str()<<endl;
    BOOST_FOREACH(PartPtr f, n->second) { dbg << "    :"<<f->str()<<endl; }
#endif
    vector<PartPtr> cur; cur.push_back(n->first);
    set<PartPtr> front = calculateIteratedDominanceFrontier(cur);
#ifndef DISABLE_SIGHT
    BOOST_FOREACH(PartPtr f, front) { dbg << "    ~"<<f->str()<<endl; }
#endif
    for(std::set<PartPtr>::const_iterator i=front.begin(); i!=front.end(); i++)
      dot << "node"<<nodesToVertices[n->first]<<" -> node"<<nodesToVertices[*i]<<endl;
  }

  dot << "}"<<endl;

#ifndef DISABLE_SIGHT
  sight::structure::graph g(dot.str());
#endif
}

std::set<SSAMemLocObjectPtr> SSAGraph::emptySSAMemLocObjectSet;
std::list<SSAMemLocObjectPtr> SSAGraph::emptySSAMemLocObjectList;
std::list<std::pair<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > > SSAGraph::emptySSAMemLocObjectMapping;

// Given a set of SSAMemLocObjectPtrs returns a corresponding set of MemLocObjectPtrs
set<MemLocObjectPtr> SSAGraph::SSAMLSet2MLSet(const set<SSAMemLocObjectPtr>& s) {
  set<MemLocObjectPtr> ret;
  for(set<SSAMemLocObjectPtr>::const_iterator i=s.begin(); i!=s.end(); i++)
    ret.insert((MemLocObjectPtr)*i);
  return ret;
}

// Return the set of uses at this part
const std::set<SSAMemLocObjectPtr>& SSAGraph::getUses(PartPtr part) const {
  //scope s(txt()<<"SSAGraph::getUses("<<part->str()<<")");

  /*{ scope s("uses", scope::high);
  for(map<PartPtr, set<SSAMemLocObjectPtr> >::const_iterator u=uses.begin(); u!=uses.end(); ++u)
    dbg << "part="<<u->first->str()<<", #uses="<<u->second.size()<<endl;
  }*/
  std::map<PartPtr, std::set<SSAMemLocObjectPtr> >::const_iterator i=uses.find(part);
  if(i!=uses.end()) {
    //dbg << "Return: #"<<i->second.size()<<endl;
    return i->second;
  }
  else              return emptySSAMemLocObjectSet;
}

std::set<MemLocObjectPtr> SSAGraph::getUsesML(PartPtr part) const
{ return SSAMLSet2MLSet(getUses(part)); }


// Return the set of defs at this part
const std::set<SSAMemLocObjectPtr>& SSAGraph::getDefs(PartPtr part) const {
  std::map<PartPtr, std::set<SSAMemLocObjectPtr> >::const_iterator i=defs.find(part);
  if(i!=defs.end()) return i->second;
  else              return emptySSAMemLocObjectSet;
}

std::set<MemLocObjectPtr> SSAGraph::getDefsML(PartPtr part) const
{ return SSAMLSet2MLSet(getDefs(part)); }

// Returns all the SSAMemLocObjects within the given given set are definite defs rather than phi defs or uses
bool allDefiniteDefs(const set<SSAMemLocObjectPtr>& defs) {
  for(set<SSAMemLocObjectPtr>::const_iterator d=defs.begin(); d!=defs.end(); ++d) {
    if((*d)->getAccess() != SSAMemLocObject::def) return false;
  }
  return true;
}

// Returns the defs that transitively reach the given use through zero or more phi nodes
std::set<SSAMemLocObjectPtr> SSAGraph::getTransitiveDefs(SSAMemLocObjectPtr use) const {
  //scope s(txt()<<"SSAGraph::getTransitiveDefs("<<use->str()<<")");
  // The definite definitions that are the origin of the def-use chains that reach use
  set<SSAMemLocObjectPtr> originDefs;

  const set<SSAMemLocObjectPtr>& initialDefs = getDefs(use);
  //dbg << "#initialDefs="<<initialDefs.size()<<endl;

  // Set phiDefs to refer to the phi definitions in curDefs and add the rest of originDefs
  set<SSAMemLocObjectPtr> phiDefs;
  for(set<SSAMemLocObjectPtr>::const_iterator d=initialDefs.begin(); d!=initialDefs.end(); ++d) {
    //indent ind; dbg << "d="<<(*d)->str()<<endl;
    
    if((*d)->getAccess() == SSAMemLocObject::def)         originDefs.insert(*d);
    else if((*d)->getAccess() == SSAMemLocObject::phiDef) phiDefs.insert(*d);
    else assert(0);
  }

  // Keep searching backwards through use->def relations until we're only left with definite defs
  int counter=0;
  // Tracks the phiDefs that have already been visited. These are are evaluated just once to avoid def cycles.
  set<SSAMemLocObjectPtr> visitedPhiDefs;
  while(phiDefs.size()>0) {
    //scope s(txt()<<"#phiDefs="<<phiDefs.size());
    // Iterate over the definitions that immediately reach the current set of phiDefs,
    // adding the phiDefs to newPhiDefs and defs to originDefs
    set<SSAMemLocObjectPtr> nextPhiDefs;
    for(set<SSAMemLocObjectPtr>::const_iterator d=phiDefs.begin(); d!=phiDefs.end(); ++d) {
      const set<SSAMemLocObjectPtr>& nextDefs = getReachingDefsAtPhiDef(*d);
      //scope s("phiDef");
      //dbg << "d="<<(*d)->str()<<", #nextDefs="<<nextDefs.size()<<endl;
      for(set<SSAMemLocObjectPtr>::const_iterator nd=nextDefs.begin(); nd!=nextDefs.end(); ++nd) {
        //scope s("nextDef");
        //dbg << "nd="<<(*nd)->str()<<", def="<<((*nd)->getAccess() == SSAMemLocObject::def)<<", phiDef="<<((*nd)->getAccess() == SSAMemLocObject::phiDef)<<endl;
        if((*nd)->getAccess() == SSAMemLocObject::def)         originDefs.insert(*nd);
        else if((*nd)->getAccess() == SSAMemLocObject::phiDef) {
          if(visitedPhiDefs.find(*nd) == visitedPhiDefs.end()) {
            nextPhiDefs.insert(*nd);
            visitedPhiDefs.insert(*nd);
          }
        }
        else assert(0);
      }
    }

    // Update phiDefs to refer to the phi defs that reach its original contents
    phiDefs = nextPhiDefs;
    counter++;
    if(counter>10) exit(-1);
  }

  return originDefs;
}

std::set<MemLocObjectPtr> SSAGraph::getTransitiveDefsML(SSAMemLocObjectPtr use) const
{ return SSAMLSet2MLSet(getTransitiveDefs(use)); }

// Collects all the defs and uses at each Part and stores them into defs and uses
void SSAGraph::collectDefsUses() {
  set<PartPtr> startStates = comp->GetStartAStates(analysis);
  for(fw_partEdgeIterator state(startStates, /*incrementalGraph*/ false); !state.isEnd(); state++) {
    PartPtr part = state.getPart();
    set<SSAMemLocObjectPtr>& partDefs = defs[part];
    set<SSAMemLocObjectPtr>& partUses = uses[part];

    set<CFGNode> cfgNodes = part->CFGNodes();
    for(set<CFGNode>::iterator cn=cfgNodes.begin(); cn!=cfgNodes.end(); cn++) {
      SgNode* sgn = cn->getNode();
      if(SgBinaryOp* binOp = isSgBinaryOp(sgn)) {
        if(SgPntrArrRefExp* par = isSgPntrArrRefExp(binOp)) {
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(par->get_rhs_operand(), state.getPartEdge(), analysis), part, par->get_rhs_operand(), SSAMemLocObject::use));
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(par->get_lhs_operand(), state.getPartEdge(), analysis), part, par->get_lhs_operand(), SSAMemLocObject::use));
        } else {
          if(isSgAssignOp(sgn) || isSgCompoundAssignOp(sgn))
            partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(binOp->get_lhs_operand(), state.getPartEdge(), analysis), part, binOp->get_lhs_operand(), SSAMemLocObject::def));

          if(!isSgAssignOp(sgn))
          //!!! We're using the memory location info of the lhs, not its value, should differentiate these
            partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(binOp->get_lhs_operand(), state.getPartEdge(), analysis), part, binOp->get_lhs_operand(), SSAMemLocObject::use));

          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(binOp->get_rhs_operand(), state.getPartEdge(), analysis), part, binOp->get_rhs_operand(), SSAMemLocObject::use));
          partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(binOp, state.getPartEdge(), analysis), part, binOp, SSAMemLocObject::def));
        }
        // !!!! Should add a use of the memory location of the LHS
      } else if(SgUnaryOp* unOp = isSgUnaryOp(sgn)) {
        if(isSgMinusMinusOp(sgn) || isSgPlusPlusOp(sgn))
          partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(unOp->get_operand(), state.getPartEdge(), analysis), part, unOp->get_operand(), SSAMemLocObject::def));

        partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(unOp->get_operand(), state.getPartEdge(), analysis), part, unOp->get_operand(), SSAMemLocObject::use));
        partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(unOp, state.getPartEdge(), analysis), part, unOp, SSAMemLocObject::def));
      } else if(SgInitializedName* iname = isSgInitializedName(sgn)) {
        if(iname->get_initializer())
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(iname->get_initializer(), state.getPartEdge(), analysis), part, iname->get_initializer(), SSAMemLocObject::use));
        partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(iname, state.getPartEdge(), analysis), part, iname, SSAMemLocObject::def));
      } else if(SgIfStmt* ifStmt = isSgIfStmt(sgn)) {
        if(SgExprStatement* exprStmt = isSgExprStatement(ifStmt->get_conditional()))
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(exprStmt->get_expression(), state.getPartEdge(), analysis), part, exprStmt->get_expression(), SSAMemLocObject::use));
      } else if(SgWhileStmt* whileStmt = isSgWhileStmt(sgn)) {
        if(SgExprStatement* exprStmt = isSgExprStatement(whileStmt->get_condition()))
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(exprStmt->get_expression(), state.getPartEdge(), analysis), part, exprStmt->get_expression(), SSAMemLocObject::use));
      } else if(SgDoWhileStmt* doWhileStmt = isSgDoWhileStmt(sgn)) {
        if(SgExprStatement* exprStmt = isSgExprStatement(doWhileStmt->get_condition()))
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(exprStmt->get_expression(), state.getPartEdge(), analysis), part, exprStmt->get_expression(), SSAMemLocObject::use));
      } else if(SgForStatement* forStmt = isSgForStatement(sgn)) {
        if(SgExprStatement* exprStmt = isSgExprStatement(forStmt->get_test()))
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(exprStmt, state.getPartEdge(), analysis), part, exprStmt, SSAMemLocObject::use));
        if(SgExprStatement* exprStmt = isSgExprStatement(forStmt->get_increment()))
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(exprStmt, state.getPartEdge(), analysis), part, exprStmt, SSAMemLocObject::use));
      } else if(SgSwitchStatement* switchStmt = isSgSwitchStatement(sgn)) {
        if(SgExprStatement* exprStmt = isSgExprStatement(switchStmt->get_item_selector()))
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(exprStmt->get_expression(), state.getPartEdge(), analysis), part, exprStmt->get_expression(), SSAMemLocObject::use));
      } else if(part->mustOutgoingFuncCall()) {
        SgFunctionCallExp* call = isSgFunctionCallExp(sgn);
        ROSE_ASSERT(call);

        // If this function is called through an expression rather than a FunctionSymbol, add a use of that expression
        if(!isSgFunctionCallExp(call->get_function()))
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(call->get_function(), state.getPartEdge(), analysis), part, call->get_function(), SSAMemLocObject::use));

        // ----------------------------
        // For each argument add a use that corresponds to the call reading its value and record
        // the use in funcArg2Param
        SgExprListExp* args = call->get_args();
        for(SgExpressionPtrList::iterator arg=args->get_expressions().begin(); arg!=args->get_expressions().end(); arg++) {
          SSAMemLocObjectPtr argUse = makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*arg, state.getPartEdge(), analysis), part, *arg, SSAMemLocObject::use);
          partUses.insert(argUse);
          //SSAMemLocObjectPtr argDef = makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*arg, state.getPartEdge(), analysis), part, *arg, SSAMemLocObject::def);
          funcArg2Param[part].push_back(make_pair(argUse, set<SSAMemLocObjectPtr>()));
          //SSAMemLocObjectPtr argDef = makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*a, state.getPartEdge(), analysis), part, *a, SSAMemLocObject::def);
          //partDefs.insert(argDef);
          // Also insert this argument's def into funcArgDefs, which maintains the order of the arguments
          //funcArgDefs[part].push_back(argDef);
        }

        // ----------------------------
        // Iterate over all the outgoing edges to function SgFunctionParameterLists.
        // For each argument add a def of the corresponding parameter and record the connection from
        //   the use of the argument to the corresponding def in funcArg2Param[part]s.

        // Edges to the SgFunctionParameterLists nodes of the functions this call may invoke
        list<PartEdgePtr> edgesToFunc = part->outEdges();

        for(list<PartEdgePtr>::iterator e=edgesToFunc.begin(); e!=edgesToFunc.end(); ++e) {
          // Get the SgParameter list of the current function entry point
          set<CFGNode> paramLists;
          ROSE_ASSERT((*e)->target() && (*e)->target()->mustFuncEntry(paramLists));
          ROSE_ASSERT(paramLists.size()==1);
          SgFunctionParameterList* params = isSgFunctionParameterList(paramLists.begin()->getNode());
          ROSE_ASSERT(params);

          // Iterate over all the function arguments and corresponding parameters, adding parameter
          // definitions to the current Part and connecting the uses of the arguments to the defs
          // of the parameters.
          ROSE_ASSERT((args->get_expressions().size() == params->get_args().size()) &&
                      (args->get_expressions().size() == funcArg2Param[part].size()));
          SgInitializedNamePtrList::iterator param = params->get_args().begin();
          list<pair<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator arg2param = funcArg2Param[part].begin();

          for(; param != params->get_args().end(); ++param, ++arg2param) {
            SgType* typeParam = (*param)->get_type();

            // Skip "..." types, which are used to specify VarArgs.
            // NEED TO SUPPORT VAR ARGS MORE EFECTIVELY IN THE FUTURE
            if(isSgTypeEllipse(typeParam)) continue;

            SSAMemLocObjectPtr paramDef = makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*param, *e, analysis), (*e)->target(), *param, SSAMemLocObject::def);
            partDefs.insert(paramDef);
            arg2param->second.insert(paramDef);
          }
        }
      } else if(SgFunctionParameterList* params = isSgFunctionParameterList(sgn)) {
        //Function func(params);

        // Make all the pass-by-value and pass-by-reference parameters uses
        for(SgInitializedNamePtrList::const_iterator param=params->get_args().begin(); param!=params->get_args().end(); ++param) {
          SgType* typeParam = (*param)->get_type();

          // Skip "..." types, which are used to specify VarArgs.
          // NEED TO SUPPORT VAR ARGS MORE EFECTIVELY IN THE FUTURE
          if(isSgTypeEllipse(typeParam)) continue;

          // Pass by reference
          /*if(isSgReferenceType(typeParam)) {
            //partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*p, state.getPartEdge(), analysis), part, SSAMemLocObject::def));
            //funcParamUses[func].push_back(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*param, state.getPartEdge(), analysis), part, SSAMemLocObject::use));
            funcParamUses[func].push_back(comp->Expr2MemLoc(*param, state.getPartEdge(), analysis));
          } else {
            //partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*p, state.getPartEdge(), analysis), part, SSAMemLocObject::def));
            //funcParamUses[func].push_back(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*param, state.getPartEdge(), analysis), part, SSAMemLocObject::use));
            funcParamUses[func].push_back(comp->Expr2MemLoc(*param, state.getPartEdge(), analysis));
          }*/

          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*param, state.getPartEdge(), analysis), part, *param, SSAMemLocObject::use));
        }

      } else if(SgAssignInitializer* init = isSgAssignInitializer(sgn)) {
        partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(init->get_operand(), state.getPartEdge(), analysis), part, init->get_operand(), SSAMemLocObject::use));
        partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(init, state.getPartEdge(), analysis), part, init, SSAMemLocObject::def));
      } else if(SgAggregateInitializer* init = isSgAggregateInitializer(sgn)) {
        const SgExpressionPtrList& exprs = init->get_initializers()->get_expressions();
        BOOST_FOREACH(SgExpression* expr, exprs) {
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(expr, state.getPartEdge(), analysis), part, expr, SSAMemLocObject::use));
        }
        partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(init, state.getPartEdge(), analysis), part, init, SSAMemLocObject::def));
      } else if(SgCompoundInitializer* init = isSgCompoundInitializer(sgn)) {
        const SgExpressionPtrList& exprs = init->get_initializers()->get_expressions();
        BOOST_FOREACH(SgExpression* expr, exprs) {
          partUses.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(expr, state.getPartEdge(), analysis), part, expr, SSAMemLocObject::use));
        }
        partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(init, state.getPartEdge(), analysis), part, init, SSAMemLocObject::def));
      } else if(SgValueExp* val = isSgValueExp(sgn)) {
        partDefs.insert(makePtr<SSAMemLocObject>(comp->Expr2MemLoc(val, state.getPartEdge(), analysis), part, val, SSAMemLocObject::def));
      }
    }
  }
}

void SSAGraph::showDefsUses() {
#ifndef DISABLE_SIGHT
  dbg << "<u>defs</u>"<<endl;
  dbg << "<table border=1>";
  for(map<PartPtr, set<SSAMemLocObjectPtr> >::iterator d=defs.begin(); d!=defs.end(); ++d) {
    dbg << "<tr><td>"<<d->first->str()<<"</td><td>";
    for(set<SSAMemLocObjectPtr>::iterator i=d->second.begin(); i!=d->second.end(); ++i)
      dbg << (*i)->str()<<endl;
    dbg << "</td></tr>";
  }
  dbg << "</table>";
  dbg <<endl;

  dbg << "<u>uses</u>"<<endl;
  dbg << "<table border=1>";
  for(map<PartPtr, set<SSAMemLocObjectPtr> >::iterator u=uses.begin(); u!=uses.end(); ++u) {
    dbg << "<tr><td>"<<u->first->str()<<"</td><td>";
    for(set<SSAMemLocObjectPtr>::iterator i=u->second.begin(); i!=u->second.end(); ++i)
      dbg << (*i)->str()<<endl;
    dbg << "</td></tr>";
  }
  dbg << "</table>";

  dbg << "<u>funcArgDefs</u>"<<endl;
  dbg << "<table border=1>";
  for(map<PartPtr, list<pair<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > > >::iterator part=funcArg2Param.begin(); part!=funcArg2Param.end(); ++part) {
    dbg << "<tr><td>"<<part->first->str()<<"</td><td><table>";
    for(list<pair<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator a2p=part->second.begin(); a2p!=part->second.end(); ++a2p) {
      dbg << "<tr><td>Arg: "<<a2p->first->str()<<"</td><td>";
      for(set<SSAMemLocObjectPtr>::iterator param=a2p->second.begin(); param!=a2p->second.end(); ++param)
        dbg << (*param)->str()<<endl;
      dbg << "</td></tr>";
    }
    dbg << "</table></td></tr>";
  }
  dbg << "</table>";
#endif
}

// Finds all the Parts where phi nodes should be placed and identifies the MemLocs they must define
/*void SSAGraph::placePhiNodes() {
  SIGHT_VERB_DECL(scope, ("placePhiNodes"), 2, atsDebugLevel)

    { scope s2("def2use");
      for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr>  >::iterator i=def2use.begin(); i!=def2use.end(); i++) {
        dbg << "i->first="<<i->first.get()<<endl;
        scope s(i->first->str());
        for(set<SSAMemLocObjectPtr>::iterator j=i->second.begin(); j!=i->second.end(); j++)
          dbg << "    "<<(*j)->str()<<endl;
      } }

  // Iterate over all the parts that are on the dominance frontier of some other parts
  for(map<PartPtr, set<PartPtr> >::iterator i=domFrontierOf.begin(); i!=domFrontierOf.end(); i++) {
    SIGHT_VERB_DECL(scope, (txt()<<"Part "<<i->first->str()), 2, atsDebugLevel)
    // We will place a Phi node at i->first that re-defines all the MemLocs defined at the nodes
    // that have i->first on their dominance frontier

    // Iterate over all the Parts that have i->first on their dominance frontiers
    for(set<PartPtr>::const_iterator j=i->second.begin(); j!=i->second.end(); j++) {
      SIGHT_VERB_DECL(scope, (txt()<<"Frontier "<<(*j)->str()<<" #"<<defs[*j].size()), 2, atsDebugLevel)
      // Iterate over all the MemLocs defined at Part j
      for(set<SSAMemLocObjectPtr>::iterator d=defs[*j].begin(); d!=defs[*j].end(); d++) {
        SIGHT_VERB(dbg << "Live="<<(((*d)->baseML->isLive(i->first.get()->inEdgeFromAny(), comp, analysis)))<<" def="<<(*d)->str()<<endl, 2, atsDebugLevel)
        // If this def is live at i->first, add it as a phi definition at that Part
        if(((*d)->baseML->isLive(i->first.get()->inEdgeFromAny(), comp, analysis))) {
          // Insert into phiMLs[i->first] if no other node in the set denotes the same set as *d
          bool foundEqSet=false;
          for(set<SSAMemLocObjectPtr>::iterator ml=phiMLs[i->first].begin(); ml!=phiMLs[i->first].end(); ml++) {
            if((*ml)->baseML->equalSet((*d)->baseML, i->first.get()->inEdgeFromAny(), comp, analysis)) {
              foundEqSet = true;
              break;
            }
          }
          SSAMemLocObjectPtr phiAccess = makePtr<SSAMemLocObject>((*d)->baseML, i->first, (SgNode*)NULL, SSAMemLocObject::usedef);
          addDefUseLink(phiAccess, *d);
          if(!foundEqSet)
            phiMLs[i->first].insert(phiAccess);
        }
      }
    }
  }
}*/

// Relaces all instances of string search in subject with replace
// From http://stackoverflow.com/questions/5343190/how-do-i-replace-all-instances-of-of-a-string-with-another-string
void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

void SSAGraph::showPhiNodes() {
#ifndef DISABLE_SIGHT
  scope s("showPhiNodes");
#endif

  /*dbg << "<u>phiDefs</u>"<<endl;
  dbg << "<table border=1>";
  for(std::map<PartPtr, std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >::iterator part=phiDefs.begin(); part!=phiDefs.end(); ++part) {
    dbg << "<tr><td>"<<part->first->str()<<"</td><td>";
    dbg << "<table>";
    for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator group=part->second.begin(); group!=part->second.end(); ++group) {
      dbg << "<tr><td>"<<group->first->str()<<"</td></tr><tr><td>";
      for(set<SSAMemLocObjectPtr>::iterator d=group->second.begin(); d!=group->second.end(); ++d)
        dbg << (*d)->str()<<endl;
      dbg << "</td></tr>";
    }
    dbg << "</table></td></tr>";
  }
  dbg << "</table>";*/

  // First, assign unique nodeIDs to all parts in the ATS and emit node descriptions for all parts
  ostringstream dot;
  set<PartPtr> startStates = comp->GetStartAStates(analysis);

  std::map<PartPtr, int> partID;
  int clusterCount=0;
  dot << "digraph PhiNodes {"<<endl;
  dot << "  color=white;"<<endl;
  {
    set<PartPtr> visited;
    set<PartPtr> worklist = startStates;
    //for(fw_partEdgeIterator state(startStates, /*incrementalGraph*/ false); !state.isEnd(); state++) {
    while(worklist.size()>0) {
      //PartPtr part = state.getPart();
      PartPtr part = *worklist.begin();
      worklist.erase(part);
      if(visited.find(part) != visited.end()) continue;
      visited.insert(part);
      list<PartEdgePtr> out = part->outEdges();
      for(list<PartEdgePtr>::iterator i=out.begin(); i!=out.end(); i++) worklist.insert((*i)->target());

      int ID = partID.size();
      string partLabel = part->str();
      // Replace all the line breaks in groupMLLabel with explicit text "\n";
      ReplaceStringInPlace(partLabel, "\n", "\\n");

      // If this is a phi node
      if(isPhiNode(part)) {
        dot << "subgraph cluster_"<<(clusterCount++)<<" {"<<endl;
        dot << "  style=filled;"<<endl;
        dot << "  color=lightgrey;"<<endl;
        dot << "  rankdir=TD;"<<endl;

        dot << "  node"<<ID<<" [label=\""<<partLabel<<"\" color=red];"<<endl;

        int j=0;
        for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator group=phiDefs[part].begin(); group!=phiDefs[part].end(); ++group, ++j) {
          ostringstream edgeDot;
          dot << "subgraph cluster_"<<(clusterCount++)<<" {"<<endl;
          dot << "  style=filled;"<<endl;
          dot << "  color=lightcyan;"<<endl;
          dot << "  rankdir=LR;"<<endl;
          string groupMLLabel = group->first->baseML->str();
          // Replace all the line breaks in groupMLLabel with explicit text "\n";
          ReplaceStringInPlace(groupMLLabel, "\n", "\\n");
          dot << "  label=\""<<groupMLLabel<<"\";"<<endl;

          int k=0;
          for(set<SSAMemLocObjectPtr>::iterator d=group->second.begin(); d!=group->second.end(); ++d, ++k) {
            string defMLLabel = (*d)->str();
            // Replace all the line breaks in groupMLLabel with explicit text "\n";
            ReplaceStringInPlace(defMLLabel, "\n", "\\n");
            dot << "  node"<<ID<<"_"<<j<<"_"<<k<<" [label=\""<<defMLLabel<<"\" border=0];"<<endl;

            if(k>0) {
              edgeDot << "node"<<ID<<"_"<<j<<"_"<<(k-1)<<" -> node"<<ID<<"_"<<j<<"_"<<k<<" [style=invis];"<<endl;
              edgeDot << "node"<<ID<<"_"<<j<<"_"<<k<<" -> node"<<ID<<"_"<<j<<"_"<<(k-1)<<" [style=invis];"<<endl;
            }
          }
          dot << "}"<<endl;

          // Place all edges inside the part's cluster, rather than the group's cluster
          dot << edgeDot.str();

          dot << "node"<<ID<<" -> node"<<ID<<"_"<<j<<"_0 [style=invis];"<<endl;
          if(j>0) {
            dot << "node"<<ID<<"_"<<(j-1)<<"_0 -> node"<<ID<<"_"<<j<<"_0 [style=invis];"<<endl;
            dot << "node"<<ID<<"_"<<j<<"_0 -> node"<<ID<<"_"<<(j-1)<<"_0 [style=invis];"<<endl;
          }
        }
        dot << "}"<<endl;
      }
      else
        dot << "node"<<ID<<" [label=\""<<partLabel<<"\"];"<<endl;
      partID[part] = ID;
    }
  }

  // Second, add transition system edges
  {
  set<PartPtr> visited;
  set<PartPtr> worklist = startStates;
  //for(fw_partEdgeIterator state(startStates, /*incrementalGraph*/ false); !state.isEnd(); state++) {
  while(worklist.size()>0) {
    //PartPtr part = state.getPart();
    PartPtr part = *worklist.begin();
    worklist.erase(part);
    if(visited.find(part) != visited.end()) continue;
    visited.insert(part);
    list<PartEdgePtr> out = part->outEdges();
    for(list<PartEdgePtr>::iterator i=out.begin(); i!=out.end(); i++) worklist.insert((*i)->target());

    //list<PartEdgePtr> out = part->outEdges();
    for(list<PartEdgePtr>::iterator e=out.begin(); e!=out.end(); e++) {
      dot << "node"<<partID[part]<<" -> node"<<partID[(*e)->target()]<<";"<<endl;
      //dbg << part->str()<<"/"<<(*e)->source()->str()<<"/#"<<partID[part]<<" -> "<<(*e)->target()->str()<<"/#"<<partID[(*e)->target()]<<endl;
    }
  }
  }
  dot << "}"<<endl;

#ifndef DISABLE_SIGHT
  sight::structure::graph g(dot.str());
#endif
}

// Returns the mapping of phiDef MemLocs at the given phiNode before the given part
// to the defs and phiDefs that reach them.
const std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> >& SSAGraph::getDefsUsesAtPhiNode(PartPtr part) const {
  ROSE_ASSERT(isPhiNode(part));
  std::map<PartPtr, std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >::const_iterator i=phiDefs.find(part);
  ROSE_ASSERT(i!=phiDefs.end());
  return i->second;
}

// Returns the set of defs and phiDefs that reach the given phiDef
const std::set<SSAMemLocObjectPtr>& SSAGraph::getReachingDefsAtPhiDef(SSAMemLocObjectPtr pd) const {
  ROSE_ASSERT(isPhiNode(pd->getLoc()));
  std::map<PartPtr, std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >::const_iterator i=phiDefs.find(pd->getLoc());
  ROSE_ASSERT(i!=phiDefs.end());
  std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> >::const_iterator j=i->second.find(pd);
  ROSE_ASSERT(j!=i->second.end());

  return j->second;
}

/*
// Returns the ID of the given phiNode part
int SSAGraph::getPhiNodeID(PartPtr part) {
  map<PartPtr, int>::iterator i=phiNodeID.find(part);
  int ID=0;
  // If the given part does not yet have an ID, generate a fresh one
  if(i==phiNodeID.end()) {
    ID = phiNodeID.size();
    phiNodeID[part] = ID;

  // Otherwise, fetch it from iterator i
  } else
    ID = i->second;

  return ID;
}*/

// Returns whether the given def terminates at the given phi node
bool SSAGraph::defTerminatesAtPhiNode(PartPtr phiPart, SSAMemLocObjectPtr def) const {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (txt()<<"defTerminatesAtPhiNode()"), 3, atsDebugLevel)
#endif

  map<PartPtr, map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > >::const_iterator node=phiDefs.find(phiPart);

#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "phiPart(found="<<(node != phiDefs.end())<<")="<<phiPart->str()<<endl, 3, atsDebugLevel)
  SIGHT_VERB(dbg << "def="<<def<<endl, 3, atsDebugLevel)
#endif

  // If this is not a phi node, def cannot terminate at it
  if(node == phiDefs.end()) return false;

  for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::const_iterator group=node->second.begin(); group!=node->second.end(); ++group) {
    BOOST_FOREACH(const SSAMemLocObjectPtr& phiDef,  group->second) {
      SIGHT_VERB(dbg << "def(equal="<<(def==phiDef)<<")="<<phiDef->str()<<endl, 3, atsDebugLevel);
      // Return true if def matches the current def that terminates at this phi node
      if(def==phiDef) return true;
    }
  }

  // We found no match, so def must not terminate here
  return false;
}

// Connect a def/phiDef to a use/phiDef that it reaches
void SSAGraph::addDefUseLink(SSAMemLocObjectPtr use, SSAMemLocObjectPtr def) {
  ROSE_ASSERT(use->getAccess()==SSAMemLocObject::use || use->getAccess()==SSAMemLocObject::phiDef);
  ROSE_ASSERT(def->getAccess()==SSAMemLocObject::def || use->getAccess()==SSAMemLocObject::phiDef);
  use2def[use].insert(def);
  def2use[def].insert(use);
  //dbg << "use="<<use.get()<<", def="<<def.get()<<endl;
}

// Erases all the regular non-phi uses recorded for the given part
void SSAGraph::eraseUsesFromU2D(PartPtr part) {
  // Find all the uses in uses[part] and phiDefs[part] and remove them from use2def and def2use
  if(uses.find(part) != uses.end()) {
    for(set<SSAMemLocObjectPtr>::iterator use=uses[part].begin(); use!=uses[part].end(); ++use) {
      // Erase use from all the defs in defs2use
      for(set<SSAMemLocObjectPtr>::iterator def=use2def[*use].begin(); def!=use2def[*use].end(); ++def) {
        def2use[*def].erase(*use);
      }

      // Clear out all the defs in use2def[*use]
      use2def[*use].clear();
    }
  }
}

// For each use->def link recorded in use2def and phiDefs, adds a corresponding
// reverse link in def2use
void SSAGraph::setDef2Use() {
  // Reset def2use in preparation of it being synchronized with use2def and phiDefs
  def2use.clear();

  for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator u=use2def.begin(); u!=use2def.end(); ++u) {
    for(set<SSAMemLocObjectPtr>::iterator def=u->second.begin(); def!=u->second.end(); ++def) {
      def2use[*def].insert(u->first);
    }
  }

  for(map<PartPtr, map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator part=phiDefs.begin(); part!=phiDefs.end(); ++part) {
    for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator u=part->second.begin(); u!=part->second.end(); ++u) {
      for(set<SSAMemLocObjectPtr>::iterator def=u->second.begin(); def!=u->second.end(); ++def) {
        def2use[*def].insert(u->first);
      }
    }
  }
}

// Returns the SSA uses for the given def
const std::set<SSAMemLocObjectPtr>& SSAGraph::getUses(SSAMemLocObjectPtr def) const {
  map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::const_iterator i=def2use.find(def);
  /*dbg << "SSAGraph::getUses("<<def->str()<<") found="<<(i != def2use.end())<<endl;
  { scope s2("def2use");
    for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr>  >::const_iterator i=def2use.begin(); i!=def2use.end(); i++) {
      dbg << "i->first="<<i->first.get()<<endl;
      scope s(i->first->str());
      for(set<SSAMemLocObjectPtr>::iterator j=i->second.begin(); j!=i->second.end(); j++)
        dbg << "    "<<(*j)->str()<<endl;
    } }*/

  if(i != def2use.end()) return i->second;
  else                   return emptySSAMemLocObjectSet;
}

std::set<MemLocObjectPtr> SSAGraph::getUsesML(SSAMemLocObjectPtr def) const
{ return SSAMLSet2MLSet(getUses(def)); }

// Returns the SSA defs for the given use
const std::set<SSAMemLocObjectPtr>& SSAGraph::getDefs(SSAMemLocObjectPtr use) const {
  //dbg << "SSAGraph::getDefs() use="<<use->str()<<endl;
  map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::const_iterator i=use2def.find(use);
  if(i != use2def.end()) return i->second;
  else                   return emptySSAMemLocObjectSet;
}

std::set<MemLocObjectPtr> SSAGraph::getDefsML(SSAMemLocObjectPtr use) const
{ return SSAMLSet2MLSet(getDefs(use)); }

// Get the list of definitions of the arguments within the function call at the given part
const std::list<std::pair<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& SSAGraph::getFunc2Params(PartPtr part) const
{
  ROSE_ASSERT(part->mustOutgoingFuncCall());
  std::map<PartPtr, std::list<std::pair<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > > >::const_iterator i=funcArg2Param.find(part);
  if(i != funcArg2Param.end()) return i->second;
  else                         return emptySSAMemLocObjectMapping;
}

/* // Matches the given use to the given def at the given edge.
// If the def must-defines the use, the use is removed from liveUses, a def->use connection 
//    is recorded and true is returned. 
// Otherwise, if def may-defines use, then use is not removed, a def->use connection is recorded
//    and false is returned.
// Otherwise, nothing is changed and false is returned.
bool SSAGraph::matchUseToDef(PartEdgePtr pedge, SSAMemLocObjectPtr use, SSAMemLocObjectPtr def,
                             set<SSAMemLocObjectPtr>& liveUses) {
  // If the current def definitely writes to the current use
  if(def->baseML->mustEqual(use->baseML, pedge, comp, analysis)) {
    // Connect the use of u to this def
    addDefUseLink(use, def);
    SIGHT_VERB_IF(2, atsDebugLevel)
    dbg << "must phi definition"<<endl;
    SIGHT_VERB_FI()

    // Erase  the current element of liveUses and advance the iterator u
    liveUses.erase(use);
    return true;

  // Else, if the current def possibly writes to the current use
  } else if(def->baseML->mayEqual(use->baseML, pedge, comp, analysis)) {
    // Connect the use of u to this def
    addDefUseLink(use, def);
    SIGHT_VERB_IF(2, atsDebugLevel)
    dbg << "may phi definition"<<endl;
    SIGHT_VERB_FI()

    // Do not remove this use from liveUses since this may not be the most recent def of *u
  }
  return false;
}*/

/*void addDefToLiveDefs(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >& curLiveDefs,
                      set<SSAMemLocObjectPtr>& curDefs,
                      PartEdge pedge) {
  foreach(SSAMemLocObjectPtr& def, curDefs) {
    // Find the group of defs in curLiveDefs that this def overlaps. Since there may be multiple,
    // we collect them all into matchingDefs
    list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator> matchingLiveDefs;
    for(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator liveDef=curLiveDefs.begin(); liveDef=!curLiveDefs; ++liveDef) {
      if(liveDef->first->mayEqual(def->baseML, pedge, comp, analysis)) {
        matchingLiveDefs.push_back(liveDef);
      }
    }

    // If this def fits into no groups in curLiveDefs, add a new one
    if(matchingLiveDefs.size()==0) {
      curLiveDefs.push_back(make_pair(def->baseML->copyML(), set<SSAMemLocObjectPtr>(def)));

    // Otherwise, if this def fits into exactly one group in curLiveDefs, add it there
    } else if(matchingLiveDefs.size()==1) {
      (*matchingLiveDefs.begin())->second.insert(def);

    // Otherwise, if this def fits into multiple groups in curLiveDefs, merge the groups
    // and add it to the merged group
    } else {
      // Points to the element of curLiveDefs into which the others will be merged
      list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator target = *matchingLiveDefs.begin();
      list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator>::iterator m=target; ++m;
      for(; m!=matchingLiveDefs.end(); ++m) {
        target->first->meetUpdate((*m)->first, pedge, comp, analysis);
        foreach(SSAMemLocObjectPtr& mergeD, (*m)->second) {
          (*target)->second.insert(mergeD);
        }
        // Having merged the current group into the target group, erase it from curLiveDefs
        curLiveDefs.erase(*m);
      }

      // Merge def with the target group
      (*target)->first->meetUpdate(def->baseML, pedge, comp, analysis);
      (*target)->second.insert(def);
    }
  }
}*/

// Given a set of SSAMemLocObject definitions that are guaranteed to denote disjoint sets,
// group them according to how their MemLocObjects overlap each other. The result is a list
// of pairs where the first element is a MemLocObject and the second is set of SSAMemLocObjects
// that transitively overlap each other and are contained inside the first element.
// It is guaranteed that the MemLocObjects in different elements of the returned list denote disjoint sets.
//list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > > groupDefs(set<SSAMemLocObjectPtr>& defs) {


// Simulate a definition of all the defs after the definitions of all the defs in curLiveDefs, updating
// curLiveDefs with the result.
// Thus, if a SSAMemLocObject in defs must-equals a MemLocObject that is a group key (first element in
//   pair) in curLiveDefs, the old mapping is replaced with the SSAMemLocObject from defs (the new
//   mapping has the same baseML but new definition location.
// However, if it is may-equals but not must-equals, the mapping from defs is added to the group (the
//   second element in pair) without removing the original.
void SSAGraph::assignDefAfterLiveDefs(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >& curLiveDefs,
                                      set<SSAMemLocObjectPtr>& defs,
                                      PartEdgePtr pedge) {
  for(set<SSAMemLocObjectPtr>::iterator def=defs.begin(); def!=defs.end(); ++def) {
    // Find the group of defs in curLiveDefs that this def overlaps. Since there may be multiple,
    // we collect them all into matchingDefs
    list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator> mustEqualLiveDefs;
    list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator> mayEqualLiveDefs;
    for(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator liveDef=curLiveDefs.begin(); liveDef!=curLiveDefs.end(); ++liveDef) {
      if(liveDef->first->mustEqual((*def)->baseML, pedge, comp, analysis)) {
        mustEqualLiveDefs.push_back(liveDef);
      } else if(liveDef->first->mayEqual((*def)->baseML, pedge, comp, analysis)) {
        mayEqualLiveDefs.push_back(liveDef);
      }
    }
    // If def mustEquals anything it may not mayEqual anything else
    ROSE_ASSERT((mustEqualLiveDefs.size()==0 && mayEqualLiveDefs.size()>0) ||
                (mustEqualLiveDefs.size()==1 && mayEqualLiveDefs.size()==0) ||
                (mustEqualLiveDefs.size()==0 && mayEqualLiveDefs.size()==0));

    // If def must-equals some group, replace the group's original set of SSAMemLocs with the new definition
    if(mustEqualLiveDefs.size()==1) {
      (*mustEqualLiveDefs.begin())->second.clear();
      (*mustEqualLiveDefs.begin())->second.insert(*def);
    }

    // If this def fits into no groups in curLiveDefs, add a new one
    else if(mayEqualLiveDefs.size()==0) {
      set<SSAMemLocObjectPtr> defSet; defSet.insert(*def);
      curLiveDefs.push_back(make_pair((*def)->baseML->copyAOType(), set<SSAMemLocObjectPtr>(defSet)));

    // Otherwise, if this def fits into exactly one group in curLiveDefs, add it there
    } else if(mayEqualLiveDefs.size()==1) {
      (*mayEqualLiveDefs.begin())->first->meetUpdate((*def)->baseML, pedge, comp, analysis);
      (*mayEqualLiveDefs.begin())->second.insert(*def);

    // Otherwise, if this def may-equals multiple groups in curLiveDefs, merge the groups
    // and add the def to the merged group
    } else {
      // Points to the element of curLiveDefs into which the others will be merged
      list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator>::iterator target = mayEqualLiveDefs.begin();
      list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator>::iterator m=target; ++m;
      for(; m!=mayEqualLiveDefs.end(); ++m) {
        (*target)->first->meetUpdate((*m)->first, pedge, comp, analysis);
        for(set<SSAMemLocObjectPtr>::iterator mergeD=(*m)->second.begin(); mergeD!=(*m)->second.end(); ++mergeD)
          (*target)->second.insert(*mergeD);
        // Having merged the current group into the target group, erase it from curLiveDefs
        curLiveDefs.erase(*m);
      }

      // Merge def with the target group
      (*target)->first->meetUpdate((*def)->baseML, pedge, comp, analysis);
      (*target)->second.insert(*def);
    }
  }
}

// Merge the two given lists of live definition groups, updating toLiveDefs with the result
// Returns true if toLiveDefs is modified and false otherwise.
bool SSAGraph::mergeLiveDefs(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >& toLiveDefs,
                             list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >& fromLiveDefs,
                             PartEdgePtr pedge) {
  SIGHT_VERB_DECL(scope, ("mergeLiveDefs"), 3, atsDebugLevel)
  bool modified = false;
  for(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator from=fromLiveDefs.begin(); from!=fromLiveDefs.end(); ++from) {
/*    // Merge may only be called when propagating liveDefs to successor Parts at the end of a transfer
    // function. Since at this point we've already done detection ambiguities and placement of phi nodes
    // to deal with these ambiguities, we're sure that each def grouping has no ambiguity and thus,
    // exactly 1 member.
    dbg << "from->first="<<from->first->str()<<endl;
    BOOST_FOREACH(const SSAMemLocObjectPtr& f, from->second) {
      dbg << "    f="<<f->str()<<endl;
    }

    ROSE_ASSERT(from->second.size()==1);*/

    //SSAMemLocObjectPtr fromDef = *from->second.begin();
    //SIGHT_VERB_DECL(scope, (txt()<<"fromDef="<<fromDef->str()), 3, atsDebugLevel)

    // Find the groups in toLiveDefs that from overlaps. Since there may be multiple,
    // we collect them all into matchingDefs
    list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator> matchingToLiveDefs;
    for(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator liveDef=toLiveDefs.begin(); liveDef!=toLiveDefs.end(); ++liveDef) {
      if(liveDef->first->mayEqual(from->first, pedge, comp, analysis)) {
        matchingToLiveDefs.push_back(liveDef);
      }
    }
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "#matchingToLiveDefs="<<matchingToLiveDefs.size()<<endl, 3, atsDebugLevel)
#endif

    // If this group in fromLiveDefs matches no groups in toLiveDefs, add a new one
    if(matchingToLiveDefs.size()==0) {
      toLiveDefs.push_back(make_pair(from->first->copyAOType(), from->second));
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "Adding new group "<<from->first->str(), 3, atsDebugLevel)
#endif
      modified = true;

    // Otherwise, if this group in fromLiveDefs matches exactly one group in toLiveDefs, add its SSAMemLocObjects to it
    } else if(matchingToLiveDefs.size()==1) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "Adding reaching defs to existing group "<<from->first->str(), 3, atsDebugLevel)
#endif

#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(3, atsDebugLevel)
      scope s("matchingToLiveDefs Before");
      for(set<SSAMemLocObjectPtr>::iterator match=(*matchingToLiveDefs.begin())->second.begin(); match!=(*matchingToLiveDefs.begin())->second.end(); ++match) {
        dbg << (*match)->str()<<endl;
      }
      SIGHT_VERB_FI()
#endif
/*
      // The elements in the matching set in toLiveDefs may relate to fromDef in two ways:
      // - fromDef is a replacement for the matching mapping (it is a phi node that the to mapping reaches
      //   on its way to this Part), or
      // - fromDef is an alternative to the matching mapping (both defs each this Part without hitting a phi node)
      // The loop below replaces any mappings in the first category with fromDef but leaves
      // all mappings in the second category in toLiveDefs.
      for(set<SSAMemLocObjectPtr>::iterator match=(*matchingToLiveDefs.begin())->second.begin(); match!=(*matchingToLiveDefs.begin())->second.end(); ) {
        // If the current match terminates at from
        if(fromDef->getAccess()==SSAMemLocObject::phiDef &&
           defTerminatesAtPhiNode(fromDef->loc, *match)) {
          SIGHT_VERB(dbg << "    Current match terminated at fromDef match="<<(*match)->str()<<endl, 3, atsDebugLevel)

          // We'll replace match with from in (*matchingToLiveDefs.begin())->second, so remove match and advance
          (*matchingToLiveDefs.begin())->second.erase(match++);
          modified = true;

        // If the current match is an alternative to from, just advance the match iterator
        } else
          ++match;
      }

      SIGHT_VERB_IF(3, atsDebugLevel)
      scope s("matchingToLiveDefs After");
      for(set<SSAMemLocObjectPtr>::iterator match=(*matchingToLiveDefs.begin())->second.begin(); match!=(*matchingToLiveDefs.begin())->second.end();  ++match) {
        dbg << (*match)->str()<<endl;
      }
      SIGHT_VERB_FI()*/

      // Finally, add all the definitions in from to (*matchingToLiveDefs.begin())->second
      unsigned int origSize=(*matchingToLiveDefs.begin())->second.size();
      BOOST_FOREACH(const SSAMemLocObjectPtr& fromDef, from->second) {
        (*matchingToLiveDefs.begin())->second.insert(fromDef);
      }
      modified = modified || (origSize!=(*matchingToLiveDefs.begin())->second.size());
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "    modified="<<modified<<", origSize="<<origSize<<", (*matchingToLiveDefs.begin())->second.size()="<<(*matchingToLiveDefs.begin())->second.size(), 3, atsDebugLevel)
#endif

    // Otherwise, if this group in fromLiveDefs matches multiple groups in toLiveDefs, merge the groups in toLiveDefs
    // and add the from group to the merged group
    } else {
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "Merging multiple to groups", 3, atsDebugLevel)
#endif
      // Points to the element of curLiveDefs into which the others will be merged
      list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator>::iterator target = matchingToLiveDefs.begin();
      list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator>::iterator m=target; ++m;
      for(; m!=matchingToLiveDefs.end(); ++m) {
        (*target)->first->meetUpdate((*m)->first, pedge, comp, analysis);
        BOOST_FOREACH(const SSAMemLocObjectPtr& mergeD, (*m)->second) {
          (*target)->second.insert(mergeD);
        }
        // Having merged the current group in toLiveDefs into the target group, erase it from toLiveDefs
        toLiveDefs.erase(*m);
      }

      // Merge the from group with the target to group
      (*target)->first->meetUpdate(from->first, pedge, comp, analysis);
      BOOST_FOREACH(const SSAMemLocObjectPtr& fromLiveDef, from->second) {
        (*target)->second.insert(fromLiveDef);
      }

      modified = true;
    }
  }
  return modified;
}

void printLiveDefs(std::string label, const list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >& curLiveDefs) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(2, atsDebugLevel)
  { scope s(label);
  for(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::const_iterator group=curLiveDefs.begin(); group!=curLiveDefs.end(); ++group) {
    scope s(txt()<<"Group "<<group->first->str());
    for(set<SSAMemLocObjectPtr>::iterator d=group->second.begin(); d!=group->second.end(); ++d) {

      dbg << (*d)->str()<<endl;
    }
  } }
  SIGHT_VERB_FI()
#endif
}

// Returns whether the two collections of live defs contain the same defs
bool SSAGraph::equalLiveDefs(const list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >& aLD,
                             const list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >& bLD)
{
  if(aLD.size() != bLD.size()) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "different sizes;"<<endl, 3, atsDebugLevel)
#endif
      return false;
  }

  list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::const_iterator a=aLD.begin(), b=bLD.begin();
  for(; a!=aLD.end(); ++a, ++b) {
    if(a->second.size() != b->second.size()) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "different sub-size in a->first="<<a->first->str()<<endl, 3, atsDebugLevel)
#endif
        return false;
    }

    set<SSAMemLocObjectPtr>::const_iterator aDef=a->second.begin(), bDef=b->second.begin();
    for(; aDef!=a->second.end(); ++aDef, ++bDef) {
      if(*aDef != *bDef) {
#ifndef DISABLE_SIGHT
        SIGHT_VERB(dbg << "different SSAML aDef="<<(*aDef)->str()<<", bDef="<<(*bDef)->str()<<endl, 3, atsDebugLevel)
#endif
          return false; }
    }
  }

#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "equal"<<endl, 3, atsDebugLevel)
#endif

    return true;
}

// Computes the mapping from MemLoc uses to their defs
void SSAGraph::computeUse2Def() {
#ifndef DISABLE_SIGHT
  SIGHT_DECL(module, (instance("computeUse2Def", 0, 0)), moduleProfile)
  SIGHT_VERB_DECL(scope, ("computeUse2Def"), 3, atsDebugLevel)
#endif

  // Maps each Part to set of MemLocs that were defined earlier than the Part and that
  // we're keeping track of to identify their definition points. The MemLocs are organized
  // according to the sets they denote. All SSAMemLocObjects that overlap each other (even transitively)
  // are grouped together into a set and connected via a pair with the MemLocObject (implemented
  // by the server analysis) that holds their union. liveDefs maintains a list of such
  // pairs and all the MemLocObjects at different elements in the list are guaranteed to
  // denote disjoint sets.
  // We separately keep track of the live definitions at the starting/incoming point of the
  // Part (inLiveDefs) and ending/outgoing point (outLiveDefs).
  map<PartPtr, list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > > > inLiveDefs,
                                                                        outLiveDefs;
  // Keeps track of liveDefs on all the incoming edges of each Part separately. During
  // the workflow algorithm we first propagate the outLiveDefs of one Part to each outgoing
  // edge and then separately merge the incoming liveDefs sets of each node. Since we
  // keep the sets around on each incoming edge we don't lose information about which path
  // a given definition came from, which is important for correctly replacing propagating
  // definitions with phiDefs that are subsequently computed.
  map<PartEdgePtr, list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > > > edgeLiveDefs;

  // Keeps track of the nodes that have been visited so far. Important since we visit successors
  // if each node if their incoming info is updated OR they've not yet been visited.
  set<PartPtr> visited;

  // Iterate over the ATS forwards, looking for propagating defs to their corresponding phiDefs and uses
  set<PartPtr> startStates = comp->GetStartAStates(analysis);

  //for(fw_partEdgeIterator state(startStates, /*incrementalGraph*/ false); !state.isEnd(); state++) {
  fw_dataflowPartEdgeIterator state(/*incrementalGraph*/ false, selectIterOrderFromEnvironment());
  set<PartPtr> empty;
  state.init(startStates, empty);
  int iter=0;
  while(!state.isEnd()) {
    PartEdgePtr pedge = state.getPartEdge();
    PartPtr part = state.grabPart();
    visited.insert(part);
    set<CFGNode> cfgNodes = part->CFGNodes();
    ROSE_ASSERT(cfgNodes.size()==1);
    CFGNode cn = *cfgNodes.begin();
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"part "<<part->str()), 3, atsDebugLevel)
#endif


#ifndef DISABLE_SIGHT
    module* iterMod;
#endif
    string label="";
    SIGHT_IF(moduleProfile)
    if(isSgVariableDeclaration(cn.getNode())) {
      const SgInitializedNamePtrList &  vars = isSgVariableDeclaration(cn.getNode())->get_variables();
      for(SgInitializedNamePtrList::const_iterator v=vars.begin(); v!=vars.end(); ++v) {
        if(v!=vars.begin()) label += ", ";
        label += (*v)->get_name().getString();
      }
    } else if(isSgFunctionCallExp(cn.getNode())) {
      if(isSgFunctionCallExp(cn.getNode())->getAssociatedFunctionSymbol())
        label = isSgFunctionCallExp(cn.getNode())->getAssociatedFunctionSymbol()->get_name().getString();
      else
        label = "unknown";
    } else if(isSgFunctionDefinition(cn.getNode())) {
      label = isSgFunctionDefinition(cn.getNode())->get_declaration()->get_name().getString();
    }

#ifndef DISABLE_SIGHT
    SIGHT_FI()
    SIGHT_DECL_REF(module, (instance("Iteration", 5, 0),
                            inputs(port(context("iter", iter,
                                                "part", part->str(),
                                                "class_name", cn.getNode()->class_name(),
                                                "label", label)))),
                   iterMod, moduleProfile)

    SIGHT_VERB(printLiveDefs("inLiveDefs", inLiveDefs[part]), 3, atsDebugLevel)
#endif

    // The liveDefs set we'll propagate to this part's successors
    // Initialize outLiveDefs from inLiveDefs
    outLiveDefs[part] = inLiveDefs[part];

    // Set curLiveDefs as a shorthand reference to outLiveDefs[part]
    list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >& curLiveDefs = outLiveDefs[part];
    //printLiveDefs("Initial curLiveDefs", curLiveDefs);

    // Erase all the uses and phi definitions at this part, if any, since we're about
    // to recompute them based on fresh reaching definition information.
    eraseUsesFromU2D(part);
    phiDefs[part].clear();

    // --------------------------------------
    // Phi nodes before this ATS node
    // --------------------------------------
    // If any grouping of defs in curLiveDefs contains more than one SSAMemLoc, there is ambiguity about
    // the definitions reaching this ATS node. We therefore place a phi node immediately before it.
    {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, ("Phi Node Detection"), 3, atsDebugLevel)
#endif
        
    // Collects iterators to all the def groups that are ambiguous
    list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator> ambiguousMemLocs;
    for(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator liveD=curLiveDefs.begin(); liveD!=curLiveDefs.end(); ++liveD) {
      if(liveD->second.size()>1) ambiguousMemLocs.push_back(liveD);
    }
    
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "#ambiguousMemLocs="<<ambiguousMemLocs.size()<<endl, 3, atsDebugLevel)
#endif

    // For each ambiguous grouping
    for(list<list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator>::iterator aml=ambiguousMemLocs.begin(); aml!=ambiguousMemLocs.end(); ++aml) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt()<<"Ambiguous: "<<(*aml)->first->str()), 3, atsDebugLevel)
#endif
      // Create a phiDef at this ATS node
      SSAMemLocObjectPtr phiDef = makePtr<SSAMemLocObject>((*aml)->first, part, (SgNode*)NULL, SSAMemLocObject::phiDef, phiDefs[part].size());
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "phiDef="<<phiDef->str()<<endl, 3, atsDebugLevel)
#endif
      phiDefs[part][phiDef] = (*aml)->second;

      // Replace the grouping in curLiveDefs with the MemLoc defined at this phiNode
      (*aml)->second.clear();
      (*aml)->second.insert(phiDef);
    }
#ifndef DISABLE_SIGHT
    SIGHT(iterMod->setInCtxt(1, context("numAmbiguousMLs", (int)ambiguousMemLocs.size(),
                                        "numPhiDefs",      (int)phiDefs[part].size())),
          moduleProfile)
    SIGHT_VERB(dbg << "#phiDefs[part]="<<phiDefs[part].size()<<endl, 3, atsDebugLevel)
#endif
    }

    // NOTE: We place phi nodes based on ambiguity about reaching defs and not based on control ambiguity.
    //       This is sound because:
    //       - We need to keep track of ambiguity regarding which sets of memory locations reach this ATS node
    //         for maintaining soundness with respect to defs to these locations
    //       - The only thing missed by this are the cases where there's a def on one path but not on another.
    //         In normal SSA we'd place an SSA node here but since the no-def case corresponds to an undefined
    //         value, we're free to assume any value for that set of memory locations. We thus ignore them and
    //         soundly pretend that the only possible values came from the defs on the paths that had them.

    // --------------------------------------
    // Computation on this ATS node
    // --------------------------------------

    // Find any uses at the current part that overlap the defs in curLiveDefs and match them to each other
    { SIGHT_VERB_DECL(scope, ("Matching uses to defs"), 3, atsDebugLevel)
    if(uses.find(part)!=uses.end()) {
      int numUsesConnected=0;
      for(set<SSAMemLocObjectPtr>::iterator u=uses[part].begin(); u!=uses[part].end(); u++) {
        SIGHT_VERB(dbg << "use="<<(*u)->str()<<endl, 3, atsDebugLevel)
        for(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator group=curLiveDefs.begin(); group!=curLiveDefs.end(); ++group) {
          SIGHT_VERB_DECL(scope, (txt()<<"Def group"<<group->first->str(), scope::low), 3, atsDebugLevel)
          if(group->first->mayEqual((*u)->baseML, pedge, comp, analysis)) {
            // We've processed any ambiguity at the phi def code above, so there should only be 1 SSAMemLocObject at each group
            ROSE_ASSERT(group->second.size()==1);

            // Connect the current definition to this use
            //addDefUseLink(*u, *group->second.begin());
            use2def[*u].insert(*group->second.begin());
            SIGHT(++numUsesConnected, moduleProfile)
            SIGHT_VERB(dbg << "Use Matched"<<endl, 3, atsDebugLevel)
          }
        }
      }

      SIGHT(iterMod->setInCtxt(2, context("numUsesConnected", numUsesConnected)),
            moduleProfile)
    }
    }

    // --------------------------------------
    // Deadness filtering
    // --------------------------------------
    // Remove from curLiveDefs all the MemLocs that are dead at this Part
    {
      int numDeadErased=0;
      for(list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > >::iterator group=curLiveDefs.begin(); group!=curLiveDefs.end();) {
        // We've processed any ambiguity at the phi def code above, so there should only be 1 SSAMemLocObject at each group
        ROSE_ASSERT(group->second.size()==1);

        if(!(group->first->isLive(pedge, comp, analysis))) {
          SIGHT_VERB(dbg << "erasing dead "<<group->first->str()<<endl, 3, atsDebugLevel)
          curLiveDefs.erase(group++);
          SIGHT(++numDeadErased, moduleProfile)
        } else
          ++group;
      }
      SIGHT(iterMod->setInCtxt(3, context("numDeadErased", numDeadErased)),
            moduleProfile)
    }

    // --------------------------------------
    // Add the defs at the current part into curLiveDefs
    // --------------------------------------
    assignDefAfterLiveDefs(curLiveDefs, defs[part], pedge);

//    printLiveDefs("curLiveDefs", curLiveDefs);

    //printLiveDefs("Final curLiveDefs", curLiveDefs);

    // Pass curLiveDefs to all of this part's successors
    list<PartEdgePtr> out = part->outEdges();
    if(part->mustOutgoingFuncCall()) {
      SIGHT_VERB_DECL(scope, ("Matching incoming call"), 3, atsDebugLevel)
      set<PartPtr> inCall = part->matchingCallParts();
      for(set<PartPtr>::iterator in=inCall.begin(); in!=inCall.end(); in++)
        out.push_back((*in)->inEdgeFromAny());
    }
    SIGHT(iterMod->setInCtxt(4, context("numOutEdges", (int)out.size())),
          moduleProfile)

    { SIGHT_VERB_DECL(scope, ("Successors"), 3, atsDebugLevel)
    for(list<PartEdgePtr>::iterator e=out.begin(); e!=out.end(); e++) {
      SIGHT_VERB(dbg << "succ "<<(*e)->target()->str()<<endl, 3, atsDebugLevel)

      // Propagate curLiveDefs to one of the incoming edges of the current successor
      edgeLiveDefs[*e] = curLiveDefs;

      // Re-merge the liveDefs info of the current successor based on all of its edges
      list<PartEdgePtr> succIn = (*e)->target()->inEdges();
      succIn.push_back((*e)->target()->inEdgeFromAny());
      list<pair<MemLocObjectPtr, set<SSAMemLocObjectPtr> > > newInLiveDefs;
      for(list<PartEdgePtr>::iterator inE=succIn.begin(); inE!=succIn.end(); ++inE) {
        SIGHT_VERB(dbg << "inE="<<(*inE)->str()<<endl, 3, atsDebugLevel)
        SIGHT_VERB(printLiveDefs(txt()<<"edgeLiveDefs[*inE]", edgeLiveDefs[*inE]), 3, atsDebugLevel)

        if(inE==succIn.begin()) newInLiveDefs = edgeLiveDefs[*inE];
        else mergeLiveDefs(newInLiveDefs, edgeLiveDefs[*inE], *inE);
      }
//      printLiveDefs(txt()<<"newInLiveDefs", newInLiveDefs);
//      printLiveDefs(txt()<<"inLiveDefs["<<(*e)->target()->str()<<"]", inLiveDefs[(*e)->target()]);

      bool modified = !equalLiveDefs(inLiveDefs[(*e)->target()], newInLiveDefs);
      inLiveDefs[(*e)->target()] = newInLiveDefs;

      SIGHT_VERB(dbg << "modified="<<modified<<endl, 3, atsDebugLevel)

      //bool modified = mergeLiveDefs(inLiveDefs[(*e)->target()], curLiveDefs, pedge);

      //printLiveDefs(string(txt()<<"liveDefs modified="<<modified<<", target="<<(*e)->target()->str()), liveDefs[(*e)->target()]);

      // If the current successor's incoming info has changed or we've not visited it yet,
      // add it to the worklist
      if(modified || (visited.find((*e)->target())==visited.end()))
        state.add(*e);
    }
    }

    /*if(part->mustOutgoingFuncCall()) {
      SIGHT_VERB_DECL(scope, ("Matching incoming call"), 3, atsDebugLevel)
      set<PartPtr> inCall = part->matchingCallParts();
      for(set<PartPtr>::iterator in=inCall.begin(); in!=inCall.end(); in++) {
        SIGHT_VERB(dbg << "in "<<in->str()<<endl, 3, atsDebugLevel)

        bool modified = mergeLiveDefs(inLiveDefs[*in], curLiveDefs, pedge);

        //printLiveDefs(string(txt()<<"liveDefs modified="<<modified<<", target="<<(*in)->str()), liveDefs[(*in)]);

        // If the current successor's incoming info has changed or we've not visited it yet,
        // add it to the worklist
        if(modified || (visited.find((*in))==visited.end()))
          state.add((*in)->inEdgeFromAny());
      }
    }*/
    ++iter;
  }

  // During the iteration we set use2def and phiDefs, which connect uses to definitions. Now
  // set def2use (the reverse mapping) based on these
  setDef2Use();
}

/*// Computes the mapping from MemLoc uses to their defs
void SSAGraph::computeUse2Def() {
  SIGHT_VERB_DECL(scope, ("computeUse2Def"), 2, atsDebugLevel)

  // Maps each Part to set of MemLocs that were used later than the Part and that
  // we're keeping track of to identify their definition points.
  map<PartPtr, set<SSAMemLocObjectPtr> > liveUses;

  // Maps each Part to the set of Functions that we just exited (if any). This is important
  // for connecting the arguments of each function call to the parameters of all the
  // functions that the call may invoke. We may place multiple functions in this set if
  // we reach their SgFunctionParameterLists before call that targets them all.
  map<PartPtr, set<Function> > exitedFuncs;
  // Records the SSAMemLocObject uses of all the parameters of the function in exitFunc
  // that are currently live.
  map<PartPtr, map<Function, list<set<SSAMemLocObjectPtr> > > > exitedFParamsUses;

  // Iterate over the ATS backwards looking for uses and propagating them to their defs
  set<PartPtr> endStates = comp->GetEndAStates(analysis);

  for(bw_partEdgeIterator state(endStates); !state.isEnd(); state++) {
    PartPtr part = state.getPart();
    set<CFGNode> cfgNodes = part->CFGNodes();
    ROSE_ASSERT(cfgNodes.size()==1);
    CFGNode cn = *cfgNodes.begin();
    SIGHT_VERB_DECL(scope, (txt()<<"part "<<part->str()), 2, atsDebugLevel)

    // The liveUses set we'll propagate to this part's predecessors
    set<SSAMemLocObjectPtr> predLiveUses;

    // Initialize predLiveUses to be the set assigned to this node and erase the liveUses
    // mapping for this part
    if(liveUses.find(part) != liveUses.end()) {
      predLiveUses = liveUses[part];
      liveUses.erase(part);
    }

    // Remove from liveUses any that have been defined at the current part
    SIGHT_VERB(dbg << "Removing def-ed uses"<<endl, 2, atsDebugLevel)
    for(set<SSAMemLocObjectPtr>::iterator u=predLiveUses.begin(); u!=predLiveUses.end(); ) {
      set<SSAMemLocObjectPtr>::iterator nextU=u; ++nextU;

      SIGHT_VERB_DECL(scope, (txt()<<"Use "<<(*u)->str(), scope::low), 2, atsDebugLevel)
      bool curErased=false;
      if(defs.find(part)!=defs.end()) {
        for(set<SSAMemLocObjectPtr>::iterator d=defs[part].begin(); d!=defs[part].end(); d++) {
          SIGHT_VERB(dbg << "def="<<(*d)->str()<<endl, 2, atsDebugLevel)
          curErased = matchUseToDef(state.getPartEdge(), *u, *d, predLiveUses);
          if(curErased) goto U_OVERWRITTEN;
        }
      }

      if(phiMLs.find(part)!=phiMLs.end()) {
        SIGHT_VERB(dbg << "Phi Node"<<endl, 2, atsDebugLevel)
        for(set<SSAMemLocObjectPtr>::iterator ml=phiMLs[part].begin(); ml!=phiMLs[part].end(); ml++) {
          SIGHT_VERB(dbg << "phiAccess="<<(*ml)->str()<<endl, 2, atsDebugLevel)
          curErased = matchUseToDef(state.getPartEdge(), *u, *ml, predLiveUses);
          if(curErased) goto U_OVERWRITTEN;
        }
      }

      if(funcArgDefs.find(part)!=funcArgDefs.end()) {
        SIGHT_VERB(dbg << "Func Argument Defs"<<endl, 2, atsDebugLevel)
        for(list<SSAMemLocObjectPtr>::iterator d=funcArgDefs[part].begin(); d!=funcArgDefs[part].end(); d++) {
          SIGHT_VERB(dbg << "argument="<<(*d)->str()<<endl, 2, atsDebugLevel)
          curErased = matchUseToDef(state.getPartEdge(), *u, *d, predLiveUses);
          if(curErased) goto U_OVERWRITTEN;
        }
      }

      U_OVERWRITTEN:
      u = nextU;
    }

    // If this is a function call, connect the defs at its arguments to the uses at the parameter lists
    // of the functions that the call may invoke. We'll add the argument uses at this call to predLiveUses below.
    if(part->mustOutgoingFuncCall()) {
      SgFunctionCallExp* call = isSgFunctionCallExp(cn.getNode());
      Function func(call);
      SgExprListExp* args = call->get_args();

      ROSE_ASSERT(args->get_expressions().size() == exitedFParamsUses[part][func].size());
      SgExpressionPtrList::iterator arg = args->get_expressions().begin();
      list<set<SSAMemLocObjectPtr> >::iterator param = exitedFParamsUses[part][func].begin();
      for(; arg!=args->get_expressions().end(); ++arg, ++param) {
        for(set<SSAMemLocObjectPtr>::iterator ml=param->begin(); ml!=param->end(); ++ml)
          addDefUseLink(*ml,
                        makePtr<SSAMemLocObject>(comp->Expr2MemLoc(*arg, state.getPartEdge(), analysis), part, *arg, SSAMemLocObject::def));
      }
    }

    // Remove from predLiveUses all the MemLocs that are dead at this Part
    for(set<SSAMemLocObjectPtr>::iterator u=predLiveUses.begin(); u!=predLiveUses.end(); ) {
      if(!((*u)->baseML->isLive(state.getPartEdge(), comp, analysis))) {
        SIGHT_VERB(dbg << "erasing dead "<<(*u)->str()<<endl, 2, atsDebugLevel)
        predLiveUses.erase(u++);
      } else
        ++u;
    }

    // Add the uses at the current part into predLiveUses
    for(set<SSAMemLocObjectPtr>::iterator u=uses[part].begin(); u!=uses[part].end(); u++)
      predLiveUses.insert(*u);

    // If this is a phi node, add all the defs at this node as uses
    / *if(phiDefs.find(part)!=phiDefs.end()) {
      for(set<MemLocObjectPtr>::iterator d=phiDefs[part].begin(); d!=phiDefs[part].end(); d++) {
        predLiveUses.insert(makePtr<SSAMemLocObject>(*d, part, SSAMemLocObject::use));
      }
    }* /

    // Unset the exitedFunc at function call sites since each call site is directly connected to
    // all the functions that it may invoke. We add the function to exitedFuncs at their SgFunctionParameterList
    // and when we then hit their corresponding SgFunctionCallExp we can immediately remove them
    if(part->mustOutgoingFuncCall())
      exitedFuncs[part].clear();

    // At SgFunctionParameterLists add Functions to exitedFuncs
    if(SgFunctionParameterList* params = isSgFunctionParameterList(cn.getNode())) {
      Function func(params);
      exitedFuncs[part].insert(func);

      exitedFParamsUses[part][func].clear();
      ROSE_ASSERT(params->get_args().size() == funcParamUses[func].size());

      list<MemLocObjectPtr>::iterator pUse = funcParamUses[func].begin();
      SgInitializedNamePtrList::const_iterator param=params->get_args().begin();
      for(; param!=params->get_args().end(); ++param, ++pUse) {
        SgType* typeParam = (*param)->get_type();

        // Skip "..." types, which are used to specify VarArgs.
        // NEED TO SUPPORT VAR ARGS MORE EFECTIVELY IN THE FUTURE
        if(isSgTypeEllipse(typeParam)) continue;

        // Pass by reference or value
        //if(isSgReferenceType(typeParam)) {
        set<SSAMemLocObjectPtr> usesOfParam;

        // Find any uses that mayEqual the current parameter and add them under this parameter in exitedFParamsUses
        for(set<SSAMemLocObjectPtr>::iterator u=predLiveUses.begin(); u!=predLiveUses.end(); ++u) {
          if((*pUse)->mayEqual((*u)->baseML, state.getPartEdge(), comp, analysis)) {
            usesOfParam.insert(*u);
          }
        }
        exitedFParamsUses[part][func].push_back(usesOfParam);
      }
    }


    SIGHT_VERB_IF(2, atsDebugLevel)
    { scope s("predLiveUses");
    for(set<SSAMemLocObjectPtr>::iterator u=predLiveUses.begin(); u!=predLiveUses.end(); ++u) {
      dbg << (*u)->str()<<endl;
    } }
    SIGHT_VERB_FI()

    // Pass predLiveUses and exitedFParamsUses to all of this part's predecessors
    list<PartEdgePtr> in = part->inEdges();
    { SIGHT_VERB_DECL(scope, ("predecessors"), 2, atsDebugLevel)
    for(list<PartEdgePtr>::iterator e=in.begin(); e!=in.end(); e++) {
      SIGHT_VERB(dbg << "pred "<<(*e)->source()->str()<<endl, 2, atsDebugLevel)
      liveUses[(*e)->source()] = predLiveUses;
      exitedFParamsUses[(*e)->source()] = exitedFParamsUses[part];
    }
    liveUses.erase(part);
    exitedFParamsUses.erase(part);
    }
  }
}*/

void SSAGraph::showUse2Def() {
  ostringstream dot;

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(2, atsDebugLevel)
  {
    scope s1("use2def");
  for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr>  >::iterator i=use2def.begin(); i!=use2def.end(); i++) {
    scope s(i->first->str());
    for(set<SSAMemLocObjectPtr>::iterator j=i->second.begin(); j!=i->second.end(); j++)
      dbg << "    "<<(*j)->str()<<endl;
  }}
#endif
#ifndef DISABLE_SIGHT
  { scope s2("def2use");
  for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr>  >::iterator i=def2use.begin(); i!=def2use.end(); i++) {
    //dbg << "i->first="<<i->first.get()<<endl;
    scope s(i->first->str());
    for(set<SSAMemLocObjectPtr>::iterator j=i->second.begin(); j!=i->second.end(); j++)
      dbg << "    "<<(*j)->str()<<endl;
  } }
  SIGHT_VERB_FI()
#endif

  dot << "digraph Use2Def {"<<endl;

  // First, assign unique nodeIDs to all parts in the ATS and emit node descriptions for all parts
  set<PartPtr> startStates = comp->GetStartAStates(analysis);
  std::map<PartPtr, int> partID;
  set<PartPtr> visited;
  for(fw_partEdgeIterator state(startStates, /*incrementalGraph*/ false); !state.isEnd(); state++) {
    PartPtr part = state.getPart();
    if(visited.find(part) != visited.end()) continue;
    visited.insert(part);

    int ID=partID.size();
    dot << "node"<<ID<<" [label=\""<<part->str()<<"\"";
    if(isPhiNode(part))
      dot << " color=red"<<endl;
    dot << "];"<<endl;
    partID[part] = ID;
  }

  // Next, add transition system edges
  /*for(fw_partEdgeIterator state(startStates, / *incrementalGraph* / false); !state.isEnd(); state++) {
    PartPtr src = state.getPartEdge()->source();
    PartPtr tgt = state.getPartEdge()->target();
    dot << "node"<<partID[src]<<" -> node"<<partID[tgt]<<";"<<endl;
  }*/
  {
    set<PartPtr> visited;
    for(fw_partEdgeIterator state(startStates, /*incrementalGraph*/ false); !state.isEnd(); state++) {
      PartPtr part = state.getPart();
      if(visited.find(part) != visited.end()) continue;
      visited.insert(part);

      list<PartEdgePtr> out = part->outEdges();
      for(list<PartEdgePtr>::iterator e=out.begin(); e!=out.end(); e++) {
        dot << "node"<<partID[part]<<" -> node"<<partID[(*e)->target()]<<";"<<endl;
      }
    }
  }

  // Add edges for def-use relations
  for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::iterator i=use2def.begin(); i!=use2def.end(); ++i) {
    for(set<SSAMemLocObjectPtr>::iterator j=i->second.begin(); j!=i->second.end(); ++j) {
      string mlLabel = (*j)->str();
      // Replace all the line breaks in mlLabel with explicit text "\n";
      ReplaceStringInPlace(mlLabel, "\n", "\\n");
      dot << "node"<<partID[(*j)->loc]<<" -> node"<<partID[i->first->loc]<<" [label=\""<<mlLabel<<"\" color=blue];"<<endl;
    }
  }

  // Add edges for def-phiDef relations
  for(map<PartPtr, map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > >::const_iterator part=phiDefs.begin(); part!=phiDefs.end(); ++part) {
    for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::const_iterator phiD=part->second.begin(); phiD!=part->second.end(); ++phiD) {
      for(set<SSAMemLocObjectPtr>::const_iterator def=phiD->second.begin(); def!=phiD->second.end(); ++def) {
        string mlLabel = phiD->first->str();
        // Replace all the line breaks in mlLabel with explicit text "\n";
        ReplaceStringInPlace(mlLabel, "\n", "\\n");
        dot << "node"<<partID[(*def)->loc]<<" -> node"<<partID[phiD->first->loc]<<" [label=\""<<mlLabel<<"\" color=green];"<<endl;
      }
    }
  }

  dot << "}"<<endl;

#ifndef DISABLE_SIGHT
  sight::structure::graph g(dot.str());
#endif
}

void SSAGraph::buildSSA() {
  if(ssaBuilt) return;
  ssaBuilt=true;

  struct timeval start, end;
  gettimeofday(&start, NULL);

  {
  SIGHT_DECL(module, (instance("buildSSA", 0, 0)), moduleProfile)

  SIGHT_VERB_DECL(scope, ("buildSSA", scope::high), 2, atsDebugLevel)

  /*getDominatorTree();
  SIGHT_VERB_IF(2, atsDebugLevel)
  scope s("Dominator Tree");
  showDominatorTree();
  SIGHT_VERB_FI()

  calculateDominanceFrontiers();
  SIGHT_VERB_IF(2, atsDebugLevel)
  scope s("Dominance Frontier");
  showDominanceFrontier();
  SIGHT_VERB_FI()*/

  collectDefsUses();
  SIGHT_VERB_IF(2, atsDebugLevel)
    
#ifndef DISABLE_SIGHT
  scope s("DefsUses");
#endif

  showDefsUses();
  SIGHT_VERB_FI()

  /*placePhiNodes();
  SIGHT_VERB_IF(2, atsDebugLevel)
  scope s("Phi Nodes");
  showPhiNodes();
  SIGHT_VERB_FI()*/

  computeUse2Def();
  SIGHT_VERB_IF(1, atsDebugLevel)

#ifndef DISABLE_SIGHT
  scope s("Use2Def");
#endif

  showUse2Def();
  showPhiNodes();
  SIGHT_VERB_FI()
  }

  gettimeofday(&end, NULL);
  cout << "      Build SSA: Elapsed="<<((end.tv_sec*1000000+end.tv_usec) -
                                               (start.tv_sec*1000000+start.tv_usec))/1000000.0<<"s"<<endl;
}

}; // namespace fuse
