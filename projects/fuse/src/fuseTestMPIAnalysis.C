#include "sage3basic.h"
#include "compose.h"
#include "mpi_value_analysis.h"
#include "const_prop_analysis.h"
#include "dead_path_elim_analysis.h"
#include "tight_composer.h"
#include "mpi_comm_context_analysis.h"
#include "mpi_comm_analysis.h"
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

  printf("Frontend done\n");fflush(stdout);

  std::list<ComposedAnalysis*> scanalyses;
  std::list<ComposedAnalysis*> tcanalyses;
  
  // Sequential composer    
  scanalyses.push_back(new MPICommContextAnalysis());
  scanalyses.push_back(new MPIValueAnalysis());
  // scanalyses.push_back(new ConstantPropagationAnalysis());
  // scanalyses.push_back(new DeadPathElimAnalysis());

  // Tight composition of analyses
  ConstantPropagationAnalysis* cp = new ConstantPropagationAnalysis();
  tcanalyses.push_back(cp);
  tcanalyses.push_back(new MPICommAnalysis(cp));
  TightComposer* tightcomposer = new TightComposer(tcanalyses);

  // Add the tight composer to sequential composer
  scanalyses.push_back(tightcomposer);
  checkDataflowInfoPass* cdip = new checkDataflowInfoPass();
  ChainComposer cc(scanalyses, cdip, true);

  cc.runAnalysis();


  if(cdip->getNumErrors() > 0) cout << cdip->getNumErrors() << " Errors Reported!"<<endl;
  else                         cout << "PASS"<<endl;

  MPI_Finalize();

  cout << "==========  E  N  D  ==========\n";
  return 0;
}
