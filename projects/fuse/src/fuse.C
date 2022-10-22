#include "sage3basic.h"
#include "compose.h"
#include "tight_composer.h"
#include "const_prop_analysis.h"
#include "live_dead_analysis.h"
#include "call_context_sensitivity_analysis.h"
#include "dead_path_elim_analysis.h"
#include "printAnalysisStates.h"
#include "pointsToAnalysis.h"
#include "virtualMethodAnalysis.h"
#include "dynamicMonitor.h"
//#include "constantAnalysis.h"
//#include "valueNumbering.h"
#include "defsAnalysis.h"
#include "analysis_tester.h"
#include <vector>
#include <set>
#include <ctype.h>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include "sageBuilder.h"
#include "sageInterface.h"
#include "sight.h"
using namespace sight;
#include <sys/time.h>
#include "fuse.h"

using namespace std;
using namespace boost::xpressive;
using namespace SageBuilder;
using namespace SageInterface;
using namespace SPRAY;
//using namespace scc_private;

#define apiDebugLevel 0

namespace fuse {

/****************
 ***** Fuse *****
 ****************/

// Regex expressions for the composition command, defined globally so that they can be used inside main
// (where they're initialized) as well as inside output_nested_results()
sregex
  Fuse::composer, Fuse::lcComposer, Fuse::lpComposer, Fuse::tComposer,
  Fuse::analysis, Fuse::noAnalysis, Fuse::cpAnalysis, Fuse::ldAnalysis, Fuse::ccsAnalysis, Fuse::dpAnalysis, Fuse::ptAnalysis,
                  Fuse::vmAnalysis, Fuse::spcpAnalysis, Fuse::spvnAnalysis,
                  Fuse::ssacpAnalysis, Fuse::ssaldAnalysis, Fuse::ssaccsAnalysis, Fuse::ssadpAnalysis, Fuse::ssaptAnalysis,
                  Fuse::ssavmAnalysis, Fuse::ssaspcpAnalysis, Fuse::ssaspvnAnalysis,
                  Fuse::dynMonitor,
  Fuse::analysisList, Fuse::compSpec;

// Records whether we've executed the basic Fuse initialization that is common to
// all instances of Fuse objects
bool Fuse::fuseGloballyInitialized=false;

// Maps the IDs of variables about which we'll report analysis results
VariableIdMapping *Fuse::vIDMap;

// The labeler that will be used to place AST annotations that capture the
// output of the Fuse analysis
FuseLabeler* Fuse::labeler;

// The annotator that will place labels at AST nodes
AstAnnotator* Fuse::annotator;

Fuse::Fuse(int argc, char **argv) {
  rootComposer = NULL;
  cdip = NULL;

  if(fuseGloballyInitialized) return;
  //#SA 8/18/14
  // Command to set up the enviroment variable to find the binary fuseLayout
  // fuseLayout is required to run fuse
  // fuseLayout binary is at the same level as fuse in the build tree
  // When compiling fuse ROSE_PREFIX is defined as -DROSE_PREFIX="\"${top_builddir}\"" which
  // is top of the build tree
  // If fuse fails to find fuseLayout set up this environment variable appropriately.
  setenv("SIGHT_LAYOUT_EXEC", (txt()<<ROSE_PREFIX<<"/projects/fuse/src/fuseLayout").c_str(), 1);
  //setenv("SIGHT_LAYOUT_EXEC", (txt()<<ROSE_PREFIX<<"/bin//fuseLayout").c_str(), 1);
  SightInit(argc, argv);
  modularApp::setNamedMeasures(namedMeasures("time", new timeMeasure()));

  initRegex();

  // Initialize the data structures that manage labeling the AST with the results
  // of Fuse analyses
  vIDMap = new VariableIdMapping();
  vIDMap->computeVariableSymbolMapping(getProject());
  //labeler = new Labeler(getProject());
  labeler = NULL;
  annotator = NULL;

  fuseGloballyInitialized = true;
}

// Returns the map the IDs of variables about which we'll report analysis results
VariableIdMapping * Fuse::getVIDMap() {
  assert(fuseGloballyInitialized);
  assert(vIDMap);
  return vIDMap;
}

// Returns the labeler
// FuseLabeler* Fuse::getLabeler() {
//   assert(fuseGloballyInitialized);
//   assert(labeler);
//   return labeler;
// }

void Fuse::initRegex() {
  static bool regexInitialized=false;

  if(regexInitialized) return;

  lcComposer = as_xpr(icase("loosechain")) | icase("lc");
  lpComposer = as_xpr(icase("loosepar"))   | icase("lp");
  tComposer  = as_xpr(icase("tight"))      | icase("t");
  composer = by_ref(lcComposer) | by_ref(lpComposer) | by_ref(tComposer);
  //composer = as_xpr(icase("loosechain")) | icase("lc") | icase("loosepar") | icase("lp");

  noAnalysis   = as_xpr(icase("noanalysis"))                        | icase("none");
  cpAnalysis   = as_xpr(icase("constantpropagationanalysis"))       | icase("constprop")   | icase("cp");
  ldAnalysis   = as_xpr(icase("livedeadmemanalysis"))               | icase("livedead")    | icase("ld");
  ccsAnalysis  = as_xpr(icase("callctxsensanalysis"))               | icase("callctxsens") | icase("ccs");
  dpAnalysis   = as_xpr(icase("deadpathelimanalysis"))              | icase("deadpath")    | icase("dp");
  ptAnalysis   = as_xpr(icase("pointstoanalysis"))                  | icase("pointsto")    | icase("pt");
  vmAnalysis   = as_xpr(icase("virtualmethodanalysis"))             | icase("virtualmem")  | icase("vm");
  spcpAnalysis = as_xpr(icase("sparseconstantpropagationanalysis")) | icase("spconstprop") | icase("spcp");
  spvnAnalysis = as_xpr(icase("sparsevaluenumberinganalysis"))      | icase("spvalnum")    | icase("spvn");
  ssacpAnalysis   = as_xpr(icase("SSA:constantpropagationanalysis"))       | icase("SSA:constprop")   | icase("SSA:cp");
  ssaldAnalysis   = as_xpr(icase("SSA:livedeadmemanalysis"))               | icase("SSA:livedead")    | icase("SSA:ld");
  ssaccsAnalysis  = as_xpr(icase("SSA:callctxsensanalysis"))               | icase("SSA:callctxsens") | icase("SSA:ccs");
  ssadpAnalysis   = as_xpr(icase("SSA:deadpathelimanalysis"))              | icase("SSA:deadpath")    | icase("SSA:dp");
  ssaptAnalysis   = as_xpr(icase("SSA:pointstoanalysis"))                  | icase("SSA:pointsto")    | icase("SSA:pt");
  ssavmAnalysis   = as_xpr(icase("SSA:virtualmethodanalysis"))             | icase("SSA:virtualmem")  | icase("SSA:vm");
  ssaspcpAnalysis = as_xpr(icase("SSA:sparseconstantpropagationanalysis")) | icase("SSA:spconstprop") | icase("SSA:spcp");
  ssaspvnAnalysis = as_xpr(icase("SSA:sparsevaluenumberinganalysis"))      | icase("SSA:spvalnum")    | icase("SSA:spvn");
  dynMonitor      = as_xpr(icase("dynamicmonitor"))                        | icase("dynmon")          | icase("dm");
  analysis = by_ref(noAnalysis)   |
             by_ref(cpAnalysis)   | by_ref(ldAnalysis) | by_ref(ccsAnalysis) |
             by_ref(dpAnalysis)   | by_ref(ptAnalysis) | by_ref(vmAnalysis)  |
             by_ref(spcpAnalysis) | by_ref(spvnAnalysis) |
             by_ref(ssacpAnalysis)   | by_ref(ssaldAnalysis) | by_ref(ssaccsAnalysis) |
             by_ref(ssadpAnalysis)   | by_ref(ssaptAnalysis) | by_ref(ssavmAnalysis)  |
             by_ref(ssaspcpAnalysis) | by_ref(ssaspvnAnalysis) |
             by_ref(dynMonitor);
  analysisList = '(' >> *_s >> (by_ref(analysis) | by_ref(compSpec)) >> *_s >> (!('(' >> *_s >>  +_w   >> *_s >> ')') ) >>
        *(*_s >> "," >> *_s >> (by_ref(analysis) | by_ref(compSpec)) >> *_s >> (!('(' >> *_s >>  +_w   >> *_s >> ')') ) ) >> *_s >> ')';
  compSpec = *_s >> by_ref(composer) >> *_s >> analysisList >> *_s;

  regexInitialized = true;
}

// Computes (if needed) and returns the SSA graph
SSAGraph* Fuse::GetSSAGraph() const {
  return rootComposer->GetSSAGraph(cdip);
}

/***************************************
 ***** Fuse::output_nested_results *****
 ***************************************/
 Fuse::output_nested_results::output_nested_results(
                              int tabs,
                              composerType& parentComposerType,
                              list<ComposedAnalysis*>& parentSubAnalyses,
                              Composer** parentComposer,
                              checkDataflowInfoPass* cdip)
    : tabs_( tabs ),
      parentComposerType(parentComposerType),
      parentSubAnalyses(parentSubAnalyses),
      parentComposer(parentComposer),
      cdip(cdip)
{
}

string Fuse::output_nested_results::composerType2Str(composerType type)
{ return (type==looseSeq? "looseSeq": (type==loosePar? "loosePar": (type==tight? "tight": (type==unknown? "unknown": "???")))); }

/****************
 ***** Fuse *****
 ****************/

// Execute a composed analysis on the AST denoted by the project.
// fuseCmd is the string representation of the composition of analyses that should be executed.
// Returns 0 on success and a non-zero error code on failure.
int Fuse::run(SgProject* project, const std::string& fuseCmd) {
  smatch what;
  if(regex_match(fuseCmd, what, compSpec)) {
    //cout << "MATCH composer\n";
    list<ComposedAnalysis*>  mySubAnalyses;
    output_nested_results::composerType rootComposerType = output_nested_results::unknown;

    cdip = new checkDataflowInfoPass();

    output_nested_results ons(0, rootComposerType, mySubAnalyses, &rootComposer, cdip);
    std::for_each(what.nested_results().begin(),
                  what.nested_results().end(),
                  ons);
    assert(rootComposer!=NULL);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    ((ChainComposer*)rootComposer)->runAnalysis();

/*      DynamicMonitor dynMon(rootComposer);
    dynMon.runAnalysis();*/

    gettimeofday(&end, NULL);
    cout << "Elapsed="<<((end.tv_sec*1000000+end.tv_usec) -
                         (start.tv_sec*1000000+start.tv_usec))/1000000.0<<"s"<<endl;

    // Create the FuseLabeler, which needs to know the ATS computed by the rootComposer
    if(labeler) delete labeler;
    labeler = new FuseLabeler(getProject(), *this);

    if(annotator) delete annotator;
    annotator = new AstAnnotator(labeler);

    //cout << "rootComposer="<<rootComposer<<" cdip->getNumErrors()="<<cdip->getNumErrors()<<endl;
    if(cdip) {
      if(cdip->getNumErrors() > 0) {
        cout << cdip->getNumErrors() << " Errors Reported!"<<endl;
        return 1;
      } else {
        cout << "PASS"<<endl;
        return 0;
      }
    } else
      return 0;

  } else {
    cout << "ERROR: cannot parse the fuse command \""<<fuseCmd<<"\"\n";
    return 2;
  }
}

// Places annotations into the AST that report the constant information that the most
// recent Fuse analysis composition has computed.
// verbose: if true, the annotations are printed after being placed
void Fuse::placeConstantPropagationAnnotations(bool verbose) {
  assert(fuseGloballyInitialized);
  ValueASTAttribute::placeLabeler(rootComposer, cdip, *labeler);
  annotator->annotateAstAttributesAsCommentsBeforeStatements(getProject(), "fuse_cp_below");
  if(verbose)
    printAttributes<ValueASTAttribute>(labeler, vIDMap, "fuse_cp_below");
}

// Places annotations into the AST that report the reaching use-definition information that the most
// recent Fuse analysis composition has computed.
// verbose: if true, the annotations are printed after being placed
void Fuse::placeUseDefAnnotations(bool verbose) {
  SIGHT_VERB_DECL(scope, ("Fuse::placeUseDefAnnotations"), 2, apiDebugLevel)
  assert(fuseGloballyInitialized);
  {
    SIGHT_VERB_DECL(scope, ("Placing", scope::high), 2, apiDebugLevel)
    FuseUDAttribute::place(*this);
  }
  {
    SIGHT_VERB_DECL(scope, ("Annotating As Comments", scope::high), 2, apiDebugLevel)
    annotator->annotateAstAttributesAsCommentsBeforeStatements(getProject(), "fuse_usedef");
  }
  if(verbose) {
    SIGHT_VERB_DECL(scope, ("Printing", scope::high), 2, apiDebugLevel)
    printAttributes<FuseUDAttribute>(labeler, vIDMap, "fuse_usedef");
  }
}

// Returns the set of entry FuseCFGNodes of the Abstract Transition System implemented by the given composer
std::set<FuseCFGNodePtr> Fuse::GetStartATSNodes()
{ return GetStartATSNodes(rootComposer); }

std::set<FuseCFGNodePtr> Fuse::GetStartATSNodes(Composer* composer)
{ return FuseCFGNode::parts2FuseCFGNodes(composer->GetStartAStates(NULL), this); }

// Returns the set of exit FuseCFGNodes of the Abstract Transition System implemented by the given composer
std::set<FuseCFGNodePtr> Fuse::GetEndATSNodes()
{ return GetEndATSNodes(rootComposer); }

std::set<FuseCFGNodePtr> Fuse::GetEndATSNodes(Composer* composer)
{ return FuseCFGNode::parts2FuseCFGNodes(composer->GetEndAStates(NULL), this); }

// Given a CFGNode from the VirtualCFG, returns the set of FuseCFGNodes that represent
// the nodes in the Abstract Transition System implemented by the given composer
// that refine this CFGNode.
std::set<FuseCFGNodePtr> Fuse::GetATSNodes(const CFGNode& cn)
{ return GetATSNodes(cn, rootComposer); }

std::set<FuseCFGNodePtr> Fuse::GetATSNodes(const CFGNode& cn, Composer* composer)
{
  std::set<CFGNode> base;
  base.insert(cn);
  std::set<PartPtr> refined;
  collectRefinedNodes(composer, refined, base);

  std::set<FuseCFGNodePtr> refinedFuseCFG;
  for(std::set<PartPtr>::iterator r=refined.begin(); r!=refined.end(); ++r)
    refinedFuseCFG.insert(makePtr<FuseCFGNode>(this, *r));

  return refinedFuseCFG;
}

// Given a CFGEdge from the VirtualCFG, returns the set of FuseCFGEdges that represent
// the edges in the Abstract Transition System implemented by the given composer
// that refine this CFGEdge.
std::set<FuseCFGEdgePtr> Fuse::GetATSEdges(const CFGEdge& ce)
{ return GetATSEdges(ce, rootComposer); }

std::set<FuseCFGEdgePtr> Fuse::GetATSEdges(const CFGEdge& ce, Composer* composer)
{
  std::set<CFGEdge> base;
  base.insert(ce);
  std::set<PartEdgePtr> refined;
  collectRefinedEdges(composer, refined, base);

  std::set<FuseCFGEdgePtr> refinedFuseCFG;
  for(std::set<PartEdgePtr>::iterator r=refined.begin(); r!=refined.end(); ++r)
    refinedFuseCFG.insert(makePtr<FuseCFGEdge>(this, *r));

  return refinedFuseCFG;
}


/*
// tps (01/14/2010) : Switching from rose.h to sage3.
#include "sage3basic.h"
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cctype>
#include <stdint.h>
using namespace std;*/

// Copy of code from rose/src/frontend/SageIII/virtualCFG/cfgToDot.C
template <typename NodeT, bool Debug>
inline void printNodeFuse(ostream& o, const NodeT& n) {
  string id = n->id();
  string nodeColor = "black";
  if (isSgStatement(n->getNode())) nodeColor = "blue";
  else if (isSgExpression(n->getNode())) nodeColor = "green";
  else if (isSgInitializedName(n->getNode())) nodeColor = "red";
  o << id << " [label=\"" << escapeString(Debug ? n->toStringForDebugging() : n->toString()) << "\", color=\"" << nodeColor << "\", style=\"" << (n->isInteresting() ? "solid" : "dotted") << "\"];\n";
}

template <typename EdgeT, bool Debug>
inline void printEdgeFuse(ostream& o, const EdgeT& e, bool isInEdge) {
  o << e->source()->id() << " -> " << e->target()->id() << " [label=\"" << escapeString(Debug ? e->toStringForDebugging() : e->toString()) << "\", style=\"" << (isInEdge ? "dotted" : "solid") << "\"];\n";
}

template <typename NodeT, typename EdgeT, bool Debug>
void printNodePlusEdgesFuse(ostream& o, NodeT n) {
  printNodeFuse<NodeT, Debug>(o, n);
  vector<EdgeT> outEdges = n->outEdges();
  for (unsigned int i = 0; i < outEdges.size(); ++i) {
    printEdgeFuse<EdgeT, Debug>(o, outEdges[i], false);
  }
  if (Debug) {
    vector<EdgeT> inEdges = n->inEdges();
    for (unsigned int i = 0; i < inEdges.size(); ++i) {
      printEdgeFuse<EdgeT, Debug>(o, inEdges[i], true);
    }
  }
}


template < typename NodeT, typename EdgeT ,bool Debug>  /*Filtered Node Type, Filtered CFG Type*/
class FuseCfgToDotImpl
{
  std::multimap < SgNode *, NodeT > exploredNodes;
  std::set < SgNode * >nodesPrinted;
  std::ostream & o;

  public:
  FuseCfgToDotImpl(std::ostream & o) :
    exploredNodes(), nodesPrinted(), o(o)
    {
    }
  void processNodes(NodeT n);
};

/* Internal template function handles the details*/
template < typename NodeT, typename EdgeT ,bool Debug>
void FuseCfgToDotImpl < NodeT, EdgeT, Debug >::processNodes(NodeT n)
{
  ROSE_ASSERT(n->getNode());
  std::pair < typename std::multimap < SgNode *, NodeT >::const_iterator,
      typename std::multimap < SgNode *, NodeT >::const_iterator > ip =
      exploredNodes.equal_range(n->getNode());
  for(typename std::multimap < SgNode *, NodeT >::const_iterator i = ip.first;
      i != ip.second; ++i)
  {
    if (i->second == n)
      return;
  }
  exploredNodes.insert(std::make_pair(n->getNode(), n));
  fuse::printNodePlusEdgesFuse<NodeT, EdgeT, Debug>(o, n);
  std::vector < EdgeT > outEdges = n->outEdges();
  for (unsigned int i = 0; i < outEdges.size(); ++i)
  {
    ROSE_ASSERT(outEdges[i]->source() == n);
    processNodes(outEdges[i]->target());
  }
  std::vector < EdgeT > inEdges = n->inEdges();
  for (unsigned int i = 0; i < inEdges.size(); ++i)
  {
    ROSE_ASSERT(inEdges[i]->target() == n);
    processNodes(inEdges[i]->source());
  }
}

ostream& Fuse::cfgToDot(ostream& o, string graphName, FuseCFGNodePtr start) {
  o << "digraph " << graphName << " {\n";
  FuseCfgToDotImpl<FuseCFGNodePtr, FuseCFGEdgePtr, false> impl(o);
  impl.processNodes(start);
  o << "}\n";
  return o;
}

ostream& Fuse::cfgToDot(ostream& o, string graphName, const std::set<FuseCFGNodePtr>& start) {
  o << "digraph " << graphName << " {\n";
  FuseCfgToDotImpl<FuseCFGNodePtr, FuseCFGEdgePtr, false> impl(o);
  for(std::set<FuseCFGNodePtr>::const_iterator n=start.begin(); n!=start.end(); ++n)
    impl.processNodes(*n);
  o << "}\n";
  return o;
}

//dump the full dot graph of a virtual control flow graph starting from SgNode (start)
void Fuse::cfgToDot(SgNode* start, const std::string& file_name)
{
  ROSE_ASSERT (start != NULL);
  cfgToDot(start->cfgForBeginning(), file_name);
}

//dump the full dot graph of a virtual control flow graph starting from CFGNode (start)
void Fuse::cfgToDot(CFGNode start, const std::string& file_name)
{
  ofstream ofile (file_name.c_str(), ios::out);
  cfgToDot(ofile, "defaultName", GetATSNodes(start));
}

//dump the full dot graph of a virtual control flow graph starting from FuseCFGNodePtr (start)
void Fuse::cfgToDot(FuseCFGNodePtr start, const std::string& file_name)
{
  ROSE_ASSERT (start);
  ofstream ofile (file_name.c_str(), ios::out);
  cfgToDot(ofile, "defaultName", start);
}

//dump the full dot graph of a virtual control flow graph starting from FuseCFGNodePtr (start)
void Fuse::cfgToDot(const std::string& file_name)
{
  ofstream ofile (file_name.c_str(), ios::out);
  cfgToDot(ofile, "defaultName", GetStartATSNodes());
}
/*
//dump the full dot graph of a virtual control flow graph starting from SgNode (start)
void cfgToDotForDebugging(SgNode* start, const std::string& file_name)
{
  ROSE_ASSERT (start != NULL);
  cfgToDotForDebugging(start->cfgForBeginning(), file_name);
}

//dump the full dot graph of a virtual control flow graph starting from CFGNode (start)
void cfgToDotForDebugging(CFGNode start, const std::string& file_name)
{
  cfgToDotForDebugging(makePtr<FuseCFGNode>(start), file_name);
}

//dump the full dot graph of a virtual control flow graph starting from FuseCFGNodePtr (start)
void cfgToDotForDebugging(FuseCFGNodePtr start, const std::string& file_name)
{
  ROSE_ASSERT (start != NULL);
  ofstream ofile (file_name.c_str(), ios::out);
  cfgToDotForDebugging(ofile, "defaultName", start);
}*/

// SSA operations
// --------------

// Returns the set of defs and phiDefs that reach the given phiDef
const std::set<SSAMemLocObjectPtr>& Fuse::getReachingDefsAtPhiDef(SSAMemLocObjectPtr pd) const
{ return GetSSAGraph()->getReachingDefsAtPhiDef(pd); }

// Returns the SSA uses for the given def
const std::set<SSAMemLocObjectPtr>& Fuse::getUses(SSAMemLocObjectPtr def) const
{ return GetSSAGraph()->getUses(def); }

// Returns the immediate SSA defs (concrete and phi defs) that directly define the given use
const std::set<SSAMemLocObjectPtr>& Fuse::getDirectDefs(SSAMemLocObjectPtr use) const
{ return GetSSAGraph()->getDefs(use); }

// Returns the concrete defs that transitively reach the given use
std::set<SSAMemLocObjectPtr> Fuse::getTransitiveDefs(SSAMemLocObjectPtr use) const
{ return GetSSAGraph()->getTransitiveDefs(use); }

// Given a Fuse MemLocObject and a VariableIdMapping, return the VariableId this MemLocObject
// denotes or the empty VariableId
VariableId Fuse::getVariableId(MemLocObjectPtr ml, VariableIdMapping& varIDMap) {
  SgNode* curSGN = ml->getBase();
  if(SgInitializedName* iname = isSgInitializedName(curSGN)) {
    return VariableId(varIDMap.variableId(iname));
  } else if(SgVarRefExp* var = isSgVarRefExp(curSGN)) {
    return VariableId(varIDMap.variableId(var));
  } else
    return VariableId();
}

// Keep a cache of the SgVarRefExp that corresponds to each SgVariableSymbol.
// By making sure that we're not continually creating new SgVarRefExps we enable Fuse's
// caching mechanisms to work more efficiently and avoid caching errors when the same pointer
// ends up pointing to SgVarRefExp for different SgVariableSymbols.
std::map<SgVariableSymbol*, SgVarRefExp*> Fuse::varSym2Ref;

// Returns a SgVarRefExp for the given SgVariableSymbol, ensuring that during the entire
// process lifetime no SgVarRefExp pointer can ever correspond to multiple symbols.
SgVarRefExp* Fuse::varSymbol2Ref(SgVariableSymbol* sym) {
  map<SgVariableSymbol*, SgVarRefExp*>::iterator i=varSym2Ref.find(sym);
  if(i==varSym2Ref.end()) {
    SgVarRefExp* ref = buildVarRefExp(sym);
    varSym2Ref[sym] = ref;
    return ref;
  } else
    return i->second;
}

#define valAttrDebugLevel 0

/***********************
 ***** FuseLabeler *****
 ***********************/

// FuseLabeler::FuseLabeler(SgNode* start, Fuse& f):Labeler(start), f(f) {
//   init();
// }
// // Registers the given LabelProperty at a fresh label ID and returns this ID
// Label FuseLabeler::registerLabel(LabelProperty lp, const std::set<FuseCFGNodePtr>& nodes) {
//   Label l = Labeler::registerLabel(lp);
//   label2Node[l].insert(nodes.begin(), nodes.end());
//   for(std::set<FuseCFGNodePtr>::const_iterator n=nodes.begin(); n!=nodes.end(); ++n)
//     node2Label[*n].insert(l);
//   return l;
// }

// void FuseLabeler::createLabels(SgNode* root) {
//   SIGHT_VERB_DECL(scope, ("FuseLabeler::createLabels()", scope::high), 2, apiDebugLevel)
//   RoseAst ast(root);
//   for(RoseAst::iterator i=ast.begin();i!=ast.end();++i) {
// //    if(int num=isLabelRelevantNode(*i)) {
// //      if(SgNodeHelper::Pattern::matchFunctionCall(*i)) {
// //        if(SgNodeHelper::Pattern::matchReturnStmtFunctionCallExp(*i)) {
// //          assert(num==3);
// //          std::set<FuseCFGNodePtr> nodesCall = f.GetATSNodes(CFGNode(*i, 2));
// //          if(nodes.size()>0) registerLabel(LabelProperty(*i,LabelProperty::LABEL_FUNCTIONCALL), nodesCall);
// //
// //          std::set<FuseCFGNodePtr> nodesReturn = f.GetATSNodes(CFGNode(*i, 3));
// //          registerLabel(LabelProperty(*i,LabelProperty::LABEL_FUNCTIONCALLRETURN), nodesReturn);
// //
// //          //registerLabel(LabelProperty(*i)); // return-stmt-label
// //        } else {
// //          assert(num==2);
// //          std::set<FuseCFGNodePtr> nodesCall = f.GetATSNodes(CFGNode(*i, 2));
// //          registerLabel(LabelProperty(*i,LabelProperty::LABEL_FUNCTIONCALL), nodesCall);
// //
// //          std::set<FuseCFGNodePtr> nodesReturn = f.GetATSNodes(CFGNode(*i, 3));
// //          registerLabel(LabelProperty(*i,LabelProperty::LABEL_FUNCTIONCALLRETURN), nodesReturn);
// //        }
// //      } else if(isSgFunctionDefinition(*i)) {
// //        assert(num==2);
// //        std::set<FuseCFGNodePtr> nodesEntry = f.GetATSNodes(CFGNode(isSgFunctionDefinition(*i)->get_declaration()->get_parameterList(), 0));
// //        registerLabel(LabelProperty(*i,LabelProperty::LABEL_FUNCTIONENTRY), nodesEntry);
// //
// //        std::set<FuseCFGNodePtr> nodesExit = f.GetATSNodes(CFGNode(*i, 3));
// //        registerLabel(LabelProperty(*i,LabelProperty::LABEL_FUNCTIONEXIT), nodesExit);
// //      } else if(isSgBasicBlock(*i)) {
// //        assert(num==2);
// //        //registerLabel(LabelProperty(*i,LabelProperty::LABEL_BLOCKBEGIN));
// //        //registerLabel(LabelProperty(*i,LabelProperty::LABEL_BLOCKEND));
// //      } else {
// //        /*// all other cases
// //        for(int j=0;j<num;j++) {
// //          registerLabel(LabelProperty(*i));
// //        }*/
// //      }
// //    }
//     SgNode* sgn = *i;

//     // Create a label for each CFGNode associated with this SgNode that also appears
//     // in the Fuse CFG
//     if(isSgGlobal(sgn)) continue;
//     if(isSgStatement(sgn)) {
//       //if(isSgExprStatement(sgn)) sgn = isSgExprStatement(sgn)->get_expression();
//       if(!isSgExprStatement(sgn)) continue;
//     }
//     //cout << "Labeled: "<<SgNode2Str(sgn)<<endl;
//     if(isSgExprStatement(sgn) || isSgExpression(sgn) || isSgInitializedName(sgn)) {
//       SgExpression* expr=NULL;
//       if(isSgExprStatement(sgn))   expr = isSgExprStatement(sgn)->get_expression();
//       else if(isSgExpression(sgn)) expr = isSgExpression(sgn);

//       SIGHT_VERB_DECL(scope, (txt()<<"    node="<<SgNode2Str(sgn)<<": cfgIndexForEnd="<<(sgn)->cfgIndexForEnd()), 2, apiDebugLevel)
//       //cout << "expr="<<SgNode2Str(expr)<<", (expr?expr:sgn)->cfgIndexForEnd()="<<(expr?expr:sgn)->cfgIndexForEnd()<<endl;
//       for(unsigned int idx=0; idx<=(expr?expr:sgn)->cfgIndexForEnd(); ++idx) {
//         std::set<FuseCFGNodePtr> nodes = f.GetATSNodes(CFGNode(expr?expr:sgn, idx));
//         SIGHT_VERB(dbg << "        idx="<<idx<<" #nodes="<<nodes.size()<<endl, 2, apiDebugLevel)
//         if(nodes.size()>0) {
//           LabelProperty::LabelType lt;
//           if(expr) {
//                  if(isSgFunctionCallExp(expr)       && idx==2) lt = LabelProperty::LABEL_FUNCTIONCALL;
//             else if(isSgFunctionCallExp(expr)       && idx==3) lt = LabelProperty::LABEL_FUNCTIONCALLRETURN;
//             else if(isSgFunctionParameterList(expr) && idx==0) lt = LabelProperty::LABEL_FUNCTIONENTRY;
//             else if(isSgFunctionDefinition(expr)    && idx==3) lt = LabelProperty::LABEL_FUNCTIONEXIT;
//             else lt = (LabelProperty::LabelType)(LabelProperty::LABEL_OTHER+idx);

//           } else
//             lt = (LabelProperty::LabelType)(LabelProperty::LABEL_OTHER+idx);

//           //cout << "Registering sgn="<<sgn<<" | "<<SgNode2Str(sgn)<<endl;
//           registerLabel(LabelProperty(sgn, lt), nodes);
//         }
//       }
//     }
//   } // for loop
// }

// // Returns the FuseCFGNodePtrs mapped to the given Label
// const std::set<FuseCFGNodePtr>& FuseLabeler::getNodes(Label l) const {
//   static std::set<FuseCFGNodePtr> emptySet;
//   map<Label, std::set<FuseCFGNodePtr> >::const_iterator i=label2Node.find(l);
//   if(i!=label2Node.end())
//     return i->second;
//   else
//     return emptySet;
// }

// // Returns the FuseCFGNodePtrs mapped to the given Label
// const std::set<Label>& FuseLabeler::getLabels(FuseCFGNodePtr n) const {
//   static std::set<Label> emptySet;
//   map<FuseCFGNodePtr, std::set<Label> >::const_iterator i=node2Label.find(n);
//   if(i!=node2Label.end())
//     return i->second;
//   else
//     return emptySet;

// }

// // Returns the LabelProperty of this label
// LabelProperty& FuseLabeler::getLabelProperty(const Label& l) {
//   assert(l < mappingLabelToLabelProperty.size());

//   return mappingLabelToLabelProperty[l];
// }


// // Returns the CFGNode denoted by this label
// CFGNode FuseLabeler::getCFGNode(const Label& l) {
//   LabelProperty& prop = getLabelProperty(l);
//   SgNode* sgn;
//   if(isSgExprStatement(prop.getNode()))
//     sgn = isSgExprStatement(prop.getNode())->get_expression();
//   else
//     sgn = prop.getNode();

//        if(prop.getLabelType() == LabelProperty::LABEL_FUNCTIONCALL)       return CFGNode(sgn, 2);
//   else if(prop.getLabelType() == LabelProperty::LABEL_FUNCTIONCALLRETURN) return CFGNode(sgn, 3);
//   else if(prop.getLabelType() == LabelProperty::LABEL_FUNCTIONENTRY)      return CFGNode(sgn, prop.getLabelType());
//   else if(prop.getLabelType() == LabelProperty::LABEL_FUNCTIONEXIT)       return CFGNode(sgn, prop.getLabelType());
//   else if(LabelProperty::LABEL_OTHER<=prop.getLabelType() && prop.getLabelType()<LabelProperty::LABEL_FUNCTIONCALL) {
//     //scope s(txt() << "node: "<<SgNode2Str(prop.getNode())<<", label="<<prop.getLabelType()<<"="<<(prop.getLabelType()-LabelProperty::LABEL_OTHER));
//     return CFGNode(sgn, prop.getLabelType()-LabelProperty::LABEL_OTHER);
//   } else
//     return CFGNode(prop.getNode(), 0);
// }

// /*****************************
//  ***** ValueASTAttribute *****
//  *****************************/

// // Returns a list of all the variables declared in the given scope
// std::list<SgVariableSymbol*> ValueASTAttribute::getAllVarSymbolsInScope(SgScopeStatement *scope) {
//   list<SgVariableSymbol*> vars;
//   SgVariableSymbol* var = scope->first_variable_symbol();
//   while(var) {
//     vars.push_back(var);
//     var = scope->next_variable_symbol();
//   }
//   return vars;
// }

// // Returns a list of all the variables declared in the scopes that contain this node
// std::list<SgVariableSymbol*> ValueASTAttribute::getAllVarSymbols(SgNode *n) {
//   n = n->get_parent();
//   list<SgVariableSymbol*> allVars;
//   while(n) {
//     if(SgScopeStatement* scope = isSgScopeStatement(n)) {
//       list<SgVariableSymbol*> scopeVars = getAllVarSymbolsInScope(scope);
//       for(list<SgVariableSymbol*>::iterator v=scopeVars.begin(); v!=scopeVars.end(); v++) {
//         allVars.push_back(*v);
//       }
//     }
//     n = n->get_parent();
//   }
//   return allVars;
// }

// ValueASTAttribute::ValueASTAttribute(SgNode* n, Composer* composer, checkDataflowInfoPass* cdip, dirT dir, std::string label):
//       composer(composer), cdip(cdip), dir(dir), label(label) {
//   SIGHT_VERB_DECL(scope, (txt()<<"ValueASTAttribute("<<SgNode2Str(n)<<")"), 1, valAttrDebugLevel)

//   if(isSgFunctionDefinition(n)) return;

//   // NOTE: this is a temporary hack where we assume the appropriate index for the CFGNode
//   //       that represents SgNode n. In the future we should change Expr2* to accept CFGNodes
//   if(isSgInitializedName(n)) cn = CFGNode(n, 1);
//   else if(isSgBinaryOp(n))   cn = CFGNode(n, 2);
//   else if(isSgUnaryOp(n)) {
//     if(isSgCastExp(n))       cn = CFGNode(n, 0);
//     else if(isSgAddressOfOp(n) || isSgPointerDerefExp(n) || isSgPlusPlusOp(n) || isSgMinusMinusOp(n)) cn = CFGNode(n, 1);
//     else                     cn = CFGNode(n, 2);
//   }
//   else if(isSgValueExp(n))   cn = CFGNode(n, 1);
//   else if(isSgFunctionCallExp(n)) cn = CFGNode(n, 2);
//   else if(isSgWhileStmt(n))  cn = CFGNode(n, 1);
//   else                       cn = CFGNode(n, 0);


//   // Collect the PartEdges (computed by the given composer) that refine the incoming or
//   // outgoing edges of the given SgNode
//   //collectRefinedEdges(composer, refinedEdges, (dir==above? cn.inEdges(): cn.outEdges()));
//   if(dir==above) collectIncomingRefinedEdges(composer, refinedEdges, cn);
//   else           collectOutgoingRefinedEdges(composer, refinedEdges, cn);

//   allInScopeVars = getAllVarSymbols(n);
//   SIGHT_VERB_IF(1, valAttrDebugLevel)
//     scope s("allInScopeVars");
//     for(std::list<SgVariableSymbol *>::iterator v=allInScopeVars.begin(); v!=allInScopeVars.end(); ++v) {
//       dbg << SgNode2Str(*v)<<endl;
//     }
//   SIGHT_VERB_FI()
// }

// ValueASTAttribute::ValueASTAttribute(CFGNode cn, Composer* composer, checkDataflowInfoPass* cdip, dirT dir, std::string label):
//       composer(composer), cdip(cdip), dir(dir), label(label), cn(cn) {
//   SIGHT_VERB_DECL(scope, (txt()<<"ValueASTAttribute("<<CFGNode2Str(cn)<<")"), 1, valAttrDebugLevel)

//   // Collect the PartEdges (computed by the given composer) that refine the incoming or
//   // outgoing edges of the given SgNode
//   //collectRefinedEdges(composer, refinedEdges, (dir==above? cn.inEdges(): cn.outEdges()));
//   if(dir==above) collectIncomingRefinedEdges(composer, refinedEdges, cn);
//   else           collectOutgoingRefinedEdges(composer, refinedEdges, cn);

//   allInScopeVars = getAllVarSymbols(cn.getNode());
//   SIGHT_VERB_IF(1, valAttrDebugLevel)
//     scope s("allInScopeVars");
//     for(std::list<SgVariableSymbol *>::iterator v=allInScopeVars.begin(); v!=allInScopeVars.end(); ++v) {
//       dbg << SgNode2Str(*v)<<endl;
//     }
//   SIGHT_VERB_FI()
// }

// // Apply Expr2Value for the given expression to all the edges in refinedEdges and return
// // the union of the resulting ValueObjects
// ValueObjectPtr ValueASTAttribute::Expr2Val(SgExpression* expr) {
//   ValueObjectPtr val;

//   SIGHT_VERB(dbg << "Expr2Val("<<SgNode2Str(expr)<<") #refinedEdges="<<refinedEdges.size()<<endl, 1, valAttrDebugLevel)
//   for(std::set<PartEdgePtr>::iterator r=refinedEdges.begin(); r!=refinedEdges.end(); r++) {
//     SIGHT_VERB(dbg << "    edge="<<(*r)->str()<<endl, 1, valAttrDebugLevel)
//     ValueObjectPtr edgeVal = composer->Expr2Val(expr, *r, cdip);
//     if(val==NULLValueObject) val = edgeVal;
//     else                     val->meetUpdate(edgeVal, *r, composer, NULL);
//   }
//   SIGHT_VERB(dbg << "Expr2Val returning val="<<val.get()<<endl, 1, valAttrDebugLevel)

//   return val;
// }

// bool ValueASTAttribute::isConstantInteger(SgVarRefExp* ref) {
//   SIGHT_VERB_DECL(scope, (txt()<<"isConstantInteger(ref="<<SgNode2Str(ref)<<")"), 1, valAttrDebugLevel)
//   ValueObjectPtr val = Expr2Val(ref);
//   SIGHT_VERB(dbg << "isConstantInteger() val="<<val.get()<<endl, 1, valAttrDebugLevel)
//   if(!val) return false;
//   SIGHT_VERB(dbg << "isConstantInteger() val="<<val->str()<<endl, 1, valAttrDebugLevel)
//   if(val->isConcrete() && isStrictIntegerType(val->getConcreteType())) {
//     std::set<boost::shared_ptr<SgValueExp> > cVals = val->getConcreteValue();
//     if(cVals.size()==1) return true;
//   }
//   return false;
// }

// bool ValueASTAttribute::isConstantInteger(VariableId varId) {
//   SIGHT_VERB_DECL(scope, (txt()<<"isConstantInteger(varID="<<SgNode2Str(Fuse::getVIDMap()->getSymbol(varId))<<")"), 1, valAttrDebugLevel)
//   ValueObjectPtr val = Expr2Val(buildVarRefExp(isSgVariableSymbol(Fuse::getVIDMap()->getSymbol(varId))));
//   SIGHT_VERB(dbg << "isConstantInteger() val="<<val.get()<<endl, 1, valAttrDebugLevel)
//   if(!val) return false;
//   SIGHT_VERB(dbg << "isConstantInteger() val(concrete="<<val->isConcrete()<<")="<<val->str()<<endl, 1, valAttrDebugLevel)
//   if(val->isConcrete() && isStrictIntegerType(val->getConcreteType())) {
//     std::set<boost::shared_ptr<SgValueExp> > cVals = val->getConcreteValue();
//     if(cVals.size()==1) return true;
//   }
//   return false;
// }

// CPAstAttributeInterface::ConstantInteger ValueASTAttribute::getConstantInteger(SgVarRefExp* ref) {
//   SIGHT_VERB(dbg << "getConstantInteger("<<SgNode2Str(ref)<<")"<<endl, 1, valAttrDebugLevel)
//   ValueObjectPtr val = Expr2Val(ref);
//   assert(val);
//   if(val->isConcrete() && isStrictIntegerType(val->getConcreteType())) {
//     SIGHT_VERB(dbg << "val="<<val->str()<<endl, 1, valAttrDebugLevel)
//     std::set<boost::shared_ptr<SgValueExp> > cVals = val->getConcreteValue();
//     if(cVals.size()==1) return getIntegerConstantValue((*cVals.begin()).get());
//   }
//   ROSE_ASSERT(0);
// }

// CPAstAttributeInterface::ConstantInteger ValueASTAttribute::getConstantInteger(VariableId varId) {
//   ValueObjectPtr val = Expr2Val(buildVarRefExp(isSgVariableSymbol(Fuse::getVIDMap()->getSymbol(varId))));
//   if(val->isConcrete() && isStrictIntegerType(val->getConcreteType())) {
//     std::set<boost::shared_ptr<SgValueExp> > cVals = val->getConcreteValue();
//     if(cVals.size()==1) return getIntegerConstantValue((*cVals.begin()).get());
//   }
//   ROSE_ASSERT(0);
// }

// ValueASTAttribute::~ValueASTAttribute() {}

// string ValueASTAttribute::toString() {
//   if(cn.getNode()==NULL) return "";

//   if(isSgExprStatement(cn.getNode())) {
//     ValueASTAttribute exprLabel(isSgExprStatement(cn.getNode())->get_expression(), composer, cdip, dir, label);
//     return exprLabel.toString();
//   } else if(isSgReturnStmt(cn.getNode())) {
//     ValueASTAttribute exprLabel(isSgReturnStmt(cn.getNode())->get_expression(), composer, cdip, dir, label);
//     return exprLabel.toString();
//   } else if(isSgIfStmt(cn.getNode())) {
//     ValueASTAttribute exprLabel(isSgIfStmt(cn.getNode())->get_conditional(), composer, cdip, dir, label);
//     return exprLabel.toString();
//   } else if(isSgCastExp(cn.getNode())) {
//     ValueASTAttribute exprLabel(isSgCastExp(cn.getNode())->get_operand(), composer, cdip, dir, label);
//     return exprLabel.toString();
//   } else if(isSgBasicBlock(cn.getNode())) {
//     // Recursively call toString on the first statement in the block
//     const SgStatementPtrList & stmt = isSgBasicBlock(cn.getNode())->get_statements();
//     if(stmt.size()==0) return "[]";
//     ValueASTAttribute exprLabel(*stmt.begin(), composer, cdip, dir, label);
//     return exprLabel.toString();
//   } else if(isSgFunctionDeclaration(cn.getNode()) || isSgFunctionDefinition(cn.getNode()) || isSgPragmaDeclaration(cn.getNode())) {
//     return "[]";
//   }

//   ostringstream s;
//   {
//   SIGHT_VERB_DECL(scope, (txt()<<"ValueASTAttribute::toString("<<CFGNode2Str(cn)<<")"), 1, valAttrDebugLevel)
//   s << "["<<label<<" : "<<SgNode2Str(cn.getNode())<<": ";
//   //cout << CFGNode2Str(cn) << ": "<<endl;;
//   int numConstants=0;
//   for(list<SgVariableSymbol *>::iterator v=allInScopeVars.begin(); v!=allInScopeVars.end(); v++) {
//     if(v!=allInScopeVars.begin()) { s << ", "; }

//     //SgVarRefExp* ref = buildVarRefExp(*v);
//     SgVarRefExp* ref = Fuse::varSymbol2Ref(*v);
//     SIGHT_VERB(dbg << "    "<<ref->unparseToString() << " = ", 1, valAttrDebugLevel)
//     s << ref->unparseToString()<<"=";
//     if(isConstantInteger(ref)) {
//       //if(numConstants>0) { s << ", "; }
//       s << getConstantInteger(ref);
//       SIGHT_VERB(dbg << getConstantInteger(ref)<<endl, 1, valAttrDebugLevel)
//       //s << getConstantInteger(ref);
//       //cout << "        "<<ref->unparseToString() << "=" << getConstantInteger(ref)<<endl;;
//       numConstants++;
//     } else {
//       SIGHT_VERB(dbg << "?"<<endl, 1, valAttrDebugLevel)
//       s << "?";
//     }

//     //delete ref;
//   }
//   s << "]";
//   }
//   //cout << endl;
//   return s.str();
// }

// void ValueASTAttribute::place(Composer* composer, checkDataflowInfoPass* cdip) {
//   RoseAst ast(getProject());
//   for(RoseAst::iterator i=ast.begin(); i!=ast.end();++i) {
//     if(isSgExpression(*i)) {
//       (*i)->setAttribute("fuse_cp_above", new ValueASTAttribute(*i, composer, cdip, above, "fuse_cp_above"));
//       (*i)->setAttribute("fuse_cp_below", new ValueASTAttribute(*i, composer, cdip, below, "fuse_cp_below"));
//     }
//   }
// }

// void ValueASTAttribute::placeLabeler(Composer* composer, checkDataflowInfoPass* cdip, Labeler& labeler) {
//   for(Labeler::iterator i=labeler.begin();i!=labeler.end();++i) {
//     SgNode* node=labeler.getNode(*i);
//     ROSE_ASSERT(node);
//     node->setAttribute("fuse_cp_above", new ValueASTAttribute(node, composer, cdip, above, "fuse_cp_above"));
//     node->setAttribute("fuse_cp_below", new ValueASTAttribute(node, composer, cdip, below, "fuse_cp_below"));
//   }
// }

// void ValueASTAttribute::placeLabeler(Composer* composer, checkDataflowInfoPass* cdip, FuseLabeler& labeler) {
//   for(Labeler::iterator i=labeler.begin();i!=labeler.end();++i) {
//     SgNode* node=labeler.getNode(*i);
//     ROSE_ASSERT(node);
//     //cout << "placeLabeler cn="<<CFGNode2Str(labeler.getCFGNode(*i))<<endl;
//     node->setAttribute("fuse_cp_above", new ValueASTAttribute(labeler.getCFGNode(*i), composer, cdip, above, "fuse_cp_above"));
//     node->setAttribute("fuse_cp_below", new ValueASTAttribute(labeler.getCFGNode(*i), composer, cdip, below, "fuse_cp_below"));
//   }
// }

// void ValueASTAttribute::show(Composer* composer, VariableIdMapping& vIDMap) {
//   RoseAst ast(getProject());
//   for(RoseAst::iterator i=ast.begin(); i!=ast.end();++i) {
//     /*if(SgExprStatement* stmt = isSgExprStatement(*i)) {
//       cout << "i="<<SgNode2Str(*i)<<endl;
//       ValueASTAttribute* above = (ValueASTAttribute*)stmt->get_expression()->getAttribute("fuse_cp_above");
//       cout << "    above="<<above<<endl;
//      // ValueASTAttribute* below = stmt->get_expression()->getAttribute("fuse_cp_aft");

//       // Determine whether any of the variables in the current scope are constants above
//       // this statement
//       SgScopeStatement* scope = getEnclosingProcedure(stmt);
//       cout << "    scope="<<SgNode2Str(scope)<<endl;
//       SgVariableSymbol *cur = scope->first_variable_symbol();
//       while(cur) {
//         cout << "    cur="<<SgNode2Str(cur)<<", isConstantInteger="<<above->isConstantInteger(Fuse::getVIDMap()->variableId(cur))<<endl;
//         if(above->isConstantInteger(Fuse::getVIDMap()->variableId(cur)))
//           cout << "Variable "<<cur->unparseToString()<<"="<<above->getConstantInteger(Fuse::getVIDMap()->variableId(cur))<<" at "<<SgNode2Str(stmt)<<endl;
//         cur = scope->next_variable_symbol();
//       }
//     }*/
//     if(SgVarRefExp* ref = isSgVarRefExp(*i)) {
//       if(isStrictIntegerType(ref->get_type())) {
//         //cout << "ref="<<SgNode2Str(ref)<<endl;
//         ValueASTAttribute* above = (ValueASTAttribute*)(ref->getAttribute("fuse_cp_below"));
//         //cout << "    above="<<above<<endl;
//         //cout << "    isConstantInteger="<<above->isConstantInteger(ref)<<endl; cout.flush();
//         if(above->isConstantInteger(ref))
//           cout << SgNode2Str(ref)<<": Value="<<above->getConstantInteger(ref)<<endl;
//       }
//     }
//   }
// }

// /***************************
//  ***** FuseUDAttribute *****
//  ***************************/

// FuseUDAttribute::FuseUDAttribute(CFGNode cn, Fuse& f) : f(f)
// {
//   std::set<CFGNode> base;
//   base.insert(cn);
//   std::set<PartPtr> refined;
//   collectRefinedNodes(f.getComposer(), refined, base);

//   // Create FuseCFGNodes from the refined parts
//   /*scope s("FuseUDAttribute::FuseUDAttribute()");
//   dbg << "    cn="<<CFGNode2Str(cn)<<endl;
//   dbg << "    #base="<<base.size()<<", #refined="<<refined.size()<<endl;*/
//   for(std::set<PartPtr>::iterator r=refined.begin(); r!=refined.end(); ++r) {
//     nodes.insert(makePtr<FuseCFGNode>(&f, *r));
//   }
// }

// FuseUDAttribute::~FuseUDAttribute()
// {}

// // variables used at this location
// VariableIdSet FuseUDAttribute::useVariables(VariableIdMapping& vim) {
//   VariableIdSet vars;
//   //scope s(txt()<<"FuseUDAttribute::useVariables() #nodes="<<nodes.size());
//   // Collect the VariableIds of all the uses at the FuseCFGNodes in nodes
//   for(std::set<FuseCFGNodePtr>::iterator n=nodes.begin(); n!=nodes.end(); ++n) {
//     const std::set<SSAMemLocObjectPtr>& uses = (*n)->getUses();
//     //dbg << "n="<<(*n)->toString()<<", #uses="<<uses.size()<<endl;
//     for(std::set<SSAMemLocObjectPtr>::const_iterator u=uses.begin(); u!=uses.end(); ++u) {
//       indent ind;
//       /*dbg << "u="<<(*u)->str()<<endl;
//       dbg << "base="<<SgNode2Str((*u)->getBase())<<endl;*/
//       if(isSgInitializedName((*u)->getBase()))
//         vars.insert(vim.variableId(isSgInitializedName((*u)->getBase())));
//       else if(isSgVarRefExp((*u)->getBase()))
//         vars.insert(vim.variableId(isSgVarRefExp((*u)->getBase())));
//     }
//   }

//   return vars;
// }

// // definitions reaching this location
// LabelSet FuseUDAttribute::definitionsOfVariable(VariableId var) {
//   //scope s(txt()<<"FuseUDAttribute::definitionsOfVariable("<<var.toString()<<"))");
//   std::set<FuseCFGNodePtr> reachingDefNodes;

//   // Find the SSAMemLocObjects that denote uses of var
//   for(std::set<FuseCFGNodePtr>::iterator n=nodes.begin(); n!=nodes.end(); ++n) {
//     const std::set<SSAMemLocObjectPtr>& uses = (*n)->getUses();
//     for(std::set<SSAMemLocObjectPtr>::const_iterator u=uses.begin(); u!=uses.end(); ++u) {
//       //dbg << "base="<<SgNode2Str((*u)->getBase())<<endl;
//       // If the current use corresponds to var
//       if((isSgInitializedName((*u)->getBase()) && f.getVIDMap()->variableId(isSgInitializedName((*u)->getBase()))==var) ||
//          (isSgVarRefExp((*u)->getBase())       && f.getVIDMap()->variableId(isSgVarRefExp((*u)->getBase()))==var)) {

//         // Iterate over all the defs of this use and add their FuseCFGNodes to reachingDefNodes
//         std::set<SSAMemLocObjectPtr> defs = f.getTransitiveDefs(*u);
//         //dbg << "#defs="<<defs.size()<<endl;
//         for(std::set<SSAMemLocObjectPtr>::iterator d=defs.begin(); d!=defs.end(); ++d) {
//           reachingDefNodes.insert((*d)->getFuseCFGNodeLoc(&f));
//         }
//       }
//     }
//   }

//   // Find the labels of the FuseCFGNodes in reachingDefNodes
//   LabelSet reachingDefLabels;
//   for(std::set<FuseCFGNodePtr>::iterator n=reachingDefNodes.begin(); n!=reachingDefNodes.end(); ++n) {
//     const std::set<Label>& labels = f.getLabeler()->getLabels(*n);
//     reachingDefLabels.insert(labels.begin(), labels.end());
// /*    for(Labeler::iterator i=f.getLabeler()->begin(); i!=f.getLabeler()->end(); ++i) {
//       // If the FuseCFGNodes associated with the current label contain the current reaching
//       // def node, add this label to reachingDefLabels
//       CFGNode cn = f.getLabeler()->getCFGNode(*i);
//       std::set<FuseCFGNodePtr> matchingNodes = f.GetATSNodes(cn);
//       if(matchingNodes.find(*n) != matchingNodes.end())
//         reachingDefLabels.insert(*i);
//     }*/
//   }

//   return reachingDefLabels;
// }

// string FuseUDAttribute::toString() {
//   ostringstream out;
//   out << "[FuseUDAttribute: ";
//   //scope s("FuseUDAttribute::toString()", scope::high);
//   VariableIdSet uses = useVariables(*f.getVIDMap());
//   //dbg << "#uses="<<uses.size()<<endl;
//   for(VariableIdSet::iterator u=uses.begin(); u!=uses.end(); ++u) {
//     out << "Use "<<u->toString()<<", Definition Locations:";//<<endl;
//     //dbg << "Use "<<u->toString()<<endl;
//     LabelSet defLabels = definitionsOfVariable(*u);
//     //dbg << "#defLabels="<<defLabels.size()<<endl;
//     for(LabelSet::iterator l=defLabels.begin(); l!=defLabels.end(); ++l) {
//       out << " "<<CFGNode2Str(f.getLabeler()->getCFGNode(*l));//<<endl;
//       //dbg << "    "<<CFGNode2Str(f.getLabeler()->getCFGNode(*l))<<endl;
//     }
//   }
//   out << "]";
//   return out.str();
// }

// void FuseUDAttribute::place(Fuse& f) {
//   for(Labeler::iterator i=f.getLabeler()->begin(); i!=f.getLabeler()->end(); ++i) {
//     //cout << "place sgn="<<f.getLabeler()->getNode(*i)<<"="<<SgNode2Str(f.getLabeler()->getNode(*i))<<endl;
//     FuseUDAttribute* attr = new FuseUDAttribute(f.getLabeler()->getCFGNode(*i), f);
//     //cout << "    attr="<<attr->toString()<<endl;
//     f.getLabeler()->getNode(*i)->setAttribute("fuse_usedef", attr);
//   }
// }

// void FuseUDAttribute::show(Fuse& f, VariableIdMapping& vim) {
//   RoseAst ast(getProject());
//   for(RoseAst::iterator i=ast.begin(); i!=ast.end();++i) {
//     if(SgVarRefExp* ref = isSgVarRefExp(*i)) {
//       FuseUDAttribute* ud = (FuseUDAttribute*)(ref->getAttribute("fuse_usedef"));

//       cout << ud->toString();
//     }
//   }
// }


/**************************************************************
 ***** VirtualCFG-like API for representing ATS structure *****
 **************************************************************/

//! Wraps a Part with an API that is almost identical to the CFGNode API except that all functions
//! that return CFGNodes and CFGEdges now return boost pointers to FuseCFGNodes and FuseCFGEdges.
//! This is to make it possible for code that currently uses VirtualCFGs to easily transition to working
//! on top of Fuse Abstract Transition Systems. It was not possible to fully retain the VirtualCFG
//! API since it returns copies of CFGNode and CFGEdge objects, making it impossible to create new
//! classs that implements the same API and derive from CFGNode and CFGEdge.

/***********************
 ***** FuseCFGNode *****
 ***********************/

// Function that generates FuseCFGNodes. Used in code regions that need to generate such nodes
// but can't include fuse.h due to circular include dependencies
FuseCFGNodePtr createFuseCFGNode(Fuse* fuseAnalysis, PartPtr part)
{ return makePtr<FuseCFGNode>(fuseAnalysis, part); }

FuseCFGNode::FuseCFGNode() {}
FuseCFGNode::FuseCFGNode(Fuse* fuseAnalysis, PartPtr part) : fuseAnalysis(fuseAnalysis), part(part) {
  std::set<CFGNode> cns = part->CFGNodes();

  // FuseCFGNodes are only compatible with ATSs where each part denotes exactly one CFGNode
  assert(cns.size()==1);

  cn = *cns.begin();
}

// Wraps all the Parts in the given set with FuseCFGNodes and returns the set of these Nodes
std::set<FuseCFGNodePtr> FuseCFGNode::parts2FuseCFGNodes(const std::set<PartPtr>& parts, Fuse* fuseAnalysis) {
  std::set<FuseCFGNodePtr> nodes;
  for(std::set<PartPtr>::const_iterator p=parts.begin(); p!=parts.end(); ++p)
    nodes.insert(makePtr<FuseCFGNode>(fuseAnalysis, *p));
  return nodes;
}

//! Pretty string for Dot node labels, etc.
std::string FuseCFGNode::toString() const
{ return cn.toString(); }

//! String for debugging graphs
std::string FuseCFGNode::toStringForDebugging() const
{ return cn.toStringForDebugging(); }

//! String representation of this Node's Fuse information
std::string FuseCFGNode::toStringFuse() const
{ return part->str(); }

//! ID to use for Dot, etc.
std::string FuseCFGNode::id() const
{ return cn.id(); }

//! The underlying AST node
SgNode* FuseCFGNode::getNode() const
{ return cn.getNode(); }

//! An identifying index within the AST node given by getNode()
unsigned int FuseCFGNode::getIndex() const
{ return cn.getIndex(); }

//! Outgoing control flow edges from this node
vector<FuseCFGEdgePtr> FuseCFGNode::outEdges() const
{
  // Get the outgoing edges of the Part
  list<PartEdgePtr> partEdges = part->outEdges();

  // Create a vector of FuseCFGEdgePtrs that wrap the part
  vector<FuseCFGEdgePtr> cfgEdge;
  for(list<PartEdgePtr>::iterator e=partEdges.begin(); e!=partEdges.end(); ++e)
    cfgEdge.push_back(makePtr<FuseCFGEdge>(fuseAnalysis, *e));

  return cfgEdge;
}

//! Incoming control flow edges to this node
std::vector<FuseCFGEdgePtr> FuseCFGNode::inEdges() const
{
  // Get the outgoing edges of the Part
  list<PartEdgePtr> partEdges = part->inEdges();

  // Create a vector of FuseCFGEdgePtrs that wrap the part
  vector<FuseCFGEdgePtr> cfgEdge;
  for(list<PartEdgePtr>::iterator e=partEdges.begin(); e!=partEdges.end(); ++e)
    cfgEdge.push_back(makePtr<FuseCFGEdge>(fuseAnalysis, *e));

  return cfgEdge;
}

// Compares the FuseCFGNodes
bool FuseCFGNode::operator==(const FuseCFGNodePtr& that) const
{ return part == that->part; }
bool FuseCFGNode::operator< (const FuseCFGNodePtr& that) const
{ return part <  that->part; }
bool FuseCFGNode::operator!=(const FuseCFGNodePtr& that) const
{ return part != that->part; }
bool FuseCFGNode::operator>=(const FuseCFGNodePtr& that) const
{ return part >= that->part; }
bool FuseCFGNode::operator<=(const FuseCFGNodePtr& that) const
{ return part <= that->part; }
bool FuseCFGNode::operator> (const FuseCFGNodePtr& that) const
{ return part >  that->part; }

// Return the set of uses at this node
const std::set<SSAMemLocObjectPtr>& FuseCFGNode::getUses() const
{ return fuseAnalysis->GetSSAGraph()->getUses(part); }

// Return the set of defs at this node
const std::set<SSAMemLocObjectPtr>& FuseCFGNode::getDefs() const
{ return fuseAnalysis->GetSSAGraph()->getDefs(part); }

// Get the list of definitions of the arguments within the function call at the given part
// Get the mapping from each argument of the function call at the given part to the corresponding
// parameters the argument defines
const std::list<std::pair<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& FuseCFGNode::getFunc2Params() const
{ return fuseAnalysis->GetSSAGraph()->getFunc2Params(part); }

// Returns whether the given Part has a phi node before or after it
bool FuseCFGNode::isPhiNode() const
{ return fuseAnalysis->GetSSAGraph()->isPhiNode(part); }

// Returns the mapping of phiDef MemLocs at the given phiNode before the given part
// to the defs and phiDefs that reach them.
const std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> >& FuseCFGNode::getDefsUsesAtPhiNode() const
{ return fuseAnalysis->GetSSAGraph()->getDefsUsesAtPhiNode(part); }

// Returns whether the given def terminates at the given phi node
bool FuseCFGNode::defTerminatesAtPhiNode(SSAMemLocObjectPtr def) const
{ return fuseAnalysis->GetSSAGraph()->defTerminatesAtPhiNode(part, def); }


/***********************
 ***** FuseCFGEdge *****
 ***********************/

//! Default constructor. Used for compatibility with containers
FuseCFGEdge::FuseCFGEdge() {}

//! Constructor
FuseCFGEdge::FuseCFGEdge(Fuse* fuseAnalysis, PartEdgePtr pedge) : fuseAnalysis(fuseAnalysis), pedge(pedge) {
  // Create a CFGEdge from the CFGNodes at pedge's source and target Parts

  std::set<CFGNode> sourceCns = pedge->source()->CFGNodes();
  // FuseCFGNodes are only compatible with ATSs where each part denotes exactly one CFGNode
  assert(sourceCns.size()==1);

  std::set<CFGNode> targetCns = pedge->target()->CFGNodes();
  // FuseCFGNodes are only compatible with ATSs where each part denotes exactly one CFGNode
  assert(targetCns.size()==1);

  ce = CFGEdge(*sourceCns.begin(), *targetCns.begin());
}

// Wraps all the PartEdges in the given set with FuseCFGEdges and returns the set of these Edges
std::set<FuseCFGEdgePtr> FuseCFGEdge::parts2FuseCFGEdges(const std::set<PartEdgePtr>& pedges, Fuse* fuseAnalysis) {
  std::set<FuseCFGEdgePtr> edges;
  for(std::set<PartEdgePtr>::const_iterator p=pedges.begin(); p!=pedges.end(); ++p)
    edges.insert(makePtr<FuseCFGEdge>(fuseAnalysis, *p));
  return edges;
}

//! Pretty string for Dot node labels, etc.
std::string FuseCFGEdge::toString() const
{
/*  cout << "FuseCFGEdge::toString() part="<<pedge->str()<<endl;
  cout << "                         ce="<<CFGNode2Str(ce.source())<<" == > "<<endl;
  cout << "                            "<<CFGNode2Str(ce.target())<<endl;*/
//  return ce.toString(); }
  return CFGEdge2Str(ce); }

//! String for debugging graphs
std::string FuseCFGEdge::toStringForDebugging() const
{ return ce.toStringForDebugging(); }

//! String representation of this Edge's Fuse information
std::string FuseCFGEdge::toStringFuse() const
{ return pedge->str(); }

//! ID to use for Dot, etc.
std::string FuseCFGEdge::id() const
{ return ce.id(); }

//! The source (beginning) CFG node
FuseCFGNodePtr FuseCFGEdge::source() const
{ return makePtr<FuseCFGNode>(fuseAnalysis, pedge->source()); }

//! The target (ending) CFG node
FuseCFGNodePtr FuseCFGEdge::target() const
{ return makePtr<FuseCFGNode>(fuseAnalysis, pedge->target()); }

//! The control flow condition that enables this edge
EdgeConditionKind FuseCFGEdge::condition() const
{
  /*cout << "FuseCFGEdge::condition() ce="<<CFGNode2Str(ce.source())<<" == > "<<endl;
  cout << "                            "<<CFGNode2Str(ce.target())<<endl;*/
  return ce.condition(); }

//! The label of the case represented by an eckCaseLabel edge
SgExpression* FuseCFGEdge::caseLabel() const
{ return ce.caseLabel(); }

//! The expression of the computed goto represented by the eckArithmeticIf* conditions
unsigned int FuseCFGEdge::computedGotoCaseIndex() const
{ return ce.computedGotoCaseIndex(); }

//! The test or case key that is tested as a condition of this control flow edge
SgExpression* FuseCFGEdge::conditionBasedOn() const
{ return ce.conditionBasedOn(); }

//! Variables going out of scope across this edge (not extensively tested)
std::vector<SgInitializedName*> FuseCFGEdge::scopesBeingExited() const
{ return ce.scopesBeingExited(); }

//! Variables coming into scope across this edge (not extensively tested)
std::vector<SgInitializedName*> FuseCFGEdge::scopesBeingEntered() const
{ return ce.scopesBeingEntered(); }

// Compares the FuseCFGEdges
bool FuseCFGEdge::operator==(const FuseCFGEdgePtr& that) const
{ return pedge == that->pedge; }
bool FuseCFGEdge::operator< (const FuseCFGEdgePtr& that) const
{ return pedge <  that->pedge; }
bool FuseCFGEdge::operator!=(const FuseCFGEdgePtr& that) const
{ return pedge != that->pedge; }
bool FuseCFGEdge::operator>=(const FuseCFGEdgePtr& that) const
{ return pedge >= that->pedge; }
bool FuseCFGEdge::operator<=(const FuseCFGEdgePtr& that) const
{ return pedge <= that->pedge; }
bool FuseCFGEdge::operator> (const FuseCFGEdgePtr& that) const
{ return pedge >  that->pedge; }

} // namespace fuse
