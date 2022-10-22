#pragma once
//#include "sight.h"
//
//// Applications that wish to disable Sight completely #define DISABLE_SIGHT
#ifdef DISABLE_SIGHT

// Class that makes it possible to generate string labels by using the << syntax.
// Examples: txt() << "a=" << (5+2) << "!"
//           txt("a=") << (5+2) << "!"
// Since this class is meant to be used by client code, it is placed inside the easier-to-access sight namespace
struct txt : std::string {
  txt() {}
  txt(const std::string& initTxt) {
    _stream << initTxt;
    assign(_stream.str());
  }
  
  template <typename T>
  txt& operator<<(T const& t) {    
    _stream << t;
    assign(_stream.str());
    return *this;
  }

  std::string str() const {
    return _stream.str();    
  }
 
  std::ostringstream _stream;
};

class printable
{
  public:
  virtual ~printable() {}
  //virtual void print(std::ofstream& ofs) const=0;
  virtual std::string str(std::string indent="") const=0;
};

#define SIGHT_VERB_DECL(Type, Args, verbThreshold, verbosity)

#define SIGHT_VERB_DECL_REF(Type, Args, ref, verbThreshold, verbosity)

#define SIGHT_VERB(code, verbThreshold, verbosity)

#define SIGHT_VERB_IF(verbThreshold, verbosity)

#define SIGHT_VERB_FI()

#define SIGHT_DECL(Type, Args, cond)

#define SIGHT_DECL_REF(Type, Args, ref, cond)

#define SIGHT(code, cond)

#define SIGHT_IF(cond)

#define SIGHT_FI()

#endif
