#include "sage3basic.h"
#include "compose.h"
#include "saveDotAnalysis.h"
#include "partitions.h"
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>
#include "ats_graph_structure.h"

using namespace std;
using namespace sight;
using namespace sight::structure;

namespace fuse {

#define ats2DotDebugLevel 0

/**************************************
 *** ATS Graph Visualizer for Sight ***
 **************************************/

void printAnchor_atsGraph(std::ostream &o, anchor a, string label, string indent);
// Helper function to print Part information
// doAnchorRankSame - if true, the anchors of this Part are placed on the same horizontal rank in the generated dot graph
void printPart_atsGraph(std::ostream &o, partDotInfoPtr info, PartPtr part, bool doAnchorRankSame, string indent);
void printPartEdge_atsGraph(map<PartPtr, partDotInfoPtr>& partInfo, PartEdgePtr e,
                            bool isInEdge, string indent);

/*************************
 ***** Ctxt2PartsMap *****
 *************************/

Ctxt2PartsMap_atsGraph::Ctxt2PartsMap_atsGraph(bool crossAnalysisBoundary, Ctxt2PartsMap_GeneratorPtr mgen, Ctxt2PartsMap_Leaf_GeneratorPtr lgen) :
  Ctxt2PartsMap(crossAnalysisBoundary, mgen, lgen) {}
Ctxt2PartsMap_atsGraph::Ctxt2PartsMap_atsGraph(bool crossAnalysisBoundary, const std::list<std::list<PartContextPtr> >& key, PartPtr part,
        Ctxt2PartsMap_GeneratorPtr mgen, Ctxt2PartsMap_Leaf_GeneratorPtr lgen) :
  Ctxt2PartsMap(crossAnalysisBoundary, key, part, mgen, lgen) {}

// Prints out the hierarchical sub-graphs of Parts denoted by this map to the given output stream
// partInfo: maps Parts to the information required to display them in the dot graph
// subgraphName: name of the subgraph that contains the current level in the map
void Ctxt2PartsMap_atsGraph::map2dot(std::ostream& o, std::map<PartPtr, partDotInfoPtr>& partInfo, std::string subgraphName, std::string indent) const {
  //cout << indent << "Ctxt2PartsMap::map2dot() subgraphName="<<subgraphName<<" #m="<<m.size()<<endl;
  l->map2dot(o, partInfo, subgraphName+"_L", indent+"    ");
  if(m.size()==0) return;

  int i=0;
  for(std::map<PartContextPtr, Ctxt2PartsMap*>::const_iterator c=m.begin(); c!=m.end(); c++, i++) {
    properties* props = new properties();

    map<string, string> sgPMap;
    sgPMap["name"] = txt()<<subgraphName<<"_N"<<i;
    sgPMap["crossAnalysisBoundary"] = (crossAnalysisBoundary? "1": "0");
    sgPMap["SgType"] = "context";

    string label = c->first.get()->str();
    boost::replace_all(label, "\n", "\\n");
    sgPMap["label"] = label;

    props->add("atsSubGraph", sgPMap);
    //dbg.enter(&obj);
    structure::sightObj obj(props);

    c->second->map2dot(o, partInfo, txt()<<subgraphName<<"_N"<<i, indent+"    ");

    //dbg.exit(&obj);
  }
}

/***************************************
 ***** Ctxt2PartsMap_Leaf_atsGraph *****
 ***************************************/
class partStr {
  public:
  string part;
  string anchor;
  partStr(string part, string anchor) : part(part), anchor(anchor) {}
};

// allAnchors - If true, the anchor field of the returned partStr object contains
//              a space-separated list of all the anchors associated with this Part.
//              If false, it contains just one representative anchor.
partStr getPartStr(std::map<PartPtr, partDotInfoPtr>& partInfo, PartPtr p, bool allAnchors) {
  partDotInfo_atsGraphPtr pi = boost::dynamic_pointer_cast<partDotInfo_atsGraph>(partInfo[p]);

  assert(pi);
  assert(pi->anchors.size()>0);
  ostringstream partStream;       partStream       << "clusterPart"<<pi->partID;
  ostringstream partAnchorStream;
  if(allAnchors) {
    for(list<anchor>::iterator a=pi->anchors.begin(); a!=pi->anchors.end(); a++) {
      if(a!=pi->anchors.begin()) partAnchorStream << " ";
      partAnchorStream << "a"<<a->getID();
    }
  }
  else
    partAnchorStream << "a"<<pi->anchors.front().getID();

  return partStr(partStream.str(), partAnchorStream.str());
}

// Prints out the hierarchical sub-graphs of Parts denoted by this map to the given output stream
// partInfo: maps Parts to the information required to display them in the dot graph
// subgraphName: name of the subgraph that contains the current level in the map
void Ctxt2PartsMap_Leaf_atsGraph::map2dot(std::ostream& o, std::map<PartPtr, partDotInfoPtr>& partInfo, std::string subgraphName, std::string indent) const {
  SIGHT_VERB_DECL(scope, (txt()<<"Ctxt2PartsMap_Leaf_atsGraph::map2dot() subgraphName="<<subgraphName<<" #m="<<m.size()), 2, ats2DotDebugLevel)

  if(m.size()==0) return;

  int i=0;
  for(map<PartContextPtr, set<PartPtr> >::const_iterator c=m.begin(); c!=m.end(); c++, i++) {
    properties* sgProps = new properties();

    map<string, string> sgPMap;
    sgPMap["name"] = txt()<<subgraphName<<i;
    sgPMap["crossAnalysisBoundary"] = "0";
    sgPMap["SgType"] = "context";

    string label = c->first.get()->str();
    boost::replace_all(label, "\n", "\\n");
    sgPMap["label"] = label;

    // Set the entry nodes to the minimum position
    bool sourceDiscovered=false;
    ostringstream source;
    for(set<PartPtr>::iterator p=c->second.begin(); p!=c->second.end(); p++) {
      if(partInfo.find(*p) == partInfo.end()) continue;

      if((*p)->maySgNodeAny<SgFunctionParameterList>()) {
        partStr ps = getPartStr(partInfo, *p, true);

        if(!sourceDiscovered) sourceDiscovered = true;
        source << ps.anchor<<" ";
      }
    }
    if(sourceDiscovered) sgPMap["source"] = source.str();

    // Set the exit nodes to the maximum position
    bool sinkDiscovered=false;
    ostringstream sink;
    for(set<PartPtr>::iterator p=c->second.begin(); p!=c->second.end(); p++) {
      if(partInfo.find(*p) == partInfo.end()) continue;

      if((*p)->maySgNodeAny<SgFunctionDefinition>()) {
        partStr ps = getPartStr(partInfo, *p, true);

        if(!sinkDiscovered) sinkDiscovered = true;
        sink << ps.anchor<<" ";
      }
    }
    if(sinkDiscovered) sgPMap["sink"] = sink.str();

    sgProps->add("atsSubGraph", sgPMap);
    structure::sightObj subGraph(sgProps);
    //dbg.enter(&subGraph);

    // Sets of all the outgoing and incoming function call states
    set<PartPtr> funcCallsOut, funcCallsIn;

    // Iterate over all the states in this context, printing them and recording the function calls
    for(set<PartPtr>::iterator p=c->second.begin(); p!=c->second.end(); p++) {
      SIGHT_VERB(dbg << "part="<<p->get()->str(), 2, ats2DotDebugLevel)

      if(partInfo.find(*p) == partInfo.end()) continue;

      set<CFGNode> matchNodes;
      if(p->get()->mayIncomingFuncCall(matchNodes))      funcCallsIn.insert(*p);
      else if(p->get()->mayOutgoingFuncCall(matchNodes)) funcCallsOut.insert(*p);
      else if((*p)->maySgNodeAny<SgFunctionParameterList>() || (*p)->maySgNodeAny<SgFunctionDefinition>())
        printPart_atsGraph(o, partInfo[*p], *p, false, indent+"  ");
      else
        printPart_atsGraph(o, partInfo[*p], *p, true, indent+"  ");
    }

    // Add invisible edges between matching in-out function call states
    int inIdx=0;
    for(set<PartPtr>::iterator in=funcCallsIn.begin(); in!=funcCallsIn.end(); in++, inIdx++) {
      properties* callSGProps = new properties();
      map<string, string> callPMap;
      callPMap["name"] = txt()<<subgraphName<<i<<"_Call"<<inIdx;
      callPMap["crossAnalysisBoundary"] = "0";
      callPMap["SgType"] = "callEdges";

      partStr psIn = getPartStr(partInfo, *in, true);
      callPMap["sink"] = psIn.anchor;

      ostringstream source;
      int numMatchedCalls=0;
      for(set<PartPtr>::iterator out=funcCallsOut.begin(); out!=funcCallsOut.end(); out++) {
        if(in->get()->mayMatchFuncCall(*out)) {
          partStr psOut = getPartStr(partInfo, *out, true);
          source << psOut.anchor << (numMatchedCalls==0? "": " ");
          numMatchedCalls++;
        }
      }
      callPMap["source"] = source.str();

      callSGProps->add("atsSubGraph", callPMap);
      structure::sightObj callSG(callSGProps);
      //dbg.enter(&callSG);

      printPart_atsGraph(o, partInfo[*in], *in, false, indent+"      ");

      for(set<PartPtr>::iterator out=funcCallsOut.begin(); out!=funcCallsOut.end(); out++) {
        if(in->get()->mayMatchFuncCall(*out)) {
          printPart_atsGraph(o, partInfo[*out], *out, false, indent+"      ");
        }
      }

      //dbg.exit(&callSG);
      // Call sub-graph exited
    }
    //dbg.exit(&subGraph);
    // Sub-graph exited
  }
}

// Helper function to print Part anchor information
void printAnchor_atsGraph(std::ostream &o, anchor a, string label, string indent)
{
  properties* anchorProps = new properties();
  map<string, string> anchorPMap;
  anchorPMap["label"] = label;
  anchorPMap["anchorID"] = txt()<<a.getID();

  anchorProps->add("atsGraphAnchor", anchorPMap);
  structure::sightObj anchorObj(anchorProps);
  //dbg.tag(&anchorObj);
}

// Helper function to print Part information
// doAnchorRankSame - if true, the anchors of this Part are placed on the same horizontal rank in the generated dot graph
void printPart_atsGraph(std::ostream &o, partDotInfoPtr info, PartPtr part, bool doAnchorRankSame, string indent)
{
  //assert(boost::dynamic_pointer_cast<partDotInfo_atsGraph>(info));
  properties* partProps = new properties();
  map<string, string> partPMap;
  partPMap["name"] = txt()<<"clusterPart"<<boost::dynamic_pointer_cast<partDotInfo_atsGraph>(info)->partID;
  partPMap["crossAnalysisBoundary"] = "0";
  partPMap["SgType"] = "part";

  set<CFGNode> nodes = part->CFGNodes();
  ostringstream label;
  for(set<CFGNode>::iterator n=nodes.begin(); n!=nodes.end(); n++) {
    if(n!=nodes.begin()) label << "\\n";
    if(isSgFunctionDefinition(n->getNode())) {
      Function func(isSgFunctionDefinition(n->getNode()));
      label << func.get_name().getString() << "() END";
    } else
      label << CFGNode2Str(*n);
  }

  // Compress the label horizontally to ensure that it is not too wide by adding line-breaks
  string labelStr = label.str();
  unsigned int lineWidth = 30;
  string labelMultLineStr = "";
  unsigned int i=0;
  while(i<labelStr.length()-lineWidth) {
    // Look for the next line-break
    unsigned int nextLB = labelStr.find_first_of("\n", i);
    // If the next line is shorter than lineWidth, add it to labelMulLineStr and move on to the next line
    if(nextLB-i < lineWidth) {
      labelMultLineStr += labelStr.substr(i, nextLB-i+1);
      i = nextLB+1;
    // If the next line is longer than lineWidth, add just lineWidth characters to labelMulLineStr
    } else {
      // If it is not much longer than lineWidth, don't break it up
      if(i>=labelStr.length()-lineWidth*1.25) break;
      labelMultLineStr += labelStr.substr(i, lineWidth) + "\\n";
      i += lineWidth;
    }
  }
  // Add the last line in labelStr to labelMulLineStr
  labelMultLineStr += labelStr.substr(i, labelStr.length()-i);

  partPMap["label"] = labelMultLineStr;

  list<anchor>& anchors = boost::dynamic_pointer_cast<partDotInfo_atsGraph>(info)->anchors;
  if(doAnchorRankSame) {
    // Ensure that all anchors in this part are on the same horizontal level
    ostringstream same;
    for(list<anchor>::iterator a=anchors.begin(); a!=anchors.end(); a++)
      same << "a" << a->getID() << " ";
    partPMap["same"] = same.str();
  }

  partProps->add("atsSubGraph", partPMap);
  structure::sightObj partObj(partProps);
  //dbg.enter(&partObj);

  // Print all the anchors inside this Part
  int j=0;
  for(list<anchor>::iterator a=anchors.begin(); a!=anchors.end(); a++, j++) {
    printAnchor_atsGraph(o, *a, txt()<<"*"<<j<<"*", indent+"    ");

    /* // Add an invisible edge from this anchor to the one that immediately follows it to
    // ensure a left-to-right order among them
    list<anchor>::iterator nextA = a; nextA++;
    if(nextA!=anchors.end())
      o << indent << "    a"<<a->getID() << " -> a"<<nextA->getID() << " [style=invis]" << endl;*/
  }

  //dbg.exit(&partObj);
  // partObj exited
}

// Printer for edges between Part clusters
void printPartEdge_atsGraph(map<PartPtr, partDotInfoPtr>& partInfo, PartEdgePtr e,
                            bool isInEdge, string indent)
{
  /*map<CFGNode, boost::shared_ptr<SgValueExp> > pv = e->getPredicateValue();
  string color = "black";
  ostringstream values;

  if(pv.size()>0) {
    // Flags that record whether all the SgValues were boolean with a true/false value.
    bool booleanTrue = true;
    bool booleanFalse = true;
    for(map<CFGNode, boost::shared_ptr<SgValueExp> >::iterator v=pv.begin(); v!=pv.end(); v++) {
      if(v!=pv.begin()) values << ", ";
      values << v->second->unparseToString();
      if(ValueObject::isValueBoolCompatible(v->second)) {
        booleanTrue  = booleanTrue  &&  ValueObject::SgValue2Bool(v->second);
        booleanFalse = booleanFalse && !ValueObject::SgValue2Bool(v->second);
      }
    }

    assert(!(booleanTrue && booleanFalse));
    if(booleanTrue)  color = "blue3";
    if(booleanFalse) color = "crimson";
  }*/

  /*partStr psSource = getPartStr(partInfo, e->source());
  partStr psTarget = getPartStr(partInfo, e->target());

  o << indent << psSource.anchor << " -> "<<psTarget.anchor <<
       "\t[lhead="<<psSource.part<<", "<<
         " ltail="<<psTarget.part<<", "<<
         " label=\"" << escapeString(values.str()) << "\","<<
         " style=\"" << (isInEdge ? "dotted" : "solid") << "\", " <<
         " color=\"" << color << "\"];\n";*/
}

// Printer for edges between individual anchor nodes
void printAnchorEdge_atsGraph(anchor fromAnchor, PartPtr fromPart,
                              anchor toAnchor,   PartPtr toPart, string indent)

{
  // !! Need to figure out whether the anchor ever got connected to a real spot in the output
  //if(toAnchor.isLocated()) {
    properties* props = new properties();

    map<string, string> pMap;

    // Record whether this edge crosses function boundaries
    set<CFGNode> matchNodes;
    pMap["crossFunc"] = (toPart->mayIncomingFuncCall(matchNodes) || fromPart->mayOutgoingFuncCall(matchNodes)? "1": "0");
    pMap["from"] = txt()<<fromAnchor.getID();
    pMap["to"]   = txt()<<toAnchor.getID();
    props->add("atsGraphEdge", pMap);

    //dbg.tag("dirEdge", properties, false);
    //dbg.tag(&obj);
    structure::sightObj obj(props);
  //}
}

class Ctxt2PartsMap_Generator_atsGraph : public Ctxt2PartsMap_Generator {
  public:
  Ctxt2PartsMap* newMap(bool crossAnalysisBoundary, Ctxt2PartsMap_GeneratorPtr mgen, Ctxt2PartsMap_Leaf_GeneratorPtr lgen) const
  { return new Ctxt2PartsMap_atsGraph(crossAnalysisBoundary, mgen, lgen); }
};

class Ctxt2PartsMap_Leaf_Generator_atsGraph : public Ctxt2PartsMap_Leaf_Generator {
  public:
  Ctxt2PartsMap_Leaf* newLeaf() const { return new Ctxt2PartsMap_Leaf_atsGraph(); }
};

/********************
 ***** atsGraph *****
 ********************/

// startPart(s) - the Parts from which the iteration of the ATS should start. The ATS graph performs a forward
//    iteration through the ATS so these should be the entry Part(s).
// partAnchors - maps each Part in the ATS to the anchors that point to blocks associated with it
// dirAligned - true if the edges between anchors are pointing in the same direction as the ATS flow of control
//    and false if they point in the opposite direcction
atsGraph::atsGraph(PartPtr startPart, boost::shared_ptr<map<PartPtr, list<anchor> > > partAnchors, bool dirAligned, properties* props) :
  graph(false, setProperties(startPart, partAnchors, dirAligned, NULL, props)), partAnchors(partAnchors), dirAligned(dirAligned)
{
  if(!props->active) return;

  startParts.insert(startPart);
}

atsGraph::atsGraph(const std::set<PartPtr>& startParts, boost::shared_ptr<map<PartPtr, list<anchor> > > partAnchors, bool dirAligned, properties* props) :
  graph(false, setProperties(startParts, partAnchors, dirAligned, NULL, props)), startParts(startParts), partAnchors(partAnchors), dirAligned(dirAligned)
{ }

atsGraph::atsGraph(const std::list<PartPtr>& startParts_, boost::shared_ptr<map<PartPtr, list<anchor> > > partAnchors, bool dirAligned, properties* props) :
  graph(false, setProperties(startParts, partAnchors, dirAligned, NULL, props)), partAnchors(partAnchors), dirAligned(dirAligned)
{
  for(list<PartPtr>::const_iterator p=startParts_.begin(); p!=startParts_.end(); ++p)
    startParts.insert(*p);
}

atsGraph::atsGraph(PartPtr startPart, boost::shared_ptr<map<PartPtr, list<anchor> > > partAnchors, bool dirAligned, const attrOp& onoffOp, properties* props) :
  graph(onoffOp, false, setProperties(startPart, partAnchors, dirAligned, &onoffOp, props)), partAnchors(partAnchors), dirAligned(dirAligned)
{
  if(!props->active) return;

  startParts.insert(startPart);
}

atsGraph::atsGraph(const std::set<PartPtr>& startParts, boost::shared_ptr<map<PartPtr, list<anchor> > > partAnchors, bool dirAligned, const attrOp& onoffOp, properties* props) :
  graph(onoffOp, false, setProperties(startParts, partAnchors, dirAligned, &onoffOp, props)), startParts(startParts), partAnchors(partAnchors), dirAligned(dirAligned)
{ }

atsGraph::atsGraph(const std::list<PartPtr>& startParts_, boost::shared_ptr<map<PartPtr, list<anchor> > > partAnchors, bool dirAligned, const attrOp& onoffOp, properties* props) :
  graph(onoffOp, false, setProperties(startParts, partAnchors, dirAligned, &onoffOp, props)), startParts(startParts), partAnchors(partAnchors), dirAligned(dirAligned)
{
  for(list<PartPtr>::const_iterator p=startParts_.begin(); p!=startParts_.end(); ++p)
    startParts.insert(*p);
}


properties* atsGraph::setProperties(PartPtr startPart, boost::shared_ptr<map<PartPtr, list<anchor> > >& partAnchors, bool dirAligned, const attrOp* onoffOp, properties* props) {
  set<PartPtr> startParts;
  startParts.insert(startPart);
  return setProperties(startParts, partAnchors, dirAligned, onoffOp, props);
}

properties* atsGraph::setProperties(const std::set<PartPtr>& startParts, boost::shared_ptr<map<PartPtr, list<anchor> > >& partAnchors, bool dirAligned, const attrOp* onoffOp, properties* props) {
  if(props==NULL) props = new properties();

  // If the current attribute query evaluates to true (we're emitting debug output) AND
  // either onoffOp is not provided or its evaluates to true
  if(attributes->query() && (onoffOp? onoffOp->apply(): true)) {
    props->active = true;
    map<string, string> newProps;
    newProps["dirAligned"] = (dirAligned? "1": "0");
    props->add("atsGraph", newProps);
  }
  else
    props->active = false;
  return props;

}

atsGraph::~atsGraph() {
  if(!props->active) return;

  int partID=0;
  for(map<PartPtr, list<anchor> >::iterator pa=partAnchors->begin(); pa!=partAnchors->end(); pa++, partID++) {
    partInfo[pa->first] = boost::make_shared<partDotInfo_atsGraph>(partID, pa->second);

    PartPtr p = pa->first;
    SIGHT_VERB(dbg << "Adding "<<p->str()<<endl, 2, ats2DotDebugLevel)
  }

  for(map<PartPtr, list<anchor> >::iterator pa=partAnchors->begin(); pa!=partAnchors->end(); pa++) {
    // Each anchor must be associated with exactly one Part
    for(list<anchor>::iterator a=pa->second.begin(); a!=pa->second.end(); a++) {
      assert(anchor2Parts.find(*a) == anchor2Parts.end());
      //cout << "anchor "<<a->getID()<<" => "<<pa->first.get()->str()<<endl;
      anchor2Parts[*a] = pa->first;
    }
  }

  // Ensure that every Part has at least one anchor associated with it to make it possible to establish
  // edges between Part clusters (graphviz can only set up edges between nodes and edges between clusters
  // are based on edges between their members)
  for(map<PartPtr, partDotInfoPtr>::iterator pi=partInfo.begin(); pi!=partInfo.end(); pi++) {
    if(boost::dynamic_pointer_cast<partDotInfo_atsGraph>(pi->second)->anchors.size()==0) {
      boost::dynamic_pointer_cast<partDotInfo_atsGraph>(pi->second)->anchors.push_back(anchor());
      PartPtr p = pi->first;
      SIGHT_VERB(dbg << "Blank mapping to "<<p->str()<<endl, 2, ats2DotDebugLevel)
    }
  }

  genDotGraph();
}

// Generates and returns the dot graph code for this graph
void atsGraph::genDotGraph()
{
  SIGHT_VERB_DECL(scope, ("dot"), 1, ats2DotDebugLevel)

  // Maps contexts to the set of parts in each context
  Ctxt2PartsMap_atsGraph ctxt2parts(false, boost::make_shared<Ctxt2PartsMap_Generator_atsGraph>(),
                                           boost::make_shared<Ctxt2PartsMap_Leaf_Generator_atsGraph>());
  //for(fw_partEdgeIterator state(startParts); state!=fw_partEdgeIterator::end(); state++) {
  for(fw_partEdgeIterator state(startParts, /*incrementalGraph*/ false); !state.isEnd(); state++) {
    PartPtr part = state.getPart();

    list<list<PartContextPtr> > key = part->getContext()->getDetailedPartContexts();
    if(key.size()==0) {
      DummyContext d;
      key.push_back(d.getSubPartContexts());
    }
    ctxt2parts.insert(key, part);

    // Generate edges between part clusters
    if(state.getPartEdge()->source() && state.getPartEdge()->target()) {
      printPartEdge_atsGraph(partInfo, state.getPartEdge(), false, "  ");
      //list<anchors>& sAnchors = boost::dynamic_pointer_cast<partDotInfo_atsGraph>(partInfo[state.getPartEdge()->source()])->anchors;
      //list<anchors>& tAnchors = boost::dynamic_pointer_cast<partDotInfo_atsGraph>(partInfo[state.getPartEdge()->target()])->anchors;
    }
  }

  ctxt2parts.map2dot(cout, partInfo);

  // Generate edges between anchor nodes

  // Between the time when an edge was inserted into edges and now, the anchors on both sides of each
  // edge should have been located (attached to a concrete location in the output). This means that
  // some of the edges are now redundant (e.g. multiple forward edges from one location that end up
  // arriving at the same location). We thus create a new set of edges based on the original list.
  // The set's equality checks will eliminate all duplicates.

  set<graphEdge> uniqueEdges;
  map<anchor, set<anchor> > fromto;
  for(list<graphEdge>::iterator e=edges.begin(); e!=edges.end(); e++) {
    graphEdge edge = *e;
    uniqueEdges.insert(edge);
    fromto[edge.getFrom()].insert(edge.getTo());
  }

  /*cout << "#edges="<<edges.size()<<", #uniqueEdges="<<uniqueEdges.size()<<endl;* /
  cout << "anchor2Parts="<<endl;
  for(map<anchor, PartPtr>::iterator i=anchor2Parts.begin(); i!=anchor2Parts.end(); i++)
    cout << i->first.str() << " => " << i->second.get()->str()<<endl;
  for(set<graphEdge>::iterator e=uniqueEdges.begin(); e!=uniqueEdges.end(); e++)
    cout << e->getFrom().getID() << " => " << e->getTo().getID()<<endl;*/

  {
  SIGHT_VERB_DECL(scope, ("Edges"), 1, ats2DotDebugLevel)
  for(set<graphEdge>::iterator e=uniqueEdges.begin(); e!=uniqueEdges.end(); e++) {
  //for(map<anchor, set<anchor> >::iterator from=uniqueEdges.begin(); from!=uniqueEdges.end(); from++) {
  //for(set<anchor>::iterator to=from->second.begin(); to!=from->second.end(); to++) {
    graphEdge edge = *e;
    SIGHT_VERB(dbg << "edge "<<edge.getFrom().str()<<" => "<<edge.getTo().str()<<endl, 1, ats2DotDebugLevel)
    /*cout << "edge "<<edge.getFrom().str()<<"("<<(anchor2Parts.find(edge.getFrom())!=anchor2Parts.end())<<") => "<<
                     edge.getTo().str()<<"("<<(anchor2Parts.find(edge.getTo())!=anchor2Parts.end())<<")"<<endl;*/
    if(anchor2Parts.find(edge.getFrom()) != anchor2Parts.end() &&
       anchor2Parts.find(edge.getTo())   != anchor2Parts.end())
          printAnchorEdge_atsGraph(edge.getFrom(), anchor2Parts[edge.getFrom()],
                                   edge.getTo(),   anchor2Parts[edge.getTo()], "  ");

    // All the anchors associated with the from anchor's Part
/*    assert(boost::dynamic_pointer_cast<partDotInfo_atsGraph>(partInfo[anchor2Parts[edge.getFrom()]]));
    list<anchor>& fromPartAnchors = boost::dynamic_pointer_cast<partDotInfo_atsGraph>(partInfo[anchor2Parts[edge.getFrom()]])->anchors;
    for(list<anchor>::iterator a=fromPartAnchors.begin(); a!=fromPartAnchors.end(); a++) {
      if(*a != edge.getFrom()) {
        for(set<anchor>::iterator to=fromto[*a].begin(); to!=fromto[*a].end(); to++)
          o << "    a"<<edge.getFrom().getID() << " -> a"<<to->getID() << " [style=\"solid\", color=\"red\"];"<<endl;// [style=invis]" << endl;
      }
    }*/
  }
  }
}

// Add a directed edge from the location of the from anchor to the location of the to anchor
void atsGraph::addDirEdge(anchor from, anchor to)
{ edges.push_back(graphEdge(from, to, true, true)); }

// Add an undirected edge between the location of the a anchor and the location of the b anchor
void atsGraph::addUndirEdge(anchor a, anchor b)
{ edges.push_back(graphEdge(a, b, false, true)); }

}; // namespace fuse
