#pragma once


#include <rose.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/dominator_tree.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/transpose_graph.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace hssa_private
{

//! A class holding a Control Flow Graph.
template <class PGNodeT, class PGEdgeT>
class PG : public boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, 
  PGNodeT, PGEdgeT> //xx1 boost::shared_ptr<PGNodeT>, boost::shared_ptr<PGEdgeT> >
{
public:
    typedef typename boost::graph_traits<PG<PGNodeT, PGEdgeT> > GraphTraits;


    typedef PGNodeT PGNodeType;
    typedef PGEdgeT PGEdgeType;

    typedef boost::shared_ptr<PGNodeType> PGNodePtr;
    typedef boost::shared_ptr<PGEdgeType> PGEdgePtr;

    typedef typename GraphTraits::vertex_descriptor Vertex;
    typedef typename GraphTraits::edge_descriptor Edge;

    typedef std::map<Vertex, Vertex> VertexVertexMap;

protected:

    //! The function definition of this PG.
    SgFunctionDefinition* funcDef_;

    //! The entry node.
    Vertex entry_;

    //! The exit node.
    Vertex exit_;

    //! A map from a PG node to the corresponding vertex
    std::map<PGNodeType, Vertex> nodesToVertices_;

    //! The dominator tree of this PG.
    VertexVertexMap dominatorTree_;

    //! The postdominator tree of this PG.
    VertexVertexMap postdominatorTree_;

public:

    //! The default constructor.
    PG() : funcDef_(NULL), entry_(GraphTraits::null_vertex()), 
      exit_(GraphTraits::null_vertex()) {}

    //! The constructor building the PG.
      explicit PG(// SgFunctionDefinition* funcDef, 
		  const std::set<PGNodeT>& startNs, 
		  const std::set<PGNodeT>& endNs)
	: funcDef_(NULL), // funcDef),
        entry_(GraphTraits::null_vertex()),
        exit_(GraphTraits::null_vertex()) {
	build(// funcDef, 
	      startNs, endNs);
      }
	
    //! Build the actual PG for the given function.
	void build(// SgFunctionDefinition* funcDef, 
		   const std::set<PGNodeT>& startNs, 
		   const std::set<PGNodeT>& endNs);

    //! Get the function definition of this PG.
    SgFunctionDefinition* getFunctionDefinition() const
    { return funcDef_; }

    //! Get the entry node of the PG
    const Vertex& getEntry() const
    { return entry_; }

    //! Get the exit node of the PG
    const Vertex& getExit() const
    { return exit_; }

    //! Build the dominator tree of this PG.
    //! @returns A map from each node to its immediate dominator.
    const VertexVertexMap& getDominatorTree();

    //! Build the postdominator tree of this PG.
    const VertexVertexMap& getPostdominatorTree();

    //! Build a reverse PG.
    PG<PGNodeT, PGEdgeT> makeReverseCopy() const;

    //! Get all PG nodes in this graph.
    //xx1 std::vector<PGNodePtr> getAllNodes() const;
    std::vector<PGNodeT> getAllNodes() const;

    //! Get all PG edges in this graph.
    //xx1 std::vector<PGEdgePtr> getAllEdges() const;
    std::vector<PGEdgeT> getAllEdges() const;

    //! Given a PG node, returns the corresponding vertex in the graph.
    //! Returns Vertex::null_vertex() if the given node is not in the graph
    Vertex getVertexForNode(const PGNodeType &node) const;

    //! Get all back edges in the PG. A back edge is one whose target dominates its source.
    std::vector<Edge> getAllBackEdges();

    //! Get all loop headers in this PG. A natural loop only has one header.
    std::vector<Vertex> getAllLoopHeaders();

protected:

    //! A internal funtion which builds the actual PG (boost::graph).
    void buildPG(PGNodeType node,
            std::map<PGNodeType, Vertex>& nodesAdded,
            std::set<PGNodeType>& nodesProcessed);

    //! Find the entry and exit of this PG and set the corresponding members.
    void setEntryAndExit(const std::set<PGNodeT>& startNs, const std::set<PGNodeT>& endNs);

    //! This class is used to copy vertices when calling copy_graph().
    struct VertexCopier {
        VertexCopier(const PG<PGNodeT, PGEdgeT>& g1, PG<PGNodeT, PGEdgeT>& g2)
        : cfg1(g1), cfg2(g2) {}

        void operator()(const Vertex& v1, Vertex& v2) const
        { cfg2[v2] = cfg1[v1]; }
        
        const PG<PGNodeT, PGEdgeT>& cfg1;
        PG<PGNodeT, PGEdgeT>& cfg2;
    };

    //! This class is used to copy edges when calling copy_graph().
    struct EdgeCopier {
        EdgeCopier(const PG<PGNodeT, PGEdgeT>& g1, PG<PGNodeT, PGEdgeT>& g2)
        : cfg1(g1), cfg2(g2) {}

        void operator()(const Edge& e1, Edge& e2) const
        { cfg2[e2] = cfg1[e1]; }

        const PG<PGNodeT, PGEdgeT>& cfg1;
        PG<PGNodeT, PGEdgeT>& cfg2;
    };
};

template <class PGNodeT, class PGEdgeT>
  void PG<PGNodeT, PGEdgeT>::build(// SgFunctionDefinition* funcDef, 
				   const std::set<PGNodeT>& startNs,
				   const std::set<PGNodeT>& endNs) {
  // ROSE_ASSERT(funcDef);
  // funcDef_ = funcDef;

  // The following two variables are used to record the nodes traversed.
    nodesToVertices_.clear();
    std::set<PGNodeType> nodesProcessed;

    // Remove all nodes and edges first.
    this->clear();
    entry_ = GraphTraits::null_vertex();
    exit_ = GraphTraits::null_vertex();

    //xxx buildPG(PGNodeType(funcDef->cfgForBeginning()), nodesToVertices_, nodesProcessed);
    BOOST_FOREACH(PGNodeT startN, startNs) {
      buildPG(startN, nodesToVertices_, nodesProcessed);
    }

    // Find the entry and exit of this PG.
    setEntryAndExit(startNs, endNs);

    //xxx ROSE_ASSERT(isSgFunctionDefinition((*this)[entry_]->getNode()));
    //xxx ROSE_ASSERT(isSgFunctionDefinition((*this)[exit_]->getNode()));
}

template <class PGNodeT, class PGEdgeT>
  void PG<PGNodeT, PGEdgeT>::setEntryAndExit(const std::set<PGNodeT>& startNs, 
					     const std::set<PGNodeT>& endNs) {
  // entry_ = startN;
  // exit_ = endN;
  
  entry_ = Vertex();
  exit_ = Vertex();

  Vertex from, to;
  from = entry_;
  // Add the edges for entry.
  BOOST_FOREACH(PGNodeT startN, startNs) {
    std::cout << "start node: " << startN->str() << std::endl;
    to = getVertexForNode(startN);
    Edge edge = add_edge(from, to, *this).first;
    // No edge mapping was added
    // (*this)[edge] = cfgEdge;
  }

  to = exit_;
  // Add the edges for exit
  BOOST_FOREACH(PGNodeT endN, endNs) {
    std::cout << "end node: " << endN->str() << std::endl;
    from = getVertexForNode(endN);
    Edge edge = add_edge(from, to, *this).first;
    // No edge mapping was added
    // (*this)[edge] = cfgEdge;
  }

  /*
    BOOST_FOREACH (Vertex v, boost::vertices(*this))
    {
        PGNodePtr node = (*this)[v];
        if (isSgFunctionDefinition(node->getNode()))
        {
            if (node->getIndex() == 0)
                entry_ = v;
            else if (node->getIndex() == 3)
                exit_ = v;
        }
    }

    //In graphs with an infinite loop, we might never get to the end vertex
    //In those cases, we need to add it explicitly
    if (exit_ == GraphTraits::null_vertex())
    {
        std::cerr << "This function may contain an infinite loop "
                "inside so that its PG cannot be built" << std::endl;
        exit_ = add_vertex(*this);
        (*this)[exit_] = PGNodePtr(new PGNodeType(funcDef_->cfgForEnd()));
    }

    ROSE_ASSERT(entry_ != GraphTraits::null_vertex());
    ROSE_ASSERT(exit_ != GraphTraits::null_vertex());
    */
}

template <class PGNodeT, class PGEdgeT>
void PG<PGNodeT, PGEdgeT>::buildPG(
        PGNodeType node,
        std::map<PGNodeType, Vertex>& nodesAdded, std::set<PGNodeType>& nodesProcessed)
{
  //xxx ROSE_ASSERT(node->getNode());

  if (nodesProcessed.count(node) > 0)
    return;
  nodesProcessed.insert(node);
  
  typename std::map<PGNodeType, Vertex>::iterator iter;
  bool inserted;
  Vertex from, to;
  
  // Add the source node.
  const PGNodeType src = node;
  //xxx ROSE_ASSERT(src.getNode());
  
  boost::tie(iter, inserted) = nodesAdded.insert(std::make_pair(src, Vertex()));
  
  if (inserted) {
    from = add_vertex(*this);
    (*this)[from] = src; //xx1 PGNodePtr(new PGNodeType(src));
    iter->second = from;
  } else {
    from = iter->second;
  }
  
  std::list<PGEdgeType> outEdges = node->outEdges();

  // std::cout << "src: " << node->str() << std::endl;
  //xx1 BOOST_FOREACH(const PGEdgeType& cfgEdge, outEdges)
  BOOST_FOREACH(PGEdgeType cfgEdge, outEdges) {
    // For each out edge, add the target node.
    PGNodeType tar = cfgEdge->target();
    //xxx ROSE_ASSERT(tar.getNode());
    
    boost::tie(iter, inserted) = nodesAdded.insert(std::make_pair(tar, Vertex()));
    // std::cout << "        dest: " << tar->str() << std::endl;
    if (inserted) {
      to = add_vertex(*this);
      (*this)[to] = tar; //xx1 PGNodePtr(new PGNodeType(tar));
      iter->second = to;
    } else {
      to = iter->second;
    }
    
    // Add the edge.
    Edge edge = add_edge(from, to, *this).first;
    (*this)[edge] = cfgEdge; //xx1 PGEdgePtr(new PGEdgeType(cfgEdge));
    
    // Build the PG recursively.
    buildPG(tar, nodesAdded, nodesProcessed);
  }
}

template <class PGNodeT, class PGEdgeT>
const typename PG<PGNodeT, PGEdgeT>::VertexVertexMap& PG<PGNodeT, PGEdgeT>::getDominatorTree()
{
    if (!dominatorTree_.empty())
        return dominatorTree_;

    boost::associative_property_map<VertexVertexMap> domTreePredMap(dominatorTree_);

    // Here we use the algorithm in boost::graph to build a map from each node to its immediate dominator.
    boost::lengauer_tarjan_dominator_tree(*this, entry_, domTreePredMap);
    return dominatorTree_;
}

template <class PGNodeT, class PGEdgeT>
const typename PG<PGNodeT, PGEdgeT>::VertexVertexMap& PG<PGNodeT, PGEdgeT>::getPostdominatorTree()
{
    if (!postdominatorTree_.empty())
        return postdominatorTree_;
    
    boost::associative_property_map<VertexVertexMap> postdomTreePredMap(postdominatorTree_);

    // Here we use the algorithm in boost::graph to build an map from each node to its immediate dominator.
    boost::lengauer_tarjan_dominator_tree(boost::make_reverse_graph(*this), exit_, postdomTreePredMap);
    return postdominatorTree_;
}

template <class PGNodeT, class PGEdgeT>
PG<PGNodeT, PGEdgeT> PG<PGNodeT, PGEdgeT>::makeReverseCopy() const
{
    PG<PGNodeT, PGEdgeT> reversePG;
    // The following function makes a reverse PG copy.
    boost::transpose_graph(*this, reversePG, 
        boost::vertex_copy(VertexCopier(*this, reversePG)).
        edge_copy(EdgeCopier(*this, reversePG)));

    // Swap entry and exit.
    reversePG.entry_ = this->exit_;
    reversePG.exit_ = this->entry_;
    return reversePG;
}

template <class PGNodeT, class PGEdgeT>
  //xx1 std::vector<typename PG<PGNodeT, PGEdgeT>::PGNodePtr>
  std::vector<PGNodeT> 
PG<PGNodeT, PGEdgeT>::getAllNodes() const
{
  //xx1 std::vector<PGNodePtr> allNodes;
  std::vector<PGNodeT> allNodes;
  BOOST_FOREACH (Vertex v, boost::vertices(*this))
    allNodes.push_back((*this)[v]);
  return allNodes;
}

template <class PGNodeT, class PGEdgeT>
  //xx1 std::vector<typename PG<PGNodeT, PGEdgeT>::PGEdgePtr>
  std::vector<PGEdgeT> 
PG<PGNodeT, PGEdgeT>::getAllEdges() const
{
  //xx1 std::vector<PGEdgePtr> allEdges;
  std::vector<PGEdgeT> allEdges;
  
  BOOST_FOREACH (const Edge& e, boost::edges(*this))
    allEdges.push_back((*this)[e]);
  return allEdges;
}

template <class PGNodeT, class PGEdgeT>
typename PG<PGNodeT, PGEdgeT>::Vertex PG<PGNodeT, PGEdgeT>::getVertexForNode(const PGNodeType &node) const
{
    typename std::map<PGNodeType, Vertex>::const_iterator vertexIter = nodesToVertices_.find(node);
    if (vertexIter == nodesToVertices_.end())
        return GraphTraits::null_vertex();
    else
    {
      //xx1 ROSE_ASSERT(*(*this)[vertexIter->second] == node);
      ROSE_ASSERT((*this)[vertexIter->second] == node);
      return vertexIter->second;
    }
}

template <class PGNodeT, class PGEdgeT>
std::vector<typename PG<PGNodeT, PGEdgeT>::Edge> PG<PGNodeT, PGEdgeT>::getAllBackEdges()
{
    std::vector<Edge> backEdges;

    // If the dominator tree is not built yet, build it now.
    getDominatorTree();

    BOOST_FOREACH (const Edge& e, boost::edges(*this))
    {
        Vertex src = boost::source(e, *this);
        Vertex tar = boost::target(e, *this);

        //Vertex v = *(dominatorTree.find(src));
        typename VertexVertexMap::const_iterator iter = dominatorTree_.find(src);
        while (iter != dominatorTree_.end())
        {
            if (iter->second == tar)
            {
                backEdges.push_back(e);
                break; // break the while loop
            }
            iter = dominatorTree_.find(iter->second);
        }
    }

    return backEdges;
}

template <class PGNodeT, class PGEdgeT>
std::vector<typename PG<PGNodeT, PGEdgeT>::Vertex> PG<PGNodeT, PGEdgeT>::getAllLoopHeaders()
{
    std::vector<Edge> backEdges = getAllBackEdges();
    std::vector<Vertex> headers;
    BOOST_FOREACH (Edge e, backEdges)
        headers.push_back(boost::target(e, *this));
    return headers;
}

/// Calculates the dominance frontier for each node in the control flow graph of the given function.
/// @param iDominatorMap map from each node to its immediate dominator
/// @param iPostDominatorMap map from each node to its immediate postdominator 
template<class PGNodeT, class PGEdgeT>
  map<PGNodeT, set<PGNodeT> > calculateDomFrontiers(// SgFunctionDefinition* func,
						    const std::set<PGNodeT>& startNs, 
						    const std::set<PGNodeT>& endNs,
						    map<PGNodeT, PGNodeT>* iDominatorMap,
						    map<PGNodeT, PGNodeT>* iPostDominatorMap) {
  // std::cout << "calc dominator" << std::endl;
  typedef PG<PGNodeT, PGEdgeT> PartitionGraph;
  
  // Build a PG first
  // PartitionGraph functionPG(func, startN, endN);
  PartitionGraph functionPG(// func, 
			    startNs, endNs);

  //Build the dominator tree
  typename PartitionGraph::VertexVertexMap dominatorTreeMap = functionPG.getDominatorTree();

  //TODO: This code converts a VertexVertex Map to a  boost graph. Should be factored out
  typedef adjacency_list<vecS, vecS, bidirectionalS, PGNodeT> TreeType;
  TreeType domTree;
  typedef typename graph_traits<TreeType>::vertex_descriptor TreeVertex;

  set<PGNodeT> addedNodes;
  map<PGNodeT, TreeVertex> pgNodeToVertex;

  BOOST_FOREACH(typename PartitionGraph::VertexVertexMap::value_type& nodeDominatorPair, dominatorTreeMap) {
    PGNodeT node = //xx1  *functionPG[nodeDominatorPair.first];
      functionPG[nodeDominatorPair.first];
    PGNodeT dominator = //xx1 *functionPG[nodeDominatorPair.second];
      functionPG[nodeDominatorPair.second];
    
    if (addedNodes.count(dominator) == 0) {
      TreeVertex newVertex = add_vertex(domTree);
      pgNodeToVertex[dominator] = newVertex;
      domTree[newVertex] = dominator;
      addedNodes.insert(dominator);
    }
    
    if (addedNodes.count(node) == 0) {
      TreeVertex newVertex = add_vertex(domTree);
      pgNodeToVertex[node] = newVertex;
      domTree[newVertex] = node;
      addedNodes.insert(node);
    }
    
    // std::cout << "dom info: " << node->str() << " dom: " << dominator->str() << std::endl;

    //Add the edge from dominator to node
    add_edge(pgNodeToVertex[dominator], pgNodeToVertex[node], domTree);
    
    if (iDominatorMap != NULL) {
      ROSE_ASSERT(iDominatorMap->count(node) == 0);
      iDominatorMap->insert(make_pair(node, dominator));
    }
  }
  
  //Get a topological ordering of the vertices
  vector<TreeVertex> reverseTopological;
  topological_sort(domTree, back_inserter(reverseTopological));
  
  //Calculate all the dominance frontiers. This algorithm is from figure 10, Cytron et. al 1991
  map<PGNodeT, set<PGNodeT> > dominanceFrontiers;

  BOOST_FOREACH(TreeVertex v, reverseTopological) {
    PGNodeT currentNode = domTree[v];
    set<PGNodeT>& currentDominanceFrontier = dominanceFrontiers[currentNode];
    
    //Local contribution: Iterate over all the successors of v in the control flow graph
    BOOST_FOREACH(PGEdgeT outEdge, currentNode->outEdges()) {
      PGNodeT successor = outEdge->target();
      
      //Get the immediate dominator of the successor
      typename PartitionGraph::Vertex successorVertex = functionPG.getVertexForNode(successor);
#if !USE_ROSE
      // DQ (11/3/2011): EDG compilains about this (but GNU allowed it, I think that EDG might be correct.
      // since it might be a private variable.  But since we are only trying to compile ROSE with ROSE (using the
      // new EDG 4.3 front-end as a tests) we can just skip this case for now.
      ROSE_ASSERT(successorVertex != PartitionGraph::GraphTraits::null_vertex());
#endif
      
      if (dominatorTreeMap.count(successorVertex) != 1) {
	// std::cout << "wrong dom succ: " << successor->str() << " with " 
	// 	  << dominatorTreeMap.count(successorVertex) << " and " 
	// 	  << currentNode->str() << std::endl;
	continue;
      }
      ROSE_ASSERT(dominatorTreeMap.count(successorVertex) == 1);
      typename PartitionGraph::Vertex iDominatorVertex = dominatorTreeMap[successorVertex];
      PGNodeT iDominator = //xx1 *functionPG[iDominatorVertex];
	functionPG[iDominatorVertex];
      
      //If we have a successor that we don't dominate, that successor is in our dominance frontier
      if (iDominator != currentNode) {
	currentDominanceFrontier.insert(successor);
      }
    }

    //"Up" contribution. Iterate over all children in the dominator tree
    typename graph_traits<TreeType>::adjacency_iterator currentIter, lastIter;
    for (tie(currentIter, lastIter) = adjacent_vertices(v, domTree); currentIter != lastIter; currentIter++) {
      PGNodeT childNode = domTree[*currentIter];
      const set<PGNodeT>& childDominanceFrontier = dominanceFrontiers[childNode];
      
      BOOST_FOREACH(PGNodeT childDFNode, childDominanceFrontier) {
	//Get the immediate dominator of the child DF node
	typename PartitionGraph::Vertex childDFVertex = functionPG.getVertexForNode(childDFNode);
#if !USE_ROSE
	// DQ (11/3/2011): EDG compilains about this (but GNU allowed it, I think that EDG might be correct.
	// since it might be a private variable.  But since we are only trying to compile ROSE with ROSE (using the
	// new EDG 4.3 front-end as a tests) we can just skip this case for now.
	ROSE_ASSERT(childDFVertex != PartitionGraph::GraphTraits::null_vertex());
#endif
	ROSE_ASSERT(dominatorTreeMap.count(childDFVertex) == 1);
	typename PartitionGraph::Vertex iDominatorVertex = dominatorTreeMap[childDFVertex];
	PGNodeT iDominator = //xx1 *functionPG[iDominatorVertex];
	  functionPG[iDominatorVertex];
	
	if (iDominator != currentNode) {
	    currentDominanceFrontier.insert(childDFNode);
	}
      }
    }
  }

  //While we're at it, calculate the postdominator tree
  if (iPostDominatorMap != NULL) {
    typename PartitionGraph::VertexVertexMap postDominatorTreeMap = functionPG.getPostdominatorTree();
    
    BOOST_FOREACH(typename PartitionGraph::VertexVertexMap::value_type& nodePostDominatorPair, postDominatorTreeMap) {
      PGNodeT node = //xx1 *functionPG[nodePostDominatorPair.first];
	functionPG[nodePostDominatorPair.first];
      PGNodeT postDominator = //xx1 *functionPG[nodePostDominatorPair.second];
	functionPG[nodePostDominatorPair.second];
      
      ROSE_ASSERT(iPostDominatorMap->count(node) == 0);
      iPostDominatorMap->insert(make_pair(node, postDominator));
    }
  }
  
  std::cout << "PG size: " << functionPG.getAllNodes().size() << std::endl;

  // Patchup the start node
  // TODO: is the patchup process still needed?
  // list<PartEdgePtr> outEdges = startN->outEdges();
  // PGNodeT startDom = (* outEdges.begin())->target();
  // dominanceFrontiers[startN].insert(startDom);

  return dominanceFrontiers;
};

/// Given the dominance frontiers of each node and a set of start nodes, calculate the iterated dominance frontier
/// of the start nodes.          
template<class PGNodeT>
set<PGNodeT> calculateIteratedDomFrontier(const map<PGNodeT, set<PGNodeT> >& dominanceFrontiers,
						 const vector<PGNodeT>& startNodes) {
  set<PGNodeT> result;
  set<PGNodeT> visitedNodes;
  vector<PGNodeT> worklist;

  worklist.insert(worklist.end(), startNodes.begin(), startNodes.end());

  while (!worklist.empty()) {
    PGNodeT currentNode = worklist.back();
    worklist.pop_back();
    visitedNodes.insert(currentNode);
    
    // Get the dominance frontier of the node and add it to the results
    ROSE_ASSERT(dominanceFrontiers.count(currentNode) != 0);
    const set<PGNodeT>& dominanceFrontier = dominanceFrontiers.find(currentNode)->second;
    
    // Add all the children to the result and to the worklist
    BOOST_FOREACH(PGNodeT dfNode, dominanceFrontier) {
      if (visitedNodes.count(dfNode) > 0)
	continue;
      
      result.insert(dfNode);
      worklist.push_back(dfNode);
    }
  }
  
  return result;
};

///
/// @param dominatorTree map from each node in the dom tree to its childrenn
/// @param iDominatorMap map from each node to its immediate dominator. 

template<class PGNodeT, class PGEdgeT>
  multimap< PGNodeT, pair<PGNodeT, PGEdgeT> >
  calculateCD(SgFunctionDefinition* function, map<PGNodeT, PGNodeT>& iPostDominatorMap, PGNodeT startN) {
  //Map from each node to the nodes it's control dependent on (and corresponding edges)
  multimap< PGNodeT, pair<PGNodeT, PGEdgeT> > controlDepdendences;
  
  //Let's iterate the control flow graph and stop every time we hit an edge with a condition
  set<PGNodeT> visited;
  set<PGNodeT> worklist;
  
  PGNodeT sourceNode = startN;
  worklist.insert(sourceNode);
  
  while (!worklist.empty()) {
    //Get the node to work on
    sourceNode = *worklist.begin();
    worklist.erase(worklist.begin());
    visited.insert(sourceNode);
    
    //For every edge, add it to the worklist
    std::list<PGEdgeT> outEdges = sourceNode->outEdges();
    BOOST_FOREACH(PGEdgeT edge, outEdges) {
      PGNodeT targetNode = edge->target();
      
      //Insert the child in the worklist if the it hasn't been visited yet
      if (visited.count(targetNode) == 0) {
	worklist.insert(targetNode);
      }
      
      //Check if we need to process this edge in control dependence calculation
      //xx1 if (edge.condition() == VirtualCFG::eckUnconditional)
      if (outEdges.size() == 1)
	continue;
      
      //We traverse from nextNode up in the postdominator tree until we reach the parent of currNode.
      PGNodeT parent;
      typename map<PGNodeT, PGNodeT>::const_iterator parentIter = iPostDominatorMap.find(sourceNode);
      ROSE_ASSERT(parentIter != iPostDominatorMap.end());
      parent = parentIter->second;
      
      //This is the node that we'll be marking as control dependent
      PGNodeT currNode = targetNode;
      while (true) {
	//If we reach the parent of the source, stop
	if (currNode == parent) {
	  break;
	}
	
	//Add a control dependence from the source to the new node
	controlDepdendences.insert(make_pair(currNode, make_pair(sourceNode, edge)));
	
	/* if (StaticSingleAssignment::getDebugExtra()) {
	  printf("%s is control-dependent on %s - %s \n", currNode.toStringForDebugging().c_str(),
		 sourceNode.toStringForDebugging().c_str(), 
		 edge.condition() == VirtualCFG::eckTrue ? "true" : "false");
	}*/
	
	//Move to the parent of the current node
	parentIter = iPostDominatorMap.find(currNode);
	ROSE_ASSERT(parentIter != iPostDominatorMap.end());
	currNode = parentIter->second;
      }
    }
  }
  
  return controlDepdendences;
};

template<class CfgNodeT, class CfgEdgeT>
  map<CfgNodeT, set<CfgNodeT> > calculateDomFrontiers_(SgFunctionDefinition* func, 
						       map<CfgNodeT, CfgNodeT>* iDominatorMap,
						       map<CfgNodeT, CfgNodeT>* iPostDominatorMap)
{
  typedef CFG<CfgNodeT, CfgEdgeT> ControlFlowGraph;

  // Build a CFG first
  ControlFlowGraph functionCfg(func);

  //Build the dominator tree
  typename ControlFlowGraph::VertexVertexMap dominatorTreeMap = functionCfg.getDominatorTree();

  //TODO: This code converts a VertexVertex Map to a  boost graph. Should be factored out
  typedef adjacency_list<vecS, vecS, bidirectionalS, CfgNodeT> TreeType;
  TreeType domTree;
  typedef typename graph_traits<TreeType>::vertex_descriptor TreeVertex;

  set<CfgNodeT> addedNodes;
  map<CfgNodeT, TreeVertex> cfgNodeToVertex;

  BOOST_FOREACH(typename ControlFlowGraph::VertexVertexMap::value_type& nodeDominatorPair, dominatorTreeMap)
    {
      CfgNodeT node = *functionCfg[nodeDominatorPair.first];
      CfgNodeT dominator = *functionCfg[nodeDominatorPair.second];

      if (addedNodes.count(dominator) == 0)
	{
	  TreeVertex newVertex = add_vertex(domTree);
	  cfgNodeToVertex[dominator] = newVertex;
	  domTree[newVertex] = dominator;
	  addedNodes.insert(dominator);
	}

      if (addedNodes.count(node) == 0)
	{
	  TreeVertex newVertex = add_vertex(domTree);
	  cfgNodeToVertex[node] = newVertex;
	  domTree[newVertex] = node;
	  addedNodes.insert(node);
	}

      //Add the edge from dominator to node
      add_edge(cfgNodeToVertex[dominator], cfgNodeToVertex[node], domTree);

      if (iDominatorMap != NULL)
	{
	  ROSE_ASSERT(iDominatorMap->count(node) == 0);
	  iDominatorMap->insert(make_pair(node, dominator));
	}
    }

  //Get a topological ordering of the vertices
  vector<TreeVertex> reverseTopological;
  topological_sort(domTree, back_inserter(reverseTopological));

  //Calculate all the dominance frontiers. This algorithm is from figure 10, Cytron et. al 1991
  map<CfgNodeT, set<CfgNodeT> > dominanceFrontiers;

  BOOST_FOREACH(TreeVertex v, reverseTopological)
    {
      CfgNodeT currentNode = domTree[v];
      set<CfgNodeT>& currentDominanceFrontier = dominanceFrontiers[currentNode];

      //Local contribution: Iterate over all the successors of v in the control flow graph

      BOOST_FOREACH(CfgEdgeT outEdge, currentNode.outEdges())
	{
	  CfgNodeT successor = outEdge.target();

	  //Get the immediate dominator of the successor
	  typename ControlFlowGraph::Vertex successorVertex = functionCfg.getVertexForNode(successor);
#if !USE_ROSE
	  // DQ (11/3/2011): EDG compilains about this (but GNU allowed it, I think that EDG might be correct.
	  // since it might be a private variable.  But since we are only trying to compile ROSE with ROSE (using the
	  // new EDG 4.3 front-end as a tests) we can just skip this case for now.
	  ROSE_ASSERT(successorVertex != ControlFlowGraph::GraphTraits::null_vertex());
#endif
	  ROSE_ASSERT(dominatorTreeMap.count(successorVertex) == 1);
	  typename ControlFlowGraph::Vertex iDominatorVertex = dominatorTreeMap[successorVertex];
	  CfgNodeT iDominator = *functionCfg[iDominatorVertex];

	  //If we have a successor that we don't dominate, that successor is in our dominance frontier
	  if (iDominator != currentNode)
	    {
	      currentDominanceFrontier.insert(successor);
	    }
	}

      //"Up" contribution. Iterate over all children in the dominator tree
      typename graph_traits<TreeType>::adjacency_iterator currentIter, lastIter;
      for (tie(currentIter, lastIter) = adjacent_vertices(v, domTree); currentIter != lastIter; currentIter++)
	{
	  CfgNodeT childNode = domTree[*currentIter];
	  const set<CfgNodeT>& childDominanceFrontier = dominanceFrontiers[childNode];

	  BOOST_FOREACH(CfgNodeT childDFNode, childDominanceFrontier)
	    {
	      //Get the immediate dominator of the child DF node
	      typename ControlFlowGraph::Vertex childDFVertex = functionCfg.getVertexForNode(childDFNode);
#if !USE_ROSE
	      // DQ (11/3/2011): EDG compilains about this (but GNU allowed it, I think that EDG might be correct.
	      // since it might be a private variable.  But since we are only trying to compile ROSE with ROSE (using the
	      // new EDG 4.3 front-end as a tests) we can just skip this case for now.
	      ROSE_ASSERT(childDFVertex != ControlFlowGraph::GraphTraits::null_vertex());
#endif
	      ROSE_ASSERT(dominatorTreeMap.count(childDFVertex) == 1);
	      typename ControlFlowGraph::Vertex iDominatorVertex = dominatorTreeMap[childDFVertex];
	      CfgNodeT iDominator = *functionCfg[iDominatorVertex];

	      if (iDominator != currentNode)
		{
		  currentDominanceFrontier.insert(childDFNode);
		}
	    }
	}
    }
   
  //While we're at it, calculate the postdominator tree
  if (iPostDominatorMap != NULL)
    {
      typename ControlFlowGraph::VertexVertexMap postDominatorTreeMap = functionCfg.getPostdominatorTree();

      BOOST_FOREACH(typename ControlFlowGraph::VertexVertexMap::value_type& nodePostDominatorPair, postDominatorTreeMap)
	{
	  CfgNodeT node = *functionCfg[nodePostDominatorPair.first];
	  CfgNodeT postDominator = *functionCfg[nodePostDominatorPair.second];

	  ROSE_ASSERT(iPostDominatorMap->count(node) == 0);
	  iPostDominatorMap->insert(make_pair(node, postDominator));
	}
    }

  std::cout << "CFG size: " << functionCfg.getAllNodes().size() << std::endl;

  return dominanceFrontiers;
}
}

