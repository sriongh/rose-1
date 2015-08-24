#include "sage3basic.h"
#include "compose.h"
#include "tight_composer.h"
#include "const_prop_analysis.h"
#include "live_dead_analysis.h"
#include "ortho_array_analysis.h"
#include "call_context_sensitivity_analysis.h"
#include "dead_path_elim_analysis.h"
#include "printAnalysisStates.h"
#include "pointsToAnalysis.h"
#include "analysis_tester.h"
#include <vector>

using namespace std;
using namespace fuse;

int main(int argc, char** argv)
{
  FuseInit(argc, argv);
  
  printf("========== S T A R T ==========\n");
    
  // Run the front end
  SgProject* project = frontend(argc, argv);
  //generatePDF(*project);
  printf("Frontend done\n");fflush(stdout);

  list<ComposedAnalysis*> tc_analyses;
  list<ComposedAnalysis*> cc_analyses;

  ConstantPropagationAnalysis* cp = new ConstantPropagationAnalysis();
  PointsToAnalysis* pt = new PointsToAnalysis();
  tc_analyses.push_back(cp);
  tc_analyses.push_back(pt);
  TightComposer* tc = new TightComposer(tc_analyses);

  checkDataflowInfoPass* cdip = new checkDataflowInfoPass();
  cc_analyses.push_back(tc);
  ChainComposer* cc = new ChainComposer(cc_analyses, cdip, true);
  cc->runAnalysis();

  if(cdip->getNumErrors() > 0) cout << cdip->getNumErrors() << " Errors Reported!"<<endl;
  else                         cout << "PASS"<<endl;
  printf("==========  E  N  D  ==========\n");
  
  return 0;
}

