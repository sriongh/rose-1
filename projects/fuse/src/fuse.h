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
#include <ctype.h>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include "sageBuilder.h"
#include "sageInterface.h"
#include "sight.h"
using namespace sight;
#include <sys/time.h>
#include "stx_analysis.h"
#include "VariableIdMapping.h"
#include "AstAnnotator.h"
#include "AnalysisInterface.h"
// using namespace boost::xpressive;
using namespace SageBuilder;
using namespace SageInterface;
//using namespace scc_private;

namespace fuse {

class FuseCFGNode;
typedef CompSharedPtr<FuseCFGNode> FuseCFGNodePtr;
class FuseCFGEdge;
typedef CompSharedPtr<FuseCFGEdge> FuseCFGEdgePtr;

class FuseSSA;

/*!
  The Fuse system represents constraints on possible application executions as a finite-state transition
  system, analogous to the control flow graph, where each state represents a set of application execution
  prefixes or suffixes. These constraints
  are implemented by arbitrary analyses inside the Fuse system and can be queried by providing a SgNode that
  denotes a finite-size SgExpression or SgInitializedName and asking for the set of
  - Values (any scalars representable as SgValues),
  - Memory Regions (any contiguous block of memory such as a malloc buffer or a global variable)
  - Memory Locations (offsets into Memory Regions that hold the state of individual scalars)
  - Code Locations

  Fuse analyses are run by creating a Fuse object an then invoking its run() method with a string that
  specifies the specific composition of analyses to be executed. To access the constraints computed by
  these analyses the following mechanisms are available:
  - Call Fuse::placeConstantPropagationAnnotations(), Fuse::placeUseDefAnnotations() and similar methods
    on the Fuse object. This uses the FuseLabeler to place the annotations that implement these APIs on all
    the relevant SgNodes. These include any SgNodes specified by the default labeler, as well as any nodes
    for which Fuse specifically computes information (Fuse mostly focuses on SgExpressions rather than
    SgStatements). Further, the annotations placed by the FuseLabeler are stored separately for each CFGNode
    associated with each SgNode, making it easy to disambiguate the execution point at which the information
    is provided.
  - Access the transition system computed by Fuse itself. The API of this transition system closely follows
    the API for the VirtualCFG. However, it was not possible to simply inherit from the VirtualCFG API because
    it is designed to operate on raw CFGNodes and CFGEdge rather than pointers to these, meaning that it cannot
    manipulate objects that inherit from these classes. Further, the structure of the Fuse transition system
    is derived from the Virtual CFG. For each CFGNode in the VirtualCFG (there is one
    such CFGNode for each execution point associated with each SgNode), there exists zero or more nodes in the
    Fuse transition system.
    - A given CFGNode is not represented in the Fuse transition system if a Fuse analysis
      has proven that this code location is unreachable.
    - A given CFGNode has multiple corresponding nodes in the Fuse transition system if some Fuse analysis(es)
      have chosen to refine the graph to improve analysis precision. For example, a call context sensitivity
      analysis may create multiple copies of a function depending on its call site, while a path sensitivity
      analysis may split the nodes that follow a given conditional, with one copy for each case.
    The Fuse transition system is implemented via the classes FuseCFGNode and FuseCFGEdge, which represent
    the graph's nodes and edges. These nodes can be accessed by:
    - Calling Fuse::GetStartATSNodes() and Fuse::GetEndATSNodes() on the Fuse object to get the set of entry
      and exit nodes of the graph.
    - Calling Fuse::GetATSNodes(const CFGNode& cn) and Fuse::GetATSNodes(const CFGEdge& ce) on the Fuse object
      to get the set of FuseCFGNodes and FuseCFGEdges that correspond to the given VirtualCFG CFGNode and
      CFGEdge.
    Given these FuseCFGNodes and FuseCFGEdges it is possible to navigate the full graph by calling
    FuseCFGNode::outEdges(), FuseCFGNode::inEdges(), FuseCFGEdge::source() and FuseCFGEdge::target().
    The Fuse transition system can be visualized by calling Fuse::cfgToDot() on the Fuse object.
    Note that all Fuse transition system functions work on FuseCFGNodePtrs and FuseCFGEdgePtrs, which are
    boost shared pointers to these objects (actually, thin wrappers around these pointers that implement
    comparison operations via the operations in the objects they refer to).

  The Fuse transition system can also be used as an SSA graph. The idea is that each FuseCFGNode has some number of
  memory location definitions and uses, represented as Fuse MemLocObjects. Tracking the relationships between
  such definitions and uses, and maintaining constraints about the values of memory location is made complicated
  by the fact that each memory location may be defined at multiple locations in the source code and there
  may be ambiguity regarding which locations are defined as each source code location.
  SSA graphs organize memory locations definitions and uses within the context of the transition system
  via the following mechanisms:
  - Each definition to or use of a memory location is represented as a unique object of type SSAMemLocObject,
    which contains the MemLocObject that describes the set of memory locations being defined or use as well as
    the Fuse transition system node of the definition or use. The Fuse SSA API makes it possible to both retrieve the
    SSAMemLocObjects that describe the definitions and uses at each FuseCFGNode (methods FuseCFGNode::getUses()
    and FuseCFGNode::getDefs()) and to retrieve the definitions that reach a use (method
    static FuseCFGNode::getDefs(SSAMemLocObjectPtr use)), as well as uses reached by a definition (method
    static FuseCFGNode::getUses(SSAMemLocObjectPtr def)).
    For example, consider the following code:
        a=1
        b=1
        a = a+b
        c = a*b
        b = b-a+c
    Even though there are two MemLocObjects a and b, the SSA graph creates a separate SSAMemLocObject for each
    assignment to a and b, which are represented below as extra indexes on these variables:
        a_0^{def}=1
        b_0^{def}=1
        a_1^{def} = a_0^{use}+b_0^{use}
        c_0^{def} = a_1^{use}*b_1^{use}
        b_1^{def} = b_2^{use}-a_2^{use}+c_1^{use}
    These indexed defs and uses are connected to each other; for example:
    - a_1^{def} --def2use--> a_1^{use}, a_2^{use}
    - b_2^{use} <--use2def-- b_0^{def}
  - To manage ambiguity about which memory locations are defined at a given FuseATSNode, SSA graphs include
    special phi nodes, which are virtual assignments to memory locations. For example, given the following program:
        a=1
        b=1
        if(???) p = &q;
        *p = n
        print *p
    The assignment to *p may write to memory location a or b and the choice is not known at compile-time.
    SSA graphs represent this ambiguity by adding a new virtual assignment of phi0 that is explicitly linked to the
    assignments of a and b as input. The value of phi0 is used as input by the print statement:
        a_0^{def}=1
        b_0^{def}=1
        if(???) p_0^{def} = {&q}_0^{use};
        phi(*p)_0^{def} = n_0^{use}
        print phi(*p)_0^{use}
    This graph connects defs to uses as follows (a subset of connections are shown):
    - a_0^{def}, b_0^{def} --def2phi--> phi(*p)_0^{def}
    - phi(*p)_0^{def} --def2use--> phi(*p)_0^{use}
    The FuseCFG manages phi nodes by tracking all ambiguity regarding memory location assignments immediately
    before each FuseCFGNode. If there is an ambiguity, the node is labeled a "phi node". For each ambiguous
    set of definitions ADSet (set of defined memory locations overlap due to imprecision in knowledge of control
    or the memory model information) it creates a new phi definition PhiD and connects all definitions in ADSet.
    PhiD is subsequently treated as a real definition of all the memory locations that may be defined at
    definitions in ADSet and each subsequent use Use of any of these memory locations is linked to PhiD
    (FuseCFGNode::getDefs(Use) returns PhiD).
    Users can check whether a given FuseCFGNode is a phi node (there exist phi definitions at this node) by
    calling FuseCFGNode::isPhiNode(). They call FuseCFGNode::getDefsUsesAtPhiNode() to get the
    mapping from each phi definition at the node (represented as an SSAMemLocObject) to the set of SSAMemLocObjects
    that denote the ambiguous definitions that reach this phi definition (may be regular or phi definitions).
    Further, users can call static FuseCFGNode::getReachingDefsAtPhiDef(SSAMemLocObjectPtr pd) to get the set of
    definitions that reach the given phi def.
*/

// class FuseLabeler;

class Fuse /*: public AliasAnalysisInterface*/ {
  // Regex expressions for the composition command, defined globally so that they can be used inside main
  // (where they're initialized) as well as inside output_nested_results()
  static sregex
         composer, lcComposer, lpComposer, tComposer,
         analysis, noAnalysis, cpAnalysis, ldAnalysis, ccsAnalysis, dpAnalysis, ptAnalysis,
                   vmAnalysis, spcpAnalysis, spvnAnalysis,
                   ssacpAnalysis, ssaldAnalysis, ssaccsAnalysis, ssadpAnalysis, ssaptAnalysis,
                   ssavmAnalysis, ssaspcpAnalysis, ssaspvnAnalysis,
                   dynMonitor,
         analysisList, compSpec;

  // Records whether we've executed the basic Fuse initialization that is common to
  // all instances of Fuse objects
  static bool fuseGloballyInitialized;

  // Maps the IDs of variables about which we'll report analysis results
  static SPRAY::VariableIdMapping *vIDMap;

  // The labeler that will be used to place AST annotations that capture the
  // output of the Fuse analysis
  // static FuseLabeler* labeler;

  // The annotator that will place labels at AST nodes
  // static AstAnnotator* annotator;

  // The root composer of the composed analysis executed by Fuse.
  // This variable is reset after each execution of the run() method to point to the
  // root composer of the most recent analysis composition.
  Composer* rootComposer;

  // The analysis that checks the results of the composed analysis against any
  // CompDebugAssert() calls inserted into the code.
  checkDataflowInfoPass* cdip;

  // The SSA graph based on the ATS implemented by the Fuse analyses this object runs
  SSAGraph* ssa;

  friend class FuseCFGNode;
  friend class FuseCFGEdge;

  protected:
  // Computes (if needed) and returns the SSA graph
  SSAGraph* GetSSAGraph() const;

  public:
  Fuse(int argc=0, char **argv=NULL);

  // Returns the map the IDs of variables about which we'll report analysis results
  static SPRAY::VariableIdMapping * getVIDMap();

  // Returns the labeler
  // static FuseLabeler* getLabeler();

  // Returns the Fuse analysis' root composer
  Composer* getComposer() const { return rootComposer; }

  // Initialize the static regular expressions so that they're ready to parse fuse commands
  static void initRegex();

  // Displays nested results to std::cout with indenting
  struct output_nested_results
  {
    typedef enum {looseSeq, loosePar, tight, unknown} composerType;

    int tabs_;
    composerType parentComposerType;
    list<ComposedAnalysis*>& parentSubAnalyses;
    Composer** parentComposer;
    checkDataflowInfoPass* cdip;

    output_nested_results(int tabs,
                          composerType& parentComposerType,
                          list<ComposedAnalysis*>& parentSubAnalyses,
                          Composer** parentComposer,
                          checkDataflowInfoPass* cdip);

    string composerType2Str(composerType type);

    template< typename BidiIterT >
    void operator ()( match_results< BidiIterT > const &what )
    {
      //scope s("operator ()");
      // first, do some indenting
      typedef typename std::iterator_traits< BidiIterT >::value_type char_type;
      //char_type space_ch = char_type(' ');

      string match = what[0];

      // If this term is an analysis rather than a composer
      smatch subWhat;
      //dbg << "match="<<match<<", parentComposerType="<<composerType2Str(parentComposerType)<<", parentComposer="<<parentComposer<<endl;
      if(regex_match(match, subWhat, analysis)) {
        /*cout << "analysis match="<<match<<", smatch="<<subWhat<<endl;
        cout << "regex_match(match, subWhat, cpAnalysis)="<<regex_match(match, subWhat, cpAnalysis)<<endl;
        cout << "regex_match(match, subWhat, ssacpAnalysis)="<<regex_match(match, subWhat, ssacpAnalysis)<<endl;*/
        // Create the selected analysis and add it to the parent's sub-analysis list
             if(regex_match(match, subWhat, cpAnalysis))      { parentSubAnalyses.push_back(new ConstantPropagationAnalysis(/*useSSA*/false)); }
        else if(regex_match(match, subWhat, ssacpAnalysis))   { parentSubAnalyses.push_back(new ConstantPropagationAnalysis(/*useSSA*/true)); }
        else if(regex_match(match, subWhat, ldAnalysis))      { parentSubAnalyses.push_back(new LiveDeadMemAnalysis()); }
        else if(regex_match(match, subWhat, ssaldAnalysis))   { parentSubAnalyses.push_back(new LiveDeadMemAnalysis()); }
        else if(regex_match(match, subWhat, dpAnalysis))      { parentSubAnalyses.push_back(new DeadPathElimAnalysis(false)); }
        else if(regex_match(match, subWhat, ssadpAnalysis))   { parentSubAnalyses.push_back(new DeadPathElimAnalysis(true)); }
        else if(regex_match(match, subWhat, ccsAnalysis))  {
          parentSubAnalyses.push_back(new CallContextSensitivityAnalysis(1, CallContextSensitivityAnalysis::callSite, /*trackBase2RefinedPartEdgeMapping*/ true));

  /*        list<ComposedAnalysis*> mySubAnalyses;
          composerType myComposerType = unknown;
          output_nested_results ons(tabs_ + 1, myComposerType, mySubAnalyses, NULL, NULL);
          std::for_each(
              subWhat.nested_results().begin(),
              subWhat.nested_results().end(),
              ons);*/
        }
        else if(regex_match(match, subWhat, ssaccsAnalysis))  { parentSubAnalyses.push_back(new CallContextSensitivityAnalysis(1, CallContextSensitivityAnalysis::callSite, /*trackBase2RefinedPartEdgeMapping*/ true)); }
        else if(regex_match(match, subWhat, ptAnalysis))      { parentSubAnalyses.push_back(new PointsToAnalysis(/*useSSA*/false)); }
        else if(regex_match(match, subWhat, ssaptAnalysis))   { parentSubAnalyses.push_back(new PointsToAnalysis(/*useSSA*/true)); }
        else if(regex_match(match, subWhat, vmAnalysis))      { parentSubAnalyses.push_back(new VirtualMethodAnalysis(/*useSSA*/false)); }
        else if(regex_match(match, subWhat, ssavmAnalysis))   { parentSubAnalyses.push_back(new VirtualMethodAnalysis(/*useSSA*/true)); }
        else if(regex_match(match, subWhat, dynMonitor))      { parentSubAnalyses.push_back(new DynamicMonitor()); }

        //else if(regex_match(match, subWhat, spcpAnalysis)) { parentSubAnalyses.push_back(new SparseConstantAnalysis()); }
        //else if(regex_match(match, subWhat, spvnAnalysis)) { parentSubAnalyses.push_back(new SparseValueNumbering()); }

      // Otherwise, if this is a composer, create the analyses in its sub-analysis list and then create the composer
      } else if(regex_match(match, subWhat, lcComposer)) {
        //std::fill_n( std::ostream_iterator<char_type>( std::cout ), tabs_ * 4, space_ch ); cout << "LOOSE SEQUENTIAL\n"<<endl;
        parentComposerType = looseSeq;
      } else if(regex_match(match, subWhat, lpComposer)) {
        //std::fill_n( std::ostream_iterator<char_type>( std::cout ), tabs_ * 4, space_ch ); cout << "LOOSE PARALLEL\n"<<endl;
        parentComposerType = loosePar;
      } else if(regex_match(match, subWhat, tComposer)) {
        parentComposerType = tight;
      // Finally, if this is a list of analyses for a given parent composer
      } else if(parentComposerType != unknown) {
        //cout << "other match="<<match<<endl;

        //assert(parentComposerType != unknown);
        list<ComposedAnalysis*> mySubAnalyses;
        composerType myComposerType = unknown;

        // Output any nested matches
        output_nested_results ons(tabs_ + 1, myComposerType, mySubAnalyses, NULL, NULL);
        std::for_each(
            what.nested_results().begin(),
            what.nested_results().end(),
            ons);
        // std::fill_n( std::ostream_iterator<char_type>( std::cout ), tabs_ * 4, space_ch );
        /*dbg << "#mySubAnalyses="<<mySubAnalyses.size()<<endl;
        for(list<ComposedAnalysis*>::iterator i=mySubAnalyses.begin(); i!=mySubAnalyses.end(); i++)
        { dbg << "    "<<(*i)->str()<<endl; }*/

        if(parentComposerType == looseSeq) {
  //dbg << "ChainComposer"<<endl;
          ChainComposer* cc = new ChainComposer(mySubAnalyses, cdip, true);
          // Until ChainComposer is made to be a ComposedAnalysis, we cannot add it to the parentSubAnalyses list. This means that
          // LooseComposer can only be used at the outer-most level of composition
          // !!!parentSubAnalyses.push_back(cc);
          if(parentComposer) *parentComposer = cc;
        } else if(parentComposerType == loosePar) {
  //dbg << "LooseParallelComposer"<<endl;
          LooseParallelComposer* lp = new LooseParallelComposer(mySubAnalyses);
          parentSubAnalyses.push_back(lp);
          if(parentComposer) *parentComposer = lp;
        } else if(parentComposerType == tight) {
  //dbg << "TightComposer parentComposer="<<parentComposer<<endl;
          //TightComposer* t = new TightComposer(mySubAnalyses, /*trackBase2RefinedPartEdgeMapping*/ false, /*useSSA*/ false);
          TightComposer* t = new TightComposer(mySubAnalyses);
          parentSubAnalyses.push_back(t);
          if(parentComposer) *parentComposer = t;
        }
      } else {
        //dbg << "Other"<<endl;
        // Output any nested matches
        list<ComposedAnalysis*> mySubAnalyses;
        composerType myComposerType = unknown;
        output_nested_results ons(tabs_ + 1, myComposerType, parentSubAnalyses, NULL, NULL);
        //output_nested_resuGetSSAGraph(ComposedAnalysis* client)=0;lts ons(tabs_ + 1, myComposerType, mySubAnalyses, NULL, NULL);
        //output_nested_results ons(tabs_ + 1, parentComposerType, parentSubAnalyses, parentComposer, NULL);
        std::for_each(
            what.nested_results().begin(),
            what.nested_results().end(),
            ons);
      }
    }
  }; // struct output_nested_results

  // Execute a composed analysis on the AST denoted by the project.
  // fuseCmd is the string representation of the composition of analyses that should be executed.
  // Returns 0 on success and a non-zero error code on failure.
  int run(SgProject* project, const std::string& fuseCmd);

  // Places annotations into the AST that report the constant information that the most
  // recent Fuse analysis composition has computed.
  // verbose: if true, the annotations are printed after being placed
  void placeConstantPropagationAnnotations(bool verbose=false);

  // Places annotations into the AST that report the reaching use-definition information that the most
  // recent Fuse analysis composition has computed.
  // verbose: if true, the annotations are printed after being placed
  void placeUseDefAnnotations(bool verbose=false);

  // Returns the set of entry and exit FuseCFGNodes of the Abstract Transition System implemented by the given composer
  std::set<FuseCFGNodePtr> GetStartATSNodes();
  std::set<FuseCFGNodePtr> GetStartATSNodes(Composer* composer);
  std::set<FuseCFGNodePtr> GetEndATSNodes();
  std::set<FuseCFGNodePtr> GetEndATSNodes(Composer* composer);

  // Given a CFGNode from the VirtualCFG, returns the set of FuseCFGNodes that represent
  // the nodes in the Abstract Transition System implemented by the given composer
  // that refine this CFGNode.
  std::set<FuseCFGNodePtr> GetATSNodes(const CFGNode& cn);
  std::set<FuseCFGNodePtr> GetATSNodes(const CFGNode& cn, Composer* composer);

  // Given a CFGEdge from the VirtualCFG, returns the set of FuseCFGEdges that represent
  // the edges in the Abstract Transition System implemented by the given composer
  // that refine this CFGEdge.
  std::set<FuseCFGEdgePtr> GetATSEdges(const CFGEdge& ce);
  std::set<FuseCFGEdgePtr> GetATSEdges(const CFGEdge& ce, Composer* composer);

  std::ostream& cfgToDot(std::ostream& o, std::string graphName, FuseCFGNodePtr start);
  std::ostream& cfgToDot(ostream& o, string graphName, const std::set<FuseCFGNodePtr>& start);
  //std::ostream& cfgToDotForDebugging(std::ostream& o, std::string graphName, FuseCFGNodePtr start);

  //! Dump the filtered dot graph of a virtual control flow graph starting from SgNode (start)
  void cfgToDot (SgNode*        start, const std::string& file_name);
  void cfgToDot (CFGNode        start, const std::string& file_name);
  void cfgToDot (FuseCFGNodePtr start, const std::string& file_name);
  void cfgToDot (const std::string& file_name);

  //! Dump the full dot graph of a virtual control flow graph starting from SgNode (start)
  /*ROSE_DLL_API void cfgToDotForDebugging (SgNode*        start, const std::string& file_name);
  ROSE_DLL_API void cfgToDotForDebugging (CFGNode        start, const std::string& file_name);
  ROSE_DLL_API void cfgToDotForDebugging (FuseCFGNodePtr start, const std::string& file_name);*/

  // SSA operations
  // --------------

  // Returns the set of defs and phiDefs that reach the given phiDef
  const std::set<SSAMemLocObjectPtr>& getReachingDefsAtPhiDef(SSAMemLocObjectPtr pd) const;

  // Returns the SSA uses for the given def
  const std::set<SSAMemLocObjectPtr>& getUses(SSAMemLocObjectPtr def) const;

  // Returns the immediate SSA defs (concrete and phi defs) that directly define the given use
  const std::set<SSAMemLocObjectPtr>& getDirectDefs(SSAMemLocObjectPtr use) const;

  // Returns the concrete defs that transitively reach the given use
  std::set<SSAMemLocObjectPtr> getTransitiveDefs(SSAMemLocObjectPtr use) const;

  // Given a Fuse MemLocObject and a VariableIdMapping, return the VariableId this MemLocObject
  // denotes or the empty VariableId
  VariableId getVariableId(MemLocObjectPtr ml, VariableIdMapping& varIDMap);

  // Keep a cache of the SgVarRefExp that corresponds to each SgVariableSymbol.
  // By making sure that we're not continually creating new SgVarRefExps we enable Fuse's
  // caching mechanisms to work more efficiently and avoid caching errors when the same pointer
  // ends up pointing to SgVarRefExp for different SgVariableSymbols.
  static std::map<SgVariableSymbol*, SgVarRefExp*> varSym2Ref;

  // Returns a SgVarRefExp for the given SgVariableSymbol, ensuring that during the entire
  // process lifetime no SgVarRefExp pointer can ever correspond to multiple symbols.
  static SgVarRefExp* varSymbol2Ref(SgVariableSymbol* sym);

  /***********************************
   ****** AliasAnalysisInterface *****
   ***********************************/
  //bool may_alias(AstInterface& fa, const AstNodePtr& r1, const AstNodePtr& r2) = 0;
}; // class Fuse
} // namespace fuse

#include "midend/abstractLayer/VariableIdMapping.h"
// #include "midend/abstractLayer/CPAstAttributeInterface.h"
// #include "midend/abstractLayer/AstAnnotator.h"
// #include "midend/abstractLayer/Labeler.h"

namespace fuse {

// class FuseLabeler : public Labeler {
//   Fuse& f;
//   std::map<Label, std::set<FuseCFGNodePtr> > label2Node;
//   std::map<FuseCFGNodePtr, std::set<Label> > node2Label;
//   public:
//   FuseLabeler(SgNode* start, Fuse& f);

//   // Registers the given LabelProperty at a fresh label ID and returns this ID
//   Label registerLabel(LabelProperty lp, const std::set<FuseCFGNodePtr>& nodes);

//   virtual void createLabels(SgNode* node);

//   // Returns the FuseCFGNodePtrs mapped to the given Label
//   const std::set<FuseCFGNodePtr>& getNodes(Label l) const;

//   // Returns the FuseCFGNodePtrs mapped to the given Label
//   const std::set<Label>& getLabels(FuseCFGNodePtr n) const;

//   // Returns the LabelProperty of this label
//   LabelProperty& getLabelProperty(const Label& l);

//   // Returns the CFGNode denoted by this label
//   CFGNode getCFGNode(const Label& l);
// }; // class FuseLabeler

/************************************
 ***** Constant Propagation API *****
 ************************************/

// class ValueASTAttribute: public CPAstAttributeInterface {
//   protected:
//   std::set<PartEdgePtr> refinedEdges;
//   Composer* composer;
//   checkDataflowInfoPass* cdip;
//   public:
//   typedef enum {above, below} dirT;
//   dirT dir;
//   std::string label;
//   std::list<SgVariableSymbol *> allInScopeVars;
//   CFGNode cn;

//   // Returns a list of all the variables declared in the given scope
//   std::list<SgVariableSymbol*> getAllVarSymbolsInScope(SgScopeStatement *scope);

//   // Returns a list of all the variables declared in the scopes that contain this node
//   std::list<SgVariableSymbol*> getAllVarSymbols(SgNode *n);

//   ValueASTAttribute(SgNode* n, Composer* composer, checkDataflowInfoPass* cdip, dirT dir, std::string label);
//   ValueASTAttribute(CFGNode cn, Composer* composer, checkDataflowInfoPass* cdip, dirT dir, std::string label);

//   // Apply Expr2Value for the given expression to all the edges in refinedEdges and return
//   // the union of the resulting ValueObjects
//   ValueObjectPtr Expr2Val(SgExpression* expr);

//   bool isConstantInteger(SgVarRefExp* ref);

//   bool isConstantInteger(VariableId varId);

//   ConstantInteger getConstantInteger(SgVarRefExp* ref);

//   ConstantInteger getConstantInteger(VariableId varId);

//   ~ValueASTAttribute();

//   string toString();

//   static void place(Composer* composer, checkDataflowInfoPass* cdip);

//   static void placeLabeler(Composer* composer, checkDataflowInfoPass* cdip, Labeler& labeler);
//   static void placeLabeler(Composer* composer, checkDataflowInfoPass* cdip, FuseLabeler& labeler);

//   static void show(Composer* composer, VariableIdMapping& vIDMap);
// }; // class ValueASTAttribute

// template<typename T>
// void printAttributes(Labeler* labeler, VariableIdMapping* vim, string attributeName) {
//    long labelNum=labeler->numberOfLabels();
//    for(long i=0;i<labelNum;++i) {
//      Label lab=i;
//      SgNode* node=labeler->getNode(i);
//      //cout << "attributeName="<<attributeName<<" node="<<SgNode2Str(node)<<", attr="<<node->getAttribute(attributeName)<<endl;

//      cout<<"@Label "<<lab<<":";
//      T* attr=dynamic_cast<T*>(node->getAttribute(attributeName));
//      if(node)
//        cout<<attr->toString()<<endl; // the attribute is casted to also allow to call other functions here
//      else
//        cout<<" none.";
//      cout<<endl;
//    }
// }

// /***********************
//  ***** Use-Def API *****
//  ***********************/
// typedef std::pair<VariableId, LabelSet> VariableIdLabelSetPair;
// typedef std::set<VariableIdLabelSetPair> UseDefInfo;

// class UDAstAttributeInterface : public DFAstAttribute {
//   public:
//   UDAstAttributeInterface() {}
//   ~UDAstAttributeInterface() {}

//   // variables used at this location
//   virtual VariableIdSet useVariables(VariableIdMapping& vim)=0;

//   // definitions reaching this location
//   virtual LabelSet definitionsOfVariable(VariableId var)=0;

//  private:
// };

// class FuseUDAttribute: public UDAstAttributeInterface {
//   Fuse& f;
//   std::set<FuseCFGNodePtr> nodes;

//   public:
//   FuseUDAttribute(CFGNode cn, Fuse& f);
//   ~FuseUDAttribute();

//   // variables used at this location
//   virtual VariableIdSet useVariables(VariableIdMapping& vim);

//   // definitions reaching this location
//   virtual LabelSet definitionsOfVariable(VariableId var);

//   string toString();

//   static void place(Fuse& f);

//   static void show(Fuse& f, VariableIdMapping& vIDMap);
// };

/**************************************************************
 ***** VirtualCFG-like API for representing ATS structure *****
 **************************************************************/

// Function that generates FuseCFGNodes. Used in code regions that need to generate such nodes
// but can't include fuse.h due to circular include dependencies
FuseCFGNodePtr createFuseCFGNode(Fuse* fuseAnalysis, PartPtr part);

//! A node in the Fuse ATS (represented as a CFG).  Each CFG node corresponds to an AST
//! node, but there can be several CFG nodes for a given AST node.
class FuseCFGNode : public boost::enable_shared_from_this<FuseCFGNode> {
  // Points to the base Fuse object that performed the analyses that produce this ATS node.
  Fuse* fuseAnalysis;

  //! Part this node wraps
  PartPtr part;

  //! The CFGNode at this part
  CFGNode cn;

  public:
  FuseCFGNode();
  FuseCFGNode(Fuse* fuseAnalysis, PartPtr part);

  // Wraps all the Parts in the given set with FuseCFGNodes and returns the set of these Nodes
  static std::set<FuseCFGNodePtr> parts2FuseCFGNodes(const std::set<PartPtr>& parts, Fuse* fuseAnalysis);

  //! Pretty string for Dot node labels, etc.
  std::string toString() const;
  //! String for debugging graphs
  std::string toStringForDebugging() const;
  //! String representation of this Node's Fuse information
  std::string toStringFuse() const;
  //! ID to use for Dot, etc.
  std::string id() const;
  //! The underlying AST node
  SgNode* getNode() const;
  //! An identifying index within the AST node given by getNode()
  unsigned int getIndex() const;

  //! Returns the Fuse Part node that this FuseCFGNode wraps
  PartPtr getPart() const { return part; }

  //! Outgoing control flow edges from this node
  std::vector<FuseCFGEdgePtr> outEdges() const;
  //! Incoming control flow edges to this node
  std::vector<FuseCFGEdgePtr> inEdges() const;

  //! Test whether this node satisfies a (fairly arbitrary) standard for
  //! "interestingness".  There are many administrative nodes in the raw CFG
  //! (nodes that do not correspond to operations in the program), and this
  //! function filters them out.
  bool isInteresting() const { return true; }

  // Compares the FuseCFGNodes
  bool operator==(const FuseCFGNodePtr& that) const;
  bool operator< (const FuseCFGNodePtr& that) const;
  bool operator!=(const FuseCFGNodePtr& that) const;
  bool operator>=(const FuseCFGNodePtr& that) const;
  bool operator<=(const FuseCFGNodePtr& that) const;
  bool operator> (const FuseCFGNodePtr& that) const;

  // SSA operations
  // --------------
  // Return the set of uses at this node
  const std::set<SSAMemLocObjectPtr>& getUses() const;

  // Return the set of defs at this node
  const std::set<SSAMemLocObjectPtr>& getDefs() const;

  // Get the list of definitions of the arguments within the function call at the given part
  // Get the mapping from each argument of the function call at the given part to the corresponding
  // parameters the argument defines
  const std::list<std::pair<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > >& getFunc2Params() const;

  // Returns whether the given Part has a phi node before or after it
  bool isPhiNode() const;

  // Returns the mapping of phiDef MemLocs at the given phiNode before the given part
  // to the defs and phiDefs that reach them.
  const std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> >& getDefsUsesAtPhiNode() const;

  // Returns whether the given def terminates at the given phi node
  bool defTerminatesAtPhiNode(SSAMemLocObjectPtr def) const;

}; // end class FuseCFGNode

//! A control flow edge connecting two CFG nodes, with an edge condition to
//! indicate edge types
class FuseCFGEdge : public boost::enable_shared_from_this<FuseCFGEdge> {
  // Points to the base Fuse object that performed the analyses that produce this ATS edge.
  Fuse* fuseAnalysis;

  //! Part this node wraps
  PartEdgePtr pedge;

  //! The CFGEdge that connects this part's source and target CFGNodes
  CFGEdge ce;

  public:
  //! Default constructor. Used for compatibility with containers
  FuseCFGEdge();

  //! Constructor
  FuseCFGEdge(Fuse* fuseAnalysis, PartEdgePtr pedge);

  // Wraps all the PartEdges in the given set with FuseCFGEdges and returns the set of these Edges
  static std::set<FuseCFGEdgePtr> parts2FuseCFGEdges(const std::set<PartEdgePtr>& pedges, Fuse* fuseAnalysis);

  //! Pretty string for Dot node labels, etc.
  std::string toString() const;
  //! String for debugging graphs
  std::string toStringForDebugging() const;
  //! String representation of this Edge's Fuse information
  std::string toStringFuse() const;
  //! ID to use for Dot, etc.
  std::string id() const;

  //! The source (beginning) CFG node
  FuseCFGNodePtr source() const;
  //! The target (ending) CFG node
  FuseCFGNodePtr target() const;

  //! The control flow condition that enables this edge
  EdgeConditionKind condition() const;

  //! The label of the case represented by an eckCaseLabel edge
  SgExpression* caseLabel() const;

  //! The expression of the computed goto represented by the eckArithmeticIf* conditions
  unsigned int computedGotoCaseIndex() const;

  //! The test or case key that is tested as a condition of this control flow edge
  SgExpression* conditionBasedOn() const;

  //! Variables going out of scope across this edge (not extensively tested)
  std::vector<SgInitializedName*> scopesBeingExited() const;
  //! Variables coming into scope across this edge (not extensively tested)
  std::vector<SgInitializedName*> scopesBeingEntered() const;

  // Compares the FuseCFGNodes
  bool operator==(const FuseCFGEdgePtr& that) const;
  bool operator< (const FuseCFGEdgePtr& that) const;
  bool operator!=(const FuseCFGEdgePtr& that) const;
  bool operator>=(const FuseCFGEdgePtr& that) const;
  bool operator<=(const FuseCFGEdgePtr& that) const;
  bool operator> (const FuseCFGEdgePtr& that) const;
}; // end FuseCFGEdge
} // namespace fuse

