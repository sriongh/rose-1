#pragma once

#include "compose.h"
#include "composed_analysis.h"
#include "ssa.h"
#include <set>

namespace fuse {

class DynamicMonitor : public UndirDataflow {
  public:
  DynamicMonitor();

  // Find all the uses at the location of the startingDef and the defs that reach them.
  // Return a mapping of these uses to their corresponding reaching defs
  std::map<SSAMemLocObjectPtr, std::set<SSAMemLocObjectPtr> > getReachingUse2Defs(const std::set<SSAMemLocObjectPtr>& startingDef);

  // Collects into the set reachingDefs the defs that transitively reach the uses in the given set and are not
  // already in defs.
  //void collectReachingDefs(const std::set<SSAMemLocObjectPtr>& uses, std::set<SSAMemLocObjectPtr>& reachingDefs);
  std::set<SSAMemLocObjectPtr> collectReachingDefs(SSAMemLocObjectPtr startingDef);

  // Returns a shared pointer to a freshly-allocated copy of this ComposedAnalysis object
  ComposedAnalysisPtr copy() { return boost::make_shared<DynamicMonitor>(); }

  void runAnalysis();

  std::string str(std::string indent="") const { return "DynamicMonitor"; }
}; // class dynamicMonitor

}; // namespace fuse
