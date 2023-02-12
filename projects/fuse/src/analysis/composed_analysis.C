#include "sage3basic.h"
using namespace std;

#define composedAnalysisDebugLevel 1
#define moduleProfile false

#include "analysis.h"
#include "composed_analysis.h"
#include "compose.h"
#include "printAnalysisStates.h"

#ifndef DISABLE_SIGHT
#include "ats_graph_structure.h"
#endif

#include <memory>
using std::auto_ptr;

#include <utility>
using std::pair;
using std::make_pair;

#include <boost/mem_fn.hpp>
using boost::mem_fn;
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

namespace fuse
{

/****************************
 ***** ComposedAnalysis *****
 ****************************/

ComposedAnalysis::ComposedAnalysis(bool trackBase2RefinedPartEdgeMapping, bool useSSA):
      useSSA(useSSA), trackBase2RefinedPartEdgeMapping(trackBase2RefinedPartEdgeMapping) {
  startStatesInitialized = false;
  endStatesInitialized = false;

  //useSSA = getenv("SSA_ANALYSIS"); //&& !implementsATSGraph();
  //cout <<str()<<": useSSA="<<useSSA<<", implementsATSGraph()="<<implementsATSGraph()<<endl;
  /*cout << str()<<":"<<endl;
  scanf("%d", &useSSA);*/
  ssa = NULL;
  ssaLatsInitialized = false;
}

// Initializes the analysis before running it
void ComposedAnalysis::initAnalysis() {
  // Creates the SSA if needed. This is done here to make sure that the SSA is created
  // without calling any methods from this analysis, even if it is composed tightly with
  // itself
  if(useSSA)
    ssa = composer->GetSSAGraph(this);
}

// Methods used by client analyses to get a MemLoc from the composer while also documenting
// whether the MemLoc corresponds to a definition or a use of the set of memory locations
// denoted by n.
MemLocObjectPtr  ComposedAnalysis::Expr2MemLocDef(SgNode* n, PartEdgePtr pedge) {
  MemLocObjectPtr ml = composer->Expr2MemLoc(n, pedge, this);
  // If we're running on the dense ATS, return this MemLoc as is
  if(!useSSA) return ml;
  // Otherwise, if we are running on top of the SSA graph, wrap this memloc with
  else        return makePtr<SSAMemLocObject>(ml, pedge->source()? pedge->source():pedge->target(), n, SSAMemLocObject::def);
}
MemLocObjectPtr  ComposedAnalysis::Expr2MemLocUse(SgNode* n, PartEdgePtr pedge) {
  //scope s("ComposedAnalysis::OperandExpr2MemLocUse");
  MemLocObjectPtr ml = composer->Expr2MemLoc(n, pedge, this);
  //dbg << "ml="<<ml->str()<<endl;
  // If we're running on the dense ATS, return this MemLoc as is
  if(!useSSA) return ml;
  // Otherwise, if we are running on top of the SSA graph, wrap this memloc with
  else {
    /*ROSE_ASSERT(ats);
    const std::set<SSAMemLocObjectPtr>& defs = ssa->getDefs(makePtr<SSAMemLocObject>(ml, pedge->source()?pedge->source():pedge->target(), n, SSAMemLocObject::use));
    ROSE_ASSERT(defs.size()==1);
    dbg << "*defs.begin()="<<(*defs.begin())->str()<<endl;
    return *defs.begin();*/
    return makePtr<SSAMemLocObject>(ml, pedge->source()? pedge->source():pedge->target(), n, SSAMemLocObject::def);
  }
}
MemLocObjectPtr ComposedAnalysis::OperandExpr2MemLocDef(SgNode* n, SgNode* operand, PartEdgePtr pedge) {
  MemLocObjectPtr ml = composer->OperandExpr2MemLoc(n, operand, pedge, this);
  // If we're running on the dense ATS, return this MemLoc as is
  if(!useSSA) return ml;
  // Otherwise, if we are running on top of the SSA graph, wrap this memloc with
  else        return makePtr<SSAMemLocObject>(ml, pedge->source()?pedge->source():pedge->target(), operand, SSAMemLocObject::def);
}
MemLocObjectPtr ComposedAnalysis::OperandExpr2MemLocUse(SgNode* n, SgNode* operand, PartEdgePtr pedge) {
  //scope s("ComposedAnalysis::OperandExpr2MemLocUse");
  MemLocObjectPtr ml = composer->OperandExpr2MemLoc(n, operand, pedge, this);
  //dbg << "ml="<<ml->str()<<endl;
  // If we're running on the dense ATS, return this MemLoc as is
  if(!useSSA) return ml;
  // Otherwise, if we are running on top of the SSA graph, wrap this memloc with
  //else             return makePtr<SSAMemLocObject>(ml, pedge->source()?pedge->source():pedge->target(), n, SSAMemLocObject::use);
  else {
    ROSE_ASSERT(ssa);
    const std::set<SSAMemLocObjectPtr>& defs = ssa->getDefs(makePtr<SSAMemLocObject>(ml, pedge->source()?pedge->source():pedge->target(), operand, SSAMemLocObject::use));
    if(defs.size()>1) {
      for(set<SSAMemLocObjectPtr>::const_iterator d=defs.begin(); d!=defs.end(); ++d)
        cerr << "d="<<(*d)->str()<<endl;
    }
    ROSE_ASSERT(defs.size()==1);
    //if(defs.size()==1) {
      //dbg << "*defs.begin()="<<(*defs.begin())->str()<<endl;
      return *defs.begin();
    /*} else {
      return MemLocObjectPtr();
    }*/
  }
}

// Return the anchor Parts of a given function
std::set<PartPtr> ComposedAnalysis::GetStartAStates()
{
  // If the result of this function has not yet been computed
  if(!startStatesInitialized) {
    // Get the result and cache it
    StartAStates = GetStartAStates_Spec();
    startStatesInitialized = true;
  }
  return StartAStates;
}

std::set<PartPtr> ComposedAnalysis::GetEndAStates()
{
  // If the result of this function has not yet been computed
  if(!endStatesInitialized) {
    // Get the result and cache it
    EndAStates = GetEndAStates_Spec();
    endStatesInitialized = true;
  }
  return EndAStates;
}

// Given a PartPtr part implemented by this ComposedAnalysis, returns the Part from its predecessor
// from which part was derived. This function caches the results.
PartPtr ComposedAnalysis::convertPart(PartPtr part)
{
  // If the result of this function has been computed, return it
  map<PartPtr, PartPtr>::iterator oldPart=refined2BasePart.find(part);
  if(oldPart!= refined2BasePart.end()) return oldPart->second;

  // Cache the result
  PartPtr result = part->getInputPart();
  refined2BasePart[part] = result;
  return result;
}

// Given a PartEdgePtr pedge implemented by this ComposedAnalysis, returns the PartEdge from its predecessor
// from which pedge was derived. This function caches the results.
PartEdgePtr ComposedAnalysis::convertPEdge(PartEdgePtr pedge)
{
  // If the result of this function has been computed, return it
  map<PartEdgePtr, PartEdgePtr>::iterator oldPEdge=refined2BasePedge.find(pedge);
  if(oldPEdge != refined2BasePedge.end()) return oldPEdge->second;

  // Cache the result
  PartEdgePtr result = pedge->getInputPartEdge();
  refined2BasePedge[pedge] = result;
  return result;
}

// Given a Part base from the ATS on which this ComposedAnalysis runs and a Part implemented
// by this composed analysis that refines base, records the mapping from the base Part
// to the refined Part.
void ComposedAnalysis::registerBase2RefinedMapping(PartPtr base, PartPtr refined) {
  // It should only be possible to call this function if this analysis implements an ATS
  // since it should be the constructor of the Parts implemented by this analysis that call
  // this function.
  assert(implementsATSGraph());

  // Register the mapping from base to refined if this is explicitly requested
  //cout << "trackBase2RefinedPartMapping="<<trackBase2RefinedPartMapping<<", base="<<(base?base->str():"NULL")<<", refined="<<(refined?refined->str():"NULL")<<endl;
  if(trackBase2RefinedPartEdgeMapping)
    base2RefinedPart[base].insert(refined);
}

// Given a PartEdge base from the ATS on which this ComposedAnalysis runs and a PartEdge implemented
// by this composed analysis that refines base, records the mapping from the base PartEdge
// to the refined PartEdge.
void ComposedAnalysis::registerBase2RefinedMapping(PartEdgePtr base, PartEdgePtr refined) {
  // It should only be possible to call this function if this analysis implements an ATS
  // since it should be the constructor of the PartEdges implemented by this analysis that call
  // this function.
  assert(implementsATSGraph());

  // Register the mapping from base to refined if this is explicitly requested
  //cout << "trackBase2RefinedPartEdgeMapping="<<trackBase2RefinedPartEdgeMapping<<", base="<<(base?base->str():"NULL")<<", refined="<<(refined?refined->str():"NULL")<<endl;
  if(trackBase2RefinedPartEdgeMapping)
    base2RefinedPartEdge[base].insert(refined);
}

// Given a Part implemented by this analysis, returns the set of refined Parts implemented
// by this analysis or the NULLPart if this relationship was not tracked.
static set<PartPtr> emptyPartSet;
const set<PartPtr>& ComposedAnalysis::getRefinedParts(PartPtr base) const {
  std::map<PartPtr, std::set<PartPtr> >::const_iterator i = base2RefinedPart.find(base);
  if(i==base2RefinedPart.end()) return emptyPartSet;
  return i->second;
}

// Given a PartEdge implemented by this analysis, returns the set of refined PartEdges implemented
// by this analysis or the NULLPartEdge if this relationship was not tracked.
static set<PartEdgePtr> emptyEdgeSet;
const set<PartEdgePtr>& ComposedAnalysis::getRefinedPartEdges(PartEdgePtr base) const {
  std::map<PartEdgePtr, std::set<PartEdgePtr> >::const_iterator i = base2RefinedPartEdge.find(base);
  if(i==base2RefinedPartEdge.end()) return emptyEdgeSet;
  return i->second;
}

// Generates the initial lattice state for the given dataflow node, in the given function. Implementations
// fill in the lattices above and below this part, as well as the facts, as needed. Since in many cases
// the lattices above and below each node are the same, implementors can alternately implement the
// genInitLattice and genInitFact functions, which are called by the default implementation of initializeState.
//%%% add containerPart, partToRefine
void ComposedAnalysis::initializeStateDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts, NodeState& state)
{
  if(getDirection()==none) return;

  // Analyses associate all arriving information with a single NULL edge and all departing information
  // with the edge on which the information departs
  if(getDirection()==fw) {
    std::vector<Lattice*> lats;
    // Generate an initial lattice
    //%%% add containerPart, partToRefine
    genInitLattice(/*part, part->inEdgeFromAny(), supersetPart*/ parts, parts.inEdgeFromAnyAll(*this), lats);

    // Associate the Lattices in lats with this Part
    state.setLatticeAbove(this, /*supersetPart->inEdgeFromAny(), */ parts.index()->inEdgeFromAny(), lats);
  } else if(getDirection()==bw) {
    std::vector<Lattice*> lats;
    // Generate an initial lattice
    genInitLattice(/*part, part->outEdgeToAny(), supersetPart*/ parts, parts.outEdgeToAnyAll(*this), lats);

    // Associate the Lattices in lats with this Part
    state.setLatticeBelow(this, /*supersetPart->outEdgeToAny(), */ parts.index()->outEdgeToAny(), lats);
  }

  // Don't initialize the departing informaiton. This will be set by ComposedAnalysis::runAnalysis() when
  // it first touches the part
  /*vector<PartEdgePtr> edges = part->outEdges();
  for(vector<PartEdgePtr>::iterator e=edges.begin(); e!=edges.end(); e++) {
    std::vector<Lattice*> lats;
    genInitLattice(func, part, *e, lats);

    if(getDirection()==fw)      state.setLatticeBelow(this, *e, lats);
    else if(getDirection()==bw) state.setLatticeAbove(this, *e, lats);
  }*/

  vector<NodeFact*> initFacts;
  genInitFact(parts, initFacts);
  state.setFacts(this, initFacts);
}

// Generates the initial lattice state for the given dataflow node, in the given function. Implementations
// fill in the lattices above and below this part, as well as the facts, as needed. Since in many cases
// the lattices above and below each node are the same, implementors can alternately implement the
// genInitLattice and genInitFact functions, which are called by the default implementation of initializeState.
void ComposedAnalysis::initializeStateSSA(PartPtr part, NodeState& state)
{
  if(getDirection()==none) return;

  // Analyses associate all arriving information with a single NULL edge and all departing information
  // with the edge on which the information departs
  AnalysisParts NULLParts;
  AnalysisPartEdges NULLPartEdges;
  if(getDirection()==fw) {
    std::vector<Lattice*> lats;
    genInitLattice(NULLParts, NULLPartEdges, lats);
    // Associate the Lattices in lats with this Part
    state.setLatticeAbove(this, NULLPartEdge, lats);
  } else if(getDirection()==bw) {
    std::vector<Lattice*> lats;
    genInitLattice(NULLParts, NULLPartEdges, lats);
    // Associate the Lattices in lats with this Part
    state.setLatticeBelow(this, NULLPartEdge, lats);
  }

  vector<NodeFact*> initFacts;
  genInitFact(NodeState2All(part), initFacts);
  state.setFacts(this, initFacts);
}


void ComposedAnalysis::runAnalysis() {
  if(useSSA) runAnalysisSSA();
  else       runAnalysisDense();
}

void ComposedAnalysis::runAnalysisDense()
{
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ComposedAnalysis", scope::medium), 1, composedAnalysisDebugLevel)
#endif

  // Quit out if this is an undirected analysis (i.e. doesn't need the fixed-point algorithm)
  if(getDirection() == none) return;

  // Set of all the Parts that have already been visited by the analysis
  set<PartPtr> visited;

  // Set of all the Parts that have been initialized
  set<PartPtr> initialized;

  // Re-analyze it from scratch
  set<PartPtr> startingParts = getInitialWorklist();
  AnalysisPartSets ultimateParts = getUltimateParts();
  //set<PartPtr> ultimateSupersetParts = getUltimateSupersetParts();
  
#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(2, composedAnalysisDebugLevel)  
    //dbg << "#startingParts="<<startingParts.size()<<" #ultimateParts="<<ultimateParts.size()<<endl;
    for(set<PartPtr>::const_iterator i=startingParts.begin(); i!=startingParts.end(); ++i) dbg << "starting="<<i->get()->str()<<endl;
    //for(set<PartPtr>::const_iterator i=ultimateParts.begin(); i!=ultimateParts.end(); i++) dbg << "ultimate="<<i->get()->str()<<endl;
    for(AnalysisPartSets::iterator i=ultimateParts.begin(); i!=ultimateParts.end(); ++i) dbg << "ultimate="<<i->str()<<endl;
    //for(set<PartPtr>::iterator start=startingParts.begin(); start!=startingParts.end(); start++) {
    //scope reg(sight::txt()<<"Starting from "<<(*start)->str(), scope::medium));
  SIGHT_VERB_FI()
#endif

    // If we're doing the analysis over the SSA graph, initialize the global state, which we'll map to the NULLPart
  // Initialize the starting states
  for(set<PartPtr>::const_iterator s=startingParts.begin(); s!=startingParts.end(); s++) {
    // client analysis registers and initialize the state for starting parts
    //initNodeState(*s, (*s)->getSupersetPart());
    AnalysisParts parts = NodeState2All(*s);
    initNodeState(/**s, (*s)->getSupersetPart()*/ parts);
    initialized.insert(*s);
  }

  // Iterate over the abstract states that are downstream from the starting states
  dataflowPartEdgeIterator* curNodeIt = getIterator();
  //for(set<PartPtr>::iterator s=startingParts.begin(); s!=startingParts.end(); s++) curNodeIt->addStart(*s);
  set<PartPtr> empty;
  curNodeIt->init(startingParts, empty);

#ifndef DISABLE_SIGHT
  // Maps each Abstract State to the anchors associated with each visit to this part by the worklist algorithm
  boost::shared_ptr<std::map<PartPtr, std::list<sight::structure::anchor> > > partAnchors = boost::make_shared<std::map<PartPtr, std::list<sight::structure::anchor> > >();

  // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
  map<PartPtr, set<sight::structure::anchor> > toAnchors;
  // Maps each Abstract State to the anchors of the AStates that lead to it, as well as the AStates themselves
  map<PartPtr, set<pair<sight::structure::anchor, PartPtr> > > fromAnchors;
  // Maps each Abstract State to the anchor of the last instance of a transfer function executed at this AState
  map<PartPtr, sight::structure::anchor> lastTransferAnchors;
  // Maps each Abstract State to the anchor of the next instance of a transfer function executed at this AState
  map<PartPtr, sight::structure::anchor> nextTransferAnchors;
#endif

  // graph widget that visualizes the flow of the worklist algorithm
  //SIGHT_VERB(atsGraph worklistGraph((getDirection() == fw? startingParts: ultimateParts), partAnchors, getDirection() == fw), 1, composedAnalysisDebugLevel)
#ifndef DISABLE_SIGHT
  atsGraph worklistGraph((getDirection() == fw? startingParts: ultimateParts.NodeStates()), partAnchors, getDirection() == fw);
#endif

  // Most analyses eliminate dead memory locations from consideration to reduce the cost of propagating
  // information about them through code regions where they're irrelevant. However, this creates issues
  // for termination detection because the following may happen:
  // - A variable's constraints are updated
  // - Analysis enters a code region where the variable is dead (e.g. a function call)
  // - Analysis reaches a point where all the information about the currently live variables does not change
  // - Analysis terminates even though there are updates about the original variable that were not
  //     noticed because it was dead at the ATS node where we noticed that there were no more updates.
  // Since this issue only comes up when variables are dead due to scoping (they'll be read later but cannot
  //   be accessed at a given Part), we address this by explicitly tracking the functions for which the analysis
  //   has processed the call but not the return, for forward analyses, or vice versa for backwards. When
  //   the analysis converges we'll add these unmatched Parts to the worklist to see if there are still updates
  //   to be processed.
  set<PartPtr> unmatchedEntryExitParts;
  bool unmatchedEntryExitPartsEmpty=false;

  // Keep iterating while unmatchedEntryExitParts is not empty
  do {
    while(curNodeIt && !curNodeIt->isEnd())
    {
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "curNodeIt="<<curNodeIt->str()<<endl, 2, composedAnalysisDebugLevel)
#endif
      //PartPtr part = curNodeIt->grabPart();
      AnalysisParts parts = NodeState2All(curNodeIt->grabPart());
      //dbg << "parts="<<parts.str()<<endl;

      bool firstVisit = visited.find(parts.NodeState()) == visited.end();
      if(firstVisit) visited.insert(parts.NodeState());

      SgNode* sgn = parts.NodeState()->CFGNodes().begin()->getNode(); assert(sgn);
      set<CFGNode> matches;
#ifndef DISABLE_SIGHT
      SIGHT_DECL(module, (instance("Process", 1, 0),
                                   port(context("part",        parts.NodeState()->str(),
                                                "firstVisit",  firstVisit,
                                                "SgNode*",     sgn,
                                                "#desc",       (int)getDescendants(/*part->getSupersetPart()*/parts.index()).size(),
                                                "outFuncCall", parts.NodeState()->mayOutgoingFuncCall(matches),
                                                "inFuncCall",  parts.NodeState()->mayIncomingFuncCall(matches)))), moduleProfile)
#endif

      // If we're at the first side of a function call (call for fw, return for bw), add the matching side
      // to unmatchedEntryExitParts.
      if((getDirection()==fw && parts.NodeState()->mayFuncEntry(matches)) ||
         (getDirection()==bw && parts.NodeState()->mayFuncExit(matches))) {
        set<PartPtr> matchingParts = parts.NodeState()->matchingEntryExitParts();
#ifndef DISABLE_SIGHT
        SIGHT_VERB(dbg << "#matchingParts="<<matchingParts.size()<<endl, 2, composedAnalysisDebugLevel)
#endif
        for(set<PartPtr>::iterator p=matchingParts.begin(); p!=matchingParts.end(); p++) {
#ifdef DISABLE_SIGHT
          SIGHT_VERB(dbg << "Adding to unmatchedEntryExitParts: "<<(*p)->str()<<endl, 2, composedAnalysisDebugLevel)
#endif
          unmatchedEntryExitParts.insert(*p);
        }
      // If we're on the other side, remove the matching side from unmatchedEntryExitParts.
      } else if((getDirection()==fw && parts.NodeState()->mayFuncExit(matches)) ||
                (getDirection()==bw && parts.NodeState()->mayFuncEntry(matches))) {
#ifndef DISABLE_SIGHT
        SIGHT_VERB(dbg << "Erasing from unmatchedEntryExitParts: "<<parts.NodeState()->str()<<endl, 2, composedAnalysisDebugLevel)
#endif
        unmatchedEntryExitParts.erase(parts.NodeState());
      }

#ifndef DISABLE_SIGHT
      sight::structure::anchor scopeAnchor = sight::structure::anchor::noAnchor;
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      // If we have previously invoked this transfer function on this Abstract State, attach the link from it to this scope
      if(nextTransferAnchors.find(parts.NodeState()) != nextTransferAnchors.end())
        toAnchors[parts.NodeState()].insert(nextTransferAnchors[parts.NodeState()]);
      SIGHT_VERB_FI()
#endif

      // reg.attachAnchor(nextTransferAnchors[parts.NodeState()]);
#ifndef DISABLE_SIGHT
      scope* reg;
      SIGHT_VERB_DECL_REF(scope, (sight::txt()<<"Cur AState "<<parts.NodeState()->str(), toAnchors[parts.NodeState()], scope::high), reg, 1, composedAnalysisDebugLevel)

      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
        scopeAnchor = reg->getAnchor();

        if(fromAnchors.size()>0) {
          scope backedges("Incoming Edges", scope::low);
          for(set<pair<sight::structure::anchor, PartPtr> >::iterator a=fromAnchors[parts.NodeState()].begin(); a!=fromAnchors[parts.NodeState()].end(); a++)
          { a->first.linkImg(a->second.get()->str()); dbg<<endl; }
        }

        scope nextprev("", scope::minimum);
        // If we've previously visited this Abstract State, set up a link to it
        if(lastTransferAnchors.find(parts.NodeState()) != lastTransferAnchors.end())
          lastTransferAnchors[parts.NodeState()].linkImg("Last visit");
        lastTransferAnchors[parts.NodeState()] = reg->getAnchor();

        // Set up a link to the next visit, if any
        sight::structure::anchor nextVisitA;
        nextTransferAnchors[parts.NodeState()] = nextVisitA;
        nextVisitA.linkImg("Next visit");

        // We've found the destination of all the links that were pointing at this scope, so we now erase them
        toAnchors.erase(parts.NodeState());
        fromAnchors.erase(parts.NodeState());
        (*partAnchors)[parts.NodeState()].push_back(reg->getAnchor());
      SIGHT_VERB_FI()
#endif

      // Maps the edges in the flow direction of this analysis to the Lattices
      // computed by this analysis' transfer function
      map<PartEdgePtr, vector<Lattice*> > dfInfoPost;

      // Call the transfer function on the CFGNodes at this Part and set dfInfoPort to be the
      // dataflow info at the end of this Part
      transferAStateDense(/*part, part->getSupersetPart(), */ parts, visited, firstVisit, initialized, curNodeIt, dfInfoPost,
                          ultimateParts /*ultimateSupersetParts,*/
#ifndef DISABLE_SIGHT
                          ,
                          scopeAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                          );

      // Make sure that the state of all of this state's descendants is initialized
      list<PartPtr>   descendants = getDescendants(parts.NodeState());
      list<PartEdgePtr> descEdges = getEdgesToDescendants(parts.NodeState());
      list<PartPtr>::iterator d; list<PartEdgePtr>::iterator de;
      for(d = descendants.begin(), de = descEdges.begin(); de != descEdges.end(); d++, de++) {
        // The part of the current descendant
        PartEdgePtr nextPartEdges = *de;
        PartPtr nextPart = (getDirection() == fw? nextPartEdges->target(): nextPartEdges->source());
        // Initialize this descendant's state if it has not yet been
        if(initialized.find(nextPart) == initialized.end()) {
          AnalysisParts nextParts = NodeState2All(nextPart);
          initNodeState(/*nextPart, nextPart->getSupersetPart()*/ nextParts);
          initialized.insert(nextPart);
        }
      }

      // Propagate the transferred dataflow information to all of this part's descendants
      //{ struct timeval tfStart, tfEnd; gettimeofday(&tfStart, NULL);
      propagateDF2DescDense(/*part, part, */ parts, visited, initialized, curNodeIt
#ifndef DISABLE_SIGHT
                            ,
                            scopeAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                            );
      //gettimeofday(&tfEnd, NULL); cout << "propagateDF2DescDense\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<endl;
      //}

      // Set the PartEdges of all the transfered Lattices
      //{struct timeval tfStart, tfEnd; gettimeofday(&tfStart, NULL);
      setDescendantLatticeLocationsDense(/*part, part*/ parts);
      //gettimeofday(&tfEnd, NULL); cout << "setDescendantLatticeLocationsDense\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<endl;
      //}
#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "curNodeIt="<<curNodeIt->str()<<endl, 2, composedAnalysisDebugLevel)
#endif
    } // end worklist iteration

    // If unmatchedEntryExitParts is not empty, add all the parts within it to the worklist
    // and resume processing
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    scope s("unmatchedEntryExitParts");
    for(set<PartPtr>::iterator p=unmatchedEntryExitParts.begin(); p!=unmatchedEntryExitParts.end(); p++) {
      dbg << "p="<<(*p)->str()<<endl;
      dbg << "adding edge "<<(*p).get()->inEdgeFromAny()<<endl;
    }
    SIGHT_VERB_FI()
#endif

    unmatchedEntryExitPartsEmpty = unmatchedEntryExitParts.size()==0;
    for(set<PartPtr>::iterator p=unmatchedEntryExitParts.begin(); p!=unmatchedEntryExitParts.end(); p++) {
      curNodeIt->add((*p).get()->inEdgeFromAny());
    }
    unmatchedEntryExitParts.clear();
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "curNodeIt="<<curNodeIt->str()<<endl, 2, composedAnalysisDebugLevel)
#endif
  } while(!unmatchedEntryExitPartsEmpty);
}

void ComposedAnalysis::runAnalysisSSA()
{
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("ComposedAnalysis", scope::medium), 1, composedAnalysisDebugLevel)
#endif

  // Quit out if this is an undirected analysis (i.e. doesn't need the fixed-point algorithm)
  if(getDirection() == none) return;

  // Set of all the Parts that have already been visited by the analysis
  set<PartPtr> visited;

  // Set of all the Parts that have been initialized
  set<PartPtr> initialized;

  // Re-analyze it from scratch
  set<PartPtr> startingParts = getInitialWorklist();
  /*set<PartPtr>*/AnalysisPartSets ultimateParts = getUltimateParts();

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(2, composedAnalysisDebugLevel)
    //dbg << "#startingParts="<<startingParts.size()<<" #ultimateParts="<<ultimateParts.size()<<endl;
    for(set<PartPtr>::iterator i=startingParts.begin(); i!=startingParts.end(); i++) dbg << "starting="<<i->get()->str()<<endl;
    //for(set<PartPtr>::iterator i=ultimateParts.begin(); i!=ultimateParts.end(); i++) dbg << "ultimate="<<i->get()->str()<<endl;
    for(AnalysisPartSets::iterator i=ultimateParts.begin(); i!=ultimateParts.end(); i++) dbg << "ultimate="<<i->str()<<endl;
    //for(set<PartPtr>::iterator start=startingParts.begin(); start!=startingParts.end(); start++) {
    //scope reg(sight::txt()<<"Starting from "<<(*start)->str(), scope::medium));
  SIGHT_VERB_FI()
#endif

  // Initialize the global state, which we'll map to the NULLPart
  AnalysisParts NULLParts;
  initNodeState(/*NULLPart, NULLPart*/NULLParts);

  // Add the starting states to the worklist
  dataflowPartEdgeIterator* curNodeIt = getIterator();
  set<PartPtr> empty;
  curNodeIt->init(startingParts, empty);

#ifndef DISABLE_SIGHT
  // Maps each Abstract State to the anchors associated with each visit to this part by the worklist algorithm
  boost::shared_ptr<std::map<PartPtr, std::list<sight::structure::anchor> > > partAnchors = boost::make_shared<std::map<PartPtr, std::list<sight::structure::anchor> > >();

  // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
  map<PartPtr, set<sight::structure::anchor> > toAnchors;
  // Maps each Abstract State to the anchors of the AStates that lead to it, as well as the AStates themselves
  map<PartPtr, set<pair<sight::structure::anchor, PartPtr> > > fromAnchors;
  // Maps each Abstract State to the anchor of the last instance of a transfer function executed at this AState
  map<PartPtr, sight::structure::anchor> lastTransferAnchors;
  // Maps each Abstract State to the anchor of the next instance of a transfer function executed at this AState
  map<PartPtr, sight::structure::anchor> nextTransferAnchors;

  // graph widget that visualizes the flow of the worklist algorithm
  //SIGSSAGraphatsGraph worklistGraph((getDirection() == fw? startingParts: ultimateParts), partAnchors, getDirection() == fw), 1, composedAnalysisDebugLevel)
  atsGraph worklistGraph((getDirection() == fw? startingParts: ultimateParts.NodeStates()), partAnchors, getDirection() == fw);
#endif

  while(curNodeIt && !curNodeIt->isEnd())
  {
    PartPtr part = curNodeIt->grabPart();
    bool firstVisit = visited.find(part) == visited.end();
    if(firstVisit) {
      visited.insert(part);
    }

#ifndef DISABLE_SIGHT
    sight::structure::anchor scopeAnchor = sight::structure::anchor::noAnchor;
    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    // If we have previously invoked this transfer function on this Abstract State, attach the link from it to this scope
    if(nextTransferAnchors.find(part) != nextTransferAnchors.end())
      toAnchors[part].insert(nextTransferAnchors[part]);
    SIGHT_VERB_FI()

    scope* reg;
    SIGHT_VERB_DECL_REF(scope, (sight::txt()<<"Cur AState "<<part->str(), toAnchors[part], scope::high), reg, 1, composedAnalysisDebugLevel)

    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      scopeAnchor = reg->getAnchor();

      if(fromAnchors.size()>0) {
        scope backedges("Incoming Edges", scope::low);
        for(set<pair<sight::structure::anchor, PartPtr> >::iterator a=fromAnchors[part].begin(); a!=fromAnchors[part].end(); a++)
        { a->first.linkImg(a->second.get()->str()); dbg<<endl; }
      }

      scope nextprev("", scope::minimum);
      // If we've previously visited this Abstract State, set up a link to it
      if(lastTransferAnchors.find(part) != lastTransferAnchors.end())
        lastTransferAnchors[part].linkImg("Last visit");
      lastTransferAnchors[part] = reg->getAnchor();

      // Set up a link to the next visit, if any
      sight::structure::anchor nextVisitA;
      nextTransferAnchors[part] = nextVisitA;
      nextVisitA.linkImg("Next visit");

      // We've found the destination of all the links that were pointing at this scope, so we now erase them
      toAnchors.erase(part);
      fromAnchors.erase(part);
      (*partAnchors)[part].push_back(reg->getAnchor());
    SIGHT_VERB_FI()
#endif

      transferPropagateAStateSSA(part, visited, firstVisit, initialized, curNodeIt
#ifndef DISABLE_SIGHT
                                 ,
                                 scopeAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                                 );
  } // end worklist iteration
}

void ComposedAnalysis::transferAStateDense(ComposedAnalysis* analysis,
                                           /*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                                           set<PartPtr>& visited,
                                           // Set of all the Parts that have been initialized
                                           bool firstVisit,
                                           set<PartPtr>& initialized,
                                           // The dataflow iterator that identifies the state of the iteration
                                           dataflowPartEdgeIterator* curNodeIt,
                                           // Maps the edges in the flow direction of this analysis to the Lattices
                                           // computed by this analysis' transfer function
                                           map<PartEdgePtr, vector<Lattice*> >& dfInfoPost,
                                           //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                                           AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
                                           ,
                                           sight::structure::anchor curPartAnchor,
                                           // graph widget that visualizes the flow of the worklist algorithm
                                           graph& worklistGraph,
                                           // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                                           map<PartPtr, set<sight::structure::anchor> >& toAnchors,
                                           // Maps each Abstract state to the anchors of the Parts that lead to it, as well as the Parts themselves
                                           map<PartPtr, set<pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                           )
{
  //struct timeval tfStart, tfEnd; gettimeofday(&tfStart, NULL);
  // The NodeState associated with this part
  NodeState* state = NodeState::getNodeState(analysis, /*part*/ parts.NodeState());

  map<PartEdgePtr, vector<Lattice*> >& dfInfoAnte = analysis->getLatticeAnte(state);
  bool modified = false;

  /* // List of edges in the superset ATS in the direction of the analysis
  list<PartEdgePtr> descSupersetEdges = getEdgesToDescendants(supersetPart);*/

  // List of of edges of the current in the direction of analysis with which we'll associate
  // Lattices computed by the analysis' transfer function
  list<PartEdgePtr> descIndexEdges = getEdgesToDescendants(parts.index());

  // If part is among the ultimate parts, which means that it has no descendants
  //if(descSupersetEdges.size()==0 && ultimateSupersetParts.find(supersetPart)!=ultimateSupersetParts.end()) {
  if(descIndexEdges.size()==0 && ultimateParts.indexes().find(parts.index())!=ultimateParts.indexes().end()) {
    
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "<b>Adding edge beyond ultimate part</b>"<<endl, 1, composedAnalysisDebugLevel)
#endif

      // Set descEdges to contain a single wildcard edge in the direction of analysis flow so that we
    // compute analysis results on both sides of starting and ending parts. This is important to simplify
    // interactions between forward and backward analyses since forward analyses begin their execution
    // based on dataflow state immediately before the starting parts and backwards begin based on information
    // immediately after the ending parts.
    /*descSupersetEdges.push_back(getDirection() == fw ? supersetPart->outEdgeToAny():
                                                       supersetPart->inEdgeFromAny());*/
    descIndexEdges.push_back(getDirection() == fw ? parts.index()->outEdgeToAny():
                                                    parts.index()->inEdgeFromAny());
  }

  // Wildcard edge in the superset ATS that comes into supersetPar in the direction of the analysis
  //PartEdgePtr wildCardSuperPartEdge = getDirection() == fw? supersetPart->inEdgeFromAny() : supersetPart->outEdgeToAny();

  // Wildcard edge coming into the current Part in the direction of the analysis. Lattices that are used
  // as input to the transfer function are stored at this edge in the NodeState.
  PartEdgePtr wildCardIndexPartEdge = getDirection() == fw? parts.index()->inEdgeFromAny() :
                                                            parts.index()->outEdgeToAny();

  // Iterate over all the CFGNodes associated with this part and merge the result of applying to transfer function
  // to all of them
  set<CFGNode> v=parts.NodeState()->CFGNodes();
  for(set<CFGNode>::iterator c=v.begin(); c!=v.end(); c++) {
    SgNode* sgn = c->getNode();
    //struct timeval preStart, preEnd; gettimeofday(&preStart, NULL);

#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (sight::txt()<<"Current CFGNode "<<CFGNode2Str(*c), scope::medium), 1, composedAnalysisDebugLevel)
#endif

    // =================== Copy incoming lattices to outgoing lattices ===================
    // For the case where dfInfoPost needs to be created fresh, this shared pointer dfInfoPostPtr will ensure that
    // the map is deallocated when dfInfoPostPtr goes out of scope.
    boost::shared_ptr<map<PartEdgePtr, vector<Lattice*> > > dfInfoPostPtr;
    // Overwrite the Lattices below this node with the lattices above this node.
    // The transfer function will then operate on these Lattices to produce the
    // correct state below this node.

    //printf("                 dfInfoAnte.size()=%d, dfInfoPost.size()=%d, this=%p\n", dfInfoAnte.size(), dfInfoPost.size(), this);
    if(c==v.begin()) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
        {scope s("Copying incoming Lattice"); dbg <<NodeState::str(dfInfoAnte); }
        /*dbg << "  To outgoing Lattice: "<<endl;
        {indent ind; dbg <<NodeState::str(dfInfoPost); }*/
      SIGHT_VERB_FI()
#endif

      // Over-write the post information with the ante information, creating it if it doesn't exist yet
      NodeState::copyLatticesOW(dfInfoPost, dfInfoAnte);
      // GB 2012-09-28: Do we even need to keep post information around after the transfer
      //                function is done or can we just deallocate it?
      //else                     NodeState::copyLattices  (dfInfoPost, dfInfoAnte);
      // If this is not the first CFGNode, create a new post state. It will be merged into the lattices in
      // the NodeState after the transfer function
    } else {
      // Since this is not the first CFGNode within Part p, create a new post state for it
      dfInfoPostPtr = boost::make_shared<map<PartEdgePtr, vector<Lattice*> > >();
      dfInfoPost = *dfInfoPostPtr.get();

#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
        dbg << "=================================="<<endl;
        dbg << "Creating outgoing state from incoming state"<<endl;
      SIGHT_VERB_FI()
#endif

      NodeState::copyLatticesOW(dfInfoPost, dfInfoAnte);
    }
    //gettimeofday(&preEnd, NULL); cout << "transferAStateDense pre\t"<<(((preEnd.tv_sec*1000000 + preEnd.tv_usec) - (preStart.tv_sec*1000000 + preStart.tv_usec)) / 1000000.0)<<endl;

    // <<<<<<<<<<<<<<<<<<< TRANSFER FUNCTION <<<<<<<<<<<<<<<<<<<
    modified = transferCFGNodeDense(analysis, /*part, supersetPart, */ parts, *c, sgn,
                                    *state, dfInfoPost,
                                    ultimateParts, //ultimateSupersetParts,
                                    /*descSupersetEdges, wildCardSuperPartEdge*/
                                    descIndexEdges, wildCardIndexPartEdge) || modified;

    // >>>>>>>>>>>>>>>>>>> TRANSFER FUNCTION >>>>>>>>>>>>>>>>>>>

    //struct timeval postStart, postEnd; gettimeofday(&postStart, NULL);
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      {scope s("Transferred: outgoing Lattice=", scope::low); dbg <<NodeState::str(dfInfoPost)<<endl; }
      //{scope s("state=", scope::low));
      //dbg <<state->str()<<endl; }
      dbg << "Transferred: "<<(modified? "<font color=\"#990000\">Modified</font>": "<font color=\"#000000\">Not Modified</font>")<<endl;
    SIGHT_VERB_FI()
#endif

    // If this is the first CFGNode within this Part, save dfInfoPost in NodeState
    if(c==v.begin()) {
      // Save dfInfoPost in the NodeState
      analysis->setLatticePost(state, dfInfoPost, firstVisit);
    }
    // If this is not the first CFGNode within this Part, merge its outgoing lattices with the outgoing
    // lattices produced by the transfer function's execution on the prior CFGNodes in this Part
    else {
      assert(c!=v.begin());
#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
        dbg << "==================================  "<<endl;
        dbg << "Merging lattice for prior CFGNodes:"<<endl;
        {indent ind; dbg <<NodeState::str(analysis->getLatticePost(state)); }
        dbg << "With lattice  for the current CFGNodes:"<<endl;
        {indent ind; dbg <<NodeState::str(dfInfoPost); }
      SIGHT_VERB_FI()
#endif
      // Merge the transferred dfInfoPost with already existing post state information
      //PartEdgePtr wildCardPartEdge = getDirection() == fw? part->inEdgeFromAny() : part->outEdgeToAny();
      //PartEdgePtr wildCardPartEdge = getDirection() == fw? parts.index()->inEdgeFromAny() : parts.index()->outEdgeToAny();
      // Make sure information is associated with individual edges
      assert(analysis->getLatticePost(state).begin()->first != wildCardIndexPartEdge);
      modified = NodeState::unionLatticeMaps(analysis->getLatticePost(state), dfInfoPost) || modified;

#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
        dbg << "Merged within Part: Lattice"<<endl;
        {indent ind; dbg <<NodeState::str(analysis->getLatticePost(state)); }
      SIGHT_VERB_FI()
#endif
    }
    //gettimeofday(&postEnd, NULL); cout << "transferAStateDense post\t"<<(((postEnd.tv_sec*1000000 + postEnd.tv_usec) - (postStart.tv_sec*1000000 + postStart.tv_usec)) / 1000000.0)<<endl;
  } // for(vector<CFGNode>::iterator c=v.begin(); c!=v.end(); c++) {

  // Now that we've computed all the transfered Lattices for each CFGNode within part and unioned
  // them, we're sure that the Lattices at the NodeState mapped to part represent the state after
  // part. Thus, any analyses that implement the ATS definitely have their state available to them
  // and we're free to get their edges in the direction of analysis flow.

  //dbg <<"state="<<state<<"="<<state->str()<<endl;

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *   // Set the PartEdges of all the transfered Lattices
  list<PartEdgePtr> descEdges = getEdgesToDescendants(part);
  assert(descEdges.size() == descSupersetEdges.size());
  for(list<PartEdgePtr>::iterator e=descEdges.begin(), eSuper=descSupersetEdges.begin();
      eSuper!=descSupersetEdges.end(); ++e, ++eSuper) {
    for(vector<Lattice*>::iterator l=dfInfoPost[*eSuper].begin(); l!=dfInfoPost[*eSuper].end(); l++) {
      //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;e="<<(*e)->str()<<endl;
      (*l)->setPartEdge(*e);//modified = (*l)->setPartEdge(*e) || modified;
    }
  }

  // Remap the MemLocs inside each edge's Lattice to account for this PartEdge's changes in scope
  for(list<PartEdgePtr>::iterator eSuper=descSupersetEdges.begin();
      eSuper!=descSupersetEdges.end(); ++eSuper) {
    analysis->remapML((getDirection()==fw? supersetPart->inEdgeFromAny():
                                           supersetPart->outEdgeToAny()), dfInfoPost[*eSuper]);

    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      scope mpsReg("Remapped DFState", scope::low);
      for(vector<Lattice*>::iterator df=dfInfoPost[*eSuper].begin(); df!=dfInfoPost[*eSuper].end(); df++)
        dbg << (*df)->str()<<endl;
    SIGHT_VERB_FI()
  }*/
  //gettimeofday(&tfEnd, NULL); cout << "transferAStateDense\t"<<(((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0)<<endl;
}

void ComposedAnalysis::transferPropagateAStateSSA(ComposedAnalysis* analysis,
                                                  PartPtr part,
                                                  set<PartPtr>& visited,
                                                  // Set of all the Parts that have been initialized
                                                  bool firstVisit,
                                                  set<PartPtr>& initialized,
                                                  // The dataflow iterator that identifies the state of the iteration
                                                  dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
                                                  ,
                                                  sight::structure::anchor curPartAnchor,
                                                  // graph widget that visualizes the flow of the worklist algorithm
                                                  graph& worklistGraph,
                                                  // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                                                  map<PartPtr, set<sight::structure::anchor> >& toAnchors,
                                                  // Maps each Abstract state to the anchors of the Parts that lead to it, as well as the Parts themselves
                                                  map<PartPtr, set<pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                                  )
{
  // The NodeState associated with the NULLPart is the global state of the SSA analysis
  NodeState* state = NodeState::getNodeState(analysis, NULLPart);
  set<CFGNode> v=part->CFGNodes();
  ROSE_ASSERT(v.size()==1);
  CFGNode cn = *v.begin();
  SgNode* sgn = cn.getNode();

  map<PartEdgePtr, vector<Lattice*> >& dfInfo = analysis->getLatticeAnte(state);

  bool modified = false;

  // If this is a Phi Node
  if(ssa->isPhiNode(part)) {
    // Iterate over all of the phi node's uses and propagate them to its defs
    const map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >& phiDefs = ssa->getDefsUsesAtPhiNode(part);
    for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::const_iterator phiD=phiDefs.begin(); phiD!=phiDefs.end(); phiD++) {
      ROSE_ASSERT(dfInfo.find(NULLPartEdge) != dfInfo.end());
      for(vector<Lattice*>::iterator df=dfInfo[NULLPartEdge].begin(); df!=dfInfo[NULLPartEdge].end(); df++) {
        // Ask the analysis to propagate information from the defs to the use
        modified = (*df)->propagateDefs2Use((MemLocObjectPtr)phiD->first, SSAGraph::SSAMLSet2MLSet(phiD->second)) || modified;

        // Now ask then analysis to propagate information from the phi node's defs to their corresponding uses
        //modified = (*df)->propagateDef2Uses(ssa->getUsesML(phiD->first), (MemLocObjectPtr)phiD->first) || modified;
      }
    }
  }

  // /*set<PartPtr>*/AnalysisPartSets ultimateParts = NodeState2All(getUltimateParts());

  // Iterate over all the CFGNodes associated with this part and merge the result of applying to transfer function
  // to all of them

#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (sight::txt()<<"Current CFGNode "<<CFGNode2Str(cn), scope::medium), 1, composedAnalysisDebugLevel)

  SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      scope s("dfInfo Before");
      { indent ind; dbg << NodeState::str(dfInfo)<<endl; }
    SIGHT_VERB_FI()
#endif

  // <<<<<<<<<<<<<<<<<<< TRANSFER FUNCTION <<<<<<<<<<<<<<<<<<<
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("Transferring", scope::medium), 1, composedAnalysisDebugLevel);
#endif

  AnalysisParts parts = NodeState2All(part);
  boost::shared_ptr<DFTransferVisitor> transferVisitor = analysis->getTransferVisitor(/*part, part*/parts, cn, *state, dfInfo);
  sgn->accept(*transferVisitor);
  modified = transferVisitor->finish() || modified;

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    scope s("dfInfo After");
    { indent ind; dbg << NodeState::str(dfInfo)<<endl; }
  SIGHT_VERB_FI()
#endif

  // >>>>>>>>>>>>>>>>>>> TRANSFER FUNCTION >>>>>>>>>>>>>>>>>>>

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    {scope s("Transferred: outgoing Lattice=", scope::low); dbg <<NodeState::str(dfInfo)<<endl; }
    dbg << "Transferred: "<<(modified? "<font color=\"#990000\">Modified</font>": "<font color=\"#000000\">Not Modified</font>")<<endl;
  SIGHT_VERB_FI()
#endif

  // =================== Populate the generated outgoing lattice to descendants (meetUpdate) ===================
  // If this is not an ultimate node in the application (starting or ending)
  //if(ultimateParts.find(curNodeIt->getPart()) == ultimateParts.end())
  // Propagate the transferred dataflow information to all of this part's descendants
  propagateDF2DescSSA(analysis, part, modified, visited, initialized, curNodeIt, dfInfo
#ifndef DISABLE_SIGHT
                      ,
                      curPartAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                      );
}


// Propagates the dataflow info from the current node's NodeState (curNodeState) to the next node's
//     NodeState (nextNodeState).
// Returns true if the next node's meet state is modified and false otherwise.
bool ComposedAnalysis::propagateStateToNextNode(
                map<PartEdgePtr, vector<Lattice*> >& curNodeState,  AnalysisParts& curParts, //PartPtr curPart, PartPtr curSupersetPart,
                map<PartEdgePtr, vector<Lattice*> >& nextNodeState, AnalysisParts& nextParts /*PartPtr nextPart, PartPtr nextSupersetPart*/)
{
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("propagateStateToNextNode", scope::medium), 1, composedAnalysisDebugLevel)
#endif
  bool modified = false;

  // curNodeState should have a single mapping to the NULLPartEdge
  /*PartEdgePtr curPartWildCardPartEdge      = getDirection()==fw? curPart->inEdgeFromAny() :         curPart->outEdgeToAny();
  PartEdgePtr curSuperPartWildCardPartEdge = getDirection()==fw? curSupersetPart->inEdgeFromAny() : curSupersetPart->outEdgeToAny();*/
  AnalysisPartEdges curPartWildCardPartEdge = getDirection()==fw? curParts.inEdgeFromAnyAll(*this) :
                                                                  curParts.outEdgeToAnyAll (*this);

  assert(curNodeState.begin()->first == curPartWildCardPartEdge.index());

  vector<Lattice*>::const_iterator itC, itN;
#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    dbg << endl << "Propagating to Next Node: "<<nextParts.str()<<endl;
    { scope s("Cur Node Lattice"); dbg<<NodeState::str(curNodeState); }

    { scope s("Next Node Lattice"); dbg<<NodeState::str(nextNodeState); }
  SIGHT_VERB_FI()
#endif

  // Update forward info above nextPart from the forward info below curPart.

  // Compute the meet of the dataflow information along the curPart->nextPart edge with the
  // next node's current state one Lattice at a time and save the result above the next node.

  // If nextNodeState is non-empty and the number of the next node's predecessors is >1,
  // we union curNodeState into it.
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << " #nextNodeState="<<nextNodeState.size()<<", #nextPart->inEdges()="<<nextParts.NodeState()->inEdges().size()<<endl, 1, composedAnalysisDebugLevel)
#endif

    if(nextNodeState.size()>0 && nextParts.NodeState()->inEdges().size()>1) {

#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "Unioning with next state"<<endl, 1, composedAnalysisDebugLevel)
#endif

    modified = NodeState::unionLatticeMaps(nextNodeState, curNodeState) || modified;
  // Otherwise, we copy curNodeState[NULLPartEdge] over it
  } else {
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "Copying over next state"<<endl, 1, composedAnalysisDebugLevel)
#endif

    /*PartEdgePtr nextPartWildCardPartEdge      = getDirection() == fw? nextPart->inEdgeFromAny() :         nextPart->outEdgeToAny();
    PartEdgePtr nextSuperPartWildCardPartEdge = getDirection() == fw? nextSupersetPart->inEdgeFromAny() : nextSupersetPart->outEdgeToAny();*/
    AnalysisPartEdges nextPartWildCardPartEdge = getDirection() == fw? nextParts.inEdgeFromAnyAll(*this) :
                                                                       nextParts.outEdgeToAnyAll(*this);
    NodeState::copyLatticesOW(nextNodeState, /*toDepartEdge*/   nextPartWildCardPartEdge.NodeState(), nextPartWildCardPartEdge.index(),
                              curNodeState,  /*fromDepartEdge*/ curPartWildCardPartEdge.NodeState(),  curPartWildCardPartEdge.index(),
                              /*adjustPEdge*/ false);
    modified = true;
  }

  //dbg << "Result:"<<endl;
  //{ indent ind(attrGE("composedAnalysisDebugLevel", 1)); dbg<<NodeState::str(nextNodeState); }

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    indent ind;
    if(modified) {
      scope s("Next node's in-data modified. Adding...");
      dbg << "Propagated: Lattice "<<endl;
      { indent ind; dbg<<NodeState::str(nextNodeState); }
    }
    else
      dbg << "  No modification on this node"<<endl;
  SIGHT_VERB_FI()
#endif

  return modified;
}

bool ComposedAnalysis::transferCFGNodeDense(
           ComposedAnalysis* analysis,
           /*PartPtr part, PartPtr supersetPart*/
           AnalysisParts& parts, CFGNode cn, SgNode* sgn,
           NodeState& state, map<PartEdgePtr, vector<Lattice*> >& dfInfo,
           //const set<PartPtr>& ultimateParts, const set<PartPtr>& ultimateSupersetParts,
           AnalysisPartSets& ultimateParts,
           /* // List of edges in the superset ATS in the direction of the analysis
           const list<PartEdgePtr> descSupersetEdges,
           // Wildcard edge in the superset ATS that comes into supersetPar in the direction of the analysis
           PartEdgePtr wildCardSuperPartEdge*/

           // List of of edges of the current in the direction of analysis with which we'll associate
           // Lattices computed by the analysis' transfer function
           const list<PartEdgePtr>& descIndexEdges,

           // Wildcard edge coming into the current Part in the direction of the analysis. Lattices that are used
           // as input to the transfer function are stored at this edge in the NodeState.
           PartEdgePtr wildCardIndexPartEdge)
{
  //struct timeval tfStart, tfEnd; gettimeofday(&tfStart, NULL);
  //cout << std::scientific;
#ifndef DISABLE_SIGHT
  SIGHT_DECL(module, (instance("transferCFGNodeDense", 1, 0),
                                     port(context("CFGNode", CFGNode2Str(cn),
                                                  "SgNode*", sgn))), moduleProfile)
  SIGHT_VERB_DECL(scope, ("Transferring", scope::medium), 1, composedAnalysisDebugLevel);
#endif
  bool modified = false;
/*{ indent ind; dbg <<"dfInfo="<<NodeState::str(dfInfo)<<endl; }

{ scope s("dfInfo");
  dbg << "wildCardSuperPartEdge="<<wildCardSuperPartEdge->str()<<endl;
  for(map<PartEdgePtr, vector<Lattice*> >::iterator i=dfInfo.begin(); i!=dfInfo.end(); ++i) {
    dbg << i->first->str()<<endl;
    dbg << "== wildCardSuperPartEdge="<<(i->first == wildCardSuperPartEdge)<<endl;
  }
}*/

  // When a dfInfo map goes into a transfer function it must only have one key: the wildcard edge
  assert(dfInfo.size()==1);
  assert(dfInfo.find(wildCardIndexPartEdge) != dfInfo.end());

  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<
  // Invoke the transfer function
  {
#ifndef DISABLE_SIGHT
    SIGHT_DECL(module, (instance("Transfer", 1, 0),
                        port(context("CFGNode", CFGNode2Str(cn),
                                     "SgNode*", sgn))), moduleProfile)
#endif

    boost::shared_ptr<DFTransferVisitor> transferVisitor = analysis->getTransferVisitor(/*part, supersetPart*/ parts, cn, state, dfInfo);
    sgn->accept(*transferVisitor);
    modified = transferVisitor->finish() || modified;
  }
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>

  {
#ifndef DISABLE_SIGHT
    SIGHT_DECL(module, (instance("After Transfer", 1, 0),
                          port(context("CFGNode", CFGNode2Str(cn),
                                       "SgNode*", sgn))), moduleProfile)
#endif
  // The transfer function must have either left dfInfo's NULL edge key alone or created one key for each
  // descendant edge

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    dbg << "dfInfo after transfer="<<endl;
    { indent ind; dbg << NodeState::str(dfInfo)<<endl; }
  SIGHT_VERB_FI()
#endif

  // If the key is still the NULL edge
  if(dfInfo.size()==1 && (dfInfo.find(wildCardIndexPartEdge) != dfInfo.end())) {
    // Adjust dfInfo to make one copy of the value for each descendant edge

#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      dbg << "Descendant edges: #descIndexEdges="<<descIndexEdges.size()<<endl;
      /*for(list<PartEdgePtr>::iterator e=descIndexEdges.begin(); e!=descIndexEdges.end(); e++)
      { indent ind; dbg << ":" << (*e)->str() << endl; }*/
      /*dbg << "dfInfo="<<endl;
      { indent ind; dbg << NodeState::str(dfInfo) << endl; }*/
    SIGHT_VERB_FI()
#endif

    if(descIndexEdges.size() > 0) {
      //list<PartEdgePtr>::iterator first=descEdges.begin();
      list<PartEdgePtr>::const_iterator firstIndex=descIndexEdges.begin();
      // First, map the original key to the first descendant edge
      dfInfo[*firstIndex] = dfInfo[wildCardIndexPartEdge];
      dfInfo.erase(wildCardIndexPartEdge);

      // Then copy this edge's value to the other descendant edges, without yet adjusting
      // the partEdge of the Lattices. These will be set when we've registered the NodeState
      // that contains the computed Lattices with part
      {
        //list<PartEdgePtr>::iterator e=first, eIndex=firstIndex;
        //for(++e, ++eIndex; eIndex!=descIndexEdges.end(); ++eIndex, ++e) {
#ifndef DISABLE_SIGHT
        SIGHT_DECL(module, (instance("CopyLattices", 1, 0),
                                port(context("CFGNode",     CFGNode2Str(cn),
                                             "SgNode*",     sgn))), moduleProfile)
#endif

        list<PartEdgePtr>::const_iterator eIndex=firstIndex;
        for(++eIndex; eIndex!=descIndexEdges.end(); ++eIndex) {
          NodeState::copyLatticesOW(dfInfo, /*toDepartEdge*/   NULLPartEdge, *eIndex,
                                    dfInfo, /*fromDepartEdge*/ NULLPartEdge, *firstIndex,
                                    /*adjustPEdge*/ false);
        }
      }
    }
    // If the key has been changed
    } else {
//GB2015-11-09 : Commenting out since in the case of Tight Composition with analyses that implement the
//               ATS this accesses the outgoing edges before they've been computed.
//GB2015-11-09      AnalysisPartEdgeLists outEdges = parts.outEdgesAll(*this);
//GB2015-11-09
//GB2015-11-09      if(outEdges.size() != dfInfo.size()) {
//GB2015-11-09        scope s("ERROR: mismatched outEdges and dfInfo");
//GB2015-11-09        {
//GB2015-11-09          scope s("outEdges");
//GB2015-11-09          dbg << outEdges.str()<<endl;
//GB2015-11-09          /*for(list<PartEdgePtr>::const_iterator e=descIndexEdges.begin(); e!=descIndexEdges.end(); ++e)
//GB2015-11-09            dbg << (*e)->str()<<endl;*/
//GB2015-11-09        }
//GB2015-11-09        {
//GB2015-11-09          scope s("dfInfo edges");
//GB2015-11-09          for(map<PartEdgePtr, vector<Lattice*> >::const_iterator e=dfInfo.begin(); e!=dfInfo.end(); ++e)
//GB2015-11-09            dbg << e->first->str()<<endl;
//GB2015-11-09        }
//GB2015-11-09      }
//GB2015-11-09
//GB2015-11-09      // Verify that it has been changed correctly and otherwise leave it alone
//GB2015-11-09      //assert(descIndexEdges.size() == dfInfo.size());
//GB2015-11-09      assert(outEdges.size() == dfInfo.size());
//GB2015-11-09      //for(list<PartEdgePtr>::const_iterator eIndex=descIndexEdges.begin(); eIndex!=descIndexEdges.end(); ++eIndex)
//GB2015-11-09      //assert(dfInfo.find(*eIndex) != dfInfo.end());
//GB2015-11-09      for(AnalysisPartEdgeLists::iterator e=outEdges.begin(); e!=outEdges.end(); ++e)
//GB2015-11-09        assert(dfInfo.find(e->index()) != dfInfo.end());
    }
  }
  //gettimeofday(&tfEnd, NULL); double tfElapsed = ((tfEnd.tv_sec*1000000 + tfEnd.tv_usec) - (tfStart.tv_sec*1000000 + tfStart.tv_usec)) / 1000000.0;
  //cout << "transferCFGNodeDense\t"<<tfElapsed<<"\t"<<CFGNode2Str(cn)<<"\t"<<analysis->str()<<endl;

  return modified;
}

//%%% add containerPart, partToRefine
void ComposedAnalysis::propagateDF2DescDense(ComposedAnalysis* analysis,
                                             /*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                                             // Set of all the Parts that have already been visited by the analysis
                                             set<PartPtr>& visited,
                                             // Set of all the Parts that have been initialized
                                             set<PartPtr>& initialized,
                                             // The dataflow iterator that identifies the state of the iteration
                                             dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
                                             ,
                                             // anchor that denotes the current abstract state in the debug output
                                             sight::structure::anchor curPartAnchor,
                                             // graph widget that visualizes the flow of the worklist algorithm
                                             graph& worklistGraph,
                                             // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                                             map<PartPtr, set<sight::structure::anchor> >& toAnchors,
                                             // Maps each Abstract state to the anchors of the Parts that lead to it, as well as the Parts themselves
                                             map<PartPtr, set<pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                             )
{
  list<PartPtr>     descendants       = getDescendants(parts.NodeState());
  list<PartEdgePtr> descEdges         = getEdgesToDescendants(parts.NodeState());
  //list<PartEdgePtr> descSupersetEdges = getEdgesToDescendants(supersetPart);
  assert(descendants.size() == descEdges.size());
  //assert(descendants.size() == descSupersetEdges.size());
  bool modified;

  NodeState* state = NodeState::getNodeState(analysis, parts.NodeState());
  map<PartEdgePtr, vector<Lattice*> >& dfInfo = analysis->getLatticePost(state);

  SgNode* sgn = parts.NodeState()->CFGNodes().begin()->getNode(); assert(sgn);
  
#ifndef DISABLE_SIGHT
  SIGHT_DECL(module, (instance("propagateDF2DescDense", 1, 0),
                               port(context("part",        parts.NodeState()->str(),
                                            "SgNode*",     sgn,
                                            "#desc",       (int)descendants.size()))), moduleProfile)

  SIGHT_VERB_DECL(scope, (sight::txt()<<"Propagating/Merging the outgoing  Lattice to all descendant nodes("<<descEdges.size()<<")", scope::medium), 1, composedAnalysisDebugLevel)
  //SIGHT_VERB(dbg << "current State="<<state->str()<<endl, 1, composedAnalysisDebugLevel)
#endif

  // Iterate over all descendants
  list<PartPtr>::iterator d;
  list<PartEdgePtr>::iterator de;
  //list<PartEdgePtr>::iterator deSuper;

#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(2, composedAnalysisDebugLevel)
    scope s(sight::txt()<<"descendants(#"<<descendants.size()<<")");
    for(list<PartPtr>::iterator d = descendants.begin(); d!=descendants.end(); ++d)
      dbg << "    "<<(*d)->str()<<endl;
  SIGHT_VERB_FI()
#endif

//  dbg << "#descEdges="<<descEdges.size()<<endl;
  for(d = descendants.begin(), de = descEdges.begin(); de != descEdges.end(); ++d, ++de) {
    /*PartEdgePtr nextPartEdge         = *de;
    PartEdgePtr nextSupersetPartEdge = nextPartEdge->getSupersetPartEdge();
    PartPtr nextPart         = (getDirection() == fw? nextPartEdge->target():         nextPartEdge->source());
    PartPtr nextSupersetPart = (getDirection() == fw? nextSupersetPartEdge->target(): nextSupersetPartEdge->source());*/
    AnalysisPartEdges nextPartEdges = NodeState2All(*de);
    AnalysisParts nextParts = (getDirection() == fw? nextPartEdges.target(): nextPartEdges.source());

#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(2, composedAnalysisDebugLevel)
      dbg << "*d="<<(*d? (*d)->str(): "NULLPart")<<endl;
      /*dbg << "nextPartEdges="<<nextPartEdges->str()<<endl;
      dbg << "nextSupersetPartEdge="<<nextSupersetPartEdge->str()<<endl;
      dbg << "nextPart="<<(nextPart? nextPartEdges->str(): "NULLPart")<<endl;
      dbg << "nextSupersetPart="<<(nextSupersetPart? nextSupersetPartEdge->str(): "NULLPart")<<endl;*/
      dbg << "nextPartEdges="<<nextPartEdges.str()<<endl;
      dbg << "s="<<nextParts.str()<<endl;
    SIGHT_VERB_FI()
#endif

      assert(nextParts.NodeState());

#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (sight::txt()<<"Descendant: "<<nextParts.str(), scope::low), 1, composedAnalysisDebugLevel)
    SIGHT_VERB(dbg << NodeState::str(dfInfo) << endl, 1, composedAnalysisDebugLevel)
#endif
    //{scope s("dfInfo"); dbg <<NodeState::str(dfInfo)<<endl; }

    // Make sure that dfInfo has a key for this descendant
    assert(dfInfo.find(nextPartEdges.index()) != dfInfo.end());

    // Add an anchor to toAnchors from the current Abstract State to its current descendant
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    sight::structure::anchor toAnchor;
    toAnchor.linkImg(); dbg <<endl;
    worklistGraph.addDirEdge(curPartAnchor, toAnchor);
    toAnchors[nextParts.NodeState()].insert(toAnchor);
    fromAnchors[nextParts.NodeState()].insert(make_pair(curPartAnchor, parts.NodeState()));
    SIGHT_VERB_FI()
#endif

    set<CFGNode> matches;
    // If this is the point of a function call after the function's body has been processed,
    // update the state at the caller to incorporate the effects of the function.
    // If there are multiple before points for this after point, their Lattices are unioned.
    if((getDirection()==fw && parts.NodeState()->mayIncomingFuncCall(matches)) ||
       (getDirection()==bw && parts.NodeState()->mayOutgoingFuncCall(matches))) {
/*      // This should only happen on parts with a single outgoing edge. If not, we should refactor this code to do the unioning once for all edges.
      assert(descEdges.size()==1);*/
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, ("Replacing State at Matching Parts", scope::medium), 1, composedAnalysisDebugLevel)
#endif

      // The set of Parts that contain the outgoing portion of the function call for this incoming portion or
      // vice versa
      set<PartPtr> matchingParts = parts.NodeState()->matchingCallParts();

      vector<Lattice*> unionLats;

      // Compute the union of all the lattices at the matching portions of this function call
      {
#ifndef DISABLE_SIGHT
        SIGHT_VERB_DECL(scope, ("matchingParts", scope::medium), 1, composedAnalysisDebugLevel)
#endif
        //for(set<PartPtr>::iterator mp=matchingParts.begin(); mp!=matchingParts.end(); mp++)
        //dbg << mp->get()->str()<<endl; }


        assert(matchingParts.size()>0);
        for(set<PartPtr>::iterator mp=matchingParts.begin(); mp!=matchingParts.end(); mp++) {
          SIGHT_VERB_DECL(scope, (mp->get()->str(), scope::low), 1, composedAnalysisDebugLevel)
          NodeState* mpState = NodeState::getNodeState(analysis, *mp);
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "mpState="<<mpState->str()<<endl, 1, composedAnalysisDebugLevel)
#endif
          map<PartEdgePtr, vector<Lattice*> >& mpDFInfo = (getDirection()==fw? analysis->getLatticeAnte(mpState) : analysis->getLatticePost(mpState));
          for(map<PartEdgePtr, vector<Lattice*> >::iterator df=mpDFInfo.begin(); df!=mpDFInfo.end(); df++) {
            for(unsigned int i=0; i<mpDFInfo.size(); i++) {
              // If this is the first matching Part, copy its lattice. Otherwise, union
              // its lattice into the previously copied lattice.
              if(mp==matchingParts.begin() && df==mpDFInfo.begin())
                unionLats.push_back(df->second[i]->copy());
              else
                unionLats[i]->meetUpdate(df->second[i]);
            }
          }
        }
      }

#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
        scope mpsReg("unionLats", scope::low);
        for(vector<Lattice*>::iterator ul=unionLats.begin(); ul!=unionLats.end(); ul++)
          dbg << (*ul)->str()<<endl;
      SIGHT_VERB_FI()
#endif
      // Update the lattices on the matching side of the function call with the
      // information produced by the function call
      //{ scope s("replacing");
      for(unsigned int i=0; i<unionLats.size(); i++) {
        Lattice* curDF = dfInfo[nextPartEdges.index()][i];
        Lattice* dfCopy = curDF->copy();
        curDF->copy(unionLats[i]);
        curDF->replaceML(dfCopy);
        delete dfCopy;
        delete unionLats[i];
      }
      //}

#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
        scope mpsReg("Replaced DFState", scope::low);
        for(vector<Lattice*>::iterator df=dfInfo[nextPartEdges.index()].begin(); df!=dfInfo[nextPartEdges.index()].end(); df++) {
          dbg << (*df)->str()<<endl;
        }
      SIGHT_VERB_FI()
#endif

    // Else, if this is a generic Part
    } else {
      // Set the partEdge of all the lattices computed by the transfer to the current edge
      for(vector<Lattice*>::iterator lat=dfInfo[nextPartEdges.index()].begin(); lat!=dfInfo[nextPartEdges.index()].end(); ++lat) {
        (*lat)->setPartEdge(*de);
      }
    }

    // Remap the MemLocs inside each edge's Lattice to account for this PartEdge's changes in scope
    analysis->remapML(nextPartEdges.index(), dfInfo[nextPartEdges.index()]);
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
    scope mpsReg("Remapped DFState", scope::low);
      dbg << NodeState::str(dfInfo[nextPartEdges.index()])<<endl;
    SIGHT_VERB_FI()
#endif

      NodeState* nextState = NodeState::getNodeState(analysis, nextParts.NodeState());
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "next State="<<nextState->str()<<endl, 1, composedAnalysisDebugLevel)
#endif

    // The temporary dfInfo map for this descendant will have its Lattice* vector from dfInfo mapped
    // under the NULL edge key
    map<PartEdgePtr, vector<Lattice*> > dfInfoNext;
    PartEdgePtr nextWildCardIndexPartEdge = getDirection()==fw? parts.index()->inEdgeFromAny() : parts.index()->outEdgeToAny();
    dfInfoNext[nextWildCardIndexPartEdge] = dfInfo[nextPartEdges.index()];

    // Propagate the Lattices below this node to its descendant
    modified = propagateStateToNextNode(dfInfoNext,                          parts,
                                        analysis->getLatticeAnte(nextState), nextParts);
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      dbg << "Propagated/merged: "<<(modified? "<font color=\"#990000\">Modified</font>": "<font color=\"#000000\">Not Modified</font>")<<endl;
      dbg << "<hline>";

      dbg << "Final modified="<<modified<<", visited="<<(visited.find(nextParts.NodeState())!=visited.end())<<" nextParts="<<nextParts.str()<<endl;
    SIGHT_VERB_FI()
#endif
    // If the next node's state gets modified as a result of the propagation, or the next node has not yet been
    // visited, add it to the processing queue.
    if(modified || visited.find(nextParts.NodeState())==visited.end()) {
      curNodeIt->add(nextPartEdges.NodeState());
    }
    //dbg << "curNodeIt=" << curNodeIt->str() << endl;
  }
}

void ComposedAnalysis::propagateDF2DescSSA(ComposedAnalysis* analysis,
                                           PartPtr part,
                                           bool modified,
                                           // Set of all the Parts that have already been visited by the analysis
                                           set<PartPtr>& visited,
                                           // Set of all the Parts that have been initialized
                                           set<PartPtr>& initialized,
                                           // The dataflow iterator that identifies the state of the iteration
                                           dataflowPartEdgeIterator* curNodeIt,
                                           map<PartEdgePtr, vector<Lattice*> >& dfInfo
#ifndef DISABLE_SIGHT
                                           ,
                                           // anchor that denotes the current abstract state in the debug output
                                           sight::structure::anchor curPartAnchor,
                                           // graph widget that visualizes the flow of the worklist algorithm
                                           graph& worklistGraph,
                                           // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                                           map<PartPtr, set<sight::structure::anchor> >& toAnchors,
                                           // Maps each Abstract state to the anchors of the Parts that lead to it, as well as the Parts themselves
                                           map<PartPtr, set<pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                           )
{
  // --------------------
  // If this is the outgoing side of a function call propagate the information def-ed of its arguments
  // of their corresponding uses
  set<CFGNode> calls;
  if(part->mustOutgoingFuncCall(calls)) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, ("Propagating argument defs"), 1, composedAnalysisDebugLevel)
#endif

    ROSE_ASSERT(calls.size());
    SgFunctionCallExp* call = isSgFunctionCallExp(calls.begin()->getNode());

    const list<pair<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > >& funcArg2Param = ssa->getFunc2Params(part);
    ROSE_ASSERT(call->get_args()->get_expressions().size() == funcArg2Param.size());
    for(list<pair<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> > >::const_iterator a2p=funcArg2Param.begin(); a2p!=funcArg2Param.end(); ++a2p) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      dbg << "argument "<<a2p->first->str()<<endl;
      dbg << "param defs(#"<<a2p->second.size()<<"):"<<endl;
      for(set<SSAMemLocObjectPtr>::const_iterator param=a2p->second.begin(); param!=a2p->second.end(); param++)
        dbg << "    "<<(*param)->str()<<endl;
      SIGHT_VERB_FI()
#endif

      // Get all the definitions that reach the current argument use
      const set<SSAMemLocObjectPtr>& argDefs = ssa->getDefs(a2p->first);
      // Propagate info from each of these defs to each corresponding parameter of any function
      // this call may invoke
      BOOST_FOREACH(const SSAMemLocObjectPtr& def, argDefs) {
        for(vector<Lattice*>::iterator df=dfInfo[NULLPartEdge].begin(); df!=dfInfo[NULLPartEdge].end(); df++) {
          // Ask then analysis to propagate information from this argument def to its corresponding uses
          modified = (*df)->propagateDef2Uses(SSAGraph::SSAMLSet2MLSet(a2p->second), (MemLocObjectPtr)def) || modified;
        }
      }
#ifndef DISABLE_SIGHT
      for(vector<Lattice*>::iterator df=dfInfo[NULLPartEdge].begin(); df!=dfInfo[NULLPartEdge].end(); df++) {
        SIGHT_VERB(dbg << "propagated="<<(*df)->str()<<endl, 1, composedAnalysisDebugLevel)
      }
#endif
    }
  }

  if(modified) {
    // --------------------
    // Collect all the Parts that used the outputs of this part
    set<PartPtr> useParts;

    {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, ("Defs->Uses:"), 1, composedAnalysisDebugLevel)
#endif

      set<SSAMemLocObjectPtr> defs = ssa->getDefs(part);
      for(set<SSAMemLocObjectPtr>::iterator d=defs.begin(); d!=defs.end(); d++) {

#ifndef DISABLE_SIGHT
        SIGHT_VERB(dbg << "def "<<(*d)->str()<<endl, 1, composedAnalysisDebugLevel)
#endif
        set<SSAMemLocObjectPtr> uses = ssa->getUses(*d);

        for(set<SSAMemLocObjectPtr>::iterator u=uses.begin(); u!=uses.end(); u++) {

#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "    use"<<(*u)->str()<<endl, 1, composedAnalysisDebugLevel)
#endif
            useParts.insert((*u)->loc);
        }
      }
    }

    if(ssa->isPhiNode(part)) {
      {
#ifndef DISABLE_SIGHT
        SIGHT_VERB_DECL(scope, ("Phi Defs->Uses:"), 1, composedAnalysisDebugLevel)
#endif
        const map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >& phiDefs = ssa->getDefsUsesAtPhiNode(part);

        for(map<SSAMemLocObjectPtr, set<SSAMemLocObjectPtr> >::const_iterator group=phiDefs.begin(); group!=phiDefs.end(); ++group) {
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "phi def "<<(group->first)->str()<<endl, 1, composedAnalysisDebugLevel)
#endif

            set<SSAMemLocObjectPtr> uses = ssa->getUses(group->first);
          for(set<SSAMemLocObjectPtr>::iterator u=uses.begin(); u!=uses.end(); u++) {

#ifndef DISABLE_SIGHT
            SIGHT_VERB(dbg << "    use"<<(*u)->str()<<endl, 1, composedAnalysisDebugLevel)
#endif
              useParts.insert((*u)->loc);
          }
        }
      }
    }

    // --------------------
    // Add all the use parts to the worklist
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "Use Parts:"<<endl, 1, composedAnalysisDebugLevel)
#endif
    for(set<PartPtr>::iterator p=useParts.begin(); p!=useParts.end(); p++) {

#ifndef DISABLE_SIGHT
      SIGHT_VERB(dbg << "    "<<(*p)->str()<<endl, 1, composedAnalysisDebugLevel)

      // Add an anchor to toAnchors from the current Abstract State to its current descendant
      SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      sight::structure::anchor toAnchor;
      toAnchor.linkImg(); dbg <<endl;
      worklistGraph.addDirEdge(curPartAnchor, toAnchor);
      toAnchors[*p].insert(toAnchor);
      fromAnchors[*p].insert(make_pair(curPartAnchor, *p));
      SIGHT_VERB_FI()
#endif

      // !!!! Deal with function call Scope transfers

      // Add the use part to the worklist
      curNodeIt->add(p->get()->inEdgeFromAny());
    }
    //dbg << "curNodeIt="<<curNodeIt->str()<<endl;
  } // if(modified)

  // --------------------
  // Add all the unvisited control successors to the worklist
  list<PartPtr>   descendants = getDescendants(part);
  list<PartEdgePtr> descEdges = getEdgesToDescendants(part);
  list<PartPtr>::iterator d; list<PartEdgePtr>::iterator de;
  for(d = descendants.begin(), de = descEdges.begin(); de != descEdges.end(); ++d, ++de) {
    if(visited.find(*d) == visited.end())
      curNodeIt->add(*de);
  }
}

// Generic version that can be called by leaf analyses
void ComposedAnalysis::setDescendantLatticeLocationsDense(
                          ComposedAnalysis* analysis,
                          /*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("setDescendantLatticeLocationsDense"), 1, composedAnalysisDebugLevel)
#endif

  // Get the edges to part's descendants in the location ATS and the superset ATS
  list<PartEdgePtr> descEdges = getEdgesToDescendants(parts.NodeState());
  //list<PartEdgePtr> descSupersetEdges = getEdgesToDescendants(supersetPart);

  // Get the dataflow information of analysis at part
  NodeState* state = NodeState::getNodeState(analysis, parts.NodeState());
#ifndef DISABLE_SIGHT
  SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
  {
     scope s("state");
     dbg<<state->str()<<endl;
  }
  SIGHT_VERB_FI()
#endif

    map<PartEdgePtr, std::vector<Lattice*> > dfInfoPost = getLatticePost(state, analysis);

  // Set the partEdge of all the lattices on the post side of this part
  //assert(descEdges.size() == descSupersetEdges.size());
  {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (sight::txt()<<"Descendant Edges (#"<<descEdges.size()<<")"), 1, composedAnalysisDebugLevel)
#endif

      for(list<PartEdgePtr>::iterator e=descEdges.begin()/*, eSuper=descSupersetEdges.begin()*/;
          e!=descEdges.end(); ++e/*, ++eSuper*/) {
        //PartEdgePtr supersetPartEdge = (*e)->getSupersetPartEdge();
        AnalysisPartEdges edge = NodeState2All(*e);
    
#ifndef DISABLE_SIGHT
        SIGHT_VERB_DECL(scope, (sight::txt()<<"edge "<<(*e)->str()), 1, composedAnalysisDebugLevel)
#endif

          for(vector<Lattice*>::iterator l=dfInfoPost[edge.index()].begin(); l!=dfInfoPost[edge.index()].end(); l++) {
            //dbg << "&nbsp;&nbsp;&nbsp;&nbsp;e="<<(*e)->str()<<endl;
            (*l)->setPartEdge(edge.NodeState());
#ifndef DISABLE_SIGHT
            SIGHT_VERB(dbg<<"lattice "<<(*l)->str()<<endl, 1, composedAnalysisDebugLevel)
#endif
            }
      }
  }

/*  // Remap the MemLocs inside each edge's Lattice to account for this PartEdge's changes in scope
  for(list<PartEdgePtr>::iterator e=descEdges.begin(); e!=descEdges.end(); ++e) {
    PartEdgePtr supersetPartEdge = (*e)->getSupersetPartEdge();
    analysis->remapML((getDirection()==fw? supersetPart->inEdgeFromAny():
                                           supersetPart->outEdgeToAny()), dfInfoPost[supersetPartEdge]);

    SIGHT_VERB_IF(1, composedAnalysisDebugLevel)
      scope mpsReg("Remapped DFState", scope::low);
      for(vector<Lattice*>::iterator df=dfInfoPost[supersetPartEdge].begin(); df!=dfInfoPost[supersetPartEdge].end(); df++)
        dbg << (*df)->str()<<endl;
    SIGHT_VERB_FI()
  }*/
}

// --- NodeState2AllMapper ---
// ComposedAnalyses must implement methods from NodeState2AllMapper. By default, these calls are routed to
// the composer of this analysis but in the case of TightComposer, which is both a ComposedAnalysis and a
// Composer, the calls are implemented by the TightComposer itself with no additional forwarding.

// Given a Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisParts object that contains all the Parts relevant for analysis.
AnalysisParts ComposedAnalysis::NodeState2All(PartPtr part, bool NodeStates_valid, bool indexes_valid, bool inputs_valid)
{ return getComposer()->NodeState2All(part, NodeStates_valid, indexes_valid, inputs_valid); }

// Given a PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
// composer, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis.
AnalysisPartEdges ComposedAnalysis::NodeState2All(PartEdgePtr pedge, bool NodeStates_valid, bool indexes_valid, bool inputs_valid)
{ return getComposer()->NodeState2All(pedge, NodeStates_valid, indexes_valid, inputs_valid); }

// Given a Part from the ATS that the analyses managed by this composed take as input,
// returns an AnalysisParts object that contains the input and index the Parts relevant for analysis.
AnalysisParts ComposedAnalysis::Input2Index(PartPtr part, bool indexes_valid, bool inputs_valid)
{ return getComposer()->Input2Index(part, indexes_valid, inputs_valid); }

// Given a PartEdge from the ATS that the analyses managed by this composed take as input,
// returns an AnalysisPartEdges object that contains the input and index the Parts relevant for analysis.
AnalysisPartEdges ComposedAnalysis::Input2Index(PartEdgePtr pedge, bool indexes_valid, bool inputs_valid)
{ return getComposer()->Input2Index(pedge, indexes_valid, inputs_valid); }

/*************************
 ***** UndirDataflow *****
 *****  FWDataflow   *****
 *****  BWDataflow   *****
 *************************/

std::map<PartEdgePtr, std::vector<Lattice*> > UndirDataflow::emptyMap;

set<PartPtr> FWDataflow::getInitialWorklist()
{
  // Initialize the set of nodes that this dataflow will iterate over
  return getComposer()->GetStartAStates(this);
  //return AnalysisPartSets(/*nodeStateParts*/ getComposer()->GetStartAStates(this),
  //                        /*indexParts*/ AnalysisPartSets::getInputs(getComposer()->GetStartAStates(this)),
  //                        /*inputParts*/ getComposer()->GetStartAStates(this));
}

set<PartPtr> BWDataflow::getInitialWorklist()
{
  return getComposer()->GetEndAStates(this);
  //return AnalysisPartSets(/*nodeStateParts*/ getComposer()->GetEndAStates(this),
  //                        /*indexParts*/ AnalysisPartSets::getInputs(getComposer()->GetEndAStates(this)),
  //                        /*inputParts*/ getComposer()->GetEndAStates(this));
}

map<PartEdgePtr, vector<Lattice*> >& FWDataflow::getLatticeAnte(NodeState *state) { return state->getLatticeAboveAllMod(this); }
map<PartEdgePtr, vector<Lattice*> >& FWDataflow::getLatticePost(NodeState *state) { return state->getLatticeBelowAllMod(this); }
map<PartEdgePtr, vector<Lattice*> >& FWDataflow::getLatticeAnte(NodeState *state, ComposedAnalysis* analysis) { return state->getLatticeAboveAllMod(analysis); }
map<PartEdgePtr, vector<Lattice*> >& FWDataflow::getLatticePost(NodeState *state, ComposedAnalysis* analysis) { return state->getLatticeBelowAllMod(analysis); }
void FWDataflow::setLatticeAnte(NodeState *state, map<PartEdgePtr, vector<Lattice*> >& dfInfo, bool overwrite) {
  if(overwrite) state->copyLatticesOW(state->getLatticeAboveAllMod(this), dfInfo);
  else          state->copyLattices  (state->getLatticeAboveAllMod(this), dfInfo);
}
void FWDataflow::setLatticePost(NodeState *state, map<PartEdgePtr, vector<Lattice*> >& dfInfo, bool overwrite) {
  if(overwrite) state->copyLatticesOW(state->getLatticeBelowAllMod(this), dfInfo);
  else          state->copyLattices  (state->getLatticeBelowAllMod(this), dfInfo);
}
map<PartEdgePtr, vector<Lattice*> >& BWDataflow::getLatticeAnte(NodeState *state) { return state->getLatticeBelowAllMod(this); }
map<PartEdgePtr, vector<Lattice*> >& BWDataflow::getLatticePost(NodeState *state) { return state->getLatticeAboveAllMod(this); }
map<PartEdgePtr, vector<Lattice*> >& BWDataflow::getLatticeAnte(NodeState *state, ComposedAnalysis* analysis) { return state->getLatticeBelowAllMod(analysis); }
map<PartEdgePtr, vector<Lattice*> >& BWDataflow::getLatticePost(NodeState *state, ComposedAnalysis* analysis) { return state->getLatticeAboveAllMod(analysis); }
void BWDataflow::setLatticeAnte(NodeState *state, map<PartEdgePtr, vector<Lattice*> >& dfInfo, bool overwrite) {
  if(overwrite) state->copyLatticesOW(state->getLatticeBelowAllMod(this), dfInfo);
  else          state->copyLattices  (state->getLatticeBelowAllMod(this), dfInfo);
}
void BWDataflow::setLatticePost(NodeState *state, map<PartEdgePtr, vector<Lattice*> >& dfInfo, bool overwrite) {
  if(overwrite) state->copyLatticesOW(state->getLatticeAboveAllMod(this), dfInfo);
  else          state->copyLattices  (state->getLatticeAboveAllMod(this), dfInfo);
}

list<PartPtr> FWDataflow::getDescendants(PartPtr part)
{
  list<PartPtr> descendants;
  list<PartEdgePtr> outEdges = part->outEdges();
  for(list<PartEdgePtr>::iterator ei=outEdges.begin(); ei!=outEdges.end(); ei++)
    descendants.push_back((*ei)->target());
  return descendants;
}

list<PartPtr> BWDataflow::getDescendants(PartPtr part)
{
  list<PartPtr> descendants;
  list<PartEdgePtr> inEdges = part->inEdges();
  for(list<PartEdgePtr>::iterator ei=inEdges.begin(); ei!=inEdges.end(); ei++)
    descendants.push_back((*ei)->source());
  return descendants;
}

list<PartEdgePtr> FWDataflow::getEdgesToDescendants(PartPtr part)
{
  return part->outEdges();
}

list<PartEdgePtr> BWDataflow::getEdgesToDescendants(PartPtr part)
{
 return part->inEdges();
}

// Returns the set of Parts that denote the end of the ATS.
// getUltimateParts() returns the Parts from the ATS over which the analysis is being run.
// If the analysis is being composed loosely, this ATS is already complete when the analysis starts.
// If it is a tight composition, the ATS is created as the analysis runs.
AnalysisPartSets FWDataflow::getUltimateParts() {
{ return ComposedAnalysis::NodeState2AllMapper::NodeState2All(getComposer()->GetEndAStates(this)); }
//  return AnalysisPartSets(/*nodeState*/ getComposer()->GetEndAStates(this),
//                          /*index*/     AnalysisPartSets::getInputs(getComposer()->GetEndAStates(this)),
//                          /*input*/     getComposer()->GetEndAStates(this));
}

// getUltimateSupersetParts() returns the Parts from the completed ATS that the current analysis
// may be refining. getUltimateParts() == getUltimateSupersetParts() for loose composition but not
// for tight composition.
/*std::set<PartPtr> FWDataflow::getUltimateSupersetParts()
//{ return getUltimateParts(); }
{
  set<PartPtr> baseParts = getComposer()->GetEndAStates(this);
  set<PartPtr> supersetParts;
  for(set<PartPtr>::iterator bp=baseParts.begin(); bp!=baseParts.end(); ++bp) {
    if(*bp) supersetParts.insert((*bp)->getSupersetPart());
    else    supersetParts.insert(NULLPart);
  }
  return supersetParts;
}*/

// Returns the set of Parts that denote the end of the ATS.
// getUltimateParts() returns the Parts from the ATS over which the analysis is being run.
// If the analysis is being composed loosely, this ATS is already complete when the analysis starts.
// If it is a tight composition, the ATS is created as the analysis runs.
AnalysisPartSets BWDataflow::getUltimateParts() {
{ return ComposedAnalysis::NodeState2AllMapper::NodeState2All(getComposer()->GetStartAStates(this)); }
//  return AnalysisPartSets(/*nodeState*/ getComposer()->GetStartAStates(this),
//                          /*index*/     AnalysisPartSets::getInputs(getComposer()->GetStartAStates(this)),
//                          /*input*/     getComposer()->GetStartAStates(this));
}

/* // getUltimateSupersetParts() returns the Parts from the completed ATS that the current analysis
// may be refining. getUltimateParts() == getUltimateSupersetParts() for loose composition but not
// for tight composition.
std::set<PartPtr> BWDataflow::getUltimateSupersetParts()
//{ return getUltimateParts(); }
{
  set<PartPtr> baseParts = getComposer()->GetStartAStates(this);
  set<PartPtr> supersetParts;
  for(set<PartPtr>::iterator bp=baseParts.begin(); bp!=baseParts.end(); ++bp) {
    if(*bp) supersetParts.insert((*bp)->getSupersetPart());
    else    supersetParts.insert(NULLPart);
  }
  return supersetParts;
}*/


dataflowPartEdgeIterator* FWDataflow::getIterator()
{ return new fw_dataflowPartEdgeIterator(/*incrementalGraph*/ false, selectIterOrderFromEnvironment()); }
dataflowPartEdgeIterator* BWDataflow::getIterator()
{ return new bw_dataflowPartEdgeIterator(/*incrementalGraph*/ false, selectIterOrderFromEnvironment()); }


/*// Given a Part from the ATS on which NodeStates are maintained for this analysis,  returns an AnalysisParts object that contains all the Parts relevant for analysis
AnalysisParts FWDataflow::NodeState2All(PartPtr part) { return AnalysisParts(part, part->getInputPart(), part); }
AnalysisParts BWDataflow::NodeState2All(PartPtr part) { return AnalysisParts(part, part->getInputPart(), part); }

// Given a PartEdge from the ATS on which NodeStates are maintained for this analysis, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis
AnalysisPartEdges FWDataflow::NodeState2All(PartEdgePtr pedge) { return AnalysisPartEdges(pedge, pedge->getInputPartEdge(), pedge); }
AnalysisPartEdges BWDataflow::NodeState2All(PartEdgePtr pedge) { return AnalysisPartEdges(pedge, pedge->getInputPartEdge(), pedge); }*/

// Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
// with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
void FWDataflow::remapML(PartEdgePtr fromPEdge, vector<Lattice*>& lat) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("FWDataflow::remapML", scope::high), 1, composedAnalysisDebugLevel)
#endif
  for(unsigned int i=0; i<lat.size(); i++) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "lat["<<i<<"]="<<(lat[i]? lat[i]->str(): "NULL")<<endl, 1, composedAnalysisDebugLevel)
#endif

      /*cout << "FWDataflow::remapML() lat[i]->getPartEdge()="<<lat[i]->getPartEdge()->str()<<endl;
cout << "FWDataflow::remapML() fromPEdge="<<fromPEdge->str()<<endl;*/
    Lattice* newL = lat[i]->getPartEdge()->forwardRemapML(lat[i], fromPEdge, this);

#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "newL="<<(newL? newL->str(): "NULL")<<endl, 1, composedAnalysisDebugLevel)
#endif
    // If any remapping was done, update lat
    if(newL) {
      lat[i]->copy(newL);
      delete newL;
    }
  }
}

// Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
// with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
void BWDataflow::remapML(PartEdgePtr fromPEdge, vector<Lattice*>& lat) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, ("BWDataflow::remapML", scope::high), 1, composedAnalysisDebugLevel)
#endif

    for(unsigned int i=0; i<lat.size(); i++) {
    //dbg << "lat["<<i<<"]->getPartEdge()="<<lat[i]->getPartEdge()->str()<<endl;
    Lattice* newL = lat[i]->getPartEdge()->backwardRemapML(lat[i], fromPEdge, this);
    // If any remapping was done, update lat
    if(newL) {
      lat[i]->copy(newL);
      delete newL;
    }
  }
}

//! Register each dataflow analysis for the given part with NodeState map
//! Each leaf dataflow analysis can initialize the NodeState calling initializeState
//! part - the Part of the ATS on top of which this analysis is running.
//! supersetPart - analyses that implement the ATS will compute a Part of their own to refine the Part .
//!                currently being analyzed. supersetPart denotes the Part that this new Part should be refining.
//!                If this the analysis is being composed loosely, part==supersetPart. However, if it is being
//!                composed tightly, the ATS that part belongs to is not fully constructed and thus, it cannot be
//!                used as a supersetPart until after the analysis has finished processing and its ATS is completed.
void FWDataflow::initNodeState(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts) {
#ifndef DISABLE_SIGHT
  SIGHT_VERB_DECL(scope, (sight::txt()<<"FWDataflow::initNodeState()"), 2, composedAnalysisDebugLevel)
  SIGHT_VERB_IF(2, composedAnalysisDebugLevel)
  /*dbg << "part="<<part->str()<<endl;
  dbg << "supersetPart="<<supersetPart->str()<<endl;*/
  //dbg << "parts="<<parts.str()<<endl;
  SIGHT_VERB_FI()
#endif
  // registers if not already registered
  NodeState* state = NodeState::getNodeState(this, /*part*/parts.NodeState());
  // fill the state with Lattices
  if(useSSA) initializeStateSSA(/*supersetPart,*/parts.input(), *state);
  else       initializeStateDense(/*part, supersetPart*/parts, *state);

#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "analysis=" << this->str() << ", state=" << state->str(this), 2, composedAnalysisDebugLevel)
#endif
}

//! Register each dataflow analysis for the given part with NodeState map
//! Each leaf dataflow analysis can initialize the NodeState calling initializeState
//! part - the Part of the ATS on top of which this analysis is running.
//! supersetPart - analyses that implement the ATS will compute a Part of their own to refine the Part .
//!                currently being analyzed. supersetPart denotes the Part that this new Part should be refining.
//!                If this the analysis is being composed loosely, part==supersetPart. However, if it is being
//!                composed tightly, the ATS that part belongs to is not fully constructed and thus, it cannot be
//!                used as a supersetPart until after the analysis has finished processing and its ATS is completed.
void BWDataflow::initNodeState(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts) {
  // registers if not already registered
  NodeState* state = NodeState::getNodeState(this, /*part*/ parts.NodeState());
  // fill the state with Lattices
  if(useSSA) initializeStateSSA(/*supersetPart, */parts.input(), *state);
  else       initializeStateDense(/*part, supersetPart, */parts, *state);

#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "analysis=" << this->str() << ", state=" << state->str(this), 2, composedAnalysisDebugLevel)
#endif
}

void FWDataflow::propagateDF2DescDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                                       // Set of all the Parts that have already been visited by the analysis
                                       std::set<PartPtr>& visited,
                                       // Set of all the Parts that have been initialized
                                       std::set<PartPtr>& initialized,
                                       // The dataflow iterator that identifies the state of the iteration
                                       dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
                                       ,                                       
                                       // anchor that denotes the current abstract state in the debug output
                                       sight::structure::anchor curPartAnchor,
                                       // graph widget that visualizes the flow of the worklist algorithm
                                       graph& worklistGraph,
                                       // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                                       std::map<PartPtr, std::set<sight::structure::anchor> >& toAnchors,
                                       // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                                       std::map<PartPtr, std::set<std::pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                       ) {
  ComposedAnalysis::propagateDF2DescDense(this, /*part, supersetPart,*/parts,
                                          visited, initialized, curNodeIt
#ifndef DISABLE_SIGHT
                                          ,
                                          curPartAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                                          );
}

void BWDataflow::propagateDF2DescDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                                       // Set of all the Parts that have already been visited by the analysis
                                       std::set<PartPtr>& visited,
                                       // Set of all the Parts that have been initialized
                                       std::set<PartPtr>& initialized,
                                       // The dataflow iterator that identifies the state of the iteration
                                       dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
                                       ,
                                       // anchor that denotes the current abstract state in the debug output
                                       sight::structure::anchor curPartAnchor,
                                       // graph widget that visualizes the flow of the worklist algorithm
                                       graph& worklistGraph,
                                       // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                                       std::map<PartPtr, std::set<sight::structure::anchor> >& toAnchors,
                                       // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                                       std::map<PartPtr, std::set<std::pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                       ) {
  ComposedAnalysis::propagateDF2DescDense(this, /*part, supersetPart,*/ parts,
                                          visited, initialized, curNodeIt
#ifndef DISABLE_SIGHT
                                          ,
                                          curPartAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                                          );
}

void FWDataflow::transferAStateDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                                     std::set<PartPtr>& visited, bool firstVisit,
                                     std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt,
                                     map<PartEdgePtr, vector<Lattice*> >& dfInfoPost,
                                     //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                                     AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
                                     ,
                                     sight::structure::anchor curPartAnchor,
                                     graph& worklistGraph,std::map<PartPtr, std::set<sight::structure::anchor> >& toAnchors,
                                     std::map<PartPtr, std::set<std::pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                     ) {
  ComposedAnalysis::transferAStateDense(this, /*part, supersetPart, */ parts,
                                        visited, firstVisit,
                                        initialized, curNodeIt, dfInfoPost,
                                        ultimateParts
#ifndef DISABLE_SIGHT
                                        , 
                                        curPartAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                                        );
}

void FWDataflow::transferPropagateAStateSSA(PartPtr part, set<PartPtr>& visited, bool firstVisit, set<PartPtr>& initialized,
                                            dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
                                            ,                                            
                                            sight::structure::anchor curPartAnchor, graph& worklistGraph,
                                            map<PartPtr, set<sight::structure::anchor> >& toAnchors,
                                            map<PartPtr, set<pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                            ) {
  ComposedAnalysis::transferPropagateAStateSSA(this, part, visited, firstVisit, initialized, curNodeIt
#ifndef DISABLE_SIGHT
                                               ,
                                               curPartAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                                               );
}

void BWDataflow::transferAStateDense(/*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts,
                                     std::set<PartPtr>& visited, bool firstVisit,
                                     std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt,
                                     map<PartEdgePtr, vector<Lattice*> >& dfInfoPost,
                                     //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                                     AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
                                     ,
                                     sight::structure::anchor curPartAnchor,
                                     graph& worklistGraph,std::map<PartPtr, std::set<sight::structure::anchor> >& toAnchors,
                                     std::map<PartPtr, std::set<std::pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                     ) {
  ComposedAnalysis::transferAStateDense(this, /*part, supersetPart, */parts,
                                        visited, firstVisit,
                                        initialized, curNodeIt, dfInfoPost,
                                        ultimateParts
#ifndef DISABLE_SIGHT
                                        , //ultimateSupersetParts,
                                        curPartAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                                        );
}

void BWDataflow::transferPropagateAStateSSA(PartPtr part, set<PartPtr>& visited, bool firstVisit, set<PartPtr>& initialized,
                                            dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
                                            ,
                                            sight::structure::anchor curPartAnchor, graph& worklistGraph,
                                            map<PartPtr, set<sight::structure::anchor> >& toAnchors,
                                            map<PartPtr, set<pair<sight::structure::anchor, PartPtr> > >& fromAnchors
#endif
                                            ) {
  ComposedAnalysis::transferPropagateAStateSSA(this, part, visited, firstVisit, initialized, curNodeIt
#ifndef DISABLE_SIGHT
                                               ,
                                               curPartAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                                               );
}

// Invokes the analysis-specific method to set the ATS location PartEdges of all the newly-computed
// Lattices at part
void FWDataflow::setDescendantLatticeLocationsDense(
                                                    /*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts) {
  ComposedAnalysis::setDescendantLatticeLocationsDense(this, /*part, supersetPart*/ parts);
}

// Invokes the analysis-specific method to set the ATS location PartEdges of all the newly-computed
// Lattices at part
void BWDataflow::setDescendantLatticeLocationsDense(
                                                    /*PartPtr part, PartPtr supersetPart*/ AnalysisParts& parts) {
  ComposedAnalysis::setDescendantLatticeLocationsDense(this, /*part, supersetPart*/ parts);
}

/******************************************************
 ***      printDataflowInfoPass                     ***
 *** Prints out the dataflow information associated ***
 *** with a given analysis for every CFG node a     ***
 *** function.                                      ***
 ******************************************************/

// Initializes the state of analysis lattices at the given function, part and edge into our out of the part
// by setting initLattices to refer to freshly-allocated Lattice objects.
void printDataflowInfoPass::genInitLattice(/*PartPtr part, PartEdgePtr pedge, PartPtr supersetPart,*/
                                           const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                                           std::vector<Lattice*>& initLattices)
{
  initLattices.push_back((Lattice*)(new BoolAndLattice(0, pedges.NodeState())));
}

bool printDataflowInfoPass::transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                                     std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)
{
#ifndef DISABLE_SIGHT
  dbg << "-----#############################--------\n";
  dbg << "Node: ["<<parts.str()<<"\n";
  dbg << "State:\n";
  SIGHT_VERB(indent ind, 1, composedAnalysisDebugLevel);
  dbg << state.str(analysis)<<endl;
#endif

  return dynamic_cast<BoolAndLattice*>(dfInfo[/*supersetPart*/parts.index()->inEdgeFromAny()][0])->set(true);
}

/***************************************************
 ***            checkDataflowInfoPass            ***
 *** Checks the results of the composed analysis ***
 *** chain at special assert calls.              ***
 ***************************************************/

// Initializes the state of analysis lattices at the given function, part and edge into our out of the part
// by setting initLattices to refer to freshly-allocated Lattice objects.
void checkDataflowInfoPass::genInitLattice(/*PartPtr part, PartEdgePtr pedge, PartPtr supersetPart,*/
                                           const AnalysisParts& parts, const AnalysisPartEdges& pedges,
                                           std::vector<Lattice*>& initLattices)
{
#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << "<<<checkDataflowInfoPass::genInitLattice"<<endl, 2, composedAnalysisDebugLevel)
#endif
    initLattices.push_back((Lattice*)(new BoolAndLattice(0, pedges.NodeState())));

#ifndef DISABLE_SIGHT
  SIGHT_VERB(dbg << ">>>checkDataflowInfoPass::genInitLattice"<<endl, 2, composedAnalysisDebugLevel)
#endif

    }

bool checkDataflowInfoPass::transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                                     std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo)
{
  set<CFGNode> nodes = parts.NodeState()->CFGNodes();
  for(set<CFGNode>::iterator n=nodes.begin(); n!=nodes.end(); n++) {
    SgFunctionCallExp* call;
    if((call = isSgFunctionCallExp(n->getNode())) && n->getIndex()==2) {
      Function func(call);
      if(func.get_name().getString().find("CompDebugAssert")==0) {
        SgExpressionPtrList args = call->get_args()->get_expressions();
        for(SgExpressionPtrList::iterator a=args.begin(); a!=args.end(); a++) {
          ValueObjectPtr v = getComposer()->OperandExpr2Val(call, *a, parts.NodeState()->inEdgeFromAny(), this);
          assert(v);

#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "v="<<v->str()<<", v->isConcrete()="<<v->isConcrete()<<", a="<<SgNode2Str(*a)<<endl, 1, composedAnalysisDebugLevel)
#endif
            ostringstream errorMesg;
          ostringstream succMesg;
          if(!v->isConcrete()) {
            errorMesg << "Debug assertion at "<<call->get_file_info()->get_filenameString()<<":"
                      <<call->get_file_info()->get_line()
                      <<" failed: concrete interpretation not available! test="
                      <<(*a)->unparseToString()<<" v="<<v->str();
          }
          else {
            set<boost::shared_ptr<SgValueExp> > concreteVals = v->getConcreteValue();
            if(concreteVals.size()==0) {
              errorMesg << "Debug assertion at "<<call->get_file_info()->get_filenameString()
                        <<":"<<call->get_file_info()->get_line()
                        <<" failed: interpretation not convertible to a boolean! test="
                        <<(*a)->unparseToString()<<" v="<<v->str()<<" v->getConcreteValue() is empty!";
            }

            for(set<boost::shared_ptr<SgValueExp> >::iterator cv=concreteVals.begin(); cv!=concreteVals.end(); cv++) {
              if(!ValueObject::isValueBoolCompatible(*cv)) {
                errorMesg << "Debug assertion at "<<call->get_file_info()->get_filenameString()<<":"
                          <<call->get_file_info()->get_line()
                          <<" failed: interpretation not convertible to a boolean! test="<<(*a)->unparseToString()
                          <<" v="<<v->str()<<" v->getConcreteValue()="<<SgNode2Str(cv->get());
              }
              else if(!ValueObject::SgValue2Bool(*cv)) {
                errorMesg << "Debug assertion at "<<call->get_file_info()->get_filenameString()<<":"
                          <<call->get_file_info()->get_line()<<" failed: test evaluates to false! test="<<(*a)->unparseToString()
                          <<" v="<<v->str()<<" v->getConcreteValue()="<<SgNode2Str(cv->get());
              }
              if (ValueObject::SgValue2Bool(*cv)) {
                succMesg << "Debug assertion at "<<call->get_file_info()->get_filenameString()<<":"
                         <<call->get_file_info()->get_line()<<"passed: test evalutes to true! test="<<(*a)->unparseToString()
                         <<" v="<<v->str()<<" v->getConcreteValue()="<<SgNode2Str(cv->get());
              }
            }
          }

          if(errorMesg.str() != "") {
            cerr << errorMesg.str() << endl;
            // dbg << "<span style=\"color:red;font-size:14pt\">"<<errorMesg.str()<<"</span>"<<endl;
            numErrors++;
          }
          if(succMesg.str() != "") {
            cout << succMesg.str() << endl;
          }
        }
      } else if(func.get_name().getString().find("CompShow")==0) {
        SgExpressionPtrList args = call->get_args()->get_expressions();
        for(SgExpressionPtrList::iterator a=args.begin(); a!=args.end(); a++) {
          ValueObjectPtr v = getComposer()->OperandExpr2Val(call, *a, parts.NodeState()->inEdgeFromAny(), this);
          assert(v);
          Sg_File_Info *info = call->get_startOfConstruct();
          cout << info->get_filenameString()<<":"<<info->get_line()<<" "<<func.get_name().getString()<<"("<<(*a)->unparseToString()<<"="<<v->str()<<")"<<endl;
        }
      }
    }
  }

  PartEdgePtr accessEdge;
  if(useSSA) accessEdge = NULLPartEdge;
  else       accessEdge = parts.index()->inEdgeFromAny();
  return dynamic_cast<BoolAndLattice*>(dfInfo[accessEdge][0])->set(true);
}

}; // namespace fuse
