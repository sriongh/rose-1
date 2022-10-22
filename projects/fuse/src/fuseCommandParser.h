#ifndef _FUSECOMMANDPARSER_H
#define _FUSECOMMANDPARSER_H

#include "compose.h"
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <exception>

namespace fuse {
  /***************
   * FuseCommand *
   ***************/
  class FuseCommand {
  public:
    typedef enum CompositionType{seq, tight, unknown} CType;
  private:
    std::list<ComposedAnalysis*> subanalyses;
    CType ctype;
    ComposedAnalysis* root; //! represents the root of this command
  public:
    FuseCommand();
    void setCType(CompositionType type);
    CompositionType getCType() const;
    void push_back(ComposedAnalysis* analysis);
    std::list<ComposedAnalysis*> getSubAnalysisList() const;
    void initFuseCommand();
    ComposedAnalysis* getFuseCommand();
    Composer* getRootComposer();
    void execute();
    std::string str() const;
    ~FuseCommand();
  };

  /******************
   * MatchException *
   ******************/
  class MatchException : public std::exception {
    std::string what_s;
  public:
    MatchException(std::string what) : what_s(what) { }
    virtual const char* what() const throw() {
      return what_s.c_str();
    }
    ~MatchException() throw() { }
  };

  /*********************
   * FuseCommandParser *
   *********************/
  /*!
   * Generic command parsing for the Fuse Compositional Analysis framework.
   * The fuse command language is described by the following grammar
   * analysis = cp | pt | mcc | mco | mv | dp
   * composer = seq | tight
   * scommand = composer(analysis . *(, analysis) | scommand)
   * command = composer( analysis | scommand . *(, analysis | scommand))
   */
  class FuseCommandParser {
    // add new composition type here
    boost::xpressive::sregex seqcomp, tightcomp;
    // to add new analysis
    // 1. add a new sregex
    // 2. initialize it under FuseCommandParser and also modify analysis regex
    // 3. add the analysis creation in matchAnalysisToken
    boost::xpressive::sregex constprop,
      constcount,
      callcontext,
      deadpath, 
      pointsto,
      analysis;
    
    boost::xpressive::sregex composer;
    boost::xpressive::sregex analysisList, atailseq, scommand, ctailseq, tailseq, headseq, command;
  public:
    FuseCommandParser();
    void matchAnalysisToken(boost::xpressive::smatch what, FuseCommand& cc);
    void matchComposerToken(boost::xpressive::smatch what, FuseCommand& cc);
    void matchHeadToken(boost::xpressive::smatch what, FuseCommand& cc);
    void matchTailToken(boost::xpressive::smatch what, FuseCommand& cc);
    void matchCommandNested(boost::xpressive::smatch what, FuseCommand& cc);
    void matchCommandToken(boost::xpressive::smatch what, FuseCommand& cc);
    void matchCommandToken(std::string what, FuseCommand& cc);
    FuseCommand* operator()(std::string command);
  };
};

#endif
