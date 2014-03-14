#include "sage3basic.h"
#include "stx_analysis.h"
#include <map>
#include <typeinfo>
#include "sageInterface.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include "VirtualCFGIterator.h"


#include <boost/function.hpp>
#include <boost/bind.hpp>


using namespace std;
using namespace sight;

//namespace bll = boost::lambda;

namespace fuse {

DEBUG_LEVEL(stxAnalysisDebugLevel, 0);

/****************************************
 ***** Function structure detection *****
 ****************************************/

// Maps each function that doesn't have a body to a newly-created SgFunctionParameterList that represents its enty point
map<Function, CFGNode> Func2Entry;
// Maps each function that doesn't have a body to a newly-created SgFunctionDefinition that represents its enty point
map<Function, CFGNode> Func2Exit;

// Inverse mapping of Func2Entry
map<CFGNode, Function> Entry2Func;
// Inverse mapping of Func2Exit
map<CFGNode, Function> Exit2Func;

// Maps the synthesized entry point of a function with no definition to its corresponding exit point
map<CFGNode, CFGNode> Entry2Exit;  
// Maps the synthesized exit point of a function with no definition to its corresponding entry point
map<CFGNode, CFGNode> Exit2Entry;

// Flag that indicates whether the above maps have been initialized.
bool FuncEntryExit_initialized=false;

class UnknownSideEffectsAttribute : public AstAttribute {
  public:
  string toString() { return "UnknownSideEffectsAttribute"; }
};

class FuncEntryExitFunctor
{
  public:
  typedef void* result_type;
  
  void* operator()(SgNode* n) {
    // If this is a function declaration, AND
    if(SgFunctionDeclaration* decl=isSgFunctionDeclaration(n)) {
      Function func(decl);
      scope sFunc("FuncEntryExitFunctor", scope::medium, attrGE("stxAnalysisDebugLevel", 2));
       
      if(stxAnalysisDebugLevel()>=2) {
        dbg << "Func "<<func.get_name().getString()<<endl;
        dbg << "Type = "<<SgNode2Str(decl->get_type())<<endl;
        dbg << "Declaration = "<<func.get_declaration()<<"="<<SgNode2Str(decl)<<endl;
        dbg << "Definition = "<<(func.get_definition()? SgNode2Str(func.get_definition()): "NULL")<<endl;
        
        /*if(decl->get_definingDeclaration())
          dbg << "Has defining Declaration: def="<<isSgFunctionDeclaration(decl->get_definingDeclaration())->get_definition()<<endl;
        else
          dbg << "Has not defining Declaration: def="<<isSgFunctionDeclaration(decl->get_firstNondefiningDeclaration())->get_definition()<<endl;*/
      }
      
      if(// This is not a declaration defined in a templated class 
         // According to rose/src/backend/unparser/languageIndependenceSupport/modified_sage.C:1308
         // "It should be impossible to reach this code since SgTemplateInstantiationDefn is not a class, function or member function type"
         //!isSgTemplateInstantiationDefn(decl->get_parent()) &&
         // And this is not a declaration of a template function (we only care about the instantiations of such functions)
         !isSgTemplateFunctionDeclaration(decl) && 
         !isSgTemplateMemberFunctionDeclaration(decl)) {
        CFGNode Entry;
        CFGNode Exit;
         
        if(stxAnalysisDebugLevel()>=2) dbg << "Function "<<(Func2Entry.find(func)==Func2Entry.end()? "NOT": "")<<" Found\n";
        
        // If this function has no definition and we have not yet added it to the data structures, do so now
        if(func.get_definition()==NULL && Func2Entry.find(func)==Func2Entry.end()) {
          /*SgFunctionParameterList* params=SageBuilder::buildFunctionParameterList();
          params->set_parent(decl);
          Entry = CFGNode(params, 0);*/

          SgBasicBlock* body = SageBuilder::buildBasicBlock();
          SgFunctionDefinition* def = new SgFunctionDefinition(func.get_declaration(), body);
          def->setAttribute("fuse:UnknownSideEffects", new UnknownSideEffectsAttribute());
          body->set_parent(def);
          def->set_parent(decl);
          def->set_file_info(decl->get_file_info());
          Exit = CFGNode(def, 3);
          if(stxAnalysisDebugLevel()>=2) {
            dbg << "Creating function "<<func.get_name().getString()<<endl;
            dbg << "decl="<<decl<<"="<<SgNode2Str(decl)<<endl;
            dbg << "def="<<def<<"="<<SgNode2Str(def)<<endl;
          }

          if(stxAnalysisDebugLevel()>=3) {
            dbg << "func2 Function "<<func.get_name().getString()<<endl;

            for(back_CFGIterator it(def->cfgForEnd()); it!=back_CFGIterator::end(); it++) {
              dbg << "it="<<CFGNode2Str(*it)<<endl;
            }
          }
        // If this function has a definition
        } else {
          // The function's exit CFGNode
          Exit = CFGNode(func.get_definition(), 3);
        }

        // Since the function's definition now exists, find its entry point
        // Find the function's entry CFG node, which is the last SgFunctionParameterList node in the function body
        { scope siter("Iteration", scope::low, attrGE("stxAnalysisDebugLevel", 2));
        //for(back_CFGIterator it(func.get_definition()->cfgForEnd()); it!=back_CFGIterator::end(); it++) {
        for(CFGIterator it(func.get_definition()->cfgForBeginning()); it!=CFGIterator::end(); it++) {
          if(stxAnalysisDebugLevel()>=2) dbg << "    it="<<CFGNode2Str(*it)<<endl;
          // Look for the last SgFunctionParameterList node reachable from the start of the function
          if(isSgFunctionParameterList((*it).getNode())) {
            Entry = *it;
            //break;
          }
        }}
        assert(Entry.getNode());

        /*dbg << func.get_name().getString()<<"() Entry="<<CFGNode2Str(Entry)<<"(cg="<<(Entry.getNode()->get_file_info()->isCompilerGenerated())<<"), "<<
                                               "Exit="<<CFGNode2Str(Exit)<<"(cg="<<(Exit.getNode()->get_file_info()->isCompilerGenerated())<<")"<<endl;*/
        Func2Entry[func] = Entry;
        Func2Exit[func]  = Exit;

        Entry2Func[Entry] = func;
        Exit2Func[Exit]   = func;

        Entry2Exit[Entry] = Exit;
        Exit2Entry[Exit]  = Entry;
      }
    }
    return NULL;
  }
};

void initFuncEntryExit() {
  if(!FuncEntryExit_initialized) {
    NodeQuery::querySubTree(SageInterface::getProject(), FuncEntryExitFunctor());
    FuncEntryExit_initialized=true;

    if(stxAnalysisDebugLevel()>=3) {
      scope reg("", scope::medium, attrGE("stxAnalysisDebugLevel", 3));
      for(map<Function, CFGNode>::iterator i=Func2Entry.begin(); i!=Func2Entry.end(); i++) {
        dbg << i->first.get_name().getString() << " ==&gt; "<<endl;
        dbg << "entry:" << CFGNode2Str(i->second) << endl;
        dbg << " exit: "<< CFGNode2Str(Func2Exit[i->first]) << endl;
      }
    }
  }
}

// Accessor functions for the above maps
CFGNode getFunc2Entry(Function func) {
  initFuncEntryExit();
  if(Func2Entry.find(func) == Func2Entry.end()) cout << "ERROR: cannot find record for function "<<func.get_name().getString()<<endl;
  assert(Func2Entry.find(func) != Func2Entry.end());
  return Func2Entry[func];
}

CFGNode getFunc2Exit(Function func) {
  initFuncEntryExit();
  if(Func2Exit.find(func) == Func2Exit.end()) cout << "ERROR: cannot find record for function "<<func.get_name().getString()<<endl;
  assert(Func2Exit.find(func) != Func2Exit.end());
  return Func2Exit[func];
}

Function getEntry2Func(CFGNode entry) {
  initFuncEntryExit();
  if(Entry2Func.find(entry) == Entry2Func.end()) cout << "ERROR: cannot find record for function "<<CFGNode2Str(entry)<<endl;
  assert(Entry2Func.find(entry) != Entry2Func.end());
  return Entry2Func[entry];
}

Function getExit2Func(CFGNode exit) {
  initFuncEntryExit();
  if(Exit2Func.find(exit) == Exit2Func.end()) cout << "ERROR: cannot find record for function "<<CFGNode2Str(exit)<<endl;
  assert(Exit2Func.find(exit) != Exit2Func.end());
  return Exit2Func[exit];
}

bool isFuncEntry(CFGNode entry) { 
  initFuncEntryExit();
  if(Entry2Exit.find(entry) == Entry2Exit.end()) cout << "ERROR: cannot find record for node "<<CFGNode2Str(entry)<<endl;
  return Entry2Exit.find(entry) != Entry2Exit.end();
}

bool isFuncExit(CFGNode exit)  { 
  initFuncEntryExit();
  if(Exit2Entry.find(exit) == Exit2Entry.end()) cout << "ERROR: cannot find record for function "<<CFGNode2Str(exit)<<endl;
  return Exit2Entry.find(exit) != Exit2Entry.end();
}

CFGNode getEntry2Exit(CFGNode entry) {
  initFuncEntryExit();
  assert(isFuncEntry(entry));
  return Entry2Exit[entry];
}

CFGNode getExit2Entry(CFGNode exit) {
  initFuncEntryExit();
  assert(isFuncExit(exit));
  return Exit2Entry[exit];
}

/***************************************
 ***** function <-> call detection *****
 ***************************************/

// Maps each function to all the SgFunctionCallExps that call it.
map<Function, set<SgFunctionCallExp*> > func2AllCalls;
// Flag that indicates whether func2AllCalls has been initialized.
bool func2AllCalls_initialized=false;

// Given a function call, returns the set of all functions that it may invoke
set<Function> getAllCalleeFuncs(SgFunctionCallExp* call) {
  initFuncEntryExit();
  
  set<Function> callees;
  
  Function callee(call);
  // If the function being called is statically known
  if(callee.isKnown())
     callees.insert(call);
  // Otherwise, find all the functions with the same type as this function call.
  // They are all possible referents of the call
  // !!! NOTE: This should be updated to compute type compatibility rather than strict equality !!!
  else {
    for(map<Function, CFGNode>::iterator f=Func2Entry.begin(); f!=Func2Entry.end(); f++) {
      if(f->first.get_type() == call->get_function()->get_type())
        callees.insert(f->first);
    }
  }
  
  return callees;
}

class func2AllCallsFunctor
{
  public:
  typedef void* result_type;
  
  void* operator()(SgNode* n) {
    if(SgFunctionCallExp* call=isSgFunctionCallExp(n)) {
      set<Function> callees = getAllCalleeFuncs(call);
      for(set<Function>::iterator c=callees.begin(); c!=callees.end(); c++)
        func2AllCalls[*c].insert(call);
    }
    return NULL;
  }
};

// Determines the association between functions and their possible call sites.
void init_func2AllCalls()
{
  
  if(!func2AllCalls_initialized) {
    NodeQuery::querySubTree(SageInterface::getProject(), func2AllCallsFunctor());
    func2AllCalls_initialized=true;
    
    if(stxAnalysisDebugLevel()>=3) {
      scope reg("func2AllCalls", scope::medium, attrGE("stxAnalysisDebugLevel", 3));
      for(map<Function, set<SgFunctionCallExp*> >::iterator i=func2AllCalls.begin(); i!=func2AllCalls.end(); i++) {
        dbg << i->first.get_name().getString() << " =&gt; "<<endl;
        indent(attrGE("stxAnalysisDebugLevel", 1));
        for(set<SgFunctionCallExp*>::iterator j=i->second.begin(); j!=i->second.end(); j++)
          dbg << SgNode2Str(*j) << endl;
      }
    }
  }
}

// Returns the set of all the function calls that may call the given function
const set<SgFunctionCallExp*>& func2Calls(Function func)
{ 
  init_func2AllCalls();
  return func2AllCalls[func];
}

/*****************************
 ***** SyntacticAnalysis *****
 *****************************/

// the top level builder for MemLocObject from any SgNode

boost::shared_ptr<SyntacticAnalysis> SyntacticAnalysis::_instance;

SyntacticAnalysis* SyntacticAnalysis::instance()
{
  if(!_instance) _instance = boost::make_shared<SyntacticAnalysis>();
  return _instance.get();
}

// Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
ComposedAnalysisPtr SyntacticAnalysis::copy() { 
  if(!_instance) _instance = boost::make_shared<SyntacticAnalysis>();
  return _instance;
}

ValueObjectPtr SyntacticAnalysis::Expr2Val(SgNode* n, PartEdgePtr pedge)
{ return SyntacticAnalysis::Expr2ValStatic(n, pedge); }

ValueObjectPtr SyntacticAnalysis::Expr2ValStatic(SgNode* n, PartEdgePtr pedge)
{ return boost::make_shared<StxValueObject>(n); }

CodeLocObjectPtr SyntacticAnalysis::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge)
{ return SyntacticAnalysis::Expr2CodeLocStatic(n, pedge); }

CodeLocObjectPtr SyntacticAnalysis::Expr2CodeLocStatic(SgNode* n, PartEdgePtr pedge)
// GB: Not sure if this is correct since there may be multiple CFGNodes for each SgNode
{ return boost::make_shared<CodeLocObject>(pedge->source(), CFGNode(n, 0)); }

// Maps the given SgNode to an implementation of the MemLocObject abstraction.
MemRegionObjectPtr SyntacticAnalysis::Expr2MemRegion(SgNode* n, PartEdgePtr pedge)
{ return Expr2MemRegionStatic(n, pedge); }

MemRegionObjectPtr SyntacticAnalysis::Expr2MemRegionStatic(SgNode* n, PartEdgePtr pedge)
{ return boost::make_shared<StxMemRegionObject>(n); }

// Maps the given SgNode to an implementation of the MemLocObject abstraction.
MemLocObjectPtr SyntacticAnalysis::Expr2MemLoc(SgNode* n, PartEdgePtr pedge) {
  return Expr2MemLocStatic(n, pedge);
}
MemLocObjectPtr SyntacticAnalysis::Expr2MemLocStatic(SgNode* n, PartEdgePtr pedge) {
  StxMemRegionObjectPtr region = boost::make_shared<StxMemRegionObject>(n);
  // If this is an expression or named memory region, we create a MemLocObject with a 0 index
  // since the syntactic analysis only generates such regions for the SgNodes that mark the 
  // entire memory region in question rather than a sub-region (subsequent analyses do this to
  // get better precision)
  if(region->getType() == StxMemRegionType::expr || 
     region->getType() == StxMemRegionType::named)
    return boost::make_shared<MemLocObject>(region, boost::make_shared<StxValueObject>(SageBuilder::buildIntVal(0)), n);
  // If this is an unknown memory region, we have no idea where inside the region we may be.
  // As such, create a MemLocObject with an unconstrained index
  else
    return boost::make_shared<MemLocObject>(region, boost::make_shared<StxValueObject>((SgNode*)NULL), n);
}


// Detects declarations of global variables, stores them in globalDeclarations
set<SgVariableDeclaration*> SyntacticAnalysis::globalDeclarations;
void SyntacticAnalysis::initGlobalDeclarations() {
  static bool initialized = false;
  if(initialized) return;
  
  Rose_STL_Container<SgNode*> globalScopes = NodeQuery::querySubTree(SageInterface::getProject(), V_SgGlobal);
  for(Rose_STL_Container<SgNode*>::iterator gs=globalScopes.begin(); gs!=globalScopes.end(); gs++) {
  //SgGlobal* global = SageInterface::getFirstGlobalScope(SageInterface::getProject());
    assert(isSgGlobal(*gs));
    const SgDeclarationStatementPtrList& decls = isSgGlobal(*gs)->get_declarations();
    for(SgDeclarationStatementPtrList::const_iterator d=decls.begin(); d!=decls.end(); d++) {
      if(!(*d)->get_file_info()->isCompilerGenerated()) {
//        scope s(txt()<<"declaration: "<<SgNode2Str(*d)<<" parent="<<(*d)->get_parent(), scope::medium, attrGE("stxAnalysisDebugLevel", 3));

        if(isSgVariableDeclaration(*d)) {
          //dbg << "definition: "<<(isSgVariableDeclaration(*d)->get_definition()? SgNode2Str(isSgVariableDeclaration(*d)->get_definition()): "NULL")<<endl;
          if(stxAnalysisDebugLevel()>=3) {
            dbg << "begin="<<CFGNode2Str((*d)->cfgForBeginning())<<endl;
            dbg << "end="<<CFGNode2Str((*d)->cfgForEnd())<<endl;
          }
          globalDeclarations.insert(isSgVariableDeclaration(*d));
        }
      }
    }
  }
  
  initialized = true;
}

// Return the anchor Parts of a given function
std::set<PartPtr> SyntacticAnalysis::GetStartAStates_Spec()
{ 
  // Return the entry points into all the global VariableDeclarations
  set<PartPtr> startStates;
  for(set<SgVariableDeclaration*>::iterator d=SyntacticAnalysis::globalDeclarations.begin(); d!=SyntacticAnalysis::globalDeclarations.end(); d++)
    startStates.insert(makePtr<StxPart>((*d)->cfgForBeginning(), this, filter));
  
  // If there are no global VariableDeclarations, the analysis entry points are the entries
  // into the non-static functions
  if(startStates.size()==0)
    addFunctionEntries(startStates, this);
  
  return startStates;
}

// Returns whether the given function can be called from outside the current compilation unit
bool isExternallyCallable(const Function& func) {
  return !SageInterface::isStatic(func.get_declaration()) &&
         !func.get_declaration()->get_file_info()->isCompilerGenerated() && 
         func.get_definition()->getAttribute("fuse:UnknownSideEffects")==NULL;
}

// Adds the entry points into all the non-static functions (can be called from the outside) to 
// the given set
template <class ArgPartPtr>
void SyntacticAnalysis::addFunctionEntries(set<ArgPartPtr>& states, SyntacticAnalysis* analysis) {
  initFuncEntryExit();
  for(map<Function, CFGNode>::iterator f=Func2Entry.begin(); f!=Func2Entry.end(); f++) {
/*      dbg << f->first.get_name().getString()<<"() declaration="<<f->first.get_declaration()<<"="<<CFGNode2Str(f->first.get_declaration())<<", static="<<SageInterface::isStatic(f->first.get_declaration())<<", compgen="<<f->first.get_declaration()->get_file_info()->isCompilerGenerated()<<endl;
      dbg << f->first.get_name().getString()<<"() definition="<<f->first.get_definition()<<"="<<CFGNode2Str(f->first.get_definition())<<", compgen="<<f->first.get_definition()->get_file_info()->isCompilerGenerated()<<", unknown="<<f->first.get_definition()->getAttribute("fuse:UnknownSideEffects")<<endl;*/
    if(isExternallyCallable(f->first))
      states.insert(makePtr<StxPart>(f->second, analysis, analysis->filter));
  }
}

set<PartPtr> SyntacticAnalysis::GetEndAStates_Spec()
{
  // Collect all the return statements
  /*list<PartPtr> endStates;
  GetReturnStmts grs(this);
  grs.runAnalysis();
  for(set<PartPtr>::iterator r=grs.returns.begin(); r!=grs.returns.end(); r++)
    endStates.push_back(*r);*/
  
  // The CFGNodes that denote the stard and end of the main() function
  /*CFGNode mainStart = getFuncStartCFG(
                           SageInterface::findMain(SageInterface::getFirstGlobalScope(SageInterface::getProject()))->get_definition());
  CFGNode mainEnd = getFuncEndCFG(
                           SageInterface::findMain(SageInterface::getFirstGlobalScope(SageInterface::getProject()))->get_definition());
  
  set<PartPtr> endStates;          
  
  // Find all the return statements in main() and add them to endStates
  for(VirtualCFG::dataflowIterator df(mainStart, mainEnd); df!=VirtualCFG::iterator::end(); df++) {
    if(SgReturnStmt* ret = isSgReturnStmt((*df).getNode()))
      endStates.insert(makePtr<StxPart>(ret, this, filter));
  }
  
  // Add main's ending point
  endStates.insert(makePtr<StxPart>(mainEnd, this, filter));*/
  
  // Return the entry points of all the non-static functions
  set<PartPtr> endStates;
  /*Function main(SageInterface::findMain(SageInterface::getFirstGlobalScope(SageInterface::getProject()))->get_definition());
  endStates.insert(makePtr<StxPart>(getFunc2Exit(main), this, filter));*/
  
  initFuncEntryExit();
  scope s("EndAStates", attrGE("stxAnalysisDebugLevel", 3));
  for(map<Function, CFGNode>::iterator f=Func2Exit.begin(); f!=Func2Exit.end(); f++) {
    /*if(!SageInterface::isStatic(f->first.get_declaration()) &&
       !f->first.get_declaration()->get_file_info()->isCompilerGenerated()) {*/
    if(isExternallyCallable(f->first)) {
      if(stxAnalysisDebugLevel()>3) dbg << CFGNode2Str(f->second)<<"()"<<endl;
      endStates.insert(makePtr<StxPart>(f->second, this, filter));
    }
  }
  
  return endStates;
}

/**********************
 ***** PARTITIONS *****
 **********************/

// A NULL CFGNode that is used as a wild-card for termination points of edges to/from anywhere
CFGNode NULLCFGNode;

StxPartPtr NULLStxPart;
StxPartEdgePtr NULLStxPartEdge;

/**************************
 ***** StxFuncContext *****
 **************************/
//StxFuncContext::StxFuncContext(Function func) : func(func) {}

StxFuncContext::StxFuncContext(CFGNode n) :
  func(Function(SageInterface::getEnclosingFunctionDeclaration(n.getNode()))),
  n(n)
{ }

// Returns a list of PartContextPtr objects that denote more detailed context information about
// this PartContext's internal contexts. If there aren't any, the function may just return a list containing
// this PartContext itself.
list<PartContextPtr> StxFuncContext::getSubPartContexts() const { 
  std::list<PartContextPtr> listOfMe;
  listOfMe.push_back(makePtr<StxFuncContext>(n));
  return listOfMe;
}

bool StxFuncContext::operator==(const PartContextPtr& that_arg) const
{
  //const StxFuncContext& that = dynamic_cast<const StxFuncContext&>(that_arg);
  const StxFuncContextPtr that = dynamicConstPtrCast<StxFuncContext>(that_arg);
  //dbg << "StxFuncContext::operator==: "<<const_cast<StxFuncContext*>(this)->str()<<" eq "<<const_cast<StxFuncContext&>(that).str()<<" = "<<(func==that.func)<<endl;
  return func==that->func;
}

bool StxFuncContext::operator< (const PartContextPtr& that_arg) const
{
  //const StxFuncContext& that = dynamic_cast<const StxFuncContext&>(that_arg);
  const StxFuncContextPtr that = dynamicConstPtrCast<StxFuncContext>(that_arg);
  //dbg << "StxFuncContext::operator<: "<<const_cast<StxFuncContext*>(this)->str()<<" lt "<<const_cast<StxFuncContext&>(that).str()<<" = "<<(func<that.func)<<endl;
  return func<that->func;
}

std::string StxFuncContext::str(std::string indent) {
  ostringstream oss;
  oss << "[StxFuncContext: "<<func.str()<<"]";
  return oss.str();
}

/*******************
 ***** StxPart *****
 *******************/

// Returns a shared pointer to this of type StxPartPtr;
StxPartPtr StxPart::get_shared_this()
{ return dynamicPtrCast<StxPart>(makePtrFromThis(shared_from_this())); }

/* // Returns true if the given edge is from the start of a short-circuit operation (|| and &&) to its end 
bool isShortCircuitEdge(CFGEdge edge) {
  return ((isSgAndOp(edge.source().getNode()) && isSgAndOp(edge.target().getNode())) ||
          (isSgOrOp(edge.source().getNode())  && isSgOrOp(edge.target().getNode()))) &&
         edge.source().getIndex()==1 && edge.target().getIndex()==2;
}*/

void makeClosureDF_rec(CFGPath path, // The current set of CFG paths
        set<CFGPath>& allPaths, // All the paths that make up the closure
        vector<CFGEdge> (CFGNode::*closure)() const, // find successor edges from a node, CFGNode::outEdges() for example
        CFGNode (CFGPath::*otherSide)() const, // node from the other side of the path: CFGPath::target()
        CFGPath (*merge)(const CFGPath&, const CFGPath&),  // merge two paths into one
        bool (*filter) (CFGNode))   // filter function 
{
  if(stxAnalysisDebugLevel()>=3) dbg << "makeClosureDF_rec: path: "<<CFGNode2Str(path.source())<<" ==&gt; "<<CFGNode2Str(path.target())<<endl;
  
  // If the edge of the current path is not interesting
  if(!filter((path.*otherSide)())) {
    // Recurse to find its extensions that may be interesting
    vector<CFGEdge> extensions = ((path.*otherSide)().*closure)(); 
    //dbg << "otherSide="<<CFGNode2Str((path.*otherSide)())<<endl;
    for(vector<CFGEdge>::iterator e=extensions.begin(); e!=extensions.end(); e++) {
      if(stxAnalysisDebugLevel()>=3) dbg << "extension "<<CFGNode2Str(e->source())<<" ==&gt; "<<CFGNode2Str(e->target())<<endl;

      /* // Skip edges from the start of a short-circuit operation (|| and &&) to its end
      if(isShortCircuitEdge(*e)) { continue; }*/
      
      indent ind(attrGE("stxAnalysisDebugLevel", 1));
      CFGPath extension = (*merge)(path, *e);
      // Extend path with e to create the full extension of path
      makeClosureDF_rec(extension, allPaths, closure, otherSide, merge, filter);
    }
  } else {
    if(stxAnalysisDebugLevel()>=3) dbg << "Interesting\n";
    // We've found an interesting extension, record it.
    allPaths.insert(path);
  }
}

// XXX: This code is duplicated from frontend/SageIII/virtualCFG/virtualCFG.C
// Make a set of raw CFG edges closure. Raw edges may have src and dest CFG nodes which are to be filtered out. 
// The method used is to connect them into CFG paths so src and dest nodes of each path are interesting, skipping intermediate filtered nodes)
map<StxPartEdgePtr, bool> makeClosureDF(const vector<CFGEdge>& orig, // raw in or out edges to be processed
                                        vector<CFGEdge> (CFGNode::*closure)() const, // find successor edges from a node, CFGNode::outEdges() for example
                                        CFGNode (CFGPath::*otherSide)() const, // node from the other side of the path: CFGPath::target()
                                        CFGPath (*merge)(const CFGPath&, const CFGPath&),  // merge two paths into one
                                        bool (*filter) (CFGNode),   // filter function 
                                        ComposedAnalysis* analysis)
{
  scope reg("makeClosureDF", scope::medium, attrGE("stxAnalysisDebugLevel", 3));
  indent ind;
  set<CFGPath> allPaths;
  for(vector<CFGEdge>::const_iterator e=orig.begin(); e!=orig.end(); e++) {
    if(stxAnalysisDebugLevel()>=3) dbg << "edge "<<CFGNode2Str(e->source())<<" ==&gt; "<<CFGNode2Str(e->target())<<endl;
    /* // Skip edges from the start of a short-circuit operation (|| and &&) to its end
    if(isShortCircuitEdge(*e)) { continue; }*/
    makeClosureDF_rec(*e, allPaths, closure, otherSide, merge, filter);
  }
  
  // Maps edges to bools. A map is used to enable efficient lookups to avoid inserting duplicate edges, 
  // which may happen in situations like an if statement with empty true and false bodies.
  map<StxPartEdgePtr, bool> edges;
  
  for (set<CFGPath>::iterator i = allPaths.begin(); i != allPaths.end(); ++i) {
    // Only if the end node of the path is interesting
    //if (((*i).*otherSide)().isInteresting())
    if (filter(((*i).*otherSide)())) {
      //edges.push_back(/*boost::static_pointer_cast<PartEdge>(*/boost::make_shared<StxPartEdge>(*i, filter)/*)*/);
      //edges.push_back(makePtr<StxPartEdge>(*i, analysis, filter));
      StxPartEdgePtr newEdge = makePtr<StxPartEdge>(*i, analysis, filter);
      if(stxAnalysisDebugLevel()>=3) dbg << "newEdge="<<newEdge->str()<<endl;
      if(edges.find(newEdge) == edges.end()) edges[newEdge] = true;
    }
  }
  //dbg << "makeClosure done: #edges=" << edges.size() << endl;
  //for(vector<DataflowEdge>::iterator e=edges.begin(); e!=edges.end(); e++)
  //    printf("Current Node %p<%s | %s>\n", e.target().getNode(), e.target().getNode()->unparseToString().c_str(), e.target().getNode()->class_name().c_str());
  //for (list<StxPartEdgePtr>::iterator i = edges.begin(); i != edges.end(); ++i) {
  
  // Make sure that for each edge either the source or the target is interesting
  for (map<StxPartEdgePtr, bool>::iterator i = edges.begin(); i != edges.end(); ++i) {
    StxPartEdgePtr edge = i->first;
    assert(edge->source()->filterAny(filter)  || 
                edge->target()->filterAny(filter)); // at least one node is interesting
  }
  return edges;
}

map<StxPartEdgePtr, bool> StxPart::getOutEdges()
{
//  scope sRet(txt()<<"StxPart::getOutEdges() ret="<<CFGNode2Str(n), scope::medium, attrGE("stxAnalysisDebugLevel", 2));
  map<StxPartEdgePtr, bool> vStx;
  SgFunctionCallExp* call;
  
  // If this is the end of a SgVariableDeclaration of a global variable
  if(isSgVariableDeclaration(n.getNode()) && n.getIndex()==1 && isSgGlobal(n.getNode()->get_parent())) {
    // The successors are the entry points of all the non-static functions
    assert(dynamic_cast<SyntacticAnalysis*>(analysis));
    //SyntacticAnalysis::addFunctionEntries(v, dynamic_cast<SyntacticAnalysis*>(analysis));
    set<StxPartPtr> entries;
    SyntacticAnalysis::addFunctionEntries(entries, dynamic_cast<SyntacticAnalysis*>(analysis));
    for(set<StxPartPtr>::iterator e=entries.begin(); e!=entries.end(); e++)
      vStx[makePtr<StxPartEdge>(n, (*e)->n, analysis)] = true;
  // If current node is a function call, connect the call to the SgFunctionParameterList of the called function.
  } else if((call = isSgFunctionCallExp(n.getNode())) && n.getIndex()==2) {
    set<Function> callees = getAllCalleeFuncs(call);
    
    if(stxAnalysisDebugLevel()>=2) {
      dbg << "type = "<<SgNode2Str(isSgFunctionCallExp(n.getNode())->get_type())<<", funcCall->get_function()="<<(isSgFunctionCallExp(n.getNode())->get_function()? SgNode2Str(isSgFunctionCallExp(n.getNode())->get_function()): "NULL")<<endl;
      dbg << "function = "<<SgNode2Str(isSgFunctionCallExp(n.getNode())->get_function())<<" function type="<<SgNode2Str(isSgFunctionCallExp(n.getNode())->get_function()->get_type())<<endl;
      scope sCallees("Callees", scope::low);
      for(set<Function>::iterator c=callees.begin(); c!=callees.end(); c++)
        dbg << c->str()<<" declaration="<<c->get_declaration()<<"="<<SgNode2Str(c->get_declaration())<<endl;
    }
    
    for(set<Function>::iterator c=callees.begin(); c!=callees.end(); c++)
      vStx[makePtr<StxPartEdge>(n, getFunc2Entry(*c), analysis)] = true;
    
    // If the callee function has a definition, connect this function call directly to the function's entry point
    /*if(callee.get_definition()) {
      assert(callee.get_params());
      //vStx[makePtr<StxPartEdge>(n, CFGNode(callee.get_params(), 1), analysis)] = true;
      vStx[makePtr<StxPartEdge>(n, getFunc2Entry(callee), analysis)] = true;
      
      {
        dbg << "The successors of call "<<CFGNode2Str(n)<<endl;
        {indent ind;
        
        map<StxPartEdgePtr, bool> outvStx = makeClosureDF(n.outEdges(), &CFGNode::outEdges, &CFGPath::target, &mergePaths, filter, analysis);
        for(map<StxPartEdgePtr, bool>::iterator i=outvStx.begin(); i!=outvStx.end(); i++) {
          dbg << i->first->source()->str() << " =&gt; "<< i->first->target()->str()<<endl;
        }}
        
        / *dbg << "The predecessors of call "<<CFGNode2Str(n)<<endl;
        {indent ind;
        
        map<StxPartEdgePtr, bool> invStx = makeClosureDF(n.inEdges(), &CFGNode::inEdges, &CFGPath::source, &mergePathsReversed, filter, analysis);
        for(map<StxPartEdgePtr, bool>::iterator i=invStx.begin(); i!=invStx.end(); i++) {
          dbg << i->first->source()->str() << " =&gt; "<< i->first->target()->str()<<endl;
        }}* /
      }
    / * // Otherwise, just connect the call to the next state in its own function
       // !!! GB 2013-05-11 - NOTE: WE'LL NEED TO DO BETTER THAN THIS TO MAKE SURE THAT WE DEAL SOUNDLY WITH SEPARATE COMPILATION
     * /
    // Otherwise, create synthetic entry and exit points for the routine and connect the call to this entry
    } else {
      //vStx[makePtr<StxPartEdge>(n, CFGNode(call, 3), analysis)] = true;
      vStx[makePtr<StxPartEdge>(n, getFunc2Entry(callee), analysis)] = true;
    }*/
  // If current node is the end of a function definition, connect it to all the calls of this function
  // !!! NOTE: we should be connecting it to all the function calls that match the calling signature
  //} else if(/*SgFunctionDefinition* def = */isSgFunctionDefinition(n.getNode())) {
  } else if(isFuncExit(n)) {
    Function func= getExit2Func(n);
  
    // If this is the synthesized exit node a function without a body
    /*if(isFuncExit(n)) func = getExit2Func(n);
    else                   func = Function(def);*/
    if(stxAnalysisDebugLevel()>=2) dbg << "Definition n="<<CFGNode2Str(n)<<" func="<<func.get_name().getString()<<" isFuncExit(n)="<<isFuncExit(n)<<endl;
    
    const set<SgFunctionCallExp*>& calls = func2Calls(func);
    if(stxAnalysisDebugLevel()>=2) dbg << "#calls="<<calls.size()<<" Connecting n="<<CFGNode2Str(n)<<endl;
    indent ind(attrGE("stxAnalysisDebugLevel", 2));
    for(set<SgFunctionCallExp*>::const_iterator c=calls.begin(); c!=calls.end(); c++) {
      CFGNode callNode(*c, 3);
      vStx[makePtr<StxPartEdge>(n, callNode, analysis, filter)]=1;
      
      /*dbg << "To the successors of call "<<CFGNode2Str(callNode)<<endl;
      indent ind;
      // Connect the SgFunctionDefinition to the nodes that follow each call to it, using makeClosureDF() to skip
      // over any nodes that are filtered out.
      map<StxPartEdgePtr, bool> outvStx = makeClosureDF(callNode.outEdges(), &CFGNode::outEdges, &CFGPath::target, &mergePaths, filter, analysis);
      for(map<StxPartEdgePtr, bool>::iterator i=outvStx.begin(); i!=outvStx.end(); i++) {
        //dbg << i->first->source()->str() << " =&gt; "<< i->first->target()->str()<<endl;
        vStx[makePtr<StxPartEdge>(n, i->first->stxTarget()->n, analysis, filter)]=1;
      }*/
    }
  // If the current node is a return statement, connect it to the function's exit SgFunctionDefinition node
  } else if(SgReturnStmt* ret = isSgReturnStmt(n.getNode())) {
    Function func(SageInterface::getEnclosingFunctionDeclaration(ret));
    if(stxAnalysisDebugLevel()>=2) {
      dbg << "returning from func="<<func.str()<<endl;
      dbg << "Exit node="<<CFGNode2Str(getFunc2Exit(func))<<endl;
    }
    vStx[makePtr<StxPartEdge>(n, getFunc2Exit(func), analysis)] = true;
  //} else if(isSgFunctionParameterList(n.getNode())) {
  } else if(isFuncEntry(n)) {
    // If this is the synthesized entry node to a function without a body, return the edge to its corresponding exit node
    /*if(isFuncEntry(n)) {
      vStx[makePtr<StxPartEdge>(n, getEntry2Exit(n),  analysis)] = true;
      return vStx;
    } else*/
      return makeClosureDF(n.outEdges(), &CFGNode::outEdges, &CFGPath::target, &mergePaths, filter, analysis);
  } else {
    return makeClosureDF(n.outEdges(), &CFGNode::outEdges, &CFGPath::target, &mergePaths, filter, analysis);
  }
//  dbg << "#vStx="<<vStx.size()<<endl;
  return vStx;
}

list<PartEdgePtr> StxPart::outEdges() {
  ostringstream oss; 
  //dbg << "n=>"<<CFGNode2Str(n)<<endl;
  //scope reg(txt()<<"StxPart::outEdges() part="<<str(), scope::medium, attrGE("stxAnalysisDebugLevel", 2));
  map<StxPartEdgePtr, bool> vStx = getOutEdges();

  list<PartEdgePtr> v;
  for(map<StxPartEdgePtr, bool>::iterator i=vStx.begin(); i!=vStx.end(); i++)
    v.push_back(dynamicPtrCast<PartEdge>(i->first));
  
//  dbg << "#v="<<v.size()<<endl;
  return v;
}

list<StxPartEdgePtr> StxPart::outStxEdges() {
  map<StxPartEdgePtr, bool> vStx = getOutEdges();
  list<StxPartEdgePtr> v;
  for(map<StxPartEdgePtr, bool>::iterator i=vStx.begin(); i!=vStx.end(); i++)
    v.push_back(i->first);
  return v;
}

map<StxPartEdgePtr, bool> StxPart::getInEdges()
{
  map<StxPartEdgePtr, bool> vStx;
  SgFunctionCallExp* call;

  // If current node is the return side of a function call, connect the call to the exit point of the called function.
  if((call = isSgFunctionCallExp(n.getNode())) && n.getIndex()==3) {
    Function callee(call);
    
    if(stxAnalysisDebugLevel()>=2) dbg << "StxPart::getInEdges() Return side of Call: callee="<<callee.str()<<" known="<<callee.isKnown()<<endl;
    
    // If the function is known
    if(callee.isKnown()) {
      if(stxAnalysisDebugLevel()>=2) dbg << "exit="<<CFGNode2Str(getFunc2Exit(callee))<<endl;
      vStx[makePtr<StxPartEdge>(getFunc2Exit(callee), n, analysis)] = true;
    // Otherwise, find all the functions with the same type as this function call.
    // They are all possible referents of the call
    // !!! NOTE: This should be updated to compute type compatibility rather than strict equality !!!
    } else {
      for(map<Function, CFGNode>::iterator f=Func2Exit.begin(); f!=Func2Exit.end(); f++) {
        if(f->first.get_type() == isSgFunctionCallExp(n.getNode())->get_function()->get_type())
          vStx[makePtr<StxPartEdge>(f->second, n, analysis)] = true;
      }
    }
    
  // If the current Node is the exit point of a function
  } else if(isFuncExit(n)) {
    Function func = getExit2Func(n);
    if(stxAnalysisDebugLevel()>=2) dbg << "Function Exit n="<<CFGNode2Str(n)<<" func="<<func.get_name().getString()<<endl;
    
    // Connect it to the immediately preceding CFGNode
    vStx = makeClosureDF(n.inEdges(), &CFGNode::inEdges, &CFGPath::source, &mergePathsReversed, filter, analysis);
    
    if(stxAnalysisDebugLevel()>=2) dbg << "-------------#vStx="<<vStx.size()<<"---------------------"<<endl;
    
    // Also connect it to all the SgReturnStmts in the function
    for(CFGIterator it(getFunc2Entry(func)); it!=CFGIterator::end(); it++) {
      if(isSgReturnStmt(it->getNode()) && it->getIndex()==1)
        vStx[makePtr<StxPartEdge>(*it, n, analysis)] = true;
    }
    if(stxAnalysisDebugLevel()>=2) dbg << "-------------#vStx="<<vStx.size()<<"---------------------"<<endl;
  // If the current node is the entry point of a function
  } else if(isFuncEntry(n)) {
    Function func = getEntry2Func(n);
  
    if(stxAnalysisDebugLevel()>=2) dbg << "Function Entry n="<<CFGNode2Str(n)<<" func="<<func.get_name().getString()<<endl;
    
    const set<SgFunctionCallExp*>& calls = func2Calls(func);
    if(stxAnalysisDebugLevel()>=2) dbg << "#calls="<<calls.size()<<" Connecting n="<<CFGNode2Str(n)<<endl;
    indent ind(attrGE("stxAnalysisDebugLevel", 2));
    for(set<SgFunctionCallExp*>::const_iterator c=calls.begin(); c!=calls.end(); c++) {
      CFGNode callNode(*c, 2);
      vStx[makePtr<StxPartEdge>(callNode, n, analysis, filter)]=1;
    }
    
    // If this function can be called from the outside, add incoming edges from all the global
    // declarations
    if(isExternallyCallable(func)) {
      // Return the entry points into all the global VariableDeclarations
      set<PartPtr> startStates;
      for(set<SgVariableDeclaration*>::iterator d=SyntacticAnalysis::globalDeclarations.begin(); d!=SyntacticAnalysis::globalDeclarations.end(); d++)
        vStx[makePtr<StxPartEdge>((*d)->cfgForEnd(), n, analysis, filter)]=1;
    }
  } else {
    // If this is the starting point of a declaration of a global variable, do not add any incoming edges
    if(isSgVariableDeclaration(n.getNode()) && n.getIndex()==0 &&
       SyntacticAnalysis::globalDeclarations.find(isSgVariableDeclaration(n.getNode())) != SyntacticAnalysis::globalDeclarations.end())
    {
      if(stxAnalysisDebugLevel()>=2) dbg << "Beginning of declaration of global variable"<<endl;
      return vStx;
    } else {
      if(stxAnalysisDebugLevel()>=2) dbg << "Internal Node"<<endl;
      return makeClosureDF(n.inEdges(), &CFGNode::inEdges, &CFGPath::source, &mergePathsReversed, filter, analysis);
    }
  }
  return vStx;
}

list<PartEdgePtr> StxPart::inEdges() {
  ostringstream oss; 
//  scope reg(txt()<<"StxPart::inEdges() part="<<str(), scope::medium, attrGE("stxAnalysisDebugLevel", 2));
  map<StxPartEdgePtr, bool> vStx = getInEdges();
 
  if(stxAnalysisDebugLevel()>=2) dbg <<"#vStx="<<vStx.size()<<endl;
  list<PartEdgePtr> v;
  for(map<StxPartEdgePtr, bool>::iterator i=vStx.begin(); i!=vStx.end(); i++)
    v.push_back(dynamicPtrCast<PartEdge>(i->first));
  return v;
}

list<StxPartEdgePtr> StxPart::inStxEdges() {
  map<StxPartEdgePtr, bool> vStx = getInEdges();
  list<StxPartEdgePtr> v;
  for(map<StxPartEdgePtr, bool>::iterator i=vStx.begin(); i!=vStx.end(); i++)
    v.push_back(i->first);
  return v;
}

set<CFGNode> StxPart::CFGNodes() const
{
  set<CFGNode> v;
  v.insert(n);
  return v;
}

// If this Part corresponds to a function call/return, returns the set of Parts that contain
// its corresponding return/call, respectively.
set<PartPtr> StxPart::matchingCallParts() const
{
  set<PartPtr> ret;

  if(isSgFunctionCallExp(n.getNode()) && n.getIndex()==2)
    ret.insert(makePtr<StxPart>(CFGNode(n.getNode(), 3), analysis));
  else if(isSgFunctionCallExp(n.getNode()) && n.getIndex()==3)
    ret.insert(makePtr<StxPart>(CFGNode(n.getNode(), 2), analysis));

  return ret;
}

/*// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    Part that denotes a subset of A (the function is called on this part), 
//    it returns a list of Parts that partition O.
std::list<PartPtr> StxPart::getOperandPart(SgNode* anchor, SgNode* operand)
{
  list<PartPtr> l;
  l.push_back(makePtr<StxPart>(operand->cfgForEnd(), analysis));
  return l;
}*/

/*class NULLCFGNode : public CFGNode {
  public:
  NULLCFGNode() : CFGNode(SageInterface::getProject(), -1) { }
};
*/
CFGNode getCFGNode() {
  static SgNode* sgn = NULL;
  if(!sgn) sgn = SageBuilder::buildNullStatement();
  CFGNode n(sgn, 0);
  return n;
}
bool isNULLCFGNode(CFGNode n) { return isSgNullStatement(n.getNode()); }



// Returns a PartEdgePtr, where the source is a wild-card part (NULLPart) and the target is this Part
PartEdgePtr StxPart::inEdgeFromAny()
{ return makePtr<StxPartEdge>(getCFGNode(), n, analysis); } ///*NULLCFGNode*/SageInterface::getGlobalScope(n.getNode())->cfgForBeginning(), n); }

// Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
PartEdgePtr StxPart::outEdgeToAny()
{ return makePtr<StxPartEdge>(n, getCFGNode(), analysis); } ///*NULLCFGNode*/SageInterface::getGlobalScope(n.getNode())->cfgForEnd()); }

bool StxPart::equal(const PartPtr& o) const
{
  /*assert(boost::dynamic_pointer_cast<StxPart>(o));
  return n == boost::dynamic_pointer_cast<StxPart>(o)->n;*/
  assert(dynamicPtrCast<StxPart>(o).get());
  return n == dynamicPtrCast<StxPart>(o)->n;
}

bool StxPart::less(const PartPtr& o) const
{
  /*assert(boost::dynamic_pointer_cast<StxPart>(o));
  return n < boost::dynamic_pointer_cast<StxPart>(o)->n;*/
  assert(dynamicPtrCast<StxPart>(o).get());
  return n < dynamicPtrCast<StxPart>(o)->n;
}

std::string StxPart::str(std::string indent)
{
  ostringstream oss;
  if(isNULLCFGNode(n.getNode())) oss << "[*]";
  else oss << CFGNode2Str(n);//", analysis="<<analysis<<"]";
  return oss.str();
}

/***********************
 ***** StxPartEdge *****
 ***********************/

PartPtr StxPartEdge::source() const {
  return stxSource();
}

StxPartPtr StxPartEdge::stxSource() const {
  if(isNULLCFGNode(p.source().getNode())) return NULLPart;
  else return makePtr<StxPart>(p.source(), analysis, filter);
}

PartPtr StxPartEdge::target() const { 
  return stxTarget();
}

StxPartPtr StxPartEdge::stxTarget() const { 
  if(isNULLCFGNode(p.target().getNode())) return NULLPart;
  else return makePtr<StxPart>(p.target(), analysis, filter);
}

// Let A={ set of execution prefixes that terminate at the given anchor SgNode }
// Let O={ set of execution prefixes that terminate at anchor's operand SgNode }
// Since to reach a given SgNode an execution must first execute all of its operands it must
//    be true that there is a 1-1 mapping m() : O->A such that o in O is a prefix of m(o).
// This function is the inverse of m: given the anchor node and operand as well as the
//    PartEdge that denotes a subset of A (the function is called on this PartEdge), 
//    it returns a list of PartEdges that partition O.
std::list<PartEdgePtr> StxPartEdge::getOperandPartEdge(SgNode* anchor, SgNode* operand)
{
  // Operand precedes anchor in the CFG, either immediately or at some distance. As such, the edge
  // we're looking for is not necessarily the edge from operand to anchor but rather the first
  // edge along the path from operand to anchor. Since operand is part of anchor's expression
  // tree we're guaranteed that there is only one such path.
  CFGNode opCFG = operand->cfgForEnd();
  //dbg << "opCFG="<<CFGNode2Str(opCFG)<<endl;
  StxPart opPart(opCFG, analysis);
  assert(opPart.outEdges().size()==1);
  list<PartEdgePtr> l;
  StxPartPtr partTarget = (*(opPart.outStxEdges().begin()))->target();
  assert(partTarget);
  assert(partTarget->n.getNode());
  l.push_back(makePtr<StxPartEdge>(opCFG, partTarget->n, analysis));
  return l;
}

// If the source Part corresponds to a conditional of some sort (if, switch, while test, etc.)
// it must evaluate some predicate and depending on its value, continue execution along one of the
// outgoing edges. The value associated with each outgoing edge is fixed and known statically.
// getPredicateValue() returns the value associated with this particular edge. Since a single 
// Part may correspond to multiple CFGNodes getPredicateValue() returns a map from each CFG node
// within its source part that corresponds to a conditional to the value of its predicate along 
// this edge.
map<CFGNode, boost::shared_ptr<SgValueExp> > StxPartEdge::getPredicateValue()
{
  CFGNode cn = p.source();
  
  map<CFGNode, boost::shared_ptr<SgValueExp> > pv;
       if(p.condition() == eckTrue)  pv[cn] = boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(true));
  else if(p.condition() == eckFalse) pv[cn] = boost::shared_ptr<SgValueExp>(SageBuilder::buildBoolValExp(false));
  else if(p.condition() == eckCaseLabel) {
    SgValueExp* val = getSGValueExp(p.caseLabel());
    if(val==NULL) cout << "p.caseLabel()="<<SgNode2Str(p.caseLabel())<<endl;
    assert(val);
    pv[cn] = boost::shared_ptr<SgValueExp>(val);
  }
  
  return pv;
}

bool StxPartEdge::equal(const PartEdgePtr& o) const
{
  assert(dynamicPtrCast<StxPartEdge>(o).get());
  /*dbg << "StxPartEdge::operator<("<<(p.source() == dynamicPtrCast<StxPartEdge>(o)->p.source() &&
         p.target() == dynamicPtrCast<StxPartEdge>(o)->p.target())<<endl; //(p == dynamicPtrCast<StxPartEdge>(o)->p)<<endl;
  dbg << "---- p="<<CFGPath2Str(p)<<endl;
  dbg << "---- dynamicPtrCast<StxPartEdge>(o)->p"<<CFGPath2Str(dynamicPtrCast<StxPartEdge>(o)->p)<<endl;*/
  //return p == dynamicPtrCast<StxPartEdge>(o)->p;
  // Since is the possible to create p either from makeClosureDF() or from its source/target CFGNode pair, we compare
  // paths in terms of just their source/target CFGNodes
  return p.source() == dynamicPtrCast<StxPartEdge>(o)->p.source() &&
         p.target() == dynamicPtrCast<StxPartEdge>(o)->p.target();
}

bool StxPartEdge::less(const PartEdgePtr& o) const
{
  assert(dynamicPtrCast<StxPartEdge>(o).get());
  /*dbg << "StxPartEdge::operator<(source="<<CFGNode2Str(p.source())<<
                                 ", o.source="<<CFGNode2Str(dynamicPtrCast<StxPartEdge>(o)->p.source())<<",\n"<<
                                     "target="<<CFGNode2Str(p.target())<<
                                 ", o.target="<<CFGNode2Str(dynamicPtrCast<StxPartEdge>(o)->p.target())<<",\n"<<
          ", source: < "<<(p.source() < dynamicPtrCast<StxPartEdge>(o)->p.source())<<" == "<<(p.source() == dynamicPtrCast<StxPartEdge>(o)->p.source())<<"\n"<<
          ", target: < "<<(p.target() < dynamicPtrCast<StxPartEdge>(o)->p.target())<<" == "<<(p.target() == dynamicPtrCast<StxPartEdge>(o)->p.target())<<"\n";*/
  /*dbg << "StxPartEdge::operator<("<<((p.source() < dynamicPtrCast<StxPartEdge>(o)->p.source()) ||
         (p.source() == dynamicPtrCast<StxPartEdge>(o)->p.source() &&
          p.target() < dynamicPtrCast<StxPartEdge>(o)->p.target()))<<endl; //(p < dynamicPtrCast<StxPartEdge>(o)->p)<<endl;
  dbg << "---- p="<<CFGPath2Str(p)<<endl;
  dbg << "---- dynamicPtrCast<StxPartEdge>(o)->p"<<CFGPath2Str(dynamicPtrCast<StxPartEdge>(o)->p)<<endl;*/
  //return p < dynamicPtrCast<StxPartEdge>(o)->p;
  // Since is the possible to create p either from makeClosureDF() or from its source/target CFGNode pair, we compare
  // paths in terms of just their source/target CFGNodes
  return (p.source() < dynamicPtrCast<StxPartEdge>(o)->p.source()) ||
         (p.source() == dynamicPtrCast<StxPartEdge>(o)->p.source() &&
          p.target() < dynamicPtrCast<StxPartEdge>(o)->p.target());
}

std::string StxPartEdge::str(std::string indent)
{
  ostringstream oss;
  oss << (isNULLCFGNode(p.source().getNode())? "*" : source()->str()) << 
         " ==&gt; " << 
         (isNULLCFGNode(p.target().getNode())? "*" : target()->str());// << ", analysis="<<analysis
  return oss.str();
}

/**************************
 ***** StxValueObject *****
 **************************/

StxValueObject::StxValueObject(SgNode* n) : ValueObject(n)
{
  // If a valid node is passed, check if it is an SgValue
  if(n) {
    if(stxAnalysisDebugLevel()>=1) {
      dbg << "StxValueObject::StxValueObject("<<SgNode2Str(n)<<")";
      dbg << " isSgCastExp(n)="<<isSgCastExp(n)<<" unwrapCasts(isSgCastExp(n))="<<(isSgCastExp(n) ? SgNode2Str(unwrapCasts(isSgCastExp(n))) : "NULL")<<" iscast="<<(isSgCastExp(n) ? isSgValueExp(unwrapCasts(isSgCastExp(n))) : 0)<<endl;
    }
    if(isSgValueExp(n)) 
      val = isSgValueExp(n);
    // If this is a value that has been wrapped in many casts
    // GB 2012-10-09 - NOTE: in the future we'll need to refine this code to accurately capture the effect of these casts!
    else if(isSgCastExp(n) && isSgValueExp(unwrapCasts(isSgCastExp(n))))
      val = isSgValueExp(unwrapCasts(isSgCastExp(n)));
    else
      val = NULL;
  // Otherwise, default this ValueObject to an unknown 
  } else 
    val = NULL;
}

StxValueObject::StxValueObject(const StxValueObject& that) : ValueObject((const ValueObject&)that), val(that.val)
{ }

bool StxValueObject::mayEqualV(ValueObjectPtr that_arg, PartEdgePtr pedge)
{
  StxValueObjectPtr that = boost::dynamic_pointer_cast <StxValueObject> (that_arg);
  // ValueObject abstractions of different types may be equal to each other (can't tell either way)
  if(!that) { return true; }
  
  // If either object is not an SgValue, they may be equal to each other
  if(val==NULL || that->val==NULL) { return true; }
  
  // If both are SgValues, equalValExp makes a definitive precise comparison
  return equalValExp(val, that->val);
}

bool StxValueObject::mustEqualV(ValueObjectPtr that_arg, PartEdgePtr pedge)
{
  //const StxValueObject & that = dynamic_cast <const StxValueObject&> (that_arg);
  StxValueObjectPtr that = boost::dynamic_pointer_cast <StxValueObject> (that_arg);
  // ValueObject abstractions of different types can't be proven to be definitely equal to each other (can't tell either way)
  if(!that) { return false; }
  
  // If either object is not an SgValue, we can't prove that must be equal to each other
  if(val==NULL || that->val==NULL) { return false; }
  
  // If both are SgValues, equalValExp makes a definitive precise comparison
  //if(stxAnalysisDebugLevel()>=1) dbg << "StxValueObject::mustEqualV calling equalValExp("<<SgNode2Str(val)<<", "<<SgNode2Str(that->val)<<")"<<endl;
  return equalValExp(val, that->val);
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool StxValueObject::equalSetV(ValueObjectPtr that_arg, PartEdgePtr pedge)
{
  //const StxValueObject & that = dynamic_cast <const StxValueObject&> (that_arg);
  StxValueObjectPtr that = boost::dynamic_pointer_cast <StxValueObject> (that_arg);
  // ValueObject abstractions of different types can't be proven to be definitely equal to each other (can't tell either way)
  if(!that) { return false; }
  
  // If neither object is not a known SgValue, they both denote the set of all Values, 
  if(val==NULL && that->val==NULL) { return true; }
  // If only one of the objects is not a known SgValue, they denote different sets
  if(val==NULL || that->val==NULL) { return false; }
  
  // If both are SgValues, equalValExp makes a definitive precise comparison
  return equalValExp(val, that->val);
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool StxValueObject::subSetV(ValueObjectPtr that_arg, PartEdgePtr pedge)
{
  //const StxValueObject & that = dynamic_cast <const StxValueObject&> (that);
  StxValueObjectPtr that = boost::dynamic_pointer_cast <StxValueObject> (that_arg);
  // ValueObject abstractions of different types can't be proven to be definitely equal to each other (can't tell either way)
  if(!that) { return false; }
  
  // If neither object is not a known SgValue, they both denote the set of all Values, 
  if(val==NULL && that->val==NULL) { return true; }
  
  // If that object denotes all SgValues and this object denotes some concrete one, this is a subset of that
  if(that->val==NULL) { return true; }
  // If it is vice versa, then this object (all) is not a subset of that object (concrete)
  else if(val==NULL) { return false; }
  
  // If both are SgValues, equalValExp returns true if they denote the same value
  return equalValExp(val, that->val);
}

// Returns true if the given pair of SgValueExps represent the same value and false otherwise
bool StxValueObject::equalValExp(SgValueExp* a, SgValueExp* b)
{
  if(isSgBoolValExp(a) && isSgBoolValExp(b)) 
    return isSgBoolValExp(a)->get_value() == isSgBoolValExp(b)->get_value();
  else if(isSgCharVal(a) && isSgCharVal(a)) 
    return isSgCharVal(a)->get_value() == isSgCharVal(b)->get_value();
  else if(isSgComplexVal(a) && isSgComplexVal(b))
    return equalValExp(isSgComplexVal(a)->get_real_value(), isSgComplexVal(b)->get_real_value()) &&
           equalValExp(isSgComplexVal(a)->get_imaginary_value(), isSgComplexVal(b)->get_imaginary_value());
  else if(isSgDoubleVal(a) && isSgDoubleVal(b))
    return isSgDoubleVal(a)->get_value() == isSgDoubleVal(b)->get_value();
  else if(isSgEnumVal(a) && isSgEnumVal(b))
    return isSgEnumVal(a)->get_value() == isSgEnumVal(b)->get_value();
  else if(isSgFloatVal(a) && isSgFloatVal(b))
    return isSgFloatVal(a)->get_value() == isSgFloatVal(b)->get_value();
  else if(isSgIntVal(a) && isSgIntVal(b))
    return isSgIntVal(a)->get_value() == isSgIntVal(b)->get_value();
  else if(isSgLongDoubleVal(a) && isSgLongDoubleVal(b))
    return isSgLongDoubleVal(a)->get_value() == isSgLongDoubleVal(b)->get_value();
  else if(isSgLongIntVal(a) && isSgLongIntVal(b))
    return isSgLongIntVal(a)->get_value() == isSgLongIntVal(b)->get_value();
  else if(isSgLongLongIntVal(a) && isSgLongLongIntVal(b))
    return isSgLongLongIntVal(a)->get_value() == isSgLongLongIntVal(b)->get_value();
  else if(isSgShortVal(a) && isSgShortVal(b))
    return isSgShortVal(a)->get_value() == isSgShortVal(b)->get_value();
  else if(isSgStringVal(a) && isSgStringVal(b))
    return isSgStringVal(a)->get_value() == isSgStringVal(b)->get_value();
  else if(isSgUnsignedCharVal(a) && isSgUnsignedCharVal(b))
    return isSgUnsignedCharVal(a)->get_value() == isSgUnsignedCharVal(b)->get_value();
  else if(isSgUnsignedIntVal(a) && isSgUnsignedIntVal(b))
    return isSgUnsignedIntVal(a)->get_value() == isSgUnsignedIntVal(b)->get_value();
  /*else if(isSgUnsigedLongLongIntVal(a) && isSgUnsigedLongLongIntVal(b))
    return isSgUnsigedLongLongIntVal(a)->get_value() == isSgUnsigedLongLongIntVal(b)->get_value();*/
  else if(isSgUnsignedLongVal(a) && isSgUnsignedLongVal(b))
    return isSgUnsignedLongVal(a)->get_value() == isSgUnsignedLongVal(b)->get_value();
  else if(isSgUnsignedShortVal(a) && isSgUnsignedShortVal(b))
    return isSgUnsignedShortVal(a)->get_value() == isSgUnsignedShortVal(b)->get_value();
  /*else if(isSgUpcMythreadVal(a) && isSgUpcMythreadVal(b))
    return isSgUpcMythreadVal(a)->get_value() == isSgUpcMythreadVal(b)->get_value();
  else if(isSgUpcThreadsVal(a) && isSgUpcThreadsVal(b))
    return isSgUpcThreadsVal(a)->get_value() == isSgUpcThreadsVal(b)->get_value();*/
  else if(isSgWcharVal(a) && isSgWcharVal(b))
    return isSgWcharVal(a)->get_value() == isSgWcharVal(b)->get_value();
  else
    return false;
}

// Computes the meet of this and that and saves the result in this.
// Returns true if this causes this to change and false otherwise.
bool StxValueObject::meetUpdateV(ValueObjectPtr that_arg, PartEdgePtr pedge)
{
  StxValueObjectPtr that = boost::dynamic_pointer_cast <StxValueObject> (that_arg);
  assert(that);
  
  // If the value objects denote different values
  if(!mustEqualV(that, pedge)) {
    // Set the value pointer of this object to NULL since we cannot represent their union with a single value
    val = NULL;
    return true;
  }
  return false;
}

bool StxValueObject::isFullV(PartEdgePtr pedge)
{ return val == NULL; }

bool StxValueObject::isEmptyV(PartEdgePtr pedge)
{ return false; }


// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool StxValueObject::isConcrete()
{
  return val;
}

// Returns the type of the concrete value (if there is one)
SgType* StxValueObject::getConcreteType()
{
  assert(val);
  SgTreeCopy copyHelp;
  return (SgType*)(val->get_type()->copy(copyHelp));
}

// Returns the concrete value (if there is one) as an SgValueExp, which allows callers to use
// the normal ROSE mechanisms to decode it
std::set<boost::shared_ptr<SgValueExp> > StxValueObject::getConcreteValue()
{
  assert(val);
  SgTreeCopy copyHelp;
  std::set<boost::shared_ptr<SgValueExp> > concreteVals;
  concreteVals.insert(boost::shared_ptr<SgValueExp>((SgValueExp*)val->copy(copyHelp)));
  return concreteVals;
}
 
//std::string StxValueObject::str(const string& indent) {
std::string StxValueObject::str(std::string indent) { // pretty print for the object
  return "[StxValueObject: "+(val? val->unparseToString() : "NULL")+"]";
}

// Allocates a copy of this object and returns a pointer to it
ValueObjectPtr StxValueObject::copyV() const 
{ return boost::make_shared<StxValueObject>(*this); }

/****************************
 ***** StxCodeLocObject *****
 **************************** /

StxCodeLocObject::StxCodeLocObject(SgNode* n, PartEdgePtr pedge) : pedge(pedge)
{
  code = isSgExpression(n);
}    

StxCodeLocObject::StxCodeLocObject(const StxCodeLocObject& that) : pedge(that.pedge), code(that.code)
{ }

bool StxCodeLocObject::mayEqualCL(CodeLocObjectPtr that_arg, PartEdgePtr pedge)
{
  StxCodeLocObjectPtr that = boost::dynamic_pointer_cast <StxCodeLocObject> (that_arg);
  if(!that) { return false; }
  // Return true if either CodeLocObject is a wildcard
  if(code==NULL || that->code==NULL) return true;
  else                               return mustEqualCL(that, pedge);
}

bool StxCodeLocObject::mustEqualCL(CodeLocObjectPtr that_arg, PartEdgePtr pedge)
{
  StxCodeLocObjectPtr that = boost::dynamic_pointer_cast <StxCodeLocObject> (that_arg);
  if(!that) { return false; }
  if(isSgFunctionCallExp(code) && isSgFunctionCallExp(that->code) &&
     isSgFunctionCallExp(code)->getAssociatedFunctionSymbol() && 
     isSgFunctionCallExp(that->code)->getAssociatedFunctionSymbol())
    return isSgFunctionCallExp(code)->getAssociatedFunctionSymbol()->get_name() == 
           isSgFunctionCallExp(that->code)->getAssociatedFunctionSymbol()->get_name();
  else
    return false;
}

// Returns whether the two abstract objects denote the same set of concrete objects
bool StxCodeLocObject::equalSetCL(CodeLocObjectPtr that_arg, PartEdgePtr pedge)
{
  StxCodeLocObjectPtr that = boost::dynamic_pointer_cast <StxCodeLocObject> (that_arg);
  if(!that) { return false; }
  // If both objects denote a concrete function, they denote the same set of those functions are equal
  if(isSgFunctionCallExp(code) && isSgFunctionCallExp(that->code) &&
     isSgFunctionCallExp(code)->getAssociatedFunctionSymbol() && 
     isSgFunctionCallExp(that->code)->getAssociatedFunctionSymbol())
    return isSgFunctionCallExp(code)->getAssociatedFunctionSymbol()->get_name() == 
           isSgFunctionCallExp(that->code)->getAssociatedFunctionSymbol()->get_name();
  // If both objects denote the set of all CodeLocs, they're equal
  else if(code==NULL && that->code==NULL)
    return true;
  else
    return false;
}

// Returns whether this abstract object denotes a non-strict subset (the sets may be equal) of the set denoted
// by the given abstract object.
bool StxCodeLocObject::subSetCL(CodeLocObjectPtr that_arg, PartEdgePtr pedge)
{
  StxCodeLocObjectPtr that = boost::dynamic_pointer_cast <StxCodeLocObject> (that_arg);
  if(!that) { return false; }
  // If both objects denote a concrete function, they denote the same set of those functions are equal
  if(isSgFunctionCallExp(code) && isSgFunctionCallExp(that->code) &&
     isSgFunctionCallExp(code)->getAssociatedFunctionSymbol() && 
     isSgFunctionCallExp(that->code)->getAssociatedFunctionSymbol())
    return isSgFunctionCallExp(code)->getAssociatedFunctionSymbol()->get_name() == 
           isSgFunctionCallExp(that->code)->getAssociatedFunctionSymbol()->get_name();
  // If both objects denote the set of all CodeLocs, they're equal
  else if(code==NULL && that->code==NULL)
    return true;
  // Of this object denotes a concrete function while that object denotes all functions, this is a subset of that
  else if(that->code == NULL)
    return true;
  // If vice versa then this object (all) is not a subset of that (concrete)
  else if(code == NULL)
    return false;
  assert(0);
}

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool StxCodeLocObject::meetUpdateCL(CodeLocObjectPtr that_arg, PartEdgePtr pedge)
{
   StxCodeLocObjectPtr that = boost::dynamic_pointer_cast <StxCodeLocObject> (that_arg);
   assert(that);
   
   // If the objects denote different code location expressions, 
   // make this into a wildcard location
   if(code != that->code) {
     code = NULL;
     return true;
   }
   
   return false;
}

bool StxCodeLocObject::isFull(PartEdgePtr pedge)
{ return code == NULL; }

bool StxCodeLocObject::isEmpty(PartEdgePtr pedge)
{ return false; }

std::string StxCodeLocObject::str(std::string indent) { // pretty print for the object
  return "[StxCodeLocObject: "+(code? code->unparseToString() : "NULL")+"]";
}

// Allocates a copy of this object and returns a pointer to it
CodeLocObjectPtr StxCodeLocObject::copyCL() const 
{ return boost::make_shared<StxCodeLocObject>(*this); }
*/

/**********************************
 ***** ABSTRACT MEMORY REGION *****
 **********************************/

// Returns the string representation of the given type
std::string StxMemRegionType::MRType2Str(regType type) {
  switch(type) {
    case expr:    return "expr";
    case named:   return "named";
    case storage: return "storage";
    case all:     return "all";
    default:      assert(0);
  }
}

StxMemRegionObject::StxMemRegionObject(SgNode* n): MemRegionObject(n) {
  // Find the correct kind for this region

  // First see if it is a named region
  if(!(type=StxNamedMemRegionType::getInstance(n)))
    // If not, see if it is an expression region
    if(!(type=StxExprMemRegionType::getInstance(n)))
      // If not, see if it is an unknown region
      if(!(type=StxStorageMemRegionType::getInstance(n)))
        // It must be one of the above, so crash if we get here
        assert(0);
}

StxMemRegionObject::StxMemRegionObject(const StxMemRegionObject& that): MemRegionObject(that), type(that.type)
{}

// Returns whether this object may/must be equal to o within the given Part p
// These methods are called by composers and should not be called by analyses.
bool StxMemRegionObject::mayEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  StxMemRegionObjectPtr that = boost::dynamic_pointer_cast<StxMemRegionObject>(o); assert(that);
  
  // If either object denotes the set of all memory regions, the two objects are may-equal
  if(getType()==StxMemRegionType::all || that->getType()==StxMemRegionType::all)
    return true;
  
  // At this point we're sure that neither this nor that are all objects
  assert(getType()!=StxMemRegionType::all && that->getType()!=StxMemRegionType::all);
  
  // Expression objects are distinct from each other and both named and storage objects
  if((getType()==StxMemRegionType::expr && that->getType()!=StxMemRegionType::expr) ||
     (getType()!=StxMemRegionType::expr && that->getType()==StxMemRegionType::expr) ||
     (getType()==StxMemRegionType::expr && that->getType()==StxMemRegionType::expr && type->getUID()!=that->type->getUID()))
    return false;
  else if(getType()==StxMemRegionType::expr && that->getType()==StxMemRegionType::expr)
    return true;
  
  // At this point we're sure that neither this nor that are expression objects
  assert(getType()!=StxMemRegionType::expr && that->getType()!=StxMemRegionType::expr);
  
  // Storage objects denote the set of memory regions in stack+heap+globals
  // All known objects are specific regions within stack+heap+globals
  // Thus, if either this or that are storage they may-equal each other
  if(getType()==StxMemRegionType::storage || that->getType()==StxMemRegionType::storage)
    return true;
  
  // At this point we're sure that both this and that are known objects
  assert(getType()==StxMemRegionType::named && that->getType()==StxMemRegionType::named);
  
  // Named objects are distinct from each other iff their symbols are.
  return type->getUID()==that->type->getUID();
}

bool StxMemRegionObject::mustEqualMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  StxMemRegionObjectPtr that = boost::dynamic_pointer_cast<StxMemRegionObject>(o); assert(that);
  
  // If either object denotes the set of all memory regions, the two objects are not must-equal since this
  // set has unbounded size
  if(getType()==StxMemRegionType::all || that->getType()==StxMemRegionType::all)
    return false;
  
  // At this point we're sure that neither this nor that are all objects
  assert(getType()!=StxMemRegionType::all && that->getType()!=StxMemRegionType::all);
  
  // Expression objects are distinct from each other and both named and storage objects
  if((getType()==StxMemRegionType::expr && that->getType()!=StxMemRegionType::expr) ||
     (getType()!=StxMemRegionType::expr && that->getType()==StxMemRegionType::expr) ||
     (getType()==StxMemRegionType::expr && that->getType()==StxMemRegionType::expr && type->getUID()!=that->type->getUID()))
    return false;
  else if(getType()==StxMemRegionType::expr && that->getType()==StxMemRegionType::expr)
    return true;
  
  // At this point we're sure that neither this nor that are expression objects
  assert(getType()!=StxMemRegionType::expr && that->getType()!=StxMemRegionType::expr);
  
  // Storage objects denote the set of memory regions in stack+heap+globals
  // All known objects are specific regions within stack+heap+globals
  // Thus, if either this or that are storage they may-equal each other. 
  // However, since storage objects are unbounded-size sets of memory regions, 
  // these objects are not must-equal.
  if(getType()==StxMemRegionType::storage || that->getType()==StxMemRegionType::storage)
    return false;
  
  // At this point we're sure that both this and that are known objects
  assert(getType()==StxMemRegionType::named && that->getType()==StxMemRegionType::named);
  
  // Named objects are distinct from each other iff their symbols are.
  return type->getUID()==that->type->getUID();
}

bool StxMemRegionObject::equalSetMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  StxMemRegionObjectPtr that = boost::dynamic_pointer_cast<StxMemRegionObject>(o); assert(that);

  // If both objects denotes the set of all memory regions, the two objects denote the same set
  if(getType()==StxMemRegionType::all && that->getType()==StxMemRegionType::all)
    return true;
  
  // If only one of them denotes the set of all memory regions, they're not the same set
  if(getType()==StxMemRegionType::all || that->getType()==StxMemRegionType::all)
    return false;
  
  // At this point we're sure that neither this nor that are all objects
  assert(getType()!=StxMemRegionType::all && that->getType()!=StxMemRegionType::all);
  
  // Expression objects are distinct from each other and both named and storage objects
  if((getType()==StxMemRegionType::expr && that->getType()!=StxMemRegionType::expr) ||
     (getType()!=StxMemRegionType::expr && that->getType()==StxMemRegionType::expr) ||
     (getType()==StxMemRegionType::expr && that->getType()==StxMemRegionType::expr && type->getUID()!=that->type->getUID()))
    return false;
  else if(getType()==StxMemRegionType::expr && that->getType()==StxMemRegionType::expr)
    return true;
  
  // At this point we're sure that neither this nor that are expression objects
  assert(getType()!=StxMemRegionType::expr && that->getType()!=StxMemRegionType::expr);
  
  // Storage objects denote the set of memory regions in stack+heap+globals
  // All known objects are specific regions within stack+heap+globals
  // If both objects denote this set, they're equal
  if(getType()==StxMemRegionType::storage && that->getType()==StxMemRegionType::storage)
    return true;
  
  // But if only one denotes this set, they're not equal
  if(getType()==StxMemRegionType::storage || that->getType()==StxMemRegionType::storage)
    return false;
  
  // At this point we're sure that both this and that are known objects
  assert(getType()==StxMemRegionType::named && that->getType()==StxMemRegionType::named);
  
  // Named objects are distinct from each other iff their symbols are.
  return type->getUID()==that->type->getUID();
}

bool StxMemRegionObject::subSetMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  StxMemRegionObjectPtr that = boost::dynamic_pointer_cast<StxMemRegionObject>(o); assert(that);
  
  // If that object denotes the set of all memory regions, it contains everything else
  if(that->getType()!=StxMemRegionType::all)
    return true;
  
  // However, if this one denotes the set of all regions but that does not
  if(getType()==StxMemRegionType::all)
    return false;
  
  // At this point we're sure that neither this nor that are all objects
  assert(getType()!=StxMemRegionType::all && that->getType()!=StxMemRegionType::all);
  
  // Expression objects are distinct from each other and both named and storage objects
  if((getType()==StxMemRegionType::expr && that->getType()!=StxMemRegionType::expr) ||
     (getType()!=StxMemRegionType::expr && that->getType()==StxMemRegionType::expr) ||
     (getType()==StxMemRegionType::expr && that->getType()==StxMemRegionType::expr && type->getUID()!=that->type->getUID()))
    return false;
  else if(getType()==StxMemRegionType::expr && that->getType()==StxMemRegionType::expr)
    return true;
  
  // At this point we're sure that neither this nor that are expression objects
  assert(getType()!=StxMemRegionType::expr && that->getType()!=StxMemRegionType::expr);
  
  // Storage objects denote the set of memory regions in stack+heap+globals
  // All known objects are specific regions within stack+heap+globals
  // If that object denotes all storage regions, it must contain this one
  if(that->getType()==StxMemRegionType::storage)
    return true;
  
  // However, if this object denotes the set of all storage regions but that one does not
  if(getType()==StxMemRegionType::storage)
    return false;
  
  // At this point we're sure that both this and that are known objects
  assert(getType()==StxMemRegionType::named && that->getType()==StxMemRegionType::named);
  
  // Named objects are distinct from each other iff their symbols are.
  return type->getUID()==that->type->getUID();
}

// Returns true if this object is live at the given part and false otherwise
bool StxMemRegionObject::isLiveMR(PartEdgePtr pedge)
{ return type->isLiveMR(pedge); }

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool StxMemRegionObject::meetUpdateMR(MemRegionObjectPtr o, PartEdgePtr pedge)
{
  StxMemRegionObjectPtr that = boost::dynamic_pointer_cast<StxMemRegionObject>(o); assert(that);
  
  // expr + * = all
  // all + * = all
  if(getType()==StxMemRegionType::expr || that->getType()==StxMemRegionType::expr ||
     getType()==StxMemRegionType::all  || that->getType()==StxMemRegionType::all) {
    if(getType()!=StxMemRegionType::all) {
      type = StxAllMemRegionType::getInstance(getBase());
      return true;
    } else
      return false;
  }
  
  // named + named = storage
  // named + storage = storage
  // storage + storage = storage
  if(getType()!=StxMemRegionType::storage) {
    type = StxStorageMemRegionType::getInstance(getBase());
    return true;
  } else
    return false;
}

// Returns a ValueObject that denotes the size of this memory region
ValueObjectPtr StxMemRegionObject::getRegionSize(PartEdgePtr pedge) const
{
  // Since the syntactic analysis does nothing interesting with region indexes, it does not provide
  // any useful info for region sizes. Client analyses that do pointer arithmetic will need to handle
  // this on their own
  return boost::make_shared<StxValueObject>((SgNode*)NULL);
}

// Returns whether this AbstractObject denotes the set of all possible execution prefixes.
bool StxMemRegionObject::isFullMR(PartEdgePtr pedge)
{
  return getType() == StxMemRegionType::all;
}

// Returns whether this AbstractObject denotes the empty set.
bool StxMemRegionObject::isEmptyMR(PartEdgePtr pedge) {
  // StxMemRegionObjects are only created for non-empty sets of memory regions
  return false;
}
       

// Allocates a copy of this object and returns a pointer to it
MemRegionObjectPtr StxMemRegionObject::copyMR() const {
  return boost::make_shared<StxMemRegionObject>(*this);
}

std::string StxMemRegionObject::str(std::string indent) { // pretty print for the object
  return txt() << "[StxMR: "<<type->str()<<"]";
}

StxExprMemRegionTypePtr NULLStxExprMemRegionType;

// If the given SgNode corresponds to a named memory region, returns a freshly-allocated
// StxExprMemRegionType that represents it. Otherwise, returns NULL.
StxExprMemRegionTypePtr StxExprMemRegionType::getInstance(SgNode* n) {
  if(SgExpression* expr = isSgExpression(n)) {
    if(!isSgVarRefExp(n) && !isSgDotExp(n) && !isSgPntrArrRefExp(n))
      return boost::make_shared<StxExprMemRegionType>(expr);
  }

  // If n is not a valid expression, return NULL
  return NULLStxExprMemRegionType;
}

// Return the list of this node's ancestors, upto and including the nearest enclosing 
// statement as well as the node itself, with the deeper ancestors placed towards the front of the list
/*list<SgNode*> getAncestorToStmt(SgNode* n) {
  list<SgNode*> ancestors;
  / *scope reg("getAncestorToStmt", scope::medium, 1,1);
  dbg << "n=["<<n->unparseToString()<<" | "<<n->class_name()<<"]"<<endl;
  indent ind(1, 1);* /

  SgNode* a = n;
  //dbg << "a=["<<a->unparseToString()<<" | "<<a->class_name()<<"]"<<endl;
  while(a!=NULL && !isSgStatement(a)) {
    ancestors.push_front(a);
    a = a->get_parent();
    / *if(a) dbg << "#ancestors="<<ancestors.size()<<" a=["<<a->unparseToString()<<" | "<<a->class_name()<<"]"<<endl;
    else  dbg << "#ancestors="<<ancestors.size()<<" a=NULL"<<endl;* /
  }
  if(a!=NULL) ancestors.push_front(a);
  return ancestors;
}*/

bool enc (SgExpression* expr, const CFGNode& n) {
  // exprr is in-scope at n if they're inside the same statement or n is an SgIfStmt, SgForStatement, SgWhileStmt 
  // or SgDoWhileStmt and exprr is inside its sub-statements
  return (SageInterface::getEnclosingStatement(n.getNode()) == 
          SageInterface::getEnclosingStatement(expr)) ||
         (isSgIfStmt(n.getNode()) && 
          isSgIfStmt(n.getNode())->get_conditional()==
          SageInterface::getEnclosingStatement(expr)) ||
         (isSgWhileStmt(n.getNode()) && 
          isSgWhileStmt(n.getNode())->get_condition()==
          SageInterface::getEnclosingStatement(expr)) ||
         (isSgDoWhileStmt(n.getNode()) && 
          isSgDoWhileStmt(n.getNode())->get_condition()==
          SageInterface::getEnclosingStatement(expr)) ||
         (isSgForStatement(n.getNode()) && 
          (isSgForStatement(n.getNode())->get_for_init_stmt()==SageInterface::getEnclosingStatement(expr) ||
           isSgForStatement(n.getNode())->get_test()         ==SageInterface::getEnclosingStatement(expr)));
}

// Returns true if this object is live at the given part and false otherwise
bool StxExprMemRegionType::isLiveMR(PartEdgePtr pedge) {
//RULE 1: Fails because it doesn't account for the fact that between an operand and its parent
  //        there may be several more nodes from another sub-branch of the expression tree
  // The anchor expression is in scope if it is equal to the current SgNode or is its operand
  //return (expr==part.getNode() || isOperand(part.getNode(), expr));

  //RULE 2: The expression is in-scope at a Part if they're inside the same statement
  //        This rule is fairly loose but at least it is easy to compute. The right rule
  //        would have been that the part is on some path between the expression and its
  //        parent but this would require an expensive graph search
  /*return SageInterface::getEnclosingStatement(expr) == 
         SageInterface::getEnclosingStatement(part.getNode());*/
  //boost::function<bool (SgExpression*, const CFGNode&)> enc1 = &enc;

  // GB 2012-10-18 - I'm not sure what to do here about edges with wildcard sources or targets.
  //                 It seems like to be fully general we need to say that something is live if it is live at
  //                 any source and any destination, meaning that we need consider all the outcomes of a wildcard.
  //                 For example, what happens when an edge may cross a scope boundary for one but not all
  //                 of the wildcard outcomes?
  return (pedge->source() ? pedge->source()->mapCFGNodeANY<bool>(boost::bind(enc, expr, _1)): false) ||
         (pedge->target() ? pedge->target()->mapCFGNodeANY<bool>(boost::bind(enc, expr, _1)): false);

  /*struct enc { public: bool op(SgExpression* expr, const CFGNode& n) {
    return SageInterface::getEnclosingStatement(expr) == 
           SageInterface::getEnclosingStatement(n.getNode());
  } }; enc e;
  return part->mapCFGNodeANY<bool>(boost::bind(&enc::op, expr, _1));*/

  //RULE 3: look for a common ancestor between expr and part.getNode(). If this ancestor134/dix
  //        is part.getNode(), below part.getNode() or exprr is an operand of part.getNode() (it is
  //        one level above part.getNode()) then it is in-scope.
  //    expr             a   b
  //     |                \ /
  //     c                 d
  //      \------- e -----/
  //               |
  //               f
  // expr is in-scope at expr, a, b, c, d but not e or f.
  //scope reg(1, 1, scope::medium, string("ExprObj::isLive[")+expr->unparseToString()+string(" | ")+expr->class_name()+string(">"));

  // If part.getNode() is equal to exprr or uses it as an operand, then expr is in-scope
  /*if(expr==part.getNode() || isOperand(part.getNode(), expr)) { //expr->get_parent()==part.getNode()) {
    //dbg << "IN-SCOPE"<<endl;
    return true;
  // Otherwise, expr is only in-scope if shares an ancestor with part.getNode() but part.getNode() 
  // is not that ancestor.
} else {
    //dbg << "expr->get_parent()=["<<expr->get_parent()->unparseToString()<<" | "<<expr->get_parent()->class_name()<<"]"<<endl;
    //dbg << "part.getNode()=["<<part.getNode()->unparseToString()<<" | "<<part.getNode()->class_name()<<"]"<<endl;
    //dbg << "isOperand(part.getNode(), expr)="<<isOperand(part.getNode(), expr)<<endl;
    // Get the ancestor lists of both nodes
    //dbg << "getAncestorToStmt(expr)"<<endl;
    list<SgNode*> anchorAncestors = getAncestorToStmt(expr);
    //dbg << "#anchorAncestors="<<anchorAncestors.size()<<endl;
    //dbg << "getAncestorToStmt(part.getNode())"<<endl;
    list<SgNode*> partAncestors = getAncestorToStmt(part.getNode());
    //dbg << "#partAncestors="<<partAncestors.size()<<endl;
    assert(isSgStatement(*anchorAncestors.begin()));

    // If the roots of the ancestor trees are mismatched, expr is not in-scope
    if(!isSgStatement(*partAncestors.begin()) || *(anchorAncestors.begin())!=*(partAncestors.begin())) {
      //dbg << "OUT-OF-SCOPE partStmt="<<isSgStatement(*partAncestors.begin())<<", sameStmt="<<(*(anchorAncestors.begin())!=*(partAncestors.begin()))<<endl;
      return false;
    }

    // Iterate through the ancestor lists from the deepest point to the shallowest, looking for a deviation
    list<SgNode*>::iterator a, p;
    for(a = anchorAncestors.begin(), p = partAncestors.begin(); 
        a!=anchorAncestors.end() && p!=partAncestors.end(); a++, p++) {
      if(*a != *p) break;
    }

    // If we stopped at the end of either ancestor list then one of the nodes is an ancestor of the other: not in-scope
    if(a==anchorAncestors.end() || p==partAncestors.end()) {
      //dbg << "OUT-OF-SCOPE (anchor end="<<(a==anchorAncestors.end())<<", part end="<<(p==partAncestors.end())<<endl;
      return false;
    }

    // Otherwise, if there are more nodes left on both ancestor lists, then expr is in-scope
    //dbg << "IN-SCOPE"<<endl;
    return true;
  }*/
}

std::string StxExprMemRegionType::str(std::string indent) { // pretty print for the object
  return txt() << "Expr: "<<SgNode2Str(expr);
}

StxNamedMemRegionTypePtr NULLStxNamedMemRegionType;

// If the given SgNode corresponds to a named memory region, returns a freshly-allocated
// StxNamedMemRegionType that represents it. Otherwise, returns NULL.
StxNamedMemRegionTypePtr StxNamedMemRegionType::getInstance(SgNode* n) {
  if(SgVarRefExp* ref = isSgVarRefExp(n)) {
    return boost::make_shared<StxNamedMemRegionType>(ref->get_symbol()->get_declaration(), ref->get_symbol());
  } else if(SgInitializedName* iname = isSgInitializedName(n)) {
    return boost::make_shared<StxNamedMemRegionType>(iname, iname->search_for_symbol_from_symbol_table());
  } else {
    return NULLStxNamedMemRegionType;
  }
}

// Return whether there exists a CFGNode within this part that is inside the function in which the anchor symbol
// is defined.
bool matchAnchorPart(SgScopeStatement* scope, const CFGNode& n) {
  SgScopeStatement* part_scope = SageInterface::getScope(n.getNode());
  assert(part_scope);
  if(scope == part_scope)
    return true;
  else
    return SageInterface::isAncestor(scope, part_scope);
}

// Returns true if this object is live at the given part and false otherwise
bool StxNamedMemRegionType::isLiveMR(PartEdgePtr pedge) {
  if(iname) {
    // This variable is in-scope if part.getNode() is inside the scope that contains its declaration
    SgScopeStatement* scope=NULL;
    assert(symbol==NULL || isSgVariableSymbol(symbol) || isSgFunctionSymbol(symbol));
    /*if(isSgVariableSymbol(symbol))
      scope = isSgVariableSymbol(symbol)->get_declaration()->get_declaration()->get_scope();
    else if(isSgFunctionSymbol(symbol))
      scope = isSgFunctionSymbol(symbol)->get_declaration()->get_scope();*/
    scope = iname->get_scope();

    assert(scope);

    if(symbol!=NULL && isSgFunctionSymbol(symbol)) return true;
    else if(symbol==NULL || isSgVariableSymbol(symbol)) {
      //dbg << "symbol="<<SgNode2Str(symbol)<<" pedge="<<pedge->str()<<endl;
      // GB 2012-10-18 - I'm not sure what to do here about edges with wildcard sources or targets.
      //                 It seems like to be fully general we need to say that something is live if it is live at
      //                 any source and any destination, meaning that we need consider all the outcomes of a wildcard.
      //                 For example, what happens when an edge may cross a scope boundary for one but not all
      //                 of the wildcard outcomes?
      return (pedge->source() ? pedge->source()->mapCFGNodeANY<bool>(boost::bind(&matchAnchorPart, scope, _1)) : false) ||
             (pedge->target() ? pedge->target()->mapCFGNodeANY<bool>(boost::bind(&matchAnchorPart, scope, _1)) : false);
    } else
      return false;
  } else
    return true;

  /*scope reg(string("NamedObj::isLiveMR(")+symbol->get_name().getString()+string(")")+string(isSgFunctionSymbol(symbol) |
  dbg << "anchorFD=";
  if(anchorFD) dbg << "["<<anchorFD->unparseToString()<<" | "<<anchorFD->class_name()<<"]"<<endl;
  else         dbg << "SgFunctionSymbol"<<endl;
  dbg << "partFD=["<<partFD->unparseToString()<<" | "<<partFD->class_name()<<"]"<<endl;
  dbg << "part=["<<part.getNode()->unparseToString()<<" | "<<part.getNode()->class_name()<<"]"<<endl;*/
}

std::string StxNamedMemRegionType::str(std::string indent) { // pretty print for the object
  return txt() << "Named: "<<(symbol? SgNode2Str(symbol): SgNode2Str(iname));
}

// Returns a freshly-allocated All memory region.
StxStorageMemRegionTypePtr StxStorageMemRegionType::getInstance(SgNode* n) {
  return boost::make_shared<StxStorageMemRegionType>();
}

std::string StxStorageMemRegionType::str(std::string indent) { // pretty print for the object
  return "Storage";
}

// Returns a freshly-allocated All memory region.
StxAllMemRegionTypePtr StxAllMemRegionType::getInstance(SgNode* n) {
  return boost::make_shared<StxAllMemRegionType>();
}

std::string StxAllMemRegionType::str(std::string indent) { // pretty print for the object
  return "All";
}

/// Visits live expressions to determine whether the given SgExpression is an operand of the visited Sgxpression
class IsOperandVisitor : public ROSE_VisitorPatternDefaultBase
{
  public:
  bool isOperand;
  SgExpression* op;

  IsOperandVisitor(SgExpression* op) : isOperand(false), op(op) {}

  // Should only be called on expressions
  void visit(SgNode *) { assert(0); }

  // Catch up any other expressions that are not yet handled
  void visit(SgExpression *)
  {
      // Function Reference
      // !!! CURRENTLY WE HAVE NO NOTION OF VARIABLES THAT IDENTIFY FUNCTIONS, SO THIS CASE IS EXCLUDED FOR NOW
      //} else if(isSgFunctionRefExp(sgn)) {
      //} else if(isSgMemberFunctionRefExp(sgn)) { 

      // !!! DON'T KNOW HOW TO HANDLE THESE
      //} else if(isSgStatementExpression(sgn)) {(

      // Typeid
      // !!! DON'T KNOW WHAT TO DO HERE SINCE THE RETURN VALUE IS A TYPE AND THE ARGUMENT'S VALUE IS NOT USED
      //} else if(isSgTypeIdOp(sgn)) {
      // Var Args
      // !!! DON'T HANDLE THESE RIGHT NOW. WILL HAVE TO IN THE FUTURE
      /*  SgVarArgOp 
          SgExpression *  get_operand_expr () const 
          SgVarArgCopyOp
          SgExpression *  get_lhs_operand () const
          SgExpression *  get_rhs_operand () const  
          SgVarArgEndOp 
          SgExpression *  get_operand_expr 
          SgVarArgStartOneOperandOp 
          SgExpression *  get_operand_expr () const 
          SgVarArgStartOp 
          SgExpression *  get_lhs_operand () const
          SgExpression *  get_rhs_operand () const */
      // !!! WHAT IS THIS?
      // SgVariantExpression


      // TODO: Make this assert(0), because unhandled expression types are likely to give wrong results
  }
  // Initializer for a variable
  void visit(SgAssignInitializer *sgn) {
    if(op == sgn->get_operand()) isOperand = true;
  }
  // Initializer for a function arguments
  void visit(SgConstructorInitializer *sgn) {
      SgExprListExp* exprList = sgn->get_args();
      for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
        if(op == *expr) {
          isOperand = true;
          return;
        }
  }
  // Initializer that captures internal stucture of structs or arrays ("int x[2] = {1,2};", it is the "1,2")
  // NOTE: Should this use abstractMemory interface ?
  void visit(SgAggregateInitializer *sgn) {
      SgExprListExp* exprList = sgn->get_initializers();
      for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
        if(op == *expr) {
          isOperand = true;
          return;
        }
  }
  // Designated Initializer 
  void visit(SgDesignatedInitializer *sgn) {
      SgExprListExp* exprList = sgn->get_designatorList();
      for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
        if(op == *expr) {
          isOperand = true;
          return;
        }
  }
  // Array References
  void visit(SgPntrArrRefExp *sgn) {
    SgExpression* arrayNameExp = NULL;
    std::vector<SgExpression*>* subscripts = new std::vector<SgExpression*>;
    SageInterface::isArrayReference(sgn, &arrayNameExp, &subscripts);

    for (std::vector<SgExpression*>::iterator i = subscripts->begin(); i != subscripts->end(); i++) {
      if(op==*i) { isOperand = true; return; }
    }
  }
  // Binary Operations
  void visit(SgBinaryOp *sgn) {
    if(op == sgn->get_lhs_operand()) { isOperand = true; return; }
    if(op == sgn->get_rhs_operand()) { isOperand = true; return; }
  }
  // Unary Operations
  void visit(SgUnaryOp *sgn) {
    if(op == sgn->get_operand()) isOperand = true;
  }
  // Conditionals (condE ? trueE : falseE)
  void visit(SgConditionalExp *sgn) {
    if(op == sgn->get_conditional_exp()) { isOperand = true; return; }
    if(op == sgn->get_true_exp())        { isOperand = true; return; }
    if(op == sgn->get_false_exp())       { isOperand = true; return; }
  }
  // Delete
  void visit(SgDeleteExp *sgn) {
      if(op == sgn->get_variable()) isOperand = true;
  }
  // New
  void visit(SgNewExp *sgn) {
      // The placement arguments are used
      SgExprListExp* exprList = sgn->get_placement_args();
      // NOTE: placement args are optional
      // exprList could be NULL
      // check for NULL before adding to used set
      if(exprList) {
          for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
              expr!=exprList->get_expressions().end(); expr++)
            if(op == *expr) {
              isOperand = true;
              return;
            }
      }

      // The placement arguments are used
      // check for NULL before adding to used set
      // not sure if this check is required for get_constructor_args()
      exprList = sgn->get_constructor_args()->get_args();
      if(exprList) {
          for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
              expr!=exprList->get_expressions().end(); expr++)
            if(op == *expr) {
              isOperand = true;
              return;
            }
      }

      // The built-in arguments are used (DON'T KNOW WHAT THESE ARE!)
      // check for NULL before adding to used set
      // not sure if this check is required for get_builtin_args()
      if(sgn->get_builtin_args()) {
          if(op == sgn->get_builtin_args()) { isOperand = true; return; }
      }
  }
  // Function Calls
  void visit(SgFunctionCallExp *sgn) {
    SgExprListExp* exprList = sgn->get_args();
    for(SgExpressionPtrList::iterator expr=exprList->get_expressions().begin();
          expr!=exprList->get_expressions().end(); expr++)
      if(op == *expr) {
        isOperand = true;
        break;
      }
  }
  // Sizeof
  void visit(SgSizeOfOp *sgn) {
      // XXX: The argument is NOT used, but its type is
      // NOTE: get_operand_expr() returns NULL when sizeof(type)
      // FIX: use get_operand_expr() only when sizeof() involves expr
      if(sgn->get_operand_expr()) {
        if(op == sgn->get_operand_expr()) { isOperand = true; return; }
      }
  }
  // This
  void visit(SgThisExp *sgn) {
  }
  // Variable Reference (we know this expression is live)
  void visit(SgVarRefExp *sgn) {
  }

  void visit(SgReturnStmt *sgn) {
    if(op == sgn->get_expression()) { isOperand = true; return; }
  }
}; // class IsOperandVisitor

// Return true if op is an operand of the given SgNode n and false otherwise.
bool isOperand(SgNode* n, SgExpression* op) {
  if(isSgExpression(n)) {
    IsOperandVisitor helper(op);
    n->accept(helper);
    return helper.isOperand;
  } else if(isSgInitializedName(n)) {
    if(op==isSgInitializedName(n)->get_initializer()) return true;
  } else if(isSgReturnStmt(n)) {
    if(op==isSgReturnStmt(n)->get_expression()) return true;
  } else if(isSgExprStatement(n)) {
    if(op==isSgExprStatement(n)->get_expression()) return true;
  } else if(isSgCaseOptionStmt(n)) {
    if(op==isSgCaseOptionStmt(n)->get_key()) return true;
    if(op==isSgCaseOptionStmt(n)->get_key_range_end()) return true;
  } else if(isSgIfStmt(n)) {
    assert(isSgExprStatement(isSgIfStmt(n)->get_conditional()));
    if(op==isSgExprStatement(isSgIfStmt(n)->get_conditional())->get_expression()) return true;
  } else if(isSgForStatement(n)) {
    assert(isSgExprStatement(isSgForStatement(n)->get_test()));
    if(op==isSgExprStatement(isSgForStatement(n)->get_test())->get_expression()) return true;
    if(op==isSgForStatement(n)->get_increment()) return true;
  } else if(isSgWhileStmt(n)) {
    assert(isSgExprStatement(isSgWhileStmt(n)->get_condition()));
    if(op==isSgExprStatement(isSgWhileStmt(n)->get_condition())->get_expression()) return true;
  } else if(isSgDoWhileStmt(n)) {
    assert(isSgExprStatement(isSgDoWhileStmt(n)->get_condition()));
    if(op==isSgExprStatement(isSgDoWhileStmt(n)->get_condition())->get_expression()) return true;
  } else if(isSgInitializedName(n)) {
    if(op==isSgInitializedName(n)->get_initializer()) return true;
  } else if(isSgInitializedName(n)) {
    if(op==isSgInitializedName(n)->get_initializer()) return true;
  } else {
     // For now we ignore the other cases but should make sure to cover them all in the future
  }

  return false;
}
}; // namespace fuse
