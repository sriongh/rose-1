#include "sage3basic.h"
#include "compose.h"
#include "const_prop_analysis.h"
#include "live_dead_analysis.h"
#include "ortho_array_analysis.h"
#include "call_context_sensitivity_analysis.h"
#include "dead_path_elim_analysis.h"
#include "printAnalysisStates.h"
#include "pointsToAnalysis.h"
#include "mpi_value_analysis.h"
#include "mpi_comm_context_analysis.h"
#include "analysis_tester.h"
#include "sight.h"
#include "mpi.h"

using namespace std;
using namespace fuse;

int main(int argc, char* argv[])
{
  FuseInit(argc, argv);
  MPI_Init(&argc, &argv);
  cout << "========== S T A R T ==========\n";

  // Run the front end
  SgProject* project = frontend(argc, argv);
  // //generatePDF(*project);
  printf("Frontend done\n");fflush(stdout);

  std::list<ComposedAnalysis*> analyses;
  // analyses.push_back(new CallContextSensitivityAnalysis(1, CallContextSensitivityAnalysis::callSite));
  analyses.push_back(new MPIValueAnalysis());
  analyses.push_back(new ConstantPropagationAnalysis());
  // analyses.push_back(new DeadPathElimAnalysis());
  analyses.push_back(new MPICommContextAnalysis());
  checkDataflowInfoPass* cdip = new checkDataflowInfoPass();
  ChainComposer cc(analyses, cdip, true);
  cc.runAnalysis();

  if(cdip->getNumErrors() > 0) cout << cdip->getNumErrors() << " Errors Reported!"<<endl;
  else                         cout << "PASS"<<endl;

  MPI_Finalize();

  cout << "==========  E  N  D  ==========\n";
  return 0;
}
