// This file holds the portions of partitions.C that cannot be compiled when sight.h is included

#include "sage3basic.h"
using namespace std;

#include "abstract_object.h"
#include "compose.h"
#include "partitions.h"
#include <boostGraphCFG.h>
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

using namespace boost;

namespace fuse {

const SSAGraph::VertexVertexMap& SSAGraph::getDominatorTree() //const
{
  if (!dominatorTree.empty())
      return dominatorTree;

  boost::associative_property_map<VertexVertexMap> domTreePredMap(dominatorTree);

  std::set<PartPtr> startStates = comp->GetStartAStates(analysis);

  // Here we use the algorithm in boost::graph to build a map from each node to its immediate dominator.
  StartPartPtr start = makePtr<StartPart>(startStates);
  boost::lengauer_tarjan_dominator_tree(*this, nodesToVertices[start], domTreePredMap);
  return dominatorTree;
}

const SSAGraph::VertexVertexMap& SSAGraph::getPostdominatorTree() //const
{
  if (!postdominatorTree.empty())
    return postdominatorTree;

  boost::associative_property_map<VertexVertexMap> postdomTreePredMap(postdominatorTree);

  std::set<PartPtr> endStates = comp->GetEndAStates(analysis);

  // Here we use the algorithm in boost::graph to build an map from each node to its immediate dominator.
/// !!!! Temporary: only consider the first starting node as the entry
/// !!!! TODO: create a starting node that precedes all the others
  EndPartPtr end = makePtr<EndPart>(endStates);
  boost::lengauer_tarjan_dominator_tree(boost::make_reverse_graph(*this), nodesToVertices[end], postdomTreePredMap);
  return postdominatorTree;
}

void SSAGraph::calculateDominanceFrontiers()
{
  //cout << "SSAGraph::calculateDominanceFrontiers() #dominanceFrontiers="<<dominanceFrontiers.size()<<endl;
  boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, PartPtr, PartEdgePtr> ATS;

  //Build the dominator tree
  SSAGraph::VertexVertexMap dominatorTreeMap = getDominatorTree();

  //TODO: This code converts a VertexVertex Map to a  boost graph. Should be factored out
  typedef adjacency_list<vecS, vecS, bidirectionalS, PartPtr> TreeType;
  TreeType domTree;
  typedef graph_traits<TreeType>::vertex_descriptor TreeVertex;

  set<PartPtr> addedNodes;
  map<PartPtr, TreeVertex> PartPtroVertex;

  BOOST_FOREACH(SSAGraph::VertexVertexMap::value_type& nodeDominatorPair, dominatorTreeMap)
  {
    PartPtr node = (*this)[nodeDominatorPair.first];
    PartPtr dominator = (*this)[nodeDominatorPair.second];

    if (addedNodes.count(dominator) == 0)
    {
      TreeVertex newVertex = add_vertex(domTree);
      PartPtroVertex[dominator] = newVertex;
      domTree[newVertex] = dominator;
      addedNodes.insert(dominator);
    }

    if (addedNodes.count(node) == 0)
    {
      TreeVertex newVertex = add_vertex(domTree);
      PartPtroVertex[node] = newVertex;
      domTree[newVertex] = node;
      addedNodes.insert(node);
    }

    //Add the edge from dominator to node
    add_edge(PartPtroVertex[dominator], PartPtroVertex[node], domTree);

    ROSE_ASSERT(iDominatorMap.count(node) == 0);
    iDominatorMap.insert(make_pair(node, dominator));
  }

  //Get a topological ordering of the vertices
  vector<TreeVertex> reverseTopological;
  topological_sort(domTree, back_inserter(reverseTopological));

  //Calculate all the dominance frontiers. This algorithm is from figure 10, Cytron et. al 1991

  /*cout << "Calculate dominance frontiers"<<endl;
  cout << "    dominatorTreeMap="<<endl;
  BOOST_FOREACH(SSAGraph::VertexVertexMap::value_type& nodeDominatorPair, dominatorTreeMap)
  {
    PartPtr node = (*this)[nodeDominatorPair.first];
    PartPtr dominator = (*this)[nodeDominatorPair.second];
    cout << "        node="<<node->str()<<endl;
    cout << "        dominator="<<dominator->str()<<endl;
  }*/
/*  cout << "    nodesToVertices(#"<<nodesToVertices.size()<<")="<<endl;
  for(map<PartPtr, Vertex>::iterator i=nodesToVertices.begin(); i!=nodesToVertices.end(); i++)
    cout << "        "<<i->second<<": "<<i->first->str()<<endl;*/
 
  BOOST_FOREACH(TreeVertex v, reverseTopological)
  {
    PartPtr currentNode = domTree[v];
    //cout << "currentNode="<<currentNode->str()<<", #dominanceFrontiers="<<dominanceFrontiers.size()<<endl;
    set<PartPtr>& currentDominanceFrontier = dominanceFrontiers[currentNode];

    //Local contribution: Iterate over all the successors of v in the control flow graph

    BOOST_FOREACH(PartEdgePtr outEdge, currentNode->outEdges())
    {
      PartPtr successor = outEdge->target();

      //Get the immediate dominator of the successor
      SSAGraph::Vertex successorVertex = getVertexForNode(successor);
      /*cout << "    successor("<<successorVertex<<")="<<successor->str()<<endl;
      cout << "    successor->inEdges="<<endl;
      BOOST_FOREACH(PartEdgePtr inEdge, successor->inEdges()) {
        cout << "        "<<inEdge->str()<<endl;
      }*/
#if !USE_ROSE
     // DQ (11/3/2011): EDG compilains about this (but GNU allowed it, I think that EDG might be correct.
     // since it might be a private variable.  But since we are only trying to compile ROSE with ROSE (using the
     // new EDG 4.3 front-end as a tests) we can just skip this case for now.
        ROSE_ASSERT(successorVertex != SSAGraph::GraphTraits::null_vertex());
#endif
      //ROSE_ASSERT(dominatorTreeMap.count(successorVertex) == 1);
      if(dominatorTreeMap.count(successorVertex) == 1) {
        SSAGraph::Vertex iDominatorVertex = dominatorTreeMap[successorVertex];
        PartPtr iDominator = (*this)[iDominatorVertex];

        // If we have a successor that we don't dominate, that successor is in our dominance frontier
        if (iDominator != currentNode)
        {
          currentDominanceFrontier.insert(successor);
          domFrontierOf[successor].insert(currentNode);
        }
      }
    }

    //"Up" contribution. Iterate over all children in the dominator tree
    graph_traits<TreeType>::adjacency_iterator currentIter, lastIter;
    for (tie(currentIter, lastIter) = adjacent_vertices(v, domTree); currentIter != lastIter; currentIter++)
    {
      PartPtr childNode = domTree[*currentIter];
      const set<PartPtr>& childDominanceFrontier = dominanceFrontiers[childNode];

      BOOST_FOREACH(PartPtr childDFNode, childDominanceFrontier)
      {
        //Get the immediate dominator of the child DF node
        SSAGraph::Vertex childDFVertex = getVertexForNode(childDFNode);
#if !USE_ROSE
       // DQ (11/3/2011): EDG compilains about this (but GNU allowed it, I think that EDG might be correct.
       // since it might be a private variable.  But since we are only trying to compile ROSE with ROSE (using the
       // new EDG 4.3 front-end as a tests) we can just skip this case for now.
          ROSE_ASSERT(childDFVertex != SSAGraph::GraphTraits::null_vertex());
#endif
        ROSE_ASSERT(dominatorTreeMap.count(childDFVertex) == 1);
        SSAGraph::Vertex iDominatorVertex = dominatorTreeMap[childDFVertex];
        PartPtr iDominator = (*this)[iDominatorVertex];

        if (iDominator != currentNode)
        {
          currentDominanceFrontier.insert(childDFNode);
          domFrontierOf[childDFNode].insert(currentNode);
        }
      }
    }
  }

  //While we're at it, calculate the postdominator tree
  SSAGraph::VertexVertexMap postDominatorTreeMap = getPostdominatorTree();

  BOOST_FOREACH(SSAGraph::VertexVertexMap::value_type& nodePostDominatorPair, postDominatorTreeMap)
  {
    PartPtr node = (*this)[nodePostDominatorPair.first];
    PartPtr postDominator = (*this)[nodePostDominatorPair.second];

    ROSE_ASSERT(iPostDominatorMap.count(node) == 0);
    iPostDominatorMap.insert(make_pair(node, postDominator));
  }
}

}; // namespace fuse
