/*****************************************
 * author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "sage3basic.h"
#include "tight_composer.h"
#include <boost/enable_shared_from_this.hpp>
#include "sight.h"
#include "stx_analysis.h"
#include <algorithm>

using namespace std;
using namespace boost;
using namespace sight;

namespace fuse {

  DEBUG_LEVEL(tightComposerDebugLevel, 0);

  /***************
   * Expr2AnyKey *
   ***************/

  bool Expr2AnyKey::operator<(const Expr2AnyKey& that) const {
    scope reg(txt()<<"Expr2AnyKey::<",
              scope::medium, attrGE("tightComposerDebugLevel", 4));

    if(tightComposerDebugLevel() >= 4) {
      dbg << "this=" << str() << endl;
      dbg << "that=" << that.str() << endl;
    }

    // Order by the sgn expression first
    if(sgn < that.sgn) {
      // if(tightComposerDebugLevel() >= 4) dbg << "this < that=" << "true";
      return true;
    }
    // Order by PartEdge if the expression is same
    else if(sgn == that.sgn && pedge < that.pedge) {
      // if(tightComposerDebugLevel() >= 4) dbg << "this < that=" << "true";
      return true;
    }
    // Order by reqtype if both expression and PartEdge are same
    else if(sgn == that.sgn && pedge == that.pedge && reqtype < that.reqtype) {
      // if(tightComposerDebugLevel() >= 4) dbg << "this < that=" << "true";     
      return true;
    }
    else {
      // if(tightComposerDebugLevel() >= 4) dbg << "this < that=" << "false";
      return false;
    }
  }

  std::string Expr2AnyKey::str(std::string indent) const {
    std::ostringstream oss;
    oss << "[sgn=" << sgn << "," << SgNode2Str(sgn) <<",";
    oss << "pedge=" << pedge->str() <<",";        
    oss << " reqtype="; 
    switch(reqtype) {
    case 0: oss << "any"; break;
    case 1: oss << "codeloc"; break;
    case 2: oss << "val"; break;
    case 3: oss << "memloc"; break;
    case 4: oss << "memregion"; break;
    case 5: oss << "atsGraph"; break;
    default: assert(0);
    }
    oss << "]";
    return oss.str();
  }


  /********************************
   * TightCompositionQueryManager *
   ********************************/

  void TightCompositionQueryManager::initializeQuery(Expr2AnyKey key) {
    scope reg(txt()<<"initializeQuery",
              scope::medium, attrGE("tightComposerDebugLevel", 4)); 
    
    if(tightComposerDebugLevel() >= 4) {
      dbg << "key=" << key.str() << endl;
      dbg << this->str();
    }
    // There is no prior entry in the map
    assert(queryStateMap.find(key) == queryStateMap.end());
    pair<Expr2AnyKey, Expr2AnyState> elem = make_pair(key, Expr2AnyState());
    queryStateMap.insert(elem);
    if(tightComposerDebugLevel() >= 4) dbg << this->str();
  }

  const Expr2AnyState TightCompositionQueryManager::getQueryState(Expr2AnyKey key) const {
    assert(queryStateMap.find(key) != queryStateMap.end());
    return queryStateMap.find(key)->second;
  }

  bool TightCompositionQueryManager::isQueryCached(Expr2AnyKey key) {
    if(queryStateMap.find(key) == queryStateMap.end())
      return false;

    Expr2AnyState& qstate = queryStateMap.find(key)->second;
    if(qstate.state == Expr2AnyState::finished) return true;   

    return false;
  }

  bool TightCompositionQueryManager::isLoopingQuery(Expr2AnyKey key, ComposedAnalysis* analysis_) {
    scope reg(txt()<<"isLoopingQuery",
          scope::medium, attrGE("tightComposerDebugLevel", 4));

    if(tightComposerDebugLevel() >= 4) {
      dbg << "key=" << key.str() << endl;
      dbg << this->str();
    }

    if(queryStateMap.find(key) == queryStateMap.end()) return false;

    Expr2AnyState& qstate = queryStateMap.find(key)->second;    

    // if(tightComposerDebugLevel() >= 4) {
    //   dbg << "key=" << key.str() << " found\n";
    //   dbg << "state=" << qstate.str() << "\n";
    // }

    // query is not in the analysis state
    if(qstate.state != Expr2AnyState::anal) return false;

    // if(tightComposerDebugLevel() >= 4) {
    //   dbg << "LastAnalysis=" << qstate.getLastAnalysisQueried() << endl;
    //   dbg << "client=" << analysis_ << endl;
    // }

    // query is in the analysis state
    if(qstate.getLastAnalysisQueried() == analysis_) return true;

    return false;
  }

  void TightCompositionQueryManager::transToAnalState(Expr2AnyKey key, ComposedAnalysis* analysis_) {
    scope reg(txt()<<"transToAnalState",
              scope::medium, attrGE("tightComposerDebugLevel", 4)); 

    if(tightComposerDebugLevel() >= 4) {
      dbg << "key=" << key.str() << endl;
      dbg << this->str();
    }

    // key is already in the map
    assert(queryStateMap.find(key) != queryStateMap.end());
    Expr2AnyState& qstate = queryStateMap.find(key)->second;
    qstate.setAnalysis(analysis_);
  }

  void TightCompositionQueryManager::transToFinishedState(Expr2AnyKey key) {
    // key is already in the map
    assert(queryStateMap.find(key) != queryStateMap.end());
    Expr2AnyState& qstate = queryStateMap.find(key)->second;
    qstate.setFinished();
    queryStateMap.erase(key);
  }

  string TightCompositionQueryManager::str(string indent) const {
    ostringstream oss;
    oss << "QueryStateMap=[";
    map<Expr2AnyKey, Expr2AnyState>::const_iterator it;
    for(it = queryStateMap.begin(); it != queryStateMap.end(); ) {
      oss << "query=" << it->first.str() << ",state=" << it->second.str();
      ++it;
      if(it != queryStateMap.end()) oss << "\n";
    }
    oss << "]\n";
    return oss.str();
  }

  /******************************
   * TightComposer Methods *
   ******************************/

  TightComposer::TightComposer(const std::list<ComposedAnalysis*>& analyses) : allAnalyses(analyses) {    
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
      
  TightComposer::TightComposer(const TightComposer& that) : allAnalyses(that.allAnalyses), dir(that.dir) {
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
                                                    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp,
                                                    function<shared_ptr<AOType> (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp,
                                                    function<shared_ptr<AOType> (SgNode*, PartEdgePtr)> parentComposerExpr2AnyOp) {
    scope reg(txt()<<"TightComposer::Expr2Any",
              scope::medium, attrGE("tightComposerDebugLevel", 3));

    list<Expr2AnyKey>::iterator qIt;

    if(tightComposerDebugLevel() >= 3) {
      dbg << "queryList=[\n";
      for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
        dbg << (*qIt).str() << endl;
      }
      dbg << "]\n";
      // dbg << "queryMap=[" << tcqm.str() << "]\n";
    }

    if(recursiveQueries(queryList, client)) {
      // return boost::make_shared<FullAOType>();
      boost::shared_ptr<AnalysisMapAOType> amao_p = boost::make_shared<AnalysisMapAOType>();
      // Query the parent composer
      boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
      for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
        Expr2AnyKey query = *qIt;     
        boost::shared_ptr<AOType> ao_p = parentComposerExpr2AnyOp(query.sgn, query.pedge);
        if(tightComposerDebugLevel() >= 3) {
          dbg << dynamic_cast<ComposedAnalysis*>(getComposer())->str() << ":" << ao_p->str() << endl;
        }
        cao_p->add(ao_p, query.pedge);
      }
      amao_p->add(dynamic_cast<ComposedAnalysis*>(getComposer()), cao_p, pedge);
      return amao_p;
    }

    initializeQueryList(queryList);

    boost::shared_ptr<AnalysisMapAOType> amao_p = boost::make_shared<AnalysisMapAOType>();
    list<ComposedAnalysis*>::iterator a = allAnalyses.begin();    
    
    // Dispatch queries to each analysis
    for( ; a != allAnalyses.end(); ++a) {
      if(implementsExpr2AnyOp(*a)) {
        boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
        for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
          Expr2AnyKey query = *qIt;
          // Transition this query to analysis state corresponding to *a
          tcqm.transToAnalState(query, *a);

          // dispatch the query to the analysis
          boost::shared_ptr<AOType> ao_p = Expr2AnyOp(*a, query.sgn, query.pedge);
          
          if(tightComposerDebugLevel() >= 3) {
            dbg << (*a)->str() << ":" << ao_p->str() << endl;
          }

          cao_p->add(ao_p, query.pedge);
        }        
        amao_p->add(*a, cao_p, pedge);
      }
    }

    // Query the parent composer
    boost::shared_ptr<CombinedAOType> cao_p = boost::make_shared<CombinedAOType>();
    for(qIt = queryList.begin(); qIt != queryList.end(); ++qIt) {
      Expr2AnyKey query = *qIt;
      tcqm.transToAnalState(query, dynamic_cast<ComposedAnalysis*>(getComposer()));

      boost::shared_ptr<AOType> ao_p = parentComposerExpr2AnyOp(query.sgn, query.pedge);

      if(tightComposerDebugLevel() >= 3) {
        dbg << dynamic_cast<ComposedAnalysis*>(getComposer())->str() << ":" << ao_p->str() << endl;
      }

      cao_p->add(ao_p, query.pedge);
    }

    amao_p->add(dynamic_cast<ComposedAnalysis*>(getComposer()), cao_p, pedge);

    if(tightComposerDebugLevel() >= 3) {
      dbg << amao_p->str() << endl;
    }

    finalizeQueryList(queryList);
    
    return amao_p;
  }
  

  CodeLocObjectPtr TightComposer::Expr2CodeLoc_ex(list<Expr2AnyKey>& queryList, PartEdgePtr pedge, ComposedAnalysis* client) {

    // Call the generic Expr2Any method to get the list of CodeLocObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2CodeLoc, _1));
    function<CodeLocObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2CodeLoc, _1, _2, _3));

    assert(getComposer() != this);
    function<CodeLocObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(bind(&Composer::Expr2CodeLoc, getComposer(), _1, _2, this));

    CodeLocObjectPtr cl_p = Expr2Any<CodeLocObject, FullCodeLocObject, 
                                     PartEdgeUnionCodeLocObject, IntersectAnalMapCodeLocObject>("Expr2CodeLoc",
                                                                                                queryList,
                                                                                                pedge,
                                                                                                client,
                                                                                                implementsExpr2AnyOp, Expr2AnyOp,
                                                                                                ComposerExpr2AnyOp);
    return cl_p;
  }

  CodeLocObjectPtr TightComposer::Expr2CodeLoc(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt()<<"TightComposer::Expr2CodeLoc(n="<<SgNode2Str(n)<<", pedge=" << pedge->str(),
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    Expr2AnyKey key(n, pedge, Composer::codeloc);
    list<Expr2AnyKey> queryList;

    queryList.push_back(key);
    CodeLocObjectPtr rcl_p = Expr2CodeLoc_ex(queryList, pedge, client);
    if(tightComposerDebugLevel() >= 2) dbg << "CL=" << rcl_p->str() << endl;
    return rcl_p;
  }
 
  
  // Variant of Expr2CodeLoc that inquires about the code location denoted by the operand of the 
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  CodeLocObjectPtr TightComposer::OperandExpr2CodeLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt()<<"TightComposer::OperandExpr2CodeLoc(n="<<SgNode2Str(n)<< ", op="<< SgNode2Str(operand) << ", pedge=" << pedge->str(),
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    list<PartEdgePtr> pedgeList = pedge->getOperandPartEdge(n, operand);
    list<Expr2AnyKey> queryList;

    list<PartEdgePtr>::iterator peIt;
    for(peIt=pedgeList.begin(); peIt != pedgeList.end(); ++peIt) {
      Expr2AnyKey key(operand, *peIt, Composer::codeloc);
      queryList.push_back(key);
    }
    CodeLocObjectPtr rcl_p = Expr2CodeLoc_ex(queryList, pedge, client);
    if(tightComposerDebugLevel() >= 2) dbg << "CL=" << rcl_p->str() << endl;
    return rcl_p;
  }

  ValueObjectPtr TightComposer::Expr2Val_ex(list<Expr2AnyKey>& queryList, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call the generic Expr2Any method to get the list of ValueObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2Val, _1));
    function<ValueObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2Val, _1, _2, _3));

    assert(getComposer() != this);
    function<ValueObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(bind(&Composer::Expr2Val, getComposer(), _1, _2, this));

    ValueObjectPtr v_p = Expr2Any<ValueObject, FullValueObject, 
                                  PartEdgeUnionValueObject, IntersectAnalMapValueObject>("Expr2Val",
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
    scope reg(txt()<<"TightComposer::Expr2Val(n="<<SgNode2Str(n)<<", pedge=" << pedge->str(),
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    Expr2AnyKey key(n, pedge, Composer::val);
    list<Expr2AnyKey> queryList;

    queryList.push_back(key);
    ValueObjectPtr rv_p = Expr2Val_ex(queryList, pedge, client);
    if(tightComposerDebugLevel() >= 2) dbg << "Val=" << rv_p->str() << endl;
    return rv_p;
  }
  
  // Variant of Expr2Val that inquires about the value of the memory location denoted by the operand of the 
  // given node n, where the part denotes the set of prefixes that terminate at SgNode n.
  ValueObjectPtr TightComposer::OperandExpr2Val(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt()<<"TightComposer::OperandExpr2Val(n="<<SgNode2Str(n)<< ", op="<< SgNode2Str(operand) << ", pedge=" << pedge->str(),
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    list<PartEdgePtr> pedgeList = pedge->getOperandPartEdge(n, operand);
    list<Expr2AnyKey> queryList;

    list<PartEdgePtr>::iterator peIt;
    for(peIt=pedgeList.begin(); peIt != pedgeList.end(); ++peIt) {
      Expr2AnyKey key(operand, *peIt, Composer::val);
      queryList.push_back(key);
    }
    ValueObjectPtr rv_p = Expr2Val_ex(queryList, pedge, client);
    if(tightComposerDebugLevel() >= 2) dbg << "Val=" << rv_p->str() << endl;
    return rv_p;
  }

  MemRegionObjectPtr TightComposer::Expr2MemRegion_ex(list<Expr2AnyKey>& queryList, PartEdgePtr pedge, ComposedAnalysis* client) {

    // Call the generic Expr2Any method to get the list of MemRegionObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2MemRegion, _1));
    function<MemRegionObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2MemRegion, _1, _2, _3));

    assert(getComposer() != this);
    function<MemRegionObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(bind(&Composer::Expr2MemRegion, getComposer(), _1, _2, this));

    MemRegionObjectPtr mr_p = Expr2Any<MemRegionObject, FullMemRegionObject, 
                                       PartEdgeUnionMemRegionObject, IntersectAnalMapMemRegionObject>("Expr2MemRegion",
                                                                                                      queryList,
                                                                                                      pedge,
                                                                                                      client,
                                                                                                      implementsExpr2AnyOp, Expr2AnyOp,
                                                                                                      ComposerExpr2AnyOp);
    return mr_p;
  }
    
  MemRegionObjectPtr TightComposer::Expr2MemRegion(SgNode* n, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt()<<"TightComposer::Expr2MemRegion(n="<<SgNode2Str(n)<<", pedge=" << pedge->str(),
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    Expr2AnyKey key(n, pedge, Composer::memregion);
    list<Expr2AnyKey> queryList;

    queryList.push_back(key);
    MemRegionObjectPtr rmr_p = Expr2MemRegion_ex(queryList, pedge, client);
    if(tightComposerDebugLevel() >= 2) dbg << "MR=" << rmr_p->str() << endl;
    return rmr_p;
  }
  
  
  // Variant of Expr2MemRegion that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemRegionObjectPtr TightComposer::OperandExpr2MemRegion(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt()<<"TightComposer::OperandExpr2MemRegion(n="<<SgNode2Str(n)<< ", op="<< SgNode2Str(operand) << ", pedge=" << pedge->str(),
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    list<PartEdgePtr> pedgeList = pedge->getOperandPartEdge(n, operand);
    list<Expr2AnyKey> queryList;

    list<PartEdgePtr>::iterator peIt;
    for(peIt=pedgeList.begin(); peIt != pedgeList.end(); ++peIt) {
      Expr2AnyKey key(operand, *peIt, Composer::memregion);
      queryList.push_back(key);
    }
    MemRegionObjectPtr rmr_p = Expr2MemRegion_ex(queryList, pedge, client);
    if(tightComposerDebugLevel() >= 2) dbg << "MR=" << rmr_p->str() << endl;
    return rmr_p;
  }

  MemLocObjectPtr TightComposer::Expr2MemLoc_ex(list<Expr2AnyKey>& queryList, PartEdgePtr pedge, ComposedAnalysis* client) {
    // Call the generic Expr2Any method to get the list of MemLocObjectPtr from clients
    function<bool (ComposedAnalysis*)> implementsExpr2AnyOp(bind(&ComposedAnalysis::implementsExpr2MemLoc, _1));
    function<MemLocObjectPtr (ComposedAnalysis*, SgNode*, PartEdgePtr)> Expr2AnyOp(bind(&ComposedAnalysis::Expr2MemLoc, _1, _2, _3));

    assert(getComposer() != this);
    function<MemLocObjectPtr (SgNode*, PartEdgePtr)> ComposerExpr2AnyOp(bind(&Composer::Expr2MemLoc, getComposer(), _1, _2, this));

    MemLocObjectPtr ml_p = Expr2Any<MemLocObject, FullMemLocObject, 
                                     PartEdgeUnionMemLocObject, IntersectAnalMapMemLocObject>("Expr2MemLoc",
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
    scope reg(txt()<<"TightComposer::Expr2MemLoc(n="<<SgNode2Str(n)<<", pedge=" << pedge->str(),
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    Expr2AnyKey key(n, pedge, Composer::memloc);
    list<Expr2AnyKey> queryList;

    queryList.push_back(key);

    MemLocObjectPtr rml_p = Expr2MemLoc_ex(queryList, pedge, client);
    if(tightComposerDebugLevel() >=2) dbg << "ML=" << rml_p->str() << endl;
    return rml_p;
  }
  
  // Variant of Expr2MemLoc that inquires about the memory location denoted by the operand of the given node n, where
  // the part denotes the set of prefixes that terminate at SgNode n.
  MemLocObjectPtr TightComposer::OperandExpr2MemLoc(SgNode* n, SgNode* operand, PartEdgePtr pedge, ComposedAnalysis* client) {
    scope reg(txt()<<"TightComposer::OperandExpr2MemLoc(n="<<SgNode2Str(n)<< ", op="<< SgNode2Str(operand) << ", pedge=" << pedge->str(),
              scope::medium, attrGE("tightComposerDebugLevel", 2));
    list<PartEdgePtr> pedgeList = pedge->getOperandPartEdge(n, operand);
    list<Expr2AnyKey> queryList;

    list<PartEdgePtr>::iterator peIt;
    for(peIt=pedgeList.begin(); peIt != pedgeList.end(); ++peIt) {
      Expr2AnyKey key(operand, *peIt, Composer::memloc);
      queryList.push_back(key);
    }

    MemLocObjectPtr rml_p = Expr2MemLoc_ex(queryList, pedge, client);
    if(tightComposerDebugLevel() >=2) dbg << "ML=" << rml_p->str() << endl;
    return rml_p;
  }
  
  // MayEquals
  // Returns whether the given pair of AbstractObjects are may-equal at the given PartEdge
  bool TightComposer::mayEqualV (ValueObjectPtr val1, ValueObjectPtr val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val1->mayEqualV(val2, pedge);
  }

  bool TightComposer::mayEqualMR(MemRegionObjectPtr mr1, MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->mayEqualMR(mr2, pedge);
  }

  // MustEquals
  bool TightComposer::mustEqualV (ValueObjectPtr val1, ValueObjectPtr val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val1->mustEqualV(val2, pedge);
  }

  bool TightComposer::mustEqualMR(MemRegionObjectPtr mr1, MemRegionObjectPtr mr2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->mustEqualMR(mr2, pedge);
  }
  
  // Returns whether the two abstract objects denote the same set of concrete objects
  bool TightComposer::equalSetV (ValueObjectPtr val1, ValueObjectPtr val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val1->equalSetV(val2, pedge);
  }

  bool TightComposer::equalSetMR(MemRegionObjectPtr mr1, MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->equalSetMR(mr2, pedge);
  }
  
  bool TightComposer::subSetV(ValueObjectPtr val1, ValueObjectPtr val2, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val1->subSetV(val2, pedge);
  }

  bool TightComposer::subSetMR(MemRegionObjectPtr mr1, MemRegionObjectPtr mr2,  PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr1->subSetMR(mr2, pedge);
  }
    
  bool TightComposer::isLiveV (ValueObjectPtr val, PartEdgePtr pedge, ComposedAnalysis* client) {
    return val->isLiveV(pedge);
  }

  bool TightComposer::isLiveMR(MemRegionObjectPtr mr, PartEdgePtr pedge, ComposedAnalysis* client) {
    return mr->isLiveMR(pedge);
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
    to->meetUpdateV(from, pedge);
  }

  bool TightComposer::meetUpdateMR(MemRegionObjectPtr to, MemRegionObjectPtr from, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    to->meetUpdateMR(from, pedge);
  }
  
  // Returns whether the given AbstractObject corresponds to the set of all sub-executions or the empty set
  bool TightComposer::isFullV (ValueObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    ao->isFullV(pedge);
  }

  bool TightComposer::isFullMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    ao->isFullMR(pedge);
  }
  
  // Returns whether the given AbstractObject corresponds to the empty set
  bool TightComposer::isEmptyV (ValueObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return ao->isEmptyV(pedge);
  }

  bool TightComposer::isEmptyMR(MemRegionObjectPtr ao, PartEdgePtr pedge, ComposedAnalysis* analysis) {
    return ao->isEmptyMR(pedge);
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
    return GetStartAStates_Spec();
  }

  // implementation is similar in principle to GetStartAStates
  std::set<PartPtr> TightComposer::GetEndAStates(ComposedAnalysis* client) {
    return GetEndAStates_Spec();
  }

  // -----------------------------------------
  // ----- Methods from ComposedAnalysis -----
  // -----------------------------------------

  // go through the list of all analyses and call their initNodeState
  void TightComposer::initNodeState(PartPtr part) {
    list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
      (*a)->initNodeState(part);
    }
  }

  // call the generic version of this function on each analysis
  void TightComposer::transferPropagateAState(PartPtr part, set<PartPtr>& visited, bool firstVisit, set<PartPtr>& initialized,
                                              dataflowPartEdgeIterator* curNodeIt, anchor curPartAnchor, graph& worklistGraph,
                                              map<PartPtr, set<anchor> >& toAnchors,
                                              map<PartPtr, set<pair<anchor, PartPtr> > >& fromAnchors) {
    list<ComposedAnalysis*>::iterator a = allAnalyses.begin();
    for( ; a != allAnalyses.end(); ++a) {
      scope reg(txt() << "TightComposer::transferPropagateAState",
                scope::medium, attrGE("composedAnalysisDebugLevel", 2));
      dbg << "ComposedAnalysis=" << (*a)->str() << endl;
      ComposedAnalysis::transferPropagateAState(*a, part, visited, firstVisit, initialized, curNodeIt, curPartAnchor, 
                                                worklistGraph, toAnchors, fromAnchors);
    }
  }

  bool TightComposer::transfer(PartPtr part, CFGNode cn, NodeState& state, 
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

  // call self GetStartAStates/GetEndAStates
  set<PartPtr> TightComposer::getUltimate() {
    direction dir = getDirection();
    if(dir == fw)
      return GetEndAStates(this);
    else if(dir == bw)
      return GetStartAStates(this);
    else assert(0);
  }

  // return the iterator based on our direction
  dataflowPartEdgeIterator* TightComposer::getIterator() {
    direction dir = getDirection();
    if(dir == fw)
      return new fw_dataflowPartEdgeIterator();
    else if(dir == bw)
      return new bw_dataflowPartEdgeIterator();
    else assert(0);
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
    return false;
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
    return getComposer()->GetStartAStates(this);
  }

  std::set<PartPtr> TightComposer::GetEndAStates_Spec() {
    return getComposer()->GetEndAStates(this);
  }

  std::string TightComposer::str(std::string indent) const {
    return txt() << "TightComposer";
  }
};
