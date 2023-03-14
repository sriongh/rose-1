#include "sage3basic.h"
using namespace std;

#include "stx_analysis.h"
#include <map>
#include <typeinfo>
#include "sageInterface.h"
#include "sageBuilder.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include "VirtualCFGIterator.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace SageBuilder;

//namespace bll = boost::lambda;

namespace fuse {

#ifndef DISABLE_SIGHT
#define stxAnalysisDebugLevel 0
#endif

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
      SIGHT_VERB_DECL(scope, ("FuncEntryExitFunctor", scope::medium), 2, stxAnalysisDebugLevel)

#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(2, stxAnalysisDebugLevel)
        dbg << "Func "<<func.get_name().getString()<<endl;
        dbg << "Type = "<<SgNode2Str(decl->get_type())<<endl;
        dbg << "Declaration = "<<func.get_declaration()<<"="<<SgNode2Str(decl)<<endl;
        dbg << "Definition = "<<(func.get_definition()? SgNode2Str(func.get_definition()): "NULL")<<endl;

        /*if(decl->get_definingDeclaration())
          dbg << "Has defining Declaration: def="<<isSgFunctionDeclaration(decl->get_definingDeclaration())->get_definition()<<endl;
        else
          dbg << "Has not defining Declaration: def="<<isSgFunctionDeclaration(decl->get_firstNondefiningDeclaration())->get_definition()<<endl;*/
      SIGHT_VERB_FI()
#endif

      if(// This is not a declaration defined in a templated class
         // According to rose/src/backend/unparser/languageIndependenceSupport/modified_sage.C:1308
         // "It should be impossible to reach this code since SgTemplateInstantiationDefn is not a class, function or member function type"
         //!isSgTemplateInstantiationDefn(decl->get_parent()) &&
         // And this is not a declaration of a template function (we only care about the instantiations of such functions)
         !isSgTemplateFunctionDeclaration(decl) &&
         !isSgTemplateMemberFunctionDeclaration(decl)) {
        CFGNode Entry;
        CFGNode Exit;

        SIGHT_VERB(dbg << "Function "<<(Func2Entry.find(func)==Func2Entry.end()? "NOT": "")<<" Found\n", 2, stxAnalysisDebugLevel)

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
#ifndef DISABLE_SIGHT
          SIGHT_VERB_IF(2, stxAnalysisDebugLevel)
            dbg << "Creating function "<<func.get_name().getString()<<endl;
            dbg << "decl="<<decl<<"="<<SgNode2Str(decl)<<endl;
            dbg << "def="<<def<<"="<<SgNode2Str(def)<<endl;
          SIGHT_VERB_FI()

          SIGHT_VERB_IF(3, stxAnalysisDebugLevel)
            dbg << "func2 Function "<<func.get_name().getString()<<endl;

            for(back_CFGIterator it(def->cfgForEnd()); it!=back_CFGIterator::end(); it++) {
              dbg << "it="<<CFGNode2Str(*it)<<endl;
            }
          SIGHT_VERB_FI()
#endif
        // If this function has a definition
        } else {
          // The function's exit CFGNode
          Exit = CFGNode(func.get_definition(), 3);
        }

        // Since the function's definition now exists, find its entry point
        // Find the function's entry CFG node, which is the last SgFunctionParameterList node in the function body
        {
        SIGHT_VERB_DECL(scope, ("Iteration", scope::low), 2, stxAnalysisDebugLevel)
        //for(back_CFGIterator it(func.get_definition()->cfgForEnd()); it!=back_CFGIterator::end(); it++) {
        for(CFGIterator it(func.get_definition()->cfgForBeginning()); it!=CFGIterator::end(); it++) {
          SIGHT_VERB(dbg << "    it="<<CFGNode2Str(*it)<<endl, 2, stxAnalysisDebugLevel)
          // Look for the last SgFunctionParameterList node reachable from the start of the function
          if(isSgFunctionParameterList((*it).getNode())) {
            //Entry = *it;
            Entry = CFGNode((*it).getNode(), isSgFunctionParameterList((*it).getNode())->get_args().size());
            break;
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

#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(3, stxAnalysisDebugLevel)
      scope reg("", scope::medium);
      for(map<Function, CFGNode>::iterator i=Func2Entry.begin(); i!=Func2Entry.end(); i++) {
        dbg << i->first.get_name().getString() << " ==&gt; "<<endl;
        dbg << "entry:" << CFGNode2Str(i->second) << endl;
        dbg << " exit: "<< CFGNode2Str(Func2Exit[i->first]) << endl;
      }
    SIGHT_VERB_FI()
#endif
  }
}

// Accessor functions for the above maps
CFGNode getStxFunc2Entry(Function func) {
  initFuncEntryExit();
  if(Func2Entry.find(func) == Func2Entry.end()) {
    cout << "ERROR: cannot find record for function "<<func.get_name().getString()<<endl;
    cout << "Func2Entry:"<<endl;
    for(map<Function, CFGNode>::iterator i=Func2Entry.begin(); i!=Func2Entry.end(); ++i)
      cout << "    "<<i->first.str()<<": "<<CFGNode2Str(i->second)<<endl;
  }
  assert(Func2Entry.find(func) != Func2Entry.end());
  return Func2Entry[func];
}

CFGNode getStxFunc2Exit(Function func) {
  initFuncEntryExit();
  if(Func2Exit.find(func) == Func2Exit.end()) {
    cout << "ERROR: cannot find record for function "<<func.get_name().getString()<<endl;
    cout << "Func2Exit:"<<endl;
    for(map<Function, CFGNode>::iterator i=Func2Exit.begin(); i!=Func2Exit.end(); ++i)
      cout << "    "<<i->first.str()<<": "<<CFGNode2Str(i->second)<<endl;
  }
  assert(Func2Exit.find(func) != Func2Exit.end());
  return Func2Exit[func];
}

Function getStxEntry2Func(CFGNode entry) {
  initFuncEntryExit();
  if(Entry2Func.find(entry) == Entry2Func.end()) {
    cout << "ERROR: cannot find record for function "<<CFGNode2Str(entry)<<endl;
    cout << "Entry2Func:"<<endl;
    for(map<CFGNode, Function>::iterator i=Entry2Func.begin(); i!=Entry2Func.end(); ++i)
      cout << "    "<<CFGNode2Str(i->first)<<": "<<i->second.str()<<endl;
  }
  assert(Entry2Func.find(entry) != Entry2Func.end());
  return Entry2Func[entry];
}

Function getStxExit2Func(CFGNode exit) {
  initFuncEntryExit();
  if(Exit2Func.find(exit) == Exit2Func.end()) {
    cout << "ERROR: cannot find record for function "<<CFGNode2Str(exit)<<endl;
    cout << "Exit2Func:"<<endl;
    for(map<CFGNode, Function>::iterator i=Exit2Func.begin(); i!=Exit2Func.end(); ++i)
      cout << "    "<<CFGNode2Str(i->first)<<": "<<i->second.str()<<endl;
  }
  assert(Exit2Func.find(exit) != Exit2Func.end());
  return Exit2Func[exit];
}

bool isStxFuncEntry(CFGNode entry) {
  initFuncEntryExit();
  /*if(Entry2Exit.find(entry) == Entry2Exit.end()) {
    cout << "ERROR: cannot find record for node "<<CFGNode2Str(entry)<<endl;
    cout << "Entry2Exit:"<<endl;
    for(map<CFGNode, CFGNode>::iterator i=Entry2Exit.begin(); i!=Entry2Exit.end(); ++i)
      cout << "    "<<CFGNode2Str(i->first)<<": "<<CFGNode2Str(i->second)<<endl;
  }*/
  return Entry2Exit.find(entry) != Entry2Exit.end();
}

bool isStxFuncExit(CFGNode exit)  {
  initFuncEntryExit();
  /*if(Exit2Entry.find(exit) == Exit2Entry.end()) {
    cout << "ERROR: cannot find record for function "<<CFGNode2Str(exit)<<endl;
    cout << "Exit2Entry:"<<endl;
    for(map<CFGNode, CFGNode>::iterator i=Exit2Entry.begin(); i!=Exit2Entry.end(); ++i)
      cout << "    "<<CFGNode2Str(i->first)<<": "<<CFGNode2Str(i->second)<<endl;
  }*/
  return Exit2Entry.find(exit) != Exit2Entry.end();
}

CFGNode getStxEntry2Exit(CFGNode entry) {
  initFuncEntryExit();
  assert(isStxFuncEntry(entry));
  return Entry2Exit[entry];
}

CFGNode getStxExit2Entry(CFGNode exit) {
  initFuncEntryExit();
  assert(isStxFuncExit(exit));
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
  SIGHT_VERB_DECL(scope, (sight::txt()<<"getAllCalleeFuncs("<<SgNode2Str(call)<<")", scope::medium), 2, stxAnalysisDebugLevel)
  initFuncEntryExit();

  set<Function> callees;

  Function callee(call);
  SIGHT_VERB(dbg << "callee.isKnown()="<<callee.isKnown()<<endl, 2, stxAnalysisDebugLevel)

  // If the function being called is statically known
  if(callee.isKnown())
    //callees.insert(call);
    callees = Function::getCallees(call);
  // Otherwise, find all the functions with the same type as this function call.
  // They are all possible referents of the call
  // !!! NOTE: This should be updated to compute type compatibility rather than strict equality !!!
  else {
    for(map<Function, CFGNode>::iterator f=Func2Entry.begin(); f!=Func2Entry.end(); f++) {
      SIGHT_VERB(dbg << "f->first.get_type()="<<SgNode2Str(f->first.get_type())<<", call->get_function()->get_type()="<<SgNode2Str(call->get_function()->get_type())<<endl, 2, stxAnalysisDebugLevel)
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

#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(3, stxAnalysisDebugLevel)
      scope reg("func2AllCalls", scope::medium);
      for(map<Function, set<SgFunctionCallExp*> >::iterator i=func2AllCalls.begin(); i!=func2AllCalls.end(); i++) {
        dbg << i->first.get_name().getString() << " =&gt; "<<endl;
        indent ind;
        for(set<SgFunctionCallExp*>::iterator j=i->second.begin(); j!=i->second.end(); j++)
          dbg << SgNode2Str(*j) << endl;
      }
    SIGHT_VERB_FI()
#endif
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
//{ return boost::make_shared<StxValueObject>(n); }
{
  static map<SgNode*, StxValueObjectPtr> cache;
  map<SgNode*, StxValueObjectPtr>::iterator cLoc = cache.find(n);
  // If the given SgNode's StxValueObject is not in the cache, create it
  if(cLoc == cache.end()) {
    StxValueObjectPtr res = boost::make_shared<StxValueObject>(n);
    cache[n] = res;
    return res;

  // Otherwise, return the currently cached object
  } else
    return cLoc->second;
}

CodeLocObjectPtr SyntacticAnalysis::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge)
{ return SyntacticAnalysis::Expr2CodeLocStatic(n, pedge); }

CodeLocObjectPtr SyntacticAnalysis::Expr2CodeLocStatic(SgNode* n, PartEdgePtr pedge)
// GB: Not sure if this is correct since there may be multiple CFGNodes for each SgNode
{ return NULLCodeLocObject;/*boost::make_shared<CodeLocObject>(pedge->source(), CFGNode(n, 0));*/ }

// Maps the given SgNode to an implementation of the MemLocObject abstraction.
MemRegionObjectPtr SyntacticAnalysis::Expr2MemRegion(SgNode* n, PartEdgePtr pedge)
{ return Expr2MemRegionStatic(n, pedge); }

MemRegionObjectPtr SyntacticAnalysis::Expr2MemRegionStatic(SgNode* n, PartEdgePtr pedge)
{
  //struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);
  //scope s(sight::txt()<<"SyntacticAnalysis::Expr2MemRegionStatic()");
  //dbg << "n="<<n<<SgNode2Str(n)<<endl;
  //dbg << "pedge="<<pedge->str()<<endl;

  static map<SgNode*, StxMemRegionObjectPtr> cache;
  map<SgNode*, StxMemRegionObjectPtr>::iterator cLoc = cache.find(n);

  // If the given SgNode's StxMemRegionObject is not in the cache, create it
  if(cLoc == cache.end()) {
    StxMemRegionObjectPtr res = boost::make_shared<StxMemRegionObject>(n);
    //dbg << "Not found res="<<res->str()<<endl;
    cache[n] = res;
    //gettimeofday(&gopeEnd, NULL); cout << "                  SyntacticAnalysis::Expr2MemRegion not cached\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
    return res;

  // Otherwise, return the currently cached object
  } else {
    //dbg << "Found cLoc->second="<<cLoc->second->str()<<endl;
    //gettimeofday(&gopeEnd, NULL); cout << "                  SyntacticAnalysis::Expr2MemRegion cached\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
    return cLoc->second;
  }


  //return boost::make_shared<StxMemRegionObject>(n);
}

// Maps the given SgNode to an implementation of the MemLocObject abstraction.
MemLocObjectPtr SyntacticAnalysis::Expr2MemLoc(SgNode* n, PartEdgePtr pedge) {
  return Expr2MemLocStatic(n, pedge);
}
MemLocObjectPtr SyntacticAnalysis::Expr2MemLocStatic(SgNode* n, PartEdgePtr pedge) {
/*  StxMemRegionObjectPtr region = boost::make_shared<StxMemRegionObject>(n);
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
*/
  static map<SgNode*, MemLocObjectPtr> cache;
  map<SgNode*, MemLocObjectPtr>::iterator cLoc = cache.find(n);
  // If the given SgNode's StxMemLocObject is not in the cache, create it
  if(cLoc == cache.end()) {
    StxMemRegionObjectPtr region = boost::dynamic_pointer_cast<StxMemRegionObject>(Expr2MemRegionStatic(n, NULLPartEdge));
    assert(region);
    MemLocObjectPtr res;
    // If this is an expression or named memory region, we create a MemLocObject with a 0 index
    // since the syntactic analysis only generates such regions for the SgNodes that mark the
    // entire memory region in question rather than a sub-region (subsequent analyses do this to
    // get better precision)
    if(region->getType() == StxMemRegionType::expr ||
       region->getType() == StxMemRegionType::named)
      res = boost::make_shared<MemLocObject>(region, boost::make_shared<StxValueObject>(SageBuilder::buildIntVal(0)), n);
    // If this is an unknown memory region, we have no idea where inside the region we may be.
    // As such, create a MemLocObject with an unconstrained index
    else
      res = boost::make_shared<MemLocObject>(region, boost::make_shared<StxValueObject>((SgNode*)NULL), n);

    cache[n] = res;
    return res;

  // Otherwise, return the currently cached object
  } else
    return cLoc->second;
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
//        scope s(sight::txt()<<"declaration: "<<SgNode2Str(*d)<<" parent="<<(*d)->get_parent(), scope::medium, attrGE("stxAnalysisDebugLevel", 3));

        if(isSgVariableDeclaration(*d)) {
          //dbg << "definition: "<<(isSgVariableDeclaration(*d)->get_definition()? SgNode2Str(isSgVariableDeclaration(*d)->get_definition()): "NULL")<<endl;
#ifndef DISABLE_SIGHT
          SIGHT_VERB_IF(3, stxAnalysisDebugLevel)
            dbg << "begin="<<CFGNode2Str((*d)->cfgForBeginning())<<endl;
            dbg << "end="<<CFGNode2Str((*d)->cfgForEnd())<<endl;
          SIGHT_VERB_FI()
#endif
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
  // If the flag is set skip global declarations from libraries (/usr/include)
#ifdef FUSE_SKIP_LIB_GLOBAL_DECLS
  Rose::StringUtility::FileNameClassification classification;
  string appPath = Rose::StringUtility::stripPathFromFileName(SageInterface::getProject()->getAbsolutePathFileNames()[0]);
#endif
    
  for(set<SgVariableDeclaration*>::iterator d=SyntacticAnalysis::globalDeclarations.begin(); d!=SyntacticAnalysis::globalDeclarations.end(); d++) {
    // If the flag is set skip global declarations from libraries (/usr/include)
#ifdef FUSE_SKIP_LIB_GLOBAL_DECLS
    string lfilename = (*d)->get_file_info()->get_filename();
    classification = Rose::StringUtility::classifyFileName(lfilename, appPath);
    if(classification.isLibraryCode()) continue;
#endif    
    startStates.insert(StxPart::create((*d)->cfgForBeginning(), this, filter));
  }

  // If there are no global VariableDeclarations, the analysis entry points are the entries
  // into the non-static functions
  if(startStates.size()==0)
    addFunctionEntries(startStates, this);

  return startStates;
}

// Returns whether the given function can be called from outside the current compilation unit
bool isExternallyCallable(const Function& func) {
  return !isSgMemberFunctionDeclaration(func.get_declaration()) &&
         !SageInterface::isStatic(func.get_declaration()) &&
         !func.get_declaration()->get_file_info()->isCompilerGenerated() &&
         func.get_definition()->getAttribute("fuse:UnknownSideEffects")==NULL;
}

// Adds the entry points into all the non-static functions (can be called from the outside) to
// the given set
template <class ArgPartPtr>
void SyntacticAnalysis::addFunctionEntries(set<ArgPartPtr>& states, SyntacticAnalysis* analysis) {
  initFuncEntryExit();

    // If the flag is set add only main's Entry to the states and skip all other non-static functions
#ifdef FUSE_START_FROM_MAIN
    SgFunctionDeclaration* mainDecl = SageInterface::findMain(SageInterface::getProject());
    if(!mainDecl) cout << "ERROR: FUSE_START_FROM_MAIN is set and main() not found!" << endl;
    assert(mainDecl);
    Function mainFn(mainDecl);
    states.insert(makePtr<StxPart>(Func2Entry[mainFn], analysis, analysis->filter));
    return;
#endif

  for(map<Function, CFGNode>::iterator f=Func2Entry.begin(); f!=Func2Entry.end(); f++) {
    //dbg << f->first.get_name().getString()<<"() declaration="<<f->first.get_declaration()<<"="<<CFGNode2Str(f->first.get_declaration())<<", static="<<SageInterface::isStatic(f->first.get_declaration())<<", compgen="<<f->first.get_declaration()->get_file_info()->isCompilerGenerated()<<endl;
    //dbg << f->first.get_name().getString()<<"() definition="<<f->first.get_definition()<<"="<<CFGNode2Str(f->first.get_definition())<<", compgen="<<f->first.get_definition()->get_file_info()->isCompilerGenerated()<<", unknown="<<f->first.get_definition()->getAttribute("fuse:UnknownSideEffects")<<endl;

    if(isExternallyCallable(f->first)) {
      //dbg << "f->second="<<CFGNode2Str(f->second);
      states.insert(StxPart::create(f->second, analysis, analysis->filter));
    }
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
      endStates.insert(StxPart::create(ret, this, filter));
  }

  // Add main's ending point
  endStates.insert(StxPart::create(mainEnd, this, filter));*/

  // Return the entry points of all the non-static functions
  set<PartPtr> endStates;
  /*Function main(SageInterface::findMain(SageInterface::getFirstGlobalScope(SageInterface::getProject()))->get_definition());
  endStates.insert(StxPart::create(getStxFunc2Exit(main), this, filter));*/

  initFuncEntryExit();
  SIGHT_VERB_DECL(scope, ("EndAStates"), 3, stxAnalysisDebugLevel)

  SgFunctionDeclaration* mainDecl = SageInterface::findMain(SageInterface::getProject());  
  if(mainDecl) {
    Function mainFn(mainDecl);
    endStates.insert(StxPart::create(Func2Exit[mainFn], this, filter));
    return endStates;
  }

  for(map<Function, CFGNode>::iterator f=Func2Exit.begin(); f!=Func2Exit.end(); f++) {
    /*if(!SageInterface::isStatic(f->first.get_declaration()) &&
       !f->first.get_declaration()->get_file_info()->isCompilerGenerated()) {*/
    if(isExternallyCallable(f->first)) {
      SIGHT_VERB(dbg << CFGNode2Str(f->second)<<"()"<<endl, 3, stxAnalysisDebugLevel)
      endStates.insert(StxPart::create(f->second, this, filter));
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
// Caches the function contexts of all the SgNodes
std::map<SgNode*, StxFuncContextPtr> StxFuncContext::FC_cache;

StxFuncContext::StxFuncContext(CFGNode n) :
  func(Function(SageInterface::getEnclosingFunctionDeclaration(n.getNode()))),
  n(n)
{ }

// Returns the StxFuncContext associated with the given CFGNode, using a previously cached
// instance of the object, if possible
StxFuncContextPtr StxFuncContext::getContext(CFGNode n) {
  map<SgNode*, StxFuncContextPtr>::iterator fcIt = FC_cache.find(n.getNode());
  if(fcIt == FC_cache.end()) {
    StxFuncContextPtr ctxt = makePtr<StxFuncContext>(n);
    FC_cache[n.getNode()] = ctxt;
    return ctxt;
  } else
    return fcIt->second;
}

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

std::string StxFuncContext::str(std::string indent) const {
  ostringstream oss;
  oss << "[StxFuncContext: "<<func.str()<<"]";
  return oss.str();
}

/*******************
 ***** StxPart *****
 *******************/

// the Syntactic Analysis interesting filter
/*bool stxVirtualCFGFilter (CFGNode cfgn)
{
  scope s(sight::txt()<<"stxVirtualCFGFilter("<<CFGNode2Str(cfgn)<<")");
  SgNode * node = cfgn.getNode();
  assert (node != NULL) ;

  //Keep the last index for initialized names. This way the definition of the variable doesn't
  //propagate to its assign initializer.
  if (isSgInitializedName(node)) {
    dbg << "isSgInitializedName: "<<(cfgn == node->cfgForEnd())<<endl;
    return (cfgn == node->cfgForEnd());
  } else if(isSgFunctionParameterList(node)) {
    dbg << "isSgFunctionParameterList: "<<(cfgn.getIndex()==isSgFunctionParameterList(node)->get_args().size())<<endl;
    return cfgn.getIndex()==isSgFunctionParameterList(node)->get_args().size();
  } else if(isSgFunctionDefinition(node)) {
    dbg << "isSgFunctionDefinition: "<<(cfgn.getIndex()==3)<<endl;
    return cfgn.getIndex()==3;
  } else if(isSgVariableDeclaration(node)) {
    dbg << "isSgVariableDeclaration: "<<true<<endl;
    return true;
  } else if(isSgFunctionCallExp(node)) {
    dbg << "isSgFunctionCallExp: "<<(cfgn.getIndex()==2 || cfgn.getIndex()==3)<<endl;
    return (cfgn.getIndex()==2 || cfgn.getIndex()==3);
  } else {
    dbg << "Other: "<<(cfgn.isInteresting())<<endl;
    return (cfgn.isInteresting());
  }
}*/


// Parts must be created via static construction methods to make it possible to separately
// initialize them. This is needed to allow Parts to register themselves with global directories,
// a process that requires the creation of a shared pointer to themselves.

// Returns a valid StxPartPtr if the CFNode is interesting and a NULLStxPart otherwise.
StxPartPtr StxPart::create(CFGNode n, ComposedAnalysis* analysis, bool (*f) (CFGNode)) {
  // If the node is interesting according to the filter, return its corresponding StxPart
  if(f(n)) {
    StxPartPtr newPart(boost::shared_ptr<StxPart>(new StxPart(n, analysis, f)));
    newPart->init();
    return newPart;
  // Otherwise, return NULL
  } else
    return NULLStxPart;
}
StxPartPtr StxPart::create(const StxPart& part) {
  StxPartPtr newPart(boost::shared_ptr<StxPart>(new StxPart(part, defaultFilter/*stxVirtualCFGFilter*/)));
  newPart->init();
  return newPart;
}
StxPartPtr StxPart::create(const StxPartPtr& part) {
  StxPartPtr newPart(boost::shared_ptr<StxPart>(new StxPart(part, defaultFilter/*stxVirtualCFGFilter*/)));
  newPart->init();
  return newPart;
}
StxPartPtr StxPart::create(const StxPart& part,    bool (*f) (CFGNode)) {
  // If the node is interesting according to the filter, return its corresponding StxPart
  if(f(part.n)) {
    StxPartPtr newPart(boost::shared_ptr<StxPart>(new StxPart(part, f)));
    newPart->init();
    return newPart;
  // Otherwise, return NULL
  } else
    return NULLStxPart;
}
StxPartPtr StxPart::create(const StxPartPtr& part, bool (*f) (CFGNode)) {
  StxPartPtr newPart(boost::shared_ptr<StxPart>(new StxPart(part, f)));
  newPart->init();
  return newPart;
}

void StxPart::init() {
  Part::init();

  // Set the Input Part of this Part to be itself since the syntactic analysis
  // runs first and doesn't refine anything else
  setInputPart(shared_from_this());
}

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
  SIGHT_VERB(dbg << "makeClosureDF_rec: path: "<<CFGNode2Str(path.source())<<" ==&gt; "<<CFGNode2Str(path.target())<<endl, 3, stxAnalysisDebugLevel);

  // If the edge of the current path is not interesting
  if(!filter((path.*otherSide)())) {
    // Recurse to find its extensions that may be interesting
    vector<CFGEdge> extensions = ((path.*otherSide)().*closure)();
    //dbg << "otherSide="<<CFGNode2Str((path.*otherSide)())<<endl;
    for(vector<CFGEdge>::iterator e=extensions.begin(); e!=extensions.end(); e++) {
      SIGHT_VERB(dbg << "extension "<<CFGNode2Str(e->source())<<" ==&gt; "<<CFGNode2Str(e->target())<<endl, 3, stxAnalysisDebugLevel);

      /* // Skip edges from the start of a short-circuit operation (|| and &&) to its end
      if(isShortCircuitEdge(*e)) { continue; }*/

      SIGHT_VERB_DECL(indent, (), 3, stxAnalysisDebugLevel);
      CFGPath extension = (*merge)(path, *e);
      // Extend path with e to create the full extension of path
      makeClosureDF_rec(extension, allPaths, closure, otherSide, merge, filter);
    }
  } else {
    SIGHT_VERB(dbg << "Interesting\n", 3, stxAnalysisDebugLevel)
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
  SIGHT_VERB_DECL(scope, ("makeClosureDF", scope::medium), 3, stxAnalysisDebugLevel)
  SIGHT_VERB_DECL(indent, (), 3, stxAnalysisDebugLevel)
  set<CFGPath> allPaths;
  for(vector<CFGEdge>::const_iterator e=orig.begin(); e!=orig.end(); e++) {
    SIGHT_VERB(dbg << "edge "<<CFGNode2Str(e->source())<<" ==&gt; "<<CFGNode2Str(e->target())<<endl, 3, stxAnalysisDebugLevel)
    /* // Skip edges from the start of a short-circuit operation (|| and &&) to its end
    if(isShortCircuitEdge(*e)) { continue; }*/
    makeClosureDF_rec(*e, allPaths, closure, otherSide, merge, filter);
  }

  // Maps edges to bools. A map is used to enable efficient lookups to avoid inserting duplicate edges,
  // which may happen in situations like an if statement with empty true and false bodies.
  map<StxPartEdgePtr, bool> edges;

  for (set<CFGPath>::iterator i = allPaths.begin(); i != allPaths.end(); ++i) {
    SIGHT_VERB(dbg << "*i="<<CFGNode2Str(i->source())<<" ==&gt; "<<CFGNode2Str(i->target())<<endl, 3, stxAnalysisDebugLevel)

    // Only if the end node of the path is interesting
    if (filter(((*i).*otherSide)())) {
      //edges.push_back(/*boost::static_pointer_cast<PartEdge>(*/boost::make_shared<StxPartEdge>(*i, filter)/*)*/);
      //edges.push_back(StxPartEdge::create(*i, analysis, filter));
      StxPartEdgePtr newEdge = StxPartEdge::create(*i, analysis, filter);
      SIGHT_VERB(dbg << "newEdge="<<(newEdge? newEdge->str(): "NULL")<<endl, 3, stxAnalysisDebugLevel)
      if(newEdge && edges.find(newEdge) == edges.end()) edges[newEdge] = true;
    }
  }
  /*dbg << "makeClosure done: #edges=" << edges.size() << endl;
  for(map<StxPartEdgePtr, bool>::iterator e=edges.begin(); e!=edges.end(); e++)
    dbg << "    edge="<<e->first->str()<<endl;*/
  //for (list<StxPartEdgePtr>::iterator i = edges.begin(); i != edges.end(); ++i) {

/*  // Make sure that for each edge either the source or the target is interesting
  for (map<StxPartEdgePtr, bool>::iterator i = edges.begin(); i != edges.end(); ++i) {
    StxPartEdgePtr edge = i->first;
    assert(edge->source()->filterAny(filter)  ||
                edge->target()->filterAny(filter)); // at least one node is interesting
  }*/
  return edges;
}

map<StxPartEdgePtr, bool> StxPart::getOutEdges()
{
  SIGHT_VERB_DECL(scope, (sight::txt()<<"StxPart::getOutEdges() ret="<<CFGNode2Str(n), scope::medium), 2, stxAnalysisDebugLevel)
  map<StxPartEdgePtr, bool> vStx;
  SgFunctionCallExp* call;

  // If this is the end of a SgVariableDeclaration of a global variable
  if(isSgVariableDeclaration(n.getNode()) && n.getIndex()==1 && isSgGlobal(n.getNode()->get_parent())) {
    // The successors are the entry points of all the non-static functions
    assert(dynamic_cast<SyntacticAnalysis*>(analysis));
    //SyntacticAnalysis::addFunctionEntries(v, dynamic_cast<SyntacticAnalysis*>(analysis));
    set<StxPartPtr> entries;
    // If there is a main function transition in to that
    SgFunctionDeclaration* mainDecl = SageInterface::findMain(SageInterface::getProject());
    if(mainDecl) {
      Function mainFn(mainDecl);
      entries.insert(StxPart::create(Func2Entry[mainFn], analysis, analysis->filter));
    }
    else {      
      SyntacticAnalysis::addFunctionEntries(entries, dynamic_cast<SyntacticAnalysis*>(analysis));
    }
    for(set<StxPartPtr>::iterator e=entries.begin(); e!=entries.end(); e++) {
      //dbg << "Edge "<<CFGNode2Str(n)<<" to "<<CFGNode2Str((*e)->n)<<endl;
      StxPartEdgePtr edge = StxPartEdge::create(n, (*e)->n, analysis);
      if(edge) vStx[edge] = true;
    }
  // If current node is a function call, connect the call to the SgFunctionParameterList of the called function.
  } else if((call = isSgFunctionCallExp(n.getNode())) && n.getIndex()==2) {
    set<Function> callees = getAllCalleeFuncs(call);

#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(2, stxAnalysisDebugLevel)
      dbg << "type = "<<SgNode2Str(isSgFunctionCallExp(n.getNode())->get_type())<<", funcCall->get_function()="<<(isSgFunctionCallExp(n.getNode())->get_function()? SgNode2Str(isSgFunctionCallExp(n.getNode())->get_function()): "NULL")<<endl;
      dbg << "function = "<<SgNode2Str(isSgFunctionCallExp(n.getNode())->get_function())<<" function type="<<SgNode2Str(isSgFunctionCallExp(n.getNode())->get_function()->get_type())<<endl;
      scope sCallees("Callees", scope::low);
      for(set<Function>::iterator c=callees.begin(); c!=callees.end(); c++)
        dbg << c->str()<<" declaration="<<c->get_declaration()<<"="<<SgNode2Str(c->get_declaration())<<endl;
    SIGHT_VERB_FI()
#endif

    for(set<Function>::iterator c=callees.begin(); c!=callees.end(); c++) {
      StxPartEdgePtr edge = StxPartEdge::create(n, getStxFunc2Entry(*c), analysis);
      if(edge) vStx[edge] = true;
    }

    // If the callee function has a definition, connect this function call directly to the function's entry point
    /*if(callee.get_definition()) {
      assert(callee.get_params());
      //vStx[StxPartEdge::create(n, CFGNode(callee.get_params(), 1), analysis)] = true;
      vStx[StxPartEdge::create(n, getStxFunc2Entry(callee), analysis)] = true;

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
      //vStx[StxPartEdge::create(n, CFGNode(call, 3), analysis)] = true;
      vStx[StxPartEdge::create(n, getStxFunc2Entry(callee), analysis)] = true;
    }*/
  // If current node is the end of a function definition, connect it to all the calls of this function
  // !!! NOTE: we should be connecting it to all the function calls that match the calling signature
  //} else if(/*SgFunctionDefinition* def = */isSgFunctionDefinition(n.getNode())) {
  } else if(isStxFuncExit(n)) {
    Function func= getStxExit2Func(n);

    // If this is the synthesized exit node a function without a body
    /*if(isStxFuncExit(n)) func = getStxExit2Func(n);
    else                   func = Function(def);*/
    SIGHT_VERB(dbg << "Definition n="<<CFGNode2Str(n)<<" func="<<func.get_name().getString()<<" isStxFuncExit(n)="<<isStxFuncExit(n)<<endl, 2, stxAnalysisDebugLevel)

    const set<SgFunctionCallExp*>& calls = func2Calls(func);
#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << "#calls="<<calls.size()<<" Connecting n="<<CFGNode2Str(n)<<endl, 2, stxAnalysisDebugLevel)
    SIGHT_VERB_DECL(indent, (), 2, stxAnalysisDebugLevel)
#endif
    for(set<SgFunctionCallExp*>::const_iterator c=calls.begin(); c!=calls.end(); c++) {
      CFGNode callNode(*c, 3);
      StxPartEdgePtr edge = StxPartEdge::create(n, callNode, analysis, filter);
      if(edge) vStx[edge]=1;

      /*dbg << "To the successors of call "<<CFGNode2Str(callNode)<<endl;
      indent ind;
      // Connect the SgFunctionDefinition to the nodes that follow each call to it, using makeClosureDF() to skip
      // over any nodes that are filtered out.
      map<StxPartEdgePtr, bool> outvStx = makeClosureDF(callNode.outEdges(), &CFGNode::outEdges, &CFGPath::target, &mergePaths, filter, analysis);
      for(map<StxPartEdgePtr, bool>::iterator i=outvStx.begin(); i!=outvStx.end(); i++) {
        //dbg << i->first->source()->str() << " =&gt; "<< i->first->target()->str()<<endl;
        vStx[StxPartEdge::create(n, i->first->stxTarget()->n, analysis, filter)]=1;
      }*/
    }

  // If the current node is a return statement, connect it to the function's exit SgFunctionDefinition node
  } else if(SgReturnStmt* ret = isSgReturnStmt(n.getNode())) {
    Function func(SageInterface::getEnclosingFunctionDeclaration(ret));
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(2, stxAnalysisDebugLevel)
      dbg << "returning from func="<<func.str()<<endl;
      dbg << "Exit node="<<CFGNode2Str(getStxFunc2Exit(func))<<endl;
    SIGHT_VERB_FI()
#endif
    StxPartEdgePtr edge = StxPartEdge::create(n, getStxFunc2Exit(func), analysis);
    if(edge) vStx[edge] = true;

  } else if(isStxFuncEntry(n)) {
    // If this is the synthesized entry node to a function without a body, return the edge to its corresponding exit node
    /*if(isStxFuncEntry(n)) {
      vStx[StxPartEdge::create(n, getStxEntry2Exit(n),  analysis)] = true;
      return vStx;
    } else*/
    return makeClosureDF(n.outEdges(), &CFGNode::outEdges, &CFGPath::target, &mergePaths, filter, analysis);

  // If this is a valid source for edges
  } else if(StxPartEdge::isValidEdgeSource(n)) {
    return makeClosureDF(n.outEdges(), &CFGNode::outEdges, &CFGPath::target, &mergePaths, filter, analysis);
  }
//  dbg << "#vStx="<<vStx.size()<<endl;
  return vStx;
}

list<PartEdgePtr> StxPart::outEdges() {
  ostringstream oss;
  //dbg << "n=>"<<CFGNode2Str(n)<<endl;
  //scope reg(sight::txt()<<"StxPart::outEdges() part="<<str(), scope::medium);
  map<StxPartEdgePtr, bool> vStx = getOutEdges();

  list<PartEdgePtr> v;
  for(map<StxPartEdgePtr, bool>::iterator i=vStx.begin(); i!=vStx.end(); i++) {
    //dbg << "edge="<<(i->first? i->first->str(): "NULL")<<endl;
    v.push_back(dynamicPtrCast<PartEdge>(i->first));
  }

  //dbg << "#v="<<v.size()<<endl;
  return v;
}

list<StxPartEdgePtr> StxPart::outStxEdges() {
  //scope reg(sight::txt()<<"StxPart::outStxEdges() part="<<str(), scope::medium);
  map<StxPartEdgePtr, bool> vStx = getOutEdges();
  list<StxPartEdgePtr> v;
  for(map<StxPartEdgePtr, bool>::iterator i=vStx.begin(); i!=vStx.end(); i++)
    v.push_back(i->first);
  return v;
}

map<StxPartEdgePtr, bool> StxPart::getInEdges()
{
//  scope s("StxPart::getInEdges()");
  map<StxPartEdgePtr, bool> vStx;
  SgFunctionCallExp* call;

  // If current node is the return side of a function call, connect the call to the exit point of the called function.
  if((call = isSgFunctionCallExp(n.getNode())) && n.getIndex()==3) {
    Function callee(call);

    SIGHT_VERB(dbg << "StxPart::getInEdges() Return side of Call: callee="<<callee.str()<<" known="<<callee.isKnown()<<endl, 2, stxAnalysisDebugLevel);

    // If the function is known
    if(callee.isKnown()) {
      SIGHT_VERB(dbg << "exit="<<CFGNode2Str(getStxFunc2Exit(callee))<<endl, 2, stxAnalysisDebugLevel)
      vStx[StxPartEdge::create(getStxFunc2Exit(callee), n, analysis)] = true;
    // Otherwise, find all the functions with the same type as this function call.
    // They are all possible referents of the call
    // !!! NOTE: This should be updated to compute type compatibility rather than strict equality !!!
    } else {
      for(map<Function, CFGNode>::iterator f=Func2Exit.begin(); f!=Func2Exit.end(); f++) {
        if(f->first.get_type() == isSgFunctionCallExp(n.getNode())->get_function()->get_type())
          vStx[StxPartEdge::create(f->second, n, analysis)] = true;
      }
    }

  // If the current Node is the exit point of a function
  } else if(isStxFuncExit(n)) {
    Function func = getStxExit2Func(n);
    SIGHT_VERB(dbg << "Function Exit n="<<CFGNode2Str(n)<<" func="<<func.get_name().getString()<<endl, 2, stxAnalysisDebugLevel)

    // Connect it to the immediately preceding CFGNode
    vStx = makeClosureDF(n.inEdges(), &CFGNode::inEdges, &CFGPath::source, &mergePathsReversed, filter, analysis);

    SIGHT_VERB(dbg << "-------------#vStx="<<vStx.size()<<"---------------------"<<endl, 2, stxAnalysisDebugLevel)

    // Also connect it to all the SgReturnStmts in the function
    for(CFGIterator it(getStxFunc2Entry(func)); it!=CFGIterator::end(); it++) {
      if(isSgReturnStmt(it->getNode()) && it->getIndex()==1)
        vStx[StxPartEdge::create(*it, n, analysis)] = true;
    }
    SIGHT_VERB(dbg << "-------------#vStx="<<vStx.size()<<"---------------------"<<endl, 2, stxAnalysisDebugLevel)
  // If the current node is the entry point of a function
  } else if(isStxFuncEntry(n)) {
    Function func = getStxEntry2Func(n);

    SIGHT_VERB(dbg << "Function Entry n="<<CFGNode2Str(n)<<" func="<<func.get_name().getString()<<endl, 2, stxAnalysisDebugLevel)

    const set<SgFunctionCallExp*>& calls = func2Calls(func);
    SIGHT_VERB(dbg << "#calls="<<calls.size()<<" Connecting n="<<CFGNode2Str(n)<<endl, 2, stxAnalysisDebugLevel)
    SIGHT_VERB_DECL(indent, (), 2, stxAnalysisDebugLevel)
    for(set<SgFunctionCallExp*>::const_iterator c=calls.begin(); c!=calls.end(); c++) {
      CFGNode callNode(*c, 2);
      vStx[StxPartEdge::create(callNode, n, analysis, filter)]=1;
    }

    SgFunctionDeclaration* mdecl = func.get_declaration();
    if(SageInterface::isMain(mdecl)) {
      // Return the entry points into all the global VariableDeclarations
      set<PartPtr> startStates;
      for(set<SgVariableDeclaration*>::iterator d=SyntacticAnalysis::globalDeclarations.begin(); d!=SyntacticAnalysis::globalDeclarations.end(); d++)
        vStx[StxPartEdge::create((*d)->cfgForEnd(), n, analysis, filter)]=1;
    }
    else {
      SgFunctionDeclaration* mainDecl = SageInterface::findMain(SageInterface::getProject());
      // If this function can be called from the outside, and there is no main 
      // add incoming edges from all the global declarations
      if(isExternallyCallable(func) && !SageInterface::isMain(mainDecl)) {
        // Return the entry points into all the global VariableDeclarations
        set<PartPtr> startStates;
        for(set<SgVariableDeclaration*>::iterator d=SyntacticAnalysis::globalDeclarations.begin(); 
            d!=SyntacticAnalysis::globalDeclarations.end(); d++)
          vStx[StxPartEdge::create((*d)->cfgForEnd(), n, analysis, filter)]=1;
      }
    }
  } else {
    // If this is the starting point of a declaration of a global variable, do not add any incoming edges
    if(isSgVariableDeclaration(n.getNode()) && n.getIndex()==0 &&
       SyntacticAnalysis::globalDeclarations.find(isSgVariableDeclaration(n.getNode())) != SyntacticAnalysis::globalDeclarations.end())
    {
      SIGHT_VERB(dbg << "Beginning of declaration of global variable"<<endl, 2, stxAnalysisDebugLevel)
      return vStx;
    // If this is a valid target for edges
    } else if(StxPartEdge::isValidEdgeTarget(n)) {
      SIGHT_VERB(dbg << "Internal Node"<<endl, 2, stxAnalysisDebugLevel)
      return makeClosureDF(n.inEdges(), &CFGNode::inEdges, &CFGPath::source, &mergePathsReversed, filter, analysis);
    }
  }
  return vStx;
}

list<PartEdgePtr> StxPart::inEdges() {
  ostringstream oss;
  SIGHT_VERB_DECL(scope, (sight::txt()<<"StxPart::inEdges() part="<<str()), 2, stxAnalysisDebugLevel)
  map<StxPartEdgePtr, bool> vStx = getInEdges();

  SIGHT_VERB(dbg <<"#vStx="<<vStx.size()<<endl, 2, stxAnalysisDebugLevel)
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
    ret.insert(StxPart::create(CFGNode(n.getNode(), 3), analysis));
  else if(isSgFunctionCallExp(n.getNode()) && n.getIndex()==3)
    ret.insert(StxPart::create(CFGNode(n.getNode(), 2), analysis));

  return ret;
}

// If this Part corresponds to a function entry/exit, returns the set of Parts that contain
// its corresponding exit/entry, respectively.
set<PartPtr> StxPart::matchingEntryExitParts() const {
  set<PartPtr> ret;
  if(isSgFunctionDefinition(n.getNode()) && n.getIndex()==3) {
    SgFunctionParameterList* params = isSgFunctionDefinition(n.getNode())->get_declaration()->get_parameterList();
    ret.insert(StxPart::create(CFGNode(params, params->get_args().size()), analysis));
  } else if(isSgFunctionParameterList(n.getNode()) && n.getIndex()==isSgFunctionParameterList(n.getNode())->get_args().size()) {
    Function func(isSgFunctionParameterList(n.getNode()));
    assert(func.get_definition());
    ret.insert(StxPart::create(CFGNode(func.get_definition(), 3), analysis));
  }

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
  l.push_back(StxPart::create(operand->cfgForEnd(), analysis));
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
{ return StxPartEdge::create(getCFGNode(), n, analysis); } ///*NULLCFGNode*/SageInterface::getGlobalScope(n.getNode())->cfgForBeginning(), n); }

// Returns a PartEdgePtr, where the target is a wild-card part (NULLPart) and the source is this Part
PartEdgePtr StxPart::outEdgeToAny()
{ return StxPartEdge::create(n, getCFGNode(), analysis); } ///*NULLCFGNode*/SageInterface::getGlobalScope(n.getNode())->cfgForEnd()); }

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
  /*scope s("StxPart::less");
  dbg << "this="<<str()<<endl;
  dbg << "o="<<o->str()<<endl;
  dbg << "n="<<n.getNode()<<", o->n="<<dynamicPtrCast<StxPart>(o)->n.getNode()<<endl;
  dbg << "less="<<(n < dynamicPtrCast<StxPart>(o)->n)<<endl;*/
  assert(dynamicPtrCast<StxPart>(o).get());
  return n < dynamicPtrCast<StxPart>(o)->n;
}

std::string StxPart::str(std::string indent) const
{
  ostringstream oss;
  if(isNULLCFGNode(n.getNode())) oss << "[*]";
  else oss << "[StxPart:"<< CFGNode2Str(n)<<"]";//", analysis="<<analysis<<"]";
  return oss.str();
}

/***********************
 ***** StxPartEdge *****
 ***********************/
void StxPartEdge::init() {
  PartEdge::init();

  // Set the Input PartEdge of this PartEdge to be itself since the syntactic analysis
  // runs first and doesn't refine anything else
  setInputPartEdge(shared_from_this());
}

// Returns whether an edge from the given source to the given target corresponds to a valid
// edge in the Fuse portion of the Virtual CFG
bool StxPartEdge::isValidEdge(CFGNode src, CFGNode tgt) {
  /*scope s("StxPartEdge::isValidEdge()");
  dbg << "src="<<CFGNode2Str(src)<<endl;
  dbg << "tgt="<<CFGNode2Str(tgt)<<endl;*/
  //if(isSgFunctionParameterList(src.getNode()) && src.getIndex()==0) return false;
  //if(isSgFunctionParameterList(tgt.getNode())) return false;
  // Edge from one of a function's parameters to SgParameterList
  if(isSgInitializedName(src.getNode()) && isSgFunctionParameterList(tgt.getNode())) return false;
  // Edge from SgParameterList to one of a function's parameters
  if(isSgFunctionParameterList(src.getNode()) && src.getIndex()<src.getNode()->cfgIndexForEnd() &&
     isSgInitializedName(tgt.getNode()))
    return false;

  /*dbg << "stxVirtualCFGFilter(src)="<<stxVirtualCFGFilter(src)<<endl;
  dbg << "stxVirtualCFGFilter(tgt)="<<stxVirtualCFGFilter(tgt)<<endl;*/

  //return stxVirtualCFGFilter(src) && stxVirtualCFGFilter(tgt);
  //dbg << "defaultFilter(src)="<<defaultFilter(src)<<", defaultFilter(tgt)="<<defaultFilter(tgt)<<endl;
  return defaultFilter(src) && defaultFilter(tgt);
}

// Returns whether this CFGNode is a valid source for any edge in the Fuse portion of the Virtual CFG
bool StxPartEdge::isValidEdgeSource(CFGNode src) {
  return defaultFilter(src);
}

// Returns whether this CFGNode is a valid target for any edge in the Fuse portion of the Virtual CFG
bool StxPartEdge::isValidEdgeTarget(CFGNode tgt) {
  if(isSgFunctionParameterList(tgt.getNode())) return false;
  // Function parameter inside an SgParameterList
  if(isSgInitializedName(tgt.getNode()) && isSgFunctionParameterList(tgt.getNode()->get_parent())) return false;

  return defaultFilter(tgt);
}

// PartEdges must be created via static construction methods to make it possible to separately
// initialize them. This is needed to allow PartEdges to register themselves with global directories,
// a process that requires the creation of a shared pointer to themselves.
StxPartEdgePtr StxPartEdge::create(CFGNode src, CFGNode tgt, ComposedAnalysis* analysis, bool (*f) (CFGNode)) {
  if(!isValidEdge(src, tgt)) return NULLStxPartEdge;

  StxPartEdgePtr newEdge(boost::shared_ptr<StxPartEdge>(new StxPartEdge(src, tgt, analysis, f)));
  newEdge->init();
  return newEdge;
}
StxPartEdgePtr StxPartEdge::create(CFGPath p, ComposedAnalysis* analysis, bool (*f) (CFGNode)) {
  if(!isValidEdge(p.source(), p.target())) return NULLStxPartEdge;

  StxPartEdgePtr newEdge(boost::shared_ptr<StxPartEdge>(new StxPartEdge(p, analysis, f)));
  newEdge->init();
  return newEdge;
}
StxPartEdgePtr StxPartEdge::create(const StxPartEdge& dfe) {
  if(!isValidEdge(dfe.stxSource()->n, dfe.stxTarget()->n)) return NULLStxPartEdge;

  StxPartEdgePtr newEdge(boost::shared_ptr<StxPartEdge>(new StxPartEdge(dfe)));
  newEdge->init();
  return newEdge;
}

PartPtr StxPartEdge::source() const {
  return stxSource();
}

StxPartPtr StxPartEdge::stxSource() const {
  if(isNULLCFGNode(p.source().getNode())) return NULLPart;
  else return StxPart::create(p.source(), analysis, filter);
}

PartPtr StxPartEdge::target() const {
  return stxTarget();
}

StxPartPtr StxPartEdge::stxTarget() const {
  if(isNULLCFGNode(p.target().getNode())) return NULLPart;
  else return StxPart::create(p.target(), analysis, filter);
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
  l.push_back(StxPartEdge::create(opCFG, partTarget->n, analysis));
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

std::string StxPartEdge::str(std::string indent) const
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

StxValueObject::StxValueObject(SgNode* n) : ValueObject(n)//, AbstractionHierarchy()
{
  // If a valid node is passed, check if it is an SgValue
  if(n) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(2, stxAnalysisDebugLevel)
      dbg << "StxValueObject::StxValueObject("<<SgNode2Str(n)<<")";
      dbg << " isSgCastExp(n)="<<isSgCastExp(n)<<" unwrapCasts(isSgCastExp(n))="<<(isSgCastExp(n) ? SgNode2Str(unwrapCasts(isSgCastExp(n))) : "NULL")<<" iscast="<<(isSgCastExp(n) ? isSgValueExp(unwrapCasts(isSgCastExp(n))) : 0)<<endl;
    SIGHT_VERB_FI()
#endif
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

StxValueObject::StxValueObject(const StxValueObject& that) :
        ValueObject((const ValueObject&)that)/*, AbstractionHierarchy(that)*/, val(that.val)
{ }

bool StxValueObject::mayEqualAO(ValueObjectPtr that_arg, PartEdgePtr pedge)
{
  StxValueObjectPtr that = boost::dynamic_pointer_cast <StxValueObject> (that_arg);
  // ValueObject abstractions of different types may be equal to each other (can't tell either way)
  if(!that) { return true; }

  // If either object is not an SgValue, they may be equal to each other
  if(val==NULL || that->val==NULL) { return true; }

  // If both are SgValues, equalValExp makes a definitive precise comparison
  return equalValExp(val, that->val);
}

bool StxValueObject::mustEqualAO(ValueObjectPtr that_arg, PartEdgePtr pedge)
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
bool StxValueObject::equalSetAO(ValueObjectPtr that_arg, PartEdgePtr pedge)
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
bool StxValueObject::subSetAO(ValueObjectPtr that_arg, PartEdgePtr pedge)
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

// Returns true if the first SgValueExp epresents a value that is < the second and false otherwise
// If both SgValueExp denote different types, returns false
bool StxValueObject::lessValExp(SgValueExp* a, SgValueExp* b)
{
  if(isSgBoolValExp(a) && isSgBoolValExp(b))
    return isSgBoolValExp(a)->get_value() < isSgBoolValExp(b)->get_value();
  else if(isSgCharVal(a) && isSgCharVal(a))
    return isSgCharVal(a)->get_value() < isSgCharVal(b)->get_value();
  else if(isSgComplexVal(a) && isSgComplexVal(b))
    return lessValExp(isSgComplexVal(a)->get_real_value(), isSgComplexVal(b)->get_real_value()) ||
           (equalValExp(isSgComplexVal(a)->get_real_value(),      isSgComplexVal(b)->get_real_value()) &&
            lessValExp (isSgComplexVal(a)->get_imaginary_value(), isSgComplexVal(b)->get_imaginary_value()));
  else if(isSgDoubleVal(a) && isSgDoubleVal(b))
    return isSgDoubleVal(a)->get_value() < isSgDoubleVal(b)->get_value();
  else if(isSgEnumVal(a) && isSgEnumVal(b))
    return isSgEnumVal(a)->get_value() < isSgEnumVal(b)->get_value();
  else if(isSgFloatVal(a) && isSgFloatVal(b))
    return isSgFloatVal(a)->get_value() < isSgFloatVal(b)->get_value();
  else if(isSgIntVal(a) && isSgIntVal(b))
    return isSgIntVal(a)->get_value() < isSgIntVal(b)->get_value();
  else if(isSgLongDoubleVal(a) && isSgLongDoubleVal(b))
    return isSgLongDoubleVal(a)->get_value() < isSgLongDoubleVal(b)->get_value();
  else if(isSgLongIntVal(a) && isSgLongIntVal(b))
    return isSgLongIntVal(a)->get_value() < isSgLongIntVal(b)->get_value();
  else if(isSgLongLongIntVal(a) && isSgLongLongIntVal(b))
    return isSgLongLongIntVal(a)->get_value() < isSgLongLongIntVal(b)->get_value();
  else if(isSgShortVal(a) && isSgShortVal(b))
    return isSgShortVal(a)->get_value() < isSgShortVal(b)->get_value();
  else if(isSgStringVal(a) && isSgStringVal(b))
    return isSgStringVal(a)->get_value() < isSgStringVal(b)->get_value();
  else if(isSgUnsignedCharVal(a) && isSgUnsignedCharVal(b))
    return isSgUnsignedCharVal(a)->get_value() < isSgUnsignedCharVal(b)->get_value();
  else if(isSgUnsignedIntVal(a) && isSgUnsignedIntVal(b))
    return isSgUnsignedIntVal(a)->get_value() < isSgUnsignedIntVal(b)->get_value();
  /*else if(isSgUnsigedLongLongIntVal(a) && isSgUnsigedLongLongIntVal(b))
    return isSgUnsigedLongLongIntVal(a)->get_value() < isSgUnsigedLongLongIntVal(b)->get_value();*/
  else if(isSgUnsignedLongVal(a) && isSgUnsignedLongVal(b))
    return isSgUnsignedLongVal(a)->get_value() < isSgUnsignedLongVal(b)->get_value();
  else if(isSgUnsignedShortVal(a) && isSgUnsignedShortVal(b))
    return isSgUnsignedShortVal(a)->get_value() < isSgUnsignedShortVal(b)->get_value();
  /*else if(isSgUpcMythreadVal(a) && isSgUpcMythreadVal(b))
    return isSgUpcMythreadVal(a)->get_value() < isSgUpcMythreadVal(b)->get_value();
  else if(isSgUpcThreadsVal(a) && isSgUpcThreadsVal(b))
    return isSgUpcThreadsVal(a)->get_value() < isSgUpcThreadsVal(b)->get_value();*/
  else if(isSgWcharVal(a) && isSgWcharVal(b))
    return isSgWcharVal(a)->get_value() < isSgWcharVal(b)->get_value();
  else
    return false;
}

// Computes the meet of this and that and saves the result in this.
// Returns true if this causes this to change and false otherwise.
bool StxValueObject::meetUpdateAO(ValueObjectPtr that_arg, PartEdgePtr pedge)
{
  StxValueObjectPtr that = boost::dynamic_pointer_cast <StxValueObject> (that_arg);
  assert(that);

  // If the value objects denote different values
  if(!mustEqualAO(that, pedge)) {
    // Set the value pointer of this object to NULL since we cannot represent their union with a single value
    val = NULL;
    return true;
  }
  return false;
}

bool StxValueObject::isFullAO(PartEdgePtr pedge)
{ return val == NULL; }

bool StxValueObject::isEmptyAO(PartEdgePtr pedge)
{ return false; }


// Returns true if this ValueObject corresponds to a concrete value that is statically-known
bool StxValueObject::isConcrete()
{
  return val;
}

// Returns the number of concrete values in this set
int StxValueObject::concreteSetSize() {
  return (isConcrete()? 1: -1);
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
std::string StxValueObject::str(std::string indent) const { // pretty print for the object
  return "[StxValueObject: "+(val? val->unparseToString() : "NULL")+"]";
}

// Allocates a copy of this object and returns a pointer to it
ValueObjectPtr StxValueObject::copyAOType() const
{ return boost::make_shared<StxValueObject>(*this); }

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& StxValueObject::getHierKey() const {
  if(!isHierKeyCached) {
    ((StxValueObject*)this)->cachedHierKey = boost::make_shared<AOSHierKey>(((StxValueObject*)this)->shared_from_this());

    // The NULL val gets an empty key since it denotes the full set
    if(val==NULL) { }
    else
      ((StxValueObject*)this)->cachedHierKey->add(boost::make_shared<comparableSgValueExp>(val));

    ((StxValueObject*)this)->isHierKeyCached = true;
  }
  return cachedHierKey;
}


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

StxMemRegionObject::StxMemRegionObject(SgNode* n): MemRegionObject(n)/*, AbstractionHierarchy()*/ {
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

StxMemRegionObject::StxMemRegionObject(const StxMemRegionObject& that):
      MemRegionObject(that)/*, AbstractionHierarchy(that)*/, type(that.type)
{}

// Returns whether this object may/must be equal to o within the given Part p
// These methods are called by composers and should not be called by analyses.
bool StxMemRegionObject::mayEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge)
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

bool StxMemRegionObject::mustEqualAO(MemRegionObjectPtr o, PartEdgePtr pedge)
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

bool StxMemRegionObject::equalSetAO(MemRegionObjectPtr o, PartEdgePtr pedge)
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

bool StxMemRegionObject::subSetAO(MemRegionObjectPtr o, PartEdgePtr pedge)
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
bool StxMemRegionObject::isLiveAO(PartEdgePtr pedge)
{ return type->isLiveAO(pedge); }

// Returns a ValueObject that denotes the size of this memory region
ValueObjectPtr StxMemRegionObject::getRegionSizeAO(PartEdgePtr pedge)
{ return type->getRegionSizeAO(pedge); }

// Computes the meet of this and that and saves the result in this
// returns true if this causes this to change and false otherwise
bool StxMemRegionObject::meetUpdateAO(MemRegionObjectPtr o, PartEdgePtr pedge)
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
bool StxMemRegionObject::isFullAO(PartEdgePtr pedge)
{
  return getType() == StxMemRegionType::all;
}

// Returns whether this AbstractObject denotes the empty set.
bool StxMemRegionObject::isEmptyAO(PartEdgePtr pedge) {
  // StxMemRegionObjects are only created for non-empty sets of memory regions
  return false;
}


// Allocates a copy of this object and returns a pointer to it
MemRegionObjectPtr StxMemRegionObject::copyAOType() const {
  return boost::make_shared<StxMemRegionObject>(*this);
}

std::string StxMemRegionObject::str(std::string indent) const { // pretty print for the object
  return sight::txt() << "[StxMR: "<<type->str()<<"]";
}

// Returns a key that uniquely identifies this particular AbstractObject in the
// set hierarchy.
const AbstractionHierarchy::hierKeyPtr& StxMemRegionObject::getHierKey() const {
  if(!isHierKeyCached) {
    ((StxMemRegionObject*)this)->cachedHierKey = boost::make_shared<AOSHierKey>(((StxMemRegionObject*)this)->shared_from_this());

    // The all object gets an empty key since it contains all the object types
    if(getType()==StxMemRegionType::all) { }
    else {
      // Named objects are a sub-set of storage so prepend an extra storage type to them both
      if(getType()==StxMemRegionType::named || getType()==StxMemRegionType::storage)
        ((StxMemRegionObject*)this)->cachedHierKey->add(boost::make_shared<StxMemRegionType::comparableType>(StxMemRegionType::storage));
      else if(getType()==StxMemRegionType::expr)
        ((StxMemRegionObject*)this)->cachedHierKey->add(boost::make_shared<StxMemRegionType::comparableType>(StxMemRegionType::expr));
      //((StxMemRegionObject*)this)->cachedHierKey->add(boost::make_shared<StxMemRegionType::comparableType>(getType()));

      type->addHierSubKey(((StxMemRegionObject*)this)->cachedHierKey);
//std::cout << "key="<<((StxMemRegionObject*)this)->cachedHierKey<<", this="<<str()<<std::endl;
    }
    ((StxMemRegionObject*)this)->isHierKeyCached = true;
  }
  return ((StxMemRegionObject*)this)->cachedHierKey;
}

StxExprMemRegionTypePtr NULLStxExprMemRegionType;

// If the given SgNode corresponds to a named memory region, returns a freshly-allocated
// StxExprMemRegionType that represents it. Otherwise, returns NULL.
StxExprMemRegionTypePtr StxExprMemRegionType::getInstance(SgNode* n) {
  if(SgExpression* expr = isSgExpression(n)) {
    // if(!isSgVarRefExp(n) && !isSgDotExp(n) && !isSgPntrArrRefExp(n))
    if(!isSgVarRefExp(n) && !isSgDotExp(n) && !isSgPntrArrRefExp(n) && !isSgPointerDerefExp(n))
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
  // expr is in-scope at n if they're inside the same statement or n is an SgIfStmt, SgForStatement, SgWhileStmt
  // or SgDoWhileStmt and exprr is inside its sub-statements
  SIGHT_VERB(dbg
             << "expr=" << SgNode2Str(expr) << ",n="
             << CFGNode2Str(n) << std::endl,
             1, stxAnalysisDebugLevel);
  SIGHT_VERB(dbg
             << "Enclosing(expr)="
             << SgNode2Str(SageInterface::getEnclosingStatement(expr)) << "\n"
             << "Enclosing(n)="
             << SgNode2Str(SageInterface::getEnclosingStatement(n.getNode()))
             << std::endl,
             1, stxAnalysisDebugLevel);
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
bool StxExprMemRegionType::isLiveAO(PartEdgePtr pedge) {
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
  /*scope s("StxExprMemRegionType::isLiveMR");
  dbg << "pedge="<<pedge->str()<<endl;
  dbg << "expr="<<SgNode2Str(expr)<<endl;
  if(pedge->source()) dbg << "is at source="<<pedge->source()->mapCFGNodeANY<bool>(boost::bind(enc, expr, _1))<<endl;
  if(pedge->target()) dbg << "is at target="<<pedge->target()->mapCFGNodeANY<bool>(boost::bind(enc, expr, _1))<<endl;*/
  SIGHT_VERB_DECL(scope, ("StxExprMemRegionType::isLiveAO", scope::medium), 1, stxAnalysisDebugLevel);
  SIGHT_VERB(dbg
             << "pedge=" << pedge->str(),
             1, stxAnalysisDebugLevel);
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

// Returns a ValueObject that denotes the size of this memory region
ValueObjectPtr StxExprMemRegionType::getRegionSizeAO(PartEdgePtr pedge)
{
  return getTypeSize(expr->get_type());
}

std::string StxExprMemRegionType::str(std::string indent) const { // pretty print for the object
  return sight::txt() << "Expr: "<<SgNode2Str(expr);
}

StxNamedMemRegionTypePtr NULLStxNamedMemRegionType;

// If the given SgNode corresponds to a named memory region, returns a freshly-allocated
// StxNamedMemRegionType that represents it. Otherwise, returns NULL.
StxNamedMemRegionTypePtr StxNamedMemRegionType::getInstance(SgNode* n) {
  if(SgVarRefExp* ref = isSgVarRefExp(n)) {
    return boost::make_shared<StxNamedMemRegionType>(ref->get_symbol()->get_declaration(), ref->get_symbol());
  } else if(SgCastExp* cast = isSgCastExp(n)) {
    // Unwrap the casts until we hit a non-cast expression
    SgExpression* operand = NULL;
    do {
      operand = cast->get_operand();
    } while((cast=isSgCastExp(cast->get_operand())));

    if(SgVarRefExp* ref = isSgVarRefExp(operand))
      return boost::make_shared<StxNamedMemRegionType>(ref->get_symbol()->get_declaration(), ref->get_symbol());
  } else if(SgInitializedName* iname = isSgInitializedName(n)) {
    return boost::make_shared<StxNamedMemRegionType>(iname, iname->search_for_symbol_from_symbol_table());
  }

  return NULLStxNamedMemRegionType;
}

// Return whether there exists a CFGNode within this part that is inside the function in which the anchor symbol
// is defined.
bool matchAnchorPart(SgScopeStatement* scopeStmt, const CFGNode& n) {
  /*scope s("matchAnchorPart");
  dbg << "n="<<CFGNode2Str(n)<<endl;*/
  SgScopeStatement* part_scope = SageInterface::getScope(n.getNode());
/*  dbg << "part_scope="<<SgNode2Str(part_scope)<<endl;
  dbg << "equal="<<(scopeStmt == part_scope)<<", isAncestor="<<SageInterface::isAncestor(scopeStmt, part_scope)<<endl;*/
  assert(part_scope);
  if(scopeStmt == part_scope)
    return true;
  else
    return SageInterface::isAncestor(scopeStmt, part_scope);
}

// Returns true if this object is live at the given part and false otherwise
bool StxNamedMemRegionType::isLiveAO(PartEdgePtr pedge) {
  //scope s(sight::txt()<<"StxNamedMemRegionType::isLiveAO("<<pedge->str()<<")");

  if(iname) {
//    dbg << "iname="<<SgNode2Str(iname)<<endl;
    // This variable is in-scope if part.getNode() is inside the scope that contains its declaration
    SgScopeStatement* scopeStmt=NULL;
    assert(symbol==NULL || isSgVariableSymbol(symbol) || isSgFunctionSymbol(symbol));
    /*if(isSgVariableSymbol(symbol))
      scope = isSgVariableSymbol(symbol)->get_declaration()->get_declaration()->get_scope();
    else if(isSgFunctionSymbol(symbol))
      scope = isSgFunctionSymbol(symbol)->get_declaration()->get_scope();*/
    scopeStmt = iname->get_scope();
    /*scope s("StxNamedMemRegionType::isLiveMR");
    dbg << "iname="<<SgNode2Str(iname)<<endl;
    dbg << "scopeStmt="<<SgNode2Str(scopeStmt)<<endl;*/

    assert(scopeStmt);
//    dbg << "symbol="<<SgNode2Str(symbol)<<endl;
//    dbg << "scopeStmt="<<SgNode2Str(scopeStmt)<<endl;

    if(symbol!=NULL && isSgFunctionSymbol(symbol)) return true;
    else if(symbol==NULL || isSgVariableSymbol(symbol)) {
      //dbg << "symbol="<<SgNode2Str(symbol)<<" pedge="<<pedge->str()<<endl;
      // GB 2012-10-18 - I'm not sure what to do here about edges with wildcard sources or targets.
      //                 It seems like to be fully general we need to say that something is live if it is live at
      //                 any source and any destination, meaning that we need consider all the outcomes of a wildcard.
      //                 For example, what happens when an edge may cross a scope boundary for one but not all
      //                 of the wildcard outcomes?
      return (pedge->source() ? pedge->source()->mapCFGNodeANY<bool>(boost::bind(&matchAnchorPart, scopeStmt, _1)) : false) ||
             (pedge->target() ? pedge->target()->mapCFGNodeANY<bool>(boost::bind(&matchAnchorPart, scopeStmt, _1)) : false);
    } else
      return false;
  } else
    return true;

  /*scope reg(string("NamedObj::isLiveAO(")+symbol->get_name().getString()+string(")")+string(isSgFunctionSymbol(symbol) |
  dbg << "anchorFD=";
  if(anchorFD) dbg << "["<<anchorFD->unparseToString()<<" | "<<anchorFD->class_name()<<"]"<<endl;
  else         dbg << "SgFunctionSymbol"<<endl;
  dbg << "partFD=["<<partFD->unparseToString()<<" | "<<partFD->class_name()<<"]"<<endl;
  dbg << "part=["<<part.getNode()->unparseToString()<<" | "<<part.getNode()->class_name()<<"]"<<endl;*/
}

// Returns a ValueObject that denotes the size of this memory region
ValueObjectPtr StxNamedMemRegionType::getRegionSizeAO(PartEdgePtr pedge)
{
  return getTypeSize(iname->get_type());
}

std::string StxNamedMemRegionType::str(std::string indent) const { // pretty print for the object
  return sight::txt() << "Named: "<<(symbol? SgNode2Str(symbol): SgNode2Str(iname));
}

// Returns a freshly-allocated All memory region.
StxStorageMemRegionTypePtr StxStorageMemRegionType::getInstance(SgNode* n) {
  return boost::make_shared<StxStorageMemRegionType>();
}

// Returns a ValueObject that denotes the size of this memory region
ValueObjectPtr StxStorageMemRegionType::getRegionSizeAO(PartEdgePtr pedge)
{
  // Return the Full value
  return boost::make_shared<StxValueObject>((SgNode*)NULL);
}

std::string StxStorageMemRegionType::str(std::string indent) const { // pretty print for the object
  return "Storage";
}

// Returns a freshly-allocated All memory region.
StxAllMemRegionTypePtr StxAllMemRegionType::getInstance(SgNode* n) {
  return boost::make_shared<StxAllMemRegionType>();
}

// Returns a ValueObject that denotes the size of this memory region
ValueObjectPtr StxAllMemRegionType::getRegionSizeAO(PartEdgePtr pedge)
{
  // Return the Full value
  return boost::make_shared<StxValueObject>((SgNode*)NULL);
}

std::string StxAllMemRegionType::str(std::string indent) const { // pretty print for the object
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

/*********************
 ***** Utilities *****
 *********************/

// Given a vector for base VirtualCFG Nodes, get the corresponding refined Pfrom
// this composer and add them to the given set of refined Parts
void collectRefinedNodes(Composer* composer, std::set<PartPtr>& refined, const std::set<CFGNode>& base) {
  //scope s(sight::txt()<<"collectRefinedNodes() #b="<<base.size());
  for(std::set<CFGNode>::const_iterator b=base.begin(); b!=base.end(); b++) {
    StxPartPtr basePart = StxPart::create(*b, SyntacticAnalysis::instance());
    // If there's a valid syntactic node at this CFGNode
    if(basePart) {
      const std::set<PartPtr>& curRefined = composer->getRefinedParts(basePart);
      //dbg << "    basePart="<<basePart->str()<<", #curRefined="<<curRefined.size()<<endl;
      refined.insert(curRefined.begin(), curRefined.end());
    }
  }
  //assert(refined.size()>0);
}

// Given a vector for base VirtualCFG edges, get the corresponding refined edges from
// this composer and add them to the given set of refined edges
void collectRefinedEdges(Composer* composer, std::set<PartEdgePtr>& refined, const std::set<CFGEdge>& base) {
  //dbg << "collectRefinedEdges() #b="<<base.size()<<endl;
  for(std::set<CFGEdge>::const_iterator b=base.begin(); b!=base.end(); b++) {
    //dbg << "    b="<<CFGEdge2Str(*b)<<endl;

    // If either end of the edge is a SgGlobal, skip it
    if(isSgGlobal(b->source().getNode()) || isSgGlobal(b->target().getNode())) continue;
    // Edges between a SgFunctionDefinition | 0 and SgParameterList are skipped
    if(isSgFunctionDefinition(b->source().getNode()) && b->source().getIndex()==0) continue;
    // Edges after a SgFunctionDefinition | 3 are skipped
    if(isSgFunctionDefinition(b->source().getNode()) && b->source().getIndex()==3) continue;

    StxPartPtr stxSource = StxPart::create(b->source(), SyntacticAnalysis::instance());
    // If there's a valid syntactic node at this CFGNode
    if(stxSource) {
      StxPartEdgePtr stxEdge = StxPartEdge::create(*b, SyntacticAnalysis::instance());
      // If there's a valid syntactic edge at this CFGEdge
      if(stxEdge) {
        //dbg << "stxEdge="<<stxEdge->str()<<endl;
        // Check whether this is a valid edge in the Syntactic ATS and if so, add its refinements to refinedEdges
        list<StxPartEdgePtr> out = stxSource->outStxEdges();
        for(list<StxPartEdgePtr>::iterator e=out.begin(); e!=out.end(); ++e) {
          //dbg << "    curOutEdge(equal="<<(*e == stxEdge)<<")="<<(*e)->str()<<endl;
          if(*e == stxEdge) {
            const std::set<PartEdgePtr>& refinedEdges = composer->getRefinedPartEdges(stxEdge);
            //dbg << "    #refinedEdges="<<refinedEdges.size()<<endl;
            refined.insert(refinedEdges.begin(), refinedEdges.end());
            break;
          }
        }
      }
    }
  }
  //assert(refined.size()>0);
}

// Given CFGNode, get the refined edges that correspond to its incoming edges from
// this attributes composer and add them to the given set of refined edges
void collectIncomingRefinedEdges(Composer* composer, std::set<PartEdgePtr>& refined, const CFGNode& base) {
  StxPartPtr stxBase = StxPart::create(base, SyntacticAnalysis::instance());
  list<PartEdgePtr> stxEdges = stxBase->inEdges();
  for(list<PartEdgePtr>::iterator e=stxEdges.begin(); e!=stxEdges.end(); ++e) {
    const std::set<PartEdgePtr>& refinedEdges = composer->getRefinedPartEdges(*e);
    refined.insert(refinedEdges.begin(), refinedEdges.end());
  }
}

// Given CFGNode, get the refined edges that correspond to its outgoing edges from
// this attributes composer and add them to the given set of refined edges
void collectOutgoingRefinedEdges(Composer* composer, std::set<PartEdgePtr>& refined, const CFGNode& base) {
  StxPartPtr stxBase = StxPart::create(base, SyntacticAnalysis::instance());
  list<PartEdgePtr> stxEdges = stxBase->outEdges();
  for(list<PartEdgePtr>::iterator e=stxEdges.begin(); e!=stxEdges.end(); ++e) {
    const std::set<PartEdgePtr>& refinedEdges = composer->getRefinedPartEdges(*e);
    refined.insert(refinedEdges.begin(), refinedEdges.end());
  }
}

// Returns the number of bytes an instance of the given SgType occupies
StxValueObjectPtr getTypeSize(SgType* type) {
  // Maps integer sizes of types to StxValueObject that encode them. This caches the few possible
  // return values of this function.
  static map<int, StxValueObjectPtr> cache;

  switch(type->variantT()) {
    case V_SgArrayType: case V_SgFunctionType: case V_SgJavaWildcardType: case V_SgModifierType: case V_SgTemplateType:
    case V_SgTypeCAFTeam: case V_SgTypeCrayPointer: case V_SgTypeDefault: case V_SgTypeEllipse: case V_SgTypeGlobalVoid:
    case V_SgTypeLabel: case V_SgTypeString: case V_SgTypeUnknown: case V_SgTypeVoid:
      // The unkown size StxValueObject is mapped under -1
      if(cache.find(-1) == cache.end()) { cache[-1] = boost::make_shared<StxValueObject>((SgNode*)NULL); }
      return cache[-1];

    case V_SgTypeComplex: case V_SgTypeLongDouble:
    case V_SgTypeLongLong: case V_SgTypeSignedLongLong: case V_SgTypeUnsignedLongLong:
      if(cache.find(16) == cache.end()) { cache[16] = boost::make_shared<StxValueObject>(buildIntVal(16)); }
      return cache[16];

    case V_SgPointerType: case V_SgReferenceType: case V_SgTypeDouble: case V_SgTypeImaginary:
    case V_SgTypeLong: case V_SgTypeSignedLong: case V_SgTypeUnsignedLong:
      if(cache.find(8) == cache.end()) { cache[8] = boost::make_shared<StxValueObject>(buildIntVal(8)); }
      return cache[8];

    case V_SgTypeFloat: case V_SgTypeInt: case V_SgTypeSignedInt: case V_SgTypeUnsignedInt:
    case V_SgTypeShort: case V_SgTypeSignedShort: case V_SgTypeUnsignedShort:
      if(cache.find(4) == cache.end()) { cache[4] = boost::make_shared<StxValueObject>(buildIntVal(4)); }
      return cache[4];

    case V_SgTypeBool: case V_SgTypeChar: case V_SgTypeSignedChar: case V_SgTypeUnsignedChar: case V_SgTypeWchar:
      if(cache.find(1) == cache.end()) { cache[1] = boost::make_shared<StxValueObject>(buildIntVal(1)); }
      return cache[1];

    case V_SgNamedType:
      {
        SgDeclarationStatement* decl = isSgNamedType(type)->get_declaration();
        ROSE_ASSERT(isSgTypedefDeclaration(decl));
        return getTypeSize(isSgTypedefDeclaration(decl)->get_base_type());
      }

    case V_SgQualifiedNameType:
      return getTypeSize(isSgQualifiedNameType(type)->get_base_type());

    default:
      return boost::make_shared<StxValueObject>((SgNode*)NULL);
      //ROSE_ASSERT(0);
  }
}

/****************************
 ***** comparableSgValue *****
 ****************************/

comparableSgValueExp::comparableSgValueExp(SgValueExp* v): v(v) {}

// This == That
bool comparableSgValueExp::equal(const comparable& that_arg) const {
  const comparableSgValueExp& that = dynamic_cast<const comparableSgValueExp&>(that_arg);
  if(v->variantT() != that.v->variantT()) return false;
  return StxValueObject::equalValExp(v, that.v);
}

// This < That
bool comparableSgValueExp::less(const comparable& that_arg) const {
  const comparableSgValueExp& that = dynamic_cast<const comparableSgValueExp&>(that_arg);
  if(v->variantT() != that.v->variantT()) return false;
  return StxValueObject::lessValExp(v, that.v);
}
std::string comparableSgValueExp::str(std::string indent) const { return SgNode2Str(v); }

}; // namespace fuse
