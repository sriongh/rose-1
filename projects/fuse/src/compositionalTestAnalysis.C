#include "sage3basic.h"
#include "compose.h"
#include "fuseCommandParser.h"
#include "const_prop_analysis.h"
#include "const_prop_count.h"
#include "dead_path_elim_analysis.h"
#include "tight_composer.h"
#include "call_context_sensitivity_analysis.h"
#include "pointsToAnalysis.h"
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <exception>
#include "sight.h"
#include "AnalysisAstAttribute.h"
#include "saveDotAnalysis.h"

using namespace fuse;

#ifndef DISABLE_SIGHT
using namespace sight;
#endif

using namespace boost::xpressive;

void FuseInit(int argc, char** argv) {
#ifndef DISABLE_SIGHT
  // Command to set up the enviroment variable to find the binary fuseLayout
  // fuseLayout is required to run fuse
  // fuseLayout binary is at the same level as fuse in the build tree
  // When compiling fuse ROSE_PREFIX is defined as -DROSE_PREFIX="\"${top_builddir}\"" which
  // is top of the build tree
  // If fuse fails to find fuseLayout set up this environment variable appropriately.

  setenv("SIGHT_LAYOUT_EXEC", (txt()<<ROSE_PREFIX<<"/projects/fuse/src/fuseLayout").c_str(), 1);

  std::string title = (txt() << "Process " << " Debug Output").c_str();
  std::string workdir = (txt() << "dbg").c_str();
  SightInit(argc, argv, title, workdir);
#endif
}

int main(int argc, char* argv[])
{
  FuseInit(argc, argv);
  cout << "========== S T A R T ==========\n";

  // Run the front end
  SgProject* project = frontend(argc, argv);

  printf("Frontend done\n");fflush(stdout);

  std::list<ComposedAnalysis*> scanalyses;
  std::list<ComposedAnalysis*> tcanalyses;

  // Check if the analysis sequence is described as pragmas
  sregex cmd_rxp = icase("fuse") >> *_s >> (s1=+_);
  boost::xpressive::smatch what;
  std::string fuse_cmd;

#if 1
  Rose_STL_Container<SgNode*> pragmas = NodeQuery::querySubTree(project, V_SgPragma);
  for(Rose_STL_Container<SgNode*>::iterator p=pragmas.begin(); p!=pragmas.end(); p++) {
    SgPragma* pragma = isSgPragma(*p);
    assert(pragma);
    // currently processing only one fuse command
    std::string pstr = pragma->get_pragma();
    if(regex_match(pstr, what, cmd_rxp)) {
      assert(what.size() == 2);      
      fuse_cmd.append(what[1]);
      break;
    }
  }

  std::cout << "Applying Fuse command: " << fuse_cmd << std::endl;
  if(fuse_cmd.length() == 0) {
    std::cerr << "No Fuse Command Found!" << std::endl;
  }
#endif
  FuseCommandParser parser;
  FuseCommand* cmd = parser(fuse_cmd);
  cmd->initFuseCommand();
  cmd->execute();
  std::list<ComposedAnalysis*> sanalyses = cmd->getSubAnalysisList();
  std::list<ComposedAnalysis*>::const_iterator s = sanalyses.begin();
  ComposedAnalysis* last = sanalyses.back();
  for(int count=0; s != sanalyses.end(); ++s, count++) {
    if(ConstPropCountAnalysis* cpc = dynamic_cast<ConstPropCountAnalysis*>(*s)) {
      cpc->generateDotFile(count);
    }
  }
  
  
  // FuseAnnotTraversal fuseannotations(sanalyses);
  // fuseannotations.traverseInputFiles(project, preorder);
  // fuseannotations.printConstantCountMapStats();
  // fuseannotations.printAssignOpConstantFirstAnalysisCount();

  std::cout << "==========  E  N  D  ==========\n";
  return 0;
}
