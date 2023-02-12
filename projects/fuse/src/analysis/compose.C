#include "sage3basic.h"

using namespace std;

#include "compose.h"
#include "ssa.h"
#include "const_prop_analysis.h"
#include "dead_path_elim_analysis.h"
#include <boost/enable_shared_from_this.hpp>
//#include "printAnalysisStates.h"
#include "saveDotAnalysis.h"
#include "stx_analysis.h"
#include "sight.h"
#include <set>
#include <queue>

using namespace boost;
namespace fuse {

#define composerDebugLevel 0
#if composerDebugDevel==0
#endif

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
      return mayEqualAO(cl1, cl2, pedge, client);
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
      return mayEqualAO(ml1, ml2, pedge, client);
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
      return mustEqualAO(cl1, cl2, pedge, client);
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
      return mustEqualAO(ml1, ml2, pedge, client);
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
      return equalSetAO(cl1, cl2, pedge, client);
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
      return equalSetAO(ml1, ml2, pedge, client);
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
      return subSetAO(cl1, cl2, pedge, client);
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
      return subSetAO(ml1, ml2, pedge, client);
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
      if(cl) return isLiveAO(cl, pedge, client);*/

    MemRegionObjectPtr mr = boost::dynamic_pointer_cast<MemRegionObject>(ao);
    if(mr) return isLiveMR(mr, pedge, client);

    /*MemLocObjectPtr ml = boost::dynamic_pointer_cast<MemLocObject>(ao);
      if(ml) return isLiveAO(ml, pedge, client);*/
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

  /*************************
   ***** CCQueryServers ****
   *************************/

  // Initialize this object with the info of the initial analysis at the start of the composition chain
  CCQueryServers::CCQueryServers(ComposedAnalysis* startAnalysis) {
    // Records the last analysis in the composition chain that can answer queries of a given type
    lastCodeLocAnalysis   = startAnalysis;
    lastValAnalysis       = startAnalysis;
    lastMemLocAnalysis    = startAnalysis;
    lastMemRegionAnalysis = startAnalysis;
    lastATSGraphAnalysis  = startAnalysis;
    lastSSAGraphAnalysis  = startAnalysis;

    // The number of ATSGraph analyses that separate the current analysis from the last analysis that
    // can answer a given query type. This is also the number of times we'll call PartEdge->getSupersetPartEdge()
    // to convert the edges of the current analysis to those of the server of the given query type.
    // There is no counter for ATS Graph queries since it would always be 0.
    ATSGraphsSinceLastCodeLocAnalysis   = 0;
    ATSGraphsSinceLastValAnalysis       = 0;
    ATSGraphsSinceLastMemLocAnalysis    = 0;
    ATSGraphsSinceLastMemRegionAnalysis = 0;
  }

  // Given the information from the prior analysis in the composition chain, create a record that
  // accounts for queries being serviced by nextAnalysis
  CCQueryServers::CCQueryServers(const CCQueryServers& info, ComposedAnalysis* nextAnalysis) {
    /*scope s("CCQueryServers()");

      dbg << "nextAnalysis="<<nextAnalysis->str()<<std::endl;
      dbg << "before info="<<const_cast<CCQueryServers&>(info).str()<<std::endl;*/

    if(nextAnalysis->implementsExpr2CodeLoc())   lastCodeLocAnalysis   = nextAnalysis;
    else                                         lastCodeLocAnalysis   = info.lastCodeLocAnalysis;
    if(nextAnalysis->implementsExpr2Val())       lastValAnalysis       = nextAnalysis;
    else                                         lastValAnalysis       = info.lastValAnalysis;
    if(nextAnalysis->implementsExpr2MemLoc())    lastMemLocAnalysis    = nextAnalysis;
    else                                         lastMemLocAnalysis    = info.lastMemLocAnalysis;
    if(nextAnalysis->implementsExpr2MemRegion()) lastMemRegionAnalysis = nextAnalysis;
    else                                         lastMemRegionAnalysis = info.lastMemRegionAnalysis;
    if(nextAnalysis->implementsATSGraph())       lastATSGraphAnalysis    = nextAnalysis;
    else                                         lastATSGraphAnalysis  = info.lastATSGraphAnalysis;

    if(nextAnalysis->implementsATSGraph() || nextAnalysis->implementsExpr2MemLoc())
      lastSSAGraphAnalysis = nextAnalysis;
    else
      lastSSAGraphAnalysis = info.lastSSAGraphAnalysis;

    // If nextAnalysis implements a given interface, then the counter for that interface is set to 0
    //    since subsequent analyses that make calls to this interface will be able to use their PartEdges,
    //    must must be implemented by nextAnalysis.
    // If nextAnalysis does not implement the interface, then queries for this interface will need
    //    to go to its predecessors, meaning that the PartEdges held by the clients will need to be
    //    converted from those implemented by nextAnalysis to those implemented by the preceding
    //    ATSGraph analysis. If nextAnalysis implements the ATS it creates an extra layer of ATS graph
    //    indirection, forcing us to increment the counter for that API. Otherwise, we leave the counter alone.
    if(nextAnalysis->implementsExpr2CodeLoc())  ATSGraphsSinceLastCodeLocAnalysis = 0;
    else if(nextAnalysis->implementsATSGraph()) ATSGraphsSinceLastCodeLocAnalysis = info.ATSGraphsSinceLastCodeLocAnalysis+1;
    else                                        ATSGraphsSinceLastCodeLocAnalysis = info.ATSGraphsSinceLastCodeLocAnalysis;

    if(nextAnalysis->implementsExpr2Val())      ATSGraphsSinceLastValAnalysis = 0;
    else if(nextAnalysis->implementsATSGraph()) ATSGraphsSinceLastValAnalysis = info.ATSGraphsSinceLastValAnalysis+1;
    else                                        ATSGraphsSinceLastValAnalysis = info.ATSGraphsSinceLastValAnalysis;

    if(nextAnalysis->implementsExpr2MemLoc())   ATSGraphsSinceLastMemLocAnalysis = 0;
    else if(nextAnalysis->implementsATSGraph()) ATSGraphsSinceLastMemLocAnalysis = info.ATSGraphsSinceLastMemLocAnalysis+1;
    else                                        ATSGraphsSinceLastMemLocAnalysis = info.ATSGraphsSinceLastMemLocAnalysis;

    if(nextAnalysis->implementsExpr2MemRegion()) ATSGraphsSinceLastMemRegionAnalysis = 0;
    else if(nextAnalysis->implementsATSGraph())  ATSGraphsSinceLastMemRegionAnalysis = info.ATSGraphsSinceLastMemRegionAnalysis+1;
    else                                         ATSGraphsSinceLastMemRegionAnalysis = info.ATSGraphsSinceLastMemRegionAnalysis;

    //dbg << "after this="<<str()<<std::endl;
  }

  std::string CCQueryServers::str(std::string indent) const {
    std::ostringstream oss;
    oss << "[CCQueryServers:"<<endl;
    oss << indent << "    lastCodeLocAnalysis="  <<lastCodeLocAnalysis->str(indent)  <<": ATSGraphsSinceLastCodeLocAnalysis="  <<ATSGraphsSinceLastCodeLocAnalysis  <<endl;
    oss << indent << "    lastValAnalysis="      <<lastValAnalysis->str(indent)      <<": ATSGraphsSinceLastValAnalysis="      <<ATSGraphsSinceLastValAnalysis      <<endl;
    oss << indent << "    lastMemLocAnalysis="   <<lastMemLocAnalysis->str(indent)   <<": ATSGraphsSinceLastMemLocAnalysis="   <<ATSGraphsSinceLastMemLocAnalysis   <<endl;
    oss << indent << "    lastMemRegionAnalysis="<<lastMemRegionAnalysis->str(indent)<<": ATSGraphsSinceLastMemRegionAnalysis="<<ATSGraphsSinceLastMemRegionAnalysis<<endl;
    oss << indent << "    lastATSGraphAnalysis=" <<lastATSGraphAnalysis->str(indent) << endl;
    oss << indent << "    lastSSAGraphAnalysis=" <<lastSSAGraphAnalysis->str(indent) <<"]";
    return oss.str();
  }

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
    allAnalyses(that.allAnalyses), doneAnalyses(that.doneAnalyses),
    currentAnalysis(that.currentAnalysis), testAnalysis(that.testAnalysis), verboseTest(that.verboseTest)
  {}

  ChainComposer::~ChainComposer() {
  }

  // Calls the operation with the ComposedAnalysis* argument
  template<class RetType>
  class CallWithCA : public ChainComposer::CallWithEitherComposedAnalysisOrPartEdge<RetType> {
    boost::function<RetType (ComposedAnalysis*)> callOp;
  public:
    CallWithCA(boost::function<RetType (ComposedAnalysis*)> callOp): callOp(callOp) {}
    RetType operator()(PartEdgePtr pedge, ComposedAnalysis* analysis, Composer::reqType type) const
    { return callOp(analysis); }
  };

  // Calls the operation with the PartEdgePtr argument
  template<class RetType>
  class CallWithPE : public ChainComposer::CallWithEitherComposedAnalysisOrPartEdge<RetType> {
    boost::function<RetType (PartEdgePtr)> callOp;
  public:
    CallWithPE(boost::function<RetType (PartEdgePtr)> callOp): callOp(callOp) {}
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
  public:
    static map<CacheKey, RetType> Expr2AnyCache;
    boost::function<RetType (ComposedAnalysis*, PartEdgePtr)> callOp;
    SgNode* n;

    CallWithCAandPE(boost::function<RetType (ComposedAnalysis*, PartEdgePtr)> callOp, SgNode* n): callOp(callOp), n(n) {}
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
                                                //boost::function<RetType (ComposedAnalysis*)> callOp,
                                                const CallWithEitherComposedAnalysisOrPartEdge<RetType>& callOp,
                                                // Returns whether a given analysis supports the operation or not
                                                //boost::function<bool (ComposedAnalysis*)> isSupported,
                                                // The type of the request (any, memloc, codeloc, val, atsGraph)
                                                Composer::reqType type,
                                                // Returns whether a given analysis implements the operation tightly (it uses itself as
                                                // a server for the operation) or loosely (it implements the operation but uses other
                                                // analyses when it needs to call it)
                                                boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)> checkTightness,
                                                // Returns a string representation of the result of the operation
                                                boost::function<string (RetType, string)> ret2Str,
                                                // Returns an object that denotes the intersection of multiple instances of RetType
                                                boost::function<RetType (const std::map<ComposedAnalysis*, RetType>&)> createIntersection,
                                                // The PartEdge at which the operation is being called
                                                PartEdgePtr pedge,
                                                // The client analysis calling the operation
                                                ComposedAnalysis* client,
                                                // Flag that indicates whether the operation's invocation should be logged in detail
                                                bool verbose) {
    SIGHT_DECL(scope, (sight::txt()<<"ChainComposer::callServerAnalysisFunc() "<<opName<<" #doneAnalyses="<<doneAnalyses.size()<<" client="<<(client? client->str(): "NULL"), scope::medium), verbose);
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


#ifndef DISABLE_SIGHT
    SIGHT_IF(verbose)
      dbg << "pedge="<<(pedge? pedge->str(): "NULL")<<endl;
    if(client) dbg << "client="<<client->str()<<", tightness="<<(checkTightness(client)==ComposedAnalysis::loose? "loose": "tight")<<endl;
    if(currentAnalysis) dbg << "currentAnalysis="<<currentAnalysis->str()<<", tightness="<<(checkTightness(currentAnalysis)==ComposedAnalysis::loose? "loose": "tight")<<endl;
    SIGHT_FI()
#endif

      /*  // If the current analysis is non-NULL and implements the desired operation tightly, call its implementation
          if(client && checkTightness(client)) {
          //endMeasure(opMeasure);
          return callOp(pedge, client, type);
          }
          if(client==NULL && currentAnalysis && checkTightness(currentAnalysis)) {
          //endMeasure(opMeasure);
          return callOp(pedge, currentAnalysis, type);
          }*/
      assert((client==NULL          || queryInfo.find(client)         !=queryInfo.end()) &&
             (currentAnalysis==NULL || queryInfo.find(currentAnalysis)!=queryInfo.end()));
    CCQueryServers& info = queryInfo[client? client: currentAnalysis];
    SIGHT(dbg << "queryInfo="<<info.str()<<endl, verbose)

      ComposedAnalysis* server;
    int pedgeUnrollCnt;
    switch(type) {
    case Composer::codeloc:   server = info.lastCodeLocAnalysis;   pedgeUnrollCnt = info.ATSGraphsSinceLastCodeLocAnalysis;   break;
    case Composer::val:       server = info.lastValAnalysis;       pedgeUnrollCnt = info.ATSGraphsSinceLastValAnalysis;       break;
    case Composer::memloc:    server = info.lastMemLocAnalysis;    pedgeUnrollCnt = info.ATSGraphsSinceLastMemLocAnalysis;    break;
    case Composer::memregion: server = info.lastMemRegionAnalysis; pedgeUnrollCnt = info.ATSGraphsSinceLastMemRegionAnalysis; break;
    case Composer::atsGraph:  server = info.lastATSGraphAnalysis;  pedgeUnrollCnt = 0;                                        break;
    case Composer::ssaGraph:  server = info.lastSSAGraphAnalysis;  pedgeUnrollCnt = 0;                                        break;
    default: assert(0);
    }
    assert(server);

    SIGHT(dbg << "server="<<server->str()<<", pedgeUnrollCnt="<<pedgeUnrollCnt<<endl, verbose)

      // Set pedge to the PartEdge of the ATS graph on which the server analysis ran
      PartEdgePtr unrolledPEdge = pedge;
    for(int i=0; i<pedgeUnrollCnt; i++)
      unrolledPEdge = unrolledPEdge->getInputPartEdge();

    //endMeasure(opMeasure);
    //cout << "server="<<server->str()<<endl;
    SIGHT(dbg << "pedge="<<pedge->str()<<endl, verbose);
    SIGHT(dbg << "unrolledPEdge="<<unrolledPEdge->str()<<endl, verbose);
    RetType v = callOp(unrolledPEdge, server, type);

    // If the current analysis is non-NULL and implements the desired operation tightly, call its implementation
    if(client && checkTightness(client)) {
      //endMeasure(opMeasure);
      /**/RetType v2 = callOp(pedge, client, type);
      map<ComposedAnalysis*, RetType> both;
      both[server] = v;
      both[client] = v2;
      v = createIntersection(both);/**/
      //v = callOp(pedge, client, type);
    } else if(client==NULL && currentAnalysis && checkTightness(currentAnalysis)) {
      //endMeasure(opMeasure);
      RetType v2 = callOp(unrolledPEdge, currentAnalysis, type);
      map<ComposedAnalysis*, RetType> both;
      both[server] = v;
      both[currentAnalysis] = v2;
      v = createIntersection(both);
    }

    SIGHT(dbg << "Returning "<<ret2Str(v, "")<<endl, verbose)
      return v;
  }

  CodeLocObjectPtr createCodeLocIntersection(ComposedAnalysis* client, const std::map<ComposedAnalysis*, CodeLocObjectPtr>& objects)
  { return boost::dynamic_pointer_cast<CodeLocObject>(boost::make_shared<AnalMapCodeLocObject>(Intersection, client, objects)); }

  ValueObjectPtr createValueIntersection(ComposedAnalysis* client, const std::map<ComposedAnalysis*, ValueObjectPtr>& objects)
  { return boost::dynamic_pointer_cast<ValueObject>(boost::make_shared<AnalMapValueObject>(Intersection, client, objects)); }

  MemLocObjectPtr createMemLocIntersection(ComposedAnalysis* client, const std::map<ComposedAnalysis*, MemLocObjectPtr>& objects)
  { return boost::dynamic_pointer_cast<MemLocObject>(boost::make_shared<AnalMapMemLocObject>(Intersection, client, objects)); }

  MemRegionObjectPtr createMemRegionIntersection(ComposedAnalysis* client, const std::map<ComposedAnalysis*, MemRegionObjectPtr>& objects)
  { return boost::dynamic_pointer_cast<MemRegionObject>(boost::make_shared<AnalMapMemRegionObject>(Intersection, client, objects)); }

  // Dummy function that acts as placeholder for intersections of booleans
  bool createBoolIntersection(const std::map<ComposedAnalysis*, bool>& objects) {
    for(std::map<ComposedAnalysis*, bool>::const_iterator i=objects.begin(); i!=objects.end(); ++i)
      if(!i->second) return false;
    return true;
  }

  // Dummy function that acts as placeholder for intersections of PartPtrs
  set<PartPtr> createPartIntersection(const std::map<ComposedAnalysis*, set<PartPtr> >& objects)
  { assert(0); }

  // -------------------------------------
  // ----- Expression Interpretation -----
  // -------------------------------------

  // Returns the AbstractObject that denotes the union of the objects in the list
  template<class RetPtrType, class UnionRetType>
  boost::shared_ptr<UnionRetType> unionAbstractObjects(std::list<RetPtrType> objects, ComposedAnalysis* client) {
    boost::shared_ptr<UnionRetType> ret = boost::make_shared<UnionRetType>(Union, client, objects);
    SIGHT_VERB(dbg << ret->str()<<endl, 1, composerDebugLevel)
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
   boost::function<RetPtrType (PartEdgePtr, ComposedAnalysis*)> callOp,
   boost::function<UnionRetPtrType (std::list<RetPtrType>)> unionOp,
   // Returns a string representation of the result of the operation
   boost::function<string (RetPtrType, string)> ret2Str)
  {
    SIGHT_VERB_DECL(scope, (sight::txt()<<"ChainComposer::Operand"<<OpName, scope::medium), 2, composerDebugLevel)
      SIGHT_VERB(dbg << "n="<<SgNode2Str(n)<<endl << "operand("<<operand<<")="<<SgNode2Str(operand)<<endl<< "pedge="<<pedge->str()<<endl, 2, composerDebugLevel)
      /*  scope reg(sight::txt()<<"ChainComposer::Operand"<<OpName, scope::medium, attrGE("composerDebugLevel", 2));
          if(composerDebugLevel()>=2) dbg << "n="<<SgNode2Str(n)<<endl << "operand("<<operand<<")="<<SgNode2Str(operand)<<endl << "pedge="<<pedge->str()<<endl;*/

      // Get the parts of the execution prefixes that terminate at the operand before continuing directly
      // to SgNode n in the given part
      list<PartEdgePtr> opPartEdges = pedge->getOperandPartEdge(n, operand);
    //if(composerDebugLevel()>=2) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(2, composerDebugLevel)
      dbg << "opPartEdges(#"<<opPartEdges.size()<<")="<<endl;
    for(list<PartEdgePtr>::iterator opE=opPartEdges.begin(); opE!=opPartEdges.end(); opE++) {
      indent ind;
      dbg << (*opE)->str()<<endl;
    }
    SIGHT_VERB_FI()
#endif

      // The MemRegionObjects that represent the operand within different Parts in opParts
      list<RetPtrType > partObjects;

    // Iterate over all the part edges to get the expression and memory MemRegionObjects for operand within those parts
    for(list<PartEdgePtr>::iterator opE=opPartEdges.begin(); opE!=opPartEdges.end(); opE++) {
      RetPtrType p = callOp(*opE, client);
      //if(composerDebugLevel()>=2) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(2, composerDebugLevel)
        dbg << "opE="<<opE->get()->str()<<endl;
      dbg << "p="<<ret2Str(p, "")<<endl;
      SIGHT_VERB_FI()
#endif
        partObjects.push_back(p);
    }

    //scope reg2("Returning", scope::low, attrGE("composerDebugLevel", 1));
    SIGHT_VERB_DECL(scope, ("Returning", scope::low), 2, composerDebugLevel)
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
    boost::function<string (Arg*, string)> op;
  public:
    CallGet2Arg(boost::function<string (Arg*, string)> op) : op(op) {}

    string operator()(ArgPtr arg1, string arg2)
    { return op(arg1.get(), arg2); }
  };


  // --- CallingExpr2CodeLoc ---

  CodeLocObjectPtr ChainComposer::Expr2CodeLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<CodeLocObjectPtr>(
                                                    "Expr2CodeLoc",
                                                    CallWithCAandPE<CodeLocObjectPtr>(
                                                                                      boost::function<CodeLocObjectPtr (ComposedAnalysis*, PartEdgePtr)>(boost::bind( &ComposedAnalysis::Expr2CodeLoc, _1, n, _2)), n),
                                                    //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
                                                    Composer::codeloc,
                                                    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
                                                    boost::function<string (CodeLocObjectPtr, string)>(
                                                                                                CallGet2Arg<CodeLocObject, CodeLocObjectPtr>(boost::function<string (CodeLocObject*, string)>(boost::bind( &CodeLocObject::str, _1, _2)))),
                                                    boost::function<CodeLocObjectPtr (const map<ComposedAnalysis*, CodeLocObjectPtr>&)>(boost::bind(createCodeLocIntersection, client, _1)),
                                                    pedge, client, false);
  }
  CodeLocObjectPtr ChainComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call Expr2CodeLoc_ex() and wrap the results with a UnionCodeLocObject
    return boost::make_shared<CombinedCodeLocObject>(Union, client, Expr2CodeLoc_ex(n, pedge, client));
  }
  // Variant of Expr2CodeLoc that inquires about the value of the code location denoted by the operand of the
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  CodeLocObjectPtr ChainComposer::OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    return OperandExpr2Any<CodeLocObjectPtr, CombinedCodeLocObject, CombinedCodeLocObjectPtr>
      ("Expr2CodeLoc", n, operand, pedge, client,
       boost::function<CodeLocObjectPtr (PartEdgePtr, ComposedAnalysis*)>(boost::bind(&ChainComposer::Expr2CodeLoc_ex, this, operand, _1, _2)),
       boost::function<CombinedCodeLocObjectPtr (std::list<CodeLocObjectPtr>)>(boost::bind(&unionAbstractObjects<CodeLocObjectPtr, CombinedCodeLocObject>, _1, client)),
       boost::function<string (CodeLocObjectPtr, string)>(
                                                   CallGet2Arg<CodeLocObject, CodeLocObjectPtr>(boost::function<string (CodeLocObject*, string)>(boost::bind( &CodeLocObject::str, _1, _2)))));
  }

  // --- Calling Expr2Val ---
  ValueObjectPtr ChainComposer::Expr2Val_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<ValueObjectPtr>(
                                                  "Expr2Val",
                                                  CallWithCAandPE<ValueObjectPtr>(
                                                                                  boost::function<ValueObjectPtr (ComposedAnalysis*, PartEdgePtr)>(boost::bind( &ComposedAnalysis::Expr2Val, _1, n, _2)), n),
                                                  //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                                  Composer::val,
                                                  boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                                  boost::function<string (ValueObjectPtr, string)>(
                                                                                            CallGet2Arg<ValueObject, ValueObjectPtr>(boost::function<string (ValueObject*, string)>(boost::bind( &ValueObject::str, _1, _2)))),
                                                  boost::function<ValueObjectPtr (const map<ComposedAnalysis*, ValueObjectPtr>&)>(boost::bind(createValueIntersection, client, _1)),
                                                  pedge, client, false);
  }
  ValueObjectPtr ChainComposer::Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call Expr2Val_ex() and wrap the results with a UnionValueObject
    return boost::make_shared<CombinedValueObject>(Union, client, Expr2Val_ex(n, pedge, client));
  }
  // Variant of Expr2Val that inquires about the value of the value denoted by the operand of the
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  ValueObjectPtr ChainComposer::OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    return OperandExpr2Any<ValueObjectPtr, CombinedValueObject, CombinedValueObjectPtr>
      ("Expr2Val", n, operand, pedge, client,
       boost::function<ValueObjectPtr (PartEdgePtr, ComposedAnalysis*)>(boost::bind(&ChainComposer::Expr2Val_ex, this, operand, _1, _2)),
       boost::function<CombinedValueObjectPtr (std::list<ValueObjectPtr>)>(boost::bind(&unionAbstractObjects<ValueObjectPtr, CombinedValueObject>, _1, client)),
       boost::function<string (ValueObjectPtr, string)>(
                                                 CallGet2Arg<ValueObject, ValueObjectPtr>(boost::function<string (ValueObject*, string)>(boost::bind( &ValueObject::str, _1, _2)))));
  }

  // --- Calling Expr2MemRegion ---
  MemRegionObjectPtr ChainComposer::Expr2MemRegion_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<MemRegionObjectPtr>(
                                                      "Expr2MemRegion",
                                                      CallWithCAandPE<MemRegionObjectPtr>(
                                                                                          boost::function<MemRegionObjectPtr (ComposedAnalysis *, PartEdgePtr)>(boost::bind( &ComposedAnalysis::Expr2MemRegion, _1, n, _2)), n),
                                                      //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                                      Composer::memregion,
                                                      boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                                      boost::function<string (MemRegionObjectPtr, string)>(
                                                                                                    CallGet2Arg<MemRegionObject, MemRegionObjectPtr>(boost::function<string (MemRegionObject*, string)>(boost::bind( &MemRegionObject::str, _1, _2)))),
                                                      boost::function<MemRegionObjectPtr (const map<ComposedAnalysis*, MemRegionObjectPtr>&)>(boost::bind(createMemRegionIntersection, client, _1)),
                                                      pedge, client, false);
  }
  MemRegionObjectPtr ChainComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    /*std::list<MemRegionObjectPtr > subObjs;
      return boost::make_shared<CombinedMemRegionObject>(Union, client, subObjs);*/
    // Call Expr2MemRegion_ex() and wrap the results with a UnionMemRegionObject
    return boost::make_shared<CombinedMemRegionObject>(Union, client, Expr2MemRegion_ex(n, pedge, client));
  }
  // Variant of Expr2MemRegion that inquires about the value of the memory region denoted by the operand of the
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  MemRegionObjectPtr ChainComposer::OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    return OperandExpr2Any<MemRegionObjectPtr, CombinedMemRegionObject, CombinedMemRegionObjectPtr>
      ("Expr2MemRegion", n, operand, pedge, client,
       boost::function<MemRegionObjectPtr (PartEdgePtr, ComposedAnalysis*)>(boost::bind(&ChainComposer::Expr2MemRegion_ex, this, operand, _1, _2)),
       boost::function<CombinedMemRegionObjectPtr (std::list<MemRegionObjectPtr>)>(boost::bind(&unionAbstractObjects<MemRegionObjectPtr, CombinedMemRegionObject>, _1, client)),
       boost::function<string (MemRegionObjectPtr, string)>(
                                                     CallGet2Arg<MemRegionObject, MemRegionObjectPtr>(boost::function<string (MemRegionObject*, string)>(boost::bind( &MemRegionObject::str, _1, _2)))));
  }

  // --- Calling Expr2MemLoc ---
  MemLocObjectPtr ChainComposer::Expr2MemLoc_ex(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<MemLocObjectPtr>(
                                                   "Expr2MemLoc",
                                                   CallWithCAandPE<MemLocObjectPtr>(
                                                                                    boost::function<MemLocObjectPtr (ComposedAnalysis*, PartEdgePtr)>(boost::bind( &ComposedAnalysis::Expr2MemLoc, _1, n, _2)), n),
                                                   //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
                                                   Composer::memloc,
                                                   boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
                                                   boost::function<string (MemLocObjectPtr, string)>(
                                                                                              CallGet2Arg<MemLocObject, MemLocObjectPtr>(boost::function<string (MemLocObject*, string)>(boost::bind( &MemLocObject::str, _1, _2)))),
                                                   boost::function<MemLocObjectPtr (const map<ComposedAnalysis*, MemLocObjectPtr>&)>(boost::bind(createMemLocIntersection, client, _1)),
                                                   pedge, client, false);
  }
  MemLocObjectPtr ChainComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call Expr2MemLoc_ex() and wrap the results with a UnionMemLocObject
    return boost::make_shared<CombinedMemLocObject>(Union, client, Expr2MemLoc_ex(n, pedge, client));
  }
  // Variant of Expr2MemLoc that inquires about the value of the code location denoted by the operand of the
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  MemLocObjectPtr ChainComposer::OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    return OperandExpr2Any<MemLocObjectPtr, CombinedMemLocObject, CombinedMemLocObjectPtr>
      ("Expr2MemLoc", n, operand, pedge, client,
       boost::function<MemLocObjectPtr (PartEdgePtr, ComposedAnalysis*)>(boost::bind(&ChainComposer::Expr2MemLoc_ex, this, operand, _1, _2)),
       boost::function<CombinedMemLocObjectPtr (std::list<MemLocObjectPtr>)>(boost::bind(&unionAbstractObjects<MemLocObjectPtr, CombinedMemLocObject>, _1, client)),
       boost::function<string (MemLocObjectPtr, string)>(
                                                  CallGet2Arg<MemLocObject, MemLocObjectPtr>(boost::function<string (MemLocObject*, string)>(boost::bind( &MemLocObject::str, _1, _2)))));
  }

  // -------------------------
  // --- Calling mayEqual* ---
  // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
  // -------------------------

  bool ChainComposer::mayEqualV (ValueObjectPtr val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("mayEqualV",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &ValueObject::mayEqualAO, val1, val2, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                        Composer::val,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::mayEqualAO(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("mayEqualCL",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &AbstractObject::mayEqualCL, cl1, cl2, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/
  bool ChainComposer::mayEqualMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    /*scope s("ChainComposer::mayEqualMR");
      dbg << "mr1="<<(mr1?mr1->str():"NULL")<<endl;
      dbg << "mr2="<<(mr2?mr2->str():"NULL")<<endl;
      dbg << "pedge="<<pedge->str()<<endl;
      dbg << "client="<<client->str()<<endl;*/
    return callServerAnalysisFunc<bool>("mayEqualMR",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &MemRegionObject::mayEqualAO, mr1, mr2, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                        Composer::memregion,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::mayEqualAO(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("mayEqualML",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &AbstractObject::mayEqualML, ml1, ml2, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/

  // --------------------------
  // --- Calling mustEqual* ---
  // Returns whether the given pair of AbstractObjects are must-equal at the given PartEdge
  // --------------------------
  bool ChainComposer::mustEqualV (ValueObjectPtr val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("mustEqualV",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &ValueObject::mustEqualAO, val1, val2, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                        Composer::val,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::mustEqualAO(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("mustEqualCL",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &AbstractObject::mustEqualCL, cl1, cl2, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/
  bool ChainComposer::mustEqualMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("mustEqualMR",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &MemRegionObject::mustEqualAO, mr1, mr2, _1))),
                                        Composer::memregion,
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::mustEqualAO(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("mustEqualML",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &AbstractObject::mustEqualML, ml1, ml2, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/
  // ------------------------
  // --- Calling equalSet ---
  // Returns whether the two abstract objects denote the same set of concrete objects
  // ------------------------

  bool ChainComposer::equalSetV (ValueObjectPtr val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("equalSetV",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &ValueObject::equalSetAO, val1, val2, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                        Composer::val,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::equalSetAO(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("equalSetCL",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &AbstractObject::equalSetCL, cl1, cl2, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/
  bool ChainComposer::equalSetMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("equalSetMR",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &MemRegionObject::equalSetAO, mr1, mr2, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                        Composer::memregion,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::equalSetAO(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("equalSetML",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &AbstractObject::equalSetML, ml1, ml2, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
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
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &ValueObject::subSetAO, val1, val2, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                        Composer::val,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::subSetAO(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("subSetCL",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &AbstractObject::subSetCL, cl1, cl2, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/
  bool ChainComposer::subSetMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("subSetMR",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &MemRegionObject::subSetAO, mr1, mr2, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                        Composer::memregion,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::subSetAO(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("subSetML",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &AbstractObject::subSetML, ml1, ml2, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/

  // -----------------------
  // --- Calling isLive* ---
  // Returns whether the given AbstractObject is live at the given part edge
  // -----------------------

  bool ChainComposer::isLiveV (ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("isLiveV",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &ValueObject::isLiveAO, val, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                        Composer::val,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  // Calls the isLiveAO() method of the given AbstractObject that denotes an operand of the given SgNode n within
  // the context of its own PartEdges and returns true if it may be live within any of them
  bool ChainComposer::OperandIsLiveV(SgNode* n, SgNode* operand, ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
    return OperandExpr2Any<bool, bool, bool>
      ("isLiveV", n, operand, pedge, client,
       boost::function<bool (PartEdgePtr, ComposedAnalysis*)>(/*CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(*/boost::bind( &ValueObject::isLiveAO, val.get(), _1))/*))*/,
       boost::function<bool (std::list<bool>)>(&unionBool),
       boost::function<string (bool, string)>(&bool2Str));
  }

  /*bool ChainComposer::isLiveAO(CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("isLiveCL",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::isLiveCL, cl, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/
  bool ChainComposer::isLiveMR(MemRegionObjectPtr  mr, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("isLiveMR",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &MemRegionObject::isLiveAO, mr, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                        Composer::memregion,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  // Calls the isLiveAO() method of the given AbstractObject that denotes an operand of the given SgNode n within
  // the context of its own PartEdges and returns true if it may be live within any of them
  bool ChainComposer::OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client) {
    return OperandExpr2Any<bool, bool, bool>
      ("isLiveMR", n, operand, pedge, client,
       boost::function<bool (PartEdgePtr, ComposedAnalysis*)>(/*CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(*/boost::bind( &MemRegionObject::isLiveAO, mr.get(), _1))/*))*/,
       boost::function<bool (std::list<bool>)>(&unionBool),
       boost::function<string (bool, string)>(&bool2Str));
  }
  /*bool ChainComposer::isLiveAO(MemLocObjectPtr  ml, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("isLiveML",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::isLiveML, ml, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
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
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &ValueObject::meetUpdateAO, toV, fromV, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                        Composer::val,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::meetUpdateAO(CodeLocObjectPtr toCL, CodeLocObjectPtr fromCL, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("meetUpdateCL",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::meetUpdateCL, toCL, fromCL, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/
  bool ChainComposer::meetUpdateMR(MemRegionObjectPtr toMR, MemRegionObjectPtr fromMR, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("meetUpdateMR",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &MemRegionObject::meetUpdateAO, toMR, fromMR, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                        Composer::memregion,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::meetUpdateAO(MemLocObjectPtr toML, MemLocObjectPtr fromML, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("meetUpdateML",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::meetUpdateML, toML, fromML, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    pedge, client, false);
    }*/

  // ----------------------
  // --- Calling isFull ---
  // Returns whether the given AbstractObject corresponds to the set of all sub-executions
  // ----------------------

  bool ChainComposer::isFullV (ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("isFullV",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &ValueObject::isFullAO, val, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                        Composer::val,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::isFullAO(CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("isFullCL",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::isFullCL, cl, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
    pedge, client, false);
    }*/
  bool ChainComposer::isFullMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("isFullMR",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &MemRegionObject::isFullAO, mr, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                        Composer::memregion,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::isFullAO(MemLocObjectPtr ml, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("isFullML",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::isFullML, ml, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
    pedge, client, false);
    }*/

  // -----------------------
  // --- Calling isEmpty ---
  // Returns whether the given AbstractObject corresponds to the empty set
  // -----------------------

  bool ChainComposer::isEmptyV (ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("isEmptyV",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &ValueObject::isEmptyAO, val, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2Val, _1)),
                                        Composer::val,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2ValTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::isEmptyAO(CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client) {
    return callServerAnalysisFunc<bool>("isEmptyCL",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::isEmptyCL, cl, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2CodeLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2CodeLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
    pedge, client, false);
    }*/
  bool ChainComposer::isEmptyMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("isEmptyMR",
                                        CallWithPE<bool>(
                                                         boost::function<bool (PartEdgePtr)>(boost::bind( &MemRegionObject::isEmptyAO, mr, _1))),
                                        //boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemRegion, _1)),
                                        Composer::memregion,
                                        boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                        boost::function<string (bool, string)>(&bool2Str),
                                        boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
                                        pedge, client, false);
  }
  /*bool ChainComposer::isEmptyAO(MemLocObjectPtr ml, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<bool>("isEmptyML",
    CallWithPE(boost::function<MemLocObjectPtr (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::isEmptyML, ml, _1)),
    boost::function<bool (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::implementsExpr2MemLoc, _1)),
    boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemLocTightness, _1)),
    boost::function<string (bool, string)>(&bool2Str),
    boost::function<bool (const map<ComposedAnalysis*, bool>&)>(createBoolIntersection),
    pedge, client, false);
    }*/

  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr ChainComposer::getRegionSizeMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client)  {
    return callServerAnalysisFunc<ValueObjectPtr>("getRegionSizeMR",
                                                  CallWithPE<ValueObjectPtr>(
                                                                             boost::function<ValueObjectPtr (PartEdgePtr)>(boost::bind( &MemRegionObject::getRegionSizeAO, mr, _1))),
                                                  Composer::memregion,
                                                  boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(boost::bind( &ComposedAnalysis::Expr2MemRegionTightness, _1)),
                                                  boost::function<string (ValueObjectPtr, string)>(
                                                                                            CallGet2Arg<ValueObject, ValueObjectPtr>(boost::function<string (ValueObject*, string)>(boost::bind( &ValueObject::str, _1, _2)))),
                                                  boost::function<ValueObjectPtr (const map<ComposedAnalysis*, ValueObjectPtr>&)>(boost::bind(createValueIntersection, client, _1)),
                                                  pedge, client, false);
  }


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
    //scope s("ChainComposer::GetStartAStates");
    return
      callServerAnalysisFunc<set<PartPtr> >("GetStartAStates",
                                            CallWithCA<set<PartPtr> >(
                                                                      boost::function<set<PartPtr> (ComposedAnalysis*)>(&ComposedAnalysis::GetStartAStates)),
                                            //boost::function<bool (ComposedAnalysis*)>(&ComposedAnalysis::implementsATSGraph, _1),
                                            Composer::atsGraph,
                                            boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(&returnLoose),
                                            boost::function<string (const set<PartPtr>&, string)>(&PartSet2Str),
                                            boost::function<set<PartPtr> (const map<ComposedAnalysis*, set<PartPtr> >&)>(createPartIntersection),
                                            NULLPartEdge, client, false);
  }

  set<PartPtr> ChainComposer::GetEndAStates(ComposedAnalysis* client) {
    return
      callServerAnalysisFunc<set<PartPtr> >("GetEndAStates",
                                            CallWithCA<set<PartPtr> >(
                                                                      boost::function<set<PartPtr> (ComposedAnalysis*)>(&ComposedAnalysis::GetEndAStates, _1)),
                                            //boost::function<bool (ComposedAnalysis*)>(&ComposedAnalysis::implementsATSGraph, _1),
                                            Composer::atsGraph,
                                            boost::function<ComposedAnalysis::implTightness (ComposedAnalysis*)>(&returnLoose),
                                            boost::function<string (const set<PartPtr>&, string)>(&PartSet2Str),
                                            boost::function<set<PartPtr> (const map<ComposedAnalysis*, set<PartPtr> >&)>(createPartIntersection),
                                            NULLPartEdge, client, false);
  }

  // Return an SSAGraph object that describes the overall structure of the transition system
  SSAGraph* ChainComposer::GetSSAGraph(ComposedAnalysis* client) {
    // Find the the analysis that most recently implemented the ATS
    ROSE_ASSERT(queryInfo[client].lastSSAGraphAnalysis);

    // Create an ATS for it if one has not already been created
    if(SSAGraphCache.find(queryInfo[client].lastSSAGraphAnalysis) == SSAGraphCache.end())
      SSAGraphCache[queryInfo[client].lastSSAGraphAnalysis] = new SSAGraph(this, client);

    return SSAGraphCache[queryInfo[client].lastSSAGraphAnalysis];
  }

  // Given a Part implemented by the entire composer, returns the set of refined Parts implemented
  // by the composer or the NULLPart if this relationship was not tracked.
  const set<PartPtr>& ChainComposer::getRefinedParts(PartPtr basePart) const {
    //scope s(sight::txt()<<"ChainComposer::getRefinedParts("<<basePart->str()<<")");
    // If we don't ygetRefinedPartset have the edges that refine base in the cache, compute them and add one
    map<PartPtr, std::set<PartPtr> >::const_iterator i=base2RefinedPart.find(basePart);
    if(i == base2RefinedPart.end()) {
      //dbg << "    not cached. #doneAnalyses="<<doneAnalyses.size()<<endl;
      boost::shared_ptr<std::set<PartPtr> > base = boost::make_shared<std::set<PartPtr> >();
      base->insert(basePart);

      // Iterate through all the completed analyses that implement an ATS
      for(std::list<ComposedAnalysis*>::const_iterator a=doneAnalyses.begin(); a!=doneAnalyses.end(); ++a) {
        if((*a)->implementsATSGraph() && *a!=SyntacticAnalysis::instance()) {
          // Given all the base edges in *base (part of the ATS on which the current
          // analysis ran), get the set of edges that refine them, gathering them into refined.
          boost::shared_ptr<std::set<PartPtr> > refined = boost::make_shared<std::set<PartPtr> >();
          //dbg << "        Analysis "<<(*a)->str()<<" implements the ATS, #base="<<base->size()<<endl;
          for(std::set<PartPtr>::iterator e=base->begin(); e!=base->end(); ++e) {
            const std::set<PartPtr>& curRefined = (*a)->getRefinedParts(*e);
            //dbg << "          refined edge="<<(*e)->str()<<", #curRefined="<<curRefined.size()<<endl;
            refined->insert(curRefined.begin(), curRefined.end());
          }
          // Drop the prior set of edges and replace them with their corresponding refined edges
          //dbg << "#base="<<base->size()<<", #refined="<<refined->size()<<endl;
          base = refined;
        }
      }
      // Store the ultimate refined edges in the cache (the set is copied and the original will be destroyed on function exit)
      //dbg << "    Setting base="<<basePart->str()<<" to "<<base->size()<<" edges = "<<base.get()<<endl;
      //((std::map<PartPtr, boost::shared_ptr<std::set<PartPtr> > >)base2RefinedPart)[base] = base;
      ((ChainComposer*)this)->base2RefinedPart[basePart] = *base.get();
      //cout << "ChainComposer::getRefinedParts("<<basePart->str()<<")  NEW exists="<<(base2RefinedPart.find(basePart)!=base2RefinedPart.end())<<endl;
      return ((ChainComposer*)this)->base2RefinedPart[basePart];
    } else {
      //cout << "ChainComposer::getRefinedParts("<<basePart->str()<<")  FOUND"<<endl;
      return i->second;
    }
  }

  // Given a PartEdge implemented by the entire composer, returns the set of refined PartEdges implemented
  // by the composer or the NULLPartEdge if this relationship was not tracked.
  const set<PartEdgePtr>& ChainComposer::getRefinedPartEdges(PartEdgePtr basePedge) const {
    //scope s(sight::txt()<<"ChainComposer::getRefinedPartEdges("<<base->str()<<")");
    map<PartEdgePtr, std::set<PartEdgePtr> >::const_iterator i=base2RefinedPartEdge.find(basePedge);
    if(i == base2RefinedPartEdge.end()) {
      //dbg << "    not cached. #doneAnalyses="<<doneAnalyses.size()<<endl;
      boost::shared_ptr<std::set<PartEdgePtr> > base = boost::make_shared<std::set<PartEdgePtr> >();
      base->insert(basePedge);

      // Iterate through all the completed analyses that implement an ATS
      for(std::list<ComposedAnalysis*>::const_iterator a=doneAnalyses.begin(); a!=doneAnalyses.end(); ++a) {
        if((*a)->implementsATSGraph() && *a!=SyntacticAnalysis::instance()) {
          // Given all the base edges in *base (part of the ATS on which the current
          // analysis ran), get the set of edges that refine them, gathering them into refined.
          boost::shared_ptr<std::set<PartEdgePtr> > refined = boost::make_shared<std::set<PartEdgePtr> >();
          //dbg << "        Analysis "<<(*a)->str()<<" implements the ATS, #base="<<base->size()<<endl;
          for(std::set<PartEdgePtr>::iterator e=base->begin(); e!=base->end(); ++e) {
            const std::set<PartEdgePtr>& curRefined = (*a)->getRefinedPartEdges(*e);
            //dbg << "          refined edge="<<(*e)->str()<<", #curRefined="<<curRefined.size()<<endl;
            refined->insert(curRefined.begin(), curRefined.end());
          }
          // Drop the prior set of edges and replace them with their corresponding refined edges
          //dbg << "#base="<<base->size()<<", #refined="<<refined->size()<<endl;
          base = refined;
        }
      }
      // Store the ultimate refined edges in the cache (the set is copied and the original will be destroyed on function exit)
      //dbg << "    Setting base="<<base->str()<<" to "<<base->size()<<" edges = "<<base.get()<<endl;
      //((std::map<PartPtr, boost::shared_ptr<std::set<PartPtr> > >)base2RefinedPartEdge)[base] = base;
      ((ChainComposer*)this)->base2RefinedPartEdge[basePedge] = *base.get();
      return ((ChainComposer*)this)->base2RefinedPartEdge[basePedge];
    } else {
      return i->second;
    }
  }

  // Returns the number of Parts that refine the given base
  unsigned int  ChainComposer::numRefinedParts    (PartPtr     base) const {
    return getRefinedParts(base).size();
  }

  // Returns the number of PartEdges that refine the given base
  unsigned int  ChainComposer::numRefinedPartEdges(PartEdgePtr base) const {
    return getRefinedPartEdges(base).size();
  }
  // -----------------------------------------
  // ----- Methods from ComposedAnalysis -----
  // -----------------------------------------

  // Runs the analysis, combining the intra-analysis with the inter-analysis of its choice
  // ChainComposer invokes the runAnalysis methods of all its constituent analyses in sequence
  void ChainComposer::runAnalysis()
  {
    /*  int j=1;
        cout << "allAnalyses:"<<endl;
        for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++, j++) {
        cout << "ChainComposer Analysis "<<j<<": "<<(*a)<<" : "<<(*a)->str("") << endl;
        cout.flush();
        }
        j=1;
        cout << "doneAnalyses:"<<endl;
        for(list<ComposedAnalysis*>::iterator a=doneAnalyses.begin(); a!=doneAnalyses.end(); a++, j++) {
        cout << "ChainComposer Analysis "<<j<<": "<<(*a)<<" : "<<(*a)->str("") << endl;
        cout.flush();
        }*/

    // Create query information for just the done analyses so that we can answer queries
    // made before the first analysis starts
    if(doneAnalyses.size()==1) {
      currentAnalysis = NULL;
      queryInfo[currentAnalysis] = CCQueryServers(doneAnalyses.back());
    }

    // memory cleanup of prior CP and DP
    std::queue<ComposedAnalysis*> doneCPDP;

    int i=1;
    for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++, i++) {
      //list<string> contextAttrs;
      //contextAttrs.push_back("ReqType");
      //trace opTimesT("OpTimes", contextAttrs, trace::showEnd, trace::boxplot);

      //scope s(sight::txt()<<"Analysis "<<i<<": "<<currentAnalysis<<" : "<<currentAnalysis->str(""), scope::medium);
      //cout << "ChainComposer Analysis "<<i<<": "<<currentAnalysis<<" : "<<currentAnalysis->str("") << endl;

      // Create a CCQueryServers object to route queries from the upcoming analysis to the corresponding servers
      // If this is the first analysis to follow the syntactic analysis
      //dbg << "#doneAnalyses="<<doneAnalyses.size()<<endl;
      if(doneAnalyses.size()==1) queryInfo[*a] = CCQueryServers(doneAnalyses.back());
      else                       queryInfo[*a] = CCQueryServers(queryInfo[doneAnalyses.back()], doneAnalyses.back());
      //dbg << "queryInfo[*a]="<<queryInfo[*a].str()<<endl;

      // Initialize the current analysis before officially setting it as current by assigning currentAnalysis to it
      (*a)->initAnalysis();

      currentAnalysis = *a;
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (sight::txt()<<"ChainComposer Running Analysis "<<i<<": "<<(*a)<<" : "<<(*a)->str(""), scope::high), 1, composerDebugLevel);
#endif

      //if(doneAnalyses.size()>0 && composerDebugLevel()>=1) {
      if(doneAnalyses.size()>0) {
#ifndef DISABLE_SIGHT
        SIGHT_VERB_IF(1, composerDebugLevel)
          set<PartPtr> startStates = GetStartAStates(currentAnalysis);
        {
          scope s("startStates");
          for(set<PartPtr>::iterator s=startStates.begin(); s!=startStates.end(); ++s)
            dbg << (*s? (*s)->str(): "NULL")<<endl;
        }
        set<PartPtr> endStates   = GetEndAStates(currentAnalysis);
        ostringstream fName; fName << "ats." << i << "." << doneAnalyses.back()->str();
        ats2dot(fName.str(), "ATS", startStates, endStates);

        SIGHT_VERB_FI()
#endif

          //      ats2dot_bw(fName.str()+".BW", "ATS", startStates, endStates);

          cout << "Analysis "<<currentAnalysis->str()<<endl;
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

      ConstantPropagationAnalysis* cp = dynamic_cast<ConstantPropagationAnalysis*>(currentAnalysis);
      DeadPathElimAnalysis* dp = dynamic_cast<DeadPathElimAnalysis*>(currentAnalysis);
      if(dp || cp) {
        doneCPDP.push(currentAnalysis);
      }

      // Record that we've completed the given analysis
      doneAnalyses.push_back(*a);

      if(doneCPDP.size() > 2) {
        ComposedAnalysis* priorCPorDP = doneCPDP.front();
        doneCPDP.pop();
        map<PartPtr, NodeState*> stateMap = NodeState::getNodeStateMap(priorCPorDP);
        std::cout << "analysis=" << priorCPorDP << ", stateMap=" << &stateMap << ", size=" << stateMap.size() << endl;
        map<PartPtr, NodeState*>::iterator strt, end;
        strt = stateMap.begin();
        end = stateMap.end();
        stateMap.erase(strt, end);
      }

      currentAnalysis = NULL;
    }

    if(testAnalysis) {
      // Create a CCQueryServers object to route queries from the upcoming analysis to the corresponding servers
      // If this is the first analysis to follow the syntactic analysis
      if(doneAnalyses.size()==1) queryInfo[testAnalysis] = CCQueryServers(doneAnalyses.back());
      else                       queryInfo[testAnalysis] = CCQueryServers(queryInfo[doneAnalyses.back()], doneAnalyses.back());

      //if(doneAnalyses.size()>1 && composerDebugLevel()>=1) {
      if(doneAnalyses.size()>1) {
#ifndef DISABLE_SIGHT      
        SIGHT_VERB_IF(1, composerDebugLevel)
          set<PartPtr> startStates = GetStartAStates(doneAnalyses.back());
        dbg << "ChainComposer::runAnalysis() #startStates="<<startStates.size()<<endl;
        set<PartPtr> endStates   = GetEndAStates(*(allAnalyses.begin()));
        ostringstream fName; fName << "ats." << i << "." << doneAnalyses.back()->str();
        ats2dot(fName.str(), "ATS", startStates, endStates);
        //      ats2dot_bw(fName.str()+".BW", "ATS", startStates, endStates);
        SIGHT_VERB_FI()
#endif
          }

      /*list<string> contextAttrs;
        contextAttrs.push_back("ReqType");
        trace opTimesT("OpTimes", contextAttrs, trace::showEnd, trace::boxplot);*/

      //scope s("---", scope::high, attrGE("composerDebugLevel", 1));
      SIGHT_VERB_DECL(scope, ("---", scope::high), 1, composerDebugLevel);
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

  // Given a Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisParts object that contains all the Parts relevant for analysis.
  AnalysisParts ChainComposer::NodeState2All(PartPtr part, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
    return AnalysisParts(NodeStates_valid? part:                 NULLPart, NodeStates_valid,
                         indexes_valid?    part->getInputPart(): NULLPart, indexes_valid,
                         inputs_valid?     part:                 NULLPart, inputs_valid);
  }

  // Given a PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis.
  AnalysisPartEdges ChainComposer::NodeState2All(PartEdgePtr pedge, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
    return AnalysisPartEdges(NodeStates_valid? pedge:                     NULLPartEdge, NodeStates_valid,
                             indexes_valid?    pedge->getInputPartEdge(): NULLPartEdge, indexes_valid,
                             inputs_valid?     pedge:                     NULLPartEdge, inputs_valid);
  }

  // Given a Part from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisParts object that contains the input and index the Parts relevant for analysis.
  AnalysisParts ChainComposer::Input2Index(PartPtr part, bool indexes_valid, bool inputs_valid) {
    return AnalysisParts(NULLPart,                                         false,
                         indexes_valid?    part->getInputPart(): NULLPart, indexes_valid,
                         inputs_valid?     part:                 NULLPart, inputs_valid);
  }

  // Given a PartEdge from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisPartEdges object that contains the input and index the Parts relevant for analysis.
  AnalysisPartEdges ChainComposer::Input2Index(PartEdgePtr pedge, bool indexes_valid, bool inputs_valid) {
    return AnalysisPartEdges(NULLPartEdge,                                              false,
                             indexes_valid?    pedge->getInputPartEdge(): NULLPartEdge, indexes_valid,
                             inputs_valid?     pedge:                     NULLPartEdge, inputs_valid);
  }

  string ChainComposer::str(string indent) const {
    ostringstream oss;
    oss << "[ChainComposer: ";
    for(list<ComposedAnalysis*>::const_iterator a=allAnalyses.begin(); a!=allAnalyses.end(); ) {
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
      return boost::make_shared<CombinedMemRegionObject>(Union, client, client->Expr2MemRegion(n, pedge));
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
      return boost::make_shared<CombinedMemLocObject>(Union, client, client->Expr2MemLoc(n, pedge));
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
  /*bool LooseParallelComposer::mayEqualAO(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->mayEqualAO(cl1, cl2, pedge, this); }*/
  bool LooseParallelComposer::mayEqualMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->mayEqualMR(mr1, mr2, pedge, this); }
  /*bool LooseParallelComposer::mayEqualAO(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->mayEqualAO(ml1, ml2, pedge, this); }*/

  // Returns whether the given pair of AbstractObjects are must-equal at the given PartEdge
  bool LooseParallelComposer::mustEqualV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->mustEqualV(val1, val2, pedge, this); }
  /*bool LooseParallelComposer::mustEqualAO(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->mustEqualAO(cl1, cl2, pedge, this); }*/
  bool LooseParallelComposer::mustEqualMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->mustEqualMR(mr1, mr2, pedge, this); }
  /*bool LooseParallelComposer::mustEqualAO(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->mustEqualAO(ml1, ml2, pedge, this); }*/

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool LooseParallelComposer::equalSetV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->equalSetV(val1, val2, pedge, this); }
  /*bool LooseParallelComposer::equalSetAO(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->equalSetAO(cl1, cl2, pedge, this); }*/
  bool LooseParallelComposer::equalSetMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->equalSetMR(mr1, mr2, pedge, this); }
  /*bool LooseParallelComposer::equalSetAO(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->equalSetAO(ml1, ml2, pedge, this); }*/

  // Returns whether abstract object ao1 denotes a non-strict subset (the sets may be equal) of the set denoted
  // by the abstract object ao2.
  bool LooseParallelComposer::subSetV (ValueObjectPtr  val1, ValueObjectPtr  val2, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->subSetV(val1, val2, pedge, this); }
  /*bool LooseParallelComposer::subSetAO(CodeLocObjectPtr cl1, CodeLocObjectPtr cl2, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->subSetAO(cl1, cl2, pedge, this); }*/
  bool LooseParallelComposer::subSetMR(MemRegionObjectPtr  mr1, MemRegionObjectPtr  mr2, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->subSetMR(mr1, mr2, pedge, this); }
  /*bool LooseParallelComposer::subSetAO(MemLocObjectPtr  ml1, MemLocObjectPtr  ml2, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->subSetAO(ml1, ml2, pedge, this); }*/

  // Returns whether the given AbstractObject is live at the given part edge
  bool LooseParallelComposer::isLiveV (ValueObjectPtr val,  PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->isLiveV(val, pedge, this); }
  /*bool LooseParallelComposer::isLiveAO(CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->isLiveAO(cl, pedge, this); }*/
  bool LooseParallelComposer::isLiveMR(MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->isLiveMR(mr, pedge, this); }
  /*bool LooseParallelComposer::isLiveAO(MemLocObjectPtr ml,  PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->isLiveAO(ml, pedge, this); }*/

  // Calls the isLive() method of the given AbstractObject that denotes an operand of the given SgNode n within
  // the context of its own PartEdges and returns true if it may be live within any of them
  bool LooseParallelComposer::OperandIsLiveV (SgNode* n, SgNode* operand, ValueObjectPtr val,  PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->OperandIsLiveV(n, operand, val, pedge, this); }
  /*bool LooseParallelComposer::OperandIsLiveAO(SgNode* n, SgNode* operand, CodeLocObjectPtr cl, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->OperandIsLiveAO(n, operand, cl, pedge, this); }*/
  bool LooseParallelComposer::OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr,  PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->OperandIsLiveMR(n, operand, mr, pedge, this); }
  /*bool LooseParallelComposer::OperandIsLiveAO(SgNode* n, SgNode* operand, MemLocObjectPtr ml,  PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->OperandIsLiveAO(n, operand, ml, pedge, this); }*/

  // Computes the meet of from and to and saves the result in to.
  // Returns true if this causes this to change and false otherwise.
  bool LooseParallelComposer::meetUpdateV    (ValueObjectPtr   to, ValueObjectPtr   from, PartEdgePtr pedge, ComposedAnalysis* analysis)
  { return getComposer()->meetUpdateV(to, from, pedge, this); }
  /*bool LooseParallelComposer::meetUpdateAO(CodeLocObjectPtr to, CodeLocObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis)
    { return getComposer()->meetUpdateAO(to, from, pedge, this); }*/
  bool LooseParallelComposer::meetUpdateMR (MemRegionObjectPtr  to, MemRegionObjectPtr  from, PartEdgePtr pedge, ComposedAnalysis* analysis)
  { return getComposer()->meetUpdateMR(to, from, pedge, this); }
  /*bool LooseParallelComposer::meetUpdateML (MemLocObjectPtr  to, MemLocObjectPtr  from, PartEdgePtr pedge, ComposedAnalysis* analysis)
    { return getComposer()->meetUpdateAO(to, from, pedge, this); }*/
  // Returns whether the given AbstractObject corresponds to the set of all sub-executions
  bool LooseParallelComposer::isFullV (ValueObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->isFullV(ao, pedge, this); }
  /*bool LooseParallelComposer::isFullAO(CodeLocObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->isFullAO(ao, pedge, this); }*/
  bool LooseParallelComposer::isFullMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->isFullMR(ao, pedge, this); }
  /*bool LooseParallelComposer::isFullAO(MemLocObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->isFullAO(ao, pedge, this); }*/


  // Returns whether the given AbstractObject corresponds to the the empty set of sub-executions
  bool LooseParallelComposer::isEmptyV (ValueObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->isEmptyV(ao, pedge, this); }
  /*bool LooseParallelComposer::isEmptyAO(CodeLocObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->isEmptyAO(ao, pedge, this); }*/
  bool LooseParallelComposer::isEmptyMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->isEmptyMR(ao, pedge, this); }

  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr LooseParallelComposer::getRegionSizeMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
  { return getComposer()->getRegionSizeMR(ao, pedge, this); }

  /*bool LooseParallelComposer::isEmptyAO(MemLocObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* client)
    { return getComposer()->isEmptyAO(ao, pedge, this); }*/

  // Return the anchor Parts of a given function
  set<PartPtr> LooseParallelComposer::GetStartAStates(ComposedAnalysis* client)
  { return getComposer()->GetStartAStates(this); }
  set<PartPtr> LooseParallelComposer::GetEndAStates(ComposedAnalysis* client)
  { return getComposer()->GetEndAStates(this); }

  // Return an SSAGraph object that describes the overall structure of the transition system
  SSAGraph* LooseParallelComposer::GetSSAGraph(ComposedAnalysis* client)
  { ROSE_ASSERT(0); }

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
      //scope reg(sight::txt()<< "LooseParallelComposer Running Analysis "<<i<<": "<<(*a)->str(""), scope::high, attrGE("composerDebugLevel", 1));
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (sight::txt()<< "LooseParallelComposer Running Analysis "<<i<<": "<<(*a)->str(""), scope::high), 1, composerDebugLevel);
#endif

      (*a)->runAnalysis();

#ifndef DISABLE_SIGHT
      dbg << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Analysis finished" << endl;
#endif

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
      //scope reg(sight::txt()<<"Expr2Val  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (sight::txt()<<"Expr2Val  : " << (*a)->str(""), scope::medium), 1, composerDebugLevel)
#endif

        try {
          ValueObjectPtr val = (*a)->Expr2Val(n, getEdgeForAnalysis(pedge, *a));
#ifndef DISABLE_SIGHT
          dbg << "Returning "<<val->str("")<<endl;
#endif
          vals.push_back(val);
        } catch (NotImplementedException exc) {
          //if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2Val() Not Implemented."<<endl;
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2Val() Not Implemented."<<endl, 1, composerDebugLevel)
#endif
            // If control reaches here then the current analysis must not implement
            // this method so we ask the remaining analyses
            continue;
        }
    }

    // If no sub-analysis implements this query, forward it to the composer
    if(vals.size()==0) {
      return getComposer()->Expr2Val(n, pedge, this);
    } else
      return boost::make_shared<CombinedValueObject>(Intersection, this, vals);
  }

  CodeLocObjectPtr LooseParallelComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge)
  {
    // List of code location that will be intersected and returned
    list<CodeLocObjectPtr> cls;

    for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
      //scope reg(sight::txt()<<"Expr2CodeLoc  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (sight::txt()<<"Expr2CodeLoc  : " << (*a)->str(""), scope::medium), 1, composerDebugLevel)
#endif

        try {
          CodeLocObjectPtr cl = (*a)->Expr2CodeLoc(n, getEdgeForAnalysis(pedge, *a));
          SIGHT_VERB(dbg << "Returning "<<cl->str("")<<endl, 1, composerDebugLevel)
            cls.push_back(cl);
        } catch (NotImplementedException exc) {
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2CodeLoc() Not Implemented."<<endl, 1, composerDebugLevel)
#endif
            // If control reaches here then the current analysis must not implement
            // this method so we ask the remaining analyses
            continue;
        }
    }

    // If no sub-analysis implements this query, forward it to the composer
    if(cls.size()==0) {
      return getComposer()->Expr2CodeLoc(n, pedge, this);
    } else
      return boost::make_shared<CombinedCodeLocObject>(Intersection, this, cls);
  }


  MemRegionObjectPtr  LooseParallelComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge)
  {
    // List of memory location that will be intersected and returned
    list<MemRegionObjectPtr> mrs;

    for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
      //scope reg(sight::txt()<<"Expr2MemRegion  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (sight::txt()<<"Expr2MemRegion  : " << (*a)->str(""), scope::medium), 1, composerDebugLevel)
#endif

        try {
          MemRegionObjectPtr mr = (*a)->Expr2MemRegion(n, getEdgeForAnalysis(pedge, *a));
          //if(composerDebugLevel() >= 1) { dbg << "Returning "<<mr->str("")<<endl; }
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "Returning "<<mr->str("")<<endl, 1, composerDebugLevel)
#endif
            mrs.push_back(mr);
        } catch (NotImplementedException exc) {
          //if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2MemRegion() Not Implemented."<<endl;
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2MemRegion() Not Implemented."<<endl, 1, composerDebugLevel)
#endif
            // If control reaches here then the current analysis must not implement
            // this method so we ask the remaining analyses
            continue;
        }
    }

    // If no sub-analysis implements this query, forward it to the composer
    return boost::make_shared<CombinedMemRegionObject>(Intersection, this, mrs);
  }


  MemLocObjectPtr  LooseParallelComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge)
  {
    // List of memory location that will be intersected and returned
    list<MemLocObjectPtr> mls;

    for(list<ComposedAnalysis*>::iterator a=allAnalyses.begin(); a!=allAnalyses.end(); a++) {
      //scope reg(sight::txt()<<"Expr2MemLoc  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (sight::txt()<<"Expr2MemLoc  : " << (*a)->str(""), scope::medium), 1, composerDebugLevel)
#endif

        try {
          MemLocObjectPtr ml = (*a)->Expr2MemLoc(n, getEdgeForAnalysis(pedge, *a));
          //if(composerDebugLevel() >= 1) { dbg << "Returning "<<ml->str("")<<endl; }
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "Returning "<<ml->str("")<<endl, 1, composerDebugLevel)
#endif
            mls.push_back(ml);
        } catch (NotImplementedException exc) {
          //if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2MemLoc() Not Implemented."<<endl;
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "&nbsp;&nbsp;&nbsp;&nbsp;Expr2MemLoc() Not Implemented."<<endl, 1, composerDebugLevel)
#endif
            // If control reaches here then the current analysis must not implement
            // this method so we ask the remaining analyses
            continue;
        }
    }

    // If no sub-analysis implements this query, forward it to the composer
    return boost::make_shared<CombinedMemLocObject>(Intersection, this, mls);
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
    scope reg(sight::txt()<<"GetStartAState  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));

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
        //scope reg(sight::txt()<<funcName<<"  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));
#ifndef DISABLE_SIGHT
        SIGHT_VERB_DECL(scope, (sight::txt()<<funcName<<"  : " << (*a)->str(""), scope::medium), 1, composerDebugLevel)
#endif

          try {
            //set<PartPtr> curParts = (*a)->GetEndAStates();
            set<PartPtr> curParts = caller.callGetStartOrEndAStates_ComposedAnalysis(*a);

            // If this is the first analysis, simply copy its curParts into intersection
            if(a==allAnalyses.begin()) {
              for(set<PartPtr>::iterator cur=curParts.begin(); cur!=curParts.end(); cur++)
                intersection[(*cur)->getInputPart()][*a] = *cur;
              // If this is not the first analysis, intersect curParts with intersection, storing the result in intersection
            } else {
              map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator i=intersection.begin();
              set<PartPtr>::iterator curI=curParts.begin();
              while(i!=intersection.end() && curI!=curParts.end()) {
                // If i and curI have the same parent Part, it must be kept in the intersection of intersection and curParts
                if((*curI)->getInputPart() == i->first) {
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
            //if(composerDebugLevel()>=1) dbg << "&nbsp;&nbsp;&nbsp;&nbsp;"<<funcName<<"() Not Implemented."<<endl;
#ifndef DISABLE_SIGHT
            SIGHT_VERB(dbg << "&nbsp;&nbsp;&nbsp;&nbsp;"<<funcName<<"() Not Implemented."<<endl, 1, composerDebugLevel)
#endif
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

      //if(composerDebugLevel() >= 1) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_IF(1, composerDebugLevel)
        dbg << "Returning ";
      for(map<PartPtr, map<ComposedAnalysis*, PartPtr> >::iterator i=intersection.begin(); i!=intersection.end(); i++) {
        indent ind();
        dbg << "Parent: "<<i->first.get()->str()<<endl;
        for(map<ComposedAnalysis*, PartPtr>::iterator j=i->second.begin(); j!=i->second.end(); j++) {
          indent ind();
          dbg << j->first->str() << " =&gt; " << j->second->str() << endl;
        }
      }
      SIGHT_VERB_FI()
#endif

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
    scope reg(sight::txt()<<"GetEndAState  : " << (*a)->str(""), scope::medium, attrGE("composerDebugLevel", 1));

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
    interParts.insert(makePtr<IntersectionPart>(singleton, (*i)->getSupersetPart(), this));
    }
    return interParts;
    }
    }*/

  // Given a Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisParts object that contains all the Parts relevant for analysis.
  AnalysisParts LooseParallelComposer::NodeState2All(PartPtr part, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
    return AnalysisParts(NodeStates_valid? part:                 NULLPart, NodeStates_valid,
                         indexes_valid?    part->getInputPart(): NULLPart, indexes_valid,
                         inputs_valid?     part:                 NULLPart, inputs_valid);
  }

  // Given a PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis.
  AnalysisPartEdges LooseParallelComposer::NodeState2All(PartEdgePtr pedge, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
    return AnalysisPartEdges(NodeStates_valid? pedge:                     NULLPartEdge, NodeStates_valid,
                             indexes_valid?    pedge->getInputPartEdge(): NULLPartEdge, indexes_valid,
                             inputs_valid?     pedge:                     NULLPartEdge, inputs_valid);
  }

  // Given a Part from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisParts object that contains the input and index the Parts relevant for analysis.
  AnalysisParts LooseParallelComposer::Input2Index(PartPtr part, bool indexes_valid, bool inputs_valid) {
    return AnalysisParts(NULLPart,                                         false,
                         indexes_valid?    part->getInputPart(): NULLPart, indexes_valid,
                         inputs_valid?     part:                 NULLPart, inputs_valid);
  }

  // Given a PartEdge from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisPartEdges object that contains the input and index the Parts relevant for analysis.
  AnalysisPartEdges LooseParallelComposer::Input2Index(PartEdgePtr pedge, bool indexes_valid, bool inputs_valid) {
    return AnalysisPartEdges(NULLPartEdge,                                              false,
                             indexes_valid?    pedge->getInputPartEdge(): NULLPartEdge, indexes_valid,
                             inputs_valid?     pedge:                     NULLPartEdge, inputs_valid);
  }

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

  string LooseParallelComposer::str(string indent) const {
    ostringstream oss;
    oss << "[LooseParallelComposer: ";
    for(list<ComposedAnalysis*>::const_iterator a=allAnalyses.begin(); a!=allAnalyses.end(); ) {
      oss << (*a)->str();
      a++;
      if(a!=allAnalyses.end()) oss << ", ";
    }
    oss << "]";
    return oss.str();
  }

}; //namespace fuse;
