#include "sage3basic.h"

using namespace std;

#include "const_prop_analysis.h"
#include "dead_path_elim_analysis.h"
#include "tight_composer.h"
#include "call_context_sensitivity_analysis.h"
#include "pointsToAnalysis.h"
#include <boost/xpressive/xpressive.hpp>
#include "fuseCommandParser.h"
#include "const_prop_count.h"

using namespace boost::xpressive;

namespace fuse {

  FuseCommand::FuseCommand() : ctype(unknown), root(0) { }
  
  void FuseCommand::setCType(CompositionType type) {
    ctype = type;
  }

  FuseCommand::CompositionType FuseCommand::getCType() const {
    return ctype;
  }
  
  void FuseCommand::push_back(ComposedAnalysis* analysis) {
    cout << "Adding analysis=" << analysis->str() << endl;
    subanalyses.push_back(analysis);
  }

  list<ComposedAnalysis*> FuseCommand::getSubAnalysisList() const {
    return subanalyses;
  }

  void FuseCommand::initFuseCommand() {
    assert(ctype != unknown);
    // All composers also inherit from ComposedAnalysis
    assert(subanalyses.size() > 0);
    if(getCType() == FuseCommand::seq) {      
      checkDataflowInfoPass* cdip = new checkDataflowInfoPass();
      root = new ChainComposer(subanalyses, cdip, true);
    }
    else if(getCType() == FuseCommand::tight) {
      root = new TightComposer(subanalyses);
    }    
  }

  ComposedAnalysis* FuseCommand::getFuseCommand() {
    initFuseCommand();
    return root;   
  }

  Composer* FuseCommand::getRootComposer() {
    assert((Composer*) root);
    return ((Composer*) root);
  }

  void FuseCommand::execute() {
    assert(root);
    ((ChainComposer*) root)->runAnalysis();
  }

  string FuseCommand::str() const {
    ostringstream oss;
    oss << "[type=";
    if(ctype==seq) oss << "seq";
    else if(ctype==tight) oss << "tight";
    else oss << "unknown";

    oss << ", analyses={";

    list<ComposedAnalysis*>::const_iterator i = subanalyses.begin();
    for( ; i != subanalyses.end(); ) {
      oss << (*i)->str();
      ++i;
      if(i != subanalyses.end()) oss << ", ";
    }
    oss << "}]";
    return oss.str();
  }


  /*********************
   * FuseCommandParser *
   *********************/
  FuseCommandParser::FuseCommandParser() {
    constprop = icase("cp");
    pointsto = icase("pt");
    callcontext = icase("ccs");
    deadpath = icase("dp");
    constcount = icase("co");
    seqcomp = icase("seq");
    tightcomp = icase("tight");
    analysis = by_ref(constprop)
      | by_ref(constcount)
      | by_ref(pointsto) 
      | by_ref(deadpath)
      | by_ref(callcontext);
    
    composer = by_ref(seqcomp) | by_ref(tightcomp);
    atailseq = *_s >> as_xpr(',') >> *_s >> analysis >> *_s;
    analysisList = '(' >> *_s >> analysis >> *by_ref(atailseq) >> ')';
    scommand = composer >> analysisList;
    ctailseq = *_s >> as_xpr(',') >> *_s >> by_ref(scommand) >> *_s;
    tailseq = atailseq | ctailseq;
    headseq = analysis | scommand;
    command = composer >> '(' >> headseq >> *tailseq >> ')';
  }
  
  // get the string and match it against all analyses
  void FuseCommandParser::matchAnalysisToken(boost::xpressive::smatch what, FuseCommand& cc) {
    string analysis_s = what[0];
    if(boost::xpressive::regex_match(analysis_s, constprop)) {
      cc.push_back(new ConstantPropagationAnalysis(false));
      // cout << "analysis=cp" << endl;
    }
    else if(boost::xpressive::regex_match(analysis_s, pointsto)) {
      cc.push_back(new PointsToAnalysis(false));
      // cout << "analysis=pointsto" << endl;
    }
    else if(boost::xpressive::regex_match(analysis_s, deadpath)) {
      cc.push_back(new DeadPathElimAnalysis());
      // cout << "analysis=deadpath" << endl;
    }
    else if(boost::xpressive::regex_match(analysis_s, callcontext)) {
      cc.push_back(new CallContextSensitivityAnalysis(2, CallContextSensitivityAnalysis::callSite, false));
    }
    else if(boost::xpressive::regex_match(analysis_s, constcount)) {
      cc.push_back(new ConstPropCountAnalysis());
    }
    else {
      ostringstream oss;
      oss << "No match in match_analysis(what=" << what[0] << ")\n";
      throw MatchException(oss.str());
    }
    // cout << "match_analyis() " << cc.str() << endl;
  }

  void FuseCommandParser::matchComposerToken(boost::xpressive::smatch what, FuseCommand& cc) {
    string compinput = what[0];
    boost::xpressive::smatch cwhat;

    if(boost::xpressive::regex_match(compinput, cwhat, seqcomp)) {
      cc.setCType(FuseCommand::seq);
      // cout << "composer=" << cwhat[0] << endl;
    }
    else if(boost::xpressive::regex_match(compinput, cwhat, tightcomp)) {
      cc.setCType(FuseCommand::tight);
      // cout << "composer=" << cwhat[0] << endl;
    }
    else {
      ostringstream oss;
      oss << "No match in match_composer(what=" << what[0] << ")\n";
      throw MatchException(oss.str());
    }
    // cout << "match_composer() " << cc.str() << endl;
  }

  // headseq is analysis or scommand
  void FuseCommandParser::matchHeadToken(boost::xpressive::smatch what, FuseCommand& cc) {
    sregex hregex = analysis | scommand;
    string hstring = what[0];

    boost::xpressive::sregex_token_iterator begin(hstring.begin(), hstring.end(), hregex), end;
    for( ; begin != end; ++begin) {
      string token = *begin;
      boost::xpressive::smatch hwhat;
      if(boost::xpressive::regex_match(token, hwhat, analysis)) {
        matchAnalysisToken(hwhat, cc);
      }
      else if(boost::xpressive::regex_match(token, hwhat, scommand)) {
        FuseCommand* scc = new FuseCommand();
        matchCommandToken(hwhat, *scc);
        ComposedAnalysis* subcommand = scc->getFuseCommand();
        assert(subcommand);
        cc.push_back(subcommand);
      }
      else {
        ostringstream oss;
        oss << "No match in match_headseq(what=" << what[0] << ")\n";
        throw MatchException(oss.str());
      }      
    }
    // cout << "match_headseq() " << cc.str() << endl;
  }

  void FuseCommandParser::matchTailToken(boost::xpressive::smatch what, FuseCommand& cc) {
    sregex tregex = analysis | scommand;
    string tailstring = what[0];
  
    boost::xpressive::sregex_token_iterator begin(tailstring.begin(), tailstring.end(), tregex), end;
    for( ; begin != end; ++begin) {
      string token = *begin;
      boost::xpressive::smatch twhat;
      if(boost::xpressive::regex_match(token, twhat, analysis)) {
        matchAnalysisToken(twhat, cc);
      }
      else if(boost::xpressive::regex_match(token, twhat, scommand)) {
        FuseCommand* scc = new FuseCommand();
        matchCommandToken(twhat, *scc);
        ComposedAnalysis* subcommand = scc->getFuseCommand();
        assert(subcommand);
        cc.push_back((ComposedAnalysis*)subcommand);
      }
      else {
        ostringstream oss;
        oss << "No match in match_tailseq(what=" << what[0] << ")\n";
        throw MatchException(oss.str());
      }
    }
    // cout << "match_tailseq() " << cc.str() << endl;
  }

  // Every command has 3 tokens
  // It consists of composer followed by '(' and headseq, tailseq and ')'
  // Use the filter iterator to break it into parts
  void FuseCommandParser::matchCommandNested(boost::xpressive::smatch what, FuseCommand& cc) {
    // composer
    sregex_id_filter_predicate composer_id(composer.regex_id());
    sregex_id_filter_predicate headseq_id(headseq.regex_id());
    sregex_id_filter_predicate tailseq_id(tailseq.regex_id());

    if(composer_id(what)) {
      matchComposerToken(what, cc);
    }
    else if(headseq_id(what)) {
      matchHeadToken(what, cc);
    }
    else if(tailseq_id(what)) {
      matchTailToken(what, cc);
    }
    else {
      ostringstream oss;
      oss << "No match in match_command_nested(what=" << what[0] << ")\n";
      throw MatchException(oss.str());
    }
    // cout << "match_nested_command() " << cc.str() << endl;
  }

  void FuseCommandParser::matchCommandToken(string what_s, FuseCommand& cc) {
    boost::xpressive::smatch cwhat;
    if(regex_match(what_s, cwhat, command)) {
      boost::xpressive::smatch::nested_results_type::const_iterator begin = cwhat.nested_results().begin();
      boost::xpressive::smatch::nested_results_type::const_iterator end = cwhat.nested_results().end();
      boost::xpressive::smatch::nested_results_type::const_iterator c;  
      for(c = begin; c != end; ++c) {
        boost::xpressive::smatch nwhat = *c;
        matchCommandNested(nwhat, cc);
      }
    }
    else {
      ostringstream oss;
      oss << "No match in match_command(what=" << what_s << ")\n";
      throw MatchException(oss.str());
    }
    // cout << "match_command() " << cc.str() << endl;
  }

  void FuseCommandParser::matchCommandToken(boost::xpressive::smatch what, FuseCommand& cc) {
    string match_s = what[0];
    assert(match_s.size() > 0);
    matchCommandToken(match_s, cc);
  }

  FuseCommand* FuseCommandParser::operator()(string cmd_s) {
    try {
      FuseCommand* cc = new FuseCommand();
      matchCommandToken(cmd_s, *cc);
      return cc;
    }
    catch(MatchException& e) {
      cerr << e.what() << endl;
      return NULL;
    }
  }
}// end namespace
