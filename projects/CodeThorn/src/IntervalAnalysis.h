#ifndef INTERVALANALYZER_H
#define INTERVALANALYZER_H

// Author: Markus Schordan, 2013.

#include <iostream>
#include "VariableIdMapping.h"
#include "Labeler.h"
#include "CFAnalysis.h"
#include "DFAnalysisBase.h"
#include "RDLattice.h"
#include "WorkList.h"
#include "AnalysisAbstractionLayer.h"
#include "DFAstAttribute.h"
#include "IntervalPropertyState.h"

/*! 
  * \author Markus Schordan
  * \date 2013.
 */

namespace SPRAY {
class IntervalAnalysis : public DFAnalysisBase {
 public:
  IntervalAnalysis();
  ~IntervalAnalysis();
  void initializeExtremalValue(Lattice* element);
  void initializeTransferFunctions();
 protected:
  virtual DFAstAttribute* createDFAstAttribute(Lattice* elem);
  NumberIntervalLattice* _numberIntervalLattice;
};
}

#endif
