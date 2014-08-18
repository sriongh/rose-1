#pragma once
#include <rose.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <ostream>
#include <fstream>
#include <sstream>
#include <boost/foreach.hpp>
#include <filteredCFG.h>
#include "staticSingleAssignment.h"
#include <boost/unordered_map.hpp>
#include "reachingDef.h"

#include <vector>

namespace hssa_private
{
  using namespace boost;
  using namespace std;
  // using namespace AbstractMemoryObject;
  using namespace ssa_private;

  class HeapReachingDef;
  // class AliasSet;

  // typedef boost::shared_ptr<SSAMemLoc> SSAMemLocPtr;
  typedef boost::shared_ptr<HeapReachingDef> HeapReachingDefPtr;
  typedef map<SgNode*, HeapReachingDefPtr> NodeHeapReachingDefTable;
  typedef map<SgNode*, ReachingDef::ReachingDefPtr> NodeReachingDefTable; 
  // typedef map<SgNode*, SSAMemLocPtr> NodeAMOTable;
  // typedef boost::shared_ptr<HeapSSALattice> HeapLatticePtr;
  typedef boost::unordered_map<SgNode*, std::set<SgNode* > > LocalDefUseTable;
  typedef std::map<SgNode* , HeapReachingDefPtr> NodeHeapReachingDefTable;
  // typedef boost::shared_ptr<AliasSet> AliasSetPtr;
  // typedef std::map<SgNode* , AliasSetPtr> NodeAliasSetTable;

class HeapReachingDef : public ReachingDef
{
 protected:
  ReachingDef::ReachingDefPtr currDef_;
  ReachingDef::ReachingDefPtr prevDef_;

 public:
   HeapReachingDef(SgNode* defNode, Type type) : ReachingDef(defNode, type) {};
  ~HeapReachingDef() {}

  bool isDPhiFunction() const;
  void addJoinedDPhi(ReachingDef::ReachingDefPtr currDef, ReachingDef::ReachingDefPtr prevDef) {
    currDef_ = currDef;
    prevDef_ = prevDef;
  };

  ReachingDef::ReachingDefPtr getCurrDef() { return currDef_; };
  ReachingDef::ReachingDefPtr getPrevDef() { return prevDef_; }; 
};

  /*
  // The object that manages the given memory object's alias's 
class AliasSet 
{
 protected:
  std::set<SSAMemLocPtr> defAliasSet;
  std::set<SSAMemLocPtr> mayAliasSet;
  
 public:
  void addDefAlias(SSAMemLocPtr defAlias);
  void addMayAlias(SSAMemLocPtr mayAlias);
  bool defAlias(SSAMemLocPtr another);
  bool mayAlias(SSAMemLocPtr another);
  void update(AliasSetPtr anotherSet);
  AliasSet* copy();
};*/

}
