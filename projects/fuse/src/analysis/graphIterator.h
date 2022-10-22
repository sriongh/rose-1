#pragma once

#include "virtualCFG.h"
#include "DataflowCFG.h"
#include "partitions.h"

#include <list>
#include <set>
#include <string>

namespace fuse {

template <class GraphEdgePtr, class GraphNodePtr>
class GEIteratorWorklist;

// The iteration proceeds from the starting nodes along incoming or outgoing edges.
// When new nodes are added to the iterator they can be placed into the worklist
// according to several policies:
typedef enum {succ_front, // The new node is visited next.
              succ_back,  // The new node is visited after all the nodes currently on the worlist
              topo_order, // All graph nodes are ordered topologically and all nodes on the worklist
                          //    are visited in this order.
              random      // New nodes are placed in a random location in the worklist.
             }
          graphIterOrderT;

// Checks the environment for any variable that specified the best iteration order to use for dataflow analysis and returns it
graphIterOrderT selectIterOrderFromEnvironment();
          
// Iterates over PartEdges in a Partition, respecting dependences in the graph.
// Supports both forward and backward iteration.
template <class GraphEdgePtr, class GraphNodePtr>
class graphEdgeIterator
{
  public:
  // Indicates whether the graph is being created incrementally as it is being traversed (true)
  // or whether the graph has already been created (false). If the former, it is expected
  // that nodes will implement method getSupersetPart() that returns the node that they are
  // refining in a graph that is guaranteed to have already been created, and same for edges
  // (method getSupersetPartEdge()).
  bool incrementalGraph;

  typedef enum {fw, bw} iterDirection;
  iterDirection dir;
  
  graphIterOrderT iterOrder;
  
  public:

  // Records whether this iterator has been initialized with some starting node(s)
  bool initialized;
  
  //std::list<GraphEdgePtr> worklist;
  boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > worklist;
  std::set<GraphEdgePtr> visited;

  public:
  graphEdgeIterator(bool incrementalGraph, iterDirection dir, graphIterOrderT iterOrder=succ_back, bool initialized=false);
  graphEdgeIterator(const GraphNodePtr start, bool incrementalGraph, iterDirection dir, graphIterOrderT iterOrder=succ_back);
  graphEdgeIterator(const std::set<GraphNodePtr>& start, bool incrementalGraph, iterDirection dir, graphIterOrderT iterOrder=succ_back);
  graphEdgeIterator(const graphEdgeIterator& that);
  virtual ~graphEdgeIterator() { }
  
  // Creates a new worklist implementation that follows the given iteration order and returns a pointer to it.
  boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > createWorklist(graphIterOrderT iterOrder);
  
  void init(const GraphNodePtr start);
  void init(const std::set<GraphNodePtr>& start);
  void init(const std::set<GraphEdgePtr>& start);

  // Given a part returns the wildcard edge pointing into it (for fw traversal) or out of it (for backward traversals
  GraphEdgePtr Part2PartEdge(GraphNodePtr part) const;
  
  // Given a PartEdge returns the Part that points in the direction of this iterator's traversal (target for fw, 
  // source for bw)
  GraphNodePtr PartEdge2DirPart(GraphEdgePtr pedge) const;
  
  // Given a PartEdge returns the Part that points in the direction opposite to this iterator's traversal (source for fw, 
  // target for bw)
  GraphNodePtr PartEdge2AntiDirPart(GraphEdgePtr pedge) const;
  
  // Given a Part returns the list of Edges in the direction of this iterator's traversals (outEdges for fw, inEdges
  // for bw);
  std::list<GraphEdgePtr> Part2DirEdges(GraphNodePtr part) const;
  
  // Given a Part returns the list of Edges in opposite of the direction of this iterator's traversals 
  // (inEdges for fw, outEdges for bw)
  std::list<GraphEdgePtr> Part2AntiDirEdges(GraphNodePtr part) const;
 
  protected:
 
  // Returns true if the given GraphNodePtr is in the worklist and false otherwise
  //bool isRemaining(const GraphEdgePtr pedge);
  
  // Find the followers of curPart (either successors or predecessors, depending on value of fwDir), push back 
  // those that have not yet been visited
  void pushAllDescendants(GraphEdgePtr pedge);
  
  public:
  void pushAllDescendants();
  
  protected:
  // Add the given PartEdge to the iterator's list of edges to follow
  virtual void add_internal(GraphEdgePtr next);
  
  public:
  // Add the given PartEdge to the iterator's list of edges to follow
  void add(GraphEdgePtr next);
  
  //protected:
  // Advances this iterator in the direction of motion.
  // If pushAllChildren=true, all of the current node's unvisited children (predecessors or successors, 
  //    depending on fwDir) are pushed onto worklist
  void advance(bool pushAllChildren);
  
  public:
  virtual void operator ++ (int);
  
  bool eq(const graphEdgeIterator& other_it) const;
  
  bool operator==(const graphEdgeIterator& other_it) const;
  
  bool operator!=(const graphEdgeIterator& it) const;
          
  virtual GraphEdgePtr operator * ();
  // virtual PartEdge* operator -> ();
  
  // Get the PartEdge that the iterator is currently is referring to
  GraphEdgePtr getPartEdge() const;
  
  // Grab the PartEdge that the iterator is currently is referring to and advance it to the following PartEdge
  GraphEdgePtr grabPartEdge();

  // Get the Part within the current PartEdge in the iterator's direction of motion (target for fw, source for bw)
  GraphNodePtr getPart() const;

  // Grab the Part within the current PartEdge in the iterator's direction of motion (target for fw, source for bw)
  // to and advance the iterator to the following PartEdge
  GraphNodePtr grabPart();
  
  // Returns an empty iterator that can be compared to any other iterator to 
  // check if it has completed passing over its iteration space
  /*static graphEdgeIterator endIter;
  static const graphEdgeIterator& end();*/
  bool isEnd() { return worklist->size()==0; }

  // Returns the number of edges currently on the worklist
  int size() { return worklist->size(); }
  
  // Contains the state of an iterator, allowing iterators to be 
  // checkpointed and restarted.
  class checkpoint
  {
    //std::list<GraphEdgePtr> remainingNodes;
    boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > worklist;
    std::set<GraphEdgePtr>  visited;
  
    public:
    checkpoint(boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > worklist, const std::set<GraphEdgePtr>& visited);
          
    checkpoint(const checkpoint& that);
    
    std::string str(std::string indent="") const;
    
    friend class graphEdgeIterator;
  };
  
  // Returns a checkpoint of this iterator's progress.
  checkpoint getChkpt();
  
  // Loads this iterator's state from the given checkpoint.
  void restartFromChkpt(checkpoint& chkpt);
  
  std::string str(std::string indent="") const;
};

// Iterates over PartEdges in a Partition, respecting dependences in the graph.
// Supports both forward and backward iteration.
template <class GraphEdgePtr, class GraphNodePtr>
class fw_graphEdgeIterator : public virtual graphEdgeIterator<GraphEdgePtr, GraphNodePtr>
{
  public:
  fw_graphEdgeIterator(bool incrementalGraph, graphIterOrderT iterOrder=succ_back, bool initialized=false);
  fw_graphEdgeIterator(const GraphNodePtr start, bool incrementalGraph, graphIterOrderT iterOrder=succ_back);
  fw_graphEdgeIterator(const std::set<GraphNodePtr>& start, bool incrementalGraph, graphIterOrderT iterOrder=succ_back);
  
  public:
  // Returns a fresh iterator that starts at node n
  static fw_graphEdgeIterator begin(const GraphNodePtr n);
};

typedef fw_graphEdgeIterator<PartEdgePtr, PartPtr> fw_partEdgeIterator;

template <class GraphEdgePtr, class GraphNodePtr>
class bw_graphEdgeIterator : public virtual graphEdgeIterator<GraphEdgePtr, GraphNodePtr>
{
  public:
  bw_graphEdgeIterator(bool incrementalGraph, graphIterOrderT iterOrder=succ_back, bool initialized=false);
  bw_graphEdgeIterator(const GraphNodePtr start, bool incrementalGraph, graphIterOrderT iterOrder=succ_back);
  bw_graphEdgeIterator(const std::set<GraphNodePtr>& start, bool incrementalGraph, graphIterOrderT iterOrder=succ_back);
    
  public:
  // Returns a fresh iterator that starts at node n
  static bw_graphEdgeIterator begin(const GraphNodePtr n);
};
//class bw_graphEdgeIterator<PartEdgePtr, PartPtr>;
typedef bw_graphEdgeIterator<PartEdgePtr, PartPtr> bw_partEdgeIterator;

template <class GraphEdgePtr, class GraphNodePtr>
class dataflowGraphEdgeIterator : public virtual graphEdgeIterator<GraphEdgePtr, GraphNodePtr>
{
  protected:
  std::set<GraphNodePtr> terminators;
  public:
  //dataflow(): iterator() {}
  
  dataflowGraphEdgeIterator(bool incrementalGraph, typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder);
  dataflowGraphEdgeIterator(const GraphNodePtr& terminator_arg,
                            bool incrementalGraph,
                            typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder);
  dataflowGraphEdgeIterator(const GraphNodePtr& start, const GraphNodePtr& terminator_arg,
                            bool incrementalGraph,
                            typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder);
  dataflowGraphEdgeIterator(const std::set<GraphNodePtr>& terminators_arg,
                            bool incrementalGraph,
                            typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder);
  dataflowGraphEdgeIterator(const GraphNodePtr& start, const std::set<GraphNodePtr>& terminators_arg,
                            bool incrementalGraph,
                            typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::iterDirection dir, graphIterOrderT iterOrder);
  
  void init(const GraphNodePtr& start_arg, const GraphNodePtr& terminator_arg);
  void init(const GraphNodePtr& start_arg, const std::set<GraphNodePtr>& terminators_arg);
  void init(const std::set<GraphNodePtr>& start_arg, const std::set<GraphNodePtr>& terminators_arg);

  // Add a Part to this iterator's set of initial iteration Parts
  void addStart(GraphNodePtr start);

  protected:
  // Add the given PartEdge to the iterator's list of edges to follow
  void add_internal(GraphEdgePtr next);
  
  public:
  void operator ++ (int);
  
  // Contains the state of an dataflow iterator, allowing dataflow 
  // iterators to be checkpointed and restarted.
  class checkpoint/* : public virtual BaseCFG::checkpoint*/
  {
    protected:
    typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint iChkpt;
    std::set<GraphNodePtr> terminators;
    
    public:
    checkpoint(const typename graphEdgeIterator<GraphEdgePtr, GraphNodePtr>::checkpoint& iChkpt, const std::set<GraphNodePtr>& terminators);
    
    checkpoint(const checkpoint &that);
            
    std::string str(std::string indent="") const;
    
    friend class dataflowGraphEdgeIterator;
  };
  
  // Returns a checkpoint of this dataflow iterator's progress.
  checkpoint getChkpt();
  
  // Loads this dataflow iterator's state from the given checkpoint.
  void restartFromChkpt(checkpoint& chkpt);
  
  std::string str(std::string indent="") const;
};

typedef dataflowGraphEdgeIterator<PartEdgePtr, PartPtr> dataflowPartEdgeIterator;

template <class GraphEdgePtr, class GraphNodePtr>
class fw_dataflowGraphEdgeIterator: public virtual dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>, 
                                    public virtual fw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>
{
  public: 
  fw_dataflowGraphEdgeIterator(bool incrementalGraph, graphIterOrderT iterOrder);
  fw_dataflowGraphEdgeIterator(const GraphNodePtr& terminator_arg, bool incrementalGraph, graphIterOrderT iterOrder);
  fw_dataflowGraphEdgeIterator(const std::set<GraphNodePtr>& terminators_arg, bool incrementalGraph, graphIterOrderT iterOrder);
  fw_dataflowGraphEdgeIterator(const GraphNodePtr end, const GraphNodePtr& terminator_arg, bool incrementalGraph, graphIterOrderT iterOrder);
  fw_dataflowGraphEdgeIterator(const GraphNodePtr end, const std::set<GraphNodePtr>& terminators_arg, bool incrementalGraph, graphIterOrderT iterOrder);
          
  //void operator ++ (int);
  
  //std::string str(std::string indent="");
};

typedef fw_dataflowGraphEdgeIterator<PartEdgePtr, PartPtr> fw_dataflowPartEdgeIterator;

template <class GraphEdgePtr, class GraphNodePtr>
class bw_dataflowGraphEdgeIterator: public virtual dataflowGraphEdgeIterator<GraphEdgePtr, GraphNodePtr>, 
                                    public virtual bw_graphEdgeIterator<GraphEdgePtr, GraphNodePtr>
{
  public: 
  bw_dataflowGraphEdgeIterator(bool incrementalGraph, graphIterOrderT iterOrder);
  bw_dataflowGraphEdgeIterator(const GraphNodePtr& terminator_arg, bool incrementalGraph, graphIterOrderT iterOrder);
  bw_dataflowGraphEdgeIterator(const std::set<GraphNodePtr>& terminators_arg, bool incrementalGraph, graphIterOrderT iterOrder);
  bw_dataflowGraphEdgeIterator(const GraphNodePtr end, const GraphNodePtr& terminator_arg, bool incrementalGraph, graphIterOrderT iterOrder);
  bw_dataflowGraphEdgeIterator(const GraphNodePtr end, const std::set<GraphNodePtr>& terminators_arg, bool incrementalGraph, graphIterOrderT iterOrder);
          
  //void operator ++ (int);
  
  //std::string str(std::string indent="");
};

typedef bw_dataflowGraphEdgeIterator<PartEdgePtr, PartPtr> bw_dataflowPartEdgeIterator;


/*template <class GraphEdgePtr, class GraphNodePtr>
std::map<GraphNodePtr, std::set<GraphNodePtr> > computeDominators() {
  set<PartPtr> startingStates = GetStartAStates(client);
  fw_dataflowPartEdgeIterator worklist;
  for(set<PartPtr>::iterator s=startingStates.begin(); s!=startingStates.end(); s++) {
    worklist->addStart(*s);
    
    set<PartPtr> onlyS; onlyS.insert(*s);
    dominators[s] = onlyS;
  }
  
  while(*worklist!=dataflowPartEdgeIterator::end()) {
    PartPtr part = worklist->getPart();
    allParts.insert(part);
    
    // There must be a dominator record for each part before we visit it
    assert(dominators.find(part) != dominators.end());
    
    // Intersect this part's dominator set with the dominator sets of all of its successors
    list<PartEdgePtr> descendants = part->outEdges();
    for(list<PartEdgePtr>::iterator de=descendants.begin(); de!=descendants.end(); de++) {
      // If the current descendant does not have a mapping in the graph, its current dominator set must be 
      // all the parts
      PartPtr target = (*de)->target();
      if(dominators.find(target) == dominators.end()) {
        // The intersection of all parts with dominators[part] is dominators[part]
        dominators[target] = dominators[part];
      } else {
        //set_intersection(dominators[target].begin(), dominators[target].end(), dominators[part].begin(), dominators[part].end(), .begin());
        set<PartPtr>::iterator partIt = dominators[part].begin(),
                               descIt = dominators[target].begin();
        while(partIt!=dominators[part].end() && descIt!=dominators[target].end()) {
          if(*partIt < *descIt) partIt++;
          else if(*partId > *descIt) descIt = dominators[target].erase(descIt);
          else if(*partId == *descIt) {
            partIt++;
            descIt++;
          }
        }
        while(descIt!=dominators[target].end()))
          descIt = dominators[target].erase(descIt);
      }
    } 
  }*/

extern template class graphEdgeIterator<PartEdgePtr, PartPtr>;
extern template class fw_graphEdgeIterator<PartEdgePtr, PartPtr>;
extern template class bw_graphEdgeIterator<PartEdgePtr, PartPtr>;
extern template class dataflowGraphEdgeIterator<PartEdgePtr, PartPtr>;
extern template class fw_dataflowGraphEdgeIterator<PartEdgePtr, PartPtr>;
extern template class bw_dataflowGraphEdgeIterator<PartEdgePtr, PartPtr>;


// Base class for various implementations of worklist data structures and iteration order.
// There is class that derives from GEIteratorWorklist for each value of enum graphIterOrderT.
// Implementations of iteration worklists are allowed to keep multiple copies of an edge
// in the worklist at the same time or may optionally eliminate duplicates.
template <class GraphEdgePtr, class GraphNodePtr>
class GEIteratorWorklist {
  protected:
  //public:
  // The iterator that this worklist is being used from. The worklist may use the
  // iterator for things like accessing the successors of a given graph node
  graphEdgeIterator<GraphEdgePtr, GraphNodePtr>* parent;
  
  public:
  GEIteratorWorklist(graphEdgeIterator<GraphEdgePtr, GraphNodePtr>* parent): parent(parent) {}
  
  GEIteratorWorklist(const GEIteratorWorklist& that): parent(that.parent) {}
  
  // Returns a freshly-allocated copy of the worklist
  virtual boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > copy() const=0;

  // Initializes this worklist, assuming that it already contains all the edges from
  // which iteration starts.
  virtual void initGivenStart(const std::set<GraphEdgePtr>& startEdges)=0;
  
  // Returns the number of edges currently on the worklist
  virtual int size() const=0;
  
  // Add the given edge to the iterator's list of edges to follow
  virtual void add(GraphEdgePtr next)=0;
  
  // Returns the next edge on the worklist but does not remove it from the worklist
  virtual GraphEdgePtr getNext() const=0;
  
  // Grabs the next edge form the worklist and returns it
  virtual GraphEdgePtr grabNext()=0;
  
  // Returns whether this and that worklists are equal. If the other worklist is not
  // the same type as this one, this method must return false.
  virtual bool eq(const GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that) const=0;
  virtual bool eq(boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > that)
  { return eq(*that.get()); }
  
  // Emit the string representation of this worklist to the given stream
  virtual void print(std::ostream& out) const=0;
}; // class GEIteratorWorklist


// Implementation of the graph iterator worklist that places all new edges before or after all 
// of the edges currently in the worklist. Edges are always grabbed from the front of the list.
template <class GraphEdgePtr, class GraphNodePtr>
class GEFrontBackIteratorWorklist : public GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> {
  protected:
  std::list<GraphEdgePtr> worklist;
  public:
  
  // Records whether we're placing new edges at the worklist's front or back
  typedef enum {front, back} placementLocT;
  placementLocT placementLoc;
  
  GEFrontBackIteratorWorklist(placementLocT placementLoc, graphEdgeIterator<GraphEdgePtr, GraphNodePtr>* parent): 
        GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>(parent), placementLoc(placementLoc) {}
  
  GEFrontBackIteratorWorklist(const GEFrontBackIteratorWorklist& that): 
      GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>(that),
      worklist(that.worklist),
      placementLoc(that.placementLoc)
  {}
  
  // Returns a freshly-allocated copy of the worklist
  boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > copy() const;
 
  // Initializes this worklist, assuming that it already contains all the edges from
  // which iteration starts.
  void initGivenStart(const std::set<GraphEdgePtr>& startEdges) {} 
  
  // Returns the number of edges currently on the worklist
  int size() const;
  
  // Returns true if the given edge  is in the worklist and false otherwise
  bool isInWorklist(GraphEdgePtr edge);
  
  // Add the given edge to the iterator's list of edges to follow
  void add(GraphEdgePtr next);
  
  // Returns the next edge on the worklist but does not remove it from the worklist
  GraphEdgePtr getNext() const;
  
  // Grabs the next edge form the worklist and returns it
  GraphEdgePtr grabNext();
  
  // Returns whether this and that worklists are equal. If the other worklist is not
  // the same type as this one, this method must return false.
  bool eq(const GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that) const;
  
  // Emit the string representation of this worklist to the given stream
  void print(std::ostream& out) const;
}; // class GEFrontBackIteratorWorklist

// Implementation of the graph iterator worklist that places all edges in the 
// worklist in graph topological order. It works best when edges are actually
// visited in this order since it builds the topological order on the fly.
template <class GraphEdgePtr, class GraphNodePtr>
class GETopoOrderIteratorWorklist : public GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> {
  protected:
  // Map from each edge's unique topological index to its object. If the graph is incremental
  // these are edges in the superset graph that is already created. Otherwise, they are edges
  // in the graph being traversed.
  // This data structure makes it possible to efficiently find the edge with the smallest index.
  std::map<int, std::set<GraphEdgePtr> > worklist;
  
  // Maps each edge and node to its topological index to efficiently figure out where into the
  // worklist to place newly-observed edges.
  // If the graph is incremental these are edges in the superset graph that is already created.
  // Otherwise, they are edges in the graph being traversed.
  std::map<GraphEdgePtr, int> edge2Idx;
  std::map<GraphNodePtr, std::set<GraphEdgePtr> > incomingEdges;
  std::map<GraphNodePtr, int> node2Idx;

  // Records the targets of edges that have already been grabbed via grabNext
  std::set<GraphNodePtr> visitedTargets;
  
  // The maximum topological index ever assigned to any edge or node
  int maxEdgeTopoIdx;
  int maxNodeTopoIdx;
  public:
  
  GETopoOrderIteratorWorklist(graphEdgeIterator<GraphEdgePtr, GraphNodePtr>* parent) :
    GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>(parent)
  {
    maxEdgeTopoIdx=0;
    maxNodeTopoIdx=0;
  }
  
  GETopoOrderIteratorWorklist(const GETopoOrderIteratorWorklist& that): 
      GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>(that),
      worklist(that.worklist),
      edge2Idx(that.edge2Idx),
      node2Idx(that.node2Idx),
      maxEdgeTopoIdx(that.maxEdgeTopoIdx),
      maxNodeTopoIdx(that.maxNodeTopoIdx)
  {}
  
  // Returns a freshly-allocated copy of the worklist
  boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > copy() const;

  // Initializes this worklist, assuming that it already contains all the edges from
  // which iteration starts.
  void initGivenStart(const std::set<GraphEdgePtr>& startEdges);

  // Returns the number of edges currently on the worklist
  int size() const;
        
  // Returns a topological index for the given edge, which may be freshly generated
  // or fetched from edge2Idx
  //int getEdgeTopoIdx(GraphEdgePtr edge);
  // Remove the given edge from edge2Idx and worklist under its current index
  // and place it back using an index that's larger than all the indexes generated
  // thus far
  void refreshEdgeIndex(GraphEdgePtr edge);
  
  // Add the given edge to the iterator's list of edges to follow
  void add(GraphEdgePtr next);
  
  // Returns the next edge on the worklist but does not remove it from the worklist
  GraphEdgePtr getNext() const;
  
  // Grabs the next edge form the worklist and returns it
  GraphEdgePtr grabNext();
  
  // Returns whether this and that worklists are equal. If the other worklist is not
  // the same type as this one, this method must return false.
  bool eq(const GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that) const;
  
  // Emit the string representation of this worklist to the given stream
  void print(std::ostream& out) const;
}; // class GETopoOrderIteratorWorklist


// Implementation of the graph iterator worklist that places all edges into the
// worklist in a random order.
template <class GraphEdgePtr, class GraphNodePtr>
class GERandomIteratorWorklist : public GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> {
  protected:
  // Map from the random number assigned to each edge to its object. 
  std::map<int, GraphEdgePtr> worklist;
  public:
  
  GERandomIteratorWorklist(graphEdgeIterator<GraphEdgePtr, GraphNodePtr>* parent) :
        GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>(parent)
  { srand(time(NULL)); }
  
  GERandomIteratorWorklist(const GERandomIteratorWorklist& that): 
      GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>(that),
      worklist(that.worklist)
  {}
  
  // Returns a freshly-allocated copy of the worklist
  boost::shared_ptr<GEIteratorWorklist<GraphEdgePtr, GraphNodePtr> > copy() const;

  // Initializes this worklist, assuming that it already contains all the edges from
  // which iteration starts.
  void initGivenStart(const std::set<GraphEdgePtr>& startEdges) {}
 
  // Returns the number of edges currently on the worklist
  int size() const;
  
  // Add the given edge to the iterator's list of edges to follow
  void add(GraphEdgePtr next);
  
  // Returns the next edge on the worklist but does not remove it from the worklist
  GraphEdgePtr getNext() const;
  
  // Grabs the next edge form the worklist and returns it
  GraphEdgePtr grabNext();
  
  // Returns whether this and that worklists are equal. If the other worklist is not
  // the same type as this one, this method must return false.
  bool eq(const GEIteratorWorklist<GraphEdgePtr, GraphNodePtr>& that) const;
  
  // Emit the string representation of this worklist to the given stream
  void print(std::ostream& out) const;
}; // class GERandomIteratorWorklist

}; // namespace fuse
