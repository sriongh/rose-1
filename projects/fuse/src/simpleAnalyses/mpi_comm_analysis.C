/****************************************
 * Author: Sriram Aananthakrishnan, 2014 *
 *****************************************/

#include "mpi_comm_analysis.h"

using namespace std;
using namespace sight;

namespace fuse {
  /******************
   * MPICommATSPart *
   ******************/
  MPICommATSPart::MPICommATSPart(PartPtr base, MPICommAnalysis* analysis) 
    : Part(base, analysis),
      this->parent(base), 
      this->mpicommanalysis(analysis) {
  }

  list<PartPtr> MPICommATSPart::outEdges() {
  }

  list<PartPtr> MPICommATSPart::inEdges() {
  }

  set<CFGNode> MPICommATSPart::CFGNodes() const {
  }

  set<PartPtr> MPICommATSPart::matchingCallParts() const {
  }

  PartEdgePtr MPICommATSPart::outEdgeToAny() {
  }

  PartEdgePtr MPICommATSPart::inEdgeFromAny() {
  }

  bool MPICommATSPart::equal(const PartPtr& that) const {
  }

  bool MPICommATSPart::less(const PartPtr& that) const {
  }

  /**********************
   * MPICommATSPartEdge *
   **********************/
  MPICommATSPartEdge::MPICommATSPartEdge(MPICommAnalysis* analysis, PartEdgePtr base)
    : PartEdge(analysis, base),
      this->parent(base),
    this->mpicommanalysis(analysis) {
  }
      
}; // end namespace
