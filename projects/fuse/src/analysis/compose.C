#include "sage3basic.h"
#include "compose.h"
#include "const_prop_analysis.h"
#include <boost/enable_shared_from_this.hpp>
//#include "printAnalysisStates.h"
#include "saveDotAnalysis.h"
#include "stx_analysis.h"
#include "sight.h"
#include <set>

using namespace std;
using namespace sight;

using namespace boost;
namespace fuse
{
DEBUG_LEVEL(composerDebugLevel, 0);

//--------------------
//----- Composer -----
//--------------------

Composer::Composer()
{
  //domInit = false; 
}

// Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
// Wrapper for calling type-specific versions of mayEqual without forcing the caller to care about the type of objects.
bool Composer::mayEqual(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge, ComposedAnalysis* client)
{
  ValueObjectPtr val1 = boost::dynamic_pointer_cast<ValueObject>(ao1);
  if(val1) { 
    ValueObjectPtr val2 = boost::dynamic_pointer_cast<ValueObject>(ao2);
    assert(val2);
    return mayEqualV(val1, val2, pedge, client);
  }
  
  /*CodeLocObjectPtr cl1 = boost::dynamic_pointer_cast<CodeLocObject>(ao1);
  if(cl1) { 
    CodeLocObjectPtr cl2 = boost::dynamic_pointer_cast<CodeLocObject>(ao2);
    assert(cl2);
    return mayEqualCL(cl1, cl2, pedge, client);
  }*/

  MemRegionObjectPtr mr1 = boost::dynamic_pointer_cast<MemRegionObject>(ao1);
  if(mr1) { 
    MemRegionObjectPtr mr2 = boost::dynamic_pointer_cast<MemRegionObject>(ao2);
    assert(mr2);
    return mayEqualMR(mr1, mr2, pedge, client);
  }

  /*MemLocObjectPtr ml1 = boost::dynamic_pointer_cast<MemLocObject>(ao1);
  if(ml1) { 
    MemLocObjectPtr ml2 = boost::dynamic_pointer_cast<MemLocObject>(ao2);
    assert(ml2);
    return mayEqualML(ml1, ml2, pedge, client);
  }*/
  
  assert(0);
}

// Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
// Wrapper for calling type-specific versions of mustEqual without forcing the caller to care about the type of objects.
bool Composer::mustEqual(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge, ComposedAnalysis* client)
{
  //dbg << "Composer::mustEqual"<<endl;
  ValueObjectPtr val1 = boost::dynamic_pointer_cast<ValueObject>(ao1);
  if(val1) { 
    ValueObjectPtr val2 = boost::dynamic_pointer_cast<ValueObject>(ao2);
    assert(val2);
    return mustEqualV(val1, val2, pedge, client);
  }
  
  /*CodeLocObjectPtr cl1 = boost::dynamic_pointer_cast<CodeLocObject>(ao1);
  if(cl1) { 
    CodeLocObjectPtr cl2 = boost::dynamic_pointer_cast<CodeLocObject>(ao2);
    assert(cl2);
    return mustEqualCL(cl1, cl2, pedge, client);
  }*/
  
  MemRegionObjectPtr mr1 = boost::dynamic_pointer_cast<MemRegionObject>(ao1);
  if(mr1) { 
    MemRegionObjectPtr mr2 = boost::dynamic_pointer_cast<MemRegionObject>(ao2);
    assert(mr2);
    return mustEqualMR(mr1, mr2, pedge, client);
  }
  
  /*MemLocObjectPtr ml1 = boost::dynamic_pointer_cast<MemLocObject>(ao1);
  if(ml1) { 
    MemLocObjectPtr ml2 = boost::dynamic_pointer_cast<MemLocObject>(ao2);
    assert(ml2);
    return mustEqualML(ml1, ml2, pedge, client);
  }*/
  
  assert(0);
}

// Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
// Wrapper for calling type-specific versions of equalSet without forcing the caller to care about the type of objects.
bool Composer::equalSet(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge, ComposedAnalysis* client)
{
  ValueObjectPtr val1 = boost::dynamic_pointer_cast<ValueObject>(ao1);
  if(val1) { 
    ValueObjectPtr val2 = boost::dynamic_pointer_cast<ValueObject>(ao2);
    assert(val2);
    return equalSetV(val1, val2, pedge, client);
  }
  
  /*CodeLocObjectPtr cl1 = boost::dynamic_pointer_cast<CodeLocObject>(ao1);
  if(cl1) { 
    CodeLocObjectPtr cl2 = boost::dynamic_pointer_cast<CodeLocObject>(ao2);
    assert(cl2);
    return equalSetCL(cl1, cl2, pedge, client);
  }*/

  MemRegionObjectPtr mr1 = boost::dynamic_pointer_cast<MemRegionObject>(ao1);
  if(mr1) { 
    MemRegionObjectPtr mr2 = boost::dynamic_pointer_cast<MemRegionObject>(ao2);
    assert(mr2);
    return equalSetMR(mr1, mr2, pedge, client);
  }

  /*MemLocObjectPtr ml1 = boost::dynamic_pointer_cast<MemLocObject>(ao1);
  if(ml1) { 
    MemLocObjectPtr ml2 = boost::dynamic_pointer_cast<MemLocObject>(ao2);
    assert(ml2);
    return equalSetML(ml1, ml2, pedge, client);
  }*/
  
  assert(0);
}

// Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
// Wrapper for calling type-specific versions of subSet without forcing the caller to care about the type of objects.
bool Composer::subSet(AbstractObjectPtr ao1, AbstractObjectPtr ao2, PartEdgePtr pedge, ComposedAnalysis* client)
{
  //dbg << "Composer::subSet"<<endl;
  ValueObjectPtr val1 = boost::dynamic_pointer_cast<ValueObject>(ao1);
  if(val1) { 
    ValueObjectPtr val2 = boost::dynamic_pointer_cast<ValueObject>(ao2);
    assert(val2);
    return subSetV(val1, val2, pedge, client);
  }
  
  /*CodeLocObjectPtr cl1 = boost::dynamic_pointer_cast<CodeLocObject>(ao1);
  if(cl1) { 
    CodeLocObjectPtr cl2 = boost::dynamic_pointer_cast<CodeLocObject>(ao2);
    assert(cl2);
    return subSetCL(cl1, cl2, pedge, client);
  }*/
  
  MemRegionObjectPtr mr1 = boost::dynamic_pointer_cast<MemRegionObject>(ao1);
  if(mr1) { 
    MemRegionObjectPtr mr2 = boost::dynamic_pointer_cast<MemRegionObject>(ao2);
    assert(mr2);
    return subSetMR(mr1, mr2, pedge, client);
  }
  
  /*MemLocObjectPtr ml1 = boost::dynamic_pointer_cast<MemLocObject>(ao1);
  if(ml1) { 
    MemLocObjectPtr ml2 = boost::dynamic_pointer_cast<MemLocObject>(ao2);
    assert(ml2);
    return subSetML(ml1, ml2, pedge, client);
  }*/
  
  assert(0);
}

// Returns whether the given AbstractObject is live at the given PartEdge
// This version is a wrapper for calling type-specific versions of isLive without forcing the caller to
// care about the type of object
bool Composer::isLive(AbstractObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{
  ValueObjectPtr val = boost::dynamic_pointer_cast<ValueObject>(ao);
  if(val) return isLiveV(val, pedge, client);
  
  /*CodeLocObjectPtr cl = boost::dynamic_pointer_cast<CodeLocObject>(ao);
  if(cl) return isLiveCL(cl, pedge, client);*/
  
  MemRegionObjectPtr mr = boost::dynamic_pointer_cast<MemRegionObject>(ao);
  if(mr) return isLiveMR(mr, pedge, client);
  
  /*MemLocObjectPtr ml = boost::dynamic_pointer_cast<MemLocObject>(ao);
  if(ml) return isLiveML(ml, pedge, client);*/
  assert(0);
}

/* // Returns whether dom is a dominator of part
std::set<PartPtr> Composer::isDominator(PartPtr part, PartPtr dom, ComposedAnalysis* client)
{

}

// Initializes the dominator-related data structures
void Composer::initializeDominators(ComposedAnalysis* client) 
{
  if(domInit) return;
  
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
  }
}*/

// --------------------------
// ----- Chain Composer -----
// --------------------------
  
// stxAnalysis - Points to the analysis that ChainComposer should run before any other to take application 
//    information provided in its syntax and represent it using Fuse abstractions. If stxAnalysis==NULL,
//    SyntacticAnalysis is used.
ChainComposer::ChainComposer(const list<ComposedAnalysis*>& analyses, 
                             ComposedAnalysis* testAnalysis, bool verboseTest, 
                             ComposedAnalysis* stxAnalysis) : 
    allAnalyses(analyses), testAnalysis(testAnalysis), verboseTest(verboseTest)
{
  //cout << "#allAnalyses="<<allAnalyses.size()<<endl;
  // If we're provided a syntactic analysis to use, employ it
  if(stxAnalysis) {
    doneAnalyses.push_front(stxAnalysis);
  // If we're not provided with a syntactic analysis to use, use the default one
  } else {
    // Create an instance of the syntactic analysis and insert it at the front of the done list.
    // This analysis will be called last if matching functions do not exist in any other
    // analysis and does not need a round of fixed-point iteration to produce its results.
    SyntacticAnalysis::instance()->setComposer(this);
    doneAnalyses.push_front((ComposedAnalysis*) SyntacticAnalysis::instance());
  }
  
  currentAnalysis = NULL;
  
  // Inform each analysis of the composer's identity
  //cout << "#allAnalyses="<<allAnalyses.size()<<" #doneAnalyses="<<doneAnalyses.size()<<endl;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    //cout << "ChainComposer::ChainComposer: "<<(*a)<<" : "<<(*a)->str("") << endl; cout.flush();
    (*a)->setComposer(this);
  }
  if(testAnalysis) testAnalysis->setComposer(this);
}

ChainComposer::ChainComposer(const ChainComposer& that) : 
  allAnalyses(allAnalyses), doneAnalyses(doneAnalyses), 
  currentAnalysis(currentAnalysis), testAnalysis(testAnalysis), verboseTest(verboseTest)
{} 


// Calls the operation with the ComposedAnalysis* argument
template<class RetType>
class CallWithCA : public ChainComposer::CallWithEitherComposedAnalysisOrPartEdge<RetType> {
  function<RetType (ComposedAnalysis*)> callOp;
  public:
  CallWithCA(function<RetType (ComposedAnalysis*)> callOp): callOp(callOp) {}
  RetType operator()(PartEdgePtr pedge, ComposedAnalysis* analysis, Composer::reqType type) const
  { return callOp(analysis); }
};

// Calls the operation with the PartEdgePtr argument
template<class RetType>
class CallWithPE : public ChainComposer::CallWithEitherComposedAnalysisOrPartEdge<RetType> {
  function<RetType (PartEdgePtr)> callOp;
  public:
  CallWithPE(function<RetType (PartEdgePtr)> callOp): callOp(callOp) {}
  RetType operator()(PartEdgePtr pedge, ComposedAnalysis* analysis, Composer::reqType type) const
  { return callOp(pedge); }
};

// Calls the operation with both the ComposedAnalysis* and the PartEdgePtr arguments, in that order
class CacheKey
{
  public:
  SgNode* n;
  PartEdgePtr pedge;
  ComposedAnalysis* server;
  Composer::reqType type;
  CacheKey(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* server, Composer::reqType type):
    n(n), pedge(pedge), server(server), type(type)
  {}
  
  bool operator==(const CacheKey& that) const
  { return n==that.n && pedge==that.pedge && server==that.server && type==that.type; }
  
  bool operator<(const CacheKey& that) const { 
    return n<that.n ||
           (n==that.n && pedge<that.pedge) ||
           (n==that.n && pedge==that.pedge && type<that.type) ||
           (n==that.n && pedge==that.pedge && type==that.type && server<that.server);
  }
};

template<class RetType>
class CallWithCAandPE : public ChainComposer::CallWithEitherComposedAnalysisOrPartEdge<RetType> {
  static map<CacheKey, RetType> Expr2AnyCache;
  
  function<RetType (ComposedAnalysis*, PartEdgePtr)> callOp;
  SgNode* n;
  
  public:
  CallWithCAandPE(function<RetType (ComposedAnalysis*, PartEdgePtr)> callOp, SgNode* n): callOp(callOp), n(n) {}
  RetType operator()(PartEdgePtr pedge, ComposedAnalysis* server, Composer::reqType type) const
  { 
    /*CacheKey key(n, pedge, server, type);
    if(Expr2AnyCache.find(key) == Expr2AnyCache.end())
      Expr2AnyCache[key] = callOp(server, pedge);
    return Expr2AnyCache[key];*/
    return callOp(server, pedge);
  }
};

template<class RetType>
map<CacheKey, RetType>   CallWithCAandPE<RetType>::Expr2AnyCache;

//template<class RetType>
//std::map<std::pair<SgNode*, Composer::reqType>, RetType> CallWithCAandPE::Expr2AnyCache;

// Generic function that looks up the composition chain from the given client 
// analysis and returns the result produced by the first instance of the function 
// called by the caller object found along the way.
template<class RetType>
RetType ChainComposer::callServerAnalysisFunc(
         // The name of the operation being called
         string opName,
         // Calls some operation on an analysis
         //function<RetType (ComposedAnalysis*)> callOp, 
         const CallWithEitherComposedAnalysisOrPartEdge<RetType>& callOp,
         // Returns whether a given analysis supports the operation or not
         //function<bool (ComposedAnalysis*)> isSupported,
         // The type of the request (any, memloc, codeloc, val, atsGraph)
         Composer::reqType type,
         // Returns whether a given analysis implements the operation tightly (it uses itself as 
         // a server for the operation) or loosely (it implements the operation but uses other 
         // analyses when it needs to call it)
         function<ComposedAnalysis::implTightness (ComposedAnalysis*)> checkTightness,
         // Returns a string representation of the result of the operation
         function<string (RetType, string)> ret2Str,
         // The PartEdge at which the operation is being called
         PartEdgePtr pedge, 
         // The client analysis calling the operation
         ComposedAnalysis* client, 
         // Flag that indicates whether the operation's invocation should be logged in detail
         bool verbose) {
  ostringstream label; if(verbose) label<<"ChainComposer::callServerAnalysisFunc() "<<opName<<" #doneAnalyses="<<doneAnalyses.size()<<" client="<<(client? client->str(): "NULL");
  scope reg(label.str(), scope::medium, attrGE("composerDebugLevel", (verbose? 1: composerDebugLevel()+1)));
//          attrGE("composerDebugLevel", 1));
  assert(doneAnalyses.size()>0);
  //assert(serverCache.find(client) != serverCache.end());
  list<ComposedAnalysis*> doneAnalyses_back;
  
  /*attr opAttr("ReqType", opName);
  measure* opMeasure = startMeasure("OpTimes", "Elapsed");*/
  
  // If the ChainComposed has already found the server analysis that implements requests of this type
/*  map<reqType, ComposedAnalysis*>::iterator server = serverCache[client].find(type);
  if(server != serverCache.end()) {
    // Pop from doneAnalyses the client, the server and all the analyses that separate them
    doneAnalyses_back.splice(doneAnalyses_back.end(), doneAnalyses, doneAnalyses.rbegin(), doneAnalyses.rbegin() + serverCache[client][type]+2);
    
    // Now invoke the given caller routine on the appropriate PartEdge
    RetType v(caller(args, pedge, *server, client));
    
    // Restore doneAnalyses by pushing back all the analyses that were removed for the sake of recursive
    // calls to callServerAnalysisFunc().
    doneAnalyses.splice(doneAnalyses.end(), doneAnalysesCache[client][type].second);
  }*/
  // Otherwise, call work through the composition chain to identify the server
  
  /*for(list<ComposedAnalysis*>::reverse_iterator a=doneAnalyses.rbegin(); a!=doneAnalyses.rend(); a++) {
      dbg << "&nbsp;&nbsp;&nbsp;&nbsp;"<<(*a)->str("")<<" : "<<(*a)<<endl;
  }*/
  
  if(verbose>=1) {
    dbg << "pedge="<<(pedge? pedge->str(): "NULL")<<endl;
    if(client) dbg << "client="<<client->str()<<", tightness="<<(checkTightness(client)==ComposedAnalysis::loose? "loose": "tight")<<endl;
    if(currentAnalysis) dbg << "currentAnalysis="<<currentAnalysis->str()<<", tightness="<<(checkTightness(currentAnalysis)==ComposedAnalysis::loose? "loose": "tight")<<endl;
  }
  
  // If the current analysis is non-NULL and implements the desired operation tightly, call its implementation
  if(client && checkTightness(client)) {
    //endMeasure(opMeasure);
    return callOp(pedge, client, type);
  }
  if(client==NULL && currentAnalysis && checkTightness(currentAnalysis)) {
    //endMeasure(opMeasure);
    return callOp(pedge, currentAnalysis, type);
  }
  assert((client==NULL          || queryInfo.find(client)         !=queryInfo.end()) &&
         (currentAnalysis==NULL || queryInfo.find(currentAnalysis)!=queryInfo.end()));
  CCQueryServers& info = queryInfo[client? client: currentAnalysis];
  if(verbose) { dbg << "queryInfo="<<info.str()<<endl; }
  
  ComposedAnalysis* server;
  int pedgeUnrollCnt;
  switch(type) {
    case Composer::codeloc:   server = info.lastCodeLocAnalysis;   pedgeUnrollCnt = info.ATSGraphsSinceLastCodeLocAnalysis;   break;
    case Composer::val:       server = info.lastValAnalysis;       pedgeUnrollCnt = info.ATSGraphsSinceLastValAnalysis;       break;
    case Composer::memloc:    server = info.lastMemLocAnalysis;    pedgeUnrollCnt = info.ATSGraphsSinceLastMemLocAnalysis;    break;
    case Composer::memregion: server = info.lastMemRegionAnalysis; pedgeUnrollCnt = info.ATSGraphsSinceLastMemRegionAnalysis; break;
    case Composer::atsGraph:  server = info.lastATSGraphAnalysis;  pedgeUnrollCnt = 0;                                        break;
    default: assert(0);
  }
  assert(server);
  
  if(verbose) { dbg << "server="<<server->str()<<", pedgeUnrollCnt="<<pedgeUnrollCnt<<endl;  }
  
  // Set pedge to the PartEdge of the ATS graph on which the server analysis ran
  for(int i=0; i<pedgeUnrollCnt; i++)
    pedge = pedge->getParent();
  
  //endMeasure(opMeasure);
  //cout << "server="<<server->str()<<endl;
  RetType v = callOp(pedge, server, type);
  if(verbose) dbg << "Returning "<<ret2Str(v, "")<<endl;
  return v;
  
/*  // Otherwise, iterate backwards through the composition chain looking for a completed analysis 
  // that implements the operation
  list<ComposedAnalysis*>::reverse_iterator a=doneAnalyses.rbegin();
  while(doneAnalyses.size() >= 0) {
    scope reg(txt()<<opName << "  : " << (*a)->str(""), scope::medium, 
              attrGE("composerDebugLevel", (verbose? 1: composerDebugLevel()+1)));
    if(composerDebugLevel() >= (verbose? 1: composerDebugLevel()+1)) {
      dbg << "pedge="<<(pedge? pedge->str(): "NULLPartEdgePtr")<<endl;
    }
    
    ComposedAnalysis* curDoneAnalysis = *a;
    // Move the current analysis from doneAnalyses onto a backup list to ensure that in recursive calls
    // to callServerAnalysisFunc() doneAnalyses excludes the current analysis. doneAnalyses will be restored
    // at the end of this function.
    doneAnalyses_back.push_front(curDoneAnalysis);
    doneAnalyses.pop_back();
    a=doneAnalyses.rbegin();
    
    // If the current completed analysis implements the given operation, invoke it
    //if(isSupported(curDoneAnalysis)) {
    bool isSupported = false;
    switch(type) {
      case Composer::codeloc:   isSupported = curDoneAnalysis->implementsExpr2CodeLoc();   break;
      case Composer::val:       isSupported = curDoneAnalysis->implementsExpr2Val();       break;
      case Composer::memloc:    isSupported = curDoneAnalysis->implementsExpr2MemLoc();    break;
      case Composer::memregion: isSupported = curDoneAnalysis->implementsExpr2MemRegion(); break;
      case Composer::atsGraph:  isSupported = curDoneAnalysis->implementsATSGraph();       break;
      default: assert(0);
    }
      // Now invoke the given caller routine on the appropriate PartEdge
    if(isSupported) {
      opMeasure->pause();
      RetType v = callOp(pedge, curDoneAnalysis);
      opMeasure->resume();
      
      // Now that we've reached the desired analysis, reconstruct doneAnalyses and return its reply
      // Restore doneAnalyses by pushing back all the analyses that were removed for the sake of recursive
      // calls to callServerAnalysisFunc().
      doneAnalyses.splice(doneAnalyses.end(), doneAnalyses_back);
      
      / *dbg << "Final State of doneAnalysis="<<endl;
      for(list<ComposedAnalysis*>::iterator a=doneAnalyses.begin(); a!=doneAnalyses.end(); a++)
        dbg << "    "<<(*a)->str("        ")<<endl;* /
      if(composerDebugLevel()>=1 && verbose) dbg << "Returning "<<ret2Str(v, "")<<endl;
      endMeasure(opMeasure);
      return v;
      // If the current analysis does not implement this method we keep looking further back in the chain
    } else {
      if(composerDebugLevel()>=1 && verbose) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;"<<opName<<" Not Implemented by "<<curDoneAnalysis->str()<<". Advancing to "<<(*a)->str("")<<endl;
    }
    
    // If the current caller object is concerned with PartEdges and the current analysis implements partition graphs, 
    // convert the current PartEdge that it implemented to the corresponding PartEdge of its precessor on which the 
    // current PartEdge is based.
    if(composerDebugLevel()>=1 && verbose) dbg << "pedge="<<(pedge? pedge->str(): "NULLPartEdge")<<" curDoneAnalysis->implementsATSGraph()="<<curDoneAnalysis->implementsATSGraph()<<endl;
    if(pedge && curDoneAnalysis->implementsATSGraph())
    { pedge = curDoneAnalysis->convertPEdge(pedge); 
      //pedge = pedge->getParent();
      if(composerDebugLevel()>=1 && verbose) dbg << "Updated: pedge="<<(pedge? pedge->str(): "NULLPartEdge")<<endl;
    }
  }
  
  // The first analysis in the chain must implement every optional method so 
  // control should never reach this point
  cerr << "ERROR: no analysis implements method "<<opName<<"(SgExpression)";
  assert(0);*/
}
 
// -------------------------------------
// ----- Expression Interpretation -----
// -------------------------------------

// Returns the AbstractObject that denotes the union of the objects in the list
template<class RetPtrType, class UnionRetType>
boost::shared_ptr<UnionRetType> unionAbstractObjects(std::list<RetPtrType> objects) {
  boost::shared_ptr<UnionRetType> ret = boost::make_shared<UnionRetType>(objects);
  if(composerDebugLevel()>=1) dbg << ret->str()<<endl;
  return ret;
}

// Returns the boolean value that denotes the disjunction of the booleans in the list
bool unionBool(std::list<bool> vals) {
  for(std::list<bool>::iterator v=vals.begin(); v!=vals.end(); v++)
    if(!*v) return false;
  return true;
}

// Invokes callOp on the PartEdge(s) that correspond to the given operand of SgNode n, with PartEdge
// pedge guaranteed to terminate at n. Returns the union of all the returned values.
template<class RetPtrType, class UnionRetType, class UnionRetPtrType>
UnionRetPtrType ChainComposer::OperandExpr2Any
                     (string OpName, SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client,
                      function<RetPtrType (PartEdgePtr, ComposedAnalysis*)> callOp,
                      function<UnionRetPtrType (std::list<RetPtrType>)> unionOp,
                      // Returns a string representation of the result of the operation
                      function<string (RetPtrType, string)> ret2Str)
{
  scope reg(txt()<<"ChainComposer::Operand"<<OpName, scope::medium, attrGE("composerDebugLevel", 2));
  if(composerDebugLevel()>=2) dbg << "n="<<SgNode2Str(n)<<endl << "operand("<<operand<<")="<<SgNode2Str(operand)<<endl << "pedge="<<pedge->str()<<endl;
  
  // Get the parts of the execution prefixes that terminate at the operand before continuing directly 
  // to SgNode n in the given part
  list<PartEdgePtr> opPartEdges = pedge->getOperandPartEdge(n, operand);
  if(composerDebugLevel()>=2) {
    dbg << "opPartEdges(#"<<opPartEdges.size()<<")="<<endl;
    for(list<PartEdgePtr>::iterator opE=opPartEdges.begin(); opE!=opPartEdges.end(); opE++) {
      indent ind;
      dbg << (*opE)->str()<<endl;
    }
  }
  
  // The MemRegionObjects that represent the operand within different Parts in opParts
  list<RetPtrType > partObjects;
  
  // Iterate over all the part edges to get the expression and memory MemRegionObjects for operand within those parts
  for(list<PartEdgePtr>::iterator opE=opPartEdges.begin(); opE!=opPartEdges.end(); opE++) {
    RetPtrType p = callOp(*opE, client);
    if(composerDebugLevel()>=2) {
      dbg << "opE="<<opE->get()->str()<<endl;
      dbg << "p="<<ret2Str(p, "")<<endl;
    }
    partObjects.push_back(p);
  }

  scope reg2("Returning", scope::low, attrGE("composerDebugLevel", 1));
  if(opPartEdges.size()>0) {
    // Return the union of all the objects
    return unionOp(partObjects);
  } else {
    UnionRetPtrType NULLObj;
    return NULLObj;
  }
}

// ----------------------
// --- Calling Expr2* ---
// ----------------------

std::string bool2Str(bool b, string indent) { return (b ? "TRUE" : "FALSE"); }

bool returnTrue(ComposedAnalysis*) { return true; }
ComposedAnalysis::implTightness returnLoose(ComposedAnalysis*) { return ComposedAnalysis::loose; }
ComposedAnalysis::implTightness returnTight(ComposedAnalysis*) { return ComposedAnalysis::tight; }

// Implements a Ret (boost::shared_ptr<Arg>, string) operator by calling .get() on the Arg
// argument and then calling the given function with the raw pointer as the first argument.
template<class Arg, class ArgPtr>
class CallGet2Arg
{
  function<string (Arg*, string)> op;
  public:
  CallGet2Arg(function<string (Arg*, string)> op) : op(op) {}

  string operator()(ArgPtr arg1, string arg2)
  { return op(arg1.get(), arg2); }
};


// --- CallingExpr2CodeLoc ---

CodeLocObjectPtr ChainComposer::Expr2CodeLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  return callServerAnalysisFunc<CodeLocObjectPtr>(
           "Expr2CodeLoc",
           CallWithCAandPE<CodeLocObjectPtr>(
               function<CodeLocObjectPtr (ComposedAnalysis*, PartEdgePtr)>(bind( &ComposedAnalysis::Expr2CodeLoc, _1, n, _2)), n),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           Composer::codeloc,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (CodeLocObjectPtr, string)>(
                 CallGet2Arg<CodeLocObject, CodeLocObjectPtr>(function<string (CodeLocObject*, string)>(bind( &CodeLocObject::str, _1, _2)))),
           pedge, client, false);
}
CodeLocObjectPtr ChainComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
  // Call Expr2CodeLoc_ex() and wrap the results with a UnionCodeLocObject
  return boost::make_shared<UnionCodeLocObject>(Expr2CodeLoc_ex(n, pedge, client));
}
// Variant of Expr2CodeLoc that inquires about the value of the code location denoted by the operand of the 
// given node n, where the part denotes the set of prefixes that terminate at SgNode n.
CodeLocObjectPtr ChainComposer::OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
  return OperandExpr2Any<CodeLocObjectPtr, UnionCodeLocObject, UnionCodeLocObjectPtr>
                ("Expr2CodeLoc", n, operand, pedge, client,
                 function<CodeLocObjectPtr (PartEdgePtr, ComposedAnalysis*)>(bind(&ChainComposer::Expr2CodeLoc_ex, this, operand, _1, _2)),
                 function<UnionCodeLocObjectPtr (std::list<CodeLocObjectPtr>)>(&unionAbstractObjects<CodeLocObjectPtr, UnionCodeLocObject>),
                 function<string (CodeLocObjectPtr, string)>(
                       CallGet2Arg<CodeLocObject, CodeLocObjectPtr>(function<string (CodeLocObject*, string)>(bind( &CodeLocObject::str, _1, _2)))));
}

// --- Calling Expr2Val ---
ValueObjectPtr ChainComposer::Expr2Val_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  return callServerAnalysisFunc<ValueObjectPtr>(
           "Expr2Val",
           CallWithCAandPE<ValueObjectPtr>(
               function<ValueObjectPtr (ComposedAnalysis*, PartEdgePtr)>(bind( &ComposedAnalysis::Expr2Val, _1, n, _2)), n),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (ValueObjectPtr, string)>(
                 CallGet2Arg<ValueObject, ValueObjectPtr>(function<string (ValueObject*, string)>(bind( &ValueObject::str, _1, _2)))),
           pedge, client, false);
}
ValueObjectPtr ChainComposer::Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  // Call Expr2Val_ex() and wrap the results with a UnionValueObject
  return boost::make_shared<UnionValueObject>(Expr2Val_ex(n, pedge, client));
}
// Variant of Expr2Val that inquires about the value of the value denoted by the operand of the 
// given node n, where the part denotes the set of prefixes that terminate at SgNode n.
ValueObjectPtr ChainComposer::OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
  return OperandExpr2Any<ValueObjectPtr, UnionValueObject, UnionValueObjectPtr>
                ("Expr2Val", n, operand, pedge, client,
                 function<ValueObjectPtr (PartEdgePtr, ComposedAnalysis*)>(bind(&ChainComposer::Expr2Val_ex, this, operand, _1, _2)),
                 function<UnionValueObjectPtr (std::list<ValueObjectPtr>)>(&unionAbstractObjects<ValueObjectPtr, UnionValueObject>),
                 function<string (ValueObjectPtr, string)>(
                       CallGet2Arg<ValueObject, ValueObjectPtr>(function<string (ValueObject*, string)>(bind( &ValueObject::str, _1, _2)))));
}

// --- Calling Expr2MemRegion ---
MemRegionObjectPtr ChainComposer::Expr2MemRegion_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  return callServerAnalysisFunc<MemRegionObjectPtr>(
           "Expr2MemRegion",
           CallWithCAandPE<MemRegionObjectPtr>(
               function<MemRegionObjectPtr (ComposedAnalysis *, PartEdgePtr)>(bind( &ComposedAnalysis::Expr2MemRegion, _1, n, _2)), n),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           Composer::memregion,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (MemRegionObjectPtr, string)>(
                 CallGet2Arg<MemRegionObject, MemRegionObjectPtr>(function<string (MemRegionObject*, string)>(bind( &MemRegionObject::str, _1, _2)))),
           pedge, client, false);
}
MemRegionObjectPtr ChainComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
  // Call Expr2MemRegion_ex() and wrap the results with a UnionMemRegionObject
  return boost::make_shared<UnionMemRegionObject>(Expr2MemRegion_ex(n, pedge, client));
}
// Variant of Expr2MemRegion that inquires about the value of the memory region denoted by the operand of the 
// given node n, where the part denotes the set of prefixes that terminate at SgNode n.
MemRegionObjectPtr ChainComposer::OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
  return OperandExpr2Any<MemRegionObjectPtr, UnionMemRegionObject, UnionMemRegionObjectPtr>
                ("Expr2MemRegion", n, operand, pedge, client,
                 function<MemRegionObjectPtr (PartEdgePtr, ComposedAnalysis*)>(bind(&ChainComposer::Expr2MemRegion_ex, this, operand, _1, _2)),
                 function<UnionMemRegionObjectPtr (std::list<MemRegionObjectPtr>)>(&unionAbstractObjects<MemRegionObjectPtr, UnionMemRegionObject>),
                 function<string (MemRegionObjectPtr, string)>(
                       CallGet2Arg<MemRegionObject, MemRegionObjectPtr>(function<string (MemRegionObject*, string)>(bind( &MemRegionObject::str, _1, _2)))));
}

// --- Calling Expr2MemLoc ---
MemLocObjectPtr ChainComposer::Expr2MemLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  return callServerAnalysisFunc<MemLocObjectPtr>(
             "Expr2MemLoc",
             CallWithCAandPE<MemLocObjectPtr>(
                 function<MemLocObjectPtr (ComposedAnalysis*, PartEdgePtr)>(bind( &ComposedAnalysis::Expr2MemLoc, _1, n, _2)), n),
             //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
             Composer::memloc,
             function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
             function<string (MemLocObjectPtr, string)>(
                 CallGet2Arg<MemLocObject, MemLocObjectPtr>(function<string (MemLocObject*, string)>(bind( &MemLocObject::str, _1, _2)))),
             pedge, client, false);
}
MemLocObjectPtr ChainComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
  // Call Expr2MemLoc_ex() and wrap the results with a UnionMemLocObject
  return boost::make_shared<UnionMemLocObject>(Expr2MemLoc_ex(n, pedge, client));
}
// Variant of Expr2MemLoc that inquires about the value of the code location denoted by the operand of the 
// given node n, where the part denotes the set of prefixes that terminate at SgNode n.
MemLocObjectPtr ChainComposer::OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
  return OperandExpr2Any<MemLocObjectPtr, UnionMemLocObject, UnionMemLocObjectPtr>
                ("Expr2MemLoc", n, operand, pedge, client,
                 function<MemLocObjectPtr (PartEdgePtr, ComposedAnalysis*)>(bind(&ChainComposer::Expr2MemLoc_ex, this, operand, _1, _2)),
                 function<UnionMemLocObjectPtr (std::list<MemLocObjectPtr>)>(&unionAbstractObjects<MemLocObjectPtr, UnionMemLocObject>),
                 function<string (MemLocObjectPtr, string)>(
                       CallGet2Arg<MemLocObject, MemLocObjectPtr>(function<string (MemLocObject*, string)>(bind( &MemLocObject::str, _1, _2)))));
}

// -------------------------
// --- Calling mayEqual* ---
// Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
// -------------------------

bool ChainComposer::mayEqualV (ValueObjectPtr val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("mayEqualV",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &ValueObject::mayEqualV, val1, val2, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val, 
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::mayEqualCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("mayEqualCL",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &AbstractObject::mayEqualCL, cl1, cl2, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
bool ChainComposer::mayEqualMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("mayEqualMR",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &MemRegionObject::mayEqualMR, mr1, mr2, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           Composer::memregion,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::mayEqualML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("mayEqualML",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &AbstractObject::mayEqualML, ml1, ml2, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/

// --------------------------
// --- Calling mustEqual* ---
// Returns whether the given pair of AbstractObjects are must-equal at the given PartEdge
// --------------------------
bool ChainComposer::mustEqualV (ValueObjectPtr val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("mustEqualV",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &ValueObject::mustEqualV, val1, val2, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val, 
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::mustEqualCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("mustEqualCL",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &AbstractObject::mustEqualCL, cl1, cl2, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
bool ChainComposer::mustEqualMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("mustEqualMR",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &MemRegionObject::mustEqualMR, mr1, mr2, _1))),
           Composer::memregion, 
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::mustEqualML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("mustEqualML",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &AbstractObject::mustEqualML, ml1, ml2, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
// ------------------------
// --- Calling equalSet ---
// Returns whether the two abstract objects denote the same set of concrete objects
// ------------------------

bool ChainComposer::equalSetV (ValueObjectPtr val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("equalSetV",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &ValueObject::equalSetV, val1, val2, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val, 
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::equalSetCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("equalSetCL",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &AbstractObject::equalSetCL, cl1, cl2, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
bool ChainComposer::equalSetMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("equalSetMR",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &MemRegionObject::equalSetMR, mr1, mr2, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           Composer::memregion, 
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::equalSetML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("equalSetML",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &AbstractObject::equalSetML, ml1, ml2, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/

// ----------------------
// --- Calling subSet ---
// Returns whether abstract object ao1 denotes a non-strict subset (the sets may be equal) of the set denoted
// by the abstract object ao2.
// ----------------------

bool ChainComposer::subSetV (ValueObjectPtr val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("subSetV",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &ValueObject::subSetV, val1, val2, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::subSetCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("subSetCL",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &AbstractObject::subSetCL, cl1, cl2, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
bool ChainComposer::subSetMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("subSetMR",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &MemRegionObject::subSetMR, mr1, mr2, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           Composer::memregion, 
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::subSetML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("subSetML",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &AbstractObject::subSetML, ml1, ml2, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/

// -----------------------
// --- Calling isLive* ---
// Returns whether the given AbstractObject is live at the given part edge
// -----------------------

bool ChainComposer::isLiveV (ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("isLiveV",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &ValueObject::isLiveV, val, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
// Calls the isLiveV() method of the given AbstractObject that denotes an operand of the given SgNode n within
// the context of its own PartEdges and returns true if it may be live within any of them
bool ChainComposer::OperandIsLiveV(SgNode* n, SgNode* operand, ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
  return OperandExpr2Any<bool, bool, bool>
                ("isLiveV", n, operand, pedge, client,
                 function<bool (PartEdgePtr, ComposedAnalysis*)>(/*CallWithPE<bool>(
                     function<bool (PartEdgePtr)>(*/bind( &ValueObject::isLiveV, val.get(), _1))/*))*/,
                 function<bool (std::list<bool>)>(&unionBool),
                 function<string (bool, string)>(&bool2Str));
}

/*bool ChainComposer::isLiveCL(CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("isLiveCL",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &ComposedAnalysis::isLiveCL, cl, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
bool ChainComposer::isLiveMR(MemRegionObjectPtr  mr, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("isLiveMR",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &MemRegionObject::isLiveMR, mr, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           Composer::memregion,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
// Calls the isLiveMR() method of the given AbstractObject that denotes an operand of the given SgNode n within
// the context of its own PartEdges and returns true if it may be live within any of them
bool ChainComposer::OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client) {
  return OperandExpr2Any<bool, bool, bool>
                ("isLiveMR", n, operand, pedge, client,
                 function<bool (PartEdgePtr, ComposedAnalysis*)>(/*CallWithPE<bool>(
                     function<bool (PartEdgePtr)>(*/bind( &MemRegionObject::isLiveMR, mr.get(), _1))/*))*/,
                 function<bool (std::list<bool>)>(&unionBool),
                 function<string (bool, string)>(&bool2Str));
}
/*bool ChainComposer::isLiveML(MemLocObjectPtr  ml, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("isLiveML",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &ComposedAnalysis::isLiveML, ml, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/

// --------------------------
// --- Calling meetUpdate ---
// Returns whether abstract object ao1 denotes a non-strict meetUpdate (the sets may be equal) of the set denoted
// by the abstract object ao2.
// --------------------------

bool ChainComposer::meetUpdateV (ValueObjectPtr toV, ValueObjectPtr fromV, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("meetUpdateV",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &ValueObject::meetUpdateV, toV, fromV, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::meetUpdateCL(CodeLocObjectPtr toCL, CodeLocObjectPtr fromCL, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("meetUpdateCL",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &ComposedAnalysis::meetUpdateCL, toCL, fromCL, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
bool ChainComposer::meetUpdateMR(MemRegionObjectPtr toMR, MemRegionObjectPtr fromMR, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("meetUpdateMR",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &MemRegionObject::meetUpdateMR, toMR, fromMR, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           Composer::memregion,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::meetUpdateML(MemLocObjectPtr toML, MemLocObjectPtr fromML, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("meetUpdateML",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &ComposedAnalysis::meetUpdateML, toML, fromML, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/

// ----------------------
// --- Calling isFull ---
// Returns whether the given AbstractObject corresponds to the set of all sub-executions
// ----------------------

bool ChainComposer::isFullV (ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("isFullV",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &ValueObject::isFullV, val, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::isFullCL(CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("isFullCL",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &ComposedAnalysis::isFullCL, cl, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
bool ChainComposer::isFullMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("isFullMR",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &MemRegionObject::isFullMR, mr, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           Composer::memregion,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::isFullML(MemLocObjectPtr ml, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("isFullML",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &ComposedAnalysis::isFullML, ml, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/

// -----------------------
// --- Calling isEmpty ---
// Returns whether the given AbstractObject corresponds to the empty set
// -----------------------

bool ChainComposer::isEmptyV (ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("isEmptyV",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &ValueObject::isEmptyV, val, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2Val, _1)),
           Composer::val,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2ValTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::isEmptyCL(CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client) {
  return callServerAnalysisFunc<bool>("isEmptyCL",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &ComposedAnalysis::isEmptyCL, cl, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/
bool ChainComposer::isEmptyMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("isEmptyMR",
           CallWithPE<bool>(
               function<bool (PartEdgePtr)>(bind( &MemRegionObject::isEmptyMR, mr, _1))),
           //function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
           Composer::memregion,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}
/*bool ChainComposer::isEmptyML(MemLocObjectPtr ml, PartEdgePtr pedge, ComposedAnalysis* client)  {
  return callServerAnalysisFunc<bool>("isEmptyML",
           CallWithPE(function<MemLocObjectPtr (ComposedAnalysis*)>(bind( &ComposedAnalysis::isEmptyML, ml, _1)),
           function<bool (ComposedAnalysis*)>(bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
           function<string (bool, string)>(&bool2Str),
           pedge, client, false);
}*/

// ---------------------------
// --- Calling Get*AStates ---
// Return the anchor Parts of an application
// ---------------------------

std::string PartSet2Str(const set<PartPtr>& parts, string indent) { 
  ostringstream oss;
  for(set<PartPtr>::const_iterator p=parts.begin(); p!=parts.end(); p++) {
    if(p!=parts.begin()) oss << ","<<endl<<indent;
    oss << p->get()->str(indent+"    ");
  }
  return oss.str();
}

set<PartPtr> ChainComposer::GetStartAStates(ComposedAnalysis* client) { 
  return callServerAnalysisFunc<set<PartPtr> >("GetStartAStates",
           CallWithCA<set<PartPtr> >(
               function<set<PartPtr> (ComposedAnalysis*)>(&ComposedAnalysis::GetStartAStates)),
           //function<bool (ComposedAnalysis*)>(&ComposedAnalysis::implementsATSGraph, _1),
           Composer::atsGraph,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(&returnLoose),
           function<string (const set<PartPtr>&, string)>(&PartSet2Str),
           NULLPartEdge, client, false);
}

set<PartPtr> ChainComposer::GetEndAStates(ComposedAnalysis* client) { 
  return callServerAnalysisFunc<set<PartPtr> >("GetEndAStates",
           CallWithCA<set<PartPtr> >(
               function<set<PartPtr> (ComposedAnalysis*)>(&ComposedAnalysis::GetEndAStates, _1)),
           //function<bool (ComposedAnalysis*)>(&ComposedAnalysis::implementsATSGraph, _1),
           Composer::atsGraph,
           function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(&returnLoose),
           function<string (const set<PartPtr>&, string)>(&PartSet2Str),
           NULLPartEdge, client, false);
}

// -----------------------------------------
// ----- Methods from ComposedAnalysis -----
// -----------------------------------------

// Runs the analysis, combining the intra-analysis with the inter-analysis of its choice
// ChainComposer invokes the runAnalysis methods of all its constituent analyses in sequence
void ChainComposer::runAnalysis()
{
  /*int j=1;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++, j++) {
    cout << "ChainComposer Analysis "<<j<<": "<<(*a)<<" : "<<(*a)->str("") << endl;
    cout.flush();
  }*/
  
  int i=1;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++, i++) {
    //list<string> contextAttrs;
    //contextAttrs.push_back("ReqType");
    //trace opTimesT("OpTimes", contextAttrs, trace::showEnd, trace::boxplot);
    
    currentAnalysis = *a;
    ostringstream label; if(composerDebugLevel()>=1) label << "ChainComposer Running Analysis "<<i<<": "<<(*a)<<" : "<<(*a)->str("");
    scope reg(label.str(), scope::high, attrGE("composerDebugLevel", 1));
    //scope s(txt()<<"Analysis "<<i<<": "<<currentAnalysis<<" : "<<currentAnalysis->str(""), scope::medium);
    //cout << "ChainComposer Analysis "<<i<<": "<<currentAnalysis<<" : "<<currentAnalysis->str("") << endl;
    
    // Create a CCQueryServers object to route queries from the upcoming analysis to the corresponding servers 
    // If this is the first analysis to follow the syntactic analysis
    if(doneAnalyses.size()==1) queryInfo[currentAnalysis] = CCQueryServers(doneAnalyses.back());
    else                       queryInfo[currentAnalysis] = CCQueryServers(queryInfo[doneAnalyses.back()], doneAnalyses.back());
    
    if(doneAnalyses.size()>0 && composerDebugLevel()>=1) {
      set<PartPtr> startStates = GetStartAStates(currentAnalysis);
      set<PartPtr> endStates   = GetEndAStates(currentAnalysis);
      ostringstream fName; fName << "ats." << i << "." << doneAnalyses.back()->str();
      ats2dot(fName.str(), "ATS", startStates, endStates);
      ats2dot_bw(fName.str()+".BW", "ATS", startStates, endStates);
    }
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    currentAnalysis->runAnalysis();
    
    gettimeofday(&end, NULL);
    cout << "  "<<(*a)->str("")<<" Elapsed="<<((end.tv_sec*1000000+end.tv_usec) - 
                                               (start.tv_sec*1000000+start.tv_usec))/1000000.0<<"s"<<endl;
    
    /*if(composerDebugLevel() >= 3) {
      scope reg("Final State", scope::medium, attrGE("composerDebugLevel", 3));
      std::vector<int> factNames;
      std::vector<int> latticeNames;
      latticeNames.push_back(0);
      printAnalysisStates paa(*a, factNames, latticeNames, printAnalysisStates::below);
      UnstructuredPassInterAnalysis up_paa(paa);
      up_paa.runAnalysis();
    }*/
    
    // Record that we've completed the given analysis
    doneAnalyses.push_back(*a);
    currentAnalysis = NULL;
  }
  
  if(testAnalysis) {
    // Create a CCQueryServers object to route queries from the upcoming analysis to the corresponding servers 
    // If this is the first analysis to follow the syntactic analysis
    if(doneAnalyses.size()==1) queryInfo[testAnalysis] = CCQueryServers(doneAnalyses.back());
    else                       queryInfo[testAnalysis] = CCQueryServers(queryInfo[doneAnalyses.back()], doneAnalyses.back());

    if(doneAnalyses.size()>1 && composerDebugLevel()>=1) {
      set<PartPtr> startStates = GetStartAStates(doneAnalyses.back());
      dbg << "ChainComposer::runAnalysis() #startStates="<<startStates.size()<<endl;
      set<PartPtr> endStates   = GetEndAStates(*(allAnalyses.begin()));
      ostringstream fName; fName << "ats." << i << "." << doneAnalyses.back()->str();
      ats2dot(fName.str(), "ATS", startStates, endStates);
      ats2dot_bw(fName.str()+".BW", "ATS", startStates, endStates);
    }
    
    /*list<string> contextAttrs;
    contextAttrs.push_back("ReqType");
    trace opTimesT("OpTimes", contextAttrs, trace::showEnd, trace::boxplot);*/
    
    scope s("---", scope::high, attrGE("composerDebugLevel", 1));
    //UnstructuredPassInterDataflow inter_up(testAnalysis);
    /*ContextInsensitiveInterProceduralDataflow inter_up(testAnalysis, getCallGraph());
    inter_up.runAnalysis();*/
    testAnalysis->runAnalysis();
  }
  
  return;
}

// The Expr2* and GetFunction*Part functions are implemented by calling the same functions the 
// most analysis in the composition chain that implements them.

// Abstract interpretation functions that return this analysis' abstractions that
// represent the outcome of the given SgExpression. The default implementations of
// these throw NotImplementedException so that if a derived class does not implement
// any of these functions, the Composer is informed.
//
// The objects returned by these functions are expected to be deallocated by their callers.
ValueObjectPtr   ChainComposer::Expr2Val(SgNode* n, PartEdgePtr pedge)
{ return Expr2Val(n, pedge, this); }

CodeLocObjectPtr ChainComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge)
{ return Expr2CodeLoc(n, pedge, this); }

MemRegionObjectPtr  ChainComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge)
{ return Expr2MemRegion(n, pedge, this); }

MemLocObjectPtr  ChainComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge)
{ return Expr2MemLoc(n, pedge, this); }


// ---------------------------
// ---- implementsExpr2* -----
// Return true if the class implements Expr2* and false otherwise
// ---------------------------

// Return true if the class implements Expr2* and false otherwise
bool ChainComposer::implementsExpr2Val()
{
  // If any analyses composed in series returns true, this function returns true
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++)
    if((*a)->implementsExpr2Val()) { return true; }
  return false;
}

bool ChainComposer::implementsExpr2CodeLoc()
{
  // If any analyses composed in series returns true, this function returns true
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++)
    if((*a)->implementsExpr2CodeLoc()) { return true; }
  return false;
}

bool ChainComposer::implementsExpr2MemRegion()
{
  // If any analyses composed in series returns true, this function returns true
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++)
    if((*a)->implementsExpr2MemRegion()) { return true; }
  return false;
}

bool ChainComposer::implementsExpr2MemLoc()
{
  // If any analyses composed in series returns true, this function returns true
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++)
    if((*a)->implementsExpr2MemLoc()) { return true; }
  return false;
}

// --------------------------
// ---- Expr2*Tightness -----
// Returns whether the class implements Expr* loosely or tightly (if it does at all)
// --------------------------

ComposedAnalysis::implTightness ChainComposer::Expr2ValTightness() {
  ComposedAnalysis::implTightness t = ComposedAnalysis::loose;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    // If this is the first analysis, just assign its tightness to t
    if(a==allAnalyses.begin()) t = (*a)->Expr2ValTightness();
    else
      // All analyses must have the same tightness for now
      assert(t == (*a)->Expr2ValTightness());
  }
  return t;
}

ComposedAnalysis::implTightness ChainComposer::Expr2CodeLocTightness() {
  ComposedAnalysis::implTightness t = ComposedAnalysis::loose;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    // If this is the first analysis, just assign its tightness to t
    if(a==allAnalyses.begin()) t = (*a)->Expr2CodeLocTightness();
    else
      // All analyses must have the same tightness for now
      assert(t == (*a)->Expr2CodeLocTightness());
  }
  return t;
}

ComposedAnalysis::implTightness ChainComposer::Expr2MemRegionTightness() {
  ComposedAnalysis::implTightness t = ComposedAnalysis::loose;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    // If this is the first analysis, just assign its tightness to t
    if(a==allAnalyses.begin()) t = (*a)->Expr2MemRegionTightness();
    else
      // All analyses must have the same tightness for now
      assert(t == (*a)->Expr2MemRegionTightness());
  }
  return t;
}

ComposedAnalysis::implTightness ChainComposer::Expr2MemLocTightness() {
  ComposedAnalysis::implTightness t = ComposedAnalysis::loose;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    // If this is the first analysis, just assign its tightness to t
    if(a==allAnalyses.begin()) t = (*a)->Expr2MemLocTightness();
    else
      // All analyses must have the same tightness for now
      assert(t == (*a)->Expr2MemLocTightness());
  }
  return t;
}


// Return the anchor Parts of the application
set<PartPtr> ChainComposer::GetStartAStates_Spec()
{ return GetStartAStates(this); }

set<PartPtr> ChainComposer::GetEndAStates_Spec()
{ return GetEndAStates(this); }

string ChainComposer::str(string indent) {
  ostringstream oss;
  oss << "[ChainComposer: ";
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); ) {
    oss << (*a)->str();
    a++;
    if(a!=allAnalyses.end()) oss << ", ";
  }
  oss << "]";
  return oss.str();
}

// -----------------------------------
// ----- Loose Parallel Composer -----
// -----------------------------------
  
LooseParallelComposer::LooseParallelComposer(const list<ComposedAnalysis*>& analyses, 
                                             knowledgeT subAnalysesImplementPartitions) : 
    allAnalyses(analyses), subAnalysesImplementPartitions(subAnalysesImplementPartitions)
{
  //initAnalysis();
  
  // Inform each analysis of the composer's identity
  //dbg << "LPC: #allAnalyses="<<allAnalyses.size()<<endl;
  //indent ind;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    (*a)->setComposer(this);
    //dbg << (*a)->str()<<endl;
  }
  
  subAnalysesImplementPartitions = Unknown;
}

// ---------------------------------
// ----- Methods from Composer -----
// ---------------------------------

// The Expr2* and GetFunction*Part functions are implemented by calling the same functions in the parent composer

// Abstract interpretation functions that return this analysis' abstractions that 
// represent the outcome of the given SgExpression. 
// The objects returned by these functions are expected to be deallocated by their callers.
ValueObjectPtr LooseParallelComposer::Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  if(client->implementsExpr2Val() && client->Expr2ValTightness()==ComposedAnalysis::tight)
    return client->Expr2Val(n, pedge);
  else
    return getComposer()->Expr2Val(n, pedge, this);
}

// Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the 
// given node n, where the part denotes the set of prefixes that terminate at SgNode n.
ValueObjectPtr LooseParallelComposer::OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
  if(client->implementsExpr2Val() && client->Expr2ValTightness()==ComposedAnalysis::tight)
    // Nee to port from ChainComposer;
    assert(0);
  else
   return getComposer()->OperandExpr2Val(n, operand, pedge, this);
}

CodeLocObjectPtr LooseParallelComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  if(client->implementsExpr2CodeLoc() && client->Expr2CodeLocTightness()==ComposedAnalysis::tight)
    return client->Expr2CodeLoc(n, pedge);
  else
    return getComposer()->Expr2CodeLoc(n, pedge, this);
}

// Variant of Expr2CodeLoc that inquires about the code location denoted by the operand of the given node n, where
// the part denotes the set of prefixes that terminate at SgNode n.
CodeLocObjectPtr LooseParallelComposer::OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
  if(client->implementsExpr2CodeLoc() && client->Expr2CodeLocTightness()==ComposedAnalysis::tight)
    // Nee to port from ChainComposer;
    assert(0);
  else
   return getComposer()->OperandExpr2CodeLoc(n, operand, pedge, this);
}

MemRegionObjectPtr LooseParallelComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  if(client->implementsExpr2MemRegion() && client->Expr2MemRegionTightness()==ComposedAnalysis::tight)
    return boost::make_shared<UnionMemRegionObject>(client->Expr2MemRegion(n, pedge));
  else
    return getComposer()->Expr2MemRegion(n, pedge, this);
}

// Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
// the part denotes the set of prefixes that terminate at SgNode n.
MemRegionObjectPtr LooseParallelComposer::OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
  if(client->implementsExpr2MemRegion() && client->Expr2MemRegionTightness()==ComposedAnalysis::tight)
    // Nee to port from ChainComposer;
    assert(0);
  else
   return getComposer()->OperandExpr2MemRegion(n, operand, pedge, this);
}

MemLocObjectPtr LooseParallelComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) { 
  if(client->implementsExpr2MemLoc() && client->Expr2MemLocTightness()==ComposedAnalysis::tight)
    return boost::make_shared<UnionMemLocObject>(client->Expr2MemLoc(n, pedge));
  else
    return getComposer()->Expr2MemLoc(n, pedge, this);
}

// Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
// the part denotes the set of prefixes that terminate at SgNode n.
MemLocObjectPtr LooseParallelComposer::OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
  if(client->implementsExpr2MemLoc() && client->Expr2MemLocTightness()==ComposedAnalysis::tight)
    // Nee to port from ChainComposer;
    assert(0);
  else
   return getComposer()->OperandExpr2MemLoc(n, operand, pedge, this);
}


// Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
bool LooseParallelComposer::mayEqualV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->mayEqualV(val1, val2, pedge, this); }
/*bool LooseParallelComposer::mayEqualCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->mayEqualCL(cl1, cl2, pedge, this); }*/
bool LooseParallelComposer::mayEqualMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->mayEqualMR(mr1, mr2, pedge, this); }
/*bool LooseParallelComposer::mayEqualML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->mayEqualML(ml1, ml2, pedge, this); }*/

// Returns whether the given pair of AbstractObjects are must-equal at the given PartEdge
bool LooseParallelComposer::mustEqualV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->mustEqualV(val1, val2, pedge, this); }
/*bool LooseParallelComposer::mustEqualCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->mustEqualCL(cl1, cl2, pedge, this); }*/
bool LooseParallelComposer::mustEqualMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->mustEqualMR(mr1, mr2, pedge, this); }
/*bool LooseParallelComposer::mustEqualML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->mustEqualML(ml1, ml2, pedge, this); }*/

// Returns whether the two abstract objects denote the same set of concrete objects
bool LooseParallelComposer::equalSetV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->equalSetV(val1, val2, pedge, this); }
/*bool LooseParallelComposer::equalSetCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->equalSetCL(cl1, cl2, pedge, this); }*/
bool LooseParallelComposer::equalSetMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->equalSetMR(mr1, mr2, pedge, this); }
/*bool LooseParallelComposer::equalSetML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->equalSetML(ml1, ml2, pedge, this); }*/

// Returns whether abstract object ao1 denotes a non-strict subset (the sets may be equal) of the set denoted
// by the abstract object ao2.
bool LooseParallelComposer::subSetV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->subSetV(val1, val2, pedge, this); }
/*bool LooseParallelComposer::subSetCL(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->subSetCL(cl1, cl2, pedge, this); }*/
bool LooseParallelComposer::subSetMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->subSetMR(mr1, mr2, pedge, this); }
/*bool LooseParallelComposer::subSetML(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->subSetML(ml1, ml2, pedge, this); }*/

// Returns whether the given AbstractObject is live at the given part edge
bool LooseParallelComposer::isLiveV (ValueObjectPtr val,  PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isLiveV(val, pedge, this); }
/*bool LooseParallelComposer::isLiveCL(CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isLiveCL(cl, pedge, this); }*/
bool LooseParallelComposer::isLiveMR(MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isLiveMR(mr, pedge, this); }
/*bool LooseParallelComposer::isLiveML(MemLocObjectPtr ml,  PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isLiveML(ml, pedge, this); }*/

// Calls the isLive() method of the given AbstractObject that denotes an operand of the given SgNode n within
// the context of its own PartEdges and returns true if it may be live within any of them
bool LooseParallelComposer::OperandIsLiveV (SgNode* n, SgNode* operand, ValueObjectPtr val,  PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->OperandIsLiveV(n, operand, val, pedge, this); }
/*bool LooseParallelComposer::OperandIsLiveCL(SgNode* n, SgNode* operand, CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->OperandIsLiveCL(n, operand, cl, pedge, this); }*/
bool LooseParallelComposer::OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->OperandIsLiveMR(n, operand, mr, pedge, this); }
/*bool LooseParallelComposer::OperandIsLiveML(SgNode* n, SgNode* operand, MemLocObjectPtr ml,  PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->OperandIsLiveML(n, operand, ml, pedge, this); }*/

// Computes the meet of from and to and saves the result in to.
// Returns true if this causes this to change and false otherwise.
bool LooseParallelComposer::meetUpdateV    (ValueObjectPtr   to, ValueObjectPtr   from, PartEdgePtr pedge, ComposedAnalysis* analysis)
{ return getComposer()->meetUpdateV(to, from, pedge, this); }
/*bool LooseParallelComposer::meetUpdateCL(CodeLocObjectPtr to, CodeLocObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis)
{ return getComposer()->meetUpdateCL(to, from, pedge, this); }*/
bool LooseParallelComposer::meetUpdateMR (MemRegionObjectPtr  to, MemRegionObjectPtr  from, PartEdgePtr pedge, ComposedAnalysis* analysis)
{ return getComposer()->meetUpdateMR(to, from, pedge, this); }
/*bool LooseParallelComposer::meetUpdateML (MemLocObjectPtr  to, MemLocObjectPtr  from, PartEdgePtr pedge, ComposedAnalysis* analysis)
{ return getComposer()->meetUpdateML(to, from, pedge, this); }*/
// Returns whether the given AbstractObject corresponds to the set of all sub-executions
bool LooseParallelComposer::isFullV (ValueObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isFullV(ao, pedge, this); }
/*bool LooseParallelComposer::isFullCL(CodeLocObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isFullCL(ao, pedge, this); }*/
bool LooseParallelComposer::isFullMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isFullMR(ao, pedge, this); }
/*bool LooseParallelComposer::isFullML(MemLocObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isFullML(ao, pedge, this); }*/


// Returns whether the given AbstractObject corresponds to the the empty set of sub-executions 
bool LooseParallelComposer::isEmptyV (ValueObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isEmptyV(ao, pedge, this); }
/*bool LooseParallelComposer::isEmptyCL(CodeLocObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isEmptyCL(ao, pedge, this); }*/
bool LooseParallelComposer::isEmptyMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isEmptyMR(ao, pedge, this); }
/*bool LooseParallelComposer::isEmptyML(MemLocObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
{ return getComposer()->isEmptyML(ao, pedge, this); }*/

// Return the anchor Parts of a given function
set<PartPtr> LooseParallelComposer::GetStartAStates(ComposedAnalysis* client)
{ return getComposer()->GetStartAStates(this); }
set<PartPtr> LooseParallelComposer::GetEndAStates(ComposedAnalysis* client)
{ return getComposer()->GetEndAStates(this); }

// -----------------------------------------
// ----- Methods from ComposedAnalysis -----
// -----------------------------------------

// Runs the analysis, combining the intra-analysis with the inter-analysis of its choice
// LooseParallelComposer invokes the runAnalysis methods of all its constituent analyses in sequence
void LooseParallelComposer::runAnalysis()
{
  // Run all the analyses without any interactions between them
  int i=1;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++, i++) {
    scope reg(txt()<< "LooseParallelComposer Running Analysis "<<i<<": "<<(*a)->str(""), scope::high, attrGE("composerDebugLevel", 1));
    
    (*a)->runAnalysis();
    
    dbg << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Analysis finished" << endl;
    
    /*if(composerDebugLevel() >= 3) {
      scope reg(Final State", scope::medium, attrGE("composerDebugLevel", 3));
      std::vector<int> factNames;
      std::vector<int> latticeNames;
      latticeNames.push_back(0);
      printAnalysisStates paa(*a, factNames, latticeNames, printAnalysisStates::below);
      UnstructuredPassInterAnalysis up_paa(paa);
      up_paa.runAnalysis();
    }*/
  }
  
  return;
}

// The Expr2* and GetFunction*Part functions are implemented by calling the same functions in each of the 
// constituent analyses and returning an Intersection object that includes their responses

// Abstract interpretation functions that return this analysis' abstractions that 
// represent the outcome of the given SgExpression. The default implementations of 
// these throw NotImplementedException so that if a derived class does not implement 
// any of these functions, the Composer is informed.
//
// The objects returned by these functions are expected to be deallocated by their callers.
ValueObjectPtr   LooseParallelComposer::Expr2Val(SgNode* n, PartEdgePtr pedge)
{
  // List of values that will be intersected and returned
  list<ValueObjectPtr> vals;
  
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    scope reg(txt()<<"Expr2Val  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));
    
    try {
      ValueObjectPtr val = (*a)->Expr2Val(n, getEdgeForAnalysis(pedge, *a));
      dbg << "Returning "<<val->str("")<<endl;
      vals.push_back(val);
    } catch (NotImplementedException exc) {
      if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2Val() Not Implemented."<<endl;
      // If control reaches here then the current analysis must not implement 
      // this method so we ask the remaining analyses
      continue;
    }
  }
  
  // If no sub-analysis implements this query, forward it to the composer
  if(vals.size()==0) {
    return getComposer()->Expr2Val(n, pedge, this);
  } else 
    return boost::make_shared<IntersectValueObject>(vals);
}

CodeLocObjectPtr LooseParallelComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge)
{
  // List of code location that will be intersected and returned
  list<CodeLocObjectPtr> cls;
  
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    scope reg(txt()<<"Expr2CodeLoc  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));

    try {
      CodeLocObjectPtr cl = (*a)->Expr2CodeLoc(n, getEdgeForAnalysis(pedge, *a));
      if(composerDebugLevel() >= 1) dbg << "Returning "<<cl->str("")<<endl;
      cls.push_back(cl);
    } catch (NotImplementedException exc) {
      if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2CodeLoc() Not Implemented."<<endl;
      // If control reaches here then the current analysis must not implement 
      // this method so we ask the remaining analyses
      continue;
    }
  }
  
  // If no sub-analysis implements this query, forward it to the composer
  if(cls.size()==0) {
    return getComposer()->Expr2CodeLoc(n, pedge, this);
  } else 
    return boost::make_shared<IntersectCodeLocObject>(cls);
}


MemRegionObjectPtr  LooseParallelComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge)
{
  // List of memory location that will be intersected and returned
  list<MemRegionObjectPtr> mrs;
  
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    scope reg(txt()<<"Expr2MemRegion  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));
    
    try {
      MemRegionObjectPtr mr = (*a)->Expr2MemRegion(n, getEdgeForAnalysis(pedge, *a));
      if(composerDebugLevel() >= 1) { dbg << "Returning "<<mr->str("")<<endl; }
      mrs.push_back(mr);
    } catch (NotImplementedException exc) {
      if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2MemRegion() Not Implemented."<<endl;
      // If control reaches here then the current analysis must not implement 
      // this method so we ask the remaining analyses
      continue;
    }
  }
  
  // If no sub-analysis implements this query, forward it to the composer
  return boost::make_shared<IntersectMemRegionObject>(mrs);
}


MemLocObjectPtr  LooseParallelComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge)
{
  // List of memory location that will be intersected and returned
  list<MemLocObjectPtr> mls;
  
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    scope reg(txt()<<"Expr2MemLoc  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));
    
    try {
      MemLocObjectPtr ml = (*a)->Expr2MemLoc(n, getEdgeForAnalysis(pedge, *a));
      if(composerDebugLevel() >= 1) { dbg << "Returning "<<ml->str("")<<endl; }
      mls.push_back(ml);
    } catch (NotImplementedException exc) {
      if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2MemLoc() Not Implemented."<<endl;
      // If control reaches here then the current analysis must not implement 
      // this method so we ask the remaining analyses
      continue;
    }
  }
  
  // If no sub-analysis implements this query, forward it to the composer
  return boost::make_shared<IntersectMemLocObject>(mls);
}

// ---------------------------
// ---- implementsExpr2* -----
// Return true if the class implements Expr2* and false otherwise
// ---------------------------

bool LooseParallelComposer::implementsExpr2Val()
{
  // If any analyses composed in parallel returns true, this function returns true
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++)
    if((*a)->implementsExpr2Val()) { return true; }
  return false;
}

bool LooseParallelComposer::implementsExpr2CodeLoc()
{
  // If any analyses composed in parallel returns true, this function returns true
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++)
    if((*a)->implementsExpr2CodeLoc()) { return true; }
  return false;
}

bool LooseParallelComposer::implementsExpr2MemRegion()
{
  // If any analyses composed in parallel returns true, this function returns true
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++)
    if((*a)->implementsExpr2MemRegion()) { return true; }
  return false;
}

bool LooseParallelComposer::implementsExpr2MemLoc()
{
  // If any analyses composed in parallel returns true, this function returns true
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++)
    if((*a)->implementsExpr2MemLoc()) { return true; }
  return false;
}

// --------------------------
// ---- Expr2*Tightness -----
// Returns whether the class implements Expr* loosely or tightly (if it does at all)
// --------------------------

ComposedAnalysis::implTightness LooseParallelComposer::Expr2ValTightness() {
  ComposedAnalysis::implTightness t = ComposedAnalysis::loose;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    // If this is the first analysis, just assign its tightness to t
    if(a==allAnalyses.begin()) t = (*a)->Expr2ValTightness();
    else
      // All analyses must have the same tightness for now
      assert(t == (*a)->Expr2ValTightness());
  }
  return t;
}

ComposedAnalysis::implTightness LooseParallelComposer::Expr2CodeLocTightness() {
  ComposedAnalysis::implTightness t = ComposedAnalysis::loose;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    // If this is the first analysis, just assign its tightness to t
    if(a==allAnalyses.begin()) t = (*a)->Expr2CodeLocTightness();
    else
      // All analyses must have the same tightness for now
      assert(t == (*a)->Expr2CodeLocTightness());
  }
  return t;
}

ComposedAnalysis::implTightness LooseParallelComposer::Expr2MemRegionTightness() {
  ComposedAnalysis::implTightness t = ComposedAnalysis::loose;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    // If this is the first analysis, just assign its tightness to t
    if(a==allAnalyses.begin()) t = (*a)->Expr2MemRegionTightness();
    else
      // All analyses must have the same tightness for now
      assert(t == (*a)->Expr2MemRegionTightness());
  }
  return t;
}

ComposedAnalysis::implTightness LooseParallelComposer::Expr2MemLocTightness() {
  ComposedAnalysis::implTightness t = ComposedAnalysis::loose;
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
    // If this is the first analysis, just assign its tightness to t
    if(a==allAnalyses.begin()) t = (*a)->Expr2MemLocTightness();
    else
      // All analyses must have the same tightness for now
      assert(t == (*a)->Expr2MemLocTightness());
  }
  return t;
}

/*// Return the anchor Parts of the application
set<PartPtr> LooseParallelComposer::GetStartAState_Spec()
{
  // The parts that will be intersected and returned
  map<ComposedAnalysis*, PartPtr> parts;
  
  // Construct the intersection of the sub-analyses responses to the GetStartAState query if we know
  // that at least one implements or if we don't yet know if any do
  if(subAnalysesImplementPartitions==True || subAnalysesImplementPartitions==Unknown) {
    for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
      scope reg(txt()<<"GetStartAState  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));

      try {
        PartPtr part = (*a)->GetStartAState();
        if(composerDebugLevel() >= 1) dbg << "Returning "<<part->str("")<<endl;
        parts[*a] = part;
      } catch (NotImplementedException exc) {
        if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;GetStartAState() Not Implemented."<<endl;
        // If control reaches here then the current analysis must not implement 
        // this method so we ask the remaining analyses
        continue;
      }
    }
  
    // Since analyses either always implement GetStartAState and GetEndAStates or they do not,
    // update our knowledge about partition implementations
    subAnalysesImplementPartitions = (parts.size()>0? True: False);
  }
  
  // If no sub-analysis implements this query, forward it to the composer
  if(subAnalysesImplementPartitions==False) {
    assert(parts.size()==0);
    return getComposer()->GetStartAState(this);
  } else {
    assert(parts.size()>0);
    return makePtr<IntersectionPart>(parts, getComposer()->GetStartAState(this), this);
  }
}*/


// Return the anchor Parts of the application
// ------------------------------------------

class callStartAStates : public LooseParallelComposer::callStartOrEndAStates {
  public:
  set<PartPtr> callGetStartOrEndAStates_ComposedAnalysis(ComposedAnalysis* analysis)
  { return analysis->GetStartAStates(); }
  set<PartPtr> callGetStartOrEndAStates_Composer(Composer* composer, ComposedAnalysis* analysis)
  { return composer->GetStartAStates(analysis); }
};

class callEndAStates : public LooseParallelComposer::callStartOrEndAStates {
  public:
  set<PartPtr> callGetStartOrEndAStates_ComposedAnalysis(ComposedAnalysis* analysis)
  { return analysis->GetEndAStates(); }
  set<PartPtr> callGetStartOrEndAStates_Composer(Composer* composer, ComposedAnalysis* analysis)
  { return composer->GetEndAStates(analysis); }
};

set<PartPtr> LooseParallelComposer::GetStartAStates_Spec()
{
  callStartAStates caller;
  return GetStartOrEndAStates_Spec(caller, "GetStartAStates");
}

set<PartPtr> LooseParallelComposer::GetEndAStates_Spec()
{
  callEndAStates caller;
  return GetStartOrEndAStates_Spec(caller, "GetEndAStates");
}

// Common functionality for GetStartAStates_Spec() and GetEndAStates_Spec()
set<PartPtr> LooseParallelComposer::GetStartOrEndAStates_Spec(callStartOrEndAStates& caller, string funcName)
{
  // Stores the Parts returned by the different analyses, indexing them by their parent Parts and the analyses 
  // that returned them. All the Parts associated with the same parent Part will be grouped into a single
  // IntersectionPartEdge
  map<PartPtr, map<ComposedAnalysis*, PartPtr> > intersection;
  
  // Construct the intersection of the sub-analyses responses to the GetEndAState query if we know
  // that at least one implements or if we don't yet know if any do
  if(subAnalysesImplementPartitions==True || subAnalysesImplementPartitions==Unknown) {
    for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
      scope reg(txt()<<funcName<<"  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));

      try {
        //set<PartPtr> curParts = (*a)->GetEndAStates();
        set<PartPtr> curParts = caller.callGetStartOrEndAStates_ComposedAnalysis(*a);
        
        // If this is the first analysis, simply copy its curParts into intersection
        if(a==allAnalyses.begin()) {
          for(set<PartPtr>::iterator cur=curParts.begin(); cur!=curParts.end(); cur++)
            intersection[(*cur)->getParent()][*a] = *cur;
        // If this is not the first analysis, intersect curParts with intersection, storing the result in intersection
        } else {
          map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator i=intersection.begin();
          set<PartPtr>::iterator curI=curParts.begin();
          while(i!=intersection.end() && curI!=curParts.end()) {
            // If i and curI have the same parent Part, it must be kept in the intersection of intersection and curParts
            if((*curI)->getParent() == i->first) {
              // A single analysis cannot return multiple Parts with the same parent Part
              assert(i->second.find(*a) == i->second.end());
              
              // Add the current Part from the current analysis to the intersection map
              (i->second)[*a] = *curI;
              
              // Advance both iterators
              i++;
              curI++;
            // if curI is ahead of i, all the Parts in intersection betwen i (inclusive) and curI exclusive
            // should be removed from intersection since they're not members of the intersection of intersection and curParts
            } else if(*curI > i->first) 
              intersection.erase(++i);
            // Finally, if i is ahead of i then it skipped some Parts in curParts, which will not be included
            // in the intersection if we simply advance curI
            else if(*curI < i->first)
              curI++;
          }
          
          // If curI has reached the end of curParts but i has not yet reached the end of intersection, the
          // Parts in intersection at and after i are not in the intersection and must thus be removed
          while(i!=intersection.end())
            intersection.erase(++i);
          
          // If i has reached the end of intersection but curI has not reached the end of curParts then the remaining
          // Parts in curParts will not be included in the intersection and we don't need to do anything to ensure this.
        }
      } catch (NotImplementedException exc) {
        if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;"<<funcName<<"() Not Implemented."<<endl;
        // If control reaches here then the current analysis must not implement 
        // this method so we ask the remaining analyses
        continue;
      }
    }
    
    // Since analyses either always implement GetStartAState and GetEndAStates or they do not,
    // update our knowledge about partition implementations
    subAnalysesImplementPartitions = (intersection.size()>0? True: False);
  }
  
  // If no sub-analysis implements this query, forward it to the composer
  if(subAnalysesImplementPartitions==False) {
    assert(intersection.size()==0);
    //return getComposer()->GetEndAStates(this);
    return caller.callGetStartOrEndAStates_Composer(getComposer(), this);
  } else {
    assert(intersection.size()>0);
    
    if(composerDebugLevel() >= 1) {
      dbg << "Returning ";
      for(map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator i=intersection.begin(); i!=intersection.end(); i++) {
        indent ind();
        dbg << "Parent: "<<i->first.get()->str()<<endl;
        for(map<ComposedAnalysis*, PartPtr>::iterator j=i->second.begin(); j!=i->second.end(); j++) {
          indent ind();
          dbg << j->first->str() << " =&gt; " << j->second->str() << endl;
        }
      }
    }
    
    // Convert all the Parts in parts into IntersectionParts to match the result of 
    set<PartPtr> interParts;
    for(map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator i=intersection.begin(); i!=intersection.end(); i++) {
      interParts.insert(makePtr<IntersectionPart>(i->second, i->first, this));
    }
    return interParts;
  }
}

/*set<PartPtr> LooseParallelComposer::GetEndAStates_Spec()
{
  // The intersection of the responses from the different analyses, which will be returned to the caller
  set<PartPtr> intersection;
  
  // Construct the intersection of the sub-analyses responses to the GetEndAState query if we know
  // that at least one implements or if we don't yet know if any do
  if(subAnalysesImplementPartitions==True || subAnalysesImplementPartitions==Unknown) {
    for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
      scope reg(txt()<<"GetEndAState  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));

      try {
        set<PartPtr> curParts = (*a)->GetEndAStates();
        // If this is the first analysis, simply copy its curParts into intersection
        if(a==allAnalyses.begin()) {
          intersection = curParts;
        // If this is not the first analysis, intersect curParts with intersection, storing the result in intersection
        }else {
          set<PartPtr>::iterator i=intersection.begin(),
                              curI=curParts.begin();
          while(i!=intersection.end() && curI!=curParts.begin()) {
            // If i and curI refer to the same Part, it must be kept in the intersection of intersection and curParts
            if(*curI == *i) {
              i++;
              curI++;
            // if curI is ahead of i, all the Parts in intersection betwen i (inclusive) and curI exclusive
            // should be removed from intersection since they're not members of the intersection of intersection and curParts
            } else if(*curI > *i) 
              intersection.erase(++i);
            // Finally, if i is ahead of i then it skipped some Parts in curParts, which will not be included
            // in the intersection if we simply advance curI
            else if(*curI < *i)
              curI++;
          }
          
          // If curI has reached the end of curParts but i has not yet reached the end of intersection, the
          // Parts in intersection at and after i are not in the intersection and must thus be removed
          while(i!=intersection.end())
            intersection.erase(++i);
          
          // If i has reached the end of intersection but curI has not reached the end of curParts then the remaining
          // Parts in curParts will not be included in the intersection and we don't need to do anything to ensure this.
        }
      } catch (NotImplementedException exc) {
        if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;GetEndAState() Not Implemented."<<endl;
        // If control reaches here then the current analysis must not implement 
        // this method so we ask the remaining analyses
        continue;
      }
    }
    
    // Since analyses either always implement GetStartAState and GetEndAStates or they do not,
    // update our knowledge about partition implementations
    subAnalysesImplementPartitions = (intersection.size()>0? True: False);
  }
  
  // If no sub-analysis implements this query, forward it to the composer
  if(subAnalysesImplementPartitions==False) {
    assert(parts.size()==0);
    return getComposer()->GetEndAStates(this);
  } else {
    assert(parts.size()>0);
    
    if(composerDebugLevel() >= 1) {
      dbg << "Returning ";
      for(set<PartPtr>::iterator i=parts.begin(); i!=parts.end(); i++) {
        if(i==parts.begin()) dbg << ", ";
        dbg << i->get()->str("");
      }
    }
    
    // Convert all the Parts in parts into IntersectionParts to match the result of 
    set<PartPtr> interParts;
    for(set<PartPtr>::iterator i=parts.begin(); i!=parts.end(); i++) {
      set<PartPtr> singleton; singleton.insert(*i);
      interParts.insert(makePtr<IntersectionPart>(singleton, (*i)->getParent(), this));
    }
    return interParts;
  }
}*/

// When Expr2* is queried for a particular analysis on edge pedge, exported by this LooseParallelComposer 
// this function translates from the pedge that the LooseParallelComposer::Expr2* is given to the PartEdge 
// that this particular sub-analysis runs on. If some of the analyses that were composed in parallel with 
// this analysis (may include this analysis) implement partition graphs, we know that 
// GetStartAState/GetEndAState wrapped them in IntersectionPartEdges. In this case this function
// converts pedge into an IntersectionPartEdge and queries its getPartEdge method. Otherwise, 
// GetStartAState/GetEndAState do no wrapping and thus, we can return pedge directly.
PartEdgePtr LooseParallelComposer::getEdgeForAnalysis(PartEdgePtr pedge, ComposedAnalysis* analysis)
{
  assert(subAnalysesImplementPartitions != Unknown);
  
  // If some sub-analyses of this composer do implement partition graphs, unwrap the IntersectionPartEdge
  // that combines their edges
  if(subAnalysesImplementPartitions==True) {
    IntersectionPartEdgePtr iEdge = dynamicPtrCast<IntersectionPartEdge>(pedge);
    assert(iEdge);
    //dbg << "getEdgeForAnalysis iEdge="<<iEdge->str()<<endl;
    return iEdge->getPartEdge(analysis);
  // Otherwise, pass back the raw edge that came from analyses that precede the composer
  } else {
    //dbg << "getEdgeForAnalysis pedge="<<pedge->str()<<endl;
    return pedge;
  }
}

string LooseParallelComposer::str(string indent) {
  ostringstream oss;
  oss << "[LooseParallelComposer: ";
  for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); ) {
    oss << (*a)->str();
    a++;
    if(a!=allAnalyses.end()) oss << ", ";
  }
  oss << "]";
  return oss.str();
}

}; //namespace fuse;
