/*****************************************
 * author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "sage3basic.h"
using namespace std;

#include "tight_composer.h"
#include <boost/enable_shared_from_this.hpp>
#include "sight.h"
#include "stx_analysis.h"
#include "ssa.h"
#include <algorithm>


using namespace boost;

#ifndef DISABLE_SIGHT
using namespace sight;
#endif

#define tightComposerDebugLevel 0

namespace fuse {

  // The cost of managing TightCompositionQueryManager::queryStateMap can grow rather large
  // and the dominant cost of this is comparing pedges when using Expr2AnyKey as keys. However,
  // the extra precision of comparing using pedges is often not needed so this flag can be used
  // to control whether it is compiled in or not.
#define EXPR2ANYKEY_COMPARES_USING_PEDGE false
  /***************
   * Expr2AnyKey *
   ***************/

  bool Expr2AnyKey::operator<(const Expr2AnyKey& that) const {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"Expr2AnyKey::<", scope::medium), 4, tightComposerDebugLevel)

      SIGHT_VERB_IF(4, tightComposerDebugLevel)
      dbg << "this=" << str() << endl;
    dbg << "that=" << that.str() << endl;
    SIGHT_VERB_FI()
#endif
      // Order by the sgn expression first
      if(sgn < that.sgn) {
        // if(tightComposerDebugLevel >= 4) dbg << "this < that=" << "true";
        return true;
      }
#if EXPR2ANYKEY_COMPARES_USING_PEDGE==true
    // Order by PartEdge if the expression is same
      else if(sgn == that.sgn && pedge < that.pedge) {
        // if(tightComposerDebugLevel >= 4) dbg << "this < that=" << "true";
        return true;
      }
#endif
    // Order by reqtype if both expression and PartEdge are same
      else if(sgn == that.sgn &&
#if EXPR2ANYKEY_COMPARES_USING_PEDGE==true
              pedge == that.pedge &&
#endif
              reqtype < that.reqtype) {
        // if(tightComposerDebugLevel >= 4) dbg << "this < that=" << "true";
        return true;
      }
      else {
        // if(tightComposerDebugLevel >= 4) dbg << "this < that=" << "false";
        return false;
      }
  }

  std::string Expr2AnyKey::str(std::string indent) const {
    std::ostringstream oss;
    oss << "[sgn=" << sgn << "," << SgNode2Str(sgn) <<",";
    if(pedge) oss << "pedge=" << pedge->str() <<",";
    oss << " reqtype=";
    switch(reqtype) {
    case 0: oss << "any";       break;
    case 1: oss << "codeloc";   break;
    case 2: oss << "val";       break;
    case 3: oss << "memloc";    break;
    case 4: oss << "memregion"; break;
    case 5: oss << "atsGraph";  break;
    default: assert(0);
    }
    oss << "]";
    return oss.str();
  }


  /********************************
   * TightCompositionQueryManager *
   ********************************/

  void TightCompositionQueryManager::initializeQuery(Expr2AnyKey key) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"initializeQuery", scope::medium), 4, tightComposerDebugLevel)

      SIGHT_VERB_IF(4, tightComposerDebugLevel)
      dbg << "key=" << key.str() << endl;
    dbg << this->str();
    SIGHT_VERB_FI()
#endif

      // There is no prior entry in the map
      map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    assert(loc == queryStateMap.end());
    pair<Expr2AnyKey, Expr2AnyState> elem = make_pair(key, Expr2AnyState());
    queryStateMap.insert(elem);
#ifndef DISABLE_SIGHT
    if(tightComposerDebugLevel >= 4) dbg << this->str();
#endif
  }

  const Expr2AnyState TightCompositionQueryManager::getQueryState(Expr2AnyKey key) const {
    map<Expr2AnyKey, Expr2AnyState>::const_iterator loc = queryStateMap.find(key);
    assert(loc != queryStateMap.end());
    return loc->second;
  }

  bool TightCompositionQueryManager::isQueryCached(Expr2AnyKey key) {
    map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    if(loc == queryStateMap.end())
      return false;

    Expr2AnyState& qstate = loc->second;
    if(qstate.state == Expr2AnyState::finished) return true;

    return false;
  }

  bool TightCompositionQueryManager::isLoopingQuery(Expr2AnyKey key, ComposedAnalysis* analysis_) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"isLoopingQuery", scope::medium), 4, tightComposerDebugLevel)

      SIGHT_VERB_IF(4, tightComposerDebugLevel)
      dbg << "key=" << key.str() << endl;
    dbg << this->str();
    SIGHT_VERB_FI()
#endif

      map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    if(loc == queryStateMap.end()) return false;

    Expr2AnyState& qstate = loc->second;

    // if(tightComposerDebugLevel >= 4) {
    //   dbg << "key=" << key.str() << " found\n";
    //   dbg << "state=" << qstate.str() << "\n";
    // }

    // query is not in the analysis state
    if(qstate.state != Expr2AnyState::anal) return false;

    // if(tightComposerDebugLevel >= 4) {
    //   dbg << "LastAnalysis=" << qstate.getLastAnalysisQueried() << endl;
    //   dbg << "client=" << analysis_ << endl;
    // }

    // query is in the analysis state
    if(qstate.getLastAnalysisQueried() == analysis_) return true;

    return false;
  }

  void TightCompositionQueryManager::transToAnalState(Expr2AnyKey key, ComposedAnalysis* analysis_) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"transToAnalState", scope::medium), 4, tightComposerDebugLevel)

      SIGHT_VERB_IF(4, tightComposerDebugLevel)
      dbg << "key=" << key.str() << endl;
    dbg << this->str();
    SIGHT_VERB_FI()
#endif

      // key is already in the map
      map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    assert(loc != queryStateMap.end());
    Expr2AnyState& qstate = loc->second;
    qstate.setAnalysis(analysis_);
  }

  void TightCompositionQueryManager::transToFinishedState(Expr2AnyKey key) {
    /*    cout << "transToFinishedState #queryStateMap="<<queryStateMap.size()<<endl;
          for(map<Expr2AnyKey, Expr2AnyState>::iterator i=queryStateMap.begin(); i!=queryStateMap.end(); ++i)
          cout << i->first.str() << ": "<< i->second.str()<<endl;*/
    // key is already in the map
    map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    assert(loc != queryStateMap.end());
    Expr2AnyState& qstate = loc->second;
    qstate.setFinished();
    queryStateMap.erase(loc);
  }

  string TightCompositionQueryManager::str(string indent) const {
    ostringstream oss;
    oss << "QueryStateMap=[";
    map<Expr2AnyKey, Expr2AnyState>::const_iterator it;
    for(it = queryStateMap.begin(); it != queryStateMap.end(); ++it) {
      if(it != queryStateMap.begin()) oss << "\n";
      oss << "query=" << it->first.str() << ", state=" << it->second.str();
    }
    oss << "]\n";
    return oss.str();
  }

  void TightCompositionQueryManagerTest::initializeQuery(Expr2AnyKey key) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"initializeQuery", scope::medium), 4, tightComposerDebugLevel)

      SIGHT_VERB_IF(4, tightComposerDebugLevel)
      dbg << "key=" << key.str() << endl;
    dbg << this->str();
    SIGHT_VERB_FI()
#endif

      // There is no prior entry in the map
      //cout << "key=" << key.str() << endl;
      //cout << this->str();
      //cout << "    ----\n";
      map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    //cout << "    ----\n";
    assert(loc == queryStateMap.end());
    pair<Expr2AnyKey, Expr2AnyState> elem = make_pair(key, Expr2AnyState());
    queryStateMap.insert(elem);
#ifndef DISABLE_SIGHT
    if(tightComposerDebugLevel >= 4) dbg << this->str();
#endif
  }

  const Expr2AnyState TightCompositionQueryManagerTest::getQueryState(Expr2AnyKey key) const {
    map<Expr2AnyKey, Expr2AnyState>::const_iterator loc = queryStateMap.find(key);
    assert(loc != queryStateMap.end());
    return loc->second;
  }

  bool TightCompositionQueryManagerTest::isQueryCached(Expr2AnyKey key) {
    map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    if(loc == queryStateMap.end())
      return false;

    Expr2AnyState& qstate = loc->second;
    if(qstate.state == Expr2AnyState::finished) return true;

    return false;
  }

  bool TightCompositionQueryManagerTest::isLoopingQuery(Expr2AnyKey key, ComposedAnalysis* analysis_) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"isLoopingQuery", scope::medium), 4, tightComposerDebugLevel)

      SIGHT_VERB_IF(4, tightComposerDebugLevel)
      dbg << "key=" << key.str() << endl;
    dbg << this->str();
    SIGHT_VERB_FI()
#endif

      map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    if(loc == queryStateMap.end()) return false;

    Expr2AnyState& qstate = loc->second;

    // if(tightComposerDebugLevel >= 4) {
    //   dbg << "key=" << key.str() << " found\n";
    //   dbg << "state=" << qstate.str() << "\n";
    // }

    // query is not in the analysis state
    if(qstate.state != Expr2AnyState::anal) return false;

    // if(tightComposerDebugLevel >= 4) {
    //   dbg << "LastAnalysis=" << qstate.getLastAnalysisQueried() << endl;
    //   dbg << "client=" << analysis_ << endl;
    // }

    // query is in the analysis state
    if(qstate.getLastAnalysisQueried() == analysis_) return true;

    return false;
  }

  void TightCompositionQueryManagerTest::transToAnalState(Expr2AnyKey key, ComposedAnalysis* analysis_) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"transToAnalState", scope::medium), 4, tightComposerDebugLevel)

      SIGHT_VERB_IF(4, tightComposerDebugLevel)
      dbg << "key=" << key.str() << endl;
    dbg << this->str();
    SIGHT_VERB_FI()
#endif

      // key is already in the map
      map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    assert(loc != queryStateMap.end());
    Expr2AnyState& qstate = loc->second;
    qstate.setAnalysis(analysis_);
  }

  void TightCompositionQueryManagerTest::transToFinishedState(Expr2AnyKey key) {
    /*    cout << "transToFinishedState #queryStateMap="<<queryStateMap.size()<<endl;
          for(map<Expr2AnyKey, Expr2AnyState>::iterator i=queryStateMap.begin(); i!=queryStateMap.end(); ++i)
          cout << i->first.str() << ": "<< i->second.str()<<endl;*/
    // key is already in the map
    map<Expr2AnyKey, Expr2AnyState>::iterator loc = queryStateMap.find(key);
    assert(loc != queryStateMap.end());
    Expr2AnyState& qstate = loc->second;
    qstate.setFinished();
    queryStateMap.erase(loc);
  }

  string TightCompositionQueryManagerTest::str(string indent) const {
    ostringstream oss;
    oss << "QueryStateMap=[";
    map<Expr2AnyKey, Expr2AnyState>::const_iterator it;
    for(it = queryStateMap.begin(); it != queryStateMap.end(); ++it) {
      if(it != queryStateMap.begin()) oss << "\n";
      oss << "query=" << it->first.str() << ", state=" << it->second.str();
    }
    oss << "]\n";
    return oss.str();
  }

  /*****************
   * TightComposer *
   *****************/

  TightComposer::TightComposer(const std::list<ComposedAnalysis*>& analyses, bool trackBase2RefinedPartEdgeMapping, bool useSSA) :
    ComposedAnalysis(trackBase2RefinedPartEdgeMapping, useSSA), allAnalyses(analyses) {
    list<ComposedAnalysis*>::iterator a=allAnalyses.begin();
    // get the first analysis' direction
    // all other analyses should be in the same direction
    dir = (*a)->getDirection();
    for( ; a!=allAnalyses.end(); ++a) {
      // Inform each analysis of the composer's identity
      (*a)->setComposer(this);
      assert(dir == (*a)->getDirection());
    }
  }

  TightComposer::TightComposer(const TightComposer& that) :
    ComposedAnalysis(that), allAnalyses(that.allAnalyses), dir(that.dir) {
  }

  void TightComposer::initializeQueryList(list<Expr2AnyKey>& queryList) {
    list<Expr2AnyKey>::const_iterator qIt = queryList.begin();
    for( ; qIt != queryList.end(); ++qIt) {
      tcqm.initializeQuery(*qIt);
    }
  }

  bool TightComposer::recursiveQueries(list<Expr2AnyKey>& queryList, ComposedAnalysis* client) {
    list<Expr2AnyKey>::const_iterator qIt = queryList.begin();
    bool loop = tcqm.isLoopingQuery(*qIt, client);
    for( ++qIt; qIt != queryList.end(); ++qIt) {
      assert(loop == tcqm.isLoopingQuery(*qIt, client));
    }
    return loop;
  }

  void TightComposer::finalizeQueryList(list<Expr2AnyKey>& queryList) {
    list<Expr2AnyKey>::const_iterator qIt = queryList.begin();
    for( ; qIt != queryList.end(); ++qIt) {
      tcqm.transToFinishedState(*qIt);
    }
  }

  void TightComposer::initializeQueryListTest(list<Expr2AnyKey>& queryList) {
    list<Expr2AnyKey>::const_iterator qIt = queryList.begin();
    for( ; qIt != queryList.end(); ++qIt) {
      tcqmTest.initializeQuery(*qIt);
    }
  }

  bool TightComposer::recursiveQueriesTest(list<Expr2AnyKey>& queryList, ComposedAnalysis* client) {
    list<Expr2AnyKey>::const_iterator qIt = queryList.begin();
    bool loop = tcqmTest.isLoopingQuery(*qIt, client);
    for( ++qIt; qIt != queryList.end(); ++qIt) {
      assert(loop == tcqmTest.isLoopingQuery(*qIt, client));
    }
    return loop;
  }

  void TightComposer::finalizeQueryListTest(list<Expr2AnyKey>& queryList) {
    list<Expr2AnyKey>::const_iterator qIt = queryList.begin();
    for( ; qIt != queryList.end(); ++qIt) {
      tcqmTest.transToFinishedState(*qIt);
    }
  }

  //! Generic method for forwarding Expr2Any queries to analyses.
  //! \param n SgNode* expression on which the query is performed.
  //! \param pedgeList List of PartEdges on which the query needs to be forwarded.
  //! \param client Analysis* making the Expr2Any query.
  //! \param reqtype Query type which is either memloc, memregion, value or codeloc.
  //! \param implementsExpr2AnyOp boost function pointer used by this method to check if an
  //! analysis implements the interface to answer this query.
  //! \param Expr2AnyOp boost function pointer to query interface method
  //! implemented by the analysis.
  //! \param parentComposerExpr2AnyOp boost function pointer to parent composers query interface
  //! method to answer this query.
  //! All queries to the TightComposer are implemented by this method.
  //! TightCompositionQueryManager maintains the state of each query.
  //! If the query is already cached then the cached object is returned.
  //! If the query is a recurring query then Full object is returned to break the cycle.
  //! If the query is not in finished state or not a recurring query then it is forwarded to each analysis.
  //! For each analysis if it implements the interface the query is forwared on each PartEdge from pedgeList.
  //! The objects from single analysis on different PartEdges are combined using the template parameter CombinedAOType.
  //! The CombinedAOType objects from different analyses are combined using AnalMapAOType.
  template<class AOType, class FullAOType, class CombinedAOType, class AnalysisMapAOType>
  boost::shared_ptr<AOType> TightComposer::Expr2Any(string opName,
                                                    list<Expr2AnyKey> queryList,
                                                    PartEdgePtr pedge,
                                                    ComposedAnalysis* client,
                                                    boost::function<bool (ComposedAnalysis*)> implementsExpr2AnyOp,
                                                    boost::function<boost::shared_ptr<AOType> (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp,
                                                    boost::function<boost::shared_ptr<AOType> (SgNode*, PartEdgePtr)> parentComposerExpr2AnyOp) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::Expr2Any", scope::medium), 3, tightComposerDebugLevel)
#endif

      list<Expr2AnyKey>::iterator qIt;

#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(3, tightComposerDebugLevel)
      dbg << "queryList=[\n";
    for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
      dbg << (*qIt).str() << endl;
    }
    dbg << "]\n";
    dbg << "queryMap=[" << tcqm.str() << "]\n";
    SIGHT_VERB_FI()
#endif
      
      if(recursiveQueries(queryList, client)) {
        // return boost::make_shared<FullAOType>();
        boost::shared_ptr<AnalysisMapAOType> amao_p = boost::make_shared<AnalysisMapAOType>(Intersection, client);
        // Query the parent composer
        boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
        for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
          Expr2AnyKey query = *qIt;
          boost::shared_ptr<AOType> ao_p = parentComposerExpr2AnyOp(query.sgn, query.pedge);
          SIGHT_VERB(dbg << dynamic_cast<ComposedAnalysis*>(getComposer())->str() << ":" << ao_p->str() << endl, 3, tightComposerDebugLevel)
            cao_p->add(ao_p, query.pedge, this, client);
        }
        amao_p->add(dynamic_cast<ComposedAnalysis*>(getComposer()), cao_p, pedge, this, client);
        return amao_p;
      }

    initializeQueryList(queryList);

    // The object that holds the result of the query
    boost::shared_ptr<AOType> res;

    // If there are multiple responses, this object holds their intersection
    boost::shared_ptr<AnalysisMapAOType> amao_p;// = boost::make_shared<AnalysisMapAOType>(Intersection, client);

    // Dispatch queries to each analysis
    int numRespondingAnalyses=0;
    for(list<ComposedAnalysis*>::iterator a = allAnalyses.begin(); a != allAnalyses.end(); ++a) {
      if(implementsExpr2AnyOp(*a)) {
        // If more than one analyses have responded to the query, create the intersection object to contain their results
        if(numRespondingAnalyses==1) {
          // Create the intersection and add the result of the first analysis to it
          amao_p = boost::make_shared<AnalysisMapAOType>(Intersection, client);
          amao_p->add(*a, res, pedge, this, client);
          // The intersection object is now the official result
          res = amao_p;
        }

        //        struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);
        // Union of the responses to all the queries
        boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
        for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
#ifndef DISABLE_SIGHT
          SIGHT_VERB_DECL(scope, (txt()<<"query "<<qIt->str()), 3, tightComposerDebugLevel)
#endif
            Expr2AnyKey query = *qIt;
          // Transition this query to analysis state corresponding to *a
          tcqm.transToAnalState(query, *a);

          // Dispatch the query to the analysis
          boost::shared_ptr<AOType> ao_p = Expr2AnyOp(*a, query.sgn, query.pedge);

#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << (*a)->str() << ":" << ao_p->str() << endl, 3, tightComposerDebugLevel)
#endif

            //dbg << "cao_p="<<cao_p->str()<<endl;
            cao_p->add(ao_p, query.pedge, this, client);
        }

        // If this is the first analysis we've discovered that implements this query,
        // set cao_p to be the result. If we discover that other analyses implement this
        // query we'll later create an intersection object and add this object to it
        if(numRespondingAnalyses==0)
          res = cao_p;
        // If this is not the first analysis to implement this query, add cao_p to the
        // intersection, which we must have already created.
        else {
          assert(cao_p);
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << "cao_p="<<cao_p->str()<<endl, 3, tightComposerDebugLevel)
#endif
            amao_p->add(*a, cao_p, pedge, this, client);
        }
        ++numRespondingAnalyses;
        //        gettimeofday(&gopeEnd, NULL); cout << "TightComposer::Expr2Any("<<(*a)->str()<<")\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
      }
    }

    {
      //      struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);
      // Query the parent composer
      if(numRespondingAnalyses==0) {
        boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
        for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
          Expr2AnyKey query = *qIt;
          tcqm.transToAnalState(query, dynamic_cast<ComposedAnalysis*>(getComposer()));

          boost::shared_ptr<AOType> ao_p = parentComposerExpr2AnyOp(query.sgn, query.pedge);

#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << dynamic_cast<ComposedAnalysis*>(getComposer())->str() << ":" << ao_p->str() << endl, 3, tightComposerDebugLevel)
#endif

            cao_p->add(ao_p, query.pedge, this, client);
        }
        res = cao_p;
        //amao_p->add(dynamic_cast<ComposedAnalysis*>(getComposer()), cao_p, pedge, this, client);
      }

      //    gettimeofday(&gopeEnd, NULL); cout << "TightComposer::Expr2Any(parent)\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
    }

#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << amao_p->str() << endl, 3, tightComposerDebugLevel)
#endif

      finalizeQueryList(queryList);
    //return amao_p;
    return res;
  }


  template<class AOType, class FullAOType, class CombinedAOType, class AnalysisMapAOType>
  boost::shared_ptr<AOType> TightComposer::Expr2AnyTest(string opName,
                                                        list<Expr2AnyKey> queryList,
                                                        PartEdgePtr pedge,
                                                        ComposedAnalysis* client,
                                                        boost::function<bool (ComposedAnalysis*)> implementsExpr2AnyOp,
                                                        boost::function<boost::shared_ptr<AOType> (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp,
                                                        boost::function<boost::shared_ptr<AOType> (SgNode*, PartEdgePtr)> parentComposerExpr2AnyOp) {
    struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::Expr2Any", scope::medium), 3, tightComposerDebugLevel)
#endif

      list<Expr2AnyKey>::iterator qIt;

#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(3, tightComposerDebugLevel)
      dbg << "queryList=[\n";
    for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
      dbg << (*qIt).str() << endl;
    }
    dbg << "]\n";
    dbg << "queryMap=[" << tcqm.str() << "]\n";
    SIGHT_VERB_FI()
#endif

      if(recursiveQueriesTest(queryList, client)) {
        // return boost::make_shared<FullAOType>();
        //      boost::shared_ptr<AnalysisMapAOType> amao_p = boost::make_shared<AnalysisMapAOType>(Intersection, client);
        // Query the parent composer
        boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
        for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
          Expr2AnyKey query = *qIt;
          boost::shared_ptr<AOType> ao_p = parentComposerExpr2AnyOp(query.sgn, query.pedge);
#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << dynamic_cast<ComposedAnalysis*>(getComposer())->str() << ":" << ao_p->str() << endl, 3, tightComposerDebugLevel)
#endif
            cao_p->add(ao_p, query.pedge, this, client);
        }
        //      amao_p->add(dynamic_cast<ComposedAnalysis*>(getComposer()), cao_p, pedge, this, client);
        gettimeofday(&gopeEnd, NULL); cout << "TightComposer::Expr2Any\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
        //      return amao_p;
        return cao_p;
      }

    initializeQueryListTest(queryList);

    // The object that holds the result of the query
    boost::shared_ptr<AOType> res;

    // If there are multiple responses, this object holds their intersection
    boost::shared_ptr<AnalysisMapAOType> amao_p;// = boost::make_shared<AnalysisMapAOType>(Intersection, client);

    // Dispatch queries to each analysis
    int numRespondingAnalyses=0;
    /*    for(list<ComposedAnalysis*>::iterator a = allAnalyses.begin(); a != allAnalyses.end(); ++a) {
          if(implementsExpr2AnyOp(*a)) {
          // If more than one analyses have responded to the query, create the intersection object to contain their results
          if(numRespondingAnalyses==1) {
          // Create the intersection and add the result of the first analysis to it
          amao_p = boost::make_shared<AnalysisMapAOType>(Intersection, client);
          amao_p->add(*a, res, pedge, this, client);
          // The intersection object is now the official result
          res = amao_p;
          }

          struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);
          // Union of the responses to all the queries
          boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
          for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
          SIGHT_VERB_DECL(scope, (txt()<<"query "<<qIt->str()), 3, tightComposerDebugLevel)
          Expr2AnyKey query = *qIt;
          // Transition this query to analysis state corresponding to *a
          tcqmTest.transToAnalState(query, *a);

          // Dispatch the query to the analysis
          boost::shared_ptr<AOType> ao_p = Expr2AnyOp(*a, query.sgn, query.pedge);

          SIGHT_VERB(dbg << (*a)->str() << ":" << ao_p->str() << endl, 3, tightComposerDebugLevel)

          //dbg << "cao_p="<<cao_p->str()<<endl;
          cao_p->add(ao_p, query.pedge, this, client);
          }

          // If this is the first analysis we've discovered that implements this query,
          // set cao_p to be the result. If we discover that other analyses implement this
          // query we'll later create an intersection object and add this object to it
          if(numRespondingAnalyses==0)
          res = cao_p;
          // If this is not the first analysis to implement this query, add cao_p to the
          // intersection, which we must have already created.
          else {
          assert(cao_p);
          SIGHT_VERB(dbg << "cao_p="<<cao_p->str()<<endl, 3, tightComposerDebugLevel)
          amao_p->add(*a, cao_p, pedge, this, client);
          }
          ++numRespondingAnalyses;
          gettimeofday(&gopeEnd, NULL); cout << "TightComposer::Expr2Any("<<(*a)->str()<<")\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
          }
          }*/

    {
      // Query the parent composer
      if(numRespondingAnalyses==0) {
        boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
        for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
          Expr2AnyKey query = *qIt;
          tcqmTest.transToAnalState(query, dynamic_cast<ComposedAnalysis*>(getComposer()));

          boost::shared_ptr<AOType> ao_p = parentComposerExpr2AnyOp(query.sgn, query.pedge);

#ifndef DISABLE_SIGHT
          SIGHT_VERB(dbg << dynamic_cast<ComposedAnalysis*>(getComposer())->str() << ":" << ao_p->str() << endl, 3, tightComposerDebugLevel)
#endif

            cao_p->add(ao_p, query.pedge, this, client);
        }
        res = cao_p;
        //amao_p->add(dynamic_cast<ComposedAnalysis*>(getComposer()), cao_p, pedge, this, client);
      }

    }

#ifndef DISABLE_SIGHT
    SIGHT_VERB(dbg << amao_p->str() << endl, 3, tightComposerDebugLevel)
#endif

      finalizeQueryListTest(queryList);
    gettimeofday(&gopeEnd, NULL); cout << "TightComposer::Expr2Any\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
    return res;
  }

  //! Generic method for forwarding GetStartAStates and GetEndAStates queries to analyses.
  //! Parts and PartEdges of different analyzes must form a hierarchy, where each Part/PartEdge
  //! of a coarser ATS is refined into one or more disjoint Parts/PartEdges of the finer ATS.
  //! In the context of tight composition this means that if multiple analyses that implement
  //! ATS functions are composed tightly, their ATSs must refine the ATS of the parent composer
  //! can cannot refine each others' ATSs. This means that:
  //! - When the tight composer calls the GetStartAStates and GetEndAStates functions of all the
  //!   analyses that implement the ATS, any GetStartAStates/GetEndAStates calls made by these
  //!   functions must be forwarded directly to the parent composer rather than each other.
  //! - The results of these calls can be represented as IntersectionParts and IntersectionPartEdges,
  //!   which assume that all the intersected Parts/PartEdges have the same parent Part/PartEdge.
  //!   These classes are then in charge of implementing Part::outEdges(), Part::inEdges(),
  //!   PartEdge::source(), and PartEdge::target().
  //! - If no analysis implements the ATS, all calls to GetStartAStates and GetEndAStates are forwarded
  //!   to the parent composer.
  //! \param n SgNode* expression on which the query is performed.
  //! \param pedgeList List of PartEdges on which the query needs to be forwarded.
  //! \param client Analysis* making the Expr2Any query.
  //! \param reqtype Query type which is either memloc, memregion, value or codeloc.
  //! \param implementsExpr2AnyOp boost function pointer used by this method to check if an
  //! analysis implements the interface to answer this query.
  //! \param Expr2AnyOp : boost function pointer to the GetStartAStates() or GetEndAStates() method
  //! implemented by a given analysis that is being tightly composed
  //! \param parentComposerExpr2AnyOp : boost function pointer to the GetStartAStates() or GetEndAStates() method
  //! implemented by the parent composer
  //! TightCompositionQueryManager maintains the state of each query.
  //! If the query is already cached then the cached object is returned.
  //! If the query is a recurring query then it is forwarded to the parent composer
  //! If the query is not in finished state or not a recurring query then it is forwarded to each analysis.
  std::set<PartPtr> TightComposer::GetAnyAStates(string opName,
                                                 list<Expr2AnyKey> queryList,
                                                 ComposedAnalysis* client,
                                                 boost::function<std::set<PartPtr> (ComposedAnalysis*)> GetAnyAStates,
                                                 boost::function<std::set<PartPtr> ()> parentGetAnyAStates)
  {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::GetAnyAStates", scope::medium), 3, tightComposerDebugLevel)
#endif

      list<Expr2AnyKey>::iterator qIt;

#ifndef DISABLE_SIGHT
    SIGHT_VERB_IF(3, tightComposerDebugLevel)
      dbg << "queryList=[\n";
    for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
      dbg << (*qIt).str() << endl;
    }
    dbg << "]\n";
    dbg << "queryMap=[" << tcqm.str() << "]\n";
    dbg << "recursiveQueries="<<recursiveQueries(queryList, client)<<endl;
    SIGHT_VERB_FI()
#endif

      if(recursiveQueries(queryList, client)) {
        return parentGetAnyAStates();
      }
    initializeQueryList(queryList);

    // Dispatch queries to each analysis and store its results into subParts
    map<ComposedAnalysis*, set<PartPtr> > subParts;
    for(list<ComposedAnalysis*>::iterator a = allAnalyses.begin(); a != allAnalyses.end(); ++a) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt()<<(*a)->str()), 3, tightComposerDebugLevel)
#endif
        if((*a)->implementsATSGraph()) {
          assert(queryList.size()==1);
          Expr2AnyKey query = *queryList.begin();
          //SIGHT_VERB_DECL(scope, (txt()<<"query "<<query.str()), 3, tightComposerDebugLevel)

          // Transition this query to analysis state corresponding to *a
          tcqm.transToAnalState(query, *a);

          // Dispatch the query to the analysis
          subParts[*a] = GetAnyAStates(*a);

          //SIGHT_VERB(dbg << (*a)->str() << ":" << ao_p->str() << endl, 3, tightComposerDebugLevel)
#ifndef DISABLE_SIGHT
          SIGHT_VERB_IF(3, tightComposerDebugLevel)
            for(set<PartPtr>::iterator p=subParts[*a].begin(); p!=subParts[*a].end(); ++p)
              dbg << (*p)->str()<<endl;
          SIGHT_VERB_FI()
#endif
            }
    }

    set<PartPtr> res;

    // If none of the tightly-composed analyses implements the ATS
    if(subParts.size()==0) {
      assert(queryList.size()==1);
      Expr2AnyKey query = *queryList.begin();
      tcqm.transToAnalState(query, dynamic_cast<ComposedAnalysis*>(getComposer()));

      // Get the set of Parts from the parent composer and add each one to res
      set<PartPtr> parentParts = parentGetAnyAStates();
      for(set<PartPtr>::iterator  p=parentParts.begin(); p!=parentParts.end(); ++p) {
        // Create an intersection and add it to res
        map<ComposedAnalysis*, PartPtr> parts;
        parts[this] = *p;
        PartPtr newPart = makePtr<IntersectionPart>(parts, *p, this);
        newPart->init();
        res.insert(newPart);
      }
      // If exactly one analysis implements it, return its results
    } else if(subParts.size()==1) {
      map<ComposedAnalysis*, PartPtr> parts;
      // Add all the parts returned by the analysis to res
      for(set<PartPtr>::iterator  p=subParts.begin()->second.begin(); p!=subParts.begin()->second.end(); ++p) {
        // Create an intersection and add it to res
        map<ComposedAnalysis*, PartPtr> parts;
        parts[this] = *p;
        PartPtr newPart = makePtr<IntersectionPart>(parts, (*p)->getInputPart(), this);
        newPart->init();
        res.insert(newPart);
      }

      // Otherwise, return the intersection of the result sets of all the analyses
      // that implement the ATS
    } else {
      res = createIntersectionPartSet(subParts, this);
      assert(res.size() == subParts.size());
    }

    finalizeQueryList(queryList);

    return res;
  }

  CodeLocObjectPtr TightComposer::Expr2CodeLoc_ex(list<Expr2AnyKey>& queryList, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call the generic Expr2Any method to get the list of CodeLocObjectPtr from clients
    boost::function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(boost::bind(&ComposedAnalysis::implementsExpr2CodeLoc, _1));
    boost::function<CodeLocObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(boost::bind(&ComposedAnalysis::Expr2CodeLoc, _1, _2, _3));

    assert(getComposer() != this);
    boost::function<CodeLocObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(boost::bind(&Composer::Expr2CodeLoc, getComposer(), _1, _2, this));

    CodeLocObjectPtr cl_p = Expr2Any<CodeLocObject, FullCodeLocObject,
                                     PartEdgeUnionCodeLocObject, AnalMapCodeLocObject>("Expr2CodeLoc",
                                                                                       queryList,
                                                                                       pedge,
                                                                                       client,
                                                                                       implementsExpr2AnyOp, Expr2AnyOp,
                                                                                       ComposerExpr2AnyOp);
    return cl_p;
  }

  CodeLocObjectPtr TightComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::Expr2CodeLoc(n="<<SgNode2Str(n)<<", pedge=" << pedge->str(), scope::medium), 2, tightComposerDebugLevel)
      pair<SgNode*, PartEdgePtr> cacheKey = make_pair(n, pedge);
    map<pair<SgNode*, PartEdgePtr>, CodeLocObjectPtr>::iterator cacheLoc = clCache.find(cacheKey);
    // If the result of this query has not yet been cached, compute it and cache it
    if(cacheLoc == clCache.end()) {
      // Create and run a single Expr2Any query for this pedge
      Expr2AnyKey key(n, pedge, Composer::codeloc);
      list<Expr2AnyKey> queryList;
      queryList.push_back(key);

      CodeLocObjectPtr rcl_p = Expr2CodeLoc_ex(queryList, pedge, client);
#ifndef DISABLE_SIGHT
      if(tightComposerDebugLevel >= 2) dbg << "CL=" << rcl_p->str() << endl;
#endif
      clCache[cacheKey] = rcl_p;
      return rcl_p;
      // Else, if the result of the query has already been computed, return the cached value
    } else
      return cacheLoc->second;
  }


  // Variant of Expr2CodeLoc that inquires about the code location denoted by the operand of the
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  CodeLocObjectPtr TightComposer::OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::OperandExpr2CodeLoc(n="<<SgNode2Str(n)<< ", op="<< SgNode2Str(operand) << ", pedge=" << pedge->str(), scope::medium), 2, tightComposerDebugLevel)
      list<PartEdgePtr> pedgeList = pedge->getOperandPartEdge(n, operand);
    if(pedgeList.size()==0) return NULLCodeLocObject;
    list<Expr2AnyKey> queryList;

    // Create one query for each PartEdge that corresponds to the use of operand at n
    list<PartEdgePtr>::iterator peIt;
    for(peIt=pedgeList.begin(); peIt != pedgeList.end(); ++peIt) {
      Expr2AnyKey key(operand, *peIt, Composer::codeloc);
      queryList.push_back(key);
    }

    // Run all the queries
    CodeLocObjectPtr rcl_p = Expr2CodeLoc_ex(queryList, pedge, client);

#ifndef DISABLE_SIGHT
    if(tightComposerDebugLevel >= 2) dbg << "CL=" << rcl_p->str() << endl;
#endif
    return rcl_p;
  }

  ValueObjectPtr TightComposer::Expr2Val_ex(list<Expr2AnyKey>& queryList, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call the generic Expr2Any method to get the list of ValueObjectPtr from clients
    boost::function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(boost::bind(&ComposedAnalysis::implementsExpr2Val, _1));
    boost::function<ValueObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(boost::bind(&ComposedAnalysis::Expr2Val, _1, _2, _3));

    assert(getComposer() != this);
    boost::function<ValueObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(boost::bind(&Composer::Expr2Val, getComposer(), _1, _2, this));

    ValueObjectPtr v_p = Expr2Any<ValueObject, FullValueObject,
                                  PartEdgeUnionValueObject, AnalMapValueObject>("Expr2Val",
                                                                                queryList,
                                                                                pedge,
                                                                                client,
                                                                                implementsExpr2AnyOp, Expr2AnyOp,
                                                                                ComposerExpr2AnyOp);
    return v_p;
  }

  // Abstract interpretation functions that return this analysis' abstractions that
  // represent the outcome of the given SgExpression.
  // The objects returned by these functions are expected to be deallocated by their callers.
  ValueObjectPtr TightComposer::Expr2Val(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::Expr2Val(n="<<SgNode2Str(n)<<", pedge=" << pedge->str(), scope::medium), 2, tightComposerDebugLevel);
#endif
    pair<SgNode*, PartEdgePtr> cacheKey = make_pair(n, pedge);
    map<pair<SgNode*, PartEdgePtr>, ValueObjectPtr>::iterator cacheLoc = vCache.find(cacheKey);
    // If the result of this query has not yet been cached, compute it and cache it
    if(cacheLoc == vCache.end()) {
      // Create and run a single Expr2Any query for this pedge
      Expr2AnyKey key(n, pedge, Composer::val);
      list<Expr2AnyKey> queryList;
      queryList.push_back(key);

      ValueObjectPtr rv_p = Expr2Val_ex(queryList, pedge, client);
#ifndef DISABLE_SIGHT
      if(tightComposerDebugLevel >= 2) dbg << "Val=" << rv_p->str() << endl;
#endif
      vCache[cacheKey] = rv_p;
      return rv_p;
      // Else, if the result of the query has already been computed, return the cached value
    } else
      return cacheLoc->second;
  }

  // Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  ValueObjectPtr TightComposer::OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::OperandExpr2Val(n="<<SgNode2Str(n)<< ", op="<< SgNode2Str(operand) << ", pedge=" << pedge->str()), 2, tightComposerDebugLevel)
#endif
      list<PartEdgePtr> pedgeList = pedge->getOperandPartEdge(n, operand);
    if(pedgeList.size()==0) return NULLValueObject;
    list<Expr2AnyKey> queryList;

    // Create one query for each PartEdge that corresponds to the use of operand at n
    list<PartEdgePtr>::iterator peIt;
    for(peIt=pedgeList.begin(); peIt != pedgeList.end(); ++peIt) {
      Expr2AnyKey key(operand, *peIt, Composer::val);
      queryList.push_back(key);
    }

    // Run all the queries
    ValueObjectPtr rv_p = Expr2Val_ex(queryList, pedge, client);
#ifndef DISABLE_SIGHT
    if(tightComposerDebugLevel >= 2) dbg << "Val=" << rv_p->str() << endl;
#endif
    return rv_p;
  }

  MemRegionObjectPtr TightComposer::Expr2MemRegion_ex(list<Expr2AnyKey>& queryList, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call the generic Expr2Any method to get the list of MemRegionObjectPtr from clients
    boost::function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(boost::bind(&ComposedAnalysis::implementsExpr2MemRegion, _1));
    boost::function<MemRegionObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(boost::bind(&ComposedAnalysis::Expr2MemRegion, _1, _2, _3));

    assert(getComposer() != this);
    boost::function<MemRegionObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(boost::bind(&Composer::Expr2MemRegion, getComposer(), _1, _2, this));
    struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);

    MemRegionObjectPtr mr_p = Expr2Any<MemRegionObject, FullMemRegionObject,
                                       PartEdgeUnionMemRegionObject, AnalMapMemRegionObject>("Expr2MemRegion",
                                                                                             queryList,
                                                                                             pedge,
                                                                                             client,
                                                                                             implementsExpr2AnyOp, Expr2AnyOp,
                                                                                             ComposerExpr2AnyOp);
    gettimeofday(&gopeEnd, NULL); cout << "TightComposer::Expr2MemRegion_ex()\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;
    return mr_p;
  }

  MemRegionObjectPtr TightComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::Expr2MemRegion(n="<<SgNode2Str(n)<<", pedge=" << pedge->str()), 2, tightComposerDebugLevel)
#endif


      /*boost::shared_ptr<PartEdgeUnionMemRegionObject> res = boost::make_shared<PartEdgeUnionMemRegionObject>();
        res->add(getComposer()->Expr2MemRegion(n, pedge, this), pedge, this, client);
        return res;*/
      //return getComposer()->Expr2MemRegion(n, pedge, this);

      pair<SgNode*, PartEdgePtr> cacheKey = make_pair(n, pedge);
    map<pair<SgNode*, PartEdgePtr>, MemRegionObjectPtr>::iterator cacheLoc = mrCache.find(cacheKey);
    // If the result of this query has not yet been cached, compute it and cache it
    if(cacheLoc == mrCache.end()) {
      // Create and run a single Expr2Any query for this pedge
      Expr2AnyKey key(n, pedge, Composer::memregion);
      list<Expr2AnyKey> queryList;
      queryList.push_back(key);

      /*      boost::shared_ptr<PartEdgeUnionMemRegionObject> cao_p = boost::make_shared<PartEdgeUnionMemRegionObject>();
              for(list<Expr2AnyKey>::iterator qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
              Expr2AnyKey query = *qIt;
              //tcqm.transToAnalState(query, dynamic_cast<ComposedAnalysis*>(getComposer()));

              MemRegionObjectPtr ao_p = getComposer()->Expr2MemRegion(n, pedge, this);

              SIGHT_VERB(dbg << dynamic_cast<ComposedAnalysis*>(getComposer())->str() << ":" << ao_p->str() << endl, 3, tightComposerDebugLevel)

              cao_p->add(ao_p, query.pedge, this, client);
              }
              return cao_p;*/

      MemRegionObjectPtr rmr_p = Expr2MemRegion_ex(queryList, pedge, client);
#ifndef DISABLE_SIGHT
      if(tightComposerDebugLevel >= 2) dbg << "MR=" << rmr_p->str() << endl;
#endif
      mrCache[cacheKey] = rmr_p;
      return rmr_p;
      // Else, if the result of the query has already been computed, return the cached value
    } else
      return cacheLoc->second;
  }


  // Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemRegionObjectPtr TightComposer::OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::OperandExpr2MemRegion(n="<<SgNode2Str(n)<< ", op="<< SgNode2Str(operand) << ", pedge=" << pedge->str(), scope::medium), 2, tightComposerDebugLevel)
#endif
      list<PartEdgePtr> pedgeList = pedge->getOperandPartEdge(n, operand);
    if(pedgeList.size()==0) return NULLMemRegionObject;
    list<Expr2AnyKey> queryList;

    // Create one query for each PartEdge that corresponds to the use of operand at n
    list<PartEdgePtr>::iterator peIt;
    for(peIt=pedgeList.begin(); peIt != pedgeList.end(); ++peIt) {
      Expr2AnyKey key(operand, *peIt, Composer::memregion);
      queryList.push_back(key);
    }

    // Run all the queries
    MemRegionObjectPtr rmr_p = Expr2MemRegion_ex(queryList, pedge, client);
#ifndef DISABLE_SIGHT
    if(tightComposerDebugLevel >= 2) dbg << "MR=" << rmr_p->str() << endl;
#endif
    return rmr_p;
  }

  MemLocObjectPtr TightComposer::Expr2MemLoc_ex(list<Expr2AnyKey>& queryList, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call the generic Expr2Any method to get the list of MemLocObjectPtr from clients
    boost::function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(boost::bind(&ComposedAnalysis::implementsExpr2MemLoc, _1));
    boost::function<MemLocObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(boost::bind(&ComposedAnalysis::Expr2MemLoc, _1, _2, _3));

    assert(getComposer() != this);
    boost::function<MemLocObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(boost::bind(&Composer::Expr2MemLoc, getComposer(), _1, _2, this));

    MemLocObjectPtr ml_p = Expr2Any<MemLocObject, FullMemLocObject,
                                    PartEdgeUnionMemLocObject, AnalMapMemLocObject>("Expr2MemLoc",
                                                                                    queryList,
                                                                                    pedge,
                                                                                    client,
                                                                                    implementsExpr2AnyOp, Expr2AnyOp,
                                                                                    ComposerExpr2AnyOp);
    return ml_p;
  }

  //! Any client of the TightComposer invokes this interface function
  //! TightComposer queries all the client analyses implementing Expr2MemLoc
  //! Returns IntersectMemLocObjectPtr
  MemLocObjectPtr TightComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::Expr2MemLoc(n="<<SgNode2Str(n)<<", pedge=" << pedge->str(), scope::medium), 2, tightComposerDebugLevel)
#endif
      pair<SgNode*, PartEdgePtr> cacheKey = make_pair(n, pedge);
    map<pair<SgNode*, PartEdgePtr>, MemLocObjectPtr>::iterator cacheLoc = mlCache.find(cacheKey);
    // If the result of this query has not yet been cached, compute it and cache it
    if(cacheLoc == mlCache.end()) {
      // Create and run a single Expr2Any query for this pedge
      Expr2AnyKey key(n, pedge, Composer::memloc);
      list<Expr2AnyKey> queryList;
      queryList.push_back(key);

      MemLocObjectPtr rml_p = Expr2MemLoc_ex(queryList, pedge, client);
#ifndef DISABLE_SIGHT
      if(tightComposerDebugLevel >= 2) dbg << "ML=" << rml_p->str() << endl;
#endif
      mlCache[cacheKey] = rml_p;
      return rml_p;
      // Else, if the result of the query has already been computed, return the cached value
    } else
      return cacheLoc->second;
  }

  // Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemLocObjectPtr TightComposer::OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::OperandExpr2MemLoc(n="<<SgNode2Str(n)<< ", op="<< SgNode2Str(operand) << ", pedge=" << pedge->str(), scope::medium), 2, tightComposerDebugLevel)
#endif

      //    struct timeval gopeStart, gopeEnd; gettimeofday(&gopeStart, NULL);
      list<PartEdgePtr> pedgeList = pedge->getOperandPartEdge(n, operand);
    if(pedgeList.size()==0) return NULLMemLocObject;
    //    gettimeofday(&gopeEnd, NULL); cout << "        TightComposer::OperandExpr2MemLoc() getOperandPartEdge\t"<<(((gopeEnd.tv_sec*1000000 + gopeEnd.tv_usec) - (gopeStart.tv_sec*1000000 + gopeStart.tv_usec)) / 1000000.0)<<endl;

    //    struct timeval queryStart, queryEnd; gettimeofday(&queryStart, NULL);
    list<Expr2AnyKey> queryList;

    // Create one query for each PartEdge that corresponds to the use of operand at n
    list<PartEdgePtr>::iterator peIt;
    //{ scope s(txt()<<"pedgeList(#"<<pedgeList.size()<<")");
    for(peIt=pedgeList.begin(); peIt != pedgeList.end(); ++peIt) {
      //dbg << (*peIt)->str()<<endl;
      Expr2AnyKey key(operand, *peIt, Composer::memloc);
      queryList.push_back(key);
    } //}

    // Run all the queries
    MemLocObjectPtr rml_p = Expr2MemLoc_ex(queryList, pedge, client);
#ifndef DISABLE_SIGHT
    if(tightComposerDebugLevel >= 2) dbg << "ML=" << rml_p->str() << endl;
#endif

    //    gettimeofday(&queryEnd, NULL); cout << "        TightComposer::OperandExpr2MemLoc() query\t"<<(((queryEnd.tv_sec*1000000 + queryEnd.tv_usec) - (queryStart.tv_sec*1000000 + queryStart.tv_usec)) / 1000000.0)<<endl;

    return rml_p;
  }

  // MayEquals
  // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
  bool TightComposer::mayEqualV (ValueObjectPtr val1, ValueObjectPtr val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val1->mayEqualAO(val2, pedge);
  }

  bool TightComposer::mayEqualMR(MemRegionObjectPtr mr1, MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->mayEqualAO(mr2, pedge);
  }

  // MustEquals
  bool TightComposer::mustEqualV (ValueObjectPtr val1, ValueObjectPtr val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val1->mustEqualAO(val2, pedge);
  }

  bool TightComposer::mustEqualMR(MemRegionObjectPtr mr1, MemRegionObjectPtr mr2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->mustEqualAO(mr2, pedge);
  }

  // Returns whether the two abstract objects denote the same set of concrete objects
  bool TightComposer::equalSetV (ValueObjectPtr val1, ValueObjectPtr val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val1->equalSetAO(val2, pedge);
  }

  bool TightComposer::equalSetMR(MemRegionObjectPtr mr1, MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->equalSetAO(mr2, pedge);
  }

  bool TightComposer::subSetV(ValueObjectPtr val1, ValueObjectPtr val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val1->subSetAO(val2, pedge);
  }

  bool TightComposer::subSetMR(MemRegionObjectPtr mr1, MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->subSetAO(mr2, pedge);
  }

  bool TightComposer::isLiveV (ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val->isLiveAO(pedge);
  }

  bool TightComposer::isLiveMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr->isLiveAO(pedge);
  }

  // Calls the isLive() method of the given AbstractObject that denotes an operand of the given SgNode n within
  // the context of its own PartEdges and returns true if it may be live within any of them
  bool TightComposer::OperandIsLiveV (SgNode* n, SgNode* operand, ValueObjectPtr val,    PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  bool TightComposer::OperandIsLiveMR(SgNode* n, SgNode* operand, MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client) {
    assert(0);
    return false;
  }

  // Computes the meet of from and to and saves the result in to.
  // Returns true if this causes this to change and false otherwise.
  bool TightComposer::meetUpdateV (ValueObjectPtr to, ValueObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return to->meetUpdateAO(from, pedge);
  }

  bool TightComposer::meetUpdateMR(MemRegionObjectPtr to, MemRegionObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return to->meetUpdateAO(from, pedge);
  }

  // Returns whether the given AbstractObject corresponds to the set of all sub-executions or the empty set
  bool TightComposer::isFullV (ValueObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return ao->isFullAO(pedge);
  }

  bool TightComposer::isFullMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return ao->isFullAO(pedge);
  }

  // Returns whether the given AbstractObject corresponds to the empty set
  bool TightComposer::isEmptyV (ValueObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return ao->isEmptyAO(pedge);
  }

  bool TightComposer::isEmptyMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return ao->isEmptyAO(pedge);
  }

  // Returns a ValueObject that denotes the size of this memory region
  ValueObjectPtr TightComposer::getRegionSizeMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return ao->getRegionSizeAO(pedge);
  }

  // query all analyses in the composition list with GetStartAStates
  // construct IntersectionPart and return the set of IntersectionParts
  // this would mean that the analyses are composed tightly and they are also
  // modifying the ATS simultaneously
  std::set<PartPtr> TightComposer::GetStartAStates(ComposedAnalysis* client) {
    // 4/8/2014: for simplicity we are not allowing analyses to be composed tightly
    // and modify the ATS at the same time
    // TightComposer is composed as a sub-analysis of LooseSequential (ChainComposer)
    // direct the query GetStartAStates to the parent composer
    //return GetStartAStates_Spec();
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::GetStartAStates("<<client->str()<<")", scope::medium), 2, tightComposerDebugLevel)
#endif
      Expr2AnyKey key(NULL, NULLPartEdge, Composer::atsGraph);
    list<Expr2AnyKey> queryList;
    queryList.push_back(key);

    // Call the generic GetAnyAStates method to get the set of starting Abstract States
    boost::function<std::set<PartPtr> (ComposedAnalysis*)> GetAnyAStatesOp(boost::bind(&ComposedAnalysis::GetStartAStates_Spec, _1));
    assert(getComposer() != this);
    boost::function<std::set<PartPtr> ()> ComposerGetAnyAStatesOp(boost::bind(&Composer::GetStartAStates, getComposer(), this));

    std::set<PartPtr> startAStates = GetAnyAStates("GetStartAStates",
                                                   queryList,
                                                   client,
                                                   GetAnyAStatesOp,
                                                   ComposerGetAnyAStatesOp);
    return startAStates;
  }

  // implementation is similar in principle to GetStartAStates
  std::set<PartPtr> TightComposer::GetEndAStates(ComposedAnalysis* client) {
    //return GetEndAStates_Spec();
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::GetEndAStates("<<client->str()<<")", scope::medium), 2, tightComposerDebugLevel)
#endif
      Expr2AnyKey key(NULL, NULLPartEdge, Composer::atsGraph);
    list<Expr2AnyKey> queryList;
    queryList.push_back(key);

    // Call the generic GetAnyAStates method to get the set of starting Abstract States
    boost::function<std::set<PartPtr> (ComposedAnalysis*)> GetAnyAStatesOp(boost::bind(&ComposedAnalysis::GetEndAStates_Spec, _1));
    assert(getComposer() != this);
    boost::function<std::set<PartPtr> ()> ComposerGetAnyAStatesOp(boost::bind(&Composer::GetEndAStates, getComposer(), this));

    std::set<PartPtr> endAStates = GetAnyAStates("GetEndAStates",
                                                 queryList,
                                                 client,
                                                 GetAnyAStatesOp,
                                                 ComposerGetAnyAStatesOp);
    return endAStates;
  }

  // Given a Part implemented by the entire composer, returns the set of refined Parts implemented
  // by the composer or the NULLPart if this relationship was not tracked.
  const std::set<PartPtr>& TightComposer::getRefinedParts(PartPtr base) const
  { assert(0); }

  // Given a PartEdge implemented by the entire composer, returns the set of refined PartEdges implemented
  // by the composer or the NULLPartEdge if this relationship was not tracked.
  const std::set<PartEdgePtr>& TightComposer::getRefinedPartEdges(PartEdgePtr base) const
  { assert(0); }

  // Returns the number of Parts that refine the given base
  unsigned int  TightComposer::numRefinedParts    (PartPtr     base) const {
    return getRefinedParts(base).size();
  }

  // Returns the number of PartEdges that refine the given base
  unsigned int  TightComposer::numRefinedPartEdges(PartEdgePtr base) const {
    return getRefinedPartEdges(base).size();
  }

  // -----------------------------------------
  // ----- Methods from ComposedAnalysis -----
  // -----------------------------------------

  // go through the list of all analyses and call their initNodeState
  void TightComposer::initNodeState(AnalysisParts& parts) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt()<<"TightComposer::initNodeState(part="<<parts.NodeState()->str()<<")"), 2, tightComposerDebugLevel)
#endif
      list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt() << "TightComposer::initNodeState("<<(*a)->str()<<")", scope::medium), 2, tightComposerDebugLevel)
#endif
        // Initialize the NodeState for the current analysis, passing it the Part from the parent composer,
        // which has a fully constructed ATS, rathern than the Part currently being constructed by this
        // TightComposer. If the current analysis implements the ATS this will enable it to build its Lattices
        // in terms of the ATS that they're refining.
        (*a)->initNodeState(/*part, part->getSupersetPart()*/ parts);
    }
  }

  void TightComposer::propagateDF2DescDense(AnalysisParts& parts,
                                            // Set of all the Parts that have already been visited by the analysis
                                            std::set<PartPtr>& visited,
                                            // Set of all the Parts that have been initialized
                                            std::set<PartPtr>& initialized,
                                            // The dataflow iterator that identifies the state of the iteration
                                            dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
                                            ,
                                            // anchor that denotes the current abstract state in the debug output
                                            anchor curPartAnchor,
                                            // graph widget that visualizes the flow of the worklist algorithm
                                            sight::structure::graph& worklistGraph,
                                            // Maps each Abstract State to the anchors of outgoing links that target it from the last visit to its predecessors
                                            std::map<PartPtr, std::set<anchor> >& toAnchors,
                                            // Maps each Abstract state to the anchors of the AStates that lead to it, as well as the AStates themselves
                                            std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
                                            ) {

#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt() << "TightComposer::propagateDF2DescDense", scope::medium), 1, tightComposerDebugLevel)
#endif
      list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt() << "Analysis "<<(*a)->str(), scope::high), 1, tightComposerDebugLevel)
#endif
        ComposedAnalysis::propagateDF2DescDense(*a, /*part, part->getSupersetPart(),*/ parts,
                                                visited, initialized, curNodeIt
#ifndef DISABLE_SIGHT
                                                ,
                                                curPartAnchor, worklistGraph, toAnchors, fromAnchors
#endif
                                                );
    }
  }

  // call the generic version of this function on each analysis
  void TightComposer::transferAStateDense(AnalysisParts& parts,
                                          std::set<PartPtr>& visited, bool firstVisit,
                                          std::set<PartPtr>& initialized, dataflowPartEdgeIterator* curNodeIt,
                                          map<PartEdgePtr, vector<Lattice*> >& dfInfoPost,
                                          //set<PartPtr>& ultimateParts, set<PartPtr>& ultimateSupersetParts,
                                          AnalysisPartSets& ultimateParts
#ifndef DISABLE_SIGHT
                                          ,
                                          anchor curPartAnchor,
                                          sight::structure::graph& worklistGraph,std::map<PartPtr, std::set<anchor> >& toAnchors,
                                          std::map<PartPtr, std::set<std::pair<anchor, PartPtr> > >& fromAnchors
#endif
                                          ) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt() << "TightComposer::transferAStateDense", scope::medium), 1, tightComposerDebugLevel)
#endif
      list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt() << "Analysis "<<(*a)->str(), scope::high), 1, tightComposerDebugLevel)
#endif
        ComposedAnalysis::transferAStateDense(*a, parts, visited, firstVisit, initialized,
                                              curNodeIt, dfInfoPost,
                                              ultimateParts /*ultimateSupersetParts*/
#ifndef DISABLE_SIGHT
                                              , 
                                              curPartAnchor,
                                              worklistGraph, toAnchors, fromAnchors
#endif
                                              );
    }
  }

  void TightComposer::transferPropagateAStateSSA(PartPtr part, set<PartPtr>& visited, bool firstVisit, set<PartPtr>& initialized,
                                                 dataflowPartEdgeIterator* curNodeIt
#ifndef DISABLE_SIGHT
                                                 , anchor curPartAnchor, sight::structure::graph& worklistGraph,
                                                 map<PartPtr, set<anchor> >& toAnchors,
                                                 map<PartPtr, set<pair<anchor, PartPtr> > >& fromAnchors
#endif
                                                 ) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt() << "TightComposer::transferPropagateAStateSSA", scope::medium), 1, tightComposerDebugLevel)
#endif
      list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt() << "Analysis "<<(*a)->str(), scope::high), 1, tightComposerDebugLevel)
#endif
        ComposedAnalysis::transferPropagateAStateSSA(*a, part, visited, firstVisit, initialized, curNodeIt
#ifndef DISABLE_SIGHT
                                                     ,
                                                     curPartAnchor,
                                                     worklistGraph, toAnchors, fromAnchors
#endif
                                                     );
    }
  }

  // Invokes the analysis-specific method to set the ATS location PartEdges of all the newly-computed
  // Lattices at part
  void TightComposer::setDescendantLatticeLocationsDense(AnalysisParts& parts) {
#ifndef DISABLE_SIGHT
    SIGHT_VERB_DECL(scope, (txt() << "TightComposer::setDescendantLatticeLocationsDense", scope::medium), 1, tightComposerDebugLevel)
#endif
      list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
#ifndef DISABLE_SIGHT
      SIGHT_VERB_DECL(scope, (txt() << "Analysis "<<(*a)->str(), scope::high), 1, tightComposerDebugLevel)
#endif
        ComposedAnalysis::setDescendantLatticeLocationsDense(*a, parts);
    }
  }

  bool TightComposer::transfer(AnalysisParts& parts, CFGNode cn, NodeState& state,
                               std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo) {
    assert(0);
    return false;
  }

  set<PartPtr> TightComposer::getInitialWorklist() {
    if(getDirection() == fw) return GetStartAStates(this);
    else if (getDirection() == bw) return GetEndAStates(this);
    else assert(0);
  }

  map<PartEdgePtr, vector<Lattice*> >& TightComposer::getLatticeAnte(NodeState *state) {
    assert(0);
    return state->getLatticeAboveAllMod(this);
  }

  map<PartEdgePtr, vector<Lattice*> >& TightComposer::getLatticePost(NodeState *state) {
    map<PartEdgePtr, vector<Lattice*> > belowLat;
    assert(0);
    return state->getLatticeBelowAllMod(this);
  }

  std::map<PartEdgePtr, std::vector<Lattice*> >& TightComposer::getLatticeAnte(NodeState *state, ComposedAnalysis* analysis) {
    if(getDirection() == fw)       return state->getLatticeAboveAllMod(analysis);
    else if (getDirection() == bw) return state->getLatticeBelowAllMod(analysis);
    else assert(0);
  }
  std::map<PartEdgePtr, std::vector<Lattice*> >& TightComposer::getLatticePost(NodeState *state, ComposedAnalysis* analysis) {
    if(getDirection() == fw)       return state->getLatticeBelowAllMod(analysis);
    else if (getDirection() == bw) return state->getLatticeAboveAllMod(analysis);
    else assert(0);
  }

  void TightComposer::setLatticeAnte(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite) {
    assert(0);
  }

  void TightComposer::setLatticePost(NodeState *state, std::map<PartEdgePtr, std::vector<Lattice*> >& dfInfo, bool overwrite) {
    assert(0);
  }

  list<PartPtr> TightComposer::getDescendants(PartPtr p) {
    list<PartPtr> descendants;
    list<PartEdgePtr> outEdges = getEdgesToDescendants(p);

    for(list<PartEdgePtr>::iterator ei=outEdges.begin(); ei!=outEdges.end(); ei++)
      descendants.push_back((*ei)->target());
    return descendants;
  }

  list<PartEdgePtr> TightComposer::getEdgesToDescendants(PartPtr part) {
    direction dir = getDirection();
    if(dir == fw)
      return part->outEdges();
    else if(dir == bw)
      return part->inEdges();
    else assert(0);
  }

  // Returns the set of Parts that denote the end of the ATS.
  // getUltimateParts() returns the Parts from the ATS over which the analysis is being run.
  // If the analysis is being composed loosely, this ATS is already complete when the analysis starts.
  // If it is a tight composition, the ATS is created as the analysis runs.
  AnalysisPartSets TightComposer::getUltimateParts() {
    direction dir = getDirection();
    if(dir == fw)
      return ComposedAnalysis::NodeState2AllMapper::NodeState2All(GetEndAStates(this));
    //return AnalysisPartSets(/*nodeState*/ getComposer()->GetEndAStates(this),
    //                        /*index*/     AnalysisPartSets::getInputs(getComposer()->GetEndAStates(this)),
    //                        /*input*/     AnalysisPartSets::getInputs(getComposer()->GetEndAStates(this)));

    else if(dir == bw)
      return ComposedAnalysis::NodeState2AllMapper::NodeState2All(GetStartAStates(this));
    //return AnalysisPartSets(/*nodeState*/ getComposer()->GetStartAStates(this),
    //                        /*index*/     AnalysisPartSets::getInputs(getComposer()->GetStartAStates(this)),
    //                        /*input*/     AnalysisPartSets::getInputs(getComposer()->GetStartAStates(this)));
    else assert(0);
  }

  /* // getUltimateSupersetParts() returns the Parts from the completed ATS that the current analysis
  // may be refining. getUltimateParts() == getUltimateSupersetParts() for loose composition but not
  // for tight composition.
  std::set<PartPtr> TightComposer::getUltimateSupersetParts() {
  direction dir = getDirection();
  if(dir == fw)
  return getComposer()->GetEndAStates(this);
  else if(dir == bw)
  return getComposer()->GetStartAStates(this);
  else assert(0);
  }*/

  // return the iterator based on our direction
  dataflowPartEdgeIterator* TightComposer::getIterator() {
    direction dir = getDirection();
    if(dir == fw)
      return new fw_dataflowPartEdgeIterator(/*incrementalGraph*/ true, selectIterOrderFromEnvironment());
    //return new fw_dataflowPartEdgeIterator(succ_back);
    else if(dir == bw)
      return new bw_dataflowPartEdgeIterator(/*incrementalGraph*/ true, selectIterOrderFromEnvironment());
    else assert(0);
  }

  // Given a Part from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisParts object that contains all the Parts relevant for analysis.
  AnalysisParts     TightComposer::NodeState2All(PartPtr part, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
    return AnalysisParts(NodeStates_valid? part:                 NULLPart, NodeStates_valid,
                         indexes_valid?    part->getInputPart(): NULLPart, indexes_valid,
                         inputs_valid?     part->getInputPart(): NULLPart, inputs_valid);
  }

  // Given a PartEdge from the ATS on which NodeStates are maintained for the analysis(es) managed by this
  // composer, returns an AnalysisPartEdges object that contains all the PartEdges relevant for analysis.
  AnalysisPartEdges TightComposer::NodeState2All(PartEdgePtr pedge, bool NodeStates_valid, bool indexes_valid, bool inputs_valid) {
    return AnalysisPartEdges(NodeStates_valid? pedge:                     NULLPartEdge, NodeStates_valid,
                             indexes_valid?    pedge->getInputPartEdge(): NULLPartEdge, indexes_valid,
                             inputs_valid?     pedge->getInputPartEdge(): NULLPartEdge, inputs_valid);
  }

  // Given a Part from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisParts object that contains the input and index the Parts relevant for analysis.
  AnalysisParts TightComposer::Input2Index(PartPtr part, bool indexes_valid, bool inputs_valid) {
    return AnalysisParts(NULLPart,                         false,
                         indexes_valid?    part: NULLPart, indexes_valid,
                         inputs_valid?     part: NULLPart, inputs_valid);
  }

  // Given a PartEdge from the ATS that the analyses managed by this composed take as input,
  // returns an AnalysisPartEdges object that contains the input and index the Parts relevant for analysis.
  AnalysisPartEdges TightComposer::Input2Index(PartEdgePtr pedge, bool indexes_valid, bool inputs_valid) {
    return AnalysisPartEdges(NULLPartEdge,                          false,
                             indexes_valid?    pedge: NULLPartEdge, indexes_valid,
                             inputs_valid?     pedge: NULLPartEdge, inputs_valid);
  }

  // Remaps the given Lattice across the scope transition (if any) of the given edge, updating the lat vector
  // with pointers to the updated Lattice objects and deleting old Lattice objects as needed.
  void TightComposer::remapML(PartEdgePtr fromPEdge, std::vector<Lattice*>& lat) {
    assert(0);
  }

  ComposedAnalysis::direction TightComposer::getDirection() {
    return dir;
  }

  // The Expr2* and GetFunction*Part functions are implemented by calling the same functions in each of the
  // constituent analyses and returning an Intersection object that includes their responses

  // Abstract interpretation functions that return this analysis' abstractions that
  // represent the outcome of the given SgExpression. The default implementations of
  // these throw NotImplementedException so that if a derived class does not implement
  // any of these functions, the Composer is informed.
  //
  // The objects returned by these functions are expected to be deallocated by their callers.
  ValueObjectPtr TightComposer::Expr2Val(SgNode* n, PartEdgePtr pedge) {
    return Expr2Val(n, pedge, this);
  }

  CodeLocObjectPtr TightComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge) {
    return Expr2CodeLoc(n, pedge, this);
  }

  MemRegionObjectPtr TightComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge) {
    return Expr2MemRegion(n, pedge, this);
  }

  MemLocObjectPtr TightComposer::Expr2MemLoc(SgNode* n, PartEdgePtr pedge) {
    return Expr2MemLoc(n, pedge, this);
  }

  // Return true if the class implements Expr2* and false otherwise
  bool TightComposer::implementsExpr2Val() {
    return true;
  }

  bool TightComposer::implementsExpr2CodeLoc() {
    return true;
  }

  bool TightComposer::implementsExpr2MemRegion() {
    return true;
  }

  bool TightComposer::implementsExpr2MemLoc() {
    return true;
  }

  bool TightComposer::implementsATSGraph() {
    return true;
  }

  // Returns whether the class implements Expr* loosely or tightly (if it does at all)
  ComposedAnalysis::implTightness TightComposer::Expr2ValTightness() {
    return ComposedAnalysis::loose;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2CodeLocTightness() {
    return ComposedAnalysis::loose;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2MemRegionTightness() {
    return ComposedAnalysis::loose;
  }

  ComposedAnalysis::implTightness TightComposer::Expr2MemLocTightness() {
    return ComposedAnalysis::loose;
  }

  // Return the anchor Parts of a given function
  // root the query to the parent composer
  // TightComposer is always run on some ATS implemented by the parent composer
  std::set<PartPtr> TightComposer::GetStartAStates_Spec() {
    //return getComposer()->GetStartAStates(this);
    return GetStartAStates(this);
  }

  std::set<PartPtr> TightComposer::GetEndAStates_Spec() {
    //return getComposer()->GetEndAStates(this);
    return GetEndAStates(this);
  }

  // Return an ATSGraph object that describes the overall structure of the transition system
  SSAGraph* TightComposer::GetSSAGraph(ComposedAnalysis* client)
  { ROSE_ASSERT(0); }

  std::string TightComposer::str(std::string indent) const {
    return txt() << "TightComposer";
  }
};
