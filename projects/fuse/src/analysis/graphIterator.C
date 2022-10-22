#include "sage3basic.h"
using namespace std;

#include "graphIterator.h"
#include <boost/make_shared.hpp>

#ifndef DISABLE_SIGHT
#include "sight.h"
using namespace sight;
#endif

#include <list>
#include <vector>
#include <set>
#include <string>
#include <iostream>

namespace fuse {

// Checks the environment for any variable that specified the best iteration order to use for dataflow analysis and returns it
graphIterOrderT selectIterOrderFromEnvironment() {
  if(getenv("FUSE_DF_ITER_ORDER")) {
    if(strcmp(getenv("FUSE_DF_ITER_ORDER"), "succ_front")==0) return succ_front;
    if(strcmp(getenv("FUSE_DF_ITER_ORDER"), "succ_back")==0)  return succ_back;
    if(strcmp(getenv("FUSE_DF_ITER_ORDER"), "topo_order")==0) return topo_order;
    if(strcmp(getenv("FUSE_DF_ITER_ORDER"), "random")==0)     return random;
  }
  // If nothing intelligible is specified, default to succ_front
  return topo_order;
}

/****************************************
 ********** PART_EDGE_ITERATOR **********
 ****************************************/

template <class GraphEdgePtr, class GraphNodePtr>
graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::graphEdgeIterator(bool incrementalGraph,
                                                                 iterDirection dir, graphIterOrderT iterOrder,
                                                                 bool initialized):
    incrementalGraph(incrementalGraph), dir(dir), iterOrder(iterOrder), initialized(initialized)
{
  worklist = createWorklist(iterOrder);
}

template <class GraphEdgePtr, class GraphNodePtr>
graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::graphEdgeIterator(const GraphNodePtr start,
                                                                 bool incrementalGraph,
                                                                 iterDirection dir, graphIterOrderT iterOrder) :
    incrementalGraph(incrementalGraph), dir(dir), iterOrder(iterOrder), initialized(false)
{
  worklist = createWorklist(iterOrder);
  init(start);
}

template <class GraphEdgePtr, class GraphNodePtr>
graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::graphEdgeIterator(const set<GraphNodePtr>& start,
                                                                 bool incrementalGraph,
                                                                 iterDirection dir, graphIterOrderT iterOrder) :
    incrementalGraph(incrementalGraph), dir(dir), iterOrder(iterOrder), initialized(false)
{
  worklist = createWorklist(iterOrder);
  init(start);
}

template <class GraphEdgePtr, class GraphNodePtr>
graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::graphEdgeIterator(const graphEdgeIterator& that) :
      incrementalGraph(that.incrementalGraph),
      dir(that.dir),
      iterOrder(that.iterOrder),
      initialized(that.initialized),
      worklist(that.worklist->copy()),
      visited(that.visited)
{}

// Creates a new worklist implementation that follows the given iteration order and returns a pointer to it.
template <class GraphEdgePtr, class GraphNodePtr>
boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::createWorklist(graphIterOrderT iterOrder) {
  switch(iterOrder) {
    case succ_front: return boost::make_shared<GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr> >(GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::front, this);
    case succ_back:  return boost::make_shared<GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr> >(GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::back,  this);
    case topo_order: return boost::make_shared<GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr> >(this);
    case random:     return boost::make_shared<GERandomIteratorWorklist   <GraphEdgePtr, GraphNodePtr> >(this);
    default: ROSE_ASSERT(0);
  }
}

template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(const GraphNodePtr start) {
  ROSE_ASSERT(!initialized);
  initialized     = true;

  // Perform any initialization tasks the worklist object requires now that the starting edges are known
  set<GraphEdgePtr> startEdges;
  startEdges.insert(Part2PartEdge(start));
  worklist->initGivenStart(startEdges);

  //GraphEdgePtr startEdge = Part2PartEdge(start);
  //remainingNodes.push_front(startEdge);
  //visited.insert(startEdge);
  add(Part2PartEdge(start));
}

template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(const set<GraphNodePtr>& start) {
  ROSE_ASSERT(!initialized);
  initialized     = true;

  // Perform any initialization tasks the worklist object requires now that the starting edges are known
  set<GraphEdgePtr> startEdges;
  for(typename set<GraphNodePtr>::const_iterator s=start.begin(); s!=start.end(); s++)
    startEdges.insert(Part2PartEdge(*s));
  worklist->initGivenStart(startEdges);

  for(typename set<GraphNodePtr>::const_iterator s=start.begin(); s!=start.end(); s++) {
    //cout << "s="<<(*s)->str()<<endl;
    add(Part2PartEdge(*s));
  }
}

template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(const set<GraphEdgePtr>& start) {
  ROSE_ASSERT(!initialized);
  initialized     = true;

  // Perform any initialization tasks the worklist object requires now that the starting edges are known
  worklist->initGivenStart(start);

  for(typename set<GraphEdgePtr>::const_iterator s=start.begin(); s!=start.end(); s++)
    add(*s);
}

// Given a part returns the wildcard edge pointing into it (for fw traversal) or out of it (for backward traversals
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::Part2PartEdge(GraphNodePtr part) const
{
  if(dir==fw) return part->inEdgeFromAny();
  else        return part->outEdgeToAny();
}

// Given a PartEdge returns the Part that points in the direction of this iterator's traversal (target for fw,
// source for bw)
template <class GraphEdgePtr, class GraphNodePtr>
GraphNodePtr graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::PartEdge2DirPart(GraphEdgePtr pedge) const
{
/*  scope s("PartEdge2DirPart");
  dbg << "pedge="<<(pedge? pedge->str(): "NULL")<<endl;
  dbg << "pedge-input="<<(pedge->getInputPartEdge()? pedge->getInputPartEdge()->str(): "NULL")<<endl;
  dbg << "dir==fw="<<(dir==fw)<<endl;*/

  if(dir==fw) return pedge->target();
  else        return pedge->source();
}

// Given a PartEdge returns the Part that points in the direction opposite to this iterator's traversal (source for fw,
// target for bw)
template <class GraphEdgePtr, class GraphNodePtr>
GraphNodePtr graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::PartEdge2AntiDirPart(GraphEdgePtr pedge) const
{
  if(dir==fw) return pedge->source();
  else        return pedge->target();
}

// Given a Part returns the list of Edges in the direction of this iterator's traversals (outEdges for fw, inEdges
// for bw).
template <class GraphEdgePtr, class GraphNodePtr>
list<GraphEdgePtr> graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::Part2DirEdges(GraphNodePtr part) const
{
  if(dir==fw) return part->outEdges();
  else        return part->inEdges();
}

// Given a Part returns the list of Edges in opposite of the direction of this iterator's traversals
// (inEdges for fw, outEdges for bw).
template <class GraphEdgePtr, class GraphNodePtr>
list<GraphEdgePtr> graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::Part2AntiDirEdges(GraphNodePtr part) const
{
  if(dir==fw) return part->inEdges();
  else        return part->outEdges();
}

/* // Returns true if the given GraphNodePtr  is in the remainingNodes list and false otherwise
template <class GraphEdgePtr, class GraphNodePtr>
bool graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::isRemaining(const GraphEdgePtr n)
{
  assert(initialized);
  for(typename list<GraphEdgePtr>::const_iterator it=remainingNodes.begin(); it!=remainingNodes.end(); it++)
  {
    // if in is currently n remainingNodes, say so
    if(*it == n) return true;
  }
  // n is not in remainingNodes
  return false;
}*/

// Find its followers (either successors or predecessors, depending on value of fwDir), push back
// those that have not yet been visited
template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::pushAllDescendants(GraphEdgePtr pedge)
{
  list<GraphEdgePtr> nextE = Part2DirEdges(this->PartEdge2DirPart(pedge));

  //dbg << "pushAllDescendants(): #nextE="<<nextE.size()<<endl;
  for(typename list<GraphEdgePtr>::iterator it=nextE.begin(); it!=nextE.end(); it++)
  {
    //dbg << "pushAllDescendants(): nextN="<<(*it ? it->get()->str(): "NullPartEdge")<<" visited="<<(visited.find(*it) != visited.end())<<endl;

    /* // if we haven't yet visited this node and don't yet have it on the remainingNodes list
    if(visited.find(*it) == visited.end() &&
       !isRemaining(*it))
    {
      remainingNodes.push_back(*it);
    }*/

    if(visited.find(*it) == visited.end()) {
      //dbg << "        Adding descendant "<<(*it? (*it)->str():"NULL")<<endl;
      worklist->add(*it);
    }
  }

  // If we still have any nodes left remaining
  if(worklist->size()>0)
  {
    // Take the next node from the front of the list and mark it as visited
    //visited.insert(remainingNodes.front());
    visited.insert(worklist->getNext());

    //dbg << "remainingNodes.front()=["<<remainingNodes.front().getNode()->unparseToString()<<" | "<<remainingNodes.front().getNode()->class_name()<<"]"<<endl;
  }
  // Since pushAllChildren always = true or = false, we only need to worry about managing visited in the true case
}

// Find its followers (either successors or predecessors, depending on value of fwDir), push back
// those that have not yet been visited
template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::pushAllDescendants() {
  //pushAllDescendants(remainingNodes.front());
  pushAllDescendants(worklist->getNext());
}

// Add the given PartEdge to the iterator's list of edges to follow
template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::add_internal(GraphEdgePtr next)
{
  // If this dataflow iterator is not initialized, initialize it now since it will now have real state
  if(!initialized) initialized = true;

  /* // If next is not currently in remainingNodes, add it
  if(!isRemaining(next))
  {
    typename set<GraphEdgePtr>::iterator nextLoc = visited.find(next);
    if(nextLoc != visited.end())
      visited.erase(visited.find(next));
    remainingNodes.push_back(next);
  }*/
  // Add the edge to the worklist and remove it from the visited set
  ROSE_ASSERT(worklist);
  worklist->add(next);
  visited.erase(next);
}

// Add the given PartEdge to the iterator's list of edges to follow
template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::add(GraphEdgePtr next) {
//cout << "graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::add() next="<<next->str()<<endl;
//cout << "PartEdge2DirPart(next)="<<(PartEdge2DirPart(next)? PartEdge2DirPart(next)->str(): "NULL")<<endl;
  // If the Part of the next PartEdge in the direction of iteration is not NULL, add it
  if(PartEdge2DirPart(next))
    add_internal(next);
  // Otherwise, if it is NULL (a wildcard), fill in the wildcard with all the edges going in this direction
  // and add them
  else {
    // At least one side of the edge must be non-NULL
    assert(PartEdge2AntiDirPart(next));
    //dbg << "graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::add() PartEdge2AntiDirPart(next)="<<(PartEdge2AntiDirPart(next)? PartEdge2AntiDirPart(next)->str(): "NULL")<<endl;
    list<GraphEdgePtr> edges = Part2DirEdges(PartEdge2AntiDirPart(next));
    //dbg << "graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::add() #edges="<<edges.size()<<endl;
    for(typename list<GraphEdgePtr>::iterator e=edges.begin(); e!=edges.end(); e++) {
      //dbg << "adding="<<e->get()->str()<<endl;
      add_internal(*e);
    }
  }
}

// Advances this iterator in the direction of motion.
// If pushAllChildren=true, all of the current node's unvisited children (predecessors or successors,
//    depending on fwDir) are pushed onto remainingNodes
// It is assumed that for a given iterator pushAllChildren either always = true or always = false.
template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::advance(bool pushAllChildren)
{
  assert(initialized);
  /*dbg << "graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::advance(pushAllChildren="<<pushAllChildren<<") #worklist="<<worklist->size()<<" worklist->getNext()="<<worklist->getNext()->str()<<endl;
  dbg <<"  visited=\n";
  for(typename set<GraphEdgePtr>::iterator it=visited.begin(); it!=visited.end(); it++)
    dbg << "      "<<(*it)->str()<<endl;*/
  //if(remainingNodes.size()>0)
  if(worklist->size()>0)
  {
    // pop the next CFG node from the front of the list
    /*GraphEdgePtr cur = remainingNodes.front();
    remainingNodes.pop_front();*/
    GraphEdgePtr cur = worklist->grabNext();
    //dbg << "cur="<<(cur? cur->str(): "NULLPart")<<endl;

    if(pushAllChildren)
      pushAllDescendants(cur);
  }
}

template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::operator ++ (int)
{
  assert(initialized);
  advance(true);
}

template <class GraphEdgePtr, class GraphNodePtr>
bool graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::eq(const graphEdgeIterator<GraphEdgePtr, GraphNodePtr>& that) const
{
  if(initialized != that.initialized) return false;

  // If both iterators are not initialized, they're equal
  if(!initialized) return true;

  //printf("iterator::eq() remainingNodes.size()=%d  that.remainingNodes.size()=%d\n", remainingNodes.size(), that.remainingNodes.size());
  /*if(remainingNodes.size() != that.remainingNodes.size()) return false;

  typename list<GraphEdgePtr>::const_iterator it1, it2;
  // look to ensure that every CFG node in that.remainingNodes appears in remainingNodes

  for(it1=remainingNodes.begin(); it1!=remainingNodes.end(); it1++)
  {
    for(it2=that.remainingNodes.begin(); it2!=that.remainingNodes.end(); it2++)
    {
      // if we found *it1 inside that.remainingNodes
      if(*it1 == *it2)
      {
        //printf("        (*it1 == *it2)\n");
        break;
      }
    }

    // the two iterators are not equal if ...

    // the current node in remainingNodes was not found in that.remainingNodes
    if(it2!=that.remainingNodes.end())
    {
      //printf("        it2!=that.remainingNodes.end()\n");
      return false;
    }

    // or the two nodes do not have the same visited status in both iterators
    if((visited.find(*it1) == visited.end()) !=
       (that.visited.find(*it1) == that.visited.end()))
    {
      //printf("        (visited.find(*it1) == visited.end()) != (that.visited.find(*it1) == that.visited.end())\n");
      return false;
    }
  }*/

  // Compare the visited sets
  if(visited != that.visited) return false;

  // Compare the worklists
  if(!worklist->eq(that.worklist)) return false;

  //printf("graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::eq: returning true\n");

  return true;
}

template <class GraphEdgePtr, class GraphNodePtr>
bool graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::operator==(const graphEdgeIterator& that) const
{
  return eq(that);
}

template <class GraphEdgePtr, class GraphNodePtr>
bool graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::operator!=(const graphEdgeIterator& it) const
{
  return !(*this == it);
}

template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::operator * ()
{
  assert(initialized);
  //return remainingNodes.front();
  return worklist->getNext();
}

// template <class GraphEdgePtr, class GraphNodePtr>
// PartEdge* graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::operator -> ()
// {
//   assert(initialized);
//   return (*(*this)).get();
// }

// Get the PartEdge that the iterator is currently is referring to
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::getPartEdge() const
{
  assert(initialized);
  return worklist->getNext();
}

// Grab the PartEdge that the iterator is currently is referring to and advance it to the following PartEdge
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::grabPartEdge()
{
  assert(initialized);
  return worklist->grabNext();
}


// Get the Part within the current PartEdge in the iterator's direction of motion (target for fw, source for bw)
template <class GraphEdgePtr, class GraphNodePtr>
GraphNodePtr graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::getPart() const
{
  assert(initialized);
  return this->PartEdge2DirPart(getPartEdge());
}

// Grab the Part within the current PartEdge in the iterator's direction of motion (target for fw, source for bw)
// to and advance the iterator to the following PartEdge
template <class GraphEdgePtr, class GraphNodePtr>
GraphNodePtr graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::grabPart()
{
  assert(initialized);
  return this->PartEdge2DirPart(grabPartEdge());
}

/*template <class GraphEdgePtr, class GraphNodePtr>
// Make sure that this iterator is initialized even though it is empty
graphEdgeIterator<GraphEdgePtr, GraphNodePtr> graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::endIter(fw, random, true);

template <class GraphEdgePtr, class GraphNodePtr>
const graphEdgeIterator<GraphEdgePtr, GraphNodePtr>& graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::end()
{
  cout << "::end returning "<<&endIter<<"="<<endIter.str() << endl;
  return endIter;
}*/

template <class GraphEdgePtr, class GraphNodePtr>
graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint::checkpoint(boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > worklist,
                                                                      const set<GraphEdgePtr>& visited)
{
  //this->remainingNodes = remainingNodes;
  this->worklist = worklist->copy();
  this->visited  = visited;
}

template <class GraphEdgePtr, class GraphNodePtr>
graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint::checkpoint(const graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint& that)
{
  //this->remainingNodes = that.remainingNodes;
  this->worklist = that.worklist->copy();
  this->visited  = that.visited;
}

template <class GraphEdgePtr, class GraphNodePtr>
string graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint::str(string indent) const
{
  ostringstream outs;
  outs << indent << "[graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint : "<<endl;
  /*for(typename list<GraphEdgePtr>::const_iterator it=remainingNodes.begin();
      it!=remainingNodes.end(); )
  {
    outs << indent << "&nbsp;&nbsp;&nbsp;&nbsp;"<<it->str();
    it++;
    if(it!=remainingNodes.end()) outs << endl;
  }*/
  worklist->print(outs);
  outs << "]";
  return outs.str();
}

// Returns a checkpoint of this iterator's progress.
template <class GraphEdgePtr, class GraphNodePtr>
typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::getChkpt()
{
  assert(initialized);
  typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint chkpt(worklist, visited);
  return chkpt;
}

// Loads this iterator's state from the given checkpoint.
template <class GraphEdgePtr, class GraphNodePtr>
void graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::restartFromChkpt(graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint& chkpt)
{
  //remainingNodes.clear();
  visited.clear();

  //remainingNodes = chkpt.remainingNodes;
  worklist = chkpt.worklist;
  visited = chkpt.visited;
  // The iterator must become initialized because it is only possible to take a checkpoints of initialized iterators
  initialized = true;
}

template <class GraphEdgePtr, class GraphNodePtr>
string graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::str(string indent) const
{
  ostringstream outs;

  if(initialized) {
    outs << "[graphEdgeIterator:"<<endl;
    /*outs << "&nbsp;&nbsp;&nbsp;&nbsp;remainingNodes(#"<<remainingNodes.size()<<") = "<<endl;
    for(typename list<GraphEdgePtr>::const_iterator it=remainingNodes.begin(); it!=remainingNodes.end(); it++)
    { outs << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"<<it->get()->str()<<endl; }*/
    worklist->print(outs);

    outs << "&nbsp;&nbsp;&nbsp;&nbsp;visited(#"<<visited.size()<<")  = \n";
    for(typename set<GraphEdgePtr>::const_iterator it=visited.begin(); it!=visited.end(); it++) {
      outs << "&nbsp;&nbsp;&nbsp;&nbsp;"<<it->get()->str()<<endl;
    }

    outs << "]";
  } else {
    outs << "[graphEdgeIterator: Uninitialized]";
  }

  return outs.str();
}

/************************************
 ******* fw_graphEdgeIterator *******
 ************************************/

template <class GraphEdgePtr, class GraphNodePtr>
fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw_graphEdgeIterator(bool incrementalGraph,
                                                                       graphIterOrderT iterOrder, bool initialized):
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder, initialized) {}

template <class GraphEdgePtr, class GraphNodePtr>
fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw_graphEdgeIterator(const GraphNodePtr start, bool incrementalGraph,
                                                                       graphIterOrderT iterOrder):
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(start, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder) { }

template <class GraphEdgePtr, class GraphNodePtr>
fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw_graphEdgeIterator(const std::set<GraphNodePtr>& start, bool incrementalGraph,
                                                                       graphIterOrderT iterOrder):
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(start, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder) { }

template <class GraphEdgePtr, class GraphNodePtr>
fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>
  fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::begin(const GraphNodePtr n)
{
  return fw_graphEdgeIterator(n, true, graphIterOrderT::succ_back);
}

/************************************
 ******* bw_graphEdgeIterator *******
 ************************************/

template <class GraphEdgePtr, class GraphNodePtr>
bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw_graphEdgeIterator(bool incrementalGraph,
                                                                       graphIterOrderT iterOrder, bool initialized):
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder, initialized) {}

template <class GraphEdgePtr, class GraphNodePtr>
bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw_graphEdgeIterator(const GraphNodePtr start, bool incrementalGraph,
                                                                       graphIterOrderT iterOrder):
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(start, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder) { }

template <class GraphEdgePtr, class GraphNodePtr>
bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw_graphEdgeIterator(const std::set<GraphNodePtr>& start, bool incrementalGraph,
                                                                       graphIterOrderT iterOrder):
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(start, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder) { }

template <class GraphEdgePtr, class GraphNodePtr>
bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>
  bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::begin(const GraphNodePtr n)
{
  return bw_graphEdgeIterator(n, true, graphIterOrderT::succ_back);
}

/***********************************
 ********** PART_ITERATOR **********
 *********************************** /

// The PartEdge-specific access operators should not be used with this iterator
GraphEdgePtr partIterator::operator * ()
{ assert(0); }

PartEdge* partIterator::operator -> ()
{ assert(0); }

GraphNodePtr partIterator::operator * ()
{
  assert(initialized);
  return PartEdge2DirPart(remainingNodes.front());
}

Part* partIterator::operator -> ()
{
  assert(initialized);
  return (*(*this))->get();
}*/

/****************************************
 ********** bw_PART_ITERATOR **********
 **************************************** /

// The PartEdge-specific access operators should not be used with this iterator
GraphEdgePtr bw_partIterator::operator * ()
{ assert(0); }

PartEdge* bw_partIterator::operator -> ()
{ assert(0); }

GraphNodePtr bw_partIterator::operator * ()
{
  assert(initialized);
  return PartEdge2DirPart(remainingNodes.front());
}

Part* bw_partIterator::operator -> ()
{
  assert(initialized);
  return (*(*this))->get();
}*/

/***********************************
 ********** DATAFLOW **********
 ***********************************/

template <class GraphEdgePtr, class GraphNodePtr>
dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::dataflowGraphEdgeIterator(
             bool incrementalGraph,
             typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder) :
    graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, dir, iterOrder, false)
{ }

template <class GraphEdgePtr, class GraphNodePtr>
dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::dataflowGraphEdgeIterator(
             const GraphNodePtr& terminator_arg,
             bool incrementalGraph,
             typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder) :
    graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, dir, iterOrder, false)
{
  terminators.insert(terminator_arg);
  // Record that the terminator has been visited to ensure that it is never analyzed
  //visited.insert(terminator_arg);
}

template <class GraphEdgePtr, class GraphNodePtr>
dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::dataflowGraphEdgeIterator(
              const GraphNodePtr& start, const GraphNodePtr& terminator_arg,
              bool incrementalGraph,
              typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder) :
    graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(start, incrementalGraph, dir, iterOrder)
{
  terminators.insert(terminator_arg);
  // Record that the terminator has been visited to ensure that it is never analyzed
  //visited.insert(terminator_arg);

  assert(start!=terminator_arg);
}

template <class GraphEdgePtr, class GraphNodePtr>
dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::dataflowGraphEdgeIterator(
              const std::set<GraphNodePtr>& terminators_arg,
              bool incrementalGraph,
              typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder) :
      graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, dir, iterOrder, true), terminators(terminators_arg)
{
  // Record that the terminator has been visited to ensure that it is never analyzed
  /*for(typename set<GraphNodePtr>::iterator i=terminators.begin(); i!=terminators.end(); i++)
    visited.insert(*i);*/
}

template <class GraphEdgePtr, class GraphNodePtr>
dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::dataflowGraphEdgeIterator(
              const GraphNodePtr& start, const std::set<GraphNodePtr>& terminators_arg,
              bool incrementalGraph,
              typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder):
    graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(start, incrementalGraph, dir, iterOrder), terminators(terminators_arg)
{
/*  // Record that the terminator has been visited to ensure that it is never analyzed
  for(typename set<GraphNodePtr>::iterator i=terminators.begin(); i!=terminators.end(); i++)
    visited.insert(*i);*/

  assert(terminators.find(start) == terminators.end());
}

template <class GraphEdgePtr, class GraphNodePtr>
void dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(const GraphNodePtr& start_arg, const GraphNodePtr& terminator_arg)
{
  terminators.insert(terminator_arg);
  // Use the init method to initialize the starting point to make sure that the object is recorded as being initialized
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(start_arg);

  assert(terminators.find(start_arg) == terminators.end());
}

template <class GraphEdgePtr, class GraphNodePtr>
void dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(const set<GraphNodePtr>& start_arg, const set<GraphNodePtr>& terminators_arg)
{
  terminators = terminators_arg;

  // Use the init method to initialize the starting point to make sure that the object is recorded as being initialized
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(start_arg);

  for(typename set<GraphNodePtr>::const_iterator s=start_arg.begin(); s!=start_arg.end(); s++)
    assert(terminators.find(*s) == terminators.end());
}

template <class GraphEdgePtr, class GraphNodePtr>
void dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(const GraphNodePtr& start_arg, const set<GraphNodePtr>& terminators_arg)
{
  terminators = terminators_arg;

  // Use the init method to initialize the starting point to make sure that the object is recorded as being initialized
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::init(start_arg);

  assert(terminators.find(start_arg) == terminators.end());
}

// Add a Part to this iterator's set of initial iteration Parts
template <class GraphEdgePtr, class GraphNodePtr>
void dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::addStart(GraphNodePtr start)
{
  this->add(this->Part2PartEdge(start));
}

// Add the given PartEdge to the iterator's list of edges to follow
template <class GraphEdgePtr, class GraphNodePtr>
void dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::add_internal(GraphEdgePtr next)
{
  // Never add a terminator state
  if(terminators.find(this->PartEdge2DirPart(next)) != terminators.end()) return;
  
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::add_internal(next);
}

template <class GraphEdgePtr, class GraphNodePtr>
void dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::operator ++ (int)
{
  if(!graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::initialized) { assert(0); }
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::advance(false);
}

template <class GraphEdgePtr, class GraphNodePtr>
dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::
  checkpoint::
    checkpoint(const typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint& iChkpt,
               const set<GraphNodePtr>& terminators):
  iChkpt(iChkpt), terminators(terminators) {}

template <class GraphEdgePtr, class GraphNodePtr>
dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint::checkpoint(const dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint &that):
  iChkpt(that.iChkpt), terminators(that.terminators) {}

template <class GraphEdgePtr, class GraphNodePtr>
string dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint::str(string indent) const
{
  ostringstream outs;
  outs << indent << "[dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint : \n";
  outs << indent << "&nbsp;&nbsp;&nbsp;&nbsp;iterator = \n"<<iChkpt.str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")<<"\n";
  outs << indent << "&nbsp;&nbsp;&nbsp;&nbsp;terminators = ";
  for(typename set<GraphNodePtr>::iterator i=terminators.begin(); i!=terminators.end(); i++) {
    if(i!=terminators.begin()) outs << ", ";
    outs << i->get()->str();
  }
  outs<<endl;
  return outs.str();
}

// Returns a checkpoint of this iterator's progress.
template <class GraphEdgePtr, class GraphNodePtr>
typename dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::getChkpt()
{
  if(!graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::initialized) { assert(0); }
  typename dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint
    chkpt(graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::getChkpt(), terminators);
  return chkpt;
}

// Loads this iterator's state from the given checkpoint.
template <class GraphEdgePtr, class GraphNodePtr>
void dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::restartFromChkpt(dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint& chkpt)
{
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::restartFromChkpt(chkpt.iChkpt);
  terminators = chkpt.terminators;
}

template <class GraphEdgePtr, class GraphNodePtr>
string dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::str(string indent) const
{
  ostringstream outs;

  if(graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::initialized) {
    outs << "[dataflowGraphEdgeIterator:\n";
    outs << "&nbsp;&nbsp;&nbsp;&nbsp;iterator = "<<graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;")<<"\n";
    outs << indent << "&nbsp;&nbsp;&nbsp;&nbsp;terminators = ";
    for(typename set<GraphNodePtr>::iterator i=terminators.begin(); i!=terminators.end(); i++) {
      if(i==terminators.begin()) outs << ", ";
      outs << i->get()->str();
    }
  } else {
    outs << "[dataflowGraphEdgeIterator: Uninitialized]";
  }

  return outs.str();
}


/*****************************
******* DATAFLOW *******
*****************************/

template <class GraphEdgePtr, class GraphNodePtr>
fw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw_dataflowGraphEdgeIterator(
                 bool incrementalGraph, graphIterOrderT iterOrder) :
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder, false),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder),
        fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, iterOrder, false) {}

template <class GraphEdgePtr, class GraphNodePtr>
fw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw_dataflowGraphEdgeIterator(
                 const GraphNodePtr& terminator_arg,
                 bool incrementalGraph,
                 graphIterOrderT iterOrder) :
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder, false),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(terminator_arg, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder),
        fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, iterOrder) {}

template <class GraphEdgePtr, class GraphNodePtr>
fw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw_dataflowGraphEdgeIterator(
                 const set<GraphNodePtr>& terminators_arg,
                 bool incrementalGraph,
                 graphIterOrderT iterOrder) :
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder, false),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(terminators_arg, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder),
        fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, iterOrder){}

template <class GraphEdgePtr, class GraphNodePtr>
fw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw_dataflowGraphEdgeIterator(
                 const GraphNodePtr end, const GraphNodePtr& terminator_arg,
                 bool incrementalGraph,
                 graphIterOrderT iterOrder):
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder, true),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(end, terminator_arg, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder),
        fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(end, incrementalGraph) {}
template <class GraphEdgePtr, class GraphNodePtr>
fw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw_dataflowGraphEdgeIterator(
                 const GraphNodePtr end, const set<GraphNodePtr>& terminators_arg,
                 bool incrementalGraph,
                 graphIterOrderT iterOrder):
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder, true),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(end, terminators_arg, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::fw, iterOrder),
        fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(end, incrementalGraph) {}

/*void dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::operator ++ (int)
{
  advance(false);
}*/

/*****************************
******* bw_DATAFLOW *******
*****************************/

template <class GraphEdgePtr, class GraphNodePtr>
bw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw_dataflowGraphEdgeIterator(
                 bool incrementalGraph, graphIterOrderT iterOrder) :
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder, false),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder),
        bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, iterOrder, false) {}
template <class GraphEdgePtr, class GraphNodePtr>
bw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw_dataflowGraphEdgeIterator(
                 const GraphNodePtr& terminator_arg,
                 bool incrementalGraph,
                 graphIterOrderT iterOrder) :
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder, false),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(terminator_arg, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder),
        bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, iterOrder) {}
template <class GraphEdgePtr, class GraphNodePtr>
bw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw_dataflowGraphEdgeIterator(
                 const set<GraphNodePtr>& terminators_arg,
                 bool incrementalGraph,
                 graphIterOrderT iterOrder) :
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder, false),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(terminators_arg, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder),
        bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, iterOrder) {}
template <class GraphEdgePtr, class GraphNodePtr>
bw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw_dataflowGraphEdgeIterator(
                 const GraphNodePtr end, const GraphNodePtr& terminator_arg,
                 bool incrementalGraph,
                 graphIterOrderT iterOrder):
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder, true),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(end, terminator_arg, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder),
        bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(end, incrementalGraph) {}
template <class GraphEdgePtr, class GraphNodePtr>
bw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw_dataflowGraphEdgeIterator(
                 const GraphNodePtr end, const set<GraphNodePtr>& terminators_arg,
                 bool incrementalGraph,
                 graphIterOrderT iterOrder):
        graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder, true),
        dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>(end, terminators_arg, incrementalGraph, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::bw, iterOrder),
        bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>(end, incrementalGraph) {}

/*void bw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::operator ++ (int)
{
  advance(false);
}*/

/*string bw_dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>::str(string indent)
{
  ostringstream outs;

  if(initialized) {
    outs << "[bw_dataflowGraphEdgeIterator:\n";
    outs << "&nbsp;&nbsp;&nbsp;&nbsp;iterator = "<<graphEdgeIterator::str(indent+"&nbsp;&nbsp;&nbsp;&nbsp;")<<"\n";
    outs << indent << "&nbsp;&nbsp;&nbsp;&nbsp;terminators = ";
    for(typename set<GraphNodePtr>::iterator i=terminators.begin(); i!=terminators.end(); i++) {
      if(i==terminators.begin()) outs << ", ";
      outs << i->get()->str();
    }
  } else {
    outs << "[bw_dataflowGraphEdgeIterator: Uninitialized]";
  }

  return outs.str();
}*/

template class graphEdgeIterator<PartEdgePtr, PartPtr>;
template class fw_graphEdgeIterator<PartEdgePtr, PartPtr>;
template class bw_graphEdgeIterator<PartEdgePtr, PartPtr>;
template class dataflowGraphEdgeIterator<PartEdgePtr, PartPtr>;
template class fw_dataflowGraphEdgeIterator<PartEdgePtr, PartPtr>;
template class bw_dataflowGraphEdgeIterator<PartEdgePtr, PartPtr>;

/***************************************
 ***** GEFrontBackIteratorWorklist *****
 ***************************************/

// Returns a freshly-allocated copy of the worklist
template <class GraphEdgePtr, class GraphNodePtr>
boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> >
    GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::copy() const
{ return boost::make_shared<GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr> >(*this); }

// Returns the number of edges currently on the worklist
template <class GraphEdgePtr, class GraphNodePtr>
int GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::size() const
{ return worklist.size(); }

// Returns true if the given edge  is in the worklist and false otherwise
template <class GraphEdgePtr, class GraphNodePtr>
bool GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::isInWorklist(GraphEdgePtr edge)
{
  for(typename list<GraphEdgePtr>::const_iterator it=worklist.begin(); it!=worklist.end(); it++)
  {
    // if edge is currently in the worklist
    if(*it == edge) return true;
  }
  // edge is not in the worklist
  return false;
}


// Add the given edge to the iterator's list of edges to follow
template <class GraphEdgePtr, class GraphNodePtr>
void GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::add(GraphEdgePtr edge) {
//  cout << "             GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::add() edge="<<edge->str()<<", isInWorklist(edge)="<<isInWorklist(edge)<<endl;
  // Add this edge if it is not already in the worklist
  if(!isInWorklist(edge)) {
    if(placementLoc==front)
      worklist.push_front(edge);
    else
      worklist.push_back(edge);
  }
}

// Returns the next edge on the worklist but does not remove it from the worklist
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::getNext() const {
  ROSE_ASSERT(worklist.size()>0);
  //dbg << "getNext: "<<(worklist.front()? worklist.front()->str(): "NULL")<<endl;
  return worklist.front();
}

// Grabs the next edge form the worklist and returns it
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::grabNext() {
  ROSE_ASSERT(worklist.size()>0);
  GraphEdgePtr edge = worklist.front();
  worklist.pop_front();
  return edge;
}

// Returns whether this and that worklists are equal. If the other worklist is not
// the same type as this one, this method must return false.
template <class GraphEdgePtr, class GraphNodePtr>
bool GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::eq(const GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that_arg) const {
  try {
    const GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that = dynamic_cast<const GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>&>(that_arg);
/*    cout << "this("<<this<<")="; print(cout); cout <<endl;
    cout << "that("<<&that<<")="<<endl;*/
    //that.print(cout); cout <<endl;
    // If these worklists operate in opposite directions, they're not equal
    if(placementLoc!=that.placementLoc) return false;

    // Two worklists are the same if they contain the same edges, regardless of their order
    // As such, transfer the values in both worklists into sets and compare the sets
    set<GraphEdgePtr> thisEdges;
    for(typename list<GraphEdgePtr>::const_iterator e=worklist.begin(); e!=worklist.end(); e++)
      thisEdges.insert(*e);

    set<GraphEdgePtr> thatEdges;
    for(typename list<GraphEdgePtr>::const_iterator e=that.worklist.begin(); e!=that.worklist.end(); e++)
      thatEdges.insert(*e);

    return thisEdges == thatEdges;
  } catch(bad_cast bc) {
    // If that does not have the same type as this, they're not equal
    return false;
  }
}

// Emit the string representation of this worklist to the given stream
template <class GraphEdgePtr, class GraphNodePtr>
void GEFrontBackIteratorWorklist<GraphEdgePtr, GraphNodePtr>::print(ostream& out) const {
  out << "GEFrontBackIteratorWorklist (#"<<size()<<"), placementLoc="<<(placementLoc==front?"front":"back")<<endl;
  for(typename list<GraphEdgePtr>::const_iterator e=worklist.begin(); e!=worklist.end(); e++) {
    out << "    "<<(*e)->str()<<endl;
  }
}

/***************************************
 ***** GETopoOrderIteratorWorklist *****
 ***************************************/

// Returns a freshly-allocated copy of the worklist
template <class GraphEdgePtr, class GraphNodePtr>
boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> >
    GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::copy() const
{ return boost::make_shared<GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr> >(*this); }

/*set<GraphNodePtr> getNodesWithNoIncoming(const map<GraphNodePtr, int>& inDegree) {
  set<GraphNodePtr> ret;
  for(map<GraphNodePtr, int>::const_iterator i=inDegree.begin(); i!=inDegree.end; ++i)
    if(i->second==0)
      ret.insert(i->first);
  return ret;
}*/

// Initializes this worklist, assuming that it already contains all the edges from
// which iteration starts.
template <class GraphEdgePtr, class GraphNodePtr>
void GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::initGivenStart(const set<GraphEdgePtr>& startEdges) {
//  cout << "##########################################"<<endl;
//  cout << "initGivenStart()"<<endl;
  // The set of edges that are on the current frontier of edges that have not yet
  // been mapped to some index
/*  set<GraphEdgePtr> frontier;

  // Initialize the frontier with the starting edges
//  for(typename map<int, GraphEdgePtr>::iterator i=worklist.begin(); i!=worklist.end(); i++)
//    frontier.insert(i->second);
  frontier = startEdges;

  while(frontier.size() != 0) {
    // Grab a node from the frontier
    GraphEdgePtr cur = *frontier.begin();
    frontier.erase(frontier.begin());

    // We must not yet have assigned an index to it so assign one now
    ROSE_ASSERT(edge2Idx.find(cur) == edge2Idx.end());
//    cout << "assigning "<<maxEdgeTopoIdx<<" to "<<cur->str()<<endl;
    edge2Idx[cur] = ++maxEdgeTopoIdx;
    if(cur->source()) edge2Idx[cur->source()->outEdgeToAny()]  = ++maxEdgeTopoIdx;
    if(cur->target()) edge2Idx[cur->target()->inEdgeFromAny()] = ++maxEdgeTopoIdx;

    if(cur->source() && node2Idx.find(cur->source())==node2Idx.end())
      node2Idx[cur->source()] = ++maxNodeTopoIdx;
    if(cur->target() && node2Idx.find(cur->target())==node2Idx.end())
      node2Idx[cur->target()] = ++maxNodeTopoIdx;

    // Push onto the frontier all outgoing edges of cur that have not yet been assigned
    // an index
    if(GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->PartEdge2DirPart(cur)) {
      list<GraphEdgePtr> succE = GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->Part2DirEdges(GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->PartEdge2DirPart(cur));
      for(typename list<GraphEdgePtr>::iterator s=succE.begin(); s!=succE.end(); s++) {
      //  cout << "    "<<s->str()<<endl;
        if(edge2Idx.find(*s) == edge2Idx.end())
          frontier.insert(*s);
      }
    }
  }*/

  set<GraphEdgePtr> initialFrontier;
  // If the graph is non-incremental initialize the frontiner to be the start edges
  if(!GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->incrementalGraph)
    initialFrontier = startEdges;
  // If it is incremental, initialize the frontier to be the supersets of the start edges,
  // which belong to the superset graph, which has already been created
  else {
    for(typename set<GraphEdgePtr>::const_iterator e=startEdges.begin(); e!=startEdges.end(); ++e)
      initialFrontier.insert((*e)->getInputPartEdge());
  }

  // Traverse the graph to calculate the in-degree of all nodes
  map<GraphNodePtr, int> inDegree;
  {

    set<GraphEdgePtr> frontier = initialFrontier;
    set<GraphEdgePtr> visited;
    while(frontier.size() != 0) {
      // Grab a node from the frontier
      GraphEdgePtr cur = *frontier.begin();
      frontier.erase(frontier.begin());
      visited.insert(cur);

      if(inDegree.find(cur->target())==inDegree.end()) inDegree[cur->target()]=1;
      else ++inDegree[cur->target()];

      // Push onto the frontier all outgoing edges of cur that have not yet been assigned
      // an index
      if(GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->PartEdge2DirPart(cur)) {
        list<GraphEdgePtr> succE = GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->Part2DirEdges(GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->PartEdge2DirPart(cur));
        for(typename list<GraphEdgePtr>::iterator s=succE.begin(); s!=succE.end(); s++) {
          if(visited.find(*s) == visited.end())
            frontier.insert(*s);
        }
      }
    }
  }

  {
    set<GraphEdgePtr> frontier = initialFrontier;
    set<GraphEdgePtr> visited;
    set<GraphEdgePtr> history;

    while(frontier.size() != 0) {
      // Grab a node from the frontier
      GraphEdgePtr cur = *frontier.begin();
      //cout << "cur="<<cur->str()<<endl;
      frontier.erase(frontier.begin());
      visited.insert(cur);

      // Decrement the in-degree of the edge's target node
      --inDegree[cur->target()];
      //cout << "    inDegree[cur->target()]="<<inDegree[cur->target()]<<endl;
      if(inDegree[cur->target()]<=0) {
        history.erase(cur);
        inDegree.erase(cur->target());
        if(cur->source()) edge2Idx[cur->source()->outEdgeToAny()]  = ++maxEdgeTopoIdx;
        edge2Idx[cur] = ++maxEdgeTopoIdx;
        if(cur->target()) edge2Idx[cur->target()->inEdgeFromAny()] = ++maxEdgeTopoIdx;

        list<GraphEdgePtr> outE = cur->target()->outEdges();
        for(typename list<GraphEdgePtr>::iterator e=outE.begin(); e!=outE.end(); ++e) {
//          cout << "    successor (visited="<<(visited.find(*e) != visited.end())<<") = "<<(*e)->str()<<endl;
          if(visited.find(*e) == visited.end())
            frontier.insert(*e);
        }
      } else
        history.insert(cur);

      if(frontier.size()==0) {
        if(history.size()>0) {
          GraphEdgePtr h = *history.begin();
//          cout << "    picking from history "<<h->str()<<endl;
          /*list<GraphEdgePtr> outE = h->outEdges();
          for(typename list<GraphEdgePtr>::iterator e=outE.begin(); e!=outE.end(); ++e) {
            if(visited.find(*e) == visited.end())
              frontier.insert(*e);
          }*/
          frontier.insert(h);
          inDegree[h->target()]=1;

          history.erase(history.begin());
        }/* else if(inDegree.size()>0){
          list<GraphEdgePtr> outE = cur->target()->outEdges();
        }
          for(typename list<GraphEdgePtr>::iterator e=outE.begin(); e!=outE.end(); ++e) {
                    history.pop_front();
        }*/
      }
    }
  }
  /*cout << "==================================================="<<endl;
  cout << "==================================================="<<endl;
  cout << "==================================================="<<endl;*/
}

// Returns the number of edges currently on the worklist
template <class GraphEdgePtr, class GraphNodePtr>
int GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::size() const
{ return worklist.size(); }

/* // Returns a topological index for the given edge, which may be freshly generated
// or fetched from edge2Idx
template <class GraphEdgePtr, class GraphNodePtr>
int GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::getEdgeTopoIdx(GraphEdgePtr edge) {
  // Set of all the edges visited during the search for an edge's index. Critical for ensuring
  // that recursive calls to getEdgeTopoIdx don't get into infinite loops around cycles in the graph.
//  static set<GraphEdgePtr> visited;

  cout<<"###getEdgeTopoIdx("<<edge->str()<<")"<<endl;
  { cout << "---edge2Idx"<<endl;
    for(typename map<GraphEdgePtr, int>::iterator i=edge2Idx.begin(); i!=edge2Idx.end(); i++)
      cout << "    "<<i->first->str()<<": "<<i->second<<endl;
  }
  typename map<GraphEdgePtr, int>::iterator it=edge2Idx.find(edge);
  // If we have not yet assigned an index to this edge, give it a new one
  if(it==edge2Idx.end()) {
    // The index of this edge must be larger than the indexes of any edges that
    // precede it in the iteration order.
    int idx;
    list<GraphEdgePtr> predE;

    if(GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->PartEdge2AntiDirPart(edge)) {
      predE = GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->Part2AntiDirEdges(GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->PartEdge2AntiDirPart(edge));
      { cout << "~~~predE"<<endl;
        for(typename list<GraphEdgePtr>::iterator p=predE.begin(); p!=predE.end(); p++)
          cout << "    "<<p->str()<<endl;
      }
    }

    // If this edge has no predecessors
    if(predE.size() == 0) {
      // Assign to it the maximum index ever observed
      idx = ++maxEdgeTopoIdx;
      cout << "No predecessors idx="<<idx<<endl;
    } else {
      // Compute the max of the predecessors' indexes.
      idx=-1;

      // Register that we've visited this edge before calling getEdgeTopoIdx() recursively
      //visited.insert(edge);

      for(typename list<GraphEdgePtr>::iterator e=predE.begin(); e!=predE.end(); e++) {
        cout <<"____Predecessor "<<(*e)->str()<<endl;
        int predIdx = getEdgeTopoIdx(*e);
        idx = (predIdx > idx? predIdx: idx);
        cout << "computed idx="<<idx<<", predIdx="<<predIdx<<endl;
      }
      idx++;

      // Update maxEdgeTopoIdx to be the largest index ever assigned to an edge
      maxEdgeTopoIdx = (idx > maxEdgeTopoIdx? idx: maxEdgeTopoIdx);

//      // Unregister this edge from visited
//      visited.erase(edge);
    }
    edge2Idx[edge] = idx;
    return idx;
  // Otherwise, grab the index from edge2Idx
  } else {
    cout << "Found idx="<<it->second<<endl;
    return it->second;
  }
}
*/

// Remove the given edge from edge2Idx and worklist under its current index
// and place it back using an index that's larger than all the indexes generated
// thus far
template <class GraphEdgePtr, class GraphNodePtr>
void GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::refreshEdgeIndex(GraphEdgePtr edge) {
   // If the edge already has an index
   typename map<GraphEdgePtr, int>::iterator edgeIdx = edge2Idx.find(edge);
   if(edgeIdx != edge2Idx.end()) {
     // Update maxEdgeTopoIdx to be the edge's new index
     ++maxEdgeTopoIdx;
     // If the edge is currently on the worklist
     typename map<int, set<GraphEdgePtr> >::iterator workLoc = worklist.find(edgeIdx->second);
     if(workLoc != worklist.end()) {
       // Try erasing it from its current location at its current index and if we succeed
       // (it was indeed on the worklist), add it to the worklist under its new index
       if(workLoc->second.erase(edge)>0)
         worklist[maxEdgeTopoIdx].insert(edge);
     }

#ifndef DISABLE_SIGHT
     dbg << "Updating edge index from "<<edgeIdx->second<<" to "<<maxEdgeTopoIdx<<", edge="<<edge->str()<<endl;
#endif

     // Record the edge's new index in edge2Idx
     edgeIdx->second = maxEdgeTopoIdx;

   }
}

// Add the given edge to the iterator's list of edges to follow
template <class GraphEdgePtr, class GraphNodePtr>
void GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::add(GraphEdgePtr edge) {
/*	cout << "###################################################"<<endl;
	cout << "###################################################"<<endl;
	cout << "###################################################"<<endl;*/

  // Pre-computed edge selection algorithm
  // -------------------------------------

  // Find the index of edge.
  typename map<GraphEdgePtr, int>::iterator i;
  // If the graph is not incremental, use the edge itself for the lookup
  if(!GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>::parent->incrementalGraph)
    i=edge2Idx.find(edge);
  // Otherwise, use the edge's superset edge from the fully created superset graph
  else
    i=edge2Idx.find(edge->getInputPartEdge());

//dbg << "(i!=edge2Idx.end())="<<(i!=edge2Idx.end())<<endl;
  if(i!=edge2Idx.end()) worklist[i->second].insert(edge);
  else {
    // cout << "WARNING: edge not found: "<<edge->str()<<endl;
    /*cout << "edge2Idx:"<<endl;
    for(typename map<GraphEdgePtr, int>::iterator e=edge2Idx.begin(); e!=edge2Idx.end(); ++e)
      cout << "    "<<e->second<<": "<<e->first->str()<<endl;
    ROSE_ASSERT(0);*/
    worklist[0].insert(edge);
  }

  // Online edge selection algorithm
  // -------------------------------
/*
  // Place the new edge into the worklist. Note that this ensures that no
  // edge can appear in the worklist more than once.
  typename map<GraphEdgePtr, int>::iterator i=edge2Idx.find(edge);
//dbg << "(i!=edge2Idx.end())="<<(i!=edge2Idx.end())<<endl;
  if(i!=edge2Idx.end()) {
    worklist[i->second].insert(edge);
  } else {
    // Add the edge and the wildcard edges coming into its target and out of its source to edge2Idx
    int edgeIdx;
    if(edge->source() && edge->target()) {
      if(edge2Idx.find(edge->source()->outEdgeToAny()) == edge2Idx.end())
        edge2Idx[edge->source()->outEdgeToAny()] = ++maxEdgeTopoIdx;

      edge2Idx[edge] = ++maxEdgeTopoIdx;
      edgeIdx = maxEdgeTopoIdx;

      if(edge2Idx.find(edge->target()->inEdgeFromAny()) == edge2Idx.end())
        edge2Idx[edge->target()->inEdgeFromAny()] = ++maxEdgeTopoIdx;
    } else {
      edge2Idx[edge] = ++maxEdgeTopoIdx;
      edgeIdx = maxEdgeTopoIdx;
    }

    // Add the edge to the worklist, under its index
    worklist[edgeIdx].insert(edge);

    // If the edge's target is known
    GraphNodePtr tgt = edge->target();
    if(tgt) {
      set<GraphEdgePtr>& tgtIn = incomingEdges[tgt];
      typename set<GraphEdgePtr>::iterator in = tgtIn.find(edge);
      // If we've discovered a new edge arriving into the target
      if(in == tgtIn.end()) {
        // If this is not the first edge to come into this target, refresh the indexes of all of the target's
        // outgoing edges (which should all exist now) to have a larger index than the index of this edge.
        if(tgtIn.size()>0) {
          // First, refresh its outgoing wildcard edge
          GraphEdgePtr wildcard = edge->target()->outEdgeToAny();
          refreshEdgeIndex(wildcard);

          // If the target of this edge has already visited, its outgoing edges must already be
          // generated (iteration over the ATS with loose and tight composition of analyses)
          if(edge->target() && visitedTargets.find(edge->target()) != visitedTargets.end()) {
            // Refresh the indexes of all the specific edges going out of target
            list<GraphEdgePtr> out = edge->target()->outEdges();
            for(typename list<GraphEdgePtr>::iterator o=out.begin(); o!=out.end(); ++o)
              refreshEdgeIndex(*o);
          }
        }

        tgtIn.insert(edge);
      }
    }
  }*/
/*
  typename map<GraphNodePtr, int>::iterator iSrc=node2Idx.find(edge->source());
  if(iSrc!=node2Idx.end()) worklist[iSrc->second].insert(edge);
  else {
    typename map<GraphNodePtr, int>::iterator iTgt=node2Idx.find(edge->target());
    if(iTgt!=node2Idx.end()) worklist[iTgt->second].insert(edge);
    else
      ROSE_ASSERT(0);
  }*/
/*    else if(worklist.size()==0) worklist[-1].insert(edge);
  else {
    // Pick an index for this edge that is smaller than any assigned index and smaller
    // than any index currently on the worklist (which may currently have some edges
    // mapped to negative indexes).
    int minIdx = worklist.begin()->first;
    minIdx = (minIdx<0?minIdx-1: -1);
    worklist[minIdx].insert(edge);
  }*/
}

// Returns the next edge on the worklist but does not remove it from the worklist
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::getNext() const {
  ROSE_ASSERT(worklist.size()>0);
  ROSE_ASSERT(worklist.begin()->second.size()>0);
  return *worklist.begin()->second.begin();
}

// Grabs the next edge form the worklist and returns it
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::grabNext() {
  ROSE_ASSERT(worklist.size()>0);
  ROSE_ASSERT(worklist.begin()->second.size()>0);
  GraphEdgePtr edge = *worklist.begin()->second.begin();
  worklist.begin()->second.erase(worklist.begin()->second.begin());
  if(worklist.begin()->second.size()==0)
    worklist.erase(worklist.begin());
  return edge;
}

// Returns whether this and that worklists are equal. If the other worklist is not
// the same type as this one, this method must return false.
template <class GraphEdgePtr, class GraphNodePtr>
bool GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::eq(const GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that_arg) const {
  try {
    const GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that = dynamic_cast<const GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>&>(that_arg);

    // Two worklists are the same if they contain the same edges, regardless of their order
    // As such, transfer the values in both worklists into sets and compare the sets
    set<GraphEdgePtr> thisEdges;
    for(typename map<int, set<GraphEdgePtr> >::const_iterator e=worklist.begin(); e!=worklist.end(); ++e) {
      for(typename set<GraphEdgePtr>::const_iterator i=e->second.begin(); i!=e->second.end(); ++i)
        thisEdges.insert(*i);
    }

    set<GraphEdgePtr> thatEdges;
    for(typename map<int, set<GraphEdgePtr> >::const_iterator e=that.worklist.begin(); e!=that.worklist.end(); ++e) {
      for(typename set<GraphEdgePtr>::const_iterator i=e->second.begin(); i!=e->second.end(); ++i)
        thatEdges.insert(*i);
    }

    return thisEdges == thatEdges;
  } catch(bad_cast bc) {
    // If that does not have the same type as this, they're not equal
    return false;
  }
}

// Emit the string representation of this worklist to the given stream
template <class GraphEdgePtr, class GraphNodePtr>
void GETopoOrderIteratorWorklist<GraphEdgePtr, GraphNodePtr>::print(ostream& out) const {
  out << "GETopoOrderIteratorWorklist (#"<<size()<<")"<<endl;
  for(typename map<int, set<GraphEdgePtr> >::const_iterator e=worklist.begin(); e!=worklist.end(); ++e) {
    for(typename set<GraphEdgePtr>::const_iterator i=e->second.begin(); i!=e->second.end(); ++i)
      out << "    idx="<<e->first<<", "<<(*i)->str()<<endl;
  }
}

/************************************
 ***** GERandomIteratorWorklist *****
 ************************************/

// Returns a freshly-allocated copy of the worklist
template <class GraphEdgePtr, class GraphNodePtr>
boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> >
    GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>::copy() const
{ return boost::make_shared<GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr> >(*this); }

// Returns the number of edges currently on the worklist
template <class GraphEdgePtr, class GraphNodePtr>
int GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>::size() const
{ return worklist.size(); }

// Add the given edge to the iterator's list of edges to follow
template <class GraphEdgePtr, class GraphNodePtr>
void GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>::add(GraphEdgePtr edge) {
  // Place the new edge into the worklist at a random index.
  worklist[rand()] = edge;
}

// Returns the next edge on the worklist but does not remove it from the worklist
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>::getNext() const {
  ROSE_ASSERT(worklist.size()>0);
  return worklist.begin()->second;
}

// Grabs the next edge form the worklist and returns it
template <class GraphEdgePtr, class GraphNodePtr>
GraphEdgePtr GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>::grabNext() {
  GraphEdgePtr edge = worklist.begin()->second;
  worklist.erase(worklist.begin());
  return edge;
}

// Returns whether this and that worklists are equal. If the other worklist is not
// the same type as this one, this method must return false.
template <class GraphEdgePtr, class GraphNodePtr>
bool GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>::eq(const GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that_arg) const {
  try {
    const GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that = dynamic_cast<const GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>&>(that_arg);
    // Two random worklists are the same if they contain the same edges, regardless of their order
    // As such, transfer the values in both worklists into sets and compare the sets
    set<GraphEdgePtr> thisEdges;
    for(typename map<int, GraphEdgePtr>::const_iterator e=worklist.begin(); e!=worklist.end(); e++)
      thisEdges.insert(e->second);

    set<GraphEdgePtr> thatEdges;
    for(typename map<int, GraphEdgePtr>::const_iterator e=that.worklist.begin(); e!=that.worklist.end(); e++)
      thatEdges.insert(e->second);

    return thisEdges == thatEdges;
  } catch(bad_cast bc) {
    // If that does not have the same type as this, they're not equal
    return false;
  }
}

// Emit the string representation of this worklist to the given stream
template <class GraphEdgePtr, class GraphNodePtr>
void GERandomIteratorWorklist<GraphEdgePtr, GraphNodePtr>::print(ostream& out) const {
  out << "GERandomIteratorWorklist (#"<<size()<<")"<<endl;
  for(typename map<int, GraphEdgePtr>::const_iterator e=worklist.begin(); e!=worklist.end(); e++) {
    out << "    <idx="<<e->first<<", "<<e->second->str()<<">"<<endl;
  }
}

}; // namespace fuse
